// SppControl.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <VersionHelpers.h>
#include <crtdbg.h>
#include <Shellapi.h>
#include <condition_variable>
#include "public.h"
#include "vJoyInterface.h"
#include "SmartPropoPlus.h"
#include "SppAudio.h"
#include "SppProcess.h"
#include "SppConfig.h"
#include "SppControl.h"
#include "SppDlg.h"
#include "SppLog.h"					
#include "SppDbg.h"
#include "WinMessages.h"
#include "..\vJoyMonitor\vJoyMonitor.h"
//#include <vld.h>

#pragma region Globals

// Globals
HWND hMain = NULL;
HWND hDialog;
class CSppConfig * Conf = NULL;
class CSppAudio * Audio = NULL;
class SppLog * LogWin = NULL;
class SppDbg * DbgObj = NULL;
class SppDlg *	Dialog = NULL;
class CSppProcess * Spp = NULL;
LPCTSTR AudioId = NULL;
HINSTANCE hDllFilters = 0;
HINSTANCE g_hInstance = 0;
HWND hLog;
bool Monitor = true;
thread * tMonitor = NULL;
thread * tDialogBox = NULL;
int     vJoyDevice = 1;
bool    vJoyDeviceEnabled = false;
bool reqPopulateFilter = false;
UINT AudioLevel[2] = {0, 0};
int isRight=-1; // -1: Uninitialized
int BitRate=-1; // -1: Uninitialized
bool AutoBitRate;
bool AutoChannel;
OperatState OperatStateMachine = UNKNOWN;
WORD Flags = FLG_NONE;
bool ReqStartCapture=false;
bool ReqStopCapture=false;
bool StartCapture=false;
bool StopCapture=false;
bool dialogbox_is_ready(false);
std::mutex mtx_dialogbox;
std::condition_variable cv_dialogbox;


#pragma endregion Globals

#pragma region Declarations
// Declarations
void		CaptureDevicesPopulate(HWND hDlg);
HINSTANCE	FilterPopulate(HWND hDlg);
void		Acquire_vJoy();
void		SelectFilter(int iFilter);
LPVOID		SelectFilterFile(LPCTSTR FilterPath);
DWORD		GetFilterFileVersion(LPTSTR FilterPath);
void		LogMessage(int Severity, int Code, LPCTSTR Msg=NULL);
void		LogMessageExt(int Severity, int Code, UINT Src, LPCTSTR Msg);
void		DbgInputSignal(bool start);
void		DbgPulse(bool start);
void		thMonitor(bool * KeepAlive);
void		thDialogBox(HWND hwnd);
void		SetMonitoring(HWND hDlg);
void		SetPulseScope(HWND hDlg);
int			vJoyDevicesPopulate(HWND hDlg);
UINT		vJoyDevicesVersion(HWND hDlg);
void		SetvJoyMapping(UINT id);
int			SelectedvJoyDevice(void);
void		SetAvailableControls(UINT id, HWND hDlg);
void		SetThreadName(char* threadName);
bool		isAboveVistaSp1();
void		AudioLevelWatch();
UINT		GetAudioQuality(bool);
void		PulseScope(BOOL start);
WORD		GetStartMode(LPTSTR lpCmdLine);
void		SetNotificationIcon(LPCTSTR);
void		ComputeOperatState(void);
void		DecoderAuto(HWND hDlg);
OperatState SetState(OperatState current, OperatState next, LPCTSTR Msg = NULL);
void		MessageLoop(void);
LRESULT		CALLBACK MainWindowProc(_In_  HWND hwnd, _In_  UINT uMsg, _In_  WPARAM wParam, _In_  LPARAM lParam);
bool		AppExit(void);
#pragma endregion Declarations

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	THREAD_NAME("Main Thread");

	//// Memory Leak detection
	////_CrtSetReportMode(_CRT_WARN | _CRT_ERROR,  _CRTDBG_MODE_DEBUG );
	//_CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF );
	//_CrtCheckMemory();


	HANDLE hDlgCLosed=NULL;
	LoadLibrary(TEXT("Msftedit.dll")); 
	g_hInstance=hInstance;

	// Ensure Vista SP2 or higher
	if (!isAboveVistaSp1())
		return -3;

	// Read Command line
	Flags = GetStartMode(lpCmdLine);

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
		_stprintf_s(msg, MAX_MSG_SIZE, CN_NO_INTERSPPAPPS, static_cast<INT>(WM_INTERSPPAPPS));
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

	// Store handle to main window as global
	hMain = hwnd;

	// Get Configuration file
	Conf = new CSppConfig(hMain);

	// If Start mode is 'Normal' - get set up from configulation file
	// Calculate the status of the GUI (Wizard/Iconified)
	if ((Flags & FLG_WIZRD) || Conf->Wizard())
		Flags |= FLG_WIZRD;

	// Get the desired operational target state (Stopped/Work)
	if ((Flags & FLG_STPD) || Conf->Stopped())
		Flags |= FLG_STPD;

	// Start the audio 
	Audio = new CSppAudio(hwnd);
	if (!Audio)
		return false;

	// Crate Debugging object
	DbgObj = new SppDbg();

	// Create Dialog box, initialize it then show it
	// Start Dialog box thread
	tDialogBox = new thread(thDialogBox, hwnd);
	std::unique_lock<std::mutex> lk(mtx_dialogbox);
	if (!tDialogBox)
			goto ExitApp;

	// WAit for dialog box initialization to complete
	while (!dialogbox_is_ready)
		cv_dialogbox.wait(lk);
	lk.unlock();


	hDialog = Dialog->GetHandle();
	CaptureDevicesPopulate(hDialog);

	// Create Log window
	LogWin = new SppLog(hInstance, hwnd);
	hLog = LogWin->GetWndHandle();
	
	// Start reading audio data
	Spp		= new CSppProcess();
	Spp->Init(hwnd);
	//vJoyDevice = Conf->SelectedvJoyDevice();
	vJoyDevice = SelectedvJoyDevice();
	if (vJoyDevice)
		SetvJoyMapping(vJoyDevice);

	// Instruct GUI of the available Controls (Axes/Buttons)
	SetAvailableControls((UINT)vJoyDevice, hDialog);

	Spp->SetAudioObj(Audio);

	// If flag FLG_STPD is set signal the state machine not to start
	// Else start processing audio
	if (!(Flags & FLG_STPD))
	{
		ReqStartCapture = true;
		SendMessage(hDialog, WMSPP_PRCS_ALIVE, (WPARAM)1, 0);

	}
	else
	{
		ReqStopCapture = true;
		SendMessage(hDialog, WMSPP_PRCS_ALIVE, (WPARAM)0, 0);
	};

	// Start monitoring channels according to config file
	SetMonitoring(hDialog);

	// Start Pulse Scope according to config file
	SetPulseScope(hDialog);

	// Start searching for correct decoder according to config file
	DecoderAuto(hDialog);

	// Open vJoy monitor
	bool MonitorOk = vJoyMonitorInit(hInstance, hwnd);

	// Populate the GUI with the avaiable vJoy Devices - return the number of devices
	int nvJoyDev = vJoyDevicesPopulate(hDialog);

	// Display the version of vJoy driver
	vJoyDevicesVersion(hDialog);

	// TODO: Test functions - remove later
	if (MonitorOk)
		StartPollingDevice(vJoyDevice); 

	// Start monitoring thread
	tMonitor = new thread(thMonitor, &Monitor);
	if (!tMonitor)
			goto ExitApp;

	// Send notification icon status data
	if (Flags & FLG_WIZRD) 
		Dialog->Show();
	else
		Dialog->Hide();

	// Loop forever in the dialog box until user kills it
	MessageLoop();
	return 0;
 
