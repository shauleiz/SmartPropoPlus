// SppControl.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Shellapi.h>
#include "vJoyInterface.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "..\SppAudio\SppAudio.h"
#include "..\SppProcess\SppProcess.h"
#include "SppControl.h"
#include "SppDlg.h"
#include "SppLog.h"
#include "SppDbg.h"
#include "WinMessages.h"

// Globals
HWND hDialog;
class CSppAudio * Audio = NULL;
class SppLog * LogWin = NULL;
class SppDbg * DbgObj = NULL;
LPCTSTR AudioId = NULL;
class CSppProcess * Spp = NULL;
HINSTANCE hDllFilters = 0;
HINSTANCE g_hInstance = 0;
HWND hLog;
bool Monitor = true;
std::mutex lg_mutex;

// Declarations
void		CaptureDevicesPopulate(HWND hDlg);
HINSTANCE	FilterPopulate(HWND hDlg);
void		Acquire_vJoy();
void		SelectFilter(int iFilter);
void		LogMessage(int Severity, int Code, LPCTSTR Msg=NULL);
void		LogMessageExt(int Severity, int Code, UINT Src, LPCTSTR Msg);
void		DbgInputSignal(bool start);
void		DbgPulse(bool start);
void		thMonitor(bool * KeepAlive);
void		RestartAudio(void);


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
	LoadLibrary(TEXT("Msftedit.dll")); 
	g_hInstance=hInstance;

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

		
	/* Test if another SppControl is running */
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
	Audio = new CSppAudio(hwnd);
	if (!Audio)
		return false;

	// Crate Debugging object
	DbgObj = new SppDbg();

	// Create Dialog box, initialize it then show it
	SppDlg *	Dialog	= new SppDlg(hInstance, hwnd);
	hDialog = Dialog->GetHandle();
	CaptureDevicesPopulate(hDialog);

	// Create Log window
	LogWin = new SppLog(hInstance, hwnd);
	hLog = LogWin->GetWndHandle();
	
	// Start reading audio data
	Spp		= new CSppProcess();
	Spp->SetAudioObj(Audio);

	if (!Spp->Start(hwnd))
	{
		LogMessage(FATAL, IDS_F_STARTSPPPRS);
		goto ExitApp;
	};
	    

	LogMessage(INFO, IDS_I_STARTSPPPRS);

	Dialog->Show(); // If not asked to be iconified

	FilterPopulate(hDialog);

	// Start monitoring thread
	static thread * tMonitor = NULL;
	tMonitor = new thread(thMonitor, &Monitor);
	if (!tMonitor)
			goto ExitApp;

	// Loop forever in the dialog box until user kills it
	Dialog->MsgLoop();

 
	// Stop monitoring
	Monitor = false;
	if (tMonitor && tMonitor->joinable())
		tMonitor->join();

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
		case WMSPP_AUDIO_CHNG:
		case WMSPP_AUDIO_ADD:
		case WMSPP_AUDIO_REM:
			CaptureDevicesPopulate(hDialog); // Gets the default endpoint & Populates GUI
			RestartAudio();
			break;

		case WMSPP_AUDIO_PROP:
			RestartAudio();
			break;

		case WMSPP_PRCS_GETID:
			return (LRESULT)AudioId;

		case WMSPP_PRCS_SETMOD:
			SendMessage(hDialog, WMSPP_PRCS_SETMOD, wParam, lParam);
			break;

		case WMSPP_DLG_MOD:
			if (Spp)
				Spp->SelectMod((LPCTSTR)wParam);
			break;

		case WMSPP_DLG_MONITOR:
			if (Spp)
				Spp->MonitorChannels((BOOL)wParam);
			break;

		case WMSPP_DLG_LOG:
			if (wParam)
				LogWin->Show();
			else
				LogWin->Hide();
			break;

		case WMSPP_DLG_INSIG:
			if (wParam)
				DbgInputSignal(true);
			else
				DbgInputSignal(false);
			break;

		case WMSPP_DLG_PULSE:
			if (wParam)
				DbgPulse(true);
			else
				DbgPulse(false);
			break;

		case WMSPP_PRCS_RCHMNT:
		case WMSPP_PRCS_PCHMNT:
			SendMessage(hDialog, uMsg, wParam, lParam);
			break;

		case WMSPP_DLG_FILTER:
			SelectFilter((int)wParam);
			break;

		case WMSPP_PRCS_GETSPR:
			return Audio->GetnSamplesPerSec();

		case WMSPP_PRCS_GETBPS:
			return Audio->GetwBitsPerSample();

		case WMSPP_PRCS_GETNCH:
			return Audio->GetNumberChannels();

		case WMSPP_LOG_PRSC+INFO:
		case WMSPP_LOG_PRSC+WARN:
		case WMSPP_LOG_PRSC+ERR:
		case WMSPP_LOG_PRSC+FATAL:
			LogMessageExt(uMsg-WMSPP_LOG_PRSC, (int)wParam, WMSPP_LOG_PRSC, (LPCTSTR)lParam);
			break;
 
		case WMSPP_LOG_AUDIO+INFO:
		case WMSPP_LOG_AUDIO+WARN:
		case WMSPP_LOG_AUDIO+ERR:
		case WMSPP_LOG_AUDIO+FATAL:
			LogMessageExt(uMsg-WMSPP_LOG_AUDIO, (int)wParam, WMSPP_LOG_AUDIO, (LPCTSTR)lParam);
			break;
 
		case WMSPP_AUDIO_INSIG:
			DbgObj->InputSignalReady((PBYTE)wParam, (PVOID)lParam);
			break;
 
		case WMSPP_PROC_PULSE:
			DbgObj->PulseReady((PVOID)wParam, (UINT)lParam);
			break;
		//case WMSPP_PRCS_GETLR:
		//	return Audio->Get;
 
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
	//LPCTSTR * names;
	//long filter_ver;
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
		LogMessage(WARN, IDS_W_FILTERDLL);
		SendMessage(hDlg, FILTER_DLL, false, 0);
		return NULL;
	}
	else
	{
		LogMessage(INFO, IDS_I_FILTERDLL);
		SendMessage(hDlg, FILTER_DLL, true, 0);
	}

	// Assign global handle to Filters' DLL
	hDllFilters = h;

	// Verify that the DLL version is not too old
	GetDllVersion = (LPFNDLLFUNC0)GetProcAddress(h,"GetDllVersion");
	if (!GetDllVersion)
	{
		SendMessage(hDlg, FILTER_DLL, false, 0);
		LogMessage(ERR, IDS_E_FILTERDLLVER);
		FreeLibrary(h);
		hDllFilters = NULL;
		return NULL;
	};

	//filter_ver = GetDllVersion();
	//if (filter_ver < 0x30100) // TODO: Newer DLLs support wide characters
	//{
	//	SendMessage(hDlg, FILTER_VER, false, filter_ver);
	//	return NULL;
	//};

	//// Build the list in the GUI
	//		Get the number of filters
	int nFilters;
	pGetNumberOfFilters = (int  (CALLBACK *)(void))GetProcAddress(h,"GetNumberOfFilters");
	if (pGetNumberOfFilters)
		nFilters  = pGetNumberOfFilters();
	else
	{
		SendMessage(hDlg, FILTER_NUM, 0, 0);
		LogMessage(ERR, IDS_E_FILTERDLLNF);
		FreeLibrary(h);
		hDllFilters = NULL;
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

	// 

	// Update global memory block
	//SetNumberOfFilters(nFilters);
		
	/* Get Selected filter from DLL */
	//int sel = -1;
	//if (pGetIndexOfSelectedFilter)
	//	sel = pGetIndexOfSelectedFilter();
	//if (sel >= 0)
	//	SetSelectedFilterIndex(sel);
	
	LogMessage(INFO, IDS_I_FILTERDLLOK);
	return h;
}

