/*
	vJoyMonitor - DLL to monitor installed vJoy devices

	This DLL should serve as the core of a joy.cpl-like application
	
	Interface:
		- Initialization: Passing to the DLL parent window as a target to messages
		- Get list of enabled vJoy devices. Each device has a zero-based index and features.
			Featurs include:
			+ ID (1 to 16 if known, -1 if unknown).
			+ Axes (Names, ranges)
			+ Buttons (number)
		- Poll device values (Axis value, button pressed)
		- All access to devices is by device index. Index may be obtained by ID (if exists)
		- Changes (in values or features) will be reported to parent window by messages

	Architecture:
	This DLL includes two implementations of the monitor logic:
	- DirectInput8 based to support full range of features (Default)
	- Legacy (WinMM) based (fall back)
	They are implemented as two classes (CvJoyMonitorDI8 / CvJoyMonitorMM) that are derived from calass CvJoyMonitor
	In addition, there are some global functions and variables that buffer between the calling application and the monitor logic
	The global functionality includes:
	- Initialization: Select the correct implementation of monitor logic, create an appropriate object, save it as a global and initialize it.
	- DLL interface functions that mirror the implementations of the public methods of CvJoyMonitor
*/

#include "stdafx.h"
#include "vJoyMonitor.h"

BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );

CvJoyMonitor::CvJoyMonitor(void) : m_ParentWnd(NULL), m_hInstance(NULL)
{
}

SPPINTERFACE_API CvJoyMonitor::CvJoyMonitor(HINSTANCE hInstance, HWND	ParentWnd) :
	m_ParentWnd(ParentWnd), m_hInstance(hInstance), m_DirectInput(false),
	m_pDI(NULL),  m_nvJoyDevices(0)

{
	HRESULT hr;


	if (!m_ParentWnd || !m_hInstance)
		return;

	// // /// //// Test if DirectInput supported.
    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&m_pDI, NULL ) ) )
        m_DirectInput = false;
	else
		m_DirectInput = true;

	IsvJoyDevice(1);
}


CvJoyMonitor::~CvJoyMonitor(void)
{
	SAFE_RELEASE( m_pDI );
}

SPPINTERFACE_API  bool CvJoyMonitor::IsvJoyDevice(UINT iJoy)
{

	// DirectInput not installed
	if (!m_DirectInput)
	{
		JOYCAPS caps;
		MMRESULT res =  joyGetDevCaps(iJoy, &caps, sizeof (JOYCAPS));
		if (res ==JOYERR_NOERROR && caps.wPid == PID1)
			return(true);
		else
			return(false);
	}

	// Direct input installed
	else
	{
		// Enumerate all devices
		if (FAILED(m_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY ) ) )
			return false;
	}

	// For every device - get its ID
	for (int i=0; i<m_nvJoyDevices; i++)
	{
	// Set the data format to "simple Joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    if( FAILED(m_pJoystick[i]->SetDataFormat( &c_dfDIJoystick2 ) ) )
        continue;

	// Enumerate the Joystick objects. The callback function enabled user
    // interface elements for objects that are found, and sets the min/max
    // values property for discovered axes.
    if( FAILED(m_pJoystick[i]->EnumObjects( EnumObjectsCallback, ( VOID* )this, DIDFT_ALL ) ) )
        continue;

	}


	return true;
}

BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
		return ((CvJoyMonitor *)pContext)->EnumJoysticks(pdidInstance);
}

BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext )
{
	// This is the ID (pdidoi)->wReportId
	return TRUE;
}

BOOL CvJoyMonitor::EnumJoysticks(const DIDEVICEINSTANCE* pdidInstance)
{
	HRESULT hr;

	if (((pdidInstance)->guidProduct).Data1 == PID2)
	{
		// Obtain an interface to the enumerated Joystick.
		hr = m_pDI->CreateDevice( pdidInstance->guidInstance, &m_pJoystick[m_nvJoyDevices], NULL );
		// If it failed, then we can't use this Joystick. (Maybe the user unplugged
		// it while we were in the middle of enumerating it.)
		if( FAILED( hr ) )
			return DIENUM_CONTINUE;
		m_nvJoyDevices++;
	}

	return TRUE;
}