ExitApp:

	AppExit();
	return 0;
}

/* Window Procedure for the amin (hidden) window*/
LRESULT CALLBACK MainWindowProc(
  _In_  HWND hwnd,
  _In_  UINT uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
  ) { 

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
			hwnd = NULL;
			PostQuitMessage(0);
			return (INT_PTR)TRUE;

		case WM_DEVICECHANGE:
			return TRUE;

 
        ////////////////////////////////////////// 
        // Process other messages. 
        //

        ////////////////////////////////////////// 
		// Called by:	CSppAudio
		// Meaning:		Audio configuation changed
		// Action:		Enumerate new audio configuration and inform GUI and CSppProcess
		case WMSPP_AUDIO_CHNG:
		case WMSPP_AUDIO_ADD:
		case WMSPP_AUDIO_REM:
			CaptureDevicesPopulate(hDialog);
			Spp->AudioChanged();
			break;

        ////////////////////////////////////////// 
		// Called by:	CSppAudio::GetDefaultSetUp()
		// Meaning:		Get audio configuration from configuration file
		// wParam:		Audio Device ID
		// Action:		Get bitrate and audio channel from configuration file
		// Return:		bitrate & Channel (Muxed)
		case WMSPP_AUDIO_GETSU:
			{
				UINT br = Conf->GetAudioDeviceBitRate((LPTSTR)wParam);
				if (!br)
					br = DEF_BITRATE;
				wstring ch = Conf->GetAudioDeviceChannel((LPTSTR)wParam);
				if (ch[0] == L'R' || ch[0] == L'r')
					br++;
				return br;
			};
			break;

        ////////////////////////////////////////// 
		// Called by:	CSppAudio::PropertyValueChanged()
		// Meaning:		Some audio property has changed
		// Action:		Pass event to CSppProcess
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

		case WMSPP_PRCS_DCDR:
			if (wParam && !lParam)
			{
				Conf->SelectModulation((LPTSTR)wParam);
				if (Spp)
					Spp->SelectMod((LPCTSTR)wParam);
			};
			SendMessage(hDialog, uMsg, wParam, lParam);

			break;


		case WMSPP_DLG_MONITOR:
			if (Spp)
				Spp->MonitorChannels((BOOL)wParam);
			Conf->MonitorChannels(wParam !=0); // Silly cast to bool to evoid warning
			break;

		case WMSPP_DLG_PLSSCOP:
			PulseScope((BOOL)wParam);
			Conf->PulseScope(wParam !=0); // Silly cast to bool to evoid warning
			SetPulseScope(hDialog);
			break;

		case WMSPP_DLG_LOG:
			if (wParam)
				LogWin->Show();
			else
				LogWin->Hide();
			break;

		//case WMSPP_DLG_RST:
		//	break;


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

		case WMSPP_DLG_VJOYSEL:
			vJoyDevice = (int)wParam;
			StartPollingDevice(vJoyDevice);
			Conf->SelectvJoyDevice((UINT)wParam);
			SetvJoyMapping((UINT)wParam);
			SetAvailableControls((UINT)vJoyDevice, hDialog);// Instruct GUI of the available Controls (Axes/Buttons)
			break;

  		case WMSPP_JMON_STRT:
			// Populate the GUI with the avaiable vJoy Devices - return the number of devices
			vJoyDevicesPopulate(hDialog);
			vJoyDevicesVersion(hDialog);
			vJoyDevice = Conf->SelectedvJoyDevice();
			SetvJoyMapping(vJoyDevice);
			SetAvailableControls((UINT)vJoyDevice, hDialog);// Instruct GUI of the available Controls (Axes/Buttons)
			break;


		case WMSPP_JMON_STP:
		case WMSPP_JMON_BTN:
		case WMSPP_PRCS_RCHMNT:
		case WMSPP_PRCS_NRCHMNT:
		case WMSPP_PRCS_NPCHMNT:
		case WMSPP_PRCS_PCHMNT:
		//case WMSPP_PRCS_ALIVE:
		case WMSPP_JMON_AXIS:
		SendMessage(hDialog, uMsg, wParam, lParam);
			break;

		case WMSPP_PRCS_ALIVE:
			if (wParam)
			{
				StopCapture = false;
				StartCapture = true;
			}
			else
			{
				StopCapture = true;
				StartCapture = false;
			}
			break;

		case WMSPP_DLG_FILTER:
			SelectFilter((int)wParam);
			break;

		case WMSPP_DLG_FLTRFILE:
			return (LRESULT)SelectFilterFile((LPCTSTR)wParam);

		case WMSPP_DLG_FDLL:
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
 
		case WMSPP_LOG_AUDIO + INFO:
		case WMSPP_LOG_AUDIO + WARN:
		case WMSPP_LOG_AUDIO + ERR:
		case WMSPP_LOG_AUDIO + FATAL:
			LogMessageExt(uMsg - WMSPP_LOG_AUDIO, (int)wParam, WMSPP_LOG_AUDIO, (LPCTSTR)lParam);
			break;

		case WMSPP_LOG_CONFIG + INFO:
		case WMSPP_LOG_CONFIG + WARN:
		case WMSPP_LOG_CONFIG + ERR:
		case WMSPP_LOG_CONFIG + FATAL:
			LogMessageExt(uMsg - WMSPP_LOG_CONFIG, (int)wParam, WMSPP_LOG_CONFIG, (LPCTSTR)lParam);
			break;

		case WMSPP_AUDIO_INSIG:
			DbgObj->InputSignalReady((PBYTE)wParam, (PVOID)lParam);
			break;
 
		case WMSPP_PROC_PULSE:
			DbgObj->PulseReady((PVOID)wParam, (UINT)lParam);
			break;

		case WMSPP_DLG_MAP:
			Spp->MappingChanged(  (Mapping*&)wParam,  Conf->SelectedvJoyDevice());
			Conf->Map(Conf->SelectedvJoyDevice(), ((Mapping *)wParam));
			SendMessage(hDialog, WMSPP_MAP_UPDT, wParam, lParam);
			break;

		//case WMSPP_DLG_MAPBTN:
		//	Spp->MappingChanged((LPVOID &)(wParam), (UINT)lParam, Conf->SelectedvJoyDevice());
		//	Conf->MapButtons(Conf->SelectedvJoyDevice(), *(array<BYTE, 128> *)(wParam));
		//	SendMessage(hDialog, WMSPP_MAPBTN_UPDT, wParam, lParam);
		//	break;

		case WMSPP_DLG_CHNL:
			if ((UCHAR)wParam !=  Audio->GetwBitsPerSample())
			{
				Audio->SetwBitsPerSample((UCHAR)wParam);
				Spp->AudioChanged();
				Conf->SetDefaultBitRate((UINT)wParam);
			};
			

			// Set L/R to Spp
			if ((TCHAR)lParam == TEXT('L') && isRight!=0)
			{
				Conf->SetDefaultChannel(TEXT("Left"));
				Spp->SetAudioChannel(true);
			}
			else if ((TCHAR)lParam == TEXT('R') && isRight!=1)
			{
				Conf->SetDefaultChannel(TEXT("Right"));
				Spp->SetAudioChannel(false);
			}
			break;

		case WMSPP_DLG_AUTO:
			if ((WORD)wParam & AUTOCHANNEL)
			{
				Conf->SetAutoChannel(((WORD)lParam & AUTOCHANNEL)!=0, DEF_CHANNEL2);
				AutoChannel = (((WORD)lParam & AUTOCHANNEL)!=0);
			};

			if ((WORD)wParam & AUTOBITRATE)
			{
				Conf->SetAutoBitRate(((WORD)lParam & AUTOBITRATE)!=0, DEF_BITRATE);
				AutoBitRate = (((WORD)lParam & AUTOBITRATE)!=0);				
			}

			if ((WORD)wParam & AUTODECODE)
			{
				Conf->SetAutoDecoder((WORD)lParam != 0);
				DecoderAuto(hDialog);
			}
			break;


		// Stop/Start Streaming
		case WMSPP_DLG_STREAM:
			if ((BOOL)wParam)
				ReqStartCapture = true;
			else
				ReqStopCapture = true;
			break;

			// User pressed OK (or Cancel) button
		case WMSPP_DLG_OK:
			return AppExit();

		// GUI became iconified or switched to wizard mode
		case WMSPP_DLG_ICONFD:
			if (wParam)
				Conf->Wizard(false);
			else
				Conf->Wizard(true);
			break;

		case WMSPP_PRCS_GETLR:
			if (Audio->CurrentChannelIsRight())
				return 1;
			else
				return 0;
			break;


		// Start scanning for correct modulation
		case WMSPP_DLG_SCAN:
			if (Spp)
				// Start (TRUE), search until detection (FALSE), or until timeout (3000mS)
				Spp->SetDecoderScanning(TRUE, FALSE, DECODER_TIMEOUT);
			break;
 
        default: 
            return DefWindowProc(hwnd, uMsg, wParam, lParam); 
    } 
    return 0; 
}

