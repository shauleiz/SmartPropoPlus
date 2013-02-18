// Audio.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Commctrl.h"
#include <vector>
#include <devicetopology.h>
#include <math.h>
#include <Mmdeviceapi.h>
#include "WinMessages.h"
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>
#include "AudioInputW7.h"
#include "NotificationClient.h"
#include <Richedit.h>
#include <Windowsx.h>
//#include "..\\Include\\PulseScope.h"

#ifdef _DEBUG
#pragma  comment(lib, "..\\lib\\Debug\\PulseScope.lib")
#else
#pragma  comment(lib, "..\\lib\\Release\\PulseScope.lib")
#endif

#define MAX_LOADSTRING 100
class CPulseScope;
extern __declspec(dllimport) void Pulse2Scope(int length, bool low, LPVOID Param);
extern __declspec(dllimport) CPulseScope * InitPulseScope(HWND);
extern __declspec(dllimport)  void DeletePulseScope(CPulseScope * obj);



// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CAudioInputW7 * g_audio;						// Audio interface object (Only one for the moment)
FILE * g_DbgFile = NULL;						// Log file for raw audio
HWND hLogDlg;
HWND hVolDlg = NULL;
CPulseScope * g_PulseScope = NULL;				// Pulse Scope object

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
bool StartLogRawAudio();
void StopLogRawAudio();
bool StartAudioVolMon(HWND hWnd);
void StopAudioVolMon(HWND hWnd);
void LogAudioVolume(int Code, int size, LPVOID Data, LPVOID Param);
inline int CalcVolumeLeft(int value);
inline int CalcVolumeRight(int value);
bool StartPulseLog(HWND hWnd);
void StopPulseLog(HWND hWnd);
bool StartDecoderPPM(HWND hWnd);
void StopDecoderPPM(HWND hWnd);
BOOL IsWindows7OrLater(void);

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

