// Audio.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Commctrl.h"
#include <vector>
#include <devicetopology.h>
#include <Mmdeviceapi.h>
#include "WinMessages.h"
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "AudioInputW7.h"
#include "NotificationClient.h"
#include <Richedit.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CAudioInputW7 * g_audio;						// Audio interface object (Only one for the moment)
HWND hLogDlg;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void				DlgPeakVolume(PVOID id, double peak);
INT_PTR CALLBACK	DlgListCaptureDevices(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	DlgAudioLog(HWND, UINT, WPARAM, LPARAM);
void	CaptureDevicesPopulate(HWND hDlg);
BOOL InitListViewColumns(HWND hWndListView) ;
void AddLine2List(HWND hWndListView, int size, LPWSTR id);




int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int  nCmdShow)
// Standard WinMain that creates single audio object
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	hLogDlg = NULL;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_AUDIO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	LoadLibrary(TEXT("Msftedit.dll"));

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUDIO));

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
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUDIO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_AUDIO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

void LogAudioUnit(int Code, int Severity, LPVOID Data)
{
	if (!hLogDlg)
		return;

	// Initialize
	LRESULT lr;
	HWND hEdit=NULL;
	WCHAR prefix[6], prtcode[8];
	GETTEXTLENGTHEX tl;
	CHARFORMAT cf;
	tl.codepage =  CP_ACP;
	tl.flags = GTL_DEFAULT;
	hEdit = GetDlgItem(hLogDlg, IDC_EDIT1);
	SendMessage(hEdit,EM_SHOWSCROLLBAR    , (WPARAM)SB_VERT, TRUE);

	// Severity to colour and Prefix
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	switch (Severity)
	{
	case ERR:
		cf.crTextColor = RGB(180,0,0);
		wmemcpy(prefix,L"[E] ",5);
		break;
	case FATAL:
		cf.crTextColor = RGB(250,0,0);
		wmemcpy(prefix,L"[F] ",5);
		break;
	case WARN:
		cf.crTextColor = RGB(255,140,0);
		wmemcpy(prefix,L"[W] ",5);
		break;
	default:
		cf.dwEffects = CFE_AUTOCOLOR;
		wmemcpy(prefix,L"[I] ",5);
	}
	SendMessage(hEdit,EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);

	// Print one Line

	// Prefix
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)prefix);
	// Code
	swprintf(prtcode, 6,L"%d: ", Code);
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)prtcode);
	// Data
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)(LPCWSTR)Data);
	// New line
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)L"\r\n");

	// Scroll to bottom
	SendMessage(hEdit,EM_SCROLLCARET       , 0, 0);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
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
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// Create an audio object
	// This object holds all the info about the computer audio endpoints (capture only)
	// After initializing, the object can receive quiries and issue notifications regarding
	// endpoints
	g_audio = new CAudioInputW7(hWnd);
	if (!g_audio)
		return FALSE;

	// Open a log window and register a callback function that will called for logging
	hLogDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LOGDLG), hWnd, DlgAudioLog);
	SetWindowPos(hLogDlg, NULL, 100,100,0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
	g_audio->RegisterLog(LogAudioUnit);


	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	LPWSTR id;
	bool dflt;
	double peak;

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
		case IDM_GETPEAK:
			peak = g_audio->GetLoudestDevice((PVOID *)(&id));
			DlgPeakVolume(id, peak);
			break;
		case IDM_LISTCAPTDEVS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_LISTCAPTDEVS), hWnd, DlgListCaptureDevices);
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
		g_audio->~CAudioInputW7();
		PostQuitMessage(0);
		break;
	case WMAPP_DEFDEV_CHANGED:
		g_audio->Enumerate();
		id = (LPWSTR)wParam;
		dflt = g_audio->IsCaptureDeviceDefault(id);
		//peak[0] = g_audio->GetChannelPeak(id,0);
		//peak[1] = g_audio->GetChannelPeak(id,1);
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




void DlgPeakVolume(PVOID id, double peak)
{
	WCHAR * OutStr = new WCHAR[1000];
	WCHAR * DevName = new WCHAR[1000];

	g_audio->GetCaptureDeviceName(id, &DevName);
	swprintf(OutStr,1000,L"Endpoint: %s\nPeak Value %f\nId: %s", DevName, peak, (LPWSTR)id);
	MessageBox(NULL, OutStr, L"Loudest Capture Device",NULL);
}

INT_PTR CALLBACK  DlgAudioLog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

