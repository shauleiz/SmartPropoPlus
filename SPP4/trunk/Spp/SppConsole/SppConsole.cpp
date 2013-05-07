// SppConsole.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Shellapi.h>
#include "vJoyInterface.h"
#include "WinMessages.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "..\SppAudio\AudioInputW7.h"
#include "..\SppMain\SppMain.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"

// Globals
HWND hDialog;
class CAudioInputW7 * Audio;
LPCTSTR AudioId = NULL;
class CSppMain * Spp = NULL;

// Declarations
void		CaptureDevicesPopulate(HWND hDlg);
HINSTANCE	FilterPopulate(HWND hDlg);
void		Acquire_vJoy();

LRESULT CALLBACK MainWindowProc(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
);


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	HANDLE hDlgCLosed=NULL;

	// TODO: Ensure Vista SP2 or higher

	// Read Command line
	// TODO: This is only an example of how to parse the command line
	int argc = 0;
	LPWSTR* argv = NULL;
	argv = CommandLineToArgvW(lpCmdLine, &argc);
	LocalFree(argv);
	// Read Command line (End)

	//Registers a system-wide messages:
	// WM_INTERSPPCONSOLE - to ensure that it is a singleton.
	// WM_INTERSPPAPPS - Messages to the user interface window.
	TCHAR msg[MAX_MSG_SIZE];
	WM_INTERSPPCONSOLE	= RegisterWindowMessage(INTERSPPCONSOLE);
	if (!WM_INTERSPPCONSOLE)
	{
		_stprintf_s(msg, MAX_MSG_SIZE, CN_NO_INTERSPPCONSOLE , WM_INTERSPPCONSOLE);
		::MessageBox(NULL,msg, SPP_MSG , MB_SYSTEMMODAL);
		return -1;
	};
	WM_INTERSPPAPPS		= RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{
		_stprintf_s(msg, MAX_MSG_SIZE, CN_NO_INTERSPPAPPS, WM_INTERSPPAPPS);
		::MessageBox(NULL,msg, SPP_MSG , MB_SYSTEMMODAL);
		return -1;
	};

		
	/* Test if another SPPConsole is running */
	HANDLE hMuxex;
	if (hMuxex=OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXCONSOLE))
	{	// another instance is already running and the second instance is NOT 
		// launched iconified then Broadcast a message.
		//TODO: if (!m_Iconified)
			::PostMessage(HWND_BROADCAST, WM_INTERSPPCONSOLE, 0, 0);
		return -2;
	}
	else
		hMuxex = CreateMutex(NULL, FALSE, MUTXCONSOLE);

	// Create main window that will receive messages from other parts of the application
	// and will relay the data to the dialog window.
	// This window will be visible only in Debug mode
	WNDCLASSEX WndClsEx;
	WndClsEx.cbSize = sizeof(WNDCLASSEX);
	WndClsEx.style = NULL;
	WndClsEx.lpfnWndProc = MainWindowProc;
	WndClsEx.cbClsExtra = 0;
	WndClsEx.cbWndExtra = 0;
	WndClsEx.hInstance = hInstance;
	WndClsEx.hIcon = NULL;
	WndClsEx.hCursor = NULL;
	WndClsEx.hbrBackground = (HBRUSH)COLOR_WINDOW;
	WndClsEx.lpszMenuName = NULL;
	WndClsEx.lpszClassName = MAIN_CLASS_NAME;
	WndClsEx.hIconSm = NULL;
	ATOM WndClassMain = RegisterClassEx(&WndClsEx);
	if (!WndClassMain)
		return false;

	HWND hwnd = CreateWindow( 
        MAIN_CLASS_NAME,        // name of window class 
        MAIN_WND_TITLE,         // title-bar string 
        WS_OVERLAPPEDWINDOW, // top-level window 
        CW_USEDEFAULT,       // default horizontal position 
        CW_USEDEFAULT,       // default vertical position 
        CW_USEDEFAULT,       // default width 
        CW_USEDEFAULT,       // default height 
        (HWND) NULL,         // no owner window 
        (HMENU) NULL,        // use class menu 
        hInstance,           // handle to application instance 
        (LPVOID) NULL);      // no window-creation data 
	DWORD err = GetLastError();
	if (!hwnd)
		return false;

	// Start the audio 
	Audio = new CAudioInputW7(hwnd);
	if (!Audio)
		return false;


	// Create Dialog box, initialize it then show it
	SppConsoleDlg *	Dialog	= new SppConsoleDlg(hInstance, hwnd);
	hDialog = Dialog->GetHandle();
	CaptureDevicesPopulate(hDialog);
	
	// Start reading audio data
	Spp		= new CSppMain();
	Spp->SetAudioObj(Audio);

	if (!Spp->Start(hwnd))
		goto ExitApp;

	Dialog->Show(); // If not asked to be iconified

	FilterPopulate(hDialog);
	Spp->AudioChanged();

	// Acquire vJoy device (number 1)
	Acquire_vJoy();

	// Loop forever in the dialog box until user kills it
	Dialog->MsgLoop();

	ExitApp:
	delete(Dialog);

	return 0;
}

