#include "stdafx.h"
#include <gdiplus.h>
#include "Vector"
#include "Windows.h"
#include "Windowsx.h"
#include "SmartPropoPlus.h"
#include "WinMessages.h"
#include "public.h"
#include "Commctrl.h"
#include "resource.h"
#include "vJoyMonitor.h"
#include "SppProcess.h"
#include "SppBtnsDlg.h"
#include "SppDlg.h"

#pragma comment (lib,"Gdiplus.lib")
#pragma comment (lib,"UxTheme.lib")

#define MAX_LOADSTRING 100

// TODO: Remove all the following defines when done with tab development
#define TAB_AUDIO_ON	1
#define TAB_DCDR_ON		1
#define TAB_FLTR_ON		1
#define TAB_JOY_ON		1

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	MsgHndlDlg(HWND, UINT, WPARAM, LPARAM);
DWORD	WINAPI		StartDlg(LPVOID hInstance);

SppDlg::SppDlg(void)
{
}

SppDlg::SppDlg(HINSTANCE hInstance, HWND	ConsoleWnd)
{
	m_hInstance = hInstance;
	m_ConsoleWnd = ConsoleWnd;
	m_StreamingState = true;
	m_hTarget = NULL;

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg, (LPARAM)this);	

	// Add icon to system tray
	m_tnid.cbSize = 0;
	TaskBarAddIcon(IDI_SPPCONSOLE, CONSOLE_TT_DEF, NULL);
	m_color = 0x0;


	return;
}

SppDlg::~SppDlg(void)
{
	// Remove notification icon
	if (m_tnid.cbSize)
		Shell_NotifyIcon(NIM_DELETE, &m_tnid);

	// Cleanup
	delete(m_hrsrc.TabAudio);
	delete(m_hrsrc.TabDcdr);
	delete(m_hrsrc.TabFltr);
	delete(m_hrsrc.TabJoy);
	delete(m_hrsrc.TabAdvnc);
}

void SppDlg::Show()
{
	m_WizMinimized = false;
	ShowWindow(m_hDlg, SW_SHOW);
	SetForegroundWindow(m_hDlg);
	UpdateWindow(m_hDlg);	
}

void SppDlg::Hide()
{
	m_WizMinimized = true;
	ShowWindow(m_hDlg, SW_HIDE);
}

void SppDlg::RegisterEndEvent(HANDLE * h)
{
	m_hEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	*h = m_hEndEvent;
}


DWORD WINAPI StartDlg(LPVOID hInstance)
{
	return (DWORD)DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg);
}

bool SppDlg::MsgLoop(void)
{
	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) 
	{ 
		if (bRet == -1)
		{
			return false;
		}

		else if (!IsWindow(m_hDlg))
			return true;

		else if (!IsDialogMessage(m_hDlg, &msg)) 
		{ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	} 

	return true;
}

void SppDlg::InitBackgroundImage(HWND hDlg)
{
	// Find the location of the center of the tab control
	// Create a rectangle into which the image will fit
	RECT ScreenRc, ClientRc;
	HWND hBase = GetDlgItem(hDlg, IDC_TABS);
	BOOL foundrect = GetWindowRect(hBase, &ScreenRc);
	POINT TopLeft = {ScreenRc.left, ScreenRc.top};
	ScreenToClient(hDlg, &TopLeft);
	ClientRc.left = TopLeft.x+100;
	ClientRc.top = TopLeft.y;
	LONG Width  = ScreenRc.right-ScreenRc.left;
	LONG Height = ScreenRc.bottom-ScreenRc.top;

	// Get the Image
	HICON hIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_BACKGROUND), IMAGE_ICON, 300, 300, NULL);

	// Create static window, load icon and display the icon
	m_hTarget = CreateWindowEx(0, _T("static"), L"", SS_ICON|WS_CHILD, ClientRc.left, ClientRc.top, Width,  Height, hDlg, NULL,	m_hInstance, 0);
	HANDLE	prev = (HANDLE)SendMessage(m_hTarget,STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
	ShowWindow(m_hTarget, SW_HIDE);

	//// Destroy Icon
 //   if (hIcon) 
 //       DestroyIcon(hIcon);
}

// Display/Hide Background image when to tabs are displayed
void SppDlg::DisplayBackgroundImage(BOOL Display)
{
	if (!m_hTarget)
		return;

	if (Display)
		ShowWindow(m_hTarget, SW_SHOW);
	else
		ShowWindow(m_hTarget, SW_HIDE);



}

// TaskBarAddIcon - adds an icon to the notification area. 
// Returns TRUE if successful, or FALSE otherwise. 
// uID - identifier of the icon 
// hicon - handle to the icon to add 
// lpszTip - tooltip text 
bool SppDlg::TaskBarAddIcon(UINT uID, LPTSTR lpszTip, LPTSTR lpszInfo)
{
	bool res = true; 
    NOTIFYICONDATA tnid;
	HRESULT hr;
	int cmd = NIM_ADD;
 
	if (m_tnid.cbSize)
		cmd = NIM_MODIFY ;


 	m_hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(uID));
	tnid.cbSize = sizeof(NOTIFYICONDATA); 
	tnid.hWnd = m_hDlg; 
	tnid.uID = IDI_SPPCONSOLE; 
	tnid.uFlags = NIF_MESSAGE | NIF_ICON; 
	tnid.uCallbackMessage = WMAPP_NOTIFYCALLBACK; 
	tnid.hIcon =	m_hIcon;// Load system tray icon
	tnid.uVersion = NOTIFYICON_VERSION_4;

	LPTSTR lpszTitle = TEXT("SmartPropoPlus Message");
	// Tool Tip
	if (lpszTip)
	{
		hr = StringCbCopyN(tnid.szTip, sizeof(tnid.szTip), lpszTip, sizeof(tnid.szTip));
		tnid.uFlags |= NIF_TIP;
	}
	else 
		tnid.szTip[0] = (TCHAR)'\0';

	// Info text
	if (lpszInfo)
	{
		hr = StringCbCopyN(tnid.szInfo, sizeof(tnid.szInfo), lpszInfo, sizeof(tnid.szInfo));
		hr = StringCbCopyN(tnid.szInfoTitle, sizeof(tnid.szInfoTitle), lpszTitle, sizeof(tnid.szInfoTitle));
		tnid.dwInfoFlags = NIIF_NONE;
		tnid.uFlags |= NIF_INFO;
	}
	else 
		tnid.szInfo[0] = (TCHAR)'\0';

	res = (TRUE == Shell_NotifyIcon(cmd, &tnid));
	if (res)
		m_tnid = tnid;  

 
     //if (m_hIcon) 
     //   DestroyIcon(m_hIcon); 

	//////////////  Icon on the dialog box next to the status text
	// Calculate location of icon - based of hidden control IDI_STATUS
	RECT ScreenRc, ClientRc;
	HWND hBase = GetDlgItem(m_hDlg, IDI_STATUS);
	BOOL foundrect = GetWindowRect(hBase, &ScreenRc);
	POINT TopLeft = {ScreenRc.left, ScreenRc.top};
	ScreenToClient(m_hDlg, &TopLeft);
	ClientRc.left = TopLeft.x;
	ClientRc.top = TopLeft.y;
	LONG Width  = ScreenRc.right-ScreenRc.left;
	LONG Height = ScreenRc.bottom-ScreenRc.top;

	// Create static window, load icon and display the icon
	HWND hTarget = CreateWindowEx(0, _T("static"), L"", SS_ICON|WS_CHILD, ClientRc.left, ClientRc.top, Width,  Height, m_hDlg, NULL,	m_hInstance, 0);
	HANDLE	prev = (HANDLE)SendMessage(hTarget,STM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIcon);
	ShowWindow(hTarget, SW_SHOW);

    if (m_hIcon) 
        DestroyIcon(m_hIcon); 

    return res; 
}


// CU informed GUI that status changed - Update notification icon
void SppDlg::SppStatusChanged( WPARAM wParam, LPARAM lParam)
{
	// Change Icon and status text
	switch (wParam)
	{

	case S0:
		SetWindowText(GetDlgItem(m_hDlg, IDS_STATUS_EDT), CONSOLE_BLN_S0);
		TaskBarAddIcon(IDI_S0, CONSOLE_TT_S0, (LPTSTR)lParam);		
		//SetStreamingState(false);
		break;
	case W7:
		TaskBarAddIcon(IDI_W7, CONSOLE_TT_W7, (LPTSTR)lParam);
		//SetStreamingState(false);
		break;
	case W71:
		TaskBarAddIcon(IDI_W7, CONSOLE_TT_W71, (LPTSTR)lParam);
		//SetStreamingState(false);
		break;
	case W72:
		TaskBarAddIcon(IDI_W7, CONSOLE_TT_W72, (LPTSTR)lParam);
		//SetStreamingState(false);
		break;
	case W8:
		TaskBarAddIcon(IDI_W8, CONSOLE_TT_W8, (LPTSTR)lParam);
		//SetStreamingState(false);
		break;
	case W9:
		TaskBarAddIcon(IDI_W8, CONSOLE_TT_W9, (LPTSTR)lParam);
		//SetStreamingState(false);
		break;
	case W10:
		TaskBarAddIcon(IDI_W10, CONSOLE_TT_W10, (LPTSTR)lParam);
		//SetStreamingState(false);
		break;
	default:
		TaskBarAddIcon(IDI_SPPCONSOLE, CONSOLE_TT_DEF, (LPTSTR)lParam);
		//SetStreamingState(true);
		break;
	}



	if (lParam)
		SetWindowText(GetDlgItem(m_hDlg, IDS_STATUS_EDT), (LPTSTR)lParam);

}

#if 0
// Loads and locks a dialog box template resource. 
// Returns the address of the locked dialog box template resource. 
// lpszResName - name of the resource. 
// Based on: http://msdn.microsoft.com/en-us/library/windows/desktop/hh298366(v=vs.85).aspx
DLGTEMPLATE*  SppDlg::DoLockDlgRes(LPCTSTR lpszResName)
{
	HRSRC hrsrc = FindResource(NULL, lpszResName, RT_DIALOG); 

	// Note that g_hInst is the global instance handle
	HGLOBAL hglb = LoadResource(m_hInstance, hrsrc);  
	return (DLGTEMPLATE *) LockResource(hglb); 

}

#endif // 0

