#pragma once
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SPPINTERFACE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SPPINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SPPINTERFACE_EXPORTS
#define SPPINTERFACE_API __declspec(dllexport)
#else
#define SPPINTERFACE_API __declspec(dllimport)
#endif

//#define STRICT
#define DIRECTINPUT_VERSION 0x0800
#define _CRT_SECURE_NO_DEPRECATE
#ifndef _WIN32_DCOM
#define _WIN32_DCOM
#endif

#include <windows.h>
#include <commctrl.h>
#include <basetsd.h>
#include <dinput.h>
#include <dinputd.h>
#include <assert.h>
#include <oleauto.h>
#include <shellapi.h>
#include <Windowsx.h>
#include <wbemidl.h>
#include <vector>
#include <thread>

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------


#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif
#define sleep_for(_X) std::this_thread::sleep_for( std::chrono::milliseconds(_X));

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

struct DI_ENUM_CONTEXT
{
    DIJOYCONFIG* pPreferredJoyCfg;
    bool bPreferredJoyCfgValid;
};

class CvJoyMonitor
{
public:
	CvJoyMonitor(void);
	CvJoyMonitor(HINSTANCE hInstance, HWND	ParentWnd);
	virtual ~CvJoyMonitor(void);
	void SetId(char id);
	int  GetIdByIndex(int iDevice);
	virtual bool ExistAxis(UINT iDevice, UINT Axis);
	virtual int  GetNumButtons(UINT iDevice);
	bool ExistAxisX(UINT iDevice);
	bool ExistAxisY(UINT iDevice);
	bool ExistAxisZ(UINT iDevice);
	bool ExistAxisRx(UINT iDevice);
	bool ExistAxisRy(UINT iDevice);
	bool ExistAxisRz(UINT iDevice);
	bool ExistAxisSL1(UINT iDevice);
	bool ExistAxisSL2(UINT iDevice);
	virtual int  GetNumDevices(void);
	virtual void StartPollingDevice(UINT iDevice);
	virtual void StopPollingDevice(UINT iDevice);
	virtual void StopPollingDevices(void);

protected:
	void PostAxisValue(UCHAR iDev, UINT Axis, UINT32 AxisValue);

protected:
	HINSTANCE				m_hInstance;
	HWND					m_ParentWnd;
	int						m_nvJoyDevices;
	std::vector<char>		m_Id;
	std::vector<bool>		m_vJoySelected;
};


