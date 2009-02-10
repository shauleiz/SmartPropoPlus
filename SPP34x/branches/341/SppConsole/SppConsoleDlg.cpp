// SppConsoleDlg.cpp : implementation file
//
#include "stdafx.h"
#include "shlwapi.h"
#include <sys/timeb.h>
#include <time.h>
#include "filterif.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "DefVolumeDlg.h"
#include "LogAudioHdrs.h"
#include "LogPulse.h"
#include "LogFmsConn.h"
#include "LogRawPulse.h"
#include "WaveRec.h"
#include "SmartPropoPlus.h"
#include "ScanInputs.h"
#include ".\sppconsoledlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg dialog

CSppConsoleDlg::CSppConsoleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSppConsoleDlg::IDD, pParent)
	, m_Timer1(0)
	, m_TimerFmsStat(0)
	//, m_WaveRecording(false)
	, m_FmsLatency(-1)
	, m_TimestampFromDll(-1)
	, m_TimestampToDll(-1)
	, m_pFilters(NULL)
	, m_AudioInput(NULL)
	, m_iSelMixer(-1)
	, m_iSelLine(-1)
	, m_RecWaveDialog(NULL)
	, m_LogAudioHdrsDialog(NULL)
	, m_LogPulseDialog(NULL)
	, m_LogFmsConnDialog(NULL)
	, m_LogRawPulseDialog(NULL)
	, m_pFmsConnLogFile(NULL)
	, m_FmsConnLogStat(Idle)
{
	//{{AFX_DATA_INIT(CSppConsoleDlg)
	m_enable_audio = FALSE; /* Version 3.3.3 - Default is FALSE */
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSppConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSppConsoleDlg)
	DDX_Control(pDX, IDC_SPK, m_spk);
	DDX_Check(pDX, IDC_ENABLE_AUDIO, m_enable_audio);
	DDX_Control(pDX, IDC_LEVEL, m_AudioLevelBar);
	DDX_Control(pDX, IDC_JS_POS1, m_JoyPosBar[0]);
	DDX_Control(pDX, IDC_JS_POS2, m_JoyPosBar[1]);
	DDX_Control(pDX, IDC_JS_POS3, m_JoyPosBar[2]);
	DDX_Control(pDX, IDC_JS_POS4, m_JoyPosBar[3]);
	DDX_Control(pDX, IDC_JS_POS5, m_JoyPosBar[4]);
	DDX_Control(pDX, IDC_JS_POS6, m_JoyPosBar[5]);
	DDX_Control(pDX, IDC_JS_POS7, m_JoyPosBar[6]);
	DDX_Control(pDX, IDC_JS_POS8, m_JoyPosBar[7]);
	DDX_Control(pDX, IDC_JS_POS9, m_JoyPosBar[8]);
	DDX_Control(pDX, IDC_JS_POS10, m_JoyPosBar[9]);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_LED_POS, m_LedValidPos);
}

static UINT NEAR WM_INTERSPPCONSOLE;
static UINT NEAR WM_INTERSPPAPPS;

BEGIN_MESSAGE_MAP(CSppConsoleDlg, CDialog)
	//{{AFX_MSG_MAP(CSppConsoleDlg)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_LBN_SELCHANGE(IDC_MOD_TYPE, OnSelchangeModType)
	ON_BN_CLICKED(IDC_SHIFT_AUTO, OnShiftAuto)
	ON_BN_CLICKED(IDC_SHIFT_POS, OnShiftPos)
	ON_BN_CLICKED(IDC_SHIFT_NEG, OnShiftNeg)
	ON_BN_CLICKED(IDC_HIDE, OnHide)
	ON_LBN_SELCHANGE(IDC_AUDIO_SRC, OnSelchangeAudioSrc)
	ON_WM_WINDOWPOSCHANGING()
	ON_CBN_SELCHANGE(IDC_MIXERDEVICE, OnSelchangeMixerdevice)
	ON_BN_CLICKED(IDC_ENABLE_AUDIO, OnEnableAudio)
	ON_WM_SETCURSOR()
	ON_COMMAND(IDC_SEL_SRC, OnSelSrc)
	ON_COMMAND(IDC_AUDIO_DST, OnAudioDst)
	ON_COMMAND(IDC_AUDIO_VOL, OnAudioVol)
	ON_COMMAND(IDC_AUDIO_PRINT, OnAudioPrint)
	ON_BN_CLICKED(IDC_PPJOYEX, OnPpjoyex)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SHOW, OnShow)
	ON_MESSAGE(WM_HIDEGUI, OnHideMsg)
	ON_MESSAGE(WM_GENERAL, OnStatusAreaMessage)
	ON_REGISTERED_MESSAGE(WM_INTERSPPCONSOLE, OnInterSppConsole)
	ON_REGISTERED_MESSAGE(WM_INTERSPPAPPS, OnInterSppApps)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify)
	ON_COMMAND_RANGE(IDC_FILTER_0, IDC_FILTER_63, OnFilterSelect)
	ON_MESSAGE(MM_MIXM_LINE_CHANGE,OnMixerLineChange)
	ON_MESSAGE(MM_MIXM_CONTROL_CHANGE,OnMixerControlChange)
	ON_WM_TIMER()
	//ON_UPDATE_COMMAND_UI(IDC_WAVE_REC, OnUpdateWaveRec)
	ON_COMMAND(IDC_WAVE_REC, OnWaveRec)
	ON_MESSAGE(MSG_WAVE_REC_START, OnWaveRecStart)
	ON_MESSAGE(MSG_WAVE_REC_STOP, OnWaveRecStop)
	ON_MESSAGE(MSG_WAVE_REC_CANCEL, OnWaveRecCancel)
	ON_MESSAGE(MSG_AUDIO_HDRS_START, OnWaveRecStart)
	ON_MESSAGE(MSG_AUDIO_HDRS_STOP, OnWaveRecStop)
	ON_MESSAGE(MSG_AUDIO_HDRS_CANCEL, OnLogAudioHdrsCancel)
	ON_MESSAGE(MSG_PULSE_START, OnWaveRecStart)
	ON_MESSAGE(MSG_PULSE_STOP, OnWaveRecStop)
	ON_MESSAGE(MSG_PULSE_CANCEL, OnLogPulseCancel)
	ON_COMMAND(IDC_ADVANCED_JITTER, OnAdvancedJitter)
	ON_COMMAND(ID_FILE_RUNFMS, OnRunFms)
	ON_WM_SYSCOMMAND()
	ON_COMMAND(ID_LOG_AUDIO_HDRS, OnLogAudioHdrs)
	ON_COMMAND(ID_LOG_PULSE, OnLogPulse)
	ON_COMMAND(ID_LOG_FMSCONNECTION, OnLogFmsConn)
	ON_MESSAGE(MSG_FMS_CONN_START, OnFmsConnStart)
	ON_MESSAGE(MSG_FMS_CONN_STOP, OnFmsConnStop)
	ON_MESSAGE(MSG_FMS_CONN_CANCEL, OnFmsConnCancel)
	ON_COMMAND(ID_LOG_RAWPULSES, OnLogRawPulse)
	ON_MESSAGE(MSG_RAW_PULSE_CANCEL, OnRawPulseCancel)
	ON_BN_CLICKED(IDC_SCANDLG, OnBnClickedScandlg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

// Global functions
#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;
    /* For security purposes, LoadLibrary should be provided with a 
       fully-qualified path to the DLL. The lpszDllName variable should be
       tested to ensure that it is a fully qualified path before it is used. */
    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, 
                          "DllGetVersion");

        /* Because some DLLs might not implement this function, you
        must test for it explicitly. Depending on the particular 
        DLL, the lack of a DllGetVersion function can be a useful
        indicator of the version. */

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
               dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}


void ThreeBeeps()
{
	Beep(1000,300);
	Beep(50000,300);
	Beep(1000,300);
	Beep(50000,300);
	Beep(1000,300);
}

void OneBeep(void)
{
	Beep(1500,300);
	Beep(50000,300);
}

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg message handlers

BOOL CSppConsoleDlg::OnInitDialog()
{
	char msg[1000];

	/*** Unique Instance ***/
	/* Register a message to communicate between two SPPConsole instances */
	WM_INTERSPPCONSOLE = RegisterWindowMessage(INTERSPPCONSOLE);
	/* Register the Inter-Process message */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "OnInitDialog(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		::MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};

	/* Test if another SPPConsole is running */
	if (m_hConsoleMuxex=OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXCONSOLE))
	{	// another instance is already running and the second instance is NOT 
		// launched iconified then Broadcast a message.
		if (!m_Iconified)
			::PostMessage(HWND_BROADCAST, WM_INTERSPPCONSOLE, 0, 0);
		// Quit (No second instance allowed)
		CDialog::OnCancel();
		return FALSE;
	}
	else
		m_hConsoleMuxex = CreateMutex(NULL, TRUE, MUTXCONSOLE);


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	EnableToolTips();
	

	// Remove button from task bar
	ModifyStyleEx(WS_EX_APPWINDOW|WS_EX_CONTEXTHELP, 0/*WS_EX_TOOLWINDOW*/);
	// Add icon to the task bar (Tray)
	TaskBarAddIcon(IDR_MAINFRAME, m_hIcon, CONSOLE_TT);
	if (m_Iconified)
		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_OFF);

	// Alway on top
	LocateDialogWindow();

	// Dialog box title
	SetWindowText(CONSOLE_TITLE);
	
	/* Initialize Modulation related controls */
	PopulateModulation();

	/* Initialize Audio source related controls */
	m_AudioInput = new CAudioInput(m_hWnd);
	PopulateAudioSource();
	m_enable_audio = ::GetCurrentAudioState();
	CheckAudioCB();
	EnableAudio(m_enable_audio);


	/* Get handle to the dialog box menu and append filter menu if needed */
	m_pMainMenu = new CMenu();
	m_pMainMenu->LoadMenu(IDR_MENU_MAIN);
	UpdateFilterMenu();
	UpdateFmsMenuItem();

	///////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////// Start Wave2Joystick and test //////////////////////////////////////
	m_Wave2Joystick = GetWave2JoystickObject();
	if (!m_Wave2Joystick)
	{
		sprintf(msg, "OnInitDialog(): Cannot initialize Object Wave2Joystick");
		::MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};
	char * DeviceName = ::GetCurrentMixerDevice();
	BOOL SetActiveDev  = SppSetActiveAudioDevice(DeviceName);
	if (DeviceName) free (DeviceName);
	BOOL SetFilter = SetFilterInterface(m_pFilters);	// Set filter to audio capture object
	BOOL Started = SppStart();							// Start audio capture
	InitJitterStatus();									// Initialize anti-jitter mechanism
	OnSelchangeModType();								// Set the Modulation type.
	PpJoyExShowCtrl();									// Display/Hide check box. Check/uncheck it
	OnPpjoyex();										// Start PPJoy according to checkbox state

	m_TimerLevel = SetTimer(TIMER_LEVEL,200,(TIMERPROC)NULL); // Start timer that tests audio level
	m_TimerJoystick = SetTimer(TIMER_JOY,50,(TIMERPROC)NULL);	// Start timer that tests joystick positions

	///////////////////////////////////////////////////////////////////////////////////////////
	
	
	CDialog::OnInitDialog();


	/**************************** Additional changes to the look of dialog box ****************************/
	m_pSpkPic = GetDlgItem(IDC_SPK);

	// Audio level (Volume) gauge - set parameters
	m_AudioLevelBar.SetRange(10,30000);							// Set the range of the audio level meter
	m_AudioLevelBar.SetBkColor(0x00C0E0C0);						// Background colour: Pale green
	m_AudioLevelBar.SendMessage(PBM_SETBARCOLOR, 0, 0x0000C000);// Bar colour: Green

	// Initialize the joystick position bars
	InitJoyPosBars();

	return TRUE;  // return TRUE  unless you set the focus to a control

}