// Handle messages from notification icon
void SppDlg::OnNotificationIcon( WPARAM wParam, LPARAM lParam)
{
	WORD event = LOWORD(lParam);

	// (Double) Left Click - Open Wizard
	if ((event == WM_LBUTTONDOWN) | (event == WM_LBUTTONDBLCLK))
	{
		Show();
		Iconified(false);
	}
	// Right Click - context menu
	if (event == WM_RBUTTONDOWN)
	{
		HMENU hIconMenu = LoadMenu(m_hInstance, MAKEINTRESOURCE(IDM_ICON));
		HMENU hPopupMenu = GetSubMenu(hIconMenu, 0); 

		// Update menu look
		MENUITEMINFO mii;

		// Get the state of the specified menu item. 
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE | MIIM_ID;    // information to get 
		GetMenuItemInfo(hPopupMenu, IDC_STREAM, FALSE, &mii); 

		// Awake check mark
		mii.fState = m_StreamingState ? MFS_CHECKED : 0;
		SetMenuItemInfo(hPopupMenu, IDC_STREAM, FALSE, &mii); 

		// Wizard/Minimized check mark 
		mii.fState = m_WizMinimized ? 0 : MFS_CHECKED;
		// Wizard/Minimized meaning 
		mii.wID = m_WizMinimized ? IDM_DISPLAY_WIZARD : IDC_HIDE;
		SetMenuItemInfo(hPopupMenu, IDM_DISPLAY_WIZARD, FALSE, &mii); 

		POINT pt;
		GetCursorPos(&pt);
		TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, m_hDlg, NULL);
		DestroyMenu(hIconMenu);
	}
}

// Reset was requested
// Reset object-by-object
LRESULT SppDlg::Reset(void)
{
	// Audio
	if (m_hrsrc.TabAudio)
		m_hrsrc.TabAudio->Reset();

	// Decoder
	if (m_hrsrc.TabDcdr)
		m_hrsrc.TabDcdr->Reset();

	// Filter
	if (m_hrsrc.TabFltr)
		m_hrsrc.TabFltr->Reset();

	// Joystick
	if (m_hrsrc.TabJoy)
		m_hrsrc.TabJoy->Reset();

	// Advanced
	if (m_hrsrc.TabAdvnc)
		m_hrsrc.TabAdvnc->Reset();


	return TRUE;
}


// Initialize the tab control
int SppDlg::InitTabs(HWND hDlg)
{
	TCITEM tie;
	HIMAGELIST hImageList;

	// Initialize structure that represents tabs
	m_hrsrc.hwndTab = GetDlgItem(hDlg,  IDC_TABS);
	if (!m_hrsrc.hwndTab) // TODO: Add log
		return 0;
			
	// Fix white background around tab icon
	SetWindowTheme(m_hrsrc.hwndTab, L" ", L" ");

	// Get the tab rectangle
	BOOL GotRect = GetWindowRect(m_hrsrc.hwndTab, &m_hrsrc.rcDisplay);
	if (!GotRect) // TODO: Add log
		return 0;

	// Create child dialogs and tabs
	tie.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM ;
	tie.iImage = -1;

	// Creating dialog boxes that will be associated with tabs.
	//int iTab=0;

	//// General
	//m_hrsrc.TabGen =   new SppTabGen(m_hInstance, m_hrsrc.hwndTab);
	//m_hrsrc.Display = m_hrsrc.TabGen;
	//tie.pszText = TEXT("General");
	//tie.lParam = m_hrsrc.Display->GetId();
 //   TabCtrl_InsertItem(m_hrsrc.hwndTab, iTab++, &tie); 

	// Audio
	m_hrsrc.TabAudio = new SppTabAudio(m_hInstance, m_hrsrc.hwndTab);
	m_hrsrc.Display = m_hrsrc.TabAudio;
	tie.pszText = TEXT("Input");
	tie.lParam = m_hrsrc.Display->GetId();
	tie.iImage = IAUDIO;
    TabCtrl_InsertItem(m_hrsrc.hwndTab, IAUDIO, &tie); 

	// Decoder
	m_hrsrc.TabDcdr = new SppTabDcdr(m_hInstance, m_hrsrc.hwndTab);
	m_hrsrc.Display = m_hrsrc.TabDcdr;
	tie.pszText = TEXT("Transmitter");
	tie.lParam = m_hrsrc.Display->GetId();
	tie.iImage = IDECODER;
    TabCtrl_InsertItem(m_hrsrc.hwndTab, IDECODER, &tie); 

	// Filter
	m_hrsrc.TabFltr = new SppTabFltr(m_hInstance, m_hrsrc.hwndTab);
	m_hrsrc.Display = m_hrsrc.TabFltr;
	tie.pszText = TEXT("Filter");
	tie.lParam = m_hrsrc.Display->GetId();
	tie.iImage = IFILTER;
    TabCtrl_InsertItem(m_hrsrc.hwndTab, IFILTER, &tie); 

	// Joystick
	m_hrsrc.TabJoy = new SppTabJoy(m_hInstance, m_hrsrc.hwndTab);
	m_hrsrc.Display = m_hrsrc.TabJoy;
	tie.pszText = TEXT("Joystick");
	tie.lParam = m_hrsrc.Display->GetId();
	tie.iImage = IJOYSTICK;
    TabCtrl_InsertItem(m_hrsrc.hwndTab, IJOYSTICK, &tie);

	// Advanced
	m_hrsrc.TabAdvnc = new SppTabAdv(m_hInstance, m_hrsrc.hwndTab);
	m_hrsrc.Display = m_hrsrc.TabAdvnc;
	tie.pszText = TEXT("Advanced");
	tie.lParam = m_hrsrc.Display->GetId();
	tie.iImage = IADVANCED ;
    TabCtrl_InsertItem(m_hrsrc.hwndTab, IADVANCED , &tie);


	// Add icons
	TabCtrl_SetPadding(m_hrsrc.hwndTab,10,5);
	hImageList = CreateTabsImageList();
	if (hImageList)
		TabCtrl_SetImageList(m_hrsrc.hwndTab, hImageList);


	// Select the tab (Tab 0)
	TabCtrl_SetCurSel(m_hrsrc.hwndTab,0);
	OnSelChanged(hDlg);

	return 2;
}

// Changed selection of tab control
void  SppDlg::OnSelChanged(HWND hDlg)
{
	// Get the current selection - extract the ID
	int iTab = TabCtrl_GetCurSel(m_hrsrc.hwndTab);
	if (iTab<0)
		return;

	TCITEM tie;
	tie.mask = TCIF_PARAM;
	BOOL sTab = TabCtrl_GetItem(m_hrsrc.hwndTab, iTab, &tie);
	if (!sTab)
		return;

	if (tie.lParam == m_hrsrc.Display->GetId())
		return;

	// Hide all dialog boxes but one - according to ID
	//m_hrsrc.TabGen->Hide();
	m_hrsrc.TabAudio->Hide();
	m_hrsrc.TabDcdr->Hide();
	m_hrsrc.TabFltr->Hide();
	m_hrsrc.TabJoy->Hide();
	m_hrsrc.TabAdvnc->Hide();

	// Show only the selected dialog box
	switch (tie.lParam)
	{
	//case IDD_GENERAL:
	//	m_hrsrc.TabGen->Show();
	//	break;

	case IDD_AUDIO:
		m_hrsrc.TabAudio->Show();
		m_hrsrc.Display = m_hrsrc.TabAudio;
		break;

	case IDD_DECODE:
		m_hrsrc.TabDcdr->Show();
		m_hrsrc.Display = m_hrsrc.TabDcdr;
		break;

	case IDD_FILTER:
		m_hrsrc.TabFltr->Show();
		m_hrsrc.Display = m_hrsrc.TabFltr;
		break;

	case IDD_JOY:
		m_hrsrc.TabJoy->Show();
		m_hrsrc.Display = m_hrsrc.TabJoy;
		break;

	case IDD_ADVANCED:
		m_hrsrc.TabAdvnc->Show();
		m_hrsrc.Display = m_hrsrc.TabAdvnc;
		break;
	};


}

//void  SppDlg::CleanAudioList(void)
//{
//	HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
//	ListView_DeleteAllItems(hAudioList);
//}

//void SppDlg::InitAudioDisplay(HWND hDlg)
//{
//	CleanAudioList();
//
//	// Change style
//	HWND hAudioList = GetDlgItem(hDlg,  IDC_LIST_AUDIOSRC);
//	ListView_SetExtendedListViewStyle(hAudioList, LVS_EX_FULLROWSELECT);
//
//	// Set columns
//	LVCOLUMN lvc;
//	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
//
//	// Add Device colum (0)
//	lvc.iSubItem = 0;
//    lvc.pszText = TEXT("Device");
//    lvc.cx = 250;               // Width of column in pixels.
//	lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.
//	ListView_InsertColumn(hAudioList, 0, &lvc);
//
//	// Add audio level colum (1)
//	lvc.iSubItem = 1;
//    lvc.pszText = TEXT("L/R Levels");
//    lvc.cx = 80;               // Width of column in pixels.
//	lvc.fmt = LVCFMT_CENTER;  // Left-aligned column.
//	ListView_InsertColumn(hAudioList, 1, &lvc);
//}

// Display the audio levels of channels (Left/Right)
// Levels are in the range 0-100
void SppDlg::DisplayAudioLevels(HWND hDlg, PVOID Id, UINT Left, UINT Right)
{

#if TAB_AUDIO_ON
	((SppTabAudio *)m_hrsrc.TabAudio)->DisplayAudioLevels( Id,  Left,  Right);
#else
	HWND hAudioList = GetDlgItem(hDlg,  IDC_LIST_AUDIOSRC);

	wstring str;
	if (Right>100) // If Right channel is over 100 then this is a mono device
		str = to_wstring(Left); // Mono
	else
		str = to_wstring(Left) + L"/" +  to_wstring(Right);

	// Get item index of by Id
	int i = FindItemById(hAudioList, (LPCTSTR)Id);
	if (i<0)
		return;

	// Set text in the format L/R (e.g. 98/3)
	ListView_SetItemText(hAudioList, i , 1, (LPTSTR)str.c_str());
#endif
}

// Find audio item in list view by its id
// Returns index to item if found or -1 if not found
int SppDlg::FindItemById(HWND hListView, LPCTSTR Id)
{
	// Get number of items
	int ItemCount = ListView_GetItemCount(hListView);
	if (ItemCount<1)
		return -1;

	LVITEM lvi;
	lvi.iSubItem=0;
	lvi.mask=LVIF_PARAM;
	for (int i=0; i<ItemCount;i++)
	{
		lvi.iItem = i;
		ListView_GetItem(hListView, &lvi);
		if (!lstrcmpi((LPCTSTR)lvi.lParam, Id))
			return i;
	}; // For loop

	// Not found
	return -1;
}

