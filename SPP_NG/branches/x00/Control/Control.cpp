// Control.cpp : Defines the entry point for SmartPropoPlus.
//
// State machine that controls the operation of other units
// Hidden main window that controls system tray icon and messages
// System tray icon & tool tip reflects state of SPP
// System tray balloon reflects changes in state of SPP
// System tray menu by right click on icon
//
// Only one instance of SPP can run on a machine (Singleton)
// SPP can run on Vista SP2 (or higher) and on Windows 7
//

#include "stdafx.h"
#include "Shellapi.h"
#include "Control.h"
#include "Common.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
NOTIFYICONDATA g_ntfdata;						// Global notification data

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL                DeleteNotificationIcon(void);
BOOL				AddNotificationIcon(HWND hWnd, UINT uID);
BOOL				AddNotificationBalloon(TCHAR *InfoTitle, TCHAR *Info, DWORD Flags);
void                ShowContextMenu(HWND hwnd, POINT pt);
BOOL				IsVistaSP2OrLater(void);
BOOL				IsSingleton(void);


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	// Make sure that OS is Vista SP2 or higher
	if (!IsVistaSP2OrLater())
	{
		MessageBox(NULL, STR_EN_OS_TOO_OLD, STR_EN_TTL_SPP_ERR, MB_OK|MB_ICONERROR);
		return FALSE;
	};


	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_CONTROL, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Verifies that this is the only instance
	if (!IsSingleton())
	{
		MessageBox(NULL, STR_EN_ALREADY_RUN, STR_EN_TTL_SPP_ERR, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		MessageBox(NULL, STR_EN_CANNOT_INIT, STR_EN_TTL_SPP_ERR, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	MSG msg;
	HACCEL hAccelTable;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CONTROL));

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



ATOM MyRegisterClass(HINSTANCE hInstance)
{
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GREEN));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_CONTROL);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GREEN));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//

   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   AddNotificationIcon(hWnd, IDI_GREEN);	// Add system-tray icon
   AddNotificationBalloon(STR_EN_TTL_SPP_NONE, STR_EN_DFLT_TOOLTIP, NIIF_INFO );// Add system-tray balloon
   ShowWindow(hWnd, SW_HIDE); // Window is hidden
   UpdateWindow(hWnd);

   return TRUE;
}

BOOL AddNotificationIcon(HWND hWnd, UINT uID)
{
   // Create system tray icon
   g_ntfdata.cbSize = sizeof(NOTIFYICONDATA);
   g_ntfdata.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
   g_ntfdata.hWnd = hWnd;
   g_ntfdata.uID = uID;
   g_ntfdata.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_SHOWTIP ; // to be extended
   g_ntfdata.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(uID));
   memcpy(g_ntfdata.szTip,STR_EN_DFLT_TOOLTIP, ARRAYSIZE(g_ntfdata.szTip));
   //LoadString(hInst, STR_EN_DFLT_TOOLTIP, ntfdata.szTip, ARRAYSIZE(ntfdata.szTip));
   g_ntfdata.uVersion = NOTIFYICON_VERSION_4;

   if (!Shell_NotifyIcon(NIM_ADD, &g_ntfdata))
	   return FALSE;
   return Shell_NotifyIcon(NIM_SETVERSION, &g_ntfdata);
}
BOOL AddNotificationBalloon(TCHAR *InfoTitle, TCHAR *Info, DWORD Flags)
{
	g_ntfdata.uFlags |= NIF_INFO;
	memcpy(g_ntfdata.szInfoTitle,InfoTitle, ARRAYSIZE(g_ntfdata.szInfoTitle));
	memcpy(g_ntfdata.szInfo,Info, ARRAYSIZE(g_ntfdata.szInfo));
	g_ntfdata.dwInfoFlags = Flags;
	return Shell_NotifyIcon(NIM_MODIFY, &g_ntfdata);
}
BOOL DeleteNotificationIcon(void)
{

   return Shell_NotifyIcon(NIM_DELETE, &g_ntfdata);
}


void ShowContextMenu(HWND hwnd, POINT pt)
{
	// Show right-click menu
    HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_CONTEXTMENU));
    if (hMenu)
    {
        HMENU hSubMenu = GetSubMenu(hMenu, 0);
        if (hSubMenu)
        {
            // our window must be foreground before calling TrackPopupMenu or the menu will not disappear when the user clicks away
            SetForegroundWindow(hwnd);

            // respect menu drop alignment
            UINT uFlags = TPM_RIGHTBUTTON;
            if (GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0)
            {
                uFlags |= TPM_RIGHTALIGN;
            }
            else
            {
                uFlags |= TPM_LEFTALIGN;
            }

            TrackPopupMenuEx(hSubMenu, uFlags, pt.x, pt.y, hwnd, NULL);
        }
        DestroyMenu(hMenu);
    }
}


BOOL IsVistaSP2OrLater(void)
{
    // Initialize the OSVERSIONINFOEX structure to Vista SP2.
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 0;
	osvi.wServicePackMajor = 2;

    // Initialize the condition mask for Vista SP2.
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

    // Perform the test.
    if (VerifyVersionInfo(	&osvi, 
							VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR,
							dwlConditionMask))
		return TRUE; // Vista SP2

	return VerifyVersionInfo(	&osvi, 
								VER_MAJORVERSION | VER_MINORVERSION,
								dwlConditionMask);

}
BOOL IsSingleton(void)
{
	// If this process is first then a new mutex is created, no waiting and returns TRUE
	// If this is a consequent process then a handle to the mutex is obtained, timeout expires
	// so function return FALSE
	HANDLE hMutex =  CreateMutex(NULL, FALSE, STR_SINGLETON_MTX);
	DWORD waitstate = WaitForSingleObject(hMutex, 10);
	return (WAIT_OBJECT_0 == waitstate);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
		DeleteNotificationIcon(); // Remove icon
		PostQuitMessage(0);
		break;
    case WMAPP_NOTIFYCALLBACK:
		// Message from the system tray (a.k.a notification area)
        switch (LOWORD(lParam))
        {
        case WM_CONTEXTMENU:
            {
				// Dispaly the system tray icon context menu
                POINT const pt = { LOWORD(wParam), HIWORD(wParam) };
                ShowContextMenu(hWnd, pt);
            }
            break;
		};
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
// Message handler for about box.
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