/* Window Procedure for the amin (hidden) window*/
LRESULT CALLBACK MainWindowProc(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
  ) {    switch (uMsg) 
    { 
        case WM_CREATE: 
            // Initialize the window. 
            return 0; 
 
        case WM_PAINT: 
            // Paint the window's client area. 
            return 0; 
 
        case WM_SIZE: 
            // Set the size and position of the window. 
            return 0; 
 
        case WM_DESTROY: 
            // Clean up window-specific data objects. 
            return 0; 
 
        // 
        // Process other messages. 
        //
		case WMAPP_DEFDEV_CHANGED:
		case WMAPP_DEV_ADDED:
		case WMAPP_DEV_REM:
			Spp->AudioChanged();
			CaptureDevicesPopulate(hDialog);
			break;

		case WMAPP_DEV_PROPTY:
			Spp->AudioChanged();
			break;

		case GET_ACTIVE_ID:
			return (LRESULT)AudioId;

		case SET_MOD_INFO:
			SendMessage(hDialog, SET_MOD_INFO, wParam, lParam);
			break;

		case MOD_CHANGED:
			if (Spp)
				Spp->SelectMod((LPCTSTR)wParam);
			break;

		case CH_MONITOR:
			if (Spp)
				Spp->MonitorChannels(wParam);
			break;

		case WMAPP_CH_MNTR:
			SendMessage(hDialog, uMsg, wParam, lParam);
			break;
 
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}

// Get all audio capture devices 
// Send their details to dialog box and mark the selected one
void CaptureDevicesPopulate(HWND hDlg)
{
	int size;
	HRESULT hr;
	jack_info jack;
	jack.struct_size = sizeof(jack_info);

	// Send message: Clear list of capture devices
	SendMessage(hDlg, REM_ALL_JACK,0, 0);
	AudioId = NULL;

	for (int i=1; i<=Audio->CountCaptureDevices(); i++)
	{
		hr =Audio->GetCaptureDeviceId(i, &size, (PVOID *)&jack.id);
		if (FAILED(hr))
			continue;

		//// Display only active devices
		//if (!Audio->IsCaptureDeviceActive((PVOID)jack.id))
		//	continue;

		// Display physical devices
		if (!Audio->IsExternal((PVOID) jack.id))
			continue;

		// Get device name from id
		hr = Audio->GetCaptureDeviceName((PVOID) jack.id, &jack.FriendlyName);
		if (FAILED(hr))
			continue;

		// Get device number of channels
		jack.nChannels = Audio->GetNumberChannels((PVOID)jack.id);

		// Get device jack color
		jack.color = Audio->GetJackColor((PVOID) jack.id);

		// Is device default
		jack.Default = Audio->IsCaptureDeviceDefault((PVOID)jack.id);
		if (jack.Default)
			AudioId = jack.id;

		SendMessage(hDlg, POPULATE_JACKS, (WPARAM)&jack, 0);
	};

}

HINSTANCE FilterPopulate(HWND hDlg)
{
	HINSTANCE h;
	LPCTSTR * names;
	long filter_ver;
	typedef UINT (CALLBACK* LPFNDLLFUNC0)();
	LPFNDLLFUNC0 GetDllVersion;
	int				(CALLBACK *pGetNumberOfFilters)(void);
	const char *    (CALLBACK *pGetFilterNameByIndexA)(const int iFilter);
	const int		(CALLBACK *pGetFilterIdByIndex)(const int iFilter);
	const int		(CALLBACK *pSelectFilterByIndex)(const int iFilter);
	const int		(CALLBACK * pGetIndexOfSelectedFilter)(void);



	// TODO: Recompile DLL without MFC
	// Load the filter DLL file - If does not exist send message to GUI (Call it 4.0.0)
	h = LoadLibraryEx(FILTERDLL_NAME, NULL, 0);
	if (!h)
	{
		SendMessage(hDlg, FILTER_DLL, false, 0);
		return NULL;
	}
	else
		SendMessage(hDlg, FILTER_DLL, true, 0);

	// Verify that the DLL version is not too old
	GetDllVersion = (LPFNDLLFUNC0)GetProcAddress(h,"GetDllVersion");
	if (!GetDllVersion)
	{
		SendMessage(hDlg, FILTER_DLL, false, 0);
		return NULL;
	};
	filter_ver = GetDllVersion();
	if (filter_ver < 0x30100) // TODO: Newer DLLs support wide characters
	{
		SendMessage(hDlg, FILTER_VER, false, filter_ver);
		return NULL;
	};

	//// Build the list in the GUI
	//		Get the number of filters
	int nFilters;
	pGetNumberOfFilters = (int  (CALLBACK *)(void))GetProcAddress(h,"GetNumberOfFilters");
	if (pGetNumberOfFilters)
		nFilters  = pGetNumberOfFilters();
	else
	{
		SendMessage(hDlg, FILTER_NUM, 0, 0);
		return NULL;
	};
	SendMessage(hDlg, FILTER_NUM, nFilters, 0);

	// Get interface functions
	pGetFilterNameByIndexA = (const char *    (CALLBACK *)(const int i))GetProcAddress(h,"GetFilterNameByIndex"); // TODO: Add support to WCHAR
	pGetFilterIdByIndex = (const int   (CALLBACK *)(const int iFilter))GetProcAddress(h,"GetFilterIdByIndex");
	pSelectFilterByIndex = (const int  (CALLBACK *)(const int iFilter))GetProcAddress(h,"SelectFilterByIndex");
	pGetIndexOfSelectedFilter = (const int  (CALLBACK *)(void))GetProcAddress(h,"GetIndexOfSelectedFilter");

	// For every filter send data to GUI
	const char * FilterName;
	int FilterId = -1;
	for (int iFilter=0; iFilter<nFilters ; iFilter++)
	{
		FilterName = pGetFilterNameByIndexA(iFilter);
		SendMessage(hDlg, FILTER_ADDA, iFilter, (LPARAM)FilterName);
	};

	// Update global memory block
	SetNumberOfFilters(nFilters);
		
	/* Get Selected filter from DLL */
	int sel = -1;
	if (pGetIndexOfSelectedFilter)
		sel = pGetIndexOfSelectedFilter();
	if (sel >= 0)
		SetSelectedFilterIndex(sel);
	
	// SetFilterNames(names);

	int iFilterSel = GetSelectedFilterIndex();

	return h;
}


void	Acquire_vJoy()
{
	// vJoy Exists and Enabled
	if (!vJoyEnabled())
	{
		MessageBox(NULL, L"vJoy device does not exist or is disabled",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION);
		return; // vJoy not installed or disabled
	}

	// Version is 2.0.2 or higher
	SHORT ver = GetvJoyVersion();
	if (ver < VJOY_MIN_VER)
	{
		MessageBox(NULL, L"vJoy device version is too old - version 2.0.2 or higher required",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION);
		return; // vJoy not installed or disabled
	}

	// For device number 1 - get status the acquire
	UINT rID = 1;
	VjdStat stat = GetVJDStatus(rID);

	switch (stat)
	{
	case VJD_STAT_OWN: // The  vJoy Device is owned by this application.
		break;

	case VJD_STAT_FREE: // The  vJoy Device is NOT owned by any application (including this one).
		if (!AcquireVJD(rID))
		{
			MessageBox(NULL, L"vJoy device nymber 1 cannot be acquired",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION); 
			return; 
		};
		break;

	case VJD_STAT_BUSY: // The  vJoy Device is owned by another application.
		MessageBox(NULL, L"vJoy device nymber 1 cannot be acquired\r\nThe Device is owned by another application",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION); 
		return; 

	case VJD_STAT_MISS: // The  vJoy Device is missing. It either does not exist or the driver is disabled.
		MessageBox(NULL, L"vJoy device nymber 1 cannot be acquired\r\nThe Device either does not exist or the driver is disabled",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION); 
		return; 

	default:
		MessageBox(NULL, L"vJoy device nymber 1 cannot be acquired for unknown reason",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION);
	}; // Switch

	// Reset device
	ResetVJD(rID);
}