// Update the position of the progress bar that corresponds to the channel
void  SppDlg::SetRawChData(UINT iCh, UINT data)
{
#if TAB_FLTR_ON
	((SppTabFltr *)m_hrsrc.TabFltr)->SetRawChData( iCh,  data);
#endif

#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetRawChData( iCh,  data);
#endif

#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->SetRawChData( iCh,  data);
#else
#endif

	// Check if this channel is supported
	if (iCh > (IDC_CH8-IDC_CH1))
		return;

	HWND hCh = GetDlgItem(m_hDlg,  IDC_CH1+iCh);
	SendMessage(hCh, PBM_SETPOS, data, 0);
}
void SppDlg::SetNumberProcCh(UINT nCh)
{
	((SppTabFltr *)m_hrsrc.TabFltr)->SetNumberProcCh( nCh);
	((SppTabJoy *)m_hrsrc.TabJoy)->SetNumberProcCh( nCh);
	// Show only existing channels
	for (UINT iCh=0; iCh<(IDC_CHPP8-IDC_CHPP1+1); iCh++)
	{
		HWND hCh = GetDlgItem(m_hDlg,  IDC_CHPP1+iCh);
		if (iCh<nCh)
			ShowWindow(hCh, SW_SHOW);
		else
			ShowWindow(hCh, SW_HIDE);
		UpdateWindow(hCh);
	};

}

// Update the number of raw channels
void SppDlg::SetNumberRawCh(UINT nCh)
{	
#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->SetNumberRawCh(nCh);
#endif	
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetNumberRawCh(nCh);
#endif	
	static UINT prevVal=100;

	// Prevent flicker
	if (prevVal == nCh)
		return;

	Edit_SetText(GetDlgItem(m_hDlg,IDS_DECODER_NCH), (to_wstring(nCh)+L" Channels").data());
	prevVal = nCh;

	// Update text of static frame
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS);
	wstring txt = L"Raw Channels (" + to_wstring(nCh) + L")";
	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());

	// Show only existing channels
	for (UINT iCh=0; iCh<(IDC_CH8-IDC_CH1+1); iCh++)
	{
		HWND hCh = GetDlgItem(m_hDlg,  IDC_CH1+iCh);
		if (iCh<nCh)
			ShowWindow(hCh, SW_SHOW);
		else
			ShowWindow(hCh, SW_HIDE);
		UpdateWindow(hCh);
	};
}

// Update the decoder quality value in the GUI
void SppDlg::SetDecoderQuality(UINT Quality)
{
#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->SetDecoderQuality(Quality);
#else
	static UINT prevVal=100;

	// Prevent flicker
	if (prevVal == Quality)
		return;
	prevVal = Quality;

	// Update text of static frame
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_SIG_DEC);
	wstring txt = L"Signal Decoder ( Quality: " + to_wstring(Quality) + L")";
	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());
#endif
}

// Update the position of the progress bar that corresponds to the channel
void  SppDlg::SetProcessedChData(UINT iCh, UINT data)
{
#if TAB_FLTR_ON
	((SppTabFltr *)m_hrsrc.TabFltr)->SetProcessedChData(iCh, data);
#endif
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetProcessedChData(iCh, data);
#else
#endif

	// Check if this channel is supported
	if (iCh > (IDC_CHPP8-IDC_CHPP1))
		return;

	HWND hCh = GetDlgItem(m_hDlg,  IDC_CHPP1+iCh);
	SendMessage(hCh, PBM_SETPOS, data, 0);

}

// Update the frame text of the vJoy device vJoy axes
void SppDlg::SetJoystickDevFrame(UCHAR iDev)
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetJoystickDevFrame( iDev);
#else
#endif

	static UINT id=100;
	wstring txt;

	if (id == iDev)
		return;
	
	id = iDev;

	HWND hFrame = GetDlgItem(m_hDlg,  IDS_JOY);
	if (id)
		txt = L"vJoy device " + to_wstring(iDev);
	else
		txt = L"vJoy device is missing";

	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());
}

void SppDlg::SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue)
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetJoystickBtnData( iDev,  BtnValue);
#else
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_JMON_BTN, iDev, (LPARAM)BtnValue);
#endif
}


// Update the position of the  progress bar that corresponds to the vJoy axis
void SppDlg::SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue)
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetJoystickAxisData( iDev,  Axis,  AxisValue);
#else
#endif
	int IdItem;

	switch (Axis)
	{
	case HID_USAGE_X:
		IdItem = IDC_X;
		break;
	case HID_USAGE_Y:
		IdItem = IDC_Y;
		break;
	case HID_USAGE_Z:
		IdItem = IDC_Z;
		break;
	case HID_USAGE_RX:
		IdItem = IDC_RX;
		break;
	case HID_USAGE_RY:
		IdItem = IDC_RY;
		break;
	case HID_USAGE_RZ:
		IdItem = IDC_RZ;
		break;
	case HID_USAGE_SL0:
		IdItem = IDC_SL0;
		break;
	case HID_USAGE_SL1:
		IdItem = IDC_SL1;
		break;

	default:
		return;
	};

	HWND hCh = GetDlgItem(m_hDlg, IdItem);
	SendMessage(hCh, PBM_SETPOS, AxisValue, 0);
}

void SppDlg::EnableFilter(BOOL cb)
{
	// Get check state
	HWND hCB = GetDlgItem(m_hDlg,  cb);
	int Enable = Button_GetCheck(hCB);

	// If checked then this is equivalent to selecting the current selected
	// If Un-Checked then send -1 as selected filter
	if (Enable)
		UpdateFilter();
	else
		SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
}

// Change GUI to reflect "Start/Stop" states
void SppDlg::SetStreamingState(BOOL isProcessingAudio)
{
	m_StreamingState = isProcessingAudio;

	// Button
	SetStreamingButton(isProcessingAudio);

	// Show/Hide Tab control
	if (isProcessingAudio)
		ShowWindow(GetDlgItem(m_hDlg,  IDC_TABS), SW_SHOW);
	else
		ShowWindow(GetDlgItem(m_hDlg,  IDC_TABS), SW_HIDE);
	

	// Enable/Disable Info frame
	EnableWindow(GetDlgItem(m_hDlg,  IDS_AUDIO_SRC),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDC_STAT_AUDIO),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDC_STAT_TX),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDS_DECODER),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDC_STAT_FILTER),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDS_FILTER),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDC_STAT_VJOY),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDS_JOY),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDS_AUDIO_CHBITS),  isProcessingAudio);
	EnableWindow(GetDlgItem(m_hDlg,  IDS_DECODER_NCH),  isProcessingAudio);
	// TODO: Enable/Disable all Progress bars

	DisplayBackgroundImage(!isProcessingAudio);
}

// Set the value of the "Start/Stop" streaming button
void SppDlg::SetStreamingButton(BOOL isProcessingAudio)
{
	static int state = -1;
	LPTSTR text;

	if (state == (int)isProcessingAudio)
		return;
	else
		state = (int)isProcessingAudio;

	HWND hStream = GetDlgItem(m_hDlg,  IDC_STREAM);
	if (isProcessingAudio)
		text = STOP;
	else
		text = START;

	SetWindowText(hStream, text);
}

void SppDlg::OnStreamStopStart(void)
{
	HWND hStream = GetDlgItem(m_hDlg,  IDC_STREAM);
	TCHAR buff[10];
	BOOL Stream = TRUE;
	GetWindowText(hStream, buff, 10);
	if (!_tcscmp(buff, STOP))
		Stream = FALSE;

	SendMessage(m_ConsoleWnd, WMSPP_DLG_STREAM , (WPARAM)Stream, 0);
}

// Set the selected filter to be displayed in the filter Combo Box
void SppDlg::SelFilter(int FilterId)
{
#if TAB_FLTR_ON
	((SppTabFltr *)m_hrsrc.TabFltr)->SelFilter(FilterId);
#else
	// Get the index of the filter (By ID)
	int i=0, data;
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	while ((data = (int)ComboBox_GetItemData(hCombo, i)) != CB_ERR)
	{
		if (data == FilterId)
		{
			// Select
			int res = ComboBox_SetCurSel(hCombo, i);
			// Checks the checkbox
			Button_SetCheck(hFilterCB, BST_CHECKED);
			break;
		};
		i++;
	};
#endif
}

void SppDlg::InitFilter(int nFilters, LPTSTR FilterName)
{
#if TAB_FLTR_ON
	((SppTabFltr *)m_hrsrc.TabFltr)->InitFilter( nFilters,  FilterName);
#else
	// Clear Filter display
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	SendMessage(hCombo,(UINT) CB_RESETCONTENT ,(WPARAM) 0,(LPARAM)0); 

	// If there are filters then prepare data for selection
	if (nFilters)
	{
		// Bring "-- Select Filter --" to top
		ComboBox_SetText(hCombo, TEXT("-- Select Filter --"));

		// Display File name
		HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
		Edit_SetText(hFilterFile, FilterName);
		UpdateWindow(hFilterFile);
	}
	else
	{
		ComboBox_Enable(hCombo, FALSE);
		HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
		Button_SetCheck(hFilterCB, BST_UNCHECKED);
	};
#endif
}

void SppDlg::AddLine2FilterListA(int FilterID, const char * FilterName)
{
#if TAB_FLTR_ON
	((SppTabFltr *)m_hrsrc.TabFltr)->AddLine2FilterListA( FilterID,  FilterName);
#else
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);

	ComboBox_Enable(hFilterList, TRUE);
	// Convert to a wchar_t*
    size_t origsize = strlen(FilterName) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t FilterNameW[newsize];
	mbstowcs_s(&convertedChars, FilterNameW, origsize, FilterName, _TRUNCATE); // Filter names are converted from ASCII to UNICODE

	int index = (int)SendMessage(hFilterList,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)FilterNameW ); 
	SendMessage(hFilterList,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)FilterID ); 
#endif
}

void SppDlg::AddLine2FilterListW(int FilterID, LPCWSTR FilterName)
{
#if TAB_FLTR_ON
	((SppTabFltr *)m_hrsrc.TabFltr)->AddLine2FilterListW( FilterID,  FilterName);
#else

	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);

	ComboBox_Enable(hFilterList, TRUE);

	int index = (int)SendMessage(hFilterList,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)FilterName ); 
	SendMessage(hFilterList,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)FilterID ); 
#endif
}

