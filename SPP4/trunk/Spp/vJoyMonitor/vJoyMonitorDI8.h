#pragma once
#include <vector>
#include <thread>
#include <set>
#include <map>
#include "vjoymonitor.h"

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

class CvJoyMonitorDI8 :	public CvJoyMonitor
{
// Class interface functions
public:
	CvJoyMonitorDI8(void);
	CvJoyMonitorDI8(HINSTANCE hInstance, HWND	ParentWnd);
	virtual ~CvJoyMonitorDI8(void);
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

// Class helper functions
private:
	wstring Convert2UniqueID(UINT vJoyID);
	UINT Convert2vJoyID(wstring& UniqueID);
	void PollDevice(wstring UniqueID);
	void SuspendPolling(wstring UniqueID);
	HWND CreateDummyWindow(void);
	void EnumerateDevices(void);
	void FreeDeviceDB(void);

// These functions are callback functions that call functions within the object
private:
	static void _CentralThread(CvJoyMonitorDI8 * This);
	static void _PollingThread(CvJoyMonitorDI8 * This, Device * dev);

// Private members
private:
	LPDIRECTINPUT8          m_pDI;	// One and only DI object 
	int						m_EnumerateCounter; // Counts the pending request to enumerate
	set<wstring>			m_vcSuspend; // Vector of keys - awaiting suspend
	set<wstring>			m_vcPoll; // Vector of keys - awaiting polling
	mapDB					m_DeviceDB; // One and only DB of devices
	recursive_mutex			m_mx_vcSuspend; // Mutex - RW to quSuspend is critical region
	recursive_mutex			m_mx_vcPoll; // Mutex - RW to quPoll is critical region
	thread *				m_thCentral; // Pointer to central thread
	HWND					m_hDummyWnd; // Handle to dummy window
	bool					m_CentralKeepalive; // Central Thread is killed when this is false
	UINT					m_CurrentID; // vJoy ID of DI device that is being enumerated
};

