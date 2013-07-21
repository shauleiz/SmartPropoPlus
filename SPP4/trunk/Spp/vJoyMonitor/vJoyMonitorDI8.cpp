/*

--------------------     JoyMonitor DI8 architecture  --------------------
---------------------------------------------------------------------------
Constructor
> Initialize joystick device DB - to be deleted by the destructor
> DirectInput8Create: Create one and only DI object - to be safe-released by the destructor
> CreateDummyWindow(): This window will handle system messages that indicate removal/insertion of HID devices - to be destroyed by the destructor (DestroyWindow())
> Start central thread
> Request Device enumeration - this will build a valid device DB

Threads:

1. Main execution thread:
Calls constructor and returns

2. Central Thread:
One and only Central thread - Killed by the destructor.
Loops forever with sleep_for(100) between iterations
Responds to requests to enumerate (DB initialization or System changed)
Responds to StartPolling requests (User selected OR post-enumeration)
Responds to StopPolling requests (User selected)
After each responce - Continue & Wait.

3. DummyWindow message pump:
Used for responding to WM_DEVICECHANGE by incrementing enumeration request counter.

4. Polling Threads:
One and only one thread per polled device (Limit is 16)
Loop while enPoll:
> Poll() DI device
> Update status (isPolling=true)
> GetDeviceState()
> If changes in position found - Update caller
Loop stopped when:
> Poll() failed
> Request to stop and exit thread (enPoll==false)
On exit from thread:
> Unaquire DI device
> Update status (isPolling=false)

Device DB
Implemented as a map of Devices
Each entry represents a device - either detected or that was removed
Access to DB by device key - Unique ID (String)
Members of entry:
> DI8 Device:	Used for Poll(), Acquire, unacquire etc.
> Type: Currently only vJoy supported
> vJoy ID: In the range 1-16
> Status members:
>> Prefered device: Set if this is the preferred (default) device - Not implemented
>> Exist: True only for devices that are currently enabled
>> Is Polling: Set/Reset by polling thread to indicate that the polling loop is running/killed  (Default=false). 
>> Enable Polling: Set to true to enable polling. Reset to kill  polling loop.
>> Request Polling: Set by external module to indicate that polling is needed. Revoked to indicate that polling is not required (default=false).
> ThPolling: Pointer to polling thread

Device Enumeration:
Maintains the device DB by going over all the DI joystick devices.
For each device - going over its objects (Buttons, Axes ..)
Devices are accessed by their unique id (wReportId - Only for vJoy)
Never removes a device entry.
By default, all devices in the DB are marked as Exist=false.
Only enumerated devices are marked as Exist=true.

Synchronization:
>Entry
>>Application Main thread
>>Calles constructor
>>Returns

> Enumeration is always executed in the Central thread - this ensures sequentiality.
> Request to enumeration:
>> In the form of a counter - Request increments counter, Starting enumeration decrements counter.
>> In constructor: Counter set to 1 (Before starting central thread)
>> From WM_DEVICECHANGE: Every change in the system increments counter
>Enumeration can start only when all Polling Threads are killed - This is dealt with by global Polling Thread Counter.

>External requests to start/kill polling threads are treated only by Central Thread.
>Central thread checks for requests in the following order:
>> Enumeration: If enumeration counter >0 then Call enumeration and go to loop.
>> Stop: If one or more devices are not to be polled - call Suspend Polling and go to loop.
>> Start:  If one or more devices are to be polled - call Poll Device  and go to loop.

Interface:
StartPolling(ID): Request to start polling a (vJoy) device by ID.
> Convert vJoy ID to Unique ID (string)
> Set  Request Polling - it will be treated by Central Thread.
> Central Thread: Call PollDevice( Unique ID)

StopPolling(ID): Request to stop polling a (vJoy) device by ID.
> Convert vJoy ID to Unique ID (string)
> Reset  Request Polling - it will be treated by Central Thread.
> Central Thread: Call SuspendPolling(Unique ID)

GetNumDevices()
GetNumButtons(ID)
ExistAxis(ID, Axis)

Major internal members:
PollDevice( Unique ID): Start polling:
> Set Enable Polling
> Start Polling thread
> Polling thread sets Is Polling

SuspendPolling(Unique ID): Stop Polling
> Reset Enable Polling
> Polling thread exits
> Resets Is Polling
> Clean pointer to thread

int m_EnumerateCounter:
> Set to 1 on creation to force first enumeration
> Incremented with every change in the system
> Decremented with every enumeration

vector m_vcSuspend:
> Container of of Keys to devices to Suspend
> Every Stop request adds an element
> Every granted stop request removes the corresponding element
> Adding/Removing are done inside a critical region

vector m_vcPoll:
> Container of of Keys to devices to Poll
> Every Start request adds an element
> Every granted Start request removes the corresponding element
> Adding/Removing are done inside a critical region
*/

