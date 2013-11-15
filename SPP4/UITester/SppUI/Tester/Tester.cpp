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

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void CaptureDevicesPopulate(HWND , int);
void SetDefaultBitRate(UINT);
void SetDefaultChannel(TCHAR);
void DisplayAudioStat(void);

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

	if (type == 2)
	{
		jack.id = _wcsdup(id[6]);
		jack.FriendlyName = _wcsdup(fn[6]);
		jack.Default = false;
		SendMessage(hTopUiWin, POPULATE_JACKS, (WPARAM)&jack, 0);
		free(jack.id);
		free(jack.FriendlyName);

		jack.id = _wcsdup(id[4]);
		jack.FriendlyName = _wcsdup(fn[4]);
		jack.Default = true;
		SendMessage(hTopUiWin, POPULATE_JACKS, (WPARAM)&jack, 0);
		free(jack.id);
		free(jack.FriendlyName);
	}
	else
	{
		for (int i=0; i<7;i++)
		{
		jack.id = _wcsdup(id[i]);
		jack.FriendlyName = _wcsdup(fn[i]);
		if(i==1) 
			jack.Default = true;
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
	std::wstring str;
	str = L"Bitrate="+ std::to_wstring(DefaultBitRate) + L"; Channel=" + DefaultChannel;
	SetWindowText(hMainAppWnd, str.c_str());
	UpdateWindow(hMainAppWnd);
}