void MessageLoop(void)
{
	MSG winmsg;
	BOOL bRet;

	while( (bRet = GetMessage( &winmsg, NULL, 0, 0 )) != 0)
	{ 

		if (bRet == -1)
		{
			// handle the error and possibly exit
			return;
		}
		else
		{
			// Handle message from a new SPP process
			if (WM_INTERSPPCONSOLE == winmsg.message)
			{
				Dialog->Show();
				Dialog->Iconified(false);
			};

			TranslateMessage(&winmsg);
			DispatchMessage(&winmsg);
		}

		if (!IsWindow(hMain))
			return;

	}
}

// Call this function once when received request to exit application
// The function:
// - Kills all child threads (Execept GUI)
// - Closes GUI window then kills GUI thread
// - Distroys application
bool		AppExit(void)
{
	// Ask user if sure
	TCHAR msg[MAX_MSG_SIZE];
	TCHAR ttl[MAX_MSG_SIZE];
	LoadString(g_hInstance, IDS_W_AREYOUSURE, reinterpret_cast< LPWSTR >( &msg ), sizeof(msg)/sizeof(TCHAR) );
	LoadString(g_hInstance, IDS_T_AREYOUSURE, reinterpret_cast< LPWSTR >( &ttl ), sizeof(ttl)/sizeof(TCHAR) );
	int kill = MessageBox(NULL, msg, ttl, MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2|MB_SYSTEMMODAL);
	if (kill == IDNO)
		return false;

	// Kill Log Window
	if (hLog)
	{
		SendMessage(hLog, WM_DESTROY, 0, 0);
		hLog = NULL;
	}

 	// Clean-up the Scope window
	if (Spp)
		Spp->RegisterPulseMonitor(1,  false);


	// Kill Dialog box
	SendMessage(hDialog, WM_DESTROY, 0,0);
	delete(Dialog);

	// Stop thread "CvJoyMonitor Central thread (Loop)"
	 vJoyMonitorClose();

	// Stop thread "SppControl Monitor Thread"
	Monitor = false;
	if (tMonitor && tMonitor->joinable())
		tMonitor->join();
	delete(tMonitor);

	// Stop cupture and monitoring signal
	if (Spp)
		delete(Spp);

	// Kill main window
	SendMessage(hMain, WM_DESTROY, 0,0);

	// Clean Up
	delete(Conf);
	delete(Audio);
	delete(DbgObj);
	if (LogWin)
		delete(LogWin);

	return true;

	// Exit	 exit(0);
	
}

// Start/Stop monitoring the pulse data using Pulse Scope
void PulseScope(BOOL start)
{
	// Set CONF file

	// Start/Stop
	Spp->RegisterPulseMonitor(1, start ? true : false);
}