#include "stdafx.h"
#include "vJoyMonitorDI8.h"

// Forward Declarations
BOOL CALLBACK _EnumJoysticks( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
BOOL CALLBACK _EnumObjects( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
LRESULT CALLBACK MainWindowProc(_In_  HWND hwnd, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam  );
void SetThreadName(char* threadName);


// Constractor
CvJoyMonitorDI8::CvJoyMonitorDI8(HINSTANCE hInstance, HWND	ParentWnd) : CvJoyMonitor(hInstance, 	ParentWnd)
{
	if (!m_ParentWnd || !m_hInstance)
		return;

	// Initialize
	m_DeviceDB.clear();
	m_thCentral=NULL;
	m_hDummyWnd=NULL;

	// // /// //// Test if DirectInput supported.
	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	// Create a DInput object
	if( FAILED(DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&m_pDI, NULL ) ) )
	{
		m_pDI->Release();
		m_pDI = NULL;
		return;
	};

	// Create a dummy window
	m_hDummyWnd = CreateDummyWindow();

	// Create vector of all devices
	m_EnumerateCounter=1;
	// Create a Central vJoyMonitor thread
	// It constantly monitors the activity of vJoyMonitor object
	m_CentralKeepalive = true;
	m_thCentral = new thread(_CentralThread, this);
}

CvJoyMonitorDI8::CvJoyMonitorDI8(void)
{
		m_DeviceDB.clear();
}

// Destructor
// Block all additional requests by killing the central thread
// Free device DB - this will also stop all polling threads
// Releace DI8 interface
// Kill dummy window
CvJoyMonitorDI8::~CvJoyMonitorDI8(void)
{
	m_CentralKeepalive = false;
	FreeDeviceDB();
	SAFE_RELEASE(m_pDI);
	DestroyWindow(m_hDummyWnd);
}

// Entry point for creating the Central Thread
void CvJoyMonitorDI8::_CentralThread(CvJoyMonitorDI8 * This)
{
	THREAD_NAME("CvJoyMonitorDI8 Central thread (Loop)");
	if (!This)
		return;

	This->CentralThread();
}

/*
One and only Central thread - Killed by the destructor.
Loops forever with sleep_for(100) between iterations
Responds to requests to enumerate (DB initialization or System changed)
Responds to StartPolling requests (User selected OR post-enumeration)
Responds to StopPolling requests (User selected)
After each responce - Continue & Wait.
*/
void CvJoyMonitorDI8::CentralThread()
{
	THREAD_NAME("CvJoyMonitorDI8 Central thread (Loop)");

	while (m_CentralKeepalive)
	{
		sleep_for(100); // MilliSec

		// Need enumeration? - If enumeration counter is positive then yes
		// Kill all polling threads then start enumerating then decrement counter
		if (m_EnumerateCounter>0)
			EnumerateDevices();

		else
		{
			// Need to kill polling threads? - if m_quSuspend is not empty then yes
			lock_guard<recursive_mutex> lock_vcSuspend(m_mx_vcSuspend);
			if (!m_vcSuspend.empty())
			{
				SuspendPolling(*(m_vcSuspend.begin()));
				continue;
				lock_vcSuspend;
			};

			// Need to create polling threads? - if m_mx_quPoll is not empty then yes
			lock_guard<recursive_mutex> lock_vcPoll(m_mx_vcPoll);
			if (!m_vcPoll.empty())
			{
				PollDevice(*(m_vcPoll.begin()));
				continue;
			};
		}
	}; // While

}

// Enumerate all existing devices and update DB accordingly
// Decrement enumerate counter when finished
void CvJoyMonitorDI8::EnumerateDevices(void)
{
	// Go over DB - for each element suspend polling and mark as not exist
	mapDB::iterator iMap;
	for (iMap=m_DeviceDB.begin(); iMap!=m_DeviceDB.end(); ++iMap)
	{
		SuspendPolling((*iMap).first);
		(*iMap).second->Exist=false;
	};//For Loop

	// Now enumerate all existing devices
	// For each device add (if does not exist) and mark as exist
	if (FAILED(m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, _EnumJoysticks, this, DIEDFL_ATTACHEDONLY ) ) )
		return;

	// Go over updated DB - for each element that request was set - add to request vector
	lock_guard<recursive_mutex> lock(m_mx_vcPoll);
	for (iMap=m_DeviceDB.begin(); iMap!=m_DeviceDB.end(); ++iMap)
		if ((*iMap).second->RqPolling)
			m_vcPoll.insert((*iMap).first);

	// Decrement counter
	m_EnumerateCounter--;
}

