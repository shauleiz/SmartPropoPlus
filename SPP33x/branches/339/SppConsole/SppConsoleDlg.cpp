// SppConsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "shlwapi.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "DefVolumeDlg.h"
#include "SmartPropoPlus.h"
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
{
	//{{AFX_DATA_INIT(CSppConsoleDlg)
	m_enable_audio = FALSE; /* Version 3.3.3 - Default is FALSE */
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_AudioInput = NULL;
	m_iSelMixer = -1;
	m_iSelLine = -1;
}

void CSppConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSppConsoleDlg)
	DDX_Control(pDX, IDC_SPK, m_spk);
	DDX_Check(pDX, IDC_ENABLE_AUDIO, m_enable_audio);
	//}}AFX_DATA_MAP
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
	HANDLE hMuxex;
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
	if (hMuxex=OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXCONSOLE))
	{	// another instance is already running and the second instance is NOT 
		// launched iconified then Broadcast a message.
		if (!m_Iconified)
			::PostMessage(HWND_BROADCAST, WM_INTERSPPCONSOLE, 0, 0);
		// Quit (No second instance allowed)
		CDialog::OnCancel();
		return FALSE;
	}
	else
		hMuxex = CreateMutex(NULL, FALSE, MUTXCONSOLE);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	EnableToolTips();
	
	// Remove button from task bar
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	// Add icon to the task bar (Tray)
	TaskBarAddIcon(IDR_MAINFRAME, m_hIcon, CONSOLE_TT);
	if (m_Iconified)
		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_OFF);

	// Alway on top
	SetWindowPos(&CWnd::wndTopMost, 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE |SWP_NOACTIVATE);
	// Dialog box title
	SetWindowText(CONSOLE_TITLE);
	
	/* Initialize Modulation related controls */
	PopulateModulation();

	/* Initialize Audio source related controls */
	m_AudioInput = new CAudioInput();
	PopulateAudioSource();
	m_enable_audio = ::GetCurrentAudioState();
	CheckAudioCB();
	EnableAudio(m_enable_audio);

	/* PPJoy interface */
	m_PpJoyExVer = GetDllVersion(PPJDLL_NAME);	// Get info about the DLL
	m_hPpJoyExDll = NULL;						// Initialize handle to DLL
	PpJoyExShowCtrl();							// Display/Hide check box. Check/uncheck it
	OnPpjoyex();								// Start PPJoy according to checkbox state

	/* Get Filter (JsChPostProc) information */
	DLLFUNC1 pStartJsChPostProc = (DLLFUNC1)GetProcAddress(m_hPpJoyExDll, "StartJsChPostProc");
	if (pStartJsChPostProc)
		(*pStartJsChPostProc)();
		
	
	
	CDialog::OnInitDialog();


	m_pSpkPic = GetDlgItem(IDC_SPK);


	return TRUE;  // return TRUE  unless you set the focus to a control

}

void CSppConsoleDlg::OnDestroy()
{
	/* Clean-up */
	delete(m_AudioInput);

	WinHelp(0L, HELP_QUIT);
	TaskBarDelIcon(IDR_MAINFRAME);
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
	SetActiveMode((LPCTSTR)text);	
	
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
	
	/* Temporary - If Auto-Detect then hide the radio buttons */
	if (sel)
		ShowShiftRB(false);
	else
		ShowShiftRB();

}

void CSppConsoleDlg::OnShiftPos() 
{
	SetPositiveShift(1);	
}


void CSppConsoleDlg::OnShiftNeg() 
{
	SetPositiveShift(0);	
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
			CDialog::OnOK();
		else				// Display baloon saying that the DLL is dying
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_DLL_STOPPED , "");

		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_OFF);

		/* Enable the DONE button */
		OK->EnableWindow(TRUE);
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
		

		/* Update GUI version in the global memory
		SetGuiDataToGlobalMemory(); */

		return 1;
	};

	if (wParam == MSG_DLLPPJSTAT)
	{
		if (!lParam)
		{	// lParam == 0 -> OK
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_PPJ_CNCT);
		} else
		if (lParam == 2)
		{ // Underlying joystick device deleted
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_PPJ_NOTFND );
		} else
		if (lParam == 6)
		{ // The joystick device handle is invalid
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_PPJ_INVHNDL);
		} else
		if (lParam == MSG_DPPJSTAT_DISCN)
		{ // The joystick disconnected (intentially)
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_PPJ_DISCNCT);
		} else
		{
			CString msg;
			msg.Format("PPJoy error: %d", lParam);
			TaskBarIconBaloon(IDR_MAINFRAME, msg );
		};
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