// Compute the state of the Operation state machine
// When state computed, OperatStateMachine is updated
// When state seems to be stable an appropreate message is sent to GUI
// State machine defined: https://docs.google.com/document/d/17zp6J4uAuh13n_Rg7E_8qoVE0qjtG7FfbYEtF9Yuo-c/edit#heading=h.ba01j0box48e
void ComputeOperatState(void)
{
	UINT PosQual=0;
	UINT JoyQual=0;
	UINT AudioQualitySel, AudioQualityOther;

	// Respond to requests to stop or start capture
	if (ReqStartCapture/* && OperatStateMachine == S0*/)
	{
		ReqStopCapture = false;
		ReqStartCapture = false;
		if (Spp->Start())
			LogMessage(INFO, IDS_I_STARTSPPPRS);
		else
		{
			LogMessage(FATAL, IDS_F_STARTSPPPRS);
			return;
		};
		FilterPopulate(hDialog);
		Spp->AudioChanged();
		OperatStateMachine = UNKNOWN;
		Conf->Stopped(false);
		return;
	};

	if (OperatStateMachine == UNKNOWN && ReqStartCapture)
	{
		ReqStartCapture = false;
		return;
	}

	if (ReqStopCapture && OperatStateMachine != S3 && OperatStateMachine != S0)
	{
		//ReqStopCapture = false;
		if (Spp->Stop())
			LogMessage(INFO, IDS_I_STOPSPPPRS);
		else
		{
			LogMessage(ERR, IDS_E_STOPSPPPRS);
			return;
		};

		OperatStateMachine = S3;
		Conf->Stopped(true);
		return;
	};

	if (OperatStateMachine == S3)
	{
		ReqStopCapture = false;
		OperatStateMachine = S0;
		return;
	};

	// Capture started or Stopped - inform GUI
	if (StartCapture)
	{
		StartCapture = false;
		SendMessage(hDialog, WMSPP_PRCS_ALIVE, (WPARAM)1, 0);
	};

	if (StopCapture)
	{
		StopCapture = false;
		SendMessage(hDialog, WMSPP_PRCS_ALIVE, (WPARAM)0, 0);
	};

	// Nothing to compute is user chose to be in stopped mode
	if (OperatStateMachine == S0)
		return;


	// Get audio quality of Selected audio channel and of Other audio channel
	AudioQualitySel = GetAudioQuality(true);
	AudioQualityOther = GetAudioQuality(false);

	// Get the qulity of the Position data - Higher than 75 is good quality
	PosQual = Spp->GetPositionDataQuality();

	// Get the qulity of the joystick communication - Higher than 90 is good quality
	JoyQual = Spp->GetJoystickCommQuality();

#if 0
	// If in W7/W71/W71 and signal is good then restart processing
	if (OperatStateMachine == W7 || OperatStateMachine == W71 || OperatStateMachine == W72)
	{
		Spp->Start();
		OperatStateMachine=UNKNOWN;
		return;
	}

#endif // 0

	// Infor GUI of quality
	SendMessage(hDialog, WMSPP_DECD_QLT,PosQual, 0);
	SendMessage(hDialog, WMSPP_JOY_QLT,JoyQual, 0);

	// State Feed (W10)
	// Perfect conditions, all is functioning
	// Joystick communication is very good (JoyQual>=75)
#if 0
	if (JoyQual>=75)
	{
		if ((AudioQualitySel<LEVEL_LO) && (AudioQualitySel<AudioQualityOther))
			OperatStateMachine = SetState(OperatStateMachine, W101, CONSOLE_BLN_W101);
		else
			OperatStateMachine = SetState(OperatStateMachine, W10, CONSOLE_BLN_W10);
		return;
	} // W10

#endif // 0

	// State Process (W9/W10)
	// Quality of Position data is good (PosQual >= 75)
    // Quality of audio is not minimal
	if (PosQual >= 75 && AudioQualitySel>LEVEL_VLO)
	{
		if (vJoyDeviceEnabled)
		{
			if ((AudioQualitySel<LEVEL_LO) && (AudioQualitySel<AudioQualityOther))
				OperatStateMachine = SetState(OperatStateMachine, W101, CONSOLE_BLN_W101);
			else
				OperatStateMachine = SetState(OperatStateMachine, W10, CONSOLE_BLN_W10); // W10
		} 
		else
		    OperatStateMachine = SetState(OperatStateMachine, W9, CONSOLE_BLN_W9);// W9
		return;
	} 

	// State NoProcess (W8)
	// Joystick communication is not good (JoyQual<90)
	// Quality of Position data is not good (PosQual < 75)
	// Audio level of selected channel is not bad
	if (AudioQualitySel>=LEVEL_LO)
	{
		if (AudioQualityOther>=LEVEL_LO)
            OperatStateMachine = SetState(OperatStateMachine, W81, CONSOLE_BLN_W81);
		else if (AudioQualityOther>AudioQualitySel)
            OperatStateMachine = SetState(OperatStateMachine, W82, CONSOLE_BLN_W82);
		else
            OperatStateMachine = SetState(OperatStateMachine, W8, CONSOLE_BLN_W8);
		return;
	}

	//
	// State NoSignal (W7)
	// Joystick communication is not good (JoyQual<90)
	// Quality of Position data is not good (PosQual < 75)
	// Audio level of selected channel is very bad
	if (AudioQualitySel<=LEVEL_LO)
	{
		if (AudioQualityOther>AudioQualitySel)
			OperatStateMachine = SetState(OperatStateMachine, W71, CONSOLE_BLN_W71);
		else if (AudioQualitySel>LEVEL_VLO)
			OperatStateMachine = SetState(OperatStateMachine, W72, CONSOLE_BLN_W72);
		else
			OperatStateMachine = SetState(OperatStateMachine, W7, CONSOLE_BLN_W7);
		//Spp->Stop();
		return;
	}

	// Default
	{
		OperatStateMachine = SetState(OperatStateMachine, UNKNOWN, L"Unknown Error");
		return;
	}

}

// Return value in the range 0-100 indicating the audio quality of a channel
// To get info for the currently selected channel - ActiveChannel=true
// To get info for the other channel - ActiveChannel=false
UINT GetAudioQuality(bool ActiveChannel)
{
	if (isRight==-1)
		return 0;

	UINT CurAudioLevel;
	if ((isRight && ActiveChannel) ||  (!isRight && !ActiveChannel))
		CurAudioLevel = AudioLevel[1];
	else
		CurAudioLevel = AudioLevel[0];

	if ( (CurAudioLevel<LEVEL_LO) && (BitRate<16))
		return 0;
	else
		return CurAudioLevel;
}