LPCTSTR SppDlg::GetDecoderFullName(LPCTSTR Type)
{
	if (!_tcscmp(Type,MOD_TYPE_PPM))
		return MOD_FNAME_PPM;
	if (!_tcscmp(Type,MOD_TYPE_PPMP))
		return MOD_FNAME_PPMP;
	if (!_tcscmp(Type,MOD_TYPE_PPMN))
		return MOD_FNAME_PPMN;
	if (!_tcscmp(Type,MOD_TYPE_PPMW))
		return MOD_FNAME_PPMW;
	if (!_tcscmp(Type,MOD_TYPE_JR))
		return MOD_FNAME_JR;
	if (!_tcscmp(Type,MOD_TYPE_FUT))
		return MOD_FNAME_FUT;
	if (!_tcscmp(Type,MOD_TYPE_AIR1))
		return MOD_FNAME_AIR1;
	if (!_tcscmp(Type,MOD_TYPE_AIR2))
		return MOD_FNAME_AIR2;
	if (!_tcscmp(Type,MOD_TYPE_WAL))
		return MOD_FNAME_WAL;

	return TEXT("--- ??? ---");
}

void SppDlg::AddLine2ModList(MOD * mod, LPCTSTR SelType)
{
		
#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->AddLine2DcdrList( mod,  SelType);
#endif	

	if (!mod)
		return;

	if (!wcscmp(L"PPM",mod->Subtype))
	{ // PPM
		HWND hPPMList = GetDlgItem(m_hDlg,  IDC_LIST_PPM);
		if (SendMessage(hPPMList, LB_FINDSTRINGEXACT , -1, (LPARAM)mod->Name) != LB_ERR)
			return;

		int pos = (int)SendMessage(hPPMList, LB_ADDSTRING, 0, (LPARAM)mod->Name);
		SendMessage(hPPMList, LB_SETITEMDATA, pos, (LPARAM) mod->Type); 
		if (!wcscmp(SelType,mod->Type))
		{
			// Copy text to Info Pane
			Edit_SetText(GetDlgItem(m_hDlg,IDS_DECODER), GetDecoderFullName(SelType));
			SendMessage(hPPMList, LB_SETCURSEL , pos, 0); 
		};
	}
	else
	{ // PCM
		HWND hPCMList = GetDlgItem(m_hDlg,  IDC_LIST_PCM);
		if (SendMessage(hPCMList, LB_FINDSTRINGEXACT , -1, (LPARAM)mod->Name) != LB_ERR)
			return;

		int pos = (int)SendMessage(hPCMList, LB_ADDSTRING, 0, (LPARAM)mod->Name); 
		SendMessage(hPCMList, LB_SETITEMDATA, pos, (LPARAM) mod->Type); 
		if (!wcscmp(SelType,mod->Type))
		{
			// Copy text to Info Pane
			Edit_SetText(GetDlgItem(m_hDlg,IDS_DECODER), GetDecoderFullName(SelType));
			SendMessage(hPCMList, LB_SETCURSEL , pos, 0);
		};
	};
}

//  Button SCAN was pressed
void SppDlg::ScanMod(void)
{
	// Notify CU that it SCAN button was pressed.
	SendMessage(m_ConsoleWnd, WMSPP_DLG_SCAN , 0, 0);
}

// Button pressed: Reply was received from CU - Now change GUI to match
void SppDlg::SelectDecoder(LPCTSTR Decoder)
{
	int list[2] = {IDC_LIST_PPM, IDC_LIST_PCM};
	HWND hList;
	int count=0;
	LPCTSTR iData;
	int SelList=-1, SelItem=-1;

	// Copy text to Info Pane
	Edit_SetText(GetDlgItem(m_hDlg,IDS_DECODER), GetDecoderFullName(Decoder));
#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->SelectDecoder( Decoder);
#endif
	for (int l=0; l<2; l++)
	{ // Loop on both lists (PPM/PCM)
		// Get a list (PPM/PCM)
		hList = GetDlgItem(m_hDlg,  list[l]);

		// Get the number of entries in the list
		count = (int)SendMessage(hList, LB_GETCOUNT , 0, 0);

		// Reset selection
		SendMessage(hList, LB_SETCURSEL , -1, 0);

		// Go over the list - if data maches the set Slected.
		for (int i=0; i<count; i++)
		{ // Loop on list members
			iData = (LPCTSTR)SendMessage(hList, LB_GETITEMDATA, i, 0);
			if (!wcscmp(iData,Decoder))
			{
				SendMessage(hList, LB_SETCURSEL , i, 0);
				break;
			}
		};  // Loop on list members
	}; // Loop on both lists (PPM/PCM)
}

// Button pressed: Timeout reached meaning detection failed
void SppDlg::SelectDecoderFailed(void)
{
	MessageBox(m_hDlg, CN_NO_DECODER, CN_NO_DECODER_HDR, MB_OK|MB_ICONWARNING|MB_TOPMOST);
}

// CU informed of state of Auto-detection of decoder
// If Automode true then:
// - Check the checkbox (IDC_DEC_AUTO)
// - Hide scan button (IDC_BTN_SCAN)
// If Automode false then:
// - Un-Check the checkbox
// - Show scan button
void SppDlg::DecoderAuto(bool automode)
{
#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->DecoderAuto( automode);
#endif

	HWND hBtn = GetDlgItem(m_hDlg,  IDC_BTN_SCAN);

	if (automode)
	{
		CheckDlgButton(m_hDlg,  IDC_DEC_AUTO, BST_CHECKED);
		ShowWindow(hBtn, SW_HIDE);
	}
	else
	{
		CheckDlgButton(m_hDlg,  IDC_DEC_AUTO, BST_UNCHECKED);
		ShowWindow(hBtn, SW_SHOW);
	};
}

// Called when the Decoder's auto checkbox is changed
// Updates CU of the current decoder-detection setup
// Gets the new value of the checkbox and sends it to the CU
void SppDlg::AutoDecParams(void)
{
#if TAB_DCDR_ON
	((SppTabDcdr *)m_hrsrc.TabDcdr)->AutoDecParams( );
#else
	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_DEC_AUTO))
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, AUTODECODE, 1);
	else
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, AUTODECODE, 0);
#endif
}

#if !TAB_JOY_ON
void SppDlg::ShowButtonMapWindow(void)
{
	if (!m_BtnsDlg)
		return;

	vJoySelected(GetDlgItem(m_hDlg,IDC_VJOY_DEVICE));
	m_BtnsDlg->Show();
}
#endif


// Tell the parent window (Main application)
// to show/hide log window
void  SppDlg::ShowLogWindow(WORD cb)
{
	// Get data
	HWND hLogChCB = GetDlgItem(m_hDlg,  cb);
	int show = Button_GetCheck(hLogChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_LOG , show, 0);
}

void  SppDlg::RecordPulse(WORD cb)
{
	// Get data
	HWND hRecordPulseChCB = GetDlgItem(m_hDlg,  cb);
	int record = Button_GetCheck(hRecordPulseChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_PULSE , record, 0);

}


void  SppDlg::RecordInSignal(WORD cb)
{
	// Get data
	HWND hRecordInSignalChCB = GetDlgItem(m_hDlg,  cb);
	int record = Button_GetCheck(hRecordInSignalChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_INSIG , record, 0);

}

#if !TAB_JOY_ON
// Mapping button clicked
// Send all mapping info to the control unit
void  SppDlg::vJoyMapping(void)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_MAPBTN_SEND,0, 0);
	
}
#endif

// Set the parameters of the audio (8/16 bits Left/Right/Mono)
// If Bitrate = 0 then don't change
// If Channel="" or Channel=NULL then don't change
void SppDlg::AudioChannelParams(UINT Bitrate, WCHAR Channel)
{
	static wstring sChannel=TEXT("???"), sBitrate=TEXT("/???");
	wstring s;

	if (Channel == TEXT('L'))
		sChannel = TEXT("Left");
	else
	if (Channel == TEXT('R'))
		sChannel = TEXT("Right");
	else
	if (Channel == TEXT('M'))
		sChannel = TEXT("Mono");

	if (Bitrate == 8)
		sBitrate=TEXT("/8bit");
	if (Bitrate == 16)
		sBitrate=TEXT("/16bit");

	s=sChannel+sBitrate;

	Edit_SetText(GetDlgItem(m_hDlg,IDS_AUDIO_CHBITS),s.data());

#if TAB_AUDIO_ON
	((SppTabAudio *)m_hrsrc.TabAudio)->AudioChannelParams(Bitrate,  Channel);
#else
	if (Bitrate == 8)
		CheckRadioButton(m_hDlg, IDC_AUD_8, IDC_AUD_16, IDC_AUD_8);
	else if (Bitrate == 16)
		CheckRadioButton(m_hDlg, IDC_AUD_8, IDC_AUD_16, IDC_AUD_16);

	if (Channel)
	{
		if (Channel == TEXT('M'))
		{
			CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_MONO);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_LEFT), SW_HIDE);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_RIGHT), SW_HIDE);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_MONO), SW_SHOW);
		}
		else
		{
			ShowWindow(GetDlgItem(m_hDlg,  IDC_LEFT), SW_SHOW);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_RIGHT), SW_SHOW);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_MONO), SW_HIDE);
			if (Channel == TEXT('L'))
				CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_LEFT);
			else if (Channel == TEXT('R'))
				CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_RIGHT);
		}
	};
#endif
}

void SppDlg::AudioAutoParams(WORD Mask, WORD Flags)
{
#if TAB_AUDIO_ON
	((SppTabAudio *)m_hrsrc.TabAudio)->AudioAutoParams( Mask,  Flags);
#else
	// Auto channel selection
	if (Mask&AUTOCHANNEL)
	{
		HWND hLeft = GetDlgItem(m_hDlg,  IDC_LEFT);
		HWND hMono = GetDlgItem(m_hDlg,  IDC_MONO);
		HWND hRight = GetDlgItem(m_hDlg,  IDC_RIGHT);
		if (Flags&AUTOCHANNEL)
		{
			EnableWindow(hLeft, FALSE);
			EnableWindow(hMono, FALSE);
			EnableWindow(hRight, FALSE);
			CheckDlgButton(m_hDlg,  IDC_CH_AUTO, BST_CHECKED);
		}
		else
		{
			EnableWindow(hLeft, TRUE);
			EnableWindow(hMono, TRUE);
			EnableWindow(hRight, TRUE);
			CheckDlgButton(m_hDlg,  IDC_CH_AUTO, BST_UNCHECKED);
		}
	}

	// Auto bit rate selection
	if (Mask&AUTOBITRATE)
	{
		HWND h8 = GetDlgItem(m_hDlg,  IDC_AUD_8);
		HWND h16 = GetDlgItem(m_hDlg,  IDC_AUD_16);
		if (Flags&AUTOBITRATE)
		{
			EnableWindow(h8, FALSE);
			EnableWindow(h16, FALSE);
			CheckDlgButton(m_hDlg,  IDC_AUD_AUTO, BST_CHECKED);
		}
		else
		{
			EnableWindow(h8, TRUE);
			EnableWindow(h16, TRUE);
			CheckDlgButton(m_hDlg,  IDC_AUD_AUTO, BST_UNCHECKED);
		}
	}
#endif
}