void		SelectFilter(int iFilter)
{
	const int  (WINAPI * pSelectFilterByIndex)(const int iFilter);
	PJS_CHANNELS (WINAPI * pProcessChannels)(PJS_CHANNELS, int max, int min);

	if (!hDllFilters)
	{
		LogMessage(WARN, IDS_W_FILTERSELFAIL);
		return;
	}

	// Update the DLL which is the selected filter
	pSelectFilterByIndex = (const int  (WINAPI *)(const int iFilter))GetProcAddress(hDllFilters,"SelectFilterByIndex");
	if (pSelectFilterByIndex)
		pSelectFilterByIndex(iFilter);
	else
	{
		LogMessage(ERR, IDS_E_FILTERSELFAIL);
		return;
	};

	// Get the pointer to the filter function
	pProcessChannels = (PJS_CHANNELS (WINAPI * )(PJS_CHANNELS, int max, int min))GetProcAddress(hDllFilters,"ProcessChannels");

	Spp->SelectFilter(iFilter, (LPVOID)pProcessChannels);
	LogMessage(INFO, IDS_I_FILTERSELOK);
}

void	Acquire_vJoy()
{
	// vJoy Exists and Enabled
	if (!vJoyEnabled())
	{
		LogMessage(WARN, IDS_W_VJOYMISSING);
		MessageBox(NULL, L"vJoy device does not exist or is disabled",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION);
		return; // vJoy not installed or disabled
	}

	// Version is 2.0.2 or higher
	SHORT ver = GetvJoyVersion();
	if (ver < VJOY_MIN_VER)
	{
		LogMessage(ERR, IDS_E_VJOYVERTOOOLD);
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
			MessageBox(NULL, L"vJoy device number 1 cannot be acquired",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION); 
			return; 
		};
		break;

	case VJD_STAT_BUSY: // The  vJoy Device is owned by another application.
		MessageBox(NULL, L"vJoy device number 1 cannot be acquired\r\nThe Device is owned by another application",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION); 
		return; 

	case VJD_STAT_MISS: // The  vJoy Device is missing. It either does not exist or the driver is disabled.
		MessageBox(NULL, L"vJoy device number 1 cannot be acquired\r\nThe Device either does not exist or the driver is disabled",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION); 
		return; 

	default:
		MessageBox(NULL, L"vJoy device number 1 cannot be acquired for unknown reason",L"SPP: vJoy error", MB_OK|MB_ICONEXCLAMATION);
	}; // Switch

	// Reset device
	ResetVJD(rID);
}
void	LogMessage(int Severity, int Code, LPCTSTR Msg)
{
	if (!hLog)
		return;

	TCHAR pBuf[1000] = {NULL};
	int len;

	if (!Msg)
	{
		len = LoadString(g_hInstance, Code, reinterpret_cast< LPWSTR >( &pBuf ), sizeof(pBuf)/sizeof(TCHAR) );
		if (len)
			Msg = pBuf;
	};

	SendMessage(hLog , WMSPP_LOG_CNTRL + Severity, (WPARAM)Code, (LPARAM)Msg);
}

