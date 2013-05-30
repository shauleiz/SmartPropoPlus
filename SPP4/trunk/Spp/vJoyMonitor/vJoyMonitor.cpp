#include "stdafx.h"
#include "vJoyMonitor.h"



CvJoyMonitor::CvJoyMonitor(void) : m_ParentWnd(NULL), m_hInstance(NULL)
{
}

SPPINTERFACE_API CvJoyMonitor::CvJoyMonitor(HINSTANCE hInstance, HWND	ParentWnd) :
	m_ParentWnd(ParentWnd), m_hInstance(hInstance), m_DirectInput(false),
	m_pDI(NULL), m_pJoystick(NULL)

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


}


CvJoyMonitor::~CvJoyMonitor(void)
{
	SAFE_RELEASE( m_pDI );
}

SPPINTERFACE_API  bool CvJoyMonitor::IsvJoyDevice(UINT iJoy)
{

	JOYCAPS caps;
	MMRESULT res =  joyGetDevCaps(iJoy, &caps, sizeof (JOYCAPS));

	if (res==JOYERR_NOERROR)
		return(true);
	else
		return(false);
}