// Called for every enumerated DI device
// Calls the coresponding class method function
BOOL CALLBACK    _EnumJoysticks( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
		return ((CvJoyMonitorDI8 *)pContext)->EnumJoysticks(pdidInstance);
}

// Called for every enumerated DI device (only vJoy device supported)
// Create a DI device and enumerate the device object - this is to obtain the device ID
// Convert this ID to Unique ID and get the corresponding entry in DB or create one
// - If exists then mark as 'exist' and update DI device
BOOL CvJoyMonitorDI8::EnumJoysticks(const DIDEVICEINSTANCE* pdidInstance)
{
	HRESULT hr;
	LPDIRECTINPUTDEVICE8 pJoystick;

	// If not vJoy device then ignore
	if (((pdidInstance)->guidProduct).Data1 != PID2)
		return TRUE;

	// This is a vJoy device - Create a DI device
	hr = m_pDI->CreateDevice( pdidInstance->guidInstance, &pJoystick, NULL );
	// If it failed, then we can't use this Joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if( FAILED( hr ) )
		return DIENUM_CONTINUE;

	// Set the data format to "simple Joystick" - a predefined data format 
	//
	// A data format specifies which controls on a device we are interested in,
	// and how they should be reported. This tells DInput that we will be
	// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
	if( FAILED(pJoystick->SetDataFormat( &c_dfDIJoystick2 )))
			return TRUE;

	// Get device objects and ID
	m_CurrentID=0;
	if( FAILED(pJoystick->EnumObjects( _EnumObjects, ( VOID* )this, DIDFT_AXIS|DIDFT_BUTTON|DIDFT_POV ) ) )
			return TRUE;


	// Convert vJoy ID to Unique ID
	wstring UniqueID = Convert2UniqueID(m_CurrentID);

	// Find device in DB by unique ID
	// If found - Update its data
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap != m_DeviceDB.end() )
	{ // Found
		(*iMap).second->Exist=true;
		SAFE_RELEASE((*iMap).second->pDeviceDI8);
		(*iMap).second->pDeviceDI8=pJoystick;
	}
	else
	{ // Not found - Create
		Device * dev = new Device();
		dev->UniqueID=UniqueID;
		dev->Exist=true;
		dev->pDeviceDI8=pJoystick;
		dev->isvJoyDevice=true;
		dev->RqPolling=false;
		dev->ThPolling=NULL;
		dev->isPolling=false;
		dev->vJoyID = m_CurrentID;
		m_DeviceDB.emplace(UniqueID,dev);
	};

	return TRUE;
}

// Called for every DI device object (Button, axis etc)
// Calls the coresponding class method function
BOOL CALLBACK _EnumObjects( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
		return ((CvJoyMonitorDI8 *)pContext)->EnumObjects(pdidoi);
}


// Called for every DI device object (Button, axis etc)
BOOL CvJoyMonitorDI8::EnumObjects( const DIDEVICEOBJECTINSTANCE* pdidoi)
{
	// Get the vJoy ID
	m_CurrentID = (pdidoi)->wReportId;

	// TODO: More data colleting in the future ...

	return TRUE;
}


// vJoy only
// Convert vJoy ID to Unique ID
wstring  CvJoyMonitorDI8::Convert2UniqueID(UINT vJoyID)
{
	// Sanity check
	if (vJoyID>16)
		return L"";

	return VJOY_UNIQUEID_PREFIX+to_wstring(vJoyID);
}

// Kill the polling thread for the given uniqueID
// If thread exists then kill it and wait until it is killed
// When killed - Update status in DB (isPolling=false) 
// and remove corresponding entry from suspend vector (m_vcSuspend)
// 
void CvJoyMonitorDI8::SuspendPolling(wstring UniqueID)
{
    // DEBUG	return;
	// This entire function is a critical region
	lock_guard<recursive_mutex> lock(m_mx_vcSuspend);

	// Find entry in DB 
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap == m_DeviceDB.end() )
		return;

	// Found - now kill thread and wait for it to exit
	(*iMap).second->EnPolling=false;
	if ((*iMap).second->ThPolling && (*iMap).second->ThPolling->joinable())
		(*iMap).second->ThPolling->join();

	// Update device entry
	delete((*iMap).second->ThPolling);
	(*iMap).second->ThPolling = NULL;
	(*iMap).second->isPolling = false;

	// Find entry in suspend vector and remove
	auto iSuspend = m_vcSuspend.find(UniqueID);
	if (iSuspend != m_vcSuspend.end())
		m_vcSuspend.erase(iSuspend);
}


// Starts the polling thread for the given uniqueID
// Wait until it is running
// When running - Update status in DB (isPolling=true) 
// and remove corresponding entry from polling vector (m_vcPoll)
// 
void CvJoyMonitorDI8::PollDevice(wstring UniqueID)
{
	HRESULT  hr;


	// This entire function is a critical region
	lock_guard<recursive_mutex> lock(m_mx_vcPoll);

	// Find entry in DB 
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap == m_DeviceDB.end() )
		return;

	// Found
	(*iMap).second->RqPolling = true;

	// Check is exists, polling requested  and not running 
	if  (!(*iMap).second->Exist || (*iMap).second->isPolling)
		return;


		// Set cooperative level so nothing interrupts the flow
	hr = (*iMap).second->pDeviceDI8->SetCooperativeLevel(NULL/*m_hDummyWnd*/, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND);
	if (hr != DI_OK)
		return;

	// Set the data format (just in case)
	hr = (*iMap).second->pDeviceDI8->SetDataFormat( &c_dfDIJoystick2 );
	if (hr != DI_OK)
		return;


	// Acquire device
	hr = (*iMap).second->pDeviceDI8->Acquire();
	if (!(hr == DI_OK || hr == S_FALSE))
		return;
	
	// now start thread and wait for it to be running
	(*iMap).second->EnPolling=true;
	(*iMap).second->ThPolling = new thread(_PollingThread, this, (*iMap).second);
	if (!(*iMap).second->ThPolling)
		return;

	// Update device entry
	(*iMap).second->isPolling = true;

	// Find entry in Poll vector and remove
	auto iPoll = m_vcPoll.find(UniqueID);
	if (iPoll != m_vcPoll.end())
		m_vcPoll.erase(iPoll);

}

// Entry point for creating a Polling Thread
void CvJoyMonitorDI8::_PollingThread(CvJoyMonitorDI8 * This, Device * dev)
{
	string ThreadName = "CvJoyMonitorDI8 Polling Thread " + to_string(dev->vJoyID);

	THREAD_NAME((char *)ThreadName.data());
	if (This)
		This->PollingThread(dev);
}