void	LogMessageExt(int Severity, int Code, UINT Src, LPCTSTR Msg)
{
	if (!hLog)
		return;

	TCHAR pBuf[1000] = {NULL};
	int len;

	if (!Msg)
	{
		len = LoadString(g_hInstance, Code, reinterpret_cast< LPWSTR >( &pBuf ), sizeof(pBuf)/sizeof(TCHAR) );
		if (len)
			Msg = pBuf;
	};

	SendMessage(hLog , Src + Severity, (WPARAM)Code, (LPARAM)Msg);
}

// Start/Stop debugging the raw input data (digitized audio)
void		DbgInputSignal(bool start)
{
	if (!DbgObj || !Audio)
		return;

	if (start)
	{
		DbgObj->StartDbgInputSignal();
		Audio->StartDbgInputSignal();
	}
	else
	{
		Audio->StopDbgInputSignal();
		DbgObj->StopDbgInputSignal();
	}
}
// Start/Stop debugging the raw input data (digitized audio)
void		DbgPulse(bool start)
{
	if (!DbgObj || !Audio)
		return;

	if (start)
	{
		DbgObj->StartDbgPulse();
		Spp->StartDbgPulse();
	}
	else
	{
		Spp->StopDbgPulse();
		DbgObj->StopDbgPulse();
	}
}


// Monitor Thread
// Polls activity of the different modules
// Reports health of the modules and attemps recovery when possible
void thMonitor(bool * KeepAlive)
{
	bool stopped = false;
	RestartAudio();

	while (*KeepAlive)
	{

		// Monitor vJoy (device #1)
		int rID = 1; // TODO: Make the device ID programable
		VjdStat stat = GetVJDStatus(rID);
		if (stat == VJD_STAT_OWN)
			Spp->vJoyReady(true);
		else
			AcquireVJD(rID) ? Spp->vJoyReady(true) :  Spp->vJoyReady(false);
#if 0

		// Change in the audio configuration was detected
		// Ask audio unit to make the needed changes
		// Then, reset flag
		if (ChangeAudio && !stopped)
		{
			// Stop Capturing thread
			// ok if stopped (Or does not exist)
			stopped = Spp->StopCaptureThread();
			break;
		};

		if (ChangeAudio && stopped)
		{
			// Start Capturing thread
			// ok if started
			stopped = Spp->StartCaptureThread();
			ChangeAudio = stopped;
		}; // Stopped

#endif

		sleep_for( 100 );// Sleep for 100 milliseconds

	}
}

// Call this function when change occured.
// It will try to restart the audio streaming
void		RestartAudio(void)
{
	int i = 0;

	std::unique_lock<std::mutex> lockRA(lg_mutex, std::defer_lock );
	bool locked = lockRA.try_lock();
	if (!locked)
		return;

	sleep_for(100);


	LogMessage(INFO, IDS_I_RESTAUDIO);
	
	// Testing
	if (!Audio)
	{
		LogMessage(ERR, IDS_E_RESTAUDIO_NOAUDIO);
		lockRA.unlock();
		return;
	};

	if (!Spp)
	{
		LogMessage(ERR, IDS_E_RESTAUDIO_NOSPP);
		lockRA.unlock();
		return;
	};

	if (!AudioId)
	{
		LogMessage(ERR, IDS_E_RESTAUDIO_NOID);
		lockRA.unlock();
		return;
	};

	// Request SppProcess to stop capture thread
	// This function blocks until thread killed
	Spp->StopCaptureThread();

	// (Re)start streaming
	if (!Audio->StartStreaming((PVOID)AudioId))
	{
		LogMessage(ERR, IDS_E_RESTAUDIO_SSFAIL);
		lockRA.unlock();
		return;
	};

	// Request SppProcess to start capture thread
	if (Spp->StartCaptureThread())
		LogMessage(INFO, IDS_I_RESTAUDIO_OK);
	else
		LogMessage(ERR, IDS_E_RESTAUDIO_FAIL);


	lockRA.unlock();
	return;
}