//// Get the parameters of the audio (8/16 bits Left/Right/Mono)
//// Send them over to the application
//// Default will be 8bit/Left channel
//void SppDlg::AudioChannelParams(void)
//{
//
//	UCHAR bits = 8;
//	TCHAR Channel = TEXT('L');
//
//	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_AUD_16))
//		bits = 16;
//
//	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_RIGHT))
//		Channel = TEXT('R');
//	else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_MONO))
//		Channel = TEXT('M');
//
//	// Send message: wParam: Number of bits, lParam: channel L/R/M
//	SendMessage(m_ConsoleWnd, WMSPP_DLG_CHNL, bits, Channel);
//
//	
//}

// Called when one of the 'Auto' checkboxs for audio are changed
// ctrl is the ID of the checkbox
// Updates CU of the current audio setup
// Gets the new value of the checkbox and sends it to the CU
void SppDlg::AutoParams(WORD ctrl)
{
	WORD mask=0;
	if (ctrl == IDC_CH_AUTO)
		mask=AUTOCHANNEL;
	else if  (ctrl == IDC_AUD_AUTO)
		mask=AUTOBITRATE;
	else 
		return;

	//AudioChannelParams();

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   ctrl))
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, mask, AUTOBITRATE|AUTOCHANNEL);
	else
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, mask, 0);
}

// Clear channel display
void  SppDlg::ClearChDisplay(UINT FirstChBar, UINT LastChBar, DWORD Color)
{
	HWND hCh;
	UINT ch= FirstChBar;
	do 
	{
		hCh = GetDlgItem(m_hDlg,  ch);
		SetWindowTheme(hCh, L" ", L" ");
		SetWindowPos(hCh, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

		SendMessage(hCh, PBM_SETRANGE ,0, 0x03ff0000); // Range: 0-1023
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		SendMessage(hCh, PBM_SETBARCOLOR , 0, Color);
		ch++;
	} while (ch<=LastChBar);

};

// Start/Stop Pulse Scope
void  SppDlg::SetPulseScope(bool cb)
{
	
	// Set checkbox
	HWND hChkBox = GetDlgItem(m_hDlg,  IDC_PLS_SCOPE);
	if (!hChkBox)
		return;

	if (cb)
		Button_SetCheck(hChkBox, BST_CHECKED);
	else
		Button_SetCheck(hChkBox, BST_UNCHECKED);
}


// Start/Stop monitoring Raw & processed channels
void  SppDlg::MonitorCh(bool cb)
{

#if 0
	// Set checkbox
	HWND hChkBox = GetDlgItem(m_hDlg,  IDC_CH_MONITOR);
	if (!hChkBox)
		return;

	if (cb)
		Button_SetCheck(hChkBox, BST_CHECKED);
	else
		Button_SetCheck(hChkBox, BST_UNCHECKED);
#endif // 0

	ClearChDisplay(IDC_CHPP1, IDC_CHPP8, RGB(0,0,0xFF));
	ClearChDisplay(IDC_CH1, IDC_CH8, RGB(0,0xFF,0));


}

// Tell the parent window (Main application)
// to stop/start monitoring the processed channel data
void  SppDlg::MonitorPrcCh(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);

	// Clear display
	ClearChDisplay(IDC_CHPP1, IDC_CHPP8, RGB(0,0,0xFF));

	// Pass request
	int start = Button_GetCheck(hMonitorChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_MONITOR , start, 0);
}


// Tell the parent window (Main application)
// to stop/start displaying pulse scope
void  SppDlg::PulseScope(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);

	// Pass request
	int start = Button_GetCheck(hMonitorChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_PLSSCOP , start, 0);
}

// Tell the parent window (Main application)
// to stop/start monitoring the raw channel data
void  SppDlg::MonitorRawCh(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);

	// Clear display
	ClearChDisplay(IDC_CH1, IDC_CH8, RGB(0,0xFF,0));

	// Pass request
	int start = Button_GetCheck(hMonitorChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_MONITOR , start, 0);
}

// Configure the vJoy Axes progress bars
void SppDlg::CfgJoyMonitor(HWND hDlg)
{
	// Clear display
	UINT ch= IDC_X;
	HWND hCh;
	do 
	{
		hCh = GetDlgItem(hDlg,  ch);
		SetWindowTheme(hCh, L" ", L" ");
		SetWindowPos(hCh, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		SendMessage(hCh, PBM_SETRANGE ,0, 0xFFFF0000); // Range: 0-64K
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		SendMessage(hCh, PBM_SETBARCOLOR , 0, RGB(0xFF,0,0));

		ch++;
	} while (ch<=IDC_SL1);

}

// Get selected filter fro GUI (if any) and send its filter index to parent window
void SppDlg::UpdateFilter(void)
{
	// Send the ID (in data) of the selected item to the parent window
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	int FilterId; 
	int iCurSel = ComboBox_GetCurSel(hFilterList);
	if (iCurSel==CB_ERR)
	{
		SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
		return;
	}

	FilterId = (int)ComboBox_GetItemData (hFilterList, iCurSel);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)FilterId, 0);

	// Checks the checkbox
	HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	Button_SetCheck(hFilterCB, BST_CHECKED);
}

void SppDlg::SetFilterInfo(LPTSTR FileName,  LPTSTR FilterName)
{

	HWND h = GetDlgItem(m_hDlg, IDS_FILTER);
	if (FilterName && FileName)
	{
		wstring  file = FileName;
		wstring  fltr = FilterName;
		wstring str = TEXT("Filter: ")+ fltr /*+ TEXT("   [File: ") + file + TEXT("]")*/;
		Edit_SetText(h, str.data());
	}
	else
		Edit_SetText(h, TEXT("--- No Filter ---"));
	
}

#if !TAB_JOY_ON
// Fill-in the actual button-mapping data - pass message to button-mapping dialog
void SppDlg::SetButtonsMappingData(BTNArr* aButtonMap, UINT nButtons)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_MAPBTN_UPDT,(WPARAM)aButtonMap, nButtons);
}
#endif

// Enable/disable controls according to vJoy device settings
void SppDlg::EnableControls(UINT id, controls * ctrl)
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->EnableControls( id,   ctrl);
#else
#endif
	UINT ch= IDC_X;
	UINT edt = IDC_SRC_X;
	HWND hCh;
	UINT iAxis=0;

	//////// Verify correct vJoy device
	//HWND hCb = GetDlgItem(m_hDlg,IDC_VJOY_DEVICE);
	//// Get the index of the selected vJoy device
	//int index = (int)SendMessage(hCb,(UINT) CB_GETCURSEL  ,(WPARAM) 0,(LPARAM)0); 
	//if (index == CB_ERR)
	//	return;

	//	// Extract the device id from the item's data
	//int SelId = (int)SendMessage(hCb,(UINT) CB_GETITEMDATA   ,(WPARAM) index,(LPARAM)0);
	//if (id != SelId)
	//	return;
	//////// Verified

	// Go over all axes
	do 
	{
		// Axis bars
		hCh = GetDlgItem(m_hDlg,  ch);
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		ShowWindow(hCh, ctrl->axis[ch-IDC_X]);
		UpdateWindow(hCh);

		//// Map edit fields
		//hEdt = GetDlgItem(m_hDlg,  edt);
		//EnableWindow(hEdt, ctrl->axis[edt-IDC_SRC_X]);
		//UpdateWindow(hEdt);

		ch++;
		edt++;
	} while (ch<=IDC_SL1);

}


// Relay the actual button-mapping data - pass message to parent
void SppDlg::SendMappingData(BTNArr* aButtonMap, UINT nButtons)
{
	// Buttons
	Mapping m;
	m.nButtons = nButtons;
	m.ButtonArray = aButtonMap;


	// Axes
	TCHAR Buffer[4];
	int nTchar;
	int out=0;
	UINT id = IDC_SRC_X;
	DWORD AxesMap = 0;

	// Go on all entries and get value. Empty is considered as channel 0
	// The value has to be one/two digits - is pusshed onto the DWORD to be sent
	do {
	((WORD *)Buffer)[0]=4;
	out=0;
	HWND hEdtBox = GetDlgItem(m_hDlg,  id);
	nTchar = Edit_GetLine(hEdtBox, 1, &Buffer, 3);
	if (nTchar == 1 || nTchar == 2)
	{
		Buffer[nTchar] = NULL;
		out = _tstoi(Buffer);
	}
 	if (out>0xF)
		out=0;
	AxesMap = (AxesMap<<4 | out);
	id++;
	} while (id <= IDC_SRC_SL1);

	m.pAxisMap = &AxesMap;
	m.nAxes = 8;

	SendMessage(m_ConsoleWnd, WMSPP_DLG_MAP, (WPARAM)&m, NULL);
}

// Fill-in the actual mapping data
void SppDlg::SetMappingData(Mapping * Map)
{

#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->SetMappingData(Map);
#else

	UINT id = IDC_SRC_SL1;
	HWND hEdtBox;
	UINT channel;
	TCHAR buffer[4];
	UINT& nAxes = Map->nAxes;
	DWORD& AxisMap = *Map->pAxisMap;

	// Go through the map and read nibble by nibble
	// Every nibble read goes to the corresponding edit box
	for (UINT i=0; i<nAxes; i++)
	{
		hEdtBox = GetDlgItem(m_hDlg,  id-i);
		channel = ((AxisMap>>(i*4))&0xF);
		_itot_s(channel, buffer, 2, 10);
		Edit_SetText(hEdtBox, buffer);
	};

	SetButtonsMappingData(Map->ButtonArray, Map->nButtons);
#endif
}