void CSppConsoleDlg::OnDestroy()
{
	/* Clean-up */
	KillTimer(m_TimerLevel);
	KillTimer(m_TimerJoystick);
	if (m_TimerFmsStat) KillTimer(m_TimerFmsStat);
	delete(m_AudioInput);
	WinHelp(0L, HELP_QUIT);
	TaskBarDelIcon(IDR_MAINFRAME);
	StoreLocationDialogWindow();
	if (m_pMainMenu) delete(m_pMainMenu);
	if (m_pFilters) delete(m_pFilters);
	CDialog::OnDestroy();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSppConsoleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//DEL UINT CSppConsoleDlg::OnNcHitTest(CPoint point )
//DEL {
//DEL 	UINT res = CWnd::OnNcHitTest(point);
//DEL 	if (res != HTCLIENT && res != HTBORDER)
//DEL 		res= res;
//DEL 	return res;
//DEL }

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSppConsoleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSppConsoleDlg::SomethingChanged()
{
	OnSelchangeAudioSrc();
	OnSelchangeMixerdevice();
	OnSelchangeModType();
	RedrawWindow();
}

void CSppConsoleDlg::OnSelchangeModType() 
{

	/* Get the List Box */
	CListBox* ModTypeList = (CListBox*)GetDlgItem(IDC_MOD_TYPE);
	if (!ModTypeList)
		return ;

	/* Get the selected item */
	CString text;
	int sel = ModTypeList->GetCurSel();
	ModTypeList->GetText(sel, text);

	/* Record the selected item in the Global memory area or the Registry */
	BOOL Ppm=TRUE;
	SetActiveMode((LPCTSTR)text);	
	if (text.Compare(MOD_NAME_PPM)) Ppm=FALSE;
	BOOL SetModulation = SppSetModulation((LPCTSTR)text);

	/* If PPM - update shift status */
	if (Ppm)
		OnShiftAuto();

	// Activate/Deactivate PPM-Only controls
	GetDlgItem(IDC_SHIFT_POL)->EnableWindow(Ppm);
	GetDlgItem(IDC_SHIFT_AUTO)->EnableWindow(Ppm);
	GetDlgItem(IDC_SHIFT_POS)->EnableWindow(Ppm);
	GetDlgItem(IDC_SHIFT_NEG)->EnableWindow(Ppm);
	UpdateJitterStatus(DONT_CHANGE,Ppm);
}

CWnd * CSppConsoleDlg::GetModeTypeCtrl()
{
	return GetDlgItem(IDC_MOD_TYPE);
}

void CSppConsoleDlg::OnShiftAuto() 
{
	/* Get the Check Box */
	CButton* AutoDetectCB = (CButton *)GetDlgItem(IDC_SHIFT_AUTO);
	if (!AutoDetectCB)
		return ;

	/* Get Status */
	int sel = AutoDetectCB->GetCheck();

	/* Record the auto detect state in the Global memory area or the Registry */ 
	SetShiftAutoDetect(sel);

	/* If auto detect selected - inform DLL */
	if (sel)
		SppSetShift(I_AUT_SHIFT);

	/* If auto detect NOT selected - set the correct shift */
	else
	{
		CButton * PositiveRB = (CButton *)GetDlgItem(IDC_SHIFT_POS);
		int PosCheck = PositiveRB->GetCheck();
		if (PosCheck)
			OnShiftPos();
		else
			OnShiftNeg();
	};
	
	/* Temporary - If Auto-Detect then hide the radio buttons */
	if (sel)
		ShowShiftRB(false);
	else
		ShowShiftRB();

}

CWnd * CSppConsoleDlg::GetShiftAutoCtrl()
{
	return GetDlgItem(IDC_SHIFT_AUTO);
}
void CSppConsoleDlg::OnShiftPos() 
{
	SetPositiveShift(1);
	SppSetShift(I_POS_SHIFT);
}
CWnd *  CSppConsoleDlg::GetShiftPosCtrl()
{
	return GetDlgItem(IDC_SHIFT_POS);
}

void CSppConsoleDlg::OnShiftNeg() 
{
	SetPositiveShift(0);	
	SppSetShift(I_NEG_SHIFT);
}

// Hide the dialog box
void CSppConsoleDlg::OnHide() 
{
	if (m_Iconified)
		return;
	m_Iconified = true;
	ShowWindow(SW_HIDE);
	
}

// Show the dialog box
void CSppConsoleDlg::OnShow() 
{
	if (!m_Iconified)
		return;
	m_Iconified = false;
	ShowWindow(SW_SHOW);
	
}


/* Get the list of Modulation types in their displayable mode */
int CSppConsoleDlg::PopulateModulation()
{

	/* Get the list from the Global memory area or the Registry */
	struct Modulations *  Modulation= GetModulation(1);
	if (!Modulation || Modulation->Active <0 )
		return -1;

	/* Get the List Box */
	int selected;
	CListBox* ModTypeList = (CListBox*)GetDlgItem(IDC_MOD_TYPE);
	if (!ModTypeList)
		return -1;

	/* Loop on the list and populate the list box */
	/* TODO: Test Modulation->ModulationList[i] */
	int i=0;
	while (Modulation->ModulationList[i])
	{
		ModTypeList->InsertString(i, ModeDisplayFromInternalName((Modulation->ModulationList[i])->ModTypeInternal));
		i++;
	};
	
	/* Get the selected modulation */
	if (Modulation && Modulation->Active >= 0)
		selected = Modulation->Active;
	else
		selected = -1;

	/* Mark it as selected */
	ModTypeList->SetCurSel(selected);

	/* Shift controls */
	CButton* AutoDetectCB = (CButton *)GetDlgItem(IDC_SHIFT_AUTO);
	AutoDetectCB->SetCheck(Modulation->ShiftAutoDetect);
	CButton * PositiveRB = (CButton *)GetDlgItem(IDC_SHIFT_POS);
	CButton * NegativeRB = (CButton *)GetDlgItem(IDC_SHIFT_NEG);
	PositiveRB->SetCheck(Modulation->PositiveShift);
	NegativeRB->SetCheck(!Modulation->PositiveShift);

	/* Temporary - If Auto-Detect then hide the radio buttons */
	if (Modulation->ShiftAutoDetect)
		ShowShiftRB(false);
	else
		ShowShiftRB();

	/* Some cleaning up */
	i=0;
	while (Modulation && Modulation->ModulationList[i])
	{
		if (Modulation->ModulationList[i]->ModTypeDisplay) delete (Modulation->ModulationList[i]->ModTypeDisplay);
		if (Modulation->ModulationList[i]->ModTypeInternal) delete (Modulation->ModulationList[i]->ModTypeInternal);
		free (Modulation->ModulationList[i]);
		i++;
	};

	free (Modulation->ModulationList);
	free (Modulation);

	return selected;
}


/*
	Display/Hide the positive & negative radio buttons
	
	MS-BUG: Calling mnemonics of a hidden control causes the dialog box to stall
	Workaround: Remove the Ampersand from the control text when hidden
*/
void CSppConsoleDlg::ShowShiftRB(bool show)
{
		CButton * PositiveRB = (CButton *)GetDlgItem(IDC_SHIFT_POS);
		CButton * NegativeRB = (CButton *)GetDlgItem(IDC_SHIFT_NEG);

		if (show)
		{
			SetDlgItemText(IDC_SHIFT_POS, MNM_POS);		// Mnemonics with '&'
			SetDlgItemText(IDC_SHIFT_NEG, MNM_NEG);		// Mnemonics with '&'
			PositiveRB->ShowWindow(SW_SHOW);
			NegativeRB->ShowWindow(SW_SHOW);
		}
		else
		{
			SetDlgItemText(IDC_SHIFT_POS, MNM_POS_HID);	// Mnemonics without '&'
			SetDlgItemText(IDC_SHIFT_NEG, MNM_NEG_HID);	// Mnemonics without '&'
			PositiveRB->ShowWindow(SW_HIDE);
			NegativeRB->ShowWindow(SW_HIDE);
		};

}

// TaskBarAddIcon - adds an icon to the taskbar status area. 
// Returns TRUE if successful, or FALSE otherwise. 
// uID - identifier of the icon. 
// hicon - handle to the icon to add. 
// lpszTip - ToolTip text. 
BOOL CSppConsoleDlg::TaskBarAddIcon(UINT uID, HICON hicon, LPSTR lpszTip)
{
    BOOL res; 
    NOTIFYICONDATA tnid={0};
    NOTIFYICONDATA_V6 tnid_v6={0};
	int DllVersion,  GoodDllVer;
	
	/* Test shell32.dll  Version number */
	DllVersion = GetDllVersion(TEXT("shell32.dll"));
	GoodDllVer = PACKVERSION(5,00);
	if (DllVersion<GoodDllVer)
	{
		/* Set the NOTIFYICONDATA fields with data - Old way*/
		tnid.cbSize = sizeof(NOTIFYICONDATA);				// Size of this structure
		tnid.hWnd = this->m_hWnd;							// Handle of this dialog box
		tnid.uID = uID;										// ID of this taskbar icon (for unique access)
		tnid.uFlags = NIF_MESSAGE | NIF_TIP | NIF_ICON;		// Supports Messaging to the application, Tooltip and icon
		tnid.uCallbackMessage = WM_GENERAL;					// Callback message
		tnid.hIcon = hicon; 		
		// Tooltip text
		if (lpszTip) 
			sprintf(tnid.szTip,lpszTip);					// Tooltip text
		// TODO: Add error handling for the HRESULT.
		else 
			tnid.szTip[0] = (TCHAR)'\0'; 
		res = Shell_NotifyIcon(NIM_ADD, &tnid); 
	}
	else
		/* Set the NOTIFYICONDATA fields with data - New way*/
	{	
		tnid_v6.cbSize = sizeof(NOTIFYICONDATA_V6);				// Size of this structure
		tnid_v6.hWnd = this->m_hWnd;							// Handle of this dialog box
		tnid_v6.uID = uID;										// ID of this taskbar icon (for unique access)
		tnid_v6.uFlags = NIF_MESSAGE|NIF_TIP|NIF_ICON|NIF_INFO;	// Supports Messaging to the application, Tooltip and icon
		tnid_v6.uCallbackMessage = WM_GENERAL;					// Callback message
		tnid_v6.hIcon = hicon; 
		tnid_v6.uVersion = NOTIFYICON_VERSION;					// Behaviour of W2K and higher
		Shell_NotifyIcon(NIM_SETVERSION, (NOTIFYICONDATA *)(&tnid_v6)); // Set as V5 icon
		tnid_v6.dwStateMask = NULL;
		tnid_v6.szInfoTitle[0] = '\0';
		tnid_v6.szInfo[0] = '\0';
		sprintf(tnid_v6.szInfoTitle,CONSOLE_BALOON_TTL);
//		sprintf(tnid_v6.szInfo,CONSOLE_BALOON_TXT);
		tnid_v6.dwInfoFlags = NIIF_NONE;
		tnid_v6.uTimeout = 0;
		// Tooltip text
		if (lpszTip) 
			sprintf(tnid_v6.szTip,lpszTip);						// Tooltip text
		// TODO: Add error handling for the HRESULT.
		else 
			tnid_v6.szTip[0] = (TCHAR)'\0'; 
		res = Shell_NotifyIcon(NIM_ADD, (NOTIFYICONDATA *)(&tnid_v6)); 
	}
	
    if (hicon) 
        DestroyIcon(hicon); 
	
	
    return res; 
}

// TaskBarDelIcon - Removes an icon to the taskbar status area. 
BOOL CSppConsoleDlg::TaskBarDelIcon(UINT uID)
{
    BOOL res; 
    NOTIFYICONDATA tnid;
	
	tnid.cbSize = sizeof(NOTIFYICONDATA);				// Size of this structure
	tnid.hWnd = this->m_hWnd;							// Handle of this dialog box
	tnid.uID = uID;										// ID of this taskbar icon (for unique access)
	
	res = Shell_NotifyIcon(NIM_DELETE, &tnid);

	return res;
}

// TaskBarIconBaloon - Display/Hide taskbar baloon
//  If BaloonText is NULL - then hide baloon
//	If BaloonTitle is NULL - then display default baloon title
//  Note: For IE V5 and higher
BOOL CSppConsoleDlg::TaskBarIconBaloon(UINT uID, const char *BaloonText, const char *BaloonTitle)
{
    BOOL res; 
	int DllVersion,  GoodDllVer;
    NOTIFYICONDATA_V6 tnid_v6={0};
	
	/* Test shell32.dll  Version number */
	DllVersion = GetDllVersion(TEXT("shell32.dll"));
	GoodDllVer = PACKVERSION(5,00);
	if (DllVersion<GoodDllVer)
		return FALSE; // Invalid for old versions

	tnid_v6.cbSize = sizeof(NOTIFYICONDATA_V6);				// Size of this structure
	tnid_v6.hWnd = this->m_hWnd;							// Handle of this dialog box
	tnid_v6.uID = uID;										// ID of this taskbar icon (for unique access)
	tnid_v6.uFlags = NIF_INFO;
	tnid_v6.dwInfoFlags = NIIF_NONE;
	tnid_v6.uTimeout = 10000;

	// Baloon title: Parameter or default
	if (BaloonTitle)
		sprintf(tnid_v6.szInfoTitle,BaloonTitle);
	else
		sprintf(tnid_v6.szInfoTitle,CONSOLE_BALOON_TTL);

	// Baloon text: If NULL then no baloon
	if (!BaloonText)
		tnid_v6.szInfo[0] = '\0';
	else
		sprintf(tnid_v6.szInfo,BaloonText);
	
	res = Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA *)(&tnid_v6));

	return res;

}

