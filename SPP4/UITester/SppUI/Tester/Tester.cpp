// Tester.cpp : Defines the entry point for the application.
//
// Application - Test SPP4 user interface
// Opens top UI window (SppTopWin) that is the UI entry point
// Interacts with SppTopWin, simulating SPP4 activity
//
// Not part of SPP4 release
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <string>
#include <array>
#include "../SppTopWin/SppTopWin.h"
#include "WinMessages.h"
#include "Tester.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND  hMainAppWnd = NULL;						// Handle to the main app window
UINT  DefaultBitRate=0;
TCHAR DefaultChannel=TEXT('U');
WCHAR * DefaultJackId = NULL;
bool AutoBitrate, AutoChannel;
UINT vJoyDeviseSelected = 0;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void CaptureDevicesPopulate(HWND , int);
void SetDefaultBitRate(UINT);
void SetDefaultChannel(TCHAR);
void DisplayAudioStat(void);
void vJoyRemoveAll(HWND hTopUiWin);
void DisplayvJoyStat(void);
void SetAvailableControls(UINT id, HWND hDlg, int stat);
void SetAxesSlope(UINT id, bool GoingUp, HWND hDlg);
void SetvJoyInputSlope(bool GoingUp, HWND hDlg);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TESTER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTER));

	// Call GUI Window (TODO: Hide)
	MyEntryPoint(hInstance, hPrevInstance, lpCmdLine, SW_SHOW, hMainAppWnd);


	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TESTER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	hMainAppWnd=hWnd;

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HWND hTopUiWin;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		hTopUiWin = GetTopUiWnd();
		switch (wmId)
		{
		case IDM_ABOUT:
			SendMessage(hTopUiWin, POPULATE_JACKS, (WPARAM)L"Test Jack",0);
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			hTopUiWin = GetTopUiWnd();
			SendMessage(hTopUiWin, WM_COMMAND, wParam,lParam);
			DestroyWindow(hWnd);
			break;

		case IDM_AUDIO_REMALL:
			SendMessage(hTopUiWin, REM_ALL_JACK, 0,0);
			break;

		case IDM_AUDIO_POP2:
			CaptureDevicesPopulate(hTopUiWin, 2);
			break;

		case IDM_AUDIO_POP7:
			CaptureDevicesPopulate(hTopUiWin, 7);
			break;

		case IDM_RIGHT_CHANNEL:
			SendMessage(hTopUiWin, SET_AUDIO_PARAMS, 8,'R');
			break;

		case IDM_LEFT_CHANNEL:
			SendMessage(hTopUiWin, SET_AUDIO_PARAMS, 16,'L');
			break;
		case IDM_AUTO_CH1:
			SendMessage(hTopUiWin, SET_AUDIO_AUTO, (WPARAM)AUTOCHANNEL,  (WPARAM)AUTOCHANNEL);
			break;
		case IDM_AUTO_CH0:
			SendMessage(hTopUiWin, SET_AUDIO_AUTO, (WPARAM)AUTOCHANNEL,  0);
			break;
		case IDM_AUTO_BR1:
			SendMessage(hTopUiWin, SET_AUDIO_AUTO, (WPARAM)AUTOBITRATE, (LPARAM)AUTOBITRATE);
			break;
		case IDM_AUTO_BR0:
			SendMessage(hTopUiWin, SET_AUDIO_AUTO, (WPARAM)AUTOBITRATE, 0);
			break;
		case IDM_LEFT_HIGH:
			SendMessage(hTopUiWin, VJOYDEV_CH_LEVEL, (WPARAM)DefaultJackId, MAKELPARAM(99,25));//
			break;
		case IDM_RIGHT_HIGH:
			SendMessage(hTopUiWin, VJOYDEV_CH_LEVEL, (WPARAM)DefaultJackId, MAKELPARAM(20,100));//
			break;
		case IDM_MONO_HIGH:
			SendMessage(hTopUiWin, VJOYDEV_CH_LEVEL, (WPARAM)DefaultJackId, MAKELPARAM(85,85));//
			break;

		case IDM_VJOY_CLEAN:
			vJoyRemoveAll(hTopUiWin);
			break;

		case IDM_ADDVJOY_1:
			vJoyRemoveAll(hTopUiWin);
			SendMessage(hTopUiWin, VJOYDEV_ADD, 1, 1); // Add device 1, Selected
			break;

		case IDM_ADDVJOY_4:
			vJoyRemoveAll(hTopUiWin);
			SendMessage(hTopUiWin, VJOYDEV_ADD, 1, 0); // Add device 1, Not Selected
			SendMessage(hTopUiWin, VJOYDEV_ADD, 2, 0); // Add device 2, Not Selected
			SendMessage(hTopUiWin, VJOYDEV_ADD, 4, 1); // Add device 4, Selected
			SendMessage(hTopUiWin, VJOYDEV_ADD, 8, 0); // Add device 8, Not Selected
			break;

		case IDM_VJOY_CTRLS1:
			SetAvailableControls(1, hTopUiWin, 1);
			SetAvailableControls(4, hTopUiWin, 1);
			break;

		case IDM_VJOY_CTRLS2:
			SetAvailableControls(1, hTopUiWin, 2);
			SetAvailableControls(4, hTopUiWin, 2);
			break;

		case IDM_AXES_LO2HI1:
			SetAxesSlope(1, true, hTopUiWin);
			break;

		case IDM_AXES_HI2LO4:
			SetAxesSlope(4, false, hTopUiWin);
			break;

		case IDM_INPUT_LO2HI:
			SetvJoyInputSlope(true, hTopUiWin);
			break;

		case IDM_INPUT_HI2LO:
			SetvJoyInputSlope(false, hTopUiWin);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CLOSE:
		hTopUiWin = GetTopUiWnd();
		SendMessage(hTopUiWin, message, wParam,lParam);
		DestroyWindow(hWnd);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WMSPP_DLG_CHNL:
		SetDefaultBitRate((UINT)wParam);
		SetDefaultChannel((TCHAR)lParam);
		DisplayAudioStat();
		break;

	case WMSPP_DLG_AUTO:
		if ((WORD)wParam & AUTOCHANNEL)
			AutoChannel = (((WORD)lParam & AUTOCHANNEL)!=0);
		if ((WORD)wParam & AUTOBITRATE)
			AutoBitrate = (((WORD)lParam & AUTOBITRATE)!=0);
		DisplayAudioStat();
		break;

	case WMSPP_DLG_VJOYSEL:
		vJoyDeviseSelected = wParam;
		DisplayvJoyStat();
		break;

	case WMSPP_DLG_MAP:
		hTopUiWin = GetTopUiWnd();
		SendMessage(hTopUiWin, WMSPP_MAP_UPDT, wParam, lParam);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CaptureDevicesPopulate(HWND hTopUiWin, int type)
{
	jack_info jack;
	jack.struct_size = sizeof(jack_info);
	SendMessage(hTopUiWin, REM_ALL_JACK,0, 0);

	WCHAR * fn[7] = 
	{
		L"FriendlyName Number 1",
		L"FriendlyName Number 2",
		L"FriendlyName Number 3",
		L"FriendlyName Number 4",
		L"FriendlyName Number 5",
		L"FriendlyName Number 6",
		L"FriendlyName Number 7"
	};

	WCHAR * id[7] = 
	{
		L"{FriendlyName Number 1}",
		L"{FriendlyName Number 2}",
		L"{FriendlyName Number 3}",
		L"{FriendlyName Number 4}",
		L"{FriendlyName Number 5}",
		L"{FriendlyName Number 6}",
		L"{FriendlyName Number 7}"
	};

	COLORREF color[7] = 
	{
		red, green, blue, white, black, yellow, cyan
	};

	if (type == 2)
	{
		jack.id = _wcsdup(id[6]);
		jack.FriendlyName = _wcsdup(fn[6]);
		jack.Default = false;
		jack.color = color[6];
		SendMessage(hTopUiWin, POPULATE_JACKS, (WPARAM)&jack, 0);
		free(jack.id);
		free(jack.FriendlyName);

		jack.id = _wcsdup(id[4]);
		DefaultJackId = _wcsdup(id[4]);
		jack.FriendlyName = _wcsdup(fn[4]);
		jack.Default = true;
		jack.color = color[4];
		SendMessage(hTopUiWin, POPULATE_JACKS, (WPARAM)&jack, 0);
		free(jack.id);
		free(jack.FriendlyName);
	}
	else
	{
		for (int i=0; i<7;i++)
		{
			jack.id = _wcsdup(id[i]);
			jack.color = color[i];
			jack.FriendlyName = _wcsdup(fn[i]);
			if(i==1)
			{
				DefaultJackId = _wcsdup(id[i]);
				jack.Default = true;
			}
			else
				jack.Default = false;
			SendMessage(hTopUiWin, POPULATE_JACKS, (WPARAM)&jack, 0);
			free(jack.id);
			free(jack.FriendlyName);
		};
	};

	return;
}

void SetDefaultBitRate(UINT br)
{	DefaultBitRate = br;}

void SetDefaultChannel(TCHAR ch)
{ DefaultChannel = ch; }

void DisplayAudioStat(void)
{
	std::wstring str, AutoChannelStr, AutoBitrateStr;

	if (AutoChannel)
		AutoChannelStr = L"(A)";
	else
		AutoChannelStr = L"(M)";

	if (AutoBitrate)
		AutoBitrateStr = L"(A)";
	else
		AutoBitrateStr = L"(M)";

	str = L"Bitrate="+ std::to_wstring(DefaultBitRate) +AutoBitrateStr+ L"; Channel=" + DefaultChannel+AutoChannelStr;
	SetWindowText(hMainAppWnd, str.c_str());
	UpdateWindow(hMainAppWnd);
}

void vJoyRemoveAll(HWND hTopUiWin)
{SendMessage(hTopUiWin, VJOYDEV_REMALL, 0, 0);}

void DisplayvJoyStat(void)
{
	std::wstring str;
	str = L"Selected vJoy Device: " + std::to_wstring(vJoyDeviseSelected);
	SetWindowText(hMainAppWnd, str.c_str());
	UpdateWindow(hMainAppWnd);
}


void SetAvailableControls(UINT id, HWND hDlg, int stat)
{
	controls ctrls;
	ctrls.axis[0] = ctrls.axis[1] =ctrls.axis[2] = ctrls.axis[3] =
		ctrls.axis[4] = ctrls.axis[5] =ctrls.axis[6] = ctrls.axis[7] = TRUE;

	// Get data from vJoy Interface
	if (stat == 1)
		ctrls.nButtons = 4;
	else
	{
		ctrls.nButtons = 8;
		ctrls.axis[5] =ctrls.axis[6] =ctrls.axis[7] = FALSE;
	}

	// Send data to GUI
	SendMessage(hDlg, VJOYDEV_SETAVAIL, id, (LPARAM)&ctrls);
}

void SetAxesSlope(UINT id, bool GoingUp, HWND hDlg)
{
	UINT32 MaxRange = 100;
	int Step = MaxRange/8; 
	UINT32 AxisValue = 0;
	UINT32 Axis=0x30;

	if (!GoingUp)
	{
		Step = -1*Step;
		AxisValue = MaxRange;
	}

	for (int i=0; i<8; i++)
	{
		Axis=0x30+i;
		AxisValue+=Step;
		PostMessage(hDlg, WMSPP_JMON_AXIS, id + (Axis<<16), AxisValue);
	};

#define MAX_BUTTONS		128
	typedef std::array<BYTE, MAX_BUTTONS> BTNArr;
	BTNArr BtnVal;
	BtnVal.fill(0);

	for (int i=0; i<32; i++)
	{
		BtnVal[i] = i%2;
		if (!GoingUp)
			BtnVal[i] = BtnVal[i] ? 0 : 1;
	}

	SendMessage(hDlg, WMSPP_JMON_BTN, id , (LPARAM)&BtnVal);
}

void SetvJoyInputSlope(bool GoingUp, HWND hDlg)
{
	UINT32 nChannels = 16;
	UINT32 MaxRange = 100;
	int Step = MaxRange/nChannels;
	UINT32 ChannelValue=0;

	if (!GoingUp)
	{
		Step = -1*Step;
		ChannelValue = MaxRange;
	}

	for (int i=0; i<16; i++)
	{
		ChannelValue+=Step;
		PostMessage(hDlg, WMSPP_PRCS_RCHMNT, i, ChannelValue);
	};
}