// AudioLevelWatch - Analyse audio level data
// Gets Audio Levels
// Updates GUI
// If the selected channel is much weaker than the other channel then inform GUI
// If the selected channel is weak (under 95 = LEVEL_VHI) then inform GUI
// If wrong channel then select the right channel (if in auto mode)
// If bitrate is not optimal then change it (if in auto mode)
void AudioLevelWatch()
{
	static bool WentAutoCh=true;
	static bool WentAutoBr=true;
	static WCHAR prevCh = L'\0';
	static UINT  prevBr = 0;
	wstring ch = L"";

	if (!AudioId)
	{
		LogMessage(ERROR, IDS_E_AUDIOID);
		return;
	}
	// Initializing channel state
	//if (isRight==-1)
	//{


	// Get the number of channels (Stereo/Mono)
	int nChannels = Audio->GetNumberChannels((PVOID)AudioId);

	if (nChannels>1)
	{
		ch = Conf->GetAudioDeviceChannel(const_cast<LPTSTR>(AudioId));
		if (ch[0] == L'R' || ch[0] == L'r')
			isRight=1;
		else
			isRight=0;
	}
	else
	{
		ch = L"Mono";
		isRight=0;
	};
	//};

	// Initialize bit rate
	BitRate = Conf->GetAudioDeviceBitRate(const_cast<LPTSTR>(AudioId));
	if (!BitRate)
		BitRate = DEF_BITRATE;

	// Get the audio levels of the selected audio device
	AudioLevel[0] = static_cast<UINT>(100* Audio->GetChannelPeak((PVOID)AudioId, 0));
	AudioLevel[1] = static_cast<UINT>(100* Audio->GetChannelPeak((PVOID)AudioId, 1));

	// Inform GUI
	if (nChannels == 1)
		PostMessage(hDialog, VJOYDEV_CH_LEVEL, (WPARAM)AudioId, MAKELPARAM(AudioLevel[0],101));
	else
		PostMessage(hDialog, VJOYDEV_CH_LEVEL, (WPARAM)AudioId, MAKELPARAM(AudioLevel[0],AudioLevel[1]));

	// Inform GUI of change in Channel or BitRate
	if ((ch[0] != prevCh) || (BitRate != prevBr))
	{
		SendMessage(hDialog, SET_AUDIO_PARAMS,BitRate, ch[0]);
		prevBr = BitRate;
		prevCh = ch[0];
	};

	// Wrong Channel?
	// Change channel to the other channel if the other channel is much louder:
	// 1. Auto channel was selected by the user
	// 2. The other channel level is very high (>95)
	// 3. The current channel is low (<50)


	// If channel selection is automatic - test if the correct channel is selected
	if (AutoChannel)
	{	
		if (isRight && AudioLevel[0]>AudioLevel[1] && AudioLevel[1]<LEVEL_LO)
		{// Right channel selected and the left channel gives a stronger signal then switch to Left signal
			if (nChannels>1)
				Conf->SetDefaultChannel(TEXT("Left"));
			else
				Conf->SetDefaultChannel(TEXT("Mono"));
			Spp->SetAudioChannel(true);
			Spp->AudioChanged();
			isRight=0;
			WentAutoCh=true;
			if (nChannels>1)
				SendMessage(hDialog, SET_AUDIO_PARAMS,0, 'L');
			else
				SendMessage(hDialog, SET_AUDIO_PARAMS,0, 'M');
		}
		else if (!isRight && AudioLevel[1]>AudioLevel[0] && AudioLevel[0]<LEVEL_LO)
		{// Left channel selected and the right channel gives a stronger signal then switch to Left signal
			Conf->SetDefaultChannel(TEXT("Right"));
			Spp->SetAudioChannel(false);
			Spp->AudioChanged();
			isRight=1;
			SendMessage(hDialog, SET_AUDIO_PARAMS,0, 'R');
		};
		SendMessage(hDialog, SET_AUDIO_AUTO, (WPARAM)AUTOCHANNEL,  (WPARAM)AUTOCHANNEL);
		WentAutoCh=true;
	} // AutoChannel
	else
	{
		// Manual mode
		if (WentAutoCh) // Just unchecked auto?
		{
			bool right = Conf->IsDefaultChannelRight();
			Spp->SetAudioChannel(!right);
			isRight=-1;
			WentAutoCh=false;
			if (nChannels == 1)
				SendMessage(hDialog, SET_AUDIO_PARAMS,0, 'M');
			else
			{
				if (right)
					SendMessage(hDialog, SET_AUDIO_PARAMS,0, 'R');
				else
					SendMessage(hDialog, SET_AUDIO_PARAMS,0, 'L');
			}
			SendMessage(hDialog, SET_AUDIO_AUTO, (WPARAM)AUTOCHANNEL,0);
			Spp->AudioChanged();
		};
	};

	// Wrong bit rate?
	// Weak signal (<80) requires a high bitrate (16)
	// Very strong signal (>95) can settle for low bit rate (8)

	// If bit rate is automatic - set it according to the quality of the audio
	if (AutoBitRate)
	{
		UINT Level;
		if (isRight)
			Level = AudioLevel[1];
		else
			Level = AudioLevel[0];

		// Too weak? change to 16 bits
		if (Level <80 && BitRate==8)
		{
			Conf->SetDefaultBitRate(16);
			Audio->SetwBitsPerSample(16);
			Spp->AudioChanged();
			BitRate=16;
			WentAutoBr=true;
			SendMessage(hDialog, SET_AUDIO_PARAMS,16, NULL);
		}

		// Changed. Always prefer 16 bits 
		else if (/*Level >LEVEL_VHI && */BitRate!=16)
		{
			BitRate=16;
			//BitRate=8;  //// Very strong? change to 8 bits (Removed)
			Conf->SetDefaultBitRate(BitRate);
			Audio->SetwBitsPerSample(BitRate);
			Spp->AudioChanged();
			WentAutoBr=true;
			SendMessage(hDialog, SET_AUDIO_PARAMS,BitRate, NULL);
		}
		SendMessage(hDialog, SET_AUDIO_AUTO, (WPARAM)AUTOBITRATE,  (WPARAM)AUTOBITRATE);
		WentAutoBr=true;
	}
	else
	{
		// Manual mode
		if (WentAutoBr) // Just unchecked auto?
		{
			BitRate = Conf->GetAudioDeviceBitRate(const_cast<LPTSTR>(AudioId));
			Audio->SetwBitsPerSample(BitRate);
			SendMessage(hDialog, SET_AUDIO_PARAMS,BitRate, NULL);
			BitRate=-1;
			WentAutoBr=false;
			SendMessage(hDialog, SET_AUDIO_AUTO, (WPARAM)AUTOBITRATE,0);
			Spp->AudioChanged();
		};
	};

}

/*
	Decoder auto-detection
	Get auto state from configuration file (Conf->IsDecoderAuto()) 
	Inform GUI of the state by sending message (SET_DEC_AUTO).
	If auto state is on then instruct SppProcess to constatly scan encoding method (Spp->SetDecoderScanning(TRUE, TRUE, 0))
*/
void DecoderAuto(HWND hDlg)
{
	// Return if configuration file is not loaded
	if (!Conf)
		return;

	// Get the auto-state of the decoder
	bool Auto = Conf->IsDecoderAuto();
	Conf->SetAutoDecoder(Auto);

	// Send state to GUI
	SendMessage(hDlg, SET_DEC_AUTO, (WPARAM)Auto, 0);

	// Instruct SppProcess to constatly scan encoding method/Stop scanning
	if (Spp)
	{
		if (Auto)
			Spp->SetDecoderScanning(TRUE, TRUE, 0);
		else
			Spp->SetDecoderScanning(FALSE, TRUE, 0);

	};
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
			BitRate = Conf->GetAudioDeviceBitRate(jack.id);
			std::wstring Channel = Conf->GetAudioDeviceChannel(jack.id);
			if (!BitRate)
				BitRate = DEF_BITRATE;

			if (!Channel.length() && jack.nChannels != 1)
					Channel = DEF_CHANNEL2;

			if (jack.nChannels == 1)
					Channel = DEF_CHANNEL1;


			SendMessage(hDlg, SET_AUDIO_PARAMS, (WPARAM)BitRate, (LPARAM)((Channel.c_str())[0]));

			// Auto/Manual channel selection
			if (Conf->IsDefaultChannelAuto())
			{
				AutoChannel=true;
				SendMessage(hDlg, SET_AUDIO_AUTO, (WPARAM)AUTOCHANNEL, (LPARAM)AUTOCHANNEL);
			}
			else
			{
				AutoChannel=false;
				SendMessage(hDlg, SET_AUDIO_AUTO, (WPARAM)AUTOCHANNEL, 0);
			};

			// Auto/Manual bit rate selection
			if (Conf->IsDefaultBitRateAuto())
			{
				AutoBitRate = true;
				SendMessage(hDlg, SET_AUDIO_AUTO, (WPARAM)AUTOBITRATE, (LPARAM)AUTOBITRATE);
			}
			else
			{
				AutoBitRate = false;
				SendMessage(hDlg, SET_AUDIO_AUTO, (WPARAM)AUTOBITRATE, 0);
			};


			if (Audio)
				Audio->SetwBitsPerSample(BitRate);
		};
	};

}

