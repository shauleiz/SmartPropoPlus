#include "stdafx.h"
#include "vJoyMonitorDI8.h"

#ifndef THREAD_NAME
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
#define THREAD_NAME(name) SetThreadName(name)
#else
#define THREAD_NAME(name)
#endif
#endif


// Forward Declarations
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
LRESULT CALLBACK MainWindowProc(_In_  HWND hwnd, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam  );


CvJoyMonitorDI8::CvJoyMonitorDI8(void)
{
	m_vecJoystick.resize(16,NULL);
}


CvJoyMonitorDI8::CvJoyMonitorDI8(HINSTANCE hInstance, HWND	ParentWnd) : CvJoyMonitor(hInstance, 	ParentWnd), 
	m_DirectInput(false),  m_pDI(NULL), m_hDummyWnd(NULL), m_EnumAllvJoyDevicesCount(0)

{
	//THREAD_NAME("CvJoyMonitorDI8");

	if (!m_ParentWnd || !m_hInstance)
		return;

	m_vecJoystick.resize(16,NULL);
	m_vJoySelected.resize(17,false);

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

	// Create a Central vJoyMonitor thread
	// It constantly monitors the activity of vJoyMonitor object
	m_CentralKeepalive = true;
	m_thCentral = new std::thread(CentralThread, this);

	// Create vector of all devices
	m_EnumAllvJoyDevicesCount=1;
}

CvJoyMonitorDI8::~CvJoyMonitorDI8(void)
{
	m_CentralKeepalive = false;
	RemoveAllJoysticks();
	SAFE_RELEASE(m_pDI);
	DestroyWindow(m_hDummyWnd);
}

void CvJoyMonitorDI8::CentralThread(CvJoyMonitorDI8 * This)
{
	THREAD_NAME("CvJoyMonitorDI8 Central thread (Loop)");
	if (!This)
		return;

	UINT size = (UINT)This->m_vJoySelected.size();
	UINT iDevice;

	while (This->m_CentralKeepalive)
	{
		sleep_for(100); // MilliSec

		// If counter positive then enumerate all vJoy Devices
		if (This->m_EnumAllvJoyDevicesCount>0)
		{
			This->EnumAllvJoyDevices();
			This->m_EnumAllvJoyDevicesCount--;
		}

		// Check if a device is waiting to be polled
		for (UINT i=1; i<size; i++)
		{
			if (!This->m_vJoySelected[i])
				continue;

			// Convert to index
			iDevice = This->Id2index(i);
			if (iDevice>=size || !This->m_vecJoystick[iDevice])
				continue;

			// If already running do nothing
			if (This->m_vecJoystick[iDevice]->tPoll)
				continue;

			// Start polling
			This->StartPollingDevice(i);

		}; // For loop

	}
}

BOOL CvJoyMonitorDI8::EnumJoysticks(const DIDEVICEINSTANCE* pdidInstance)
{
	HRESULT hr;

	if (((pdidInstance)->guidProduct).Data1 == PID2)
	{
		// Obtain an interface to the enumerated Joystick.
		m_vecJoystick[m_nvJoyDevices] = new DEV;
		m_vecJoystick[m_nvJoyDevices]->Id =0;
		m_vecJoystick[m_nvJoyDevices]->poll = false;
		m_vecJoystick[m_nvJoyDevices]->tPoll = NULL;
		hr = m_pDI->CreateDevice( pdidInstance->guidInstance, &(m_vecJoystick[m_nvJoyDevices]->pJoystick), NULL );
		// If it failed, then we can't use this Joystick. (Maybe the user unplugged
		// it while we were in the middle of enumerating it.)
		if( FAILED( hr ) )
			return DIENUM_CONTINUE;
		m_nvJoyDevices++;
	}

	return TRUE;
}