void LogRawAudio(int Code, int size, LPVOID Data, LPVOID Param)
{
	static bool eightbit = true;
	// Audio Unit gets a new packet
	if (ALOG_GETPCK == Code)
	{
		if (size == 8)
			eightbit = true;
		else
			eightbit = false;

		if (!Param)
			return;
		fprintf((FILE *)Param,"\n*********** Packet (%d-bit) **************", size);
		return;
	}

	// Print packet
	if (ALOG_PACK == Code)
	{
		if (!Param)
			return;
		for (int i=0; i<size; i+=2)
		{
			if (Param) // Should be implemented with propper synch
			{
				if (eightbit)
					fprintf((FILE *)Param,"\n[%04d] %03d:%03d", i/2, ((BYTE *)Data)[i], ((BYTE *)Data)[i+1]);
				else
					fprintf((FILE *)Param,"\n[%04d] %06d:%06d", i/2, ((SHORT *)Data)[i], ((SHORT *)Data)[i+1]);
			};
		};
		return;
	};
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
	RECT parent_rect;
	GetWindowRect (hWnd, &parent_rect);
	hLogDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_LOGDLG), hWnd, DlgAudioLog);
	SetWindowLong(hLogDlg, GWL_STYLE, WS_CHILD);
	SetWindowPos(hLogDlg, NULL, parent_rect.left+6,parent_rect.top+50,0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
	ShowWindow(hLogDlg, SW_SHOW); 
	g_audio->RegisterLog(LogAudioUnit, NULL);

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

INT_PTR CALLBACK  DlgAudioVol(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_L), PBM_SETRANGE ,0,0x80000000);
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_R), PBM_SETRANGE ,0,0x80000000);
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_L), PBM_SETSTEP, (WPARAM) 1, 0);
		SendMessage(GetDlgItem(hDlg, IDC_PROGRESS_R), PBM_SETSTEP, (WPARAM) 1, 0);
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
	HWND hList=NULL, hRbL=NULL, hRbR=NULL;
	switch (message)
	{
	case WM_INITDIALOG:
		hList = GetDlgItem(hDlg, IDC_LIST1);
		InitListViewColumns(hList);
		CaptureDevicesPopulate(hDlg);
		hRbL = GetDlgItem(hDlg, IDC_RADIOBUTTON_L);
		hRbR = GetDlgItem(hDlg, IDC_RADIOBUTTON_R);
		Button_SetCheck(hRbL, BST_CHECKED);
		Button_SetCheck(hRbR, BST_UNCHECKED);

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
			ListView_GetItemText(hList, sel, 6, id, 200);

			// Get Left/Right channel
			hRbR = GetDlgItem(hDlg, IDC_RADIOBUTTON_R);
			bool RightChecked = (Button_GetCheck(hRbR) == BST_CHECKED);

			// Start capture endpoint stream by id
			bool stream_started = g_audio->StartStreaming((PVOID)id, RightChecked);
			// Refresh
			g_audio->Enumerate();
			ListView_DeleteAllItems(GetDlgItem(hDlg, IDC_LIST1));
			CaptureDevicesPopulate(hDlg);

			return (INT_PTR)stream_started;
		};

		// Start/stop logging raw audio
		if (LOWORD(wParam) == IDC_LOGAUDIO)
		{
			LRESULT check = Button_GetCheck(GetDlgItem(hDlg, IDC_LOGAUDIO));
			if (BST_CHECKED == check)
			{
				Button_Enable(GetDlgItem(hDlg, IDC_AUDIOVOL), false);
				StartLogRawAudio();
			}
			else
			{
				Button_Enable(GetDlgItem(hDlg, IDC_AUDIOVOL), true);
				StopLogRawAudio();
			};
			return (INT_PTR)TRUE;;
		};

		// Start/stop display audio volume
		if (LOWORD(wParam) == IDC_AUDIOVOL)
		{
			LRESULT check = Button_GetCheck(GetDlgItem(hDlg, IDC_AUDIOVOL));
			if (BST_CHECKED == check)
			{
				Button_Enable(GetDlgItem(hDlg, IDC_LOGAUDIO), false);				
				StartAudioVolMon(hDlg);
			}
			else
			{
				Button_Enable(GetDlgItem(hDlg, IDC_LOGAUDIO), true);				
				StopAudioVolMon(hDlg);
			};
			return (INT_PTR)TRUE;;
		};

		// Start/Stop Logging pulses (to Scope)
		if (LOWORD(wParam) == IDC_LOGPULSE)
		{
			LRESULT check = Button_GetCheck(GetDlgItem(hDlg, IDC_LOGPULSE));
			if (BST_CHECKED == check)
			{
				StartPulseLog(hDlg);
			}
			else
			{
				StopPulseLog(hDlg);
			};
			return (INT_PTR)TRUE;;
		};

		// Start/Stop PPM Decoder
		if (LOWORD(wParam) == IDC_DECPPM)
		{
			LRESULT check = Button_GetCheck(GetDlgItem(hDlg, IDC_DECPPM));
			if (BST_CHECKED == check)
			{
				StartDecoderPPM(hDlg);
			}
			else
			{
				StopDecoderPPM(hDlg);
			};
			return (INT_PTR)TRUE;;
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
		ListView_SetItemText(hWndListView, index, 4, L"+");

	// Get Jack info (Debug only)
	KSJACK_DESCRIPTION  JackDesc;
	hr = g_audio->GetJackInfo((PVOID) id, &JackDesc);
	COLORREF color = g_audio->GetJackColor((PVOID) id);

	// Number of channels
	TCHAR nChStr[3];
	int n = g_audio->GetNumberChannels((PVOID)id);
	_stprintf_s(nChStr,3,L"%0d", n);
	ListView_SetItemText(hWndListView, index, 5,nChStr)

	// Id for later use
	ListView_SetItemText(hWndListView, index, 6, id)

}
bool StartLogRawAudio()
{
	bool res = true;
	//fopen_s(&g_DbgFile,"audio.txt","a+");
	g_DbgFile = fopen("audio.txt","w+");
	if (g_DbgFile)
	{
		fprintf(g_DbgFile,"\n\n+++++++++++ Starting Log +++++++++++");
		g_audio->RegisterAudioLog(LogRawAudio, (PVOID)g_DbgFile);
	}
	else
		res = false;

	return res;
}
void StopLogRawAudio()
{
	g_audio->RegisterAudioLog(NULL);
	Sleep(1000);
	if (g_DbgFile)
	{
		fprintf(g_DbgFile,"\n\n+++++++++++ Stopping Log +++++++++++");
		fclose(g_DbgFile);
	};
	g_DbgFile = NULL;
}

bool StartAudioVolMon(HWND hWnd)
{
	// Open a audio monitoring window
	hVolDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIAG_VOL), hWnd, DlgAudioVol);
	SetWindowPos(hVolDlg, NULL, 120,120,0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
	g_audio->RegisterAudioLog(LogAudioVolume, (PVOID)hVolDlg);
	return true;
}
void StopAudioVolMon(HWND hWnd)
{
	g_audio->RegisterAudioLog(NULL);
	SendMessage(hVolDlg,WM_COMMAND    , (WPARAM)IDCANCEL, NULL);
	hVolDlg = NULL;
}