// Pulse Scope setup from config file and send it to SppProcess
// If data not available then use default value
void SetPulseScope(HWND hDlg)
{
	int monitor = 1; // Default value
	if (Conf)
		monitor = Conf->PulseScope();
	if (monitor == -1)
		monitor = 0;

	BOOL bMonitor = 1;
	if (!monitor)
		bMonitor = 0;

	if (Spp)
		Spp->RegisterPulseMonitor(1, monitor ? true : false);

	SendMessage(hDlg, PULSE_SCOPE, (WPARAM)bMonitor, 0);
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
	UINT			(CALLBACK * GetDllVersion)(void);
	int				(CALLBACK *pGetNumberOfFilters)(void);
	const char *    (CALLBACK *pGetFilterNameByIndexA)(const int iFilter);
	LPCWSTR			(CALLBACK *pGetFilterNameByIndexW)(const int iFilter);
	const int		(CALLBACK *pGetFilterIdByIndex)(const int iFilter);
	const int		(CALLBACK *pSelectFilterByIndex)(const int iFilter);
	const int		(CALLBACK * pGetIndexOfSelectedFilter)(void);




	// TODO: Release former library

	// Get filter file name from config file
	wstring wsFileName(Conf->FilterFile());
	LPCTSTR FileName = wsFileName.data();

	// Load the filter DLL file - If does not exist send message to GUI (Call it 4.0.0)
	h = LoadLibraryEx(FileName, NULL, 0);
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

	// Prepare file name to send to GUI
	wchar_t * ShortName = new TCHAR[MAX_PATH];
	wchar_t * Ext = new TCHAR[MAX_PATH];
	wchar_t * out = new TCHAR[MAX_PATH];
	_tsplitpath_s((const wchar_t *)FileName, NULL, NULL, NULL, NULL, ShortName, MAX_PATH, Ext, MAX_PATH);
	_stprintf_s(out, MAX_PATH, TEXT("%s%s"), ShortName, Ext);
	delete[] ShortName;
	delete[] Ext;


	// Assign global handle to Filters' DLL
	hDllFilters = h;

	// Verify that the DLL version is not too old
	GetDllVersion = (UINT(CALLBACK *)(void))GetProcAddress(h,"GetDllVersion");
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
	pGetNumberOfFilters = (int (CALLBACK *)(void))GetProcAddress(h,"GetNumberOfFilters");
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
	SendMessage(hDlg, FILTER_NUM, nFilters, (LPARAM)out);
	delete[] out;

	// Get selected filter
	UINT idSel = Conf->GetSelectedFilter();

	// Get interface functions
	pGetFilterNameByIndexA = (const char * (CALLBACK *)(const int iFilter))GetProcAddress(h,"GetFilterNameByIndex"); // TODO: Add support to WCHAR
	pGetFilterNameByIndexW = (LPCWSTR (CALLBACK *)(const int iFilter))GetProcAddress(h,"GetFilterNameByIndexW"); // TODO: Add support to WCHAR
	pGetFilterIdByIndex = (const int   (CALLBACK *)(const int iFilter))GetProcAddress(h,"GetFilterIdByIndex");
	pSelectFilterByIndex = (const int  (CALLBACK *)(const int iFilter))GetProcAddress(h,"SelectFilterByIndex");
	pGetIndexOfSelectedFilter = (const int  (CALLBACK *)(void))GetProcAddress(h,"GetIndexOfSelectedFilter");

	// For every filter send data to GUI and to config file
	const char * FilterName = NULL;
	WCHAR FilterNameWa[255] = {NULL};
	LPWSTR FilterNameW = &FilterNameWa[0];
	int FilterId = -1;
	for (int iFilter=0; iFilter<nFilters ; iFilter++)
	{
		FilterName = NULL;
		FilterNameWa[0] = NULL;
		size_t converted;

		if (pGetFilterNameByIndexW)
			FilterNameW = const_cast<WCHAR *>(pGetFilterNameByIndexW(iFilter));
		else
			FilterName = pGetFilterNameByIndexA(iFilter);

		// Sanity check
		if (!FilterName && !FilterNameW)
			return NULL;

		// If got a char* then convert to LPCWSTR
		if ((!FilterNameW || !wcslen(FilterNameW)) && FilterName)
			mbstowcs_s(&converted, FilterNameW, 255, FilterName, strlen(FilterName));

		if (!FilterNameW)
			return h;

		FilterId = pGetFilterIdByIndex(iFilter);
		Conf->AddFilter(FilterId, FilterNameW);
		SendMessage(hDlg, FILTER_ADDW, FilterId, (LPARAM)FilterNameW);
		if (FilterId == idSel)
		{
			SendMessage(hDlg, FILTER_SELCTED, FilterId, 0);
			SelectFilter(FilterId);
		}
	};
	
	LogMessage(INFO, IDS_I_FILTERDLLOK);
	return h;
}

// Get full path to filter file
// Test the file validity
// If valid:
// 1. Write to config file
// 2. Return the file name (Not path)
// 3. Request thread thMonitor to populate list of filters
LPVOID SelectFilterFile(LPCTSTR FilterPath)
{

	// Test if valid and get version
	wchar_t * out = new TCHAR[MAX_PATH];
	DWORD FilterVer = GetFilterFileVersion((LPTSTR)FilterPath);
	if (FilterVer == -1)
		return NULL;

	// Convert version to string
	wstring strVer = to_wstring((FilterVer>>24) & 0xFF) + L"." + to_wstring((FilterVer>>16) & 0xFF) + L"." + to_wstring((FilterVer>>8) & 0xFF) + L"." + to_wstring((FilterVer>>0) & 0xFF);

	// Write to config file
	Conf->FilterFile((LPTSTR)FilterPath, (LPTSTR)strVer.data());

	// Extract file name
	wchar_t * FileName = new TCHAR[MAX_PATH];
	wchar_t * Ext = new TCHAR[MAX_PATH];
	_tsplitpath_s((const wchar_t *)FilterPath, NULL, NULL, NULL, NULL, FileName, MAX_PATH, Ext, MAX_PATH);
	_stprintf_s(out, MAX_PATH, TEXT("%s%s"), FileName, Ext);
	delete[] FileName;
	delete[] Ext;

	reqPopulateFilter = true;

return (LPVOID)out;
}

