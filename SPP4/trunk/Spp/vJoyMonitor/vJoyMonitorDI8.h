#pragma once
#include <vector>
#include <thread>
#include "vjoymonitor.h"

#define VJOY_MONITOR_CLASS L"Dummy Class"
#define VJOY_MONITOR_TITLE L"vJoyMonitor Window"

// Device status and polling control - valid only for existing devices
struct DEV {
	LPDIRECTINPUTDEVICE8 pJoystick;	// DirectInput Device
	bool	poll;					// Enable polling (Must be true for polling)
	std::thread * tPoll;			// Polling thread
	UINT	Id;						// vJoy ID (Range: 1-16)
};


class CvJoyMonitorDI8 :
	public CvJoyMonitor
{
public:
	CvJoyMonitorDI8(void);
	CvJoyMonitorDI8(HINSTANCE hInstance, HWND	ParentWnd);
	virtual ~CvJoyMonitorDI8(void);
	BOOL EnumJoysticks(const DIDEVICEINSTANCE* pdidInstance);
	void SetId(char id);
	bool ExistAxis(UINT iDevice, UINT Axis);
	int  GetNumButtons(UINT iDevice);
	int  GetNumDevices(void) ;
	void StartPollingDevice(UINT iDevice);
	void StopPollingDevice(UINT iDevice);
	void PollDevice(UINT iDevice);
	//void StopPollingDevices(void);

public:
	int	m_EnumAllvJoyDevicesCount;

private:
	static void LaunchPolling(LPVOID This, UINT iDevice);
	int Id2index(UINT iDevice);
	bool RemoveJoystick(UINT iDevice);
	int RemoveAllJoysticks(void);
	HWND CreateDummyWindow(void);
	static void CentralThread(CvJoyMonitorDI8 * This);
	int EnumAllvJoyDevices(void);

private:
	bool					m_DirectInput;
	LPDIRECTINPUT8          m_pDI;
	int						m_iDev;
	std::vector<DEV *>		m_vecJoystick;
	HWND					m_hDummyWnd;
	mutex					m_mxEnumAllvJoyDevices;
	bool					m_CentralKeepalive;
	std::thread	*			m_thCentral;
};