void  SppDlg::AddLine2AudioList(jack_info * jack)
{
	// Print jack name to summary
	if (jack->Default)
	{
		Edit_SetText(GetDlgItem(m_hDlg,IDS_AUDIO_SRC),jack->FriendlyName);
		if (m_color != jack->color)
		{
			m_color = jack->color;
			OnPaint();
		}		
	};


	//// Audio jack must have at least one audio channel (mono)
	//HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
	//if (!jack->nChannels)
	//	return;


	//// Insert audio jack name
	//LV_ITEM item;
	//item.mask = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE |LVIF_PARAM;
	//item.iItem = 0;
	//item.iSubItem = 0;
	//item.pszText = jack->FriendlyName;
 //   item.stateMask = 0;
 //   item.iSubItem  = 0;
 //   item.state     = 0;
	//item.lParam = (LPARAM)jack->id;
	//int i = ListView_InsertItem(hAudioList, &item);

	//if (jack->nChannels == 1)
	//	ListView_SetItemText(hAudioList, i, 1, TEXT("0"))
	//else
	//	ListView_SetItemText(hAudioList, i, 1, TEXT("0/0")); 

	//// Set the default jack as focused (and selected)
	//if (jack->Default)
	//	ListView_SetItemState(hAudioList, i, 0xF|LVIS_FOCUSED, 0xF|LVIS_FOCUSED);

}

// Get the selected vJoy device
// Extract the device id from the item's data
// Send device id to CU
void  SppDlg::vJoySelected(HWND hCb)
{
	// Get the index of the selected vJoy device
	int index = (int)SendMessage(hCb,(UINT) CB_GETCURSEL  ,(WPARAM) 0,(LPARAM)0); 
	if (index == CB_ERR)
		return;

	// Extract the device id from the item's data
	int id = (int)SendMessage(hCb,(UINT) CB_GETITEMDATA   ,(WPARAM) index,(LPARAM)0);
	if (id == CB_ERR)
		return;

	// Send device id to CU
	SendMessage(m_ConsoleWnd, WMSPP_DLG_VJOYSEL, (WPARAM)id, 0);
}

// Remove all vJoy Entries
void  SppDlg::vJoyRemoveAll()
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->vJoyRemoveAll( );
#else
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	SendMessage(hCombo,(UINT) CB_RESETCONTENT ,(WPARAM) 0,(LPARAM)0); 
#endif
}

// Add vJoy device entry to combo box
// Set the id as item data
void  SppDlg::vJoyDevAdd(UINT id)
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->vJoyDevAdd( id);
#else
	wstring vjoyid = L"vJoy " + to_wstring(id);
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	int index = (int)SendMessage(hCombo,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)(vjoyid.data()) ); 
	SendMessage(hCombo,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)id ); 
#endif
}

// Set the selected vJoy device
void  SppDlg::vJoyDevSelect(UINT id)
{
#if TAB_JOY_ON
	((SppTabJoy *)m_hrsrc.TabJoy)->vJoyDevSelect( id);
#else
	wstring vjoyid = L"vJoy " + to_wstring(id);
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	int index = (int)SendMessage(hCombo,(UINT) CB_FINDSTRINGEXACT ,(WPARAM) -1,(LPARAM)(vjoyid.data()) ); 
	if (index == CB_ERR)
		return;
	index =  (int)SendMessage(hCombo,(UINT) CB_SETCURSEL ,(WPARAM) index, 0); 
#endif
}

// Informs Parent window (CU) that the user pressed OK or Cancel button
void SppDlg::ExitWithOK(bool OkSelected)
{
	SendMessage(m_ConsoleWnd, WMSPP_DLG_OK, (WPARAM)OkSelected, 0);
}

// Informs Parent window (CU) that the user went to/from iconified dispaly mode
void SppDlg::Iconified(bool icon)
{
	SendMessage(m_ConsoleWnd, WMSPP_DLG_ICONFD, (WPARAM)icon, 0);
}

void SppDlg::SelChanged(WORD ListBoxId, HWND hListBox)
{
	// Case the message origin is one of the Modulation PPM/PCM list boxes
	// Clear all selection from the other box then get the selected entry.
	// Notify parent window of the new selected item
	if ((ListBoxId==IDC_LIST_PPM) || (ListBoxId==IDC_LIST_PCM))
	{
		HWND hPpm = GetDlgItem(m_hDlg,  IDC_LIST_PPM);
		HWND hPcm = GetDlgItem(m_hDlg,  IDC_LIST_PCM);
		int SelPrev = -1;
		int SelNew = -1;
		HWND hPrev = NULL;
		HWND hNew = NULL;

		if (ListBoxId==IDC_LIST_PPM)
		{
			hPrev = hPcm;
			hNew  = hPpm;
		}
		else
		{
			hPrev = hPpm;
			hNew  = hPcm;
		}

		SelPrev = ListBox_GetCurSel(hPrev);
		SelNew = ListBox_GetCurSel(hNew);
		SendMessage(hPrev, LB_SETCURSEL , -1, 0);
		if (SelNew == LB_ERR)
			return;

			// Get a handle to the parent window
		LPCTSTR mod = (LPCTSTR)SendMessage(hNew, LB_GETITEMDATA, SelNew, NULL);
		SendMessage(m_ConsoleWnd, WMSPP_DLG_MOD, (WPARAM)mod, 0);

		// Make sure decoder selection is manual
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, AUTODECODE, 0);
	};
}

// Create a list of immages to be used by tab control
HIMAGELIST SppDlg::CreateTabsImageList(void)
{
	HICON icon;
	ICONINFO iconinf;
	HIMAGELIST hList;
	HBITMAP bitmap;

	int icons[] = {IDI_AUDIO, IDI_DECODER, IDI_FILTER, IDI_JOYSTICK, IDI_ADVANCED};
	// Creat a list of 32x32 images that may contain up to 4 icons
	hList = ImageList_Create(32, 32, ILC_COLOR32, ILAST, 0); // 
	if (!hList)
		return hList;

	// Load icons to the limmage list
    for (int i =0; i<ILAST; ++i)
    {
        icon = reinterpret_cast<HICON>(LoadImage(m_hInstance, MAKEINTRESOURCE(icons[i]), IMAGE_ICON, 32, 32, LR_LOADTRANSPARENT));
		if (!icon)
			continue;
        GetIconInfo(icon, &iconinf);
        bitmap = iconinf.hbmColor;
		int res = ImageList_Add(hList, bitmap, NULL);
        DestroyIcon(icon);
    };

	return hList;
}

// Create one central  Tooltip object
HWND SppDlg::CreateToolTip(HWND hDlg)
{
	LRESULT  added, active;
	const int Controls[] = {\
		IDS_AUDIO_SRC, IDC_HIDE, IDC_STREAM,  IDS_AUDIO_CHBITS, IDS_DECODER, IDS_DECODER_NCH, \
		IDS_FILTER, IDS_JOY, IDS_STATUS_EDT, IDOK, \
		IDC_CH1, IDC_CH2, IDC_CH3, IDC_CH4, IDC_CH5, IDC_CH6, IDC_CH7, IDC_CH8, \
		IDC_CHPP1, IDC_CHPP2, IDC_CHPP3, IDC_CHPP4, IDC_CHPP5, IDC_CHPP6, IDC_CHPP7, IDC_CHPP8, \
		IDC_X, IDC_Y, IDC_Z, IDC_RX, IDC_RY, IDC_RZ, IDC_SL0, IDC_SL1 \
	};

	if (!hDlg || !m_hInstance)
		return (HWND)NULL;

	// Create the tooltip.
	HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                              WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON | WS_EX_TOOLWINDOW,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              hDlg, NULL, 
                              m_hInstance, NULL);

	if  (!hwndTip)
		m_hwndToolTip = (HWND)NULL;
	else
		m_hwndToolTip = hwndTip;


   // Initializing Tooltip per control
   if (m_hwndToolTip)
   {
	   // General initialization
	   TOOLINFO toolInfo = { 0 };
	   toolInfo.cbSize = TTTOOLINFO_V1_SIZE;
	   toolInfo.hwnd = hDlg;
	   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	   toolInfo.lpszText = LPSTR_TEXTCALLBACK;

	   // Loop on all controls that require tooltip
	   for (auto ctrl : Controls)
	   {
		   HWND hwndTool = GetDlgItem(hDlg, ctrl);
		   toolInfo.uId = (UINT_PTR)hwndTool;
		   added = SendMessage(m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
		   active = SendMessage(m_hwndToolTip, TTM_ACTIVATE, TRUE, (LPARAM)&toolInfo);
	   };
   }

   return m_hwndToolTip;
}

// Display callback-type tooltip
// lpttt:	Pointer to tooltip structure
// TxtID:	The resource ID of the text to display
// TitleID:	The resource ID of the title - default is "no title"
// Icon:	Icon to display. Possible values are TTI_NONE (default), TTI_INFO, TTI_WARNING, TTI_ERROR
void SppDlg::DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, int TitleID, int Icon)
{
	TCHAR ControlText[MAX_MSG_SIZE] ={0};
	TCHAR TitleText[MAX_MSG_SIZE] ={0};

	LoadString(m_hInstance, TxtID,  ControlText, MAX_MSG_SIZE);
	lpttt->lpszText = ControlText;

	if (TitleID<0)
		SendMessage(m_hwndToolTip, TTM_SETTITLE, TTI_NONE, (LPARAM) TEXT(""));
	else
	{
		LoadString(m_hInstance, TitleID,  TitleText, MAX_MSG_SIZE);
		SendMessage(m_hwndToolTip, TTM_SETTITLE, Icon, (LPARAM) TitleText);
	};
}

