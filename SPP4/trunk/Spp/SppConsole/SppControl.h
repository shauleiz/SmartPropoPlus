#pragma once

#include "resource.h"

#ifdef X64
#ifdef _DEBUG
#pragma  comment(lib, "..\\x64\\Debug\\SppProcess.lib")
#pragma  comment(lib, "..\\x64\\Debug\\vJoyInterface.lib")
#pragma  comment(lib, "..\\x64\\Debug\\vJoyMonitor.lib")
#else
#pragma  comment(lib, "..\\x64\\Release\\SppProcess.lib")
#pragma  comment(lib, "..\\x64\\Release\\vJoyInterface.lib")
#pragma  comment(lib, "..\\x64\\Release\\vJoyMonitor.lib")
#endif
#else
#ifdef _DEBUG
#pragma  comment(lib, "..\\Debug\\SppProcess.lib")
#pragma  comment(lib, "..\\Debug\\vJoyInterface.lib")
#pragma  comment(lib, "..\\Debug\\vJoyMonitor.lib")
#else
#pragma  comment(lib, "..\\Release\\SppProcess.lib")
#pragma  comment(lib, "..\\Release\\vJoyInterface.lib")
#pragma  comment(lib, "..\\Release\\vJoyMonitor.lib")
#endif
#endif

#ifdef _DEBUG
#define THREAD_NAME(name) SetThreadName(name)
#else
#define THREAD_NAME(name)
#endif

#define MAIN_CLASS_NAME TEXT("Main_Window_Class_Name")
#define MAIN_WND_TITLE TEXT("Main Window (Debug Mode)")
#define	DEF_BITRATE	8
#define DEF_CHANNEL	TEXT("Left")

static UINT NEAR WM_INTERSPPCONSOLE;
static UINT NEAR WM_INTERSPPAPPS;

struct jack_info
{
	/* Inter-unit information about jack/capture endpoint information */
	int	struct_size;
	WCHAR * id;
	COLORREF color;
	WCHAR * FriendlyName;
	//bool	Enabled;
	bool	Default;
	int		nChannels;
};