/*
// TaskBarIconToolTip - Display/Hide taskbar tooltip
*/
BOOL CSppConsoleDlg::TaskBarIconToolTip(UINT uID, const char *ToolTipText)
{
    BOOL res; 
    NOTIFYICONDATA tnid;
	
	tnid.cbSize = sizeof(NOTIFYICONDATA);				// Size of this structure
	tnid.hWnd = this->m_hWnd;							// Handle of this dialog box
	tnid.uID = uID;										// ID of this taskbar icon (for unique access)
	tnid.uFlags =  NIF_TIP;								// Supports Tooltip 
	if (ToolTipText) 
		sprintf(tnid.szTip,ToolTipText);					// Tooltip text
	// TODO: Add error handling for the HRESULT.
	else 
		tnid.szTip[0] = (TCHAR)'\0'; 
	
	res = Shell_NotifyIcon(NIM_MODIFY, &tnid);

	return res;
}


void CSppConsoleDlg::StartIconified(bool iconified)
{
	m_Iconified = iconified;
}


/* 
	Handler to mouse activity over the task bar icon
*/
LRESULT CSppConsoleDlg::OnStatusAreaMessage(WPARAM wParam, LPARAM lParam)
{
	UINT uID; 
    UINT uMouseMsg; 
 
    uID = (UINT) wParam; 
    uMouseMsg = (UINT) lParam; 
 
	/* Left button */
    if (uMouseMsg == WM_LBUTTONDOWN  || uMouseMsg == WM_LBUTTONDBLCLK) 
		OnShow();

	/* Right button */
	else if (uMouseMsg == WM_RBUTTONDOWN)
		ShowTrayIconMenu(IDR_MENU_TEST);


	return 1;
}

LRESULT CSppConsoleDlg::OnMixerLineChange(WPARAM wParam, LPARAM lParam)
{
	return 0;
}
/*
	3.4.0
	Handle for Mixer Control Changed
	Ignore input parameters - just get the current input line and change the selection
*/
LRESULT CSppConsoleDlg::OnMixerControlChange(WPARAM wParam, LPARAM lParam)
{
	unsigned int iLine;
	static unsigned int iOldLine;
	bool res;
	DWORD ThreadId=0;

	res = GetCurrentInputLineFromSystem(&iLine);
	if (res && iOldLine != iLine)
	{
		iOldLine = iLine;
		/* Get the line name list box */
		CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
		if (!InputLineNameList)
			return 1;
		InputLineNameList->SetCurSel(iLine);
		m_iSelLine = iLine;
	}
	return 0;
}


/*
	Display the Tray Icon menu
*/
void CSppConsoleDlg::ShowTrayIconMenu(UINT uID)
{
	
	CMenu menu;
	if (!menu.LoadMenu(uID))
		return;
	CMenu* pSubMenu = menu.GetSubMenu(0);
	if (!pSubMenu) 
		return;
	
	// Make first menu item the default (bold font)
	::SetMenuDefaultItem(pSubMenu->m_hMenu, 0, TRUE);
	
	// Deactivate (grey) the irrelevant options
	WINDOWPLACEMENT wp;
	GetWindowPlacement(&wp);
	RECT rect;
	this->GetWindowRect(&rect);

	if (m_Iconified)
		::EnableMenuItem(pSubMenu->m_hMenu, IDC_HIDE, MF_GRAYED);
	else
		::EnableMenuItem(pSubMenu->m_hMenu, IDC_SHOW, MF_GRAYED);
	
	if (isDllActive())
		::EnableMenuItem(pSubMenu->m_hMenu, IDOK, MF_GRAYED);

	
	// Display the menu at the current mouse location. There's a "bug"
	// (Microsoft calls it a feature) in Windows 95 that requires calling
	// SetForegroundWindow. To find out more, search for Q135788 in MSDN.
	//
	CPoint mouse;
	GetCursorPos(&mouse);
	::SetForegroundWindow(m_hWnd);	
	::TrackPopupMenu(pSubMenu->m_hMenu, 0, mouse.x, mouse.y, 0,
		m_hWnd, NULL);
	
}

/*
	This is how we make sure that the dialog box is started hidden (if needed to)
	Thanks to http://www.voidnish.com/articles/ShowArticle.aspx?code=dlgboxtricks
	by Nishant Sivakumar (Nish)
*/
void CSppConsoleDlg::OnWindowPosChanging(WINDOWPOS FAR* lpwndpos) 
{
	if (m_Iconified)
		lpwndpos->flags &= ~SWP_SHOWWINDOW;

	CDialog::OnWindowPosChanging(lpwndpos);	
}

/*
	Message handler for message WM_INTERSPPCONSOLE
	The message is sent when a second instance is launched
	The second instance launches it and here, the first instance handle it.
	If the first instance is iconified then the handler make it visible
*/
LRESULT CSppConsoleDlg::OnInterSppConsole(WPARAM wParam, LPARAM lParam) 
{
	if (m_Iconified)
		OnShow();

	return 0;
}

