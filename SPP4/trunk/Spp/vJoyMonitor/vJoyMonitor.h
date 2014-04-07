#pragma once
//#define STRICT
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#define _CRT_SECURE_NO_DEPRECATE
#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

#include <vector>
#include <thread>
#include <set>
#include <map>
#include <dinput.h>
#include <dinputd.h>
#include "vjoymonitor.h"

#ifdef SPPINTERFACE_EXPORTS
#define SPPINTERFACE_API __declspec(dllexport)
#else
#define SPPINTERFACE_API __declspec(dllimport)
#endif

//#define STRICT
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#define _CRT_SECURE_NO_DEPRECATE
#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif


//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------


#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#define PID1	0xbead
#define PID2	0xbead1234

// HID Descriptor definitions
#define HID_USAGE_X		0x30
#define HID_USAGE_Y		0x31
#define HID_USAGE_Z		0x32
#define HID_USAGE_RX	0x33
#define HID_USAGE_RY	0x34
#define HID_USAGE_RZ	0x35
#define HID_USAGE_SL0	0x36
#define HID_USAGE_SL1	0x37
#define HID_USAGE_WHL	0x38
#define HID_USAGE_POV	0x39


// DLL Interface
SPPINTERFACE_API bool vJoyMonitorInit(HINSTANCE hInstance, HWND	ParentWnd);
SPPINTERFACE_API int  GetIdByIndex(int iDevice);
SPPINTERFACE_API bool ExistAxis(UINT iDevice, UINT Axis);
SPPINTERFACE_API bool ExistAxisX(UINT iDevice);
SPPINTERFACE_API bool ExistAxisY(UINT iDevice);
SPPINTERFACE_API bool ExistAxisZ(UINT iDevice);
SPPINTERFACE_API bool ExistAxisRx(UINT iDevice);
SPPINTERFACE_API bool ExistAxisRy(UINT iDevice);
SPPINTERFACE_API bool ExistAxisRz(UINT iDevice);
SPPINTERFACE_API bool ExistAxisSL1(UINT iDevice);
SPPINTERFACE_API bool ExistAxisSL2(UINT iDevice);
SPPINTERFACE_API int  GetNumButtons(UINT iDevice);
SPPINTERFACE_API int  GetNumvJoyDevices(void);
SPPINTERFACE_API void StartPollingDevice(UINT iDevice);
SPPINTERFACE_API void StopPollingDevice(UINT iDevice);
SPPINTERFACE_API void StopPollingDevices(void);
SPPINTERFACE_API void vJoyMonitorClose(void);


#define VJOY_MONITOR_CLASS		L"Dummy Class"
#define VJOY_MONITOR_TITLE		L"vJoyMonitor Window"
#define VJOY_UNIQUEID_PREFIX	L"vJoy_UniqueID_"
#define mapDB map<wstring, Device*>

#ifdef _DEBUG
#define THREAD_NAME(name) SetThreadName(name)
#else
#define THREAD_NAME(name)
#endif

/*
Device DB
Implemented as a map of Devices
Each entry represents a device - either detected or that was removed
Access to DB by device key - Unique ID (String)
*/
struct Device {
	wstring UniqueID;					// Unique ID of this device
	LPDIRECTINPUTDEVICE8 pDeviceDI8;	// Pointer to DirectInput Device
	bool isvJoyDevice;					// Always true
	UINT vJoyID;						// In the range 1-16 (Meaningfull only if isvJoyDevice=true)
	thread * ThPolling;					// Pointer to polling thread
	// Status
	bool isPrefered;					// Not implemented
	bool Exist;							// True only for devices that are currently enabled
	bool isPolling;						// Set/Reset by polling thread to indicate that the polling loop is running/killed  (Default=false). 
	bool EnPolling;						// Set to true to enable polling. Reset to kill  polling loop.
	bool RqPolling;						// Set by external module to indicate that polling is needed. Revoked to indicate that polling is not required (default=false).
};

class CvJoyMonitor
{
// Class interface functions
public:
	CvJoyMonitor(void);
	CvJoyMonitor(HINSTANCE hInstance, HWND	ParentWnd);
	virtual ~CvJoyMonitor(void);
	bool ExistAxis(UINT iDevice, UINT Axis);
	int  GetNumButtons(UINT iDevice);
	int  GetNumDevices(void) ;
	void StartPollingDevice(UINT vJoyID);
	void StopPollingDevice(UINT vJoyID);

// Private functions that are called from static/global functions
public:
	void CentralThread();
	BOOL EnumJoysticks(const DIDEVICEINSTANCE* pdidInstance);
	BOOL EnumObjects( const DIDEVICEOBJECTINSTANCE* pdidoi);
	void PollingThread(Device * dev);
	void IncEnumCount(void);
	int  GetIdByIndex(int iDevice);
	void StopPollingDevices(void);

// Class helper functions
private:
	wstring Convert2UniqueID(UINT vJoyID);
	UINT Convert2vJoyID(wstring& UniqueID);
	void PollDevice(wstring UniqueID);
	void SuspendPolling(wstring UniqueID);
	HWND CreateDummyWindow(void);
	void EnumerateDevices(void);
	void FreeDeviceDB(void);
	void StopCentralThread(void);

// These functions are callback functions that call functions within the object
private:
	static void _CentralThread(CvJoyMonitor * This);
	static void _PollingThread(CvJoyMonitor * This, Device * dev);
	void PostAxisValue(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SendButtonValue(UCHAR iDev,  BTNArr btnState);

// Private members
private:
	LPDIRECTINPUT8          m_pDI;	// One and only DI object 
	int						m_EnumerateCounter; // Counts the pending request to enumerate
	set<wstring>			m_ctSuspend; // Container (set) of keys - awaiting suspend
	set<wstring>			m_ctPoll; // Container (set) of keys - awaiting polling
	mapDB					m_DeviceDB; // One and only DB of devices
	recursive_mutex			m_mx_ctSuspend; // Mutex - RW to ctSuspend is critical region
	recursive_mutex			m_mx_ctPoll; // Mutex - RW to ctPoll is critical region
	thread *				m_thCentral; // Pointer to central thread
	HWND					m_hDummyWnd; // Handle to dummy window
	bool					m_CentralKeepalive; // Central Thread is killed when this is false
	UINT					m_CurrentID; // vJoy ID of DI device that is being enumerated
	HINSTANCE				m_hInstance;
	HWND					m_ParentWnd;
	int						m_nvJoyDevices;
	std::vector<char>		m_Id;
	std::vector<bool>		m_vJoySelected;
};