int CvJoyMonitorDI8::EnumAllvJoyDevices(void)
{

	lock_guard<mutex> lg(m_mxEnumAllvJoyDevices);

	// Cleaning up
	RemoveAllJoysticks();
	m_nvJoyDevices=0;
	m_Id.clear();

	// Enumerate all devices - results is a vector of pointers to vJoy devices and the number of devices
	if (FAILED(m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY ) ) )
		return 0;
	m_Id.resize(m_nvJoyDevices,0);

	// For every device - get its ID
	for (m_iDev=0; m_iDev<m_nvJoyDevices; m_iDev++)
	{
		// Set the data format to "simple Joystick" - a predefined data format 
		//
		// A data format specifies which controls on a device we are interested in,
		// and how they should be reported. This tells DInput that we will be
		// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
		if( FAILED(m_vecJoystick[m_iDev]->pJoystick->SetDataFormat( &c_dfDIJoystick2 ) ) )
			continue;

		// Enumerate the Joystick objects. The callback function enabled user
		// interface elements for objects that are found, and sets the min/max
		// values property for discovered axes.
		if( FAILED(m_vecJoystick[m_iDev]->pJoystick->EnumObjects( EnumObjectsCallback, ( VOID* )this, DIDFT_AXIS|DIDFT_BUTTON|DIDFT_POV ) ) )
			continue;
	}


	return m_nvJoyDevices;
}

void  CvJoyMonitorDI8::SetId(char id)
{
	m_Id[m_iDev] = id;
	m_vecJoystick[m_iDev]->Id = id;
}

BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
		return ((CvJoyMonitorDI8 *)pContext)->EnumJoysticks(pdidInstance);
}

BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
	((CvJoyMonitorDI8 *)pContext)->SetId((char)((pdidoi)->wReportId));
	int inst = DIDFT_GETINSTANCE((pdidoi)->dwType);
	int ButtonNumber=0;
	if ((pdidoi)->dwType & DIDFT_BUTTON)
		ButtonNumber = inst+1;
	return TRUE;
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
			 This->m_EnumAllvJoyDevicesCount++;
			 return TRUE;

        default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}

bool CvJoyMonitorDI8::ExistAxis(UINT Id, UINT Axis)
{
	int iDevice = Id2index(Id);

	if ((iDevice<0) | (m_vecJoystick.size()<=(UINT)iDevice) || (Axis > HID_USAGE_SL1) || (Axis< HID_USAGE_X))
		return false;

	// Get data about axis Rx for this device
	DIDEVICEOBJECTINSTANCE Info;
	Info.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);
	HRESULT hr = m_vecJoystick[iDevice]->pJoystick->GetObjectInfo(&Info, DIMAKEUSAGEDWORD(1, Axis), DIPH_BYUSAGE);

	if (hr == S_OK)
		return true;
	else
		return false;
}

int  CvJoyMonitorDI8::GetNumButtons(UINT Id)
{
	int iDevice = Id2index(Id);

	if ((iDevice<0) | (m_vecJoystick.size()<=(UINT)iDevice))
		return -1;

		// Get joystick capabilities
		// extract number of Buttons and number of Axes
		DIDEVCAPS Caps;
		Caps.dwSize = sizeof(DIDEVCAPS);
		if( FAILED(m_vecJoystick[iDevice]->pJoystick->GetCapabilities(&Caps )))
			return 0;
		else
			return Caps.dwButtons;
}

int   CvJoyMonitorDI8::GetNumDevices(void) 
{
	return m_nvJoyDevices;
}


// Start polling a vJoy device by Id
// This function has to be called after every reconnection of the device.
// Data is NOT buffered
// No need to stop polling befaore calling this function
void CvJoyMonitorDI8::StartPollingDevice(UINT Id) 
{
	HRESULT  hr;

	// Set device as selected
	m_vJoySelected[Id] = true;

	// Convert to index
	UINT iDevice = Id2index(Id);

	// Chack range
	if (iDevice >= m_vecJoystick.size())
		return;

	// Check if joystick exists
	if (!m_vecJoystick[iDevice])
		return;

	// Stop polling of this device (Just in case)
	StopPollingDevice(Id);
		
	// (Re)Set device as selected
	m_vJoySelected[Id] = true;

	// Set cooperative level so nothing interrupts the flow
	hr = m_vecJoystick[iDevice]->pJoystick->SetCooperativeLevel(m_hDummyWnd, DISCL_NONEXCLUSIVE|DISCL_BACKGROUND);
	if (hr != DI_OK)
		return;

	// Set the data format (just in case)
	hr = m_vecJoystick[iDevice]->pJoystick->SetDataFormat( &c_dfDIJoystick2 );
	if (hr != DI_OK)
		return;

	// Acquire device
	hr = m_vecJoystick[iDevice]->pJoystick->Acquire();
	if (!(hr == DI_OK || hr == S_FALSE))
		return;

	// Enable the polling loop in a separate thread
	m_vecJoystick[iDevice]->poll = true;

		// Start monitoring thread
	m_vecJoystick[iDevice]->tPoll = new std::thread(CvJoyMonitorDI8::LaunchPolling, this, iDevice);
	if (!m_vecJoystick[iDevice]->tPoll)
			return;
}	