void		SelectFilter(int FilterId)
{
	// Verify that DLL file is loaded
	if (!hDllFilters)
	{
		LogMessage(WARN, IDS_W_FILTERSELFAIL);
		return;
	}

	// Get Procedure Addresses from DLL
	const int		(WINAPI * pSelectFilterByIndex)(const int iFilter);
	PJS_CHANNELS	(WINAPI * pProcessChannels)(PJS_CHANNELS, int max, int min);
	int				(CALLBACK *pGetNumberOfFilters)(void);
	const int		(CALLBACK *pGetFilterIdByIndex)(const int iFilter);
	const char *    (CALLBACK *pGetFilterNameByIndexA)(const int iFilter);
	LPCWSTR			(CALLBACK *pGetFilterNameByIndexW)(const int iFilter);
	pGetNumberOfFilters = (int  (CALLBACK *)(void))GetProcAddress(hDllFilters,"GetNumberOfFilters");
	pGetFilterIdByIndex = (const int   (CALLBACK *)(const int iFilter))GetProcAddress(hDllFilters,"GetFilterIdByIndex");
	pSelectFilterByIndex = (const int  (WINAPI *)(const int iFilter))GetProcAddress(hDllFilters,"SelectFilterByIndex");
	pProcessChannels = (PJS_CHANNELS (WINAPI * )(PJS_CHANNELS, int max, int min))GetProcAddress(hDllFilters,"ProcessChannels");
	pGetFilterNameByIndexA = (const char *    (CALLBACK *)(const int i))GetProcAddress(hDllFilters,"GetFilterNameByIndex"); // TODO: Add support to WCHAR
	pGetFilterNameByIndexW = (LPCWSTR (CALLBACK *)(const int iFilter))GetProcAddress(hDllFilters,"GetFilterNameByIndexW"); // TODO: Add support to WCHAR


	int nFilters;
	if (pGetNumberOfFilters)
		nFilters  = pGetNumberOfFilters();
	else
		return;

	// Get the filter index by filter ID
	int iSel=-1;
	for (int iFilter=0; iFilter<nFilters ; iFilter++)
	{
		if (FilterId == pGetFilterIdByIndex(iFilter))
		{
			iSel=iFilter;
			break;
		};
	};

	// Update the DLL which is the selected filter
	if (pSelectFilterByIndex)
		pSelectFilterByIndex(iSel);
	else
	{
		LogMessage(ERR, IDS_E_FILTERSELFAIL);
		return;
	};

	// Mark selected filter in config file as selected
	WCHAR NameWa[255] = {NULL};
	LPWSTR NameW = &NameWa[0];
	size_t converted;
	const char * Name = NULL;
	if (pGetFilterNameByIndexW)
		NameW = const_cast<WCHAR *>(pGetFilterNameByIndexW(iSel));
	else
		Name = pGetFilterNameByIndexA(iSel);

	// If got a char* then convert to LPCWSTR
	if (Name && (!NameW || !wcslen(NameW)))
		mbstowcs_s(&converted, NameW, 255, Name, strlen(Name));


	Conf->AddFilter(FilterId, NameW, true);

	// Get the pointer to the filter function
	Spp->SelectFilter(iSel, (LPVOID)pProcessChannels);
	LogMessage(INFO, IDS_I_FILTERSELOK);
}

