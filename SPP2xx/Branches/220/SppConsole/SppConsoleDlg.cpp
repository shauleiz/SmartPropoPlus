// SppConsoleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "shlwapi.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "..\\GlobalData\\GlobalData.h"

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
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSppConsoleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSppConsoleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
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
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_SHOW, OnShow)
	ON_MESSAGE(WM_HIDEGUI, OnHide)
	ON_MESSAGE(WM_GENERAL, OnStatusAreaMessage)
	ON_REGISTERED_MESSAGE(WM_INTERSPPCONSOLE, OnInterSppConsole)
	ON_REGISTERED_MESSAGE(WM_INTERSPPAPPS, OnInterSppApps)
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify)
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


/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg message handlers

BOOL CSppConsoleDlg::OnInitDialog()
{
	/*** Unique Instance ***/
	/* Register a message to communicate between two SPPConsole instances */
	WM_INTERSPPCONSOLE = RegisterWindowMessage(INTERSPPCONSOLE);
	/* Test if another SPPConsole is running */
	HANDLE hMuxex = CreateMutex(NULL, FALSE, MUTEX1);
	DWORD LastError = GetLastError();
	if (LastError == ERROR_ALREADY_EXISTS)
	{	// another instance is already running and the second instance is NOT 
		// launched iconified then Broadcast a message.
		if (!m_Iconified)
			::PostMessage(HWND_BROADCAST, WM_INTERSPPCONSOLE, 0, 0);
		// Quit (No second instance allowed)
		CDialog::OnCancel();
		return FALSE;
	};

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	EnableToolTips();
	
	// Remove button from task bar
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	// Add icon to the task bar (Tray)
	TaskBarAddIcon(IDR_MAINFRAME, m_hIcon, CONSOLE_TT);
	TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_OFF);

	// Alway on top
	SetWindowPos(&CWnd::wndTopMost, 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE |SWP_NOACTIVATE);
	// Dialog box title
	SetWindowText(CONSOLE_TITLE);
	
	CDialog::OnInitDialog();
	
	/* Initialize Modulation related controls */
	PopulateModulation();

	/* Register the Inter-Process message */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);

	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSppConsoleDlg::OnDestroy()
{
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

void CSppConsoleDlg::OnSelchangeAudioSrc() 
{
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
*/
LRESULT CSppConsoleDlg::OnInterSppApps(WPARAM wParam, LPARAM lParam)
{
	/* DLL is stopping */
	if (wParam == MSG_DLLSTOPPING)
	{
		if (m_Iconified)	// Kill application if DLL is dying and dialog box iconified
			CDialog::OnOK();
		else				// Display baloon saying that the DLL is dying
			TaskBarIconBaloon(IDR_MAINFRAME, BALOON_DLL_STOPPED , "");

		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_OFF);
		return 1;
	};
	
	if (wParam == MSG_DLLSTARTING)
	{
		/* Display baloon */
		CString Msg = "\t";

		if (m_Iconified)	// Show an allaborate baloon
			Msg = GetStatusString();
		TaskBarIconBaloon(IDR_MAINFRAME, (LPCTSTR)(Msg) , BALOON_DLL_STARTING);
		TaskBarIconToolTip(IDR_MAINFRAME, CONSOLE_TT_ON);

		/* Update GUI version in the global memory */
		SetGuiDataToGlobalMemory();

		return 1;
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
	if (!isDllActive())
		CDialog::OnOK();
	else
		OnHide();
}

void CSppConsoleDlg::OnCancel()
{
	OnOK();
}

/* Test is Winmm.DLL is (still) active */
bool CSppConsoleDlg::isDllActive()
{
	bool out;

	HANDLE hMuxex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX2);
	if (!hMuxex)
		out = false;
	else
		out = true;

	CloseHandle(hMuxex);
	return out;
}

/*
	Create a textual summary to display in the opening baloon
*/
CString CSppConsoleDlg::GetStatusString()
{
	CString tmp;
	CString Mod="", Auto="", Shift="", Out;
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
	
	Out = '\n'+Mod+'\n'+Auto+'\n'+Shift;
	return Out;
}

/* Tool Tip massage handler */
BOOL CSppConsoleDlg::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
    UINT nID =pNMHDR->idFrom;
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