// Display callback-type tooltip
// lpttt:	Pointer to tooltip structure
// TxtID:	The resource ID of the text to display
// TitleStr:The title string
// Icon:	Icon to display. Possible values are TTI_NONE (default), TTI_INFO, TTI_WARNING, TTI_ERROR
void SppDlg::DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, LPCTSTR TitleStr , int Icon)
{
	TCHAR ControlText[MAX_MSG_SIZE] ={0};
	TCHAR TitleText[MAX_MSG_SIZE] ={0};

	LoadString(m_hInstance, TxtID,  ControlText, MAX_MSG_SIZE);
	lpttt->lpszText = ControlText;

	SendMessage(m_hwndToolTip, TTM_SETTITLE, Icon, (LPARAM) TitleStr);
}
void SppDlg::UpdateToolTip(LPVOID param)
{
	LPNMTTDISPINFO lpttt = (LPNMTTDISPINFO)param;
	TCHAR ControlText[MAX_MSG_SIZE] ={0};
	TCHAR TitleText[MAX_MSG_SIZE] ={0};
	int ControlTextSize = 0;

	// Since the id field of the control in the tooltip was defined as a handle - it has to be converted back
	int CtrlId = GetDlgCtrlID((HWND)lpttt->hdr.idFrom);

	// Handle to the tooltip window
	HWND hToolTip = lpttt->hdr.hwndFrom;

	switch (CtrlId) // Per-control tooltips
	{
	// Name of audio jack
	case IDS_AUDIO_SRC:
		DisplayToolTip(lpttt, IDS_I_AUDIO_SRC, IDS_T_AUDIO_SRC);
		break;

	// Minimize button
	case IDC_HIDE:
		DisplayToolTip(lpttt, IDS_I_HIDE);
		break;

	// Sleep/Wake-up button
	case IDC_STREAM:
		ControlTextSize = Button_GetText((HWND)lpttt->hdr.idFrom, ControlText, MAX_MSG_SIZE);
		if (ControlTextSize>0 && ControlTextSize<MAX_MSG_SIZE)
		{
			// Button is "Wake-up"
			if (!_tcscmp(ControlText, START))
			{
				DisplayToolTip(lpttt, IDS_I_START, START, TTI_WARNING);
			}
			else
			{
				DisplayToolTip(lpttt, IDS_I_STOP, STOP, TTI_INFO);
			}
		};
		break;

	case IDS_AUDIO_CHBITS:
		DisplayToolTip(lpttt, IDS_I_AUDIO_CHBITS);
		break;

	case IDS_DECODER:
		DisplayToolTip(lpttt, IDS_I_DECODER, IDS_T_DECODER);
		break;

	case IDS_DECODER_NCH:
		DisplayToolTip(lpttt, IDS_I_DECODER_NCH);
		break;

	case IDS_FILTER:
		DisplayToolTip(lpttt, IDS_I_FILTER, IDS_T_FILTER);
		break;

	case IDS_JOY:
		DisplayToolTip(lpttt, IDS_I_JOY, IDS_T_JOY);
		break;

	case IDS_STATUS_EDT:
		DisplayToolTip(lpttt, IDS_I_STATUS_EDT);
		break;

	case IDOK:
		DisplayToolTip(lpttt, IDS_I_OK);
		break;

	case IDC_CH1:
		DisplayToolTip(lpttt, IDS_I_CH1);
		break;

	case IDC_CH2:
		DisplayToolTip(lpttt, IDS_I_CH2);
		break;

	case IDC_CH3:
		DisplayToolTip(lpttt, IDS_I_CH3);
		break;

	case IDC_CH4:
		DisplayToolTip(lpttt, IDS_I_CH4);
		break;

	case IDC_CH5:
		DisplayToolTip(lpttt, IDS_I_CH5);
		break;

	case IDC_CH6:
		DisplayToolTip(lpttt, IDS_I_CH6);
		break;

	case IDC_CH7:
		DisplayToolTip(lpttt, IDS_I_CH7);
		break;

	case IDC_CH8:
		DisplayToolTip(lpttt, IDS_I_CH8);
		break;

	case IDC_CHPP1:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP1);
		break;

	case IDC_CHPP2:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP2);
		break;

	case IDC_CHPP3:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP3);
		break;

	case IDC_CHPP4:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP4);
		break;

	case IDC_CHPP5:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP5);
		break;

	case IDC_CHPP6:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP6);
		break;

	case IDC_CHPP7:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP7);
		break;

	case IDC_CHPP8:
		DisplayToolTip(lpttt, IDS_I_CHPP, IDS_T_CHPP8);
		break;

	case IDC_X:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_X);
		break;

	case IDC_Y:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_Y);
		break;

	case IDC_Z:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_Z);
		break;

	case IDC_RX:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_RX);
		break;

	case IDC_RY:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_RY);
		break;

	case IDC_RZ:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_RZ);
		break;

	case IDC_SL0:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_SL0);
		break;

	case IDC_SL1:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_SL1);
		break;

	default:
		break;
	}
}

VOID SppDlg::OnPaint(void)
{

	HDC          hdc;
	PAINTSTRUCT  ps;

	// Get control's dimentions
	HWND hJack = GetDlgItem(m_hDlg, IDI_JACK);
	RECT rect;
	GetClientRect(hJack, &rect);

	// Clear current jack bitmap
	InvalidateRect(hJack, NULL, TRUE);

	// If color unknown (-1) then don't draw
	//if (m_color == 0xFFFFFFFF)
	//	return;

	// Colors
	Gdiplus::Color Blue = Gdiplus::Color(255, 0, 0, 255);
	Gdiplus::Color PalePink = Gdiplus::Color(100, 255, 0, 0);
	Gdiplus::Color Black = Gdiplus::Color(255, 0, 0, 0);
	Gdiplus::Color LightGrey = Gdiplus::Color(255, 150, 150, 150);
	Gdiplus::Color Jack =  Gdiplus::Color(m_color + 0x80000000);

	// Pens & Brushes
	Gdiplus::Pen		penBlue(Blue); 
	Gdiplus::Pen		penBlack(Black); 
	Gdiplus::Pen		penLightGrey(LightGrey); 
	Gdiplus::SolidBrush solidBrushPink(PalePink); // Pink (Transparrent)
	Gdiplus::SolidBrush solidBrushBlack(Black); // Pink (Transparrent)
	Gdiplus::SolidBrush solidBrushJack(Jack); //
	Gdiplus::SolidBrush solidBrushWhite(Gdiplus::Color::White); //

	// Paint
	hdc = BeginPaint(hJack, &ps);
	Gdiplus::Graphics graphics(hdc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias8x8 );

	
	// If color unknown (-1) then just clear the area
	if (m_color == 0xFFFFFFFF)
	{
		COLORREF bg = GetSysColor(COLOR_MENU);
		graphics.Clear(bg+0xFF000000);
		return;
	}

	// Circles & Rings
	graphics.DrawEllipse(&penBlack, 1, 1, rect.right-2, rect.bottom-2);
	graphics.DrawEllipse(&penLightGrey, 3, 3, rect.right-6, rect.bottom-6);
	graphics.DrawEllipse(&penBlack, 4, 4, rect.right-8, rect.bottom-8);
	graphics.FillEllipse(&solidBrushJack, 4, 4, rect.right-8, rect.bottom-8);
	graphics.FillEllipse(&solidBrushBlack, 8,8, rect.right-16, rect.bottom-16);
	graphics.DrawEllipse(&penLightGrey, 9, 9, rect.right-18, rect.bottom-18);

	EndPaint(hJack, &ps);
}

// Message handler for top spp dialog box.
INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppDlg * DialogObj = NULL;

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	switch (message)
	{

	case WM_PAINT:
		DialogObj->OnPaint();
	break;

	case WM_DESTROY:
			DestroyWindow(hDlg);
			hDlg = NULL;
//			PostQuitMessage(0);
			return (INT_PTR)TRUE;

	case WM_NOTIFY:

		if (((LPNMHDR)lParam)->idFrom  == IDC_BTN_MAP)
		{
			return  (INT_PTR)TRUE;
		}

				
		if (((LPNMHDR)lParam)->idFrom  == IDC_TABS)
		{
			DialogObj->OnSelChanged(hDlg);
			return  (INT_PTR)TRUE;
		}

		// Tooltips
		if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
		{
			DialogObj->UpdateToolTip((LPVOID)lParam);
			return  (INT_PTR)TRUE;
		};
		
		return (INT_PTR)TRUE;


	case WM_INITDIALOG:
		DialogObj = (SppDlg *)lParam;

		// Initialize common controls
		INITCOMMONCONTROLSEX InitCtrlEx;
		InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
		InitCtrlEx.dwICC  =  0 /*ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_STANDARD_CLASSES*/;
		InitCommonControlsEx(&InitCtrlEx);

		// Initialize GDI+.
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		DialogObj->InitTabs(hDlg); // Initialize the tab control
		DialogObj->CfgJoyMonitor(hDlg); // Initialize vJoy Monitoring
		DialogObj->InitFilterDisplay(hDlg); // Initialize Filter section of the GUI
		DialogObj->CreateToolTip(hDlg); // Initialize tooltip object
		DialogObj->InitBackgroundImage(hDlg); // Initialize backgrown image

#if !TAB_JOY_ON
		DialogObj->CreateBtnsDlg(hDlg); // Create button dialog box
#endif
//		DialogObj->InitAudioDisplay(hDlg); // Initialize audio source display
		return (INT_PTR)TRUE;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MINIMIZE)
		{
			SendMessage(DialogObj->GetHandle(),WM_COMMAND,IDC_HIDE,0);
			return (INT_PTR)TRUE;
		}

		if ((wParam & 0xFFF0) == SC_CLOSE)
		{
			SendMessage(DialogObj->GetHandle(),WM_COMMAND,IDOK,0);
			return (INT_PTR)TRUE;
		}

		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			DialogObj->ExitWithOK(LOWORD(wParam) == IDOK);
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDC_HIDE)
		{
			DialogObj->Hide();
			DialogObj->Iconified(true);
			break;
		}

		if (LOWORD(wParam) == IDM_DISPLAY_WIZARD)
		{
			DialogObj->Show();
			DialogObj->Iconified(false);
			break;
		}

		if (HIWORD(wParam) == LBN_SELCHANGE )
		{
			DialogObj->SelChanged(LOWORD(wParam), (HWND)lParam);
			break;
		}

		//if (LOWORD(wParam) == IDC_CH_MONITOR)
		//{
		//	DialogObj->MonitorPrcCh(LOWORD(wParam));
		//	DialogObj->MonitorRawCh(LOWORD(wParam));
		//	break;
		//};

		if (LOWORD(wParam) == IDC_PLS_SCOPE)
		{
			DialogObj->PulseScope(LOWORD(wParam));
			break;
		};

		if (LOWORD(wParam) == IDC_CH_LOG)
		{
			DialogObj->ShowLogWindow(LOWORD(wParam));
			break;
		};

		if (LOWORD(wParam) == IDC_CH_INSIG)
		{
			DialogObj->RecordInSignal(LOWORD(wParam));
			break;
		};

		if (LOWORD(wParam) == IDC_CH_PULSE)
		{
			DialogObj->RecordPulse(LOWORD(wParam));
			break;
		};
		
#if !TAB_JOY_ON
		if (LOWORD(wParam)  == IDC_BTN_MAP && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->vJoyMapping();
			break;
		}

		if (LOWORD(wParam)  == IDC_BTN_MAPBTNS && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->ShowButtonMapWindow();
			break;
		}