DWORD		GetFilterFileVersion(LPTSTR FilterPath)
{
	HINSTANCE h;
	//typedef UINT (CALLBACK* LPFNDLLFUNC0)();
	//LPFNDLLFUNC0 GetDllVersion;
	UINT			(CALLBACK * GetDllVersion)(void);

	//int				(CALLBACK *pGetNumberOfFilters)(void);
	//const char *    (CALLBACK *pGetFilterNameByIndexA)(const int iFilter);
	//const int		(CALLBACK *pGetFilterIdByIndex)(const int iFilter);
	//const int		(CALLBACK *pSelectFilterByIndex)(const int iFilter);
	//const int		(CALLBACK * pGetIndexOfSelectedFilter)(void);

	// Load the filter DLL file - If does not exist send message to GUI (Call it 4.0.0)
	h = LoadLibraryEx(FilterPath, NULL, 0);
	if (!h)
	{
		DWORD err = GetLastError();
		if (err==0xc1)
			LogMessage(WARN, IDS_W_FILTERDLL, TEXT("x64/x86 DLL incompatibility"));
		return -1;
	}
	else
	{
		LogMessage(INFO, IDS_I_FILTERDLL);
	}

	// Version 3.x.x - Verify that the DLL version is not too old
	GetDllVersion = (UINT(CALLBACK *)(void))GetProcAddress(h,"GetDllVersion");
	if (!GetDllVersion)
	{
		LogMessage(ERR, IDS_E_FILTERDLLVER);
		FreeLibrary(h);
		return -1;
	};

	DWORD filter_ver = GetDllVersion();
	FreeLibrary(h);
	return filter_ver;
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

	SendMessageTimeout(hLog , WMSPP_LOG_CNTRL + Severity, (WPARAM)Code, (LPARAM)Msg, SMTO_ABORTIFHUNG, 1000, NULL);
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

// Dialog Box thread
void thDialogBox(HWND hwnd)
{
	THREAD_NAME("Dialog Box Thread");
	Dialog	= new SppDlg(g_hInstance, hwnd);
	std::unique_lock<std::mutex> lk(mtx_dialogbox);
	dialogbox_is_ready = true;
	cv_dialogbox.notify_one();
	mtx_dialogbox.unlock();
	Dialog->MsgLoop();
	lk.release();
}

// Monitor Thread
// Polls activity of the different modules
// Reports health of the modules and attemps recovery when possible
void thMonitor(bool * KeepAlive)
{
	bool stopped = false;
	int rID=0;
	OperatState oState = UNKNOWN;
	UINT PosQual=0;

	THREAD_NAME("SppControl Monitor Thread");

	while (*KeepAlive)
	{

		Sleep_For( 100 );// Sleep for 100 milliseconds
		
		// Gets Audio Levels - optimizes and Updates GUI
		AudioLevelWatch();

		// Inform SppProcess of the current info
		// This will go to the SppScope information pane
		if (Spp)
			Spp->Info2Scope(BitRate, isRight, Audio->GetNumberChannels());


		//  Compute the state of the Operation state machine and inform GUI
		ComputeOperatState(); // TODO: Much tweeking

		// Inform GUI of changes in the Start/Stop conditions
		if (OperatStateMachine != oState)
		{
			oState = OperatStateMachine;
			SetNotificationIcon(NULL);
		};

		// Test if need to populate filter
		if (reqPopulateFilter)
		{
			FilterPopulate(hDialog);
			reqPopulateFilter=false;
		};


		// Test state of vJoy device - only for legal vJoy device Id
		if (vJoyDevice)
		{
			// vJoy changed - relinquish previous and update config file
			if (rID != vJoyDevice)
			{
				if (!rID)
				{
					if (vJoyEnabled())
					{
                        vJoyDeviceEnabled = true;
						vJoyDevice = Conf->SelectedvJoyDevice();
						vJoyDevicesPopulate(hDialog);
						vJoyDevicesVersion(hDialog);
						SetvJoyMapping(vJoyDevice);
						SetAvailableControls((UINT)vJoyDevice, hDialog);// Instruct GUI of the available Controls (Axes/Buttons)
						//StartPollingDevice(vJoyDevice);
					}
					else
					{
                        vJoyDeviceEnabled = false;
						continue;
					};
				}
				else
					RelinquishVJD(rID);

				rID = vJoyDevice; 
				Spp->vJoyDeviceId(rID);
			};

			// Check status of vJoy device - if not owened try to acquire
			// Inform SppProcess
			VjdStat stat = GetVJDStatus(rID);
			if (stat == VJD_STAT_OWN)
				Spp->vJoyReady(true);
			else
				AcquireVJD(rID) ? Spp->vJoyReady(true) :  Spp->vJoyReady(false);

			// If vJoy device is missing then inform the GUI and select another one
			stat = GetVJDStatus(rID);
			if (stat == VJD_STAT_MISS)
			{
				//StopPollingDevices();
				Spp->vJoyReady(false);
				RelinquishVJD(rID);
				rID=0;
				continue;
			};
		}; // if (vJoyDevice)

	}; // while (*KeepAlive) 
}

// Connect to the vJoy interface
// Get the vJoy driver version
// send message with version (as UINT: 00aabbcc = aa.bb.cc) to dialog box
UINT vJoyDevicesVersion(HWND hDlg)
{
	INT Major, Mid, Minor, Ver;
	LPTSTR str = (LPTSTR)GetvJoySerialNumberString();
	if (!str || (_tcslen(str) < 5))
		return 0;

	// Version string is valid - extract data
	_stscanf_s(str, TEXT("%d.%d.%d"), &Major, &Mid, &Minor);
	Ver = (Major << 16) + (Mid << 8) + Minor;

	// Send message to dialog box - Version
	SendMessage(hDlg, VJOYDEV_SETVER, Ver, 0);

	return Ver;
}


// Connect to the vJoy interface
// Get the number of devices
// Enumerate through all devices - for each device send message to dialog box
// If configuration file already indicate a selected device (and this device is valid) then mark as selected in the dialog box
// If the indicated device is in valid or none of the devices is indicated then mark the lowest-id device as selected
int vJoyDevicesPopulate(HWND hDlg)
{
	int nDev=0;
	UINT selected;
	int id;
	bool match = false;
	bool found = false;
	VjdStat stat;
	int DefId = 0;

	// Get selected device from conf file
	selected = Conf->SelectedvJoyDevice();

	// Clear the GUI
	SendMessage(hDlg, VJOYDEV_REMALL, 0, 0);

	// Loop on all devices
	for (id=1; id< 16; id++)
	{
		// Make sure this vJoy device is usable
		stat = GetVJDStatus(id);
		if (stat == VJD_STAT_BUSY || stat == VJD_STAT_MISS || stat == VJD_STAT_UNKN)
			continue;

		// If there's no selected device then select this one
		if (!selected)
		{
			selected = id;
			Conf->SelectvJoyDevice(id);
		};

		// Temporary default id
		if (!DefId)
			DefId = id;

		// Add device to GUI
		match =  (id == selected);
		SendMessage(hDlg, VJOYDEV_ADD, id, (LPARAM)match);
		nDev++;

		// Indicate that there is a selected vJoy device (Sticky)
		if (match)
			found = true;
	}; // For loop

	// In case none was selected, select the first one
	if (!found)
	{
		selected = DefId;
		Conf->SelectvJoyDevice(selected);
		SendMessage(hDlg, VJOYDEV_SETSEL, selected, 0);
	};

	return nDev;
}

// Return the device ID of the slected vJoy device
// Take if from the configuration file and check if exists
// If valid ID (0<ID<17) and exists the return it
// Otherwise look for the lowest ID that exists and return it
// If not found return 0
int SelectedvJoyDevice(void)
{
	VjdStat stat;
	int DefId = 0;

	// Get the selected. If none selected then vJoyDevice is 0
	// If the selected is valid the return it
	int SelDevice = Conf->SelectedvJoyDevice();
	if (SelDevice)
	{
		stat = GetVJDStatus(SelDevice);
		if ((stat == VJD_STAT_OWN) || (stat == VJD_STAT_FREE))
			return SelDevice;
	};

	// Search for the first available device
	// Loop on all devices
	for (int id=1; id< 16; id++)
	{
		// Make sure this vJoy device is usable
		stat = GetVJDStatus(id);
		if (stat == VJD_STAT_BUSY || stat == VJD_STAT_MISS || stat == VJD_STAT_UNKN)
			continue;

		if ((stat == VJD_STAT_OWN) || (stat == VJD_STAT_FREE))
		{
			Conf->SelectvJoyDevice(id);
			return id;
		};

	}; // For loop


	return 0; // Not found
}

void SetAvailableControls(UINT id, HWND hDlg)
{
	controls ctrls;
	// Get data from vJoy Interface
	ctrls.nButtons = GetVJDButtonNumber(id);
	for (UINT i=0; i<8; i++)
		ctrls.axis[i] = (GetVJDAxisExist(id, HID_USAGE_X+i) == TRUE);

	// Send data to GUI
	SendMessage(hDlg, VJOYDEV_SETAVAIL, id, (LPARAM)&ctrls);
}

void SetvJoyMapping(UINT id)
{
	// Axes
	const UINT nAxes = 8;
	DWORD dAxisMap = Conf->MapAxis(id);

	// Buttons
	BTNArr aButtonMap = {0};
	UINT nButtons = (UINT)aButtonMap.size();
	Conf->GetMapButtons(id, &aButtonMap);

	Mapping Map;
	Map.nAxes = nAxes;
	Map.pAxisMap = &dAxisMap;
	Map.nButtons = nButtons;
	Map.ButtonArray = &aButtonMap;
	Conf->Map(id, &Map);

	if (id)
		Spp->MappingChanged(&Map, id); 
	SendMessage(hDialog, WMSPP_MAP_UPDT, (WPARAM)&Map, 0);
}

// Send the status to the notification icon 
void SetNotificationIcon(LPCTSTR Message)
{
	if (!hDialog)
		return;

	SendMessage(hDialog, WMSPP_STAT_UPDT, (WPARAM)OperatStateMachine, (LPARAM)Message);
	LogMessage(INFO,IDS_I_STATUSCHANGED ,Message);
}

// Set a new status
// Test that this is not a glitch
// If not a glitch then set the status and send notification
OperatState SetState(OperatState current, OperatState next, LPCTSTR Msg)
{
	static UINT StabilityCount=0;
	static OperatState tmp = S0;

	// Change?
	if (current == next)
		return next;

	// Wait for State to stabilize
	if (tmp != next)
	{
		StabilityCount=0;
		tmp = next;
	}
	else
		StabilityCount++;

	// Test
	if (StabilityCount < 5)
		return current;

	// OK, reached stability
	if (Msg)
		SetNotificationIcon(Msg);
	return next;
}


// Get the start mode out of the command line
// Options are:
// -n|-N	Normal mode (default)
// -w|-W	Wizard mode
// -s|-S	Stopped mode
WORD	GetStartMode(LPTSTR lpCmdLine)
{
	WORD out = FLG_NONE;

	// Read Command line
	int argc = 0;
	LPWSTR* argv = NULL;
	argv = CommandLineToArgvW(lpCmdLine, &argc);
	for (int i=0; i<argc; i++)
	{
		// Wizard 
		if (!_tcsncicmp(argv[i], TEXT("-w"),2))
		{
			out |= FLG_WIZRD;
			break;
		};

		// Stopped
		if (!_tcsncicmp(argv[i], TEXT("-s"),2))
		{
			out |= FLG_STPD;
			break;
		};
	};

	LocalFree(argv);
	return out;
}

// TODO: Remove pragma and solve problem
#pragma warning( push )
#pragma warning( disable : 4996 )
bool isAboveVistaSp1()
{

	BOOL res = IsWindowsVistaSP1OrGreater();
	if (res == TRUE)
		return true;
	else
		return false;


#if 0
	OSVERSIONINFOEX OsInfo;
	OsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((LPOSVERSIONINFO)&OsInfo);

	// Vista or higher
	if (OsInfo.dwMajorVersion < 6)
		return false;

	// Windows7 or higher
	if (OsInfo.dwMinorVersion > 0)
		return true;

	// Vista SP2 or higher Vista
	if (OsInfo.wServicePackMajor > 1)
		return true;
	else
		return false;
#endif // 0

}
#pragma warning( pop )

#ifdef _DEBUG
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(char* threadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = GetCurrentThreadId();
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
#endif
