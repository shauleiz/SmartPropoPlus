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

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
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

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
#endif

class CvJoyMonitor
{
public:
	CvJoyMonitor(void);
	SPPINTERFACE_API CvJoyMonitor(HINSTANCE hInstance, HWND	ParentWnd);
	virtual ~CvJoyMonitor(void);
	SPPINTERFACE_API  bool CvJoyMonitor::IsvJoyDevice(UINT iJoy);

private:
	HINSTANCE				m_hInstance;
	HWND					m_ParentWnd;
	bool					m_DirectInput;
	LPDIRECTINPUT8          m_pDI;
	LPDIRECTINPUTDEVICE8    m_pJoystick;

};