INT_PTR CALLBACK  DlgListCaptureDevices(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hList=NULL;
	switch (message)
	{
	case WM_INITDIALOG:
		hList = GetDlgItem(hDlg, IDC_LIST1);
		InitListViewColumns(hList);
		CaptureDevicesPopulate(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		// Refresh button
		if (LOWORD(wParam) == IDREFRESH)
		{
			g_audio->Enumerate();
			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST1));
			CaptureDevicesPopulate(hDlg);
			return (INT_PTR)TRUE;
		};

		// Select button
		if (LOWORD(wParam) == IDSELECT)
		{
			TCHAR  id[200];
			hList = GetDlgItem(hDlg, IDC_LIST1);
			int sel = ListView_GetSelectionMark(hList);
			if (sel <0)
				return (INT_PTR)FALSE;
			// Get the endpoint Id and open this endpoint for capture
			ListView_GetItemText(hList, sel, 5, id, 200);
			// Start capture endpoint stream by id
			bool stream_started = g_audio->StartStreaming((PVOID)id);
			// Refresh
			g_audio->Enumerate();
			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST1));
			CaptureDevicesPopulate(hDlg);

			return (INT_PTR)stream_started;
		};

		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void CaptureDevicesPopulate(HWND hDlg)
{
	float Value = 0, max = 0;

	HWND hList = GetDlgItem(hDlg, IDC_LIST1);

	// Loop on all devices
	int size;
	LPWSTR id;
	for (int i=0; i<g_audio->CountCaptureDevices(); i++)
	{
		HRESULT hr =g_audio->GetCaptureDeviceId(i, &size, (PVOID *)&id);
		if (FAILED(hr))
			continue;
		AddLine2List(hList, size, id);
	};

}

BOOL InitListViewColumns(HWND hWndListView) 
// InitListViewColumns: Adds columns to a list-view control.
// hWndListView:        Handle to the list-view control. 
// Returns TRUE if successful, and FALSE otherwise. 
{ 
    LVCOLUMN lvc;
    int iCol;

    // Initialize the LVCOLUMN structure.
    // The mask specifies that the format, width, text,
    // and subitem members of the structure are valid.
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    // Add the columns.
    for (iCol = 0; iCol < 7; iCol++)
    {
        lvc.iSubItem = iCol;
        lvc.pszText = L"---";
        lvc.cx = 100;               // Width of column in pixels.
		lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.

		 if ( iCol == 0 )
		 {
			 lvc.pszText = L"Endpoint";
			 lvc.cx = 300; 
		 };

		 if ( iCol == 1 )
		 {
			 lvc.pszText = L"Default";
			 lvc.cx = 50; 
		 };

		 if ( iCol == 2 )
		 {
			 lvc.pszText = L"Active";
			 lvc.cx = 50; 
		 };

		 if ( iCol == 3 )
		 {
			 lvc.pszText = L"Volume(%)";
			 lvc.cx = 80; 
		 };

		 if ( iCol == 4 )
		 {
			 lvc.pszText = L"External";
			 lvc.cx = 50; 
		 };

		 if ( iCol == 5 )
		 {
			 lvc.pszText = L"Ch";
			 lvc.cx = 50; 
		 };
		 if ( iCol == 6 )
		 {
			 lvc.pszText = L"Id";
			 lvc.cx = 180; 
		 };


        // Insert the columns into the list view.
        if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
            return FALSE;
    }
    
    return TRUE;
} 


void AddLine2List(HWND hWndListView, int size, LPWSTR id)
{
	LV_ITEM item;

	item.mask = LVIF_TEXT;
	item.iItem = 1000;
	item.iSubItem = 0;
	//item.pszText = id;
	item.cchTextMax = 260;

	// Get device name from id
	HRESULT hr = g_audio->GetCaptureDeviceName((PVOID) id, &item.pszText);
	int index = ListView_InsertItem(hWndListView, &item);

	// Get device attributes:
	/// Default device?
	if (g_audio->IsCaptureDeviceDefault((PVOID) id))
		ListView_SetItemText(hWndListView, index, 1, L"+")

	/// Active device?
	if (g_audio->IsCaptureDeviceActive((PVOID) id))
	{
		ListView_SetItemText(hWndListView, index, 2, L"+")

		/// Channel peak levels
		TCHAR StrLevels[40];
		double Peak = g_audio->GetDevicePeak(id);
		_stprintf_s(StrLevels,40,L"%.0f", 100*Peak);
		ListView_SetItemText(hWndListView, index, 3, StrLevels);
	}
	else
		ListView_SetItemText(hWndListView, index, 3, L"-");

	// Is physical-external device?
	if (g_audio->IsExternal((PVOID) id))
		ListView_SetItemText(hWndListView, index, 4, L"+")

	// Number of channels
	TCHAR nChStr[3];
	int n = g_audio->GetNumberChannels((PVOID)id);
	_stprintf_s(nChStr,3,L"%0d", n);
	ListView_SetItemText(hWndListView, index, 5,nChStr)

	// Id for later use
	ListView_SetItemText(hWndListView, index, 6, id)

}