void LogAudioVolume(int Code, int size, LPVOID Data, LPVOID Param)
{
	static bool eightbit = true;
	HWND hVolDlg = (HWND)Param;
	UINT Value_L=0, Value_R=0;



	// Audio Unit gets a new packet
	if (ALOG_GETPCK == Code)
	{
		if (size == 8)
			eightbit = true;
		else
			eightbit = false;
		return;
	}

	// Print packet
	if (ALOG_PACK == Code)
	{
		if (!Param)
			return;

		HWND hL = GetDlgItem(hVolDlg, IDC_PROGRESS_L);
		HWND hR = GetDlgItem(hVolDlg, IDC_PROGRESS_R);

		for (int i=0; i<size; i+=2)
		{
			if (Param) // Should be implemented with propper synch
			{	// Calculate Values
				if (eightbit)
				{
					Value_L = CalcVolumeLeft (128*((BYTE *)Data)[i]);
					Value_R = CalcVolumeRight(128*((BYTE *)Data)[i+1]);
				}
				else
				{
					Value_L = CalcVolumeLeft(1024+(((SHORT *)Data)[i])/64);
					Value_R = CalcVolumeLeft(1024+(((SHORT *)Data)[i+1])/64);
				}

			};

		};

		// Put data on progress bar
		SendMessage(hL, PBM_SETPOS  , (Value_L),0);
		SendMessage(hR, PBM_SETPOS  , (Value_R),0);

		return;
	};
}

inline int CalcVolumeLeft(int value)
{
	// Based on RCAudio V 3.0 : (C) Philippe G.De Coninck 2007
	static int volume;
	static int aud_max_val, aud_min_val;
	int delta_max = abs(value - aud_max_val);
	int delta_min = abs(value - aud_min_val);

	if (delta_max > delta_min) aud_min_val = (4*aud_min_val + value)/5;
	else aud_max_val = (4*aud_max_val + value)/5;

	if (aud_max_val < aud_min_val + 2) {
		aud_max_val = aud_min_val + 1;
		aud_min_val = aud_min_val - 1;
	}

	int threshold = (aud_max_val + aud_min_val)/2;
	volume = (99*volume + abs((int)(value - threshold)))/100;
	return volume;
}

inline int CalcVolumeRight(int value)
{
	// Based on RCAudio V 3.0 : (C) Philippe G.De Coninck 2007
	static int volume;
	static double aud_max_val, aud_min_val;
	double delta_max = fabs(value - aud_max_val);
	double delta_min = fabs(value - aud_min_val);

	if (delta_max > delta_min) aud_min_val = (4*aud_min_val + value)/5;
	else aud_max_val = (4*aud_max_val + value)/5;

	if (aud_max_val < aud_min_val + 2) {
		aud_max_val = aud_min_val + 1;
		aud_min_val = aud_min_val - 1;
	}

	double threshold = (aud_max_val + aud_min_val)/2;
	volume = (99*volume + abs((int)(value - threshold)))/100;
	return volume;
}
// Process Pulse callback function - stub for oscilloscope
//void ProcessPulse2Scope(int length, bool low, LPVOID Param)
//{
//	/*
//	This function is registerred as a ProcessPulse function periodically called by ACU
//	It collects the pulse data and syncs on the longest pulse as trigger
//	It then creates a buffer of sampled pulse data.
//	The buffer is refreshed periodically
//	This buffer is fed into the Oscilloscope (http://www.codeproject.com/Articles/43426/A-Beautiful-Oscilloscope-Based-on-DirectX)
//	*/
//
//	// For a specified time-window - Find the longest pulse
//	// This period will serve as scope-trigger
//#define TIMEWINDOW 500
//	static DWORD window_begin=0, window_end=0; 
//	window_end = timeGetTime();
//	if ((window_end - window_begin)>TIMEWINDOW)
//		window_begin+=10;
//
//	static int longist=0;
//	static DWORD longist_time=0;
//
//
//}


bool StartPulseLog(HWND hWnd)
{
	// Open a audio monitoring window
	//hVolDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIAG_VOL), hWnd, DlgAudioVol);
	//SetWindowPos(hVolDlg, NULL, 120,120,0,0, SWP_NOSIZE | SWP_SHOWWINDOW);
	//CStub * scope = new CStub;
	g_PulseScope = InitPulseScope(hWnd);
	g_audio->RegisterProcessPulse((LPVOID)(Pulse2Scope), (PVOID)g_PulseScope);
	return true;
}
void StopPulseLog(HWND hWnd)
{
	g_audio->RegisterProcessPulse(NULL);
	DeletePulseScope(g_PulseScope);
}

bool StartDecoderPPM(HWND hWnd)
{
	return false;
}

void StopDecoderPPM(HWND hWnd)
{
	return;
}