// This is the polling loop - running on a dedicated thread
// Will loop as long as the corresponding EnPolling is true
void CvJoyMonitorDI8::PollingThread(Device * dev)
{
	HRESULT  hr;
	DIJOYSTATE2 state={1}, prevState={0};

	while (dev && dev->pDeviceDI8 && dev->EnPolling)
	{
		// Poll
		hr = dev->pDeviceDI8->Poll();

		// If polling fails then try to unacquire, wait then acquire again
		// If this fails too then go to beginning of loop
		if (hr != DI_OK && hr != DI_NOEFFECT)
		{
				dev->pDeviceDI8->Unacquire();
				SAFE_RELEASE(dev->pDeviceDI8);
				return;
		};

		// Get the state of the joystick - test what has changed and report changes
		hr = dev->pDeviceDI8->GetDeviceState(sizeof(DIJOYSTATE2), (LPVOID)(&state));
		if (hr != DI_OK)
		{
				dev->pDeviceDI8->Unacquire();
				SAFE_RELEASE(dev->pDeviceDI8);
				return;
		}
		else
		{ // Look for changes and report
			UCHAR iDevice = dev->vJoyID;
			// Axes
			if (state.lX != prevState.lX)
				PostAxisValue(iDevice, HID_USAGE_X, state.lX);
			if (state.lY != prevState.lY)
				PostAxisValue(iDevice, HID_USAGE_Y, state.lY);
			if (state.lZ != prevState.lZ)
				PostAxisValue(iDevice, HID_USAGE_Z, state.lZ);
			if (state.lRx != prevState.lRx)
				PostAxisValue(iDevice, HID_USAGE_RX, state.lRx);
			if (state.lRy != prevState.lRy)
				PostAxisValue(iDevice, HID_USAGE_RY, state.lRy);
			if (state.lRz != prevState.lRz)
				PostAxisValue(iDevice, HID_USAGE_RZ, state.lRz);
			if (state.rglSlider[0] != prevState.rglSlider[0])
				PostAxisValue(iDevice, HID_USAGE_SL0, state.rglSlider[0]);
			if (state.rglSlider[1] != prevState.rglSlider[1])
				PostAxisValue(iDevice, HID_USAGE_SL1, state.rglSlider[1]);

			// Buttons
			if (state.rgbButtons != prevState.rgbButtons)
			{// TODO: Write code
			}
			// Update
			prevState =  state;

			sleep_for(20); // MilliSec
		} // Look for changes and report

	}; // While


		// Release DI interface
		if (dev->pDeviceDI8)
			dev->pDeviceDI8->Unacquire();
}

// Instruct object to start a polling thread for the given vJoy ID
// Convert vJoyID to Unique ID then add the Unique ID to the Poll vector (m_vcPoll)
// The central thread will read this and take care of the rest
void CvJoyMonitorDI8::StartPollingDevice(UINT vJoyID)
{
	// Convert vJoy ID to Unique ID
	wstring UniqueID = Convert2UniqueID(vJoyID);

	// verify that this Unique ID is valid - Find corresponding device in DB by unique ID
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap == m_DeviceDB.end() )
		return;

	lock_guard<recursive_mutex> lock(m_mx_vcPoll);
	m_vcPoll.insert(UniqueID);
}

// Instruct object to stop a polling thread for the given vJoy ID
// Convert vJoyID to Unique ID then add the Unique ID to the Suspend vector (m_vcSuspend)
// The central thread will read this and take care of the rest
void CvJoyMonitorDI8::StopPollingDevice(UINT vJoyID)
{
	// Convert vJoy ID to Unique ID
	wstring UniqueID = Convert2UniqueID(vJoyID);

	// verify that this Unique ID is valid - Find corresponding device in DB by unique ID
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap == m_DeviceDB.end() )
		return;

	lock_guard<recursive_mutex> lock(m_mx_vcSuspend);
	m_vcSuspend.insert(UniqueID);
}

// Test if a given Axis esists in a given vJoy device
bool CvJoyMonitorDI8::ExistAxis(UINT vJoyID, UINT Axis)
{
	// Convert vJoy ID to Unique ID
	wstring UniqueID = Convert2UniqueID(vJoyID);

	// verify that this Unique ID is valid - Find corresponding device in DB by unique ID
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap == m_DeviceDB.end() )
		return false;

	// Verify that the device exists and valid
	if (!(*iMap).second->Exist || !(*iMap).second->pDeviceDI8)
		return false;

	// Get data about axis  for this device
	DIDEVICEOBJECTINSTANCE Info;
	Info.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
	HRESULT hr = (*iMap).second->pDeviceDI8->GetObjectInfo(&Info, DIMAKEUSAGEDWORD(1, Axis), DIPH_BYUSAGE);

	if (hr == S_OK)
		return true;
	else
		return false;
}