/*
	Handler to the WM_INTERSPPAPPS message that is used by the DLL to
	communicate with the GUI.
	- MSG_DLLSTOPPING: DLL is stopping: Kill GUI unless the dialog is visible or show a baloon
	- MSG_DLLSTARTING: DLL is starting: Open a baloon and update GUI version
	- MSG_DLLSTOPPING: DLL encountered PPJoy change of status
*/
LRESULT CSppConsoleDlg::OnInterSppApps(WPARAM wParam, LPARAM lParam)
{
	CButton* OK = (CButton *)GetDlgItem(IDOK);

	/* DLL is stopping */
	if (wParam == MSG_DLLSTOPPING)
	{
		if (m_Iconified)	// Kill application if DLL is dying and dialog box iconified
			OnOK();
		else				// Display baloon saying that the DLL is dying
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_DLL_STOPPED , "");

		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_OFF);

		/* Enable the DONE button */
		OK->EnableWindow(TRUE);

		/* FMS info frame becomes inactive */
		GetDlgItem(IDC_FMS_FRAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_FMS_STATUS)->EnableWindow(FALSE);
		GetDlgItem(IDC_FMS_STATUS)->SetWindowText("Disconnected");
		if (m_TimerFmsStat) KillTimer(m_TimerFmsStat);

		/* 'Run FMS' menu item is set to default */
		UpdateFmsMenuItem();

		return 1;
	};
	
	if (wParam == MSG_DLLSTARTING)
	{
		/* Display baloon */
		CString Msg = "\t";

		if (m_Iconified)	// Show an elaborate baloon
			Msg = GetStatusString();
		TaskBarIconBaloon(IDR_MAINFRAME, (LPCTSTR)(Msg) , BALOON_DLL_STARTING);
		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_ON);

		/* Disable (Grey) the DONE button */
		OK->EnableWindow(FALSE);
		
		/* FMS info frame becomes active */
		GetDlgItem(IDC_FMS_FRAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_FMS_STATUS)->EnableWindow(TRUE);
		GetDlgItem(IDC_FMS_STATUS)->SetWindowText("Connected");
		m_TimerFmsStat = SetTimer(TIMER_FMS_STAT,200,(TIMERPROC)NULL);

		/* 'Run FMS' menu item is grayed */
		UpdateFmsMenuItem(NULL);

		return 1;
	};

	/* Called every time FMS requests joystick position */
	if (wParam == MSG_DLLTIMESTAMP)
	{
		m_TimestampFromDll = (UINT)lParam;
		return 1;
	};

	/* Called every time joystick position is sent to FMS*/
	if (wParam == MSG_TIMESTAMP2DLL)
	{
		m_TimestampToDll = (UINT)lParam;
		return 1;
	};

	if (wParam == MSG_DLLPPJSTAT)
	{
		CString msg;
		static LPARAM prev=0x12345;
		int error = (int)(lParam&0xFFFF);
		int vjsIndex = (int)(lParam>>16);
		char Display; // Display mask: 0x0001->Balloon; 0x0010->Status Window

		// Print data only if data has changed
		if (prev == lParam)
			return 1;
		else
			prev = lParam;

		if (!(lParam&0xFFFF))
		{	// lParam == 0 -> OK
			msg.Format(BALOON_PPJ_CNCT, vjsIndex);
			Display=3; // Both
		} else
		if ((lParam&0xFFFF) == 2)
		{ // Underlying joystick device deleted
			msg.Format(BALOON_PPJ_NOTFND, vjsIndex);
			Display=3; // Both
		} else
		if ((lParam&0xFFFF) == 6)
		{ // The joystick device handle is invalid
			msg.Format(BALOON_PPJ_INVHNDL);
			Display=3; // Both
		} else
		if ((lParam&0xFFFF) == MSG_DPPJSTAT_DISCN)
		{ // The joystick disconnected (intentially)
			msg.Format(BALOON_PPJ_DISCNCT, vjsIndex);
			Display=3; // Both
		} else
		if ((lParam&0xFFFF) == MSG_DPPJSTAT_NOTCN)
		{ // The joystick disconnected (intentially)
			msg.Format(BALOON_PPJ_NOTCNCT, vjsIndex);
			Display=2; // Both
		} else
		{			
			msg.Format("PPJoy error: %d", lParam&0xFFFF);
			Display=3; // Both
		};

		if (Display & 0x2)
		{
			CEdit* PpjoyStatusEdit = (CEdit *)GetDlgItem(IDC_PPJOY_STATUS); // PPJoy Status window
			PpjoyStatusEdit->SetWindowText(msg);
		};
		if (Display & 0x1)
			TaskBarIconBaloon(IDR_MAINFRAME, msg);
		return 1;
	}
	
	/* Message from WINMM about joystick filter status */
	if (wParam == MSG_JSCHPPEVAIL)
	{

//		ASSERT(0);
		if (lParam == 0)
			m_JsChPostProcEvail = false;
		else
		{
			m_JsChPostProcEvail = true;
			UpdateFilterMenu();
		};

	};
	
	return 0;
}

/*
The OK/Done/Exit button acts differently:
-	If Winmm.DLL runs it acts as HIDE
-	If Winmm.DLL does not run it acts as OK
*/
void CSppConsoleDlg::OnOK() 
{
	if (m_hConsoleMuxex)
	{
		ReleaseMutex(m_hConsoleMuxex);
		CloseHandle(m_hConsoleMuxex);
	};
	if (m_Wave2Joystick)
	{
		StopInterfaces();
		SppStop();
	}

	CDialog::OnOK();
/*	if (!isDllActive())
	else
		OnHide();*/
}

void CSppConsoleDlg::OnCancel()
{
	OnOK();
}

/* Test is Winmm.DLL is (still) active */
bool CSppConsoleDlg::isDllActive()
{
		/* Test if winmm.dll is running */
	m_hMuxex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXWINMM);

	if (!m_hMuxex)
		return false;

	CloseHandle(m_hMuxex);
	return true;
}

/*
	Create a textual summary to display in the opening baloon
*/
CString CSppConsoleDlg::GetStatusString()
{
	CString tmp;
	CString Mod="", Auto="", Shift="", Audio="", Out;
	int autodetect;

	/* Modulation type */
	CListBox* ModTypeList = (CListBox*)GetDlgItem(IDC_MOD_TYPE);
	if (ModTypeList)
	{
		/* Get the current selected item */
		int index = ModTypeList->GetCurSel();
		if (index != LB_ERR )
		{	/* If there is a selected item - get its text */
			ModTypeList->GetText(index, tmp);
			Mod.Format(BALOON_DLL_MODTYPE,tmp);
		};
	};
	
	
	/* Auto status */
	CButton* AutoDetectCB = (CButton *)GetDlgItem(IDC_SHIFT_AUTO);
	if (AutoDetectCB)
	{
		/* Get Status */
		autodetect = AutoDetectCB->GetCheck();
		if (autodetect)
			Auto = BALOON_DLL_AUTOON;
		else
			Auto = BALOON_DLL_AUTOOFF;
	}
	
	/* Shift value */
	if (!autodetect)
	{	/* Get the shidt data if not auto-detect*/
		CButton * PositiveRB = (CButton *)GetDlgItem(IDC_SHIFT_POS);
		CButton * NegativeRB = (CButton *)GetDlgItem(IDC_SHIFT_NEG);
		if (NegativeRB && PositiveRB)
		{	/* Get shift */
			if (PositiveRB->GetCheck())
				Shift = BALOON_DLL_SHIFTPOS;
			else
				Shift = BALOON_DLL_SHIFTNEG;
		};
	};

	/* Audio status */
	CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
	if (InputLineNameList && InputLineNameList->IsWindowEnabled())
	{
		int sel = InputLineNameList->GetCurSel();
		if (sel<0)
			tmp="";
		else
			InputLineNameList->GetText(sel, tmp);
		Audio.Format(BALOON_DLL_AUDIO, tmp);
	};

	Out = Mod+Auto+Shift+Audio;

	return Out;
}

/* Tool Tip massage handler */
BOOL CSppConsoleDlg::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    DWORD64 nID = pNMHDR->idFrom;
    if (pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
            pTTT->lpszText = MAKEINTRESOURCE(nID);
            pTTT->hinst = AfxGetResourceHandle();
            return(TRUE);
        }
    }
    return(FALSE);
}

/*
	Calculate the status of the SPP to FMS joystick position message round trip
	based on the following parameters:
	m_TimestampToDll:	Time stamp of message sent from SPP to winmm.dll
	m_TimestampFromDll:	Time stamp of the message received at to winmm.dll

	Status calculation:
	*	OK:		status=0
		When communication is good, both timestamps (m_TimestampToDll, m_TimestampFromDll)should be equal 
		or m_TimestampToDll should be slightly bigger.
	*	SLOW:	status=1
		When both m_TimestampToDll & m_TimestampFromDll advance but m_TimestampToDll is much bigger than m_TimestampFromDll
		It is considered as a slow connection.
	*	NO_DATA:	status=2
	 	When m_TimestampToDll does not advance, it means that data is not sent to winmm.dll
	*	ON_HOLD:	status=3
		When m_TimestampFromDll does not advance, it means that winmm.dll is not reading the joystick position message.
		This is the case when FMS is out of focus.
	*	PROBLEM:	status=4
	 	If m_TimestampToDll < m_TimestampFromDll then we have a problem
*/
int CSppConsoleDlg::GetFmsStatusString(CString &StatStr)
{
	static int TimestampToDll, TimestampFromDll, CntSlow, CntHold, CntData, status;
	const char *StatList[] = FMS_STAT_STR;

	int FmsLatency = m_TimestampToDll-m_TimestampFromDll;

	if (TimestampToDll == m_TimestampToDll)
	{
		CntSlow=0;
		CntHold=0;
		if (++CntData > 2)
		{
			status = 2;
			CntData=0;
		};
	}
	else if (TimestampFromDll == m_TimestampFromDll)
	{
		CntSlow=CntData=0;
		if (++CntHold > 6)
		{
			status = 3;
			CntHold=0;
		};
	}
	else if (FmsLatency < 0)
		status = 4;
	else if (FmsLatency > 300) 
	{
		CntHold=CntData=0;
		if (++CntSlow > 10)
		{
			status = 1;
			CntSlow=0;
		};
	}
	else
	{
		status = 0;
		CntSlow=CntHold=CntData=0;
	};

	TimestampFromDll	= m_TimestampFromDll;
	TimestampToDll		= m_TimestampToDll;
	StatStr = StatList[status];

	// Log file section
	if ((m_FmsConnLogStat != Printing) && (m_FmsConnLogStat != Started)) return status;
	if (m_FmsConnLogStat == Started) m_FmsConnLogStat = Printing;

   struct _timeb timebuffer;
   _ftime( &timebuffer );
   tm * LocalTime = localtime(&timebuffer.time);

   fprintf(m_pFmsConnLogFile, "%02d:%02d:%02d.%03d\t[%d]\t%-35s\t%0d\t%0d\t%d\n", LocalTime->tm_hour, LocalTime->tm_min, LocalTime->tm_sec,timebuffer.millitm,status, StatStr,TimestampToDll&0xFFFF,TimestampFromDll&0xFFFF, FmsLatency);

	return status;
}

