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
#include "vJoyMonitorMM.h"
#include "vJoyMonitorDI8.h"
#include "../include/WinMessages.h"


// Global variables
CvJoyMonitor * g_MainObj = NULL; // Points to the active vJoyMonitor object
bool g_isDI8;


// Globals functions
SPPINTERFACE_API bool vJoyMonitorInit(HINSTANCE hInstance, HWND	ParentWnd)
{
	LPDIRECTINPUT8          pDI=NULL;
	// Test if DI8 installed. If Installed create an object, initialize it and store it
	if (!ParentWnd || !hInstance)
		return false;

	if (g_MainObj)
	{
		delete g_MainObj;
		g_MainObj = NULL;
	};

	// // /// //// Test if DirectInput supported.
	// Register with the DirectInput subsystem and get a pointer
	// to a IDirectInput interface we can use.
	// Create a DInput object
	if( FAILED( DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION, IID_IDirectInput8, ( VOID** )&pDI, NULL ) ) )
	{
		g_MainObj = new CvJoyMonitorMM(hInstance, ParentWnd);
		g_isDI8 = false;
	}
	else
	{
		g_MainObj = new CvJoyMonitorDI8(hInstance, ParentWnd);
		g_isDI8 = true;
	}

	SAFE_RELEASE( pDI );
	if (!g_MainObj)
		return false;
	else
		return true;
}
SPPINTERFACE_API int  GetIdByIndex(int iDevice)
{
	if (g_MainObj)
		return g_MainObj->GetIdByIndex(iDevice);
	else
		return -1;
}
SPPINTERFACE_API bool ExistAxis(UINT iDevice, UINT Axis)
{
	if (g_MainObj)
		return g_MainObj->ExistAxis( iDevice,  Axis);
	else
		return false;
}

SPPINTERFACE_API bool ExistAxisX(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_X);}
SPPINTERFACE_API bool ExistAxisY(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_Y);}
SPPINTERFACE_API bool ExistAxisZ(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_Z);}
SPPINTERFACE_API bool ExistAxisRx(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_RX);}
SPPINTERFACE_API bool ExistAxisRy(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_RY);}
SPPINTERFACE_API bool ExistAxisRz(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_RZ);}
SPPINTERFACE_API bool ExistAxisSL0(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_SL0);}
SPPINTERFACE_API bool ExistAxisSL1(UINT iDevice) {return ExistAxis(iDevice, HID_USAGE_SL1);}
SPPINTERFACE_API int  GetNumButtons(UINT iDevice) 
{
	if (g_MainObj)
		return g_MainObj->GetNumButtons(iDevice);
	else
		return -1;

}
SPPINTERFACE_API int  GetNumvJoyDevices(void)
{
	if (g_MainObj)
		return g_MainObj->GetNumDevices();
	else
		return -1;

}
SPPINTERFACE_API void StartPollingDevice(UINT iDevice)
	{
	if (g_MainObj)
		return g_MainObj->StartPollingDevice(iDevice);
}
SPPINTERFACE_API void StopPollingDevice(UINT iDevice)
	{
	if (g_MainObj)
		return g_MainObj->StopPollingDevice(iDevice);
}

SPPINTERFACE_API void StopPollingDevices(void)
{
	if (g_MainObj)
		return g_MainObj->StopPollingDevices();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////// Class CvJoyMonitor /////////////////////////////////////////////////////////////////////////////////
CvJoyMonitor::CvJoyMonitor(void) : m_ParentWnd(NULL), m_hInstance(NULL)
{
}

CvJoyMonitor::CvJoyMonitor(HINSTANCE hInstance, HWND	ParentWnd) :
	m_ParentWnd(ParentWnd), m_hInstance(hInstance),  m_nvJoyDevices(0)

{
	if (!m_ParentWnd || !m_hInstance)
		return;

	m_Id.resize(16, -1);
}


CvJoyMonitor::~CvJoyMonitor(void)
{
}

void  CvJoyMonitor::SetId(char id)
{
}

int  CvJoyMonitor::GetIdByIndex(int iDevice)
{
	if (iDevice >= m_nvJoyDevices || iDevice < 0)
		return 0;

	return m_Id[iDevice];
}

bool CvJoyMonitor::ExistAxis(UINT iDevice, UINT Axis)
{return false;}

int  CvJoyMonitor::GetNumButtons(UINT iDevice) {return -1;}
int   CvJoyMonitor::GetNumDevices(void) {return -1;}
void CvJoyMonitor::StartPollingDevice(UINT iDevice) {}
void CvJoyMonitor::StopPollingDevice(UINT iDevice) {}
void CvJoyMonitor::StopPollingDevices(void) {}

void CvJoyMonitor::PostAxisValue(UCHAR iDev, UINT Axis, UINT32 AxisValue)
{
	PostMessage(m_ParentWnd, WMSPP_JMON_AXIS,  m_Id[iDev] + (Axis<<16), AxisValue);
}