// Stop polling a joystick device
// The joystick is called by ID (Useful only for vJoy)
// The ID is converted to vwctor index
// If the corresponding device is 
void CvJoyMonitorDI8::StopPollingDevice(UINT Id) 
{
	// Convert to index
	int iDevice = Id2index(Id);

	if (iDevice<0 || iDevice>15)
		return;
	
	// Set device as not selected
	m_vJoySelected[Id] = false;


	// Verify that device being polled
	if (!m_vecJoystick[iDevice] || !m_vecJoystick[iDevice]->poll)
		return;

	// Instruct thread to stop polling and wait for it to stop
	m_vecJoystick[iDevice]->poll = false;
	if (m_vecJoystick[iDevice]->tPoll && m_vecJoystick[iDevice]->tPoll->joinable())
		m_vecJoystick[iDevice]->tPoll->join();

	// Delete and clear the thread object
	delete(m_vecJoystick[iDevice]->tPoll);
	m_vecJoystick[iDevice]->tPoll = NULL;
}
void CvJoyMonitorDI8::LaunchPolling(LPVOID This, UINT iDevice)
{
	THREAD_NAME("CvJoyMonitorDI8 LaunchPolling");
	if (This)
		((CvJoyMonitorDI8 *)This)->PollDevice(iDevice);
}

void CvJoyMonitorDI8::PollDevice(UINT iDevice)
{
	HRESULT  hr;
	DIJOYSTATE2 state={1}, prevState={0};

	while (m_vecJoystick[iDevice]->poll)
	{

		// Poll
		hr = m_vecJoystick[iDevice]->pJoystick->Poll();

		// If polling fails then try to unacquire, wait then acquire again
		if (hr != DI_OK && hr != DI_NOEFFECT)
		{
			m_vecJoystick[iDevice]->pJoystick->Unacquire();
			sleep_for(200); // MilliSec
			hr =m_vecJoystick[iDevice]->pJoystick->Acquire();
			if FAILED(hr)
				continue;
		};

		// Get the state of the joystick - test what has changed and report changes
		hr = m_vecJoystick[iDevice]->pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), (LPVOID)(&state));
		if (hr == DI_OK)
		{ // Look for changes and report

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
		}
	}; // While

	// Release DI interface
	if (m_vecJoystick[iDevice]->pJoystick)
		m_vecJoystick[iDevice]->pJoystick->Unacquire();
}

//void CvJoyMonitorDI8::StopPollingDevices()
//{
//	size_t limit = m_poll.size();
//	for (size_t i=0; i<limit; i++)
//		m_poll[i] = false;
//}

int CvJoyMonitorDI8::Id2index(UINT Id)
{

	if (m_vecJoystick.empty())
		return -1;

	for (UINT i=0; i<m_vecJoystick.size(); i++)
	{
		if (m_vecJoystick[i] && m_vecJoystick[i]->Id == Id)
			return i;
	};

	return -1;
}

// Remove an entry from the vectore of joystick
// Removal by index ( index is in the range 0-15)
// After removal vector member becomes NULL
bool CvJoyMonitorDI8::RemoveJoystick(UINT iDevice)
{
	// Chack range
	if (iDevice >= m_vecJoystick.size())
		return false;

	// Check if joystick exists
	if (!m_vecJoystick[iDevice])
		return false;

	// Before removal - stop polling (if polling)
	StopPollingDevice(m_vecJoystick[iDevice]->Id);

	// Release
	m_vecJoystick[iDevice]->pJoystick->Release();
	delete m_vecJoystick[iDevice];
	m_vecJoystick[iDevice] = NULL;
	return true;
}

// Loop on joystick vectore
// Remove all entries
// In the end - the vector is clean
// Returns the number of joisticks actually removed
int CvJoyMonitorDI8::RemoveAllJoysticks(void)
{
	int size = (int)m_vecJoystick.size();
	int count=0;

	for (int i=0; i<size; i++)
	{
		if (RemoveJoystick(i))
			count++;
	}; // For

	return count;
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