void CSppConsoleDlg::PopulateAudioSource()
{
	/*** Mixer Device combo box ***/

	/* Get the Mixer Device Box */
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return;

	/* Loop on the list and populate the combo box */
	int nAudioDev = m_AudioInput->GetCountMixerDevice();
	for (int i=0 ; i<nAudioDev ; i++)
		MixerList->InsertString(i, m_AudioInput->GetMixerDeviceName(i));

	/* No audio device detected */
	if (nAudioDev < 1)
	{
		m_enable_audio = false;
		EnableAudio(m_enable_audio);
		CheckAudioCB();
	}
	else
	{
		m_iSelMixer = GetCurrentMixerDevice();
		MixerList->SetCurSel(m_iSelMixer);
		m_AudioInput->SetCurrentMixerDevice(m_iSelMixer);
	};


	/*** List of Inputs ***/
	PopulateInputLines();
}

CWnd * CSppConsoleDlg::GetMixerDeviceCtrl()
{
	return GetDlgItem(IDC_MIXERDEVICE);
}

void CSppConsoleDlg::PopulateInputLines()
{
	/*** Get the selected Mixer device ***/
	/* Get the Mixer Device Box */
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return;

	/* Get the selected item */
	int sel = MixerList->GetCurSel( );
	if (sel == CB_ERR)
		return;
	else
		m_iSelMixer = sel;

	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(sel);

	/* Get the line name list box */
	CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
	if (!InputLineNameList)
		return;

	/* Initialize */
	InputLineNameList->ResetContent();

	/* Loop on all Input Lines of the selected device */
	const char * LineName;
	int nLine = 0;
	while (md && (LineName = md->GetInputLineName(nLine)))
	{
		InputLineNameList->InsertString(nLine, LineName);
		nLine++;
	};

	/* Get current Input Line from the registry */
	m_iSelLine = GetCurrentInputLine();
	InputLineNameList->SetCurSel(m_iSelLine);
}

CWnd * CSppConsoleDlg::GetAudioLinesCtrl()
{
	return GetDlgItem(IDC_AUDIO_SRC);
}

void CSppConsoleDlg::OnSelchangeMixerdevice() 
{
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return;

	/* Clear the joystick position data */
	m_JoystickPos[0] = 0;

	/* Get the selected item */
	int sel = MixerList->GetCurSel( );
	if (sel == CB_ERR)
		return;

	/* Set the selected Mixer Device as current one in the registry */
	SetCurrentMixerDevice(sel);
	PopulateInputLines();

	/****** 3.4.0 - Synch the selected source line with the system ******/
	/* Get the selected Mixer device */
	m_iSelMixer = MixerList->GetCurSel();
	if (m_iSelMixer < 0)
		return;
	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(m_iSelMixer);

	/* Select the Input Line */
	if (md)
		md->SetSelectedInputLine(m_iSelLine);

	char * MixerDevice = ::GetCurrentMixerDevice();
	BOOL SetActiveDev  = SppSetActiveAudioDevice(MixerDevice);
	if (MixerDevice) free (MixerDevice);

	BOOL Started = SppStart();
	return;

}

void CSppConsoleDlg::OnSelchangeAudioSrc() 
{
	/* Get the line name list box */
	CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
	if (!InputLineNameList)
		return;

	/* Get the newly selected Input Line*/
	m_iSelLine = InputLineNameList->GetCurSel();
	if (m_iSelLine < 0)
		return;

	/* Set the selectedInput Line as current one in the registry */
	SetCurrentInputLine(m_iSelLine);

	/* Get the Mixer Device Box */
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return;

	/* Get the selected Mixer device */
	m_iSelMixer = MixerList->GetCurSel();
	if (m_iSelMixer < 0)
		return;
	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(m_iSelMixer);

	/* Select the Input Line */
	if (md)
		md->SetSelectedInputLine(m_iSelLine);
}

void CSppConsoleDlg::OnEnableAudio() 
{

	// Toggle
	//m_enable_audio = !m_enable_audio;
	CButton* AudioCB = (CButton*)GetDlgItem(IDC_ENABLE_AUDIO);
	m_enable_audio = AudioCB->GetCheck();

	// Register in registry
	::SetCurrentAudioState(m_enable_audio);

	// Do it
	EnableAudio(m_enable_audio);

	// Inform the Wave2Joystick module
	char * MixerDevice = ::GetCurrentMixerDevice();
	BOOL SetActiveDev  = SppSetActiveAudioDevice(MixerDevice);
	if (MixerDevice) free (MixerDevice);

	BOOL Started = SppStart();

}

void CSppConsoleDlg::EnableAudio(int enable)
{
	static int SelLine=-1, SelMixer=-1;

	// Enable/disable audio controls
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return;
	MixerList->EnableWindow(enable);

	CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
	if (!InputLineNameList)
		return;
	InputLineNameList->EnableWindow(enable);

	/* Enable/Disable the speaker picture */
	CStatic * spk = (CStatic *)GetDlgItem(IDC_SPK);
	if (spk)
	{
		if (enable)
			spk->ShowWindow(SW_SHOW);
		else
			spk->ShowWindow(SW_HIDE);
	};


	// If disable then store the current values and restore the values before start
	int iMixer, iLine; // Index of Mixer & Line to display
	if (!enable)
	{
		SelMixer = MixerList->GetCurSel();
		SelLine  = InputLineNameList->GetCurSel();
		m_AudioInput->Restore();
		iMixer = GetCurrentMixerDevice();
		iLine = GetCurrentInputLine();
	}
	else
	{
		// If uninitialized then get preset values
		if (SelMixer<0)
			SelMixer = GetCurrentMixerDevice();
		if (SelLine<0)
			SelLine = GetCurrentInputLine();

		// Restore set-up
		CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(SelMixer);
		if (md && SelLine > -1)
			md->SetSelectedInputLine(SelLine);		
		iMixer = SelMixer;
		iLine  = SelLine;
	};

	// Update display
	MixerList->SetCurSel(iMixer);
	InputLineNameList->SetCurSel(iLine);
	
}

/*
	Get the last-used mixer device
	First check the registry.
	Then check the current set-up
	If no data (because this is the first time) then get the preferred (default) mixer device
	Return the index of the current mixer device
*/
int CSppConsoleDlg::GetCurrentMixerDevice()
{
	char * MixerName = ::GetCurrentMixerDevice();

	/* If no mixer selected - get the Preferred Mixer */

	if (MixerName && strlen(MixerName)) // Calculate the mixer device ID from mixer name
	{
		int MixerId = m_AudioInput->GetMixerDeviceID(MixerName);
		::SetCurrentMixerDevice(MixerName);
		int index =  m_AudioInput->GetMixerDeviceIndex(MixerName);
		free(MixerName);
		return index;
	};

	/* Default */
	return m_AudioInput->GetPreferredMixerDeviceIndex();
}


/* IFAIK - there is no such thing, so I just return the first one */
//DEL bool CSppConsoleDlg::GetCurrentMixerDeviceFromSystem(int *iMixer)
//DEL {
//DEL 	*iMixer=0;
//DEL 	return true;
//DEL }

/*
	Get the last-used input line
	First check the registry.
	Then check the current set-up
	Return the index of the current input line
*/
int CSppConsoleDlg::GetCurrentInputLine()
{
	unsigned int iLine;

	if (GetSavedCurrentInputLine(&iLine))
		return iLine;
	if (GetCurrentInputLineFromSystem(&iLine))
		return iLine;

	return 0;

}

/*
	Get Input Line index from the registry in two steps
	1. Get the Source Line ID from the registry
	2. Convert Source Line ID into Index

*/
bool CSppConsoleDlg::GetSavedCurrentInputLine(unsigned int *iLine)
{
	/* Get the Source Line ID from the registry */
	unsigned int SrcID;
	int res = ::GetCurrentInputLine(&SrcID);
	if (!res)
		return false;

	/* Convert Source Line ID into Index */
	int iMixer = m_AudioInput->GetCurrentMixerDevice();
	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(iMixer);
	if (md)
		return md->GetInputLineIndex(SrcID, iLine);
	else
		return false;

}

/*
	Get Input Line Index from the system via the AudioInput object
	1. Get current Mixer Device
	2. Get Selected Input Line of this Mixer Device
*/
bool CSppConsoleDlg::GetCurrentInputLineFromSystem(unsigned int *iLine)
{
	bool res;
	int iMixer = m_AudioInput->GetCurrentMixerDevice();
	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(iMixer);
	if (md)
		res = md->GetSelectedInputLine(iLine);
	else
		res = false;

	return res;
}



/*
	Set the selected (current) mixer device
*/
void CSppConsoleDlg::SetCurrentMixerDevice(unsigned int iMixer)
{
	/* Update AudioInput object */
	m_AudioInput->SetCurrentMixerDevice(iMixer);

	/* Get the Mixer Device string and put it in the registry */
	const char * MixerName = m_AudioInput->GetMixerDeviceName(iMixer);
	::SetCurrentMixerDevice(MixerName);
}

/*
	Set the selected (current) input line
*/
void CSppConsoleDlg::SetCurrentInputLine(int iLine)
{
	/* Get the current Mixer Device */
	int iMixer = GetCurrentMixerDevice();
	/* Set the current Mixer Device in the registry */
	SetCurrentMixerDevice(iMixer);

	/* Get the Source Line ID of the selected line */
	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(iMixer);
	unsigned int SrcID;
	if (!md || !md->GetInputLineSrcID(&SrcID, iLine))
		return;

	/* Set the Source Line ID in the registry */
	const char * MixerName = m_AudioInput->GetMixerDeviceName(iMixer);
	::SetCurrentInputLine(MixerName, SrcID);
}

CWnd * CSppConsoleDlg::GetCheckAudioCtrl()
{
	return GetDlgItem(IDC_ENABLE_AUDIO);
}


void CSppConsoleDlg::CheckAudioCB()
{
		CButton* AudioCB = (CButton*)GetDlgItem(IDC_ENABLE_AUDIO);
		AudioCB->SetCheck(m_enable_audio);
}


/*
	Test location of mouse - enable disable Tx audio according to location
*/
BOOL CSppConsoleDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	static bool inSpkPic=false;

	if (pWnd == m_pSpkPic && !inSpkPic)
	{
		Beep(3000,10);		/* Short beep to mark mouse entering the area */
		inSpkPic = true;	/* In the area of the picture of the speaker */
	}
	else if (pWnd != m_pSpkPic && inSpkPic)
		inSpkPic = false;	/* Out of the area of the picture of the speaker */

	/* Hear/Mute the Tx */
	HearTx(inSpkPic);

	BOOL res = CDialog::OnSetCursor(pWnd, nHitTest, message);
	return res;
}

/*
	Hear/Mute the Tx
	If true:
		Unmute the Input Line			(save status before doing so)
		Unmute the speakers				(save status before doing so)
	If false:
		Restore status of Input Line	(To the value before unmute)
		Restore status of Speakers		(To the value before unmute)
*/
void CSppConsoleDlg::HearTx(bool hear)
{
	static bool PrevHearValue=false, PrevLineMute;
	bool ddd;

	/* If nothing changed then NOP */
	if (hear == PrevHearValue)
		return;

	if (m_iSelMixer<0 || m_iSelLine<0)
		return;
	/* Unmute the selected Input Line */
	CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(m_iSelMixer);
	if (!md)
		return;

	/* If changed to hear=true then temporarily unmute all */
	if (hear)
	{
		PrevLineMute = md->MuteSelectedInputLine(m_iSelLine, false, true);
		md->SetSpeakers(false,false);
	}

	/* If changed to hear=false then restore mute values */
	else
	{
		md->MuteSelectedInputLine(m_iSelLine, PrevLineMute, true);
		md->SetSpeakers(true);
	};

		ddd = PrevLineMute;

	/* save current value and exit */
	PrevHearValue=hear;
	return;
}

void CSppConsoleDlg::OnSelSrc() 
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	int errorcode;
	CString Msg;

	// Launch Recording Control (sndvol32 -r)		
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);

	OnHide();
	if (!CreateProcess(NULL,"sndvol32.exe -r", NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		Msg.Format( "Cannot start application - error code:%d\nFile \"sndvol32.exe\" is probably missing",errorcode);
		MessageBox(Msg, NULL , MB_ICONERROR);
		OnShow();
		return;
	};
	

	// Wait until exit
	DWORD rc = WaitForSingleObject(ProcessInformation.hProcess, INFINITE);	
	OnShow();

	/* Get the line name list box */
	CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
	if (!InputLineNameList)
		return;

	// Update values
	unsigned int iLine;
	bool res = GetCurrentInputLineFromSystem(&iLine);
	if (res)
	{
		m_iSelLine = iLine;
		InputLineNameList->SetCurSel(m_iSelLine);
		SetCurrentInputLine(m_iSelLine);
	};
}

// Gray menu item (Audio Source) if sndvol32.exe does not exist
//DEL void CSppConsoleDlg::OnUpdateSelSrc(CCmdUI* pCmdUI) 
//DEL {
//DEL }

void CSppConsoleDlg::OnAudioDst() 
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	int errorcode;
	CString Msg;

	// Launch Volume Control (sndvol32)
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);

	OnHide();
	if (!CreateProcess(NULL,"sndvol32.exe", NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		Msg.Format( "Cannot start application - error code:%d\nFile \"sndvol32.exe\" is probably missing",errorcode);
		MessageBox(Msg, NULL , MB_ICONERROR);
		OnShow();
		return;
	};

	// Wait until exit
	DWORD rc = WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
	OnShow();

	/* Get the line name list box */
	CListBox* InputLineNameList = (CListBox*)GetDlgItem(IDC_AUDIO_SRC);
	if (!InputLineNameList)
		return;

	// Update values
	unsigned int iLine;
	bool res = GetCurrentInputLineFromSystem(&iLine);
	if (res)
	{
		m_iSelLine = iLine;
		InputLineNameList->SetCurSel(m_iSelLine);
		SetCurrentInputLine(m_iSelLine);
	};	
}


/*
	Menu item selected - Set Default Audio values
*/
void CSppConsoleDlg::OnAudioVol() 
{
	/* Hide main dialog box */
	OnHide();
	/* Start dialog */
	CDefVolumeDlg dlg;
	INT_PTR nResponse = dlg.DoModal();

	/* Use new value to update volume */
	OnSelchangeAudioSrc();

	/* Restore main dialog box */
	OnShow();
}

/*
	Menu item selected - Run audio info gathering routine
*/
void CSppConsoleDlg::OnAudioPrint() 
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	int errorcode;
	CString Msg;
	DWORD ExitCode;

	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);

	if (!CreateProcess(NULL,"AudioStudy.exe -m", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		Msg.Format( "Cannot start application - error code:%d\nFile \"AudioStudy.exe\" is probably missing",errorcode);
		MessageBox(Msg, NULL , MB_ICONERROR);
		return;
	};

	// Wait until exit
	DWORD rc = WaitForSingleObject(ProcessInformation.hProcess, 3000);

	// Success?
	GetExitCodeProcess(ProcessInformation.hProcess, &ExitCode);
	if (ExitCode)
		MessageBox("Output file is: AudioStatus.txt\n It is on your Desktop");
	else
		MessageBox("Cannot Create output file", NULL, MB_ICONERROR);

}

/*
	Initialize the PPJoy extension interface
*/
void CSppConsoleDlg::PpJoyExShowCtrl()
{
	CButton * PpJoyBtn = (CButton *)GetDlgItem(IDC_PPJOYEX);
	PpJoyBtn->SetCheck(::GetCurrentPpjoyState()); // Check/Uncheck control
	PpJoyBtn->ShowWindow(SW_SHOW); // Show control
}

/* PPJoy extension checked/unchecked */
void CSppConsoleDlg::OnPpjoyex() 
{
	CString Msg;

	// Get the button
	CButton * PpJoyBtn = (CButton *)GetDlgItem(IDC_PPJOYEX);
	if (!PpJoyBtn)
		return;

	// 3.3.1 - This function is relevant only with PPJoy
	UINT uStyle = PpJoyBtn->GetStyle();
	if (!(uStyle & WS_VISIBLE))
		return;

	/* Checed -> Start the PPJoy Interface ; Unchecked -> Stop it */
	int checked = PpJoyBtn->GetCheck();

	if (checked)
	{
			StartPPJoyInterface(1);
			m_Timer1 = SetTimer(TIMER_PPJ,200,(TIMERPROC)NULL);
	}
	else
	{
			StopPPJoyInterface(1);
			KillTimer(m_Timer1);
	};


	// Set the check value to the registry
	::SetCurrentPpjoyState(checked);
}

/*
	Create/update filter menu
*/
void CSppConsoleDlg::UpdateFilterMenu()
{
	int nFilters;
	
	// Initialize the filter object and get number of filters
	if (!m_pFilters)
		m_pFilters = new CFilterIf();
	if (m_pFilters && m_pFilters->Init())
		nFilters = m_pFilters->GetNumberOfFilters();
	else
		return;

	if (nFilters<1)
		return;

	CString itemStr;
	int i=0, location;
	while (int size = m_pMainMenu->GetMenuString(i, itemStr, MF_BYPOSITION))
	{
		if (!itemStr.Find("&Filter"))
		{
			location = i+1;
			break;
		};
		i++;
		location = 0;
	};


	/* Create the filter menu to attach - mark selected item (if any) */
	CMenu * pFilterMenu = new CMenu;
	m_iSelFilter = m_pFilters->GetIndexOfSelected();
	int res = pFilterMenu->CreatePopupMenu( );
	for (int index = 0; index<nFilters; index++)
	{
		CString item = m_pFilters->GetFilterName(index);
		if (m_iSelFilter == index)
			pFilterMenu->AppendMenu(MF_STRING|MF_CHECKED, IDC_FILTER_0+index, item);
		else
			pFilterMenu->AppendMenu(MF_STRING|MF_UNCHECKED, IDC_FILTER_0+index, item);
	};

	/* If does not exist, create as third  and attach the filter menu */
	if (!location)
	{
		if (i<2)
			location = i+1;
		else
			location = 2;

		int res = m_pMainMenu->InsertMenu(location, MF_BYPOSITION|MF_STRING|MF_POPUP , (UINT_PTR) pFilterMenu->GetSafeHmenu(),  "&Filter");
		if (!res)
			return;

		SetMenu(m_pMainMenu);
		m_hFilterMenu = pFilterMenu->Detach();
		delete (pFilterMenu);
	}

	/* Set the selected item */
	int p = m_pFilters->SetSelected(m_iSelFilter);
}

//DEL void CSppConsoleDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
//DEL {
//DEL 	CDialog::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
//DEL 	
//DEL 	if (bSysMenu == TRUE && m_FilterMenuIndex == nIndex)
//DEL 	{
//DEL 		int i =0;
//DEL 	};
//DEL 
//DEL }

//DEL void CSppConsoleDlg::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
//DEL {
//DEL 	CDialog::OnMenuSelect(nItemID, nFlags, hSysMenu);
//DEL 	int id=0;
//DEL 	
//DEL 	if ((nFlags&MF_SYSMENU != MF_SYSMENU) && (nFlags&MF_POPUP != MF_POPUP) && (nItemID>=IDC_FILTER_0) && (nItemID<=IDC_FILTER_20))
//DEL 	{
//DEL 		/* Filter menu */
//DEL 		id = nItemID;
//DEL 	};
//DEL 
//DEL 	if (!hSysMenu && nFlags==0xFFFF)
//DEL 	{
//DEL 		id = 0;
//DEL 	};
//DEL }

/*
	One of the items in the filters menu were selected
*/
void CSppConsoleDlg::OnFilterSelect(UINT nID)
{
	int offset = nID - IDC_FILTER_0;
	CString FilterName;

	/* Get the menu */
	CMenu FilterMenu;
	BOOL attached = FilterMenu.Attach(m_hFilterMenu);
	if (!attached)
		return;

	/* Get the selected entry's status - is it checked ?*/
	UINT stat = FilterMenu.GetMenuState(nID, MF_BYCOMMAND);
	if (stat&MF_CHECKED)
	{/* Checked:*/
		m_iSelFilter = -1;/* Clear index of selected filter (nothing selected )*/
		FilterMenu.CheckMenuItem(nID, MF_BYCOMMAND|MF_UNCHECKED); /* Uncheck */
		FilterName = "";
	}
	else
	{/* Not checked:*/
		if (m_iSelFilter>=0)
			FilterMenu.CheckMenuItem(IDC_FILTER_0+m_iSelFilter, MF_BYCOMMAND|MF_UNCHECKED); /* Clear the checked entry */
		m_iSelFilter = offset; /* Set the index of selected filter */
		FilterMenu.CheckMenuItem(nID, MF_BYCOMMAND|MF_CHECKED); /* Check */
		FilterMenu.GetMenuString(nID, FilterName,MF_BYCOMMAND);
	};

	/* Set selected filter */
	if (m_pFilters)
		m_pFilters->SetSelected(m_iSelFilter);
	SetSelectedFilter(FilterName);

	/* Free the menu */
	FilterMenu.Detach();
}

LRESULT CSppConsoleDlg::OnHideMsg(WPARAM wParam , LPARAM lParam)
{
	OnHide();
	return 1;
}

// Sets the initial position of the dialog box
// First try to get co-ordinates from the registry and set the dialog box according to it
// Then if no such values - set the dialog box at the lower part of the display
int CSppConsoleDlg::LocateDialogWindow(void)
{
	int x,y;
	/***  Get data from the registry ***/
	if (!GetLocationDialogWindow(&x,&y))
	{	// No data in registry
		/*** Calculate the location wher the dialog box has to be ***/
		RECT DialogRect,  WorkRect;
		// Get the size of the dialog box & the work area of desktop
		GetWindowRect(&DialogRect);
		SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkRect, 0);
		// Place the dialog box in the middle just above the bottom
		x = (WorkRect.right+WorkRect.left-DialogRect.right)/2;
		y = (WorkRect.bottom - (DialogRect.bottom-DialogRect.top));
	};

	SetWindowPos(&CWnd::wndTopMost, x, y,0,0,SWP_NOSIZE |SWP_NOACTIVATE);
	return 0;
}

// Store current location of top/left corner of SppConsole dialog box
int CSppConsoleDlg::StoreLocationDialogWindow(void)
{
	RECT DialogRect;

	GetWindowRect(&DialogRect);
	SetLocationDialogWindow(DialogRect.left,DialogRect.top);
	return 0;
}


void CSppConsoleDlg::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == TIMER_PPJ)
		TestPPJoyInterface(1);
	else if (nIDEvent == TIMER_LEVEL)
	{
		SppGetAudioLevel(&m_AudioLevel);
		m_AudioLevelBar.SetPos(m_AudioLevel);

		
	}
	else if (nIDEvent == TIMER_FMS_STAT)
	{
		CString msg;
		int Status;
		// FMS Status
		Status = GetFmsStatusString(msg);
		GetDlgItem(IDC_FMS_STATUS)->SetWindowText(msg);
		// TODO - Print data into file
	}
	else if (nIDEvent == TIMER_JOY)
	{
		// Get joystick position data  and display it
		if (SppGetFilteredPos(m_JoystickPos))
			SetJoyPosBars(m_JoystickPos);
		else
			SetJoyPosBars(NULL);
	};

	CDialog::OnTimer(nIDEvent);
}

// Initialize the joystick position bars
void CSppConsoleDlg::InitJoyPosBars()
{
	for (int i=0 ; i<N_JOY_BARS; i++) m_JoyPosBar[i].SetRange(0,1500);							// Set the range of the joystick positions

	// Set the validity LED
	// Blinking green LED - show (off state)
	CWnd *pWndRed = (CWnd *)GetDlgItem(IDC_LED_POS);
	m_LedValidPos.InitLed(pWndRed,ID_LED_RED, ID_SHAPE_ROUND);
	pWndRed->ShowWindow(SW_SHOW);
	m_LedValidPos.SwitchOff();
}

// Set the joystick position bars
// The input is an array of integers.
// position[0] is the number of positions
// position[1] and up are the values
// If position is NULL - clear all bars
void CSppConsoleDlg::SetJoyPosBars(int * position)
{
	int nValidPos;
	static int * pPrevPos = (int *)0x12345;
	bool Refresh;

	if (position != pPrevPos)
	{
		pPrevPos = position;
		Refresh = true;
	}
	else
		Refresh = false;


	if (position)
	{
		nValidPos = position[0];
		//m_LedValidPos.Blink(100);
		if (Refresh) m_LedValidPos.SwitchOn();
	}
	else 
	{
		nValidPos = 0;
		if (Refresh) m_LedValidPos.SwitchOff();
	};

	if (nValidPos > N_JOY_BARS) nValidPos =N_JOY_BARS; // Limit to the actual number of bars
	for (int i=0; i<nValidPos; i++) m_JoyPosBar[i].SetPos(position[i+1]); // Set bar values
	for (int j=N_JOY_BARS-1; j>=nValidPos; j--) m_JoyPosBar[j].SetPos(0); // Clear unused bars
}

// Start the Input audio wave dialog box
void CSppConsoleDlg::OnWaveRec()
{
	if (!m_Wave2Joystick)
		return;

	if (!m_RecWaveDialog)
	{
		m_RecWaveDialog = new CWaveRec();
		BOOL created = m_RecWaveDialog->Create(IDD_REC_DIALOG, this);
	};
	BOOL Shown = m_RecWaveDialog->ShowWindow(SW_SHOW);
}		   

// Message handler - Wave recording started
LRESULT CSppConsoleDlg::OnWaveRecStart(WPARAM wParam, LPARAM lParam)
{
	// Prohibit changing sound card while recording
	GetDlgItem(IDC_MIXERDEVICE)->EnableWindow(false);
	GetDlgItem(IDC_ENABLE_AUDIO)->EnableWindow(false);
	GetDlgItem(IDC_AUDIO_SRC)->EnableWindow(false);
	//m_WaveRecording = true;
	return NULL;
}

// Message handler - Wave recording stopped
LRESULT CSppConsoleDlg::OnWaveRecStop(WPARAM wParam, LPARAM lParam)
{
	// Allow changing sound card when recording stop
	GetDlgItem(IDC_MIXERDEVICE)->EnableWindow(true);
	GetDlgItem(IDC_ENABLE_AUDIO)->EnableWindow(true);
	GetDlgItem(IDC_AUDIO_SRC)->EnableWindow(true);
	//m_WaveRecording = false;
	return NULL;
}

// Message handler - Wave recording dialog box destroyed
LRESULT CSppConsoleDlg::OnWaveRecCancel(WPARAM wParam, LPARAM lParam)
{
	if (m_RecWaveDialog)
	{
		//free(m_RecWaveDialog);
		m_RecWaveDialog = NULL;
	};
	//m_WaveRecording = false;
	return NULL;
}

// Start/stop Anti-jitter 
void CSppConsoleDlg::OnAdvancedJitter()
{
	// Toggle jitter status
	UpdateJitterStatus(TOGGLE);
	return;


}

// Initialize anti-jitter status according to registry
void CSppConsoleDlg::InitJitterStatus(void)
{

	// Get data fro registry
	int RawData = GetAntiJitterState();
	if (!RawData)
		m_AntiJitter = false;
	else
		m_AntiJitter = true;

	// If registry uninitialized - do it now (Default: ON);
	if (RawData<0)
		SetAntiJitterState(1);

	UpdateJitterStatus(m_AntiJitter);
}

void CSppConsoleDlg::UpdateJitterStatus(int NewStat, BOOL Enable)
{	
	CMenu  Advanced;
	CString itemStr;
	HMENU hAdvanced;
	int i=0;

	// Search for menu item 'Advanced'
	while (int size = m_pMainMenu->GetMenuString(i, itemStr, MF_BYPOSITION))
	{
		if (!itemStr.Find("&Advanced"))
		{
			hAdvanced = m_pMainMenu->GetSubMenu(i)->GetSafeHmenu();
			BOOL Attached = Advanced.Attach(hAdvanced);
			break;
		};
		i++;
	};
	if (!Advanced)
		return;

	// Get the 'Anti-Jitter' item
	UINT stat = Advanced.GetMenuState(IDC_ADVANCED_JITTER, MF_BYCOMMAND);
	if (stat == 0xFFFFFFFF)
		return;

	// Enable/Disable menu item
	if (Enable)
		Advanced.EnableMenuItem(IDC_ADVANCED_JITTER, MF_BYCOMMAND|MF_ENABLED);
	else
		Advanced.EnableMenuItem(IDC_ADVANCED_JITTER, MF_BYCOMMAND|MF_GRAYED);

	// Case: Toggle check sign
	if (NewStat==TOGGLE)
	{
		if (stat&MF_CHECKED)
			NewStat = 0;
		else
			NewStat = 1;
	};

	// Case: set/unset check sign
	if (NewStat!=DONT_CHANGE)
	{
		// Check/uncheck item
		if (NewStat)
		{
			m_AntiJitter = true;
			SetAntiJitterState(1);
			stat = Advanced.CheckMenuItem(IDC_ADVANCED_JITTER, MF_BYCOMMAND|MF_CHECKED);	/* Check */
		}
		else
		{
			m_AntiJitter = false;
			SetAntiJitterState(0);
			stat = Advanced.CheckMenuItem(IDC_ADVANCED_JITTER, MF_BYCOMMAND|MF_UNCHECKED);	/* Uncheck */
		};
	};

	SetMenu(m_pMainMenu);
	Advanced.Detach();

	// Enable/Disable jitter
	SppSetAntiJitter(m_AntiJitter);

}

void CSppConsoleDlg::UpdateFmsMenuItem(int enable)
{
	CMenu	FileMenu;
	HMENU	hFileMenu;
	TCHAR	Path[MAX_PATH+1];
			
	/* Get menu item 'Run FMS' (ID_FILE_RUNFMS) and deactivate it by default */
	hFileMenu = m_pMainMenu->GetSubMenu(0)->GetSafeHmenu();
	BOOL Attached = FileMenu.Attach(hFileMenu);
	FileMenu.EnableMenuItem(ID_FILE_RUNFMS, MF_BYCOMMAND|MF_GRAYED);

	/* Explicit request to Enable */
	if (enable)
	{
		/* Test if FMS exists and ready to be used */
		int FmsStat = GetFmsFullPath(Path,  MAX_PATH);
		if (FmsStat == 1)
			FileMenu.EnableMenuItem(ID_FILE_RUNFMS, MF_BYCOMMAND|MF_ENABLED);
	};

	/* Complete */
	SetMenu(m_pMainMenu);
	FileMenu.Detach();
}