int CSppConsoleDlg::SetMixerSelectionByName(const char * MixerName)
{

	/* Get the Mixer Device Box */
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return CB_ERR;

	return MixerList->FindStringExact(-1, MixerName);
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

void CSppConsoleDlg::OnSelchangeMixerdevice() 
{
	CComboBox* MixerList = (CComboBox*)GetDlgItem(IDC_MIXERDEVICE);
	if (!MixerList)
		return;

	/* Get the selected item */
	int sel = MixerList->GetCurSel( );
	if (sel == CB_ERR)
		return;

	/* Set the selected Mixer Device as current one in the registry */
	SetCurrentMixerDevice(sel);
	PopulateInputLines();
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
	m_enable_audio = !m_enable_audio;

	// Register in registry
	::SetCurrentAudioState(m_enable_audio);

	// Do it
	EnableAudio(m_enable_audio);
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
	if (!enable)
	{
		SelMixer = MixerList->GetCurSel();
		SelLine  = InputLineNameList->GetCurSel();
		m_AudioInput->Restore();
	}
	else
	{
		// If uninitialized then get preset values
		if (SelMixer<0)
			SelMixer = GetCurrentMixerDevice();
		if (SelLine<0)
			SelLine = GetCurrentInputLine();

		// Restore set-up
		MixerList->SetCurSel(SelMixer);
		InputLineNameList->SetCurSel(SelLine);
		CAudioInput::CMixerDevice * md = m_AudioInput->GetMixerDevice(SelMixer);
		if (md && SelLine > -1)
			md->SetSelectedInputLine(SelLine);		
	};
	
}

/*
	Get the last-used mixer device
	First check the registry.
	Then check the current set-up
	Return the index of the current mixer device
*/
int CSppConsoleDlg::GetCurrentMixerDevice()
{
	char * mixer = ::GetCurrentMixerDevice();
	if (mixer)
		return m_AudioInput->GetMixerDeviceIndex(mixer);
	else
		return 0; // Default 
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



	// If DLL connected (Let's assume it is) then 
	// 1. Get the name of the new mixer device
	const char * MixerName = m_AudioInput->GetMixerDeviceName(iMixer);
	// 2. Compare to the name of the current mixer device
	int iCurMixer = m_AudioInput->GetCurrentMixerDevice();
	// 3. If they are identical then NOP
	if (iCurMixer == iMixer)
		return;

	//  A. Enter Wait state (Cursor)
	HANDLE hMixerSwitchEvent = CreateEvent(NULL, FALSE, FALSE, EVENT_MIXER);
	AfxGetApp()->DoWaitCursor(1);

	//  B. Place request on the global memory for the DLL to switch mixer device
	::SwitchMixerRequest(MixerName);

	//  C. Wait for ack from DLL or timeout
	WaitForSingleObject(hMixerSwitchEvent,2000);
	CloseHandle(hMixerSwitchEvent);
	AfxGetApp()->DoWaitCursor(-1);

	// Get the name of the actual device from global memory 
	// Update AudioInput object
	const char * ActualMixerName = ::GetMixerName();
	iMixer = SetMixerSelectionByName(ActualMixerName);


	::SetCurrentMixerDevice(ActualMixerName);
	m_AudioInput->SetCurrentMixerDevice(iMixer);
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
	TCHAR SndVolExe[80] = "sndvol32.exe -r";

	// Launch Recording Control (sndvol32 -r)		
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);

	OnHide();
	if (isVista())
		sprintf(SndVolExe,"rundll32.exe shell32.dll,Control_RunDLL mmsys.cpl,,1");

	if (!CreateProcess(NULL,SndVolExe, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		Msg.Format( "Cannot start application - error code:%d\nFile \"%s\" is probably missing",errorcode,SndVolExe);
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
	TCHAR SndVolExe[280] = "sndvol32.exe";

	// Launch Volume Control (sndvol32)
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);

	OnHide();
	if (isVista())
		sprintf(SndVolExe,"rundll32.exe shell32.dll,Control_RunDLL mmsys.cpl,,0");

	if (!CreateProcess(NULL,SndVolExe, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		Msg.Format( "Cannot start application - error code:%d\nFile \"%s\" is probably missing",errorcode,SndVolExe);
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
		MessageBox("Output file is: AudioStatus.txt\n It is on your Desctop");
	else
		MessageBox("Cannot Create output file", NULL, MB_ICONERROR);

}

/*
	Initialize the PPJoy extension interface
*/
void CSppConsoleDlg::PpJoyExShowCtrl()
{
	CButton * PpJoyBtn = (CButton *)GetDlgItem(IDC_PPJOYEX);

	// File PPJoyEx.dll not found or version incompatible 
	// then do not show relevant control
	if (m_PpJoyExVer < 30000)
		PpJoyBtn->ShowWindow(SW_HIDE);
	else
	{	/* File PPJoyEx.dll is OK */
		PpJoyBtn->SetCheck(::GetCurrentPpjoyState()); // Check/Uncheck control
		PpJoyBtn->ShowWindow(SW_SHOW); // Show control
	};
}

/* PPJoy extension checked/unchecked */
void CSppConsoleDlg::OnPpjoyex() 
{
	DLLFUNC1 pStartPPJoyInterface, pStopPPJoyInterface;
	DWORD errorcode;
	CString Msg;

	// Get the button
	CButton * PpJoyBtn = (CButton *)GetDlgItem(IDC_PPJOYEX);
	if (!PpJoyBtn)
		return;

	// 3.3.1 - This function is relevant only with PPJoy
	UINT uStyle = PpJoyBtn->GetStyle();
	if (!(uStyle & WS_VISIBLE))
		return;


	// Get the handle to the PPJoy DLL
	if (!m_hPpJoyExDll)
		m_hPpJoyExDll= LoadLibrary(PPJDLL_NAME);
	if (!m_hPpJoyExDll)
	{	/* improved error reporting - 3.3.1 */
		errorcode = GetLastError();
		Msg.Format( "Cannot load %s - error code:%d\nFile \"PPJoyEx.dll.exe\" is probably missing",PPJDLL_NAME, errorcode);
		MessageBox(Msg, NULL , MB_ICONERROR);
		return;
	};

	/* Checed -> Start the PPJoy Interface ; Unchecked -> Stop it */
	int checked = PpJoyBtn->GetCheck();
	if (checked)
	{
		pStartPPJoyInterface = (DLLFUNC1)GetProcAddress(m_hPpJoyExDll, "StartPPJoyInterface");
		if (!pStartPPJoyInterface)
		{	/* improved error reporting - 3.3.1 */
			errorcode = GetLastError();
			Msg.Format( "Cannot load function \"StartPPJoyInterface\" - error code:%d\nFile \"PPJoyEx.dll.exe\" is probably corrupted", errorcode);
			MessageBox(Msg, NULL , MB_ICONERROR);
			return;
		}
		(*pStartPPJoyInterface)();
	}
	else
	{
		pStopPPJoyInterface = (DLLFUNC1)GetProcAddress(m_hPpJoyExDll, "StopPPJoyInterface");
		if (!pStopPPJoyInterface)
		{	/* improved error reporting - 3.3.1 */
			errorcode = GetLastError();
			Msg.Format( "Cannot load function \"StopPPJoyInterface\" - error code:%d\nFile \"PPJoyEx.dll.exe\" is probably corrupted", errorcode);
			MessageBox(Msg, NULL , MB_ICONERROR);
			return;
		}
		(*pStopPPJoyInterface)();
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
	
	nFilters = GetNumberOfFilters();
	if (nFilters<1)
		return;

	/* Filter Exists ? */
	CMenu menu;
	if (!menu.LoadMenu(IDR_MENU_MAIN))
		return;

	CString itemStr;
	int i=0, location;
	while (int size = menu.GetMenuString(i, itemStr, MF_BYPOSITION))
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
	m_iSelFilter = GetSelectedFilterIndex();
	int res = pFilterMenu->CreatePopupMenu( );
	for (int index = 0; index<nFilters; index++)
	{
		CString item = GetFilterNameByIndex(index);
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

		int res = menu.InsertMenu(location, MF_BYPOSITION|MF_STRING|MF_POPUP , (UINT_PTR) pFilterMenu->GetSafeHmenu(),  "&Filter");
		if (!res)
			return;

		SetMenu(&menu);
		m_hFilterMenu = pFilterMenu->Detach();
	}

	/* Set the selected item */
	SetSelectedFilterIndex(m_iSelFilter);


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
	}
	else
	{/* Not checked:*/
		if (m_iSelFilter>=0)
			FilterMenu.CheckMenuItem(IDC_FILTER_0+m_iSelFilter, MF_BYCOMMAND|MF_UNCHECKED); /* Clear the checked entry */
		m_iSelFilter = offset; /* Set the index of selected filter */
		FilterMenu.CheckMenuItem(nID, MF_BYCOMMAND|MF_CHECKED); /* Check */
	};

	/* Set selected filter */
//	CString FilterName;
//	FilterMenu.GetMenuString(nID, FilterName,MF_BYCOMMAND);
	SetSelectedFilterIndex(m_iSelFilter);

	/* Free the menu */
	FilterMenu.Detach();
}

LRESULT CSppConsoleDlg::OnHideMsg(WPARAM wParam , LPARAM lParam)
{
	OnHide();
	return 1;
}