// Get the number of buttons for a given vJoy device
int  CvJoyMonitorDI8::GetNumButtons(UINT Id)
{
	// Convert vJoy ID to Unique ID
	wstring UniqueID = Convert2UniqueID(Id);

	// verify that this Unique ID is valid - Find corresponding device in DB by unique ID
	mapDB::iterator iMap;
	iMap = m_DeviceDB.find(UniqueID);
	if( iMap == m_DeviceDB.end() )
		return false;

	// Verify that the device exists and valid
	if (!(*iMap).second->Exist || !(*iMap).second->pDeviceDI8)
		return false;


		// Get joystick capabilities
		// extract number of Buttons
		DIDEVCAPS Caps;
		Caps.dwSize = sizeof(DIDEVCAPS);
		if( FAILED((*iMap).second->pDeviceDI8->GetCapabilities(&Caps )))
			return 0;
		else
			return Caps.dwButtons;
}

// Get the number of devices that are currently active (exist==true)
int   CvJoyMonitorDI8::GetNumDevices(void) 
{
	UINT count=0;
	// Go over DB 
	mapDB::iterator iMap;
	for (iMap=m_DeviceDB.begin(); iMap!=m_DeviceDB.end(); ++iMap)
		if ((*iMap).second->Exist)
			count++;
	return count;
}

// Free Device DB and perform clean up
// Go over the DB - For each entry:
// - Suspend polling thread
// - Release DI device and free memory
// - Free Device structure memory
// - Erase DB entry
void CvJoyMonitorDI8::FreeDeviceDB(void)
{
	// Go over DB 
	auto iMap = m_DeviceDB.begin();

	while (iMap != m_DeviceDB.end())
	{
		SuspendPolling((*iMap).first);
		SAFE_RELEASE((*iMap).second->pDeviceDI8);
		free ((*iMap).second);
		m_DeviceDB.erase(iMap);
		iMap++;
	}; // While loop

}

HWND CvJoyMonitorDI8::CreateDummyWindow(void)
{
	// Create main window that will receive messages when device is inserted/removed
	// and will relay the data to the dialog window.
	// This window will be visible only in Debug mode
	WNDCLASSEX WndClsEx;
	WndClsEx.cbSize = sizeof(WNDCLASSEX);
	WndClsEx.style = NULL;
	WndClsEx.lpfnWndProc = MainWindowProc;
	WndClsEx.cbClsExtra = 0;
	WndClsEx.cbWndExtra = 0;
	WndClsEx.hInstance = m_hInstance;
	WndClsEx.hIcon = NULL;
	WndClsEx.hCursor = NULL;
	WndClsEx.hbrBackground = (HBRUSH)COLOR_WINDOW;
	WndClsEx.lpszMenuName = NULL;
	WndClsEx.lpszClassName = VJOY_MONITOR_CLASS;
	WndClsEx.hIconSm = NULL;
	ATOM WndClassMain = RegisterClassEx(&WndClsEx);
	if (!WndClassMain)
		return NULL;

	HWND hwnd = CreateWindow( 
        VJOY_MONITOR_CLASS,        // name of window class 
        VJOY_MONITOR_TITLE,         // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        CW_USEDEFAULT,       // default width 
        CW_USEDEFAULT,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
        m_hInstance,           // handle to application instance 
        (LPVOID) this);      // no window-creation data 
	DWORD err = GetLastError();
	if (!hwnd)
		return NULL;
	else
		return hwnd;
}

/* Window Procedure for the (hidden) window*/
LRESULT CALLBACK MainWindowProc(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
  ) 
{ 
	static CvJoyMonitorDI8 * This = NULL;
	static LONG W,L;

	  switch (uMsg) 
    { 
        case WM_CREATE: 
            // Initialize the window.
			This = (CvJoyMonitorDI8 *)(( CREATESTRUCT *)lParam)->lpCreateParams;
            return 0; 
 
        case WM_PAINT: 
            // Paint the window's client area. 
            return 0; 
 
        case WM_SIZE: 
            // Set the size and position of the window. 
            return 0; 
 
        case WM_DESTROY: 
            // Clean up window-specific data objects. 
            return 0; 

		case WM_DEVICECHANGE:
			 This->IncEnumCount();
			 return TRUE;

        default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}

// Increment the counter of requests to enumerate devices
void CvJoyMonitorDI8::IncEnumCount(void)
{
	m_EnumerateCounter++;
}


#ifdef _DEBUG
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(char* threadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = GetCurrentThreadId();
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
#endif