void CSppConsoleDlg::OnRunFms()
{
	TCHAR	Path[MAX_PATH+1];
	int FmsStat = GetFmsFullPath(Path,  MAX_PATH);/* Test if FMS exists and ready to be used */
	if (FmsStat != 1)
		return;

	/* Run FMS */
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	if (!CreateProcess(NULL,Path, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		int errorcode;
		CString Msg;
		errorcode = GetLastError();
		Msg.Format( "Cannot start application - error code:%d\nFile \"%s\" is probably missing",errorcode, Path);
		MessageBox(Msg, NULL , MB_ICONERROR);
		return;
	};
}
void CSppConsoleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if (nID == SC_MINIMIZE)
		OnHide();
	else
		CDialog::OnSysCommand(nID, lParam);
}

// Log audio headers
void CSppConsoleDlg::OnLogAudioHdrs()
{
	if (!m_Wave2Joystick)
		return;

	if (!m_LogAudioHdrsDialog)
	{
		m_LogAudioHdrsDialog = new CLogAudioHdrs();
		BOOL created = m_LogAudioHdrsDialog->Create(IDD_AUDIO_HDR_DIALOG, this);
	}
	BOOL Shown = m_LogAudioHdrsDialog->ShowWindow(SW_SHOW);
	return;
}

// Message handler - Wave recording dialog box destroyed
LRESULT CSppConsoleDlg::OnLogAudioHdrsCancel(WPARAM wParam, LPARAM lParam)
{
	if (m_LogAudioHdrsDialog)
	{
		m_LogAudioHdrsDialog = NULL;
	};
	return NULL;
}

// Log pulses
void CSppConsoleDlg::OnLogPulse()
{
		if (!m_Wave2Joystick)
		return;

	if (!m_LogPulseDialog)
	{
		m_LogPulseDialog = new CLogPulse();
		BOOL created = m_LogPulseDialog->Create(IDD_AUDIO_HDR_DIALOG, this);
		m_LogPulseDialog->SetWindowText("Joystick & Pulse Logger");
		WINDOWINFO wi;
		wi.cbSize = sizeof(WINDOWINFO);
		m_LogPulseDialog->GetWindowInfo(&wi);
		m_LogPulseDialog->MoveWindow(wi.rcWindow.left+40, wi.rcWindow.top+40, wi.rcWindow.right-wi.rcWindow.left, wi.rcWindow.bottom-wi.rcWindow.top);
	}
	BOOL Shown = m_LogPulseDialog->ShowWindow(SW_SHOW);
	return;
}

// Message handler - Wave recording dialog box destroyed
LRESULT CSppConsoleDlg::OnLogPulseCancel(WPARAM wParam, LPARAM lParam)
{
	if (m_LogPulseDialog)
	{
		//free(m_LogPulseDialog);
		m_LogPulseDialog = NULL;
	};
	return NULL;
}

// Log FMS Connection
void CSppConsoleDlg::OnLogFmsConn()
{
	if (!m_LogFmsConnDialog)
	{
		m_LogFmsConnDialog = new CLogFmsConn();
		BOOL created = m_LogFmsConnDialog->Create(IDD_AUDIO_HDR_DIALOG, this);
		m_LogFmsConnDialog->SetWindowText("FMS Connection Logger");
		WINDOWINFO wi;
		wi.cbSize = sizeof(WINDOWINFO);
		m_LogFmsConnDialog->GetWindowInfo(&wi);
		m_LogFmsConnDialog->MoveWindow(wi.rcWindow.left+80, wi.rcWindow.top+80, wi.rcWindow.right-wi.rcWindow.left, wi.rcWindow.bottom-wi.rcWindow.top);
	}
	BOOL Shown = m_LogFmsConnDialog->ShowWindow(SW_SHOW);
	return;
}

LRESULT CSppConsoleDlg::OnFmsConnStart(WPARAM wParam, LPARAM lParam)
{
	// Is it OK to start logging?
	if (m_FmsConnLogStat != Idle)
		return NULL;

	// Prohibit changing sound card while recording
	GetDlgItem(IDC_MIXERDEVICE)->EnableWindow(false);
	GetDlgItem(IDC_ENABLE_AUDIO)->EnableWindow(false);
	GetDlgItem(IDC_AUDIO_SRC)->EnableWindow(false);


	// Open output file, write header and flag ready
	TCHAR WaveFileName[MAX_PATH], CurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, CurrentDir);
	sprintf(WaveFileName,"%s\\FmsConnecion.Log", CurrentDir);

	// Open file for writing
	m_pFmsConnLogFile = fopen(WaveFileName, "w");
	if (!m_pFmsConnLogFile)
	{
		OnFmsConnStop(wParam, lParam);
		m_FmsConnLogStat = Idle;
		return NULL;
	};

	// Print header
	time_t aclock;
	time( &aclock );  
	fprintf(m_pFmsConnLogFile,"+======================================================================================================\n");
	fprintf(m_pFmsConnLogFile,"+ FMS Joystick-connection information - Started at %s", _tasctime(localtime( &aclock )));
	fprintf(m_pFmsConnLogFile,"+\n");
	fprintf(m_pFmsConnLogFile,"+ Time:		Time the line was printed\n");
	fprintf(m_pFmsConnLogFile,"+ Status:	Connection status (Code & message)\n");
	fprintf(m_pFmsConnLogFile,"+ T send:	Timestamp (mSec) of the most recent joystick data that was sent to FMS\n");
	fprintf(m_pFmsConnLogFile,"+ T fms:	Timestamp (mSec) of the most recent joystick data that was read by FMS\n");
	fprintf(m_pFmsConnLogFile,"+ Latency:	Delta between 'T send' & 'Tfms' - Zero is optimal. >300 is OK\n");
	fprintf(m_pFmsConnLogFile,"+\n");
	fprintf(m_pFmsConnLogFile,"%s\t\t\t%-35s\t\t%s\t%s\t%s\n","Time", "Status", "T send ", "T fms", "Latency");
	fprintf(m_pFmsConnLogFile,"+======================================================================================================\n");
	fflush(m_pFmsConnLogFile);
	m_FmsConnLogStat = Started;

	return NULL;
}

LRESULT CSppConsoleDlg::OnFmsConnStop(WPARAM wParam, LPARAM lParam)
{	
	// Allow changing sound card when recording stop
	GetDlgItem(IDC_MIXERDEVICE)->EnableWindow(true);
	GetDlgItem(IDC_ENABLE_AUDIO)->EnableWindow(true);
	GetDlgItem(IDC_AUDIO_SRC)->EnableWindow(true);

	// Is it OK to stop logging?
	if (m_FmsConnLogStat == Idle || m_FmsConnLogStat == Finish || m_FmsConnLogStat == Stopping)
		return NULL;

	// If the other thread is printing then change status and wait 100 milisecs
	if (m_FmsConnLogStat == Printing)
	{
		m_FmsConnLogStat = Stopping;
		Sleep(300);
	};

	// It is assumed that by now the printing thread stopped printing
	// If for some reason the no printing was done then the status now is 'Started'
	m_FmsConnLogStat = Finish;

	// Print Footer
	time_t aclock;
	time( &aclock ); 
	fflush(m_pFmsConnLogFile);
	fprintf(m_pFmsConnLogFile,"+======================================================================================================\n");
	fprintf(m_pFmsConnLogFile,"+ FMS Joystick-connection information - Stopped at %s", asctime(localtime( &aclock )));
	fprintf(m_pFmsConnLogFile,"+======================================================================================================\n");
	fclose(m_pFmsConnLogFile);

	// Footer was printed and file closed - time to move status to 'Idle'
	m_FmsConnLogStat = Idle;
	return NULL;
}

// Message handler - Wave recording dialog box destroyed
LRESULT CSppConsoleDlg::OnFmsConnCancel(WPARAM wParam, LPARAM lParam)
{
	if (m_LogFmsConnDialog)
	{
		//free(m_LogFmsConnDialog);
		m_LogFmsConnDialog = NULL;
	};
	return NULL;
}

// Log pulses inside ProcessPulseXXX (Including the name of the function)
void CSppConsoleDlg::OnLogRawPulse()
{
	if (!m_LogRawPulseDialog)
	{
		m_LogRawPulseDialog = new CLogRawPulse();
		BOOL created = m_LogRawPulseDialog->Create(IDD_AUDIO_HDR_DIALOG, this);
		m_LogRawPulseDialog->SetWindowText("Raw Pulse Logger");
		WINDOWINFO wi;
		wi.cbSize = sizeof(WINDOWINFO);
		m_LogRawPulseDialog->GetWindowInfo(&wi);
		m_LogRawPulseDialog->MoveWindow(wi.rcWindow.left+100, wi.rcWindow.top+100, wi.rcWindow.right-wi.rcWindow.left, wi.rcWindow.bottom-wi.rcWindow.top);
	}
	BOOL Shown = m_LogRawPulseDialog->ShowWindow(SW_SHOW);
	return;
}

LRESULT CSppConsoleDlg::OnRawPulseCancel(WPARAM wParam, LPARAM lParam)
{
	if (m_LogRawPulseDialog)
	{
		//free(m_LogRawPulseDialog);
		m_LogRawPulseDialog = NULL;
	}
	return NULL;
}

// Start the 'Scan Input' dialog box
void CSppConsoleDlg::OnBnClickedScandlg()
{
	// Before calling the dialog box - enable selection of audio sources
	CButton* AudioCB = (CButton*)GetDlgItem(IDC_ENABLE_AUDIO);
	int AudioOrigCheckState = AudioCB->GetCheck();
	if (AudioOrigCheckState != BST_CHECKED)
	{
		AudioCB->SetCheck(BST_CHECKED);
		OnEnableAudio();
	};

	CScanInputs ScanInputsDlg(this);
	INT_PTR nResponse = ScanInputsDlg.DoModal();

	// If exit with cancel then restore state of audio sources checkbox
	if (nResponse == IDCANCEL)
	{
		AudioCB->SetCheck(AudioOrigCheckState);
		OnEnableAudio();
	};
}

int CSppConsoleDlg::GetAudioLevel()
{
	int Level;
	BOOL res = SppGetAudioLevel(&Level);
	if (res)
		return Level;
	else
		return -1;
}

int CSppConsoleDlg::GetNumJoystickPos()
{
	if (!SppGetFilteredPos(m_JoystickPos)) return 0;
	if (m_JoystickPos) return m_JoystickPos[0];
	return 0;
}