#endif

		if (LOWORD(wParam)  == IDC_BTN_SCAN && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->ScanMod();
			break;
		}

		if  (LOWORD(wParam)  == IDC_AUD_8 || LOWORD(wParam)  == IDC_AUD_16 ||  LOWORD(wParam)  == IDC_LEFT || LOWORD(wParam)  == IDC_RIGHT /*|| LOWORD(wParam)  == IDC_MONO*/) 
		{
			//DialogObj->AudioChannelParams();
			break;
		}

		if  (LOWORD(wParam)  == IDC_CH_AUTO) 
		{
			DialogObj->AutoParams(IDC_CH_AUTO);
			break;
		}

		if  (LOWORD(wParam)  == IDC_AUD_AUTO) 
		{
			DialogObj->AutoParams(IDC_AUD_AUTO);
			break;
		}

		if  (LOWORD(wParam)  == IDC_DEC_AUTO) 
		{
			DialogObj->AutoDecParams();
			break;
		}



		if (LOWORD(wParam)  == IDC_VJOY_DEVICE && HIWORD(wParam) == CBN_SELENDOK  )
		{
			DialogObj->vJoySelected((HWND)lParam);
			break;
		}

		if (LOWORD(wParam)  == IDC_BTN_FILTERBROWSE && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->OnFilterFileBrowse();
			break;
		}

		if (LOWORD(wParam)  == IDC_STREAM && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->OnStreamStopStart();
			break;
		}

		if  (LOWORD(wParam)  == IDC_COMBO_FILTERS && HIWORD(wParam) == CBN_SELENDOK   )
		{
			DialogObj->UpdateFilter();
			break;
		}

		if (LOWORD(wParam) == IDC_CH_FILTER)
		{
			DialogObj->EnableFilter(LOWORD(wParam));
			break;
		};

		break; // No match for WM_COMMAND


	case REM_ALL_JACK:
		// DialogObj->CleanAudioList();
		break;

	case POPULATE_JACKS:
		DialogObj->AddLine2AudioList((jack_info *)(wParam));
		break;

	case SET_AUDIO_PARAMS:
		DialogObj->AudioChannelParams((UINT)wParam, (WCHAR)lParam);
		break;
		
	case SET_AUDIO_AUTO:
		DialogObj->AudioAutoParams((UINT)wParam, (WCHAR)lParam);
		break;

	case WMSPP_PRCS_SETMOD:
		DialogObj->AddLine2ModList((MOD *)(wParam), (LPCTSTR)(lParam));
		break;

	case WMSPP_PRCS_RCHMNT:
		DialogObj->SetRawChData((UINT)wParam, (UINT)lParam);
		break;

	case WMSPP_PRCS_NRCHMNT:
		DialogObj->SetNumberRawCh((UINT)wParam);
		break;

	case WMSPP_PRCS_NPCHMNT:
		DialogObj->SetNumberProcCh((UINT)wParam);
		break;

	case WMSPP_DECD_QLT:
		DialogObj->SetDecoderQuality((UINT)wParam);
		break;

	case WMSPP_PRCS_ALIVE:
		DialogObj->SetStreamingState((BOOL)wParam);
		break;


	case WMSPP_PRCS_PCHMNT:
		DialogObj->SetProcessedChData((UINT)wParam, (UINT)lParam);
		break;

	case FILTER_ADDA:
		DialogObj->AddLine2FilterListA((int)wParam, (const char *)lParam);
		break;

	case FILTER_ADDW:
		DialogObj->AddLine2FilterListW((int)wParam, (LPCWSTR)lParam);
		break;

	case FILTER_NUM:
		DialogObj->InitFilter((int)wParam, (LPTSTR)lParam);
		break;

	case FILTER_SELCTED:
		DialogObj->SelFilter((int)wParam);
		break;


	case WMSPP_JMON_AXIS:
		DialogObj->SetJoystickAxisData((UCHAR)(wParam&0xFF), (UINT)(wParam>>16), (UINT32)lParam);
		DialogObj->SetJoystickDevFrame((UCHAR)(wParam&0xFF));
		break;

	case WMSPP_JMON_BTN:
		DialogObj->SetJoystickBtnData((UCHAR)wParam, (BTNArr *)lParam);
		break;

	case WMSPP_MAP_UPDT:
		DialogObj->SetMappingData((Mapping *)wParam);
		break;

	case WMSPP_STAT_UPDT:
		 DialogObj->SppStatusChanged(  wParam,  lParam);
		break;

	//case WMSPP_MAPBTN_UPDT:
	//	DialogObj->SetButtonsMappingData((BTNArr *)wParam, (UINT)lParam);
	//	break;

	case MONITOR_CH:
		DialogObj->MonitorCh(wParam != 0); // Silly way to cast to bool
		break;

	case PULSE_SCOPE:
		DialogObj->SetPulseScope(wParam != 0); // Silly way to cast to bool
		break;

	case VJOYDEV_ADD:
		DialogObj->vJoyDevAdd((UINT)wParam);
		if (lParam)
			DialogObj->vJoyDevSelect((UINT)wParam);
		break;

	case VJOYDEV_REMALL:
		DialogObj->vJoyRemoveAll();
		break;

	case WMSPP_DLG_MAPBTN:
		DialogObj->SendMappingData((BTNArr *)wParam, (UINT)lParam);
		break;

	case VJOYDEV_SETAVAIL:
		DialogObj->EnableControls((UINT)wParam, (controls*)lParam);
		break;

	case VJOYDEV_CH_LEVEL:
		DialogObj->DisplayAudioLevels(hDlg, (PVOID)wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case WMAPP_NOTIFYCALLBACK:
		DialogObj->OnNotificationIcon(wParam,  lParam);
		break;

	case WMSPP_DLG_FLTR:
		DialogObj->SetFilterInfo((LPTSTR)wParam,  (LPTSTR)lParam);
		break;


	case WMSPP_PRCS_DCDR:
		if (wParam && !lParam)
			DialogObj->SelectDecoder((LPCTSTR)wParam);
		else if (!wParam)
			DialogObj->SelectDecoderFailed();
		break;

	case SET_DEC_AUTO:
		DialogObj->DecoderAuto(wParam != 0);
		break;

	case WMSPP_DLG_CHNL:
		return DialogObj->RelayToConsoleWnd(message,  wParam,  lParam);

	case WMSPP_DLG_RST:
		return DialogObj->Reset();

	case WMSPP_DLG_MOD:
		Edit_SetText(GetDlgItem(DialogObj->GetHandle(),IDS_DECODER),  DialogObj->GetDecoderFullName((LPCTSTR)wParam));
		return DialogObj->RelayToConsoleWnd(message,  wParam,  lParam);

	case WMSPP_DLG_AUTO:
	case WMSPP_DLG_SCAN:
	case WMSPP_DLG_FLTRFILE:
	case WMSPP_DLG_FILTER:
	case WMSPP_DLG_VJOYSEL:
	case WMSPP_DLG_MAP:
	case WMSPP_DLG_PLSSCOP:
	case WMSPP_DLG_LOG:
	case WMSPP_DLG_INSIG:
	case WMSPP_DLG_PULSE:
		return DialogObj->RelayToConsoleWnd(message,  wParam,  lParam);
		break;


	}
	return (INT_PTR)FALSE;
}

LRESULT SppDlg::RelayToConsoleWnd(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = SendMessage(m_ConsoleWnd, message,  wParam,  lParam);
	SetWindowLongPtr (m_hDlg, 0, res);
	return TRUE;
}

HWND SppDlg::GetHandle(void)
{
	return m_hDlg;
}

#if !TAB_JOY_ON
// Create Button mapping dialog box
void SppDlg::CreateBtnsDlg(HWND hDlg)
{
	m_BtnsDlg = new SppBtnsDlg(m_hInstance, hDlg);
}
#endif

// Initialize Filters section
void SppDlg::InitFilterDisplay(HWND hDlg)
{
	// Get handles to the controls
	HWND hFilterFile	= GetDlgItem(hDlg,  IDC_EDIT_FILTERFILE);
	HWND hFilterCB		= GetDlgItem(hDlg,  IDC_CH_FILTER);
	HWND hFilters		= GetDlgItem(hDlg,  IDC_COMBO_FILTERS);
	HWND hFilterEdt		= GetDlgItem(hDlg, IDS_FILTER);

	// Clear Filter File, Unselect checkbox and gray-out Selected Filters
	Edit_SetText(hFilterFile, TEXT("Select Filter File"));
	Button_SetCheck(hFilterCB, BST_UNCHECKED);
	ComboBox_Enable(hFilters, FALSE);
	Edit_SetText(hFilterEdt, TEXT("--- No Filter ---"));
}

// Respond to Browse button
// If file selected then sends message WMSPP_DLG_FLTRFILE with full path to selected file
// CU tests file - if valid then file name (NOT full path) is displayed
void SppDlg::OnFilterFileBrowse(void)
{

	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[MAX_PATH];       // buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hDlg;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = TEXT("DLL Files\0*.DLL\0All\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrTitle = TEXT("Open Filter File");

	// Display the Open dialog box. 
	BOOL ok = GetOpenFileName(&ofn);
	if (!ok)
		return;

	// If file selected then send it to CU - wait to see results
	// If FileName not NULL then use it to display
	LRESULT info;
	info = SendMessage(m_ConsoleWnd, WMSPP_DLG_FLTRFILE , (WPARAM)ofn.lpstrFile, 0);
	if (!info)
		return;

	// Display File Name
	HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
	Edit_SetText(hFilterFile, (LPTSTR)info);
	UpdateWindow(hFilterFile);
	delete[] (LPVOID)info;
}

#if 0
// Respond to Browse button
// Open a IFileOpenDialog to get full path of the filters DLL file
// Based on http://msdn.microsoft.com/en-us/library/windows/desktop/bb776913(v=vs.85).aspx#api
void SppDlg::OnFilterFileBrowse(void)
{
	// CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (FAILED(hr))
		return;

	// No Event handler at the moment
	// Set the options on the dialog.
	DWORD dwFlags;

	// Before setting, always get the options first in order not to override existing options.
	hr = pfd->GetOptions(&dwFlags);
	if (FAILED(hr))
		return;

	// In this case, get shell items only for file system items.
	hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
	if (FAILED(hr))
		return;

	// Set the file types to display only. Notice that, this is a 1-based array.
	hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
	if (FAILED(hr))
		return;

	// Set the selected file type index to dll.
	hr = pfd->SetFileTypeIndex(1);
	if (FAILED(hr))
		return;

	// Set the default extension to be ".dll" file.
	hr = pfd->SetDefaultExtension(L"dll");
	if (FAILED(hr))
		return;

	IKnownFolderManager *pkfm = NULL;
	IKnownFolder * ppkf = NULL;
	hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
	hr = pkfm->GetFolderByName(L".", &ppkf);

	// Show the dialog
	hr = pfd->Show(NULL);
	if (FAILED(hr))
		return;

}
#endif
