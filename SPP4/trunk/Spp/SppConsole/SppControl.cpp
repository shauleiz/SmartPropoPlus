// SppControl.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Shellapi.h>
#include "vJoyInterface.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "..\SppAudio\SppAudio.h"
#include "..\SppProcess\SppProcess.h"
#include "SppConfig.h"
#include "SppControl.h"
#include "SppDlg.h"
#include "SppLog.h"
#include "SppDbg.h"
#include "WinMessages.h"
#include "..\vJoyMonitor\vJoyMonitor.h"

// Globals
HWND hDialog;
class CSppConfig * Conf = NULL;
class CSppAudio * Audio = NULL;
class SppLog * LogWin = NULL;
class SppDbg * DbgObj = NULL;
class CvJoyMonitor * vJoyMon = NULL;
LPCTSTR AudioId = NULL;
class CSppProcess * Spp = NULL;
HINSTANCE hDllFilters = 0;
HINSTANCE g_hInstance = 0;
HWND hLog;
bool Monitor = true;


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
void		SetMonitoring(HWND hDlg);


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

	// Get Configuration file
	Conf = new CSppConfig();

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
	DWORD Map = Conf->MapAxis(1); // vJoy Device 1 (TODO: Make it configurable)
	Map = Spp->MappingChanged(Map,8); // load mapping to SppProcess object and get new map  (TODO: Make number of axes configurable)
	Conf->MapAxis(1, Map); // vJoy Device 1 (TODO: Make it configurable)
	SendMessage(hDialog, WMSPP_MAP_UPDT, Map, 8); // TODO: Make number of axes configurable
	Spp->SetAudioObj(Audio);

	if (!Spp->Start(hwnd))
	{
		LogMessage(FATAL, IDS_F_STARTSPPPRS);
		goto ExitApp;
	};
	    

	LogMessage(INFO, IDS_I_STARTSPPPRS);


	FilterPopulate(hDialog);
	Spp->AudioChanged(); // TODO: Remove later
	Dialog->Show(); // If not asked to be iconified

	// Start monitoring thread
	static thread * tMonitor = NULL;
	tMonitor = new thread(thMonitor, &Monitor);

	if (!tMonitor)
			goto ExitApp;

	// Start monitoring channels according to config file
	SetMonitoring(hDialog);

	// Open vJoy monitor
	bool MonitorOk = vJoyMonitorInit(hInstance, hwnd);
	// TODO: Test functions - remove later
	if (MonitorOk)
		StartPollingDevice(0);

	// Loop forever in the dialog box until user kills it
	// TODO: Initialize dialog from registry
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
  ) { 
	  DWORD Map;

	  switch (uMsg) 
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
			CaptureDevicesPopulate(hDialog);
			Spp->AudioChanged();
			break;

		case WMSPP_AUDIO_GETSU:
			{
				UINT br = Conf->GetAudioDeviceBitRate((LPTSTR)wParam);
				wstring ch = Conf->GetAudioDeviceChannel((LPTSTR)wParam);
				if (ch[0] == L'R' || ch[0] == L'r')
					br++;
				return br;
			};
			break;

		case WMSPP_AUDIO_PROP:
			Spp->AudioChanged();
			break;

		case WMSPP_PRCS_GETID:
			return (LRESULT)AudioId;

		case WMSPP_PRCS_SETMOD:
			SendMessage(hDialog, WMSPP_PRCS_SETMOD, wParam, lParam);
			Conf->AddModulation((PVOID)wParam);
			break;

		case WMSPP_PRCS_GETMOD:
			{
				wstring modtype = Conf->GetSelectedModulation();
				LPCTSTR mod = _wcsdup(modtype.c_str());
				return (LRESULT)(mod);
			};
			break;

		case WMSPP_DLG_MOD:
			Conf->SelectModulation((LPTSTR)wParam);
			if (Spp)
				Spp->SelectMod((LPCTSTR)wParam);
			break;

		case WMSPP_DLG_MONITOR:
			if (Spp)
				Spp->MonitorChannels((BOOL)wParam);
			Conf->MonitorChannels((BOOL)wParam);
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

		case WMSPP_JMON_AXIS:
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

		case WMSPP_DLG_MAP:
			Map = Spp->MappingChanged(  (DWORD)wParam, (UINT)lParam);
			Conf->MapAxis(1, Map); // vJoy Device 1 (TODO: Make it configurable)
			SendMessage(hDialog, WMSPP_MAP_UPDT, Map, lParam);
			break;

		case WMSPP_DLG_CHNL:
			if ((UCHAR)wParam !=  Audio->GetwBitsPerSample())
			{
				Audio->SetwBitsPerSample((UCHAR)wParam);
				Spp->AudioChanged();
			};
			Conf->SetDefaultBitRate((UINT)wParam);

			// Set L/R to Spp
			if ((TCHAR)lParam == TEXT('L'))
			{
				Conf->SetDefaultChannel(TEXT("Left"));
				Spp->SetAudioChannel(true);
			}
			else if ((TCHAR)lParam == TEXT('R'))
			{
				Conf->SetDefaultChannel(TEXT("Right"));
				Spp->SetAudioChannel(false);
			}

			break;

		case WMSPP_PRCS_GETLR:
			if (Conf->IsDefaultChannelRight())
				return 1;
			else
				return 0;
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
	if (!Audio)
		return;

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
		if (!jack.nChannels)
			continue;

		// Get device jack color
		jack.color = Audio->GetJackColor((PVOID) jack.id);

		// Is device default
		jack.Default = Audio->IsCaptureDeviceDefault((PVOID)jack.id);
		if (jack.Default)
			AudioId = jack.id;

		// Send data to GUI
		SendMessage(hDlg, POPULATE_JACKS, (WPARAM)&jack, 0);

		// Record data in configuration file
		Conf->AddAudioDevice(jack.id, jack.FriendlyName, jack.Default);

		// For default device, get Bitrate and channel data from configuration file
		// If absent get default
		// Send data to GUI and update  configuration file
		// Update Audio
		if (jack.Default)
		{
			UINT BitRate = Conf->GetAudioDeviceBitRate(jack.id);
			WCHAR * Channel =  _wcsdup(Conf->GetAudioDeviceChannel(jack.id).c_str());
			if (!BitRate)
				BitRate = DEF_BITRATE;
			if (!wcslen(Channel))
				Channel = DEF_CHANNEL;
			SendMessage(hDlg, SET_AUDIO_PARAMS, (WPARAM)BitRate, (LPARAM)(Channel[0]));

			if (Audio)
				Audio->SetwBitsPerSample(BitRate);
		};
	};

}

// Channel monitoring setup from config file and send it to SppProcess
// If data not available then use default value
void SetMonitoring(HWND hDlg)
{
	int monitor = 1; // Default value
	if (Conf)
		monitor = Conf->MonitorChannels();

	BOOL bMonitor = 1;
	if (!monitor)
		bMonitor = 0;

	if (Spp)
		Spp->MonitorChannels(bMonitor);

	SendMessage(hDlg, MONITOR_CH, (WPARAM)bMonitor, 0);
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

	while (*KeepAlive)
	{

		sleep_for( 100 );// Sleep for 100 milliseconds

		// Monitor vJoy (device #1)
		int rID = 1; // TODO: Make the device ID programable
		VjdStat stat = GetVJDStatus(rID);
		if (stat == VJD_STAT_OWN)
			Spp->vJoyReady(true);
		else
			AcquireVJD(rID) ? Spp->vJoyReady(true) :  Spp->vJoyReady(false);


	}
}
