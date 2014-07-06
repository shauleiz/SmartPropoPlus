// JoyMonitor.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "smartpropoplus.h"
#include "vJoyInterface.h"
#include "vJoyMonitor.h"
#include "JoyMonitorDlg.h"
#include "JoyMonitor.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CJoyMonitorDlg * Dialog = NULL;					// Pointer to the dialog box object
int	CurrDeviceId = -1;							// ID of vJoy device that is monitorred

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				SetAvailableControls(UINT id, HWND hDlg);
int					DetectAvailableDevices(HWND hDlg);			

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
	LoadString(hInstance, IDC_JOYMONITOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_JOYMONITOR));

	 

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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JOYMONITOR));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_JOYMONITOR);
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

   if (!hWnd)
   {
      return FALSE;
   }


   //ShowWindow(hWnd, nCmdShow);
   //UpdateWindow(hWnd);

   // Start the main dialog box
   Dialog	= new CJoyMonitorDlg(hInstance, hWnd);

   	// Open vJoy monitor
	bool MonitorOk = vJoyMonitorInit(hInstance, hWnd);

	// Get available vJoy devices and update combo box
	int selected = DetectAvailableDevices( Dialog->GetHandle());

	//
	SetAvailableControls(selected, Dialog->GetHandle());

	// Start polling vJoy Device 1 (Hard coded)
	StartPollingDevice(selected);

	CurrDeviceId = selected;

   return TRUE;
}

// Scan for the existing vJoy devices and set one of them as current
// Call the dialog box for every detected device
// Call the  dialog box to mark the detected one
int DetectAvailableDevices(HWND hDlg)
{
	bool exist=false;
	int sel=-1;

	// Loop on all possible vJoy devices
	for (int i=1; i<=16; i++)
	{
		if(GetVJDStatus(i) == VJD_STAT_FREE  || GetVJDStatus(i) == VJD_STAT_BUSY)
		{
			Dialog->AddDevice(i, !exist);
			if (!exist)
				sel = i;
			exist=true;
		}
	}; // Loop

	return sel;
}


void SetAvailableControls(UINT id, HWND hDlg)
{
	controls ctrls;
	// Get data from vJoy Interface
	ctrls.nButtons = GetVJDButtonNumber(id);
	for (UINT i=0; i<8; i++)
		ctrls.axis[i] = GetVJDAxisExist(id, HID_USAGE_X+i);

	// Send data to GUI
	SendMessage(hDlg, VJOYDEV_SETAVAIL, id, (LPARAM)&ctrls);
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

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WMSPP_JMON_AXIS:
		Dialog->SetJoystickAxisData((UCHAR)(wParam&0xFF), (UINT)(wParam>>16), (UINT32)lParam);
		break;

	case WMSPP_JMON_BTN:
		Dialog->SetButtonValues((UINT)wParam, (BTNArr *)lParam);
		break;

	case WMSPP_JMON_STP:
		Dialog->JoystickStopped((UCHAR)wParam);		
		break;

	case WMSPP_DLG_VJOYSEL:
		SetAvailableControls((UINT)wParam, Dialog->GetHandle());
		StopPollingDevice(CurrDeviceId);
		StartPollingDevice((UINT)wParam);
		CurrDeviceId = (UINT)wParam;
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
