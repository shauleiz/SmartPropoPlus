#include "stdafx.h"
#include "Windowsx.h"
#include "SmartPropoPlus.h"
#include "WinMessages.h"
#include "public.h"
#include "Commctrl.h"
#include "resource.h"
#include "..\vJoyMonitor\vJoyMonitor.h"
#include "SppBtnsDlg.h"
#include "SppDlg.h"
#include "SppTab.h"

// GLobals

// IDs of raw channel progress bars 
static const int g_RawBarId[] = 
{IDC_CH1,IDC_CH2,IDC_CH3,IDC_CH4,IDC_CH5,IDC_CH6,IDC_CH7,IDC_CH8,\
IDC_CH9,IDC_CH10,IDC_CH11,IDC_CH12,IDC_CH13,IDC_CH14,IDC_CH15,IDC_CH16};

// IDs of post-processed (filtered) channel progress bars 
static const int g_PpBarId[] = {IDC_CHPP1,IDC_CHPP2,IDC_CHPP3,IDC_CHPP4,IDC_CHPP5,IDC_CHPP6,IDC_CHPP7,IDC_CHPP8};

// IDs of Joystick monitoring progress bars
static const int g_JoyBarId[] = {IDC_X,IDC_Y,IDC_Z,IDC_RX,IDC_RY,IDC_RZ,IDC_SL0,IDC_SL1};

// IDs of raw channel titles (1...8) 
static const int g_RawTitleId[] = 
{IDC_TXT_CH1,IDC_TXT_CH2,IDC_TXT_CH3,IDC_TXT_CH4,IDC_TXT_CH5,IDC_TXT_CH6,IDC_TXT_CH7,IDC_TXT_CH8,\
IDC_TXT_CH9, IDC_TXT_CH10, IDC_TXT_CH11,IDC_TXT_CH12,IDC_TXT_CH13,IDC_TXT_CH14,IDC_TXT_CH15,IDC_TXT_CH16};

// IDs of post-processed (filtered) titles (A...H) 
static const int g_PpTitleId[] = {IDC_TXT_CHPP1,IDC_TXT_CHPP2,IDC_TXT_CHPP3,IDC_TXT_CHPP4,IDC_TXT_CHPP5,IDC_TXT_CHPP6,IDC_TXT_CHPP7,IDC_TXT_CHPP8};

// IDs of joystick monitoring titles (X...SL1) 
static const int g_JoyTitleId[] = {IDC_TXT_X,IDC_TXT_Y,IDC_TXT_Z,IDC_TXT_RX,IDC_TXT_RY,IDC_TXT_RZ,IDC_TXT_SL0,IDC_TXT_SL1};

INT_PTR CALLBACK	MsgHndlTabDlg(HWND, UINT, WPARAM, LPARAM);

SppTab::SppTab(void)
{
}

SppTab::SppTab(HINSTANCE hInstance, HWND ParentWnd, int Id, DLGPROC MsgHndlDlg) : 
	m_hDlg(0),
	m_DlgId(-1),
	m_vRawBarId(g_RawBarId, g_RawBarId+sizeof(g_RawBarId)/ sizeof(int)),
	m_vPpBarId(g_PpBarId, g_PpBarId+sizeof(g_PpBarId)/ sizeof(int)),
	m_vJoyBarId(g_JoyBarId, g_JoyBarId+sizeof(g_JoyBarId)/ sizeof(int)),
	m_vRawTitleId(g_RawTitleId, g_RawTitleId+sizeof(g_RawTitleId)/ sizeof(int)),
	m_vJoyTitleId(g_JoyTitleId, g_JoyTitleId+sizeof(g_JoyTitleId)/ sizeof(int)),
	m_vPpTitleId(g_PpTitleId, g_PpTitleId+sizeof(g_PpTitleId)/ sizeof(int))
{
	m_hInstance = hInstance;
	m_TopDlgWnd = GetParent(ParentWnd);
	m_DlgId = Id;

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(m_DlgId), ParentWnd, MsgHndlDlg, (LPARAM)this);	

	return;
}

SppTab::~SppTab(void)
{
}

HWND SppTab::GetHandle(void)
{
	return m_hDlg;
}



int SppTab::GetId(void)
{
	return m_DlgId;
}

void SppTab::Reset()
{
}

void SppTab::Show()
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}
 
void SppTab::Hide()	
{
	ShowWindow(m_hDlg, SW_HIDE);
}

void SppTab::ShowArrayOfItems(HWND hDlg, int nCmdShow, const int items[], UINT size)
{
	for (UINT i=0; i<size; i++)
		ShowWindow(GetDlgItem(hDlg,  items[i]), nCmdShow);
}

// Show/Hide the first 'size' dialog items in a given vector if IDs
// If size=0 or if size too big then do it to the entire vector
void SppTab::ShowArrayOfItems(HWND hDlg, int nCmdShow, std::vector< int> items, UINT size)
{
	if (!size || size>items.size())
	{
		for (auto item : items)
			ShowWindow(GetDlgItem(hDlg,  item), nCmdShow);
	}
	else
	{
		for (UINT i=0; i<size; i++)
			ShowWindow(GetDlgItem(hDlg,  items[i]), nCmdShow);
	}
}

void SppTab::ShowArrayOfItems(HWND hDlg, bool Enable, const int items[], UINT size)
{
	for (UINT i=0; i<size; i++)
		EnableWindow(GetDlgItem(hDlg,  items[i]), Enable);
}

// Enable/Disable the first 'size' dialog items in a given vector if IDs - Disable the rest
// If size=0 or if size too big then do it to the entire vector
void SppTab::ShowArrayOfItems(HWND hDlg, bool Enable, std::vector< int> items, UINT size)
{
	if (!size || size>items.size())
	{
		for (auto item : items)
			EnableWindow(GetDlgItem(hDlg,  item), Enable);
	}
	else
	{
		for (auto item : items)
			EnableWindow(GetDlgItem(hDlg,  item), false);
		for (UINT i=0; i<size; i++)
			EnableWindow(GetDlgItem(hDlg,  items[i]), Enable);
	}

}

void SppTab::ResetArrayOfBars(HWND hDlg, const int items[], UINT size)
{
	for (UINT i=0; i<size; i++)
		SendMessage(GetDlgItem(hDlg,  items[i]), PBM_SETPOS, 0, 0);
}

void SppTab::ResetArrayOfBars(HWND hDlg, std::vector< int> items)
{
	for (auto item : items)
		SendMessage(GetDlgItem(hDlg,  item), PBM_SETPOS, 0, 0);
}

// Set the dialog box inside the tab control
void SppTab::SetPosition(HWND hDlg)
{
	SetWindowPos(hDlg, HWND_TOP,20,40, 0, 0,SWP_NOSIZE);
}

// Initialize array of progress-bars that desplay data such as channel position or axes
//
// hDlg  -	Handle to Dialog box (Tab)
// Color -	Color of bars in COLORREF (R,G,B)
// vBars -  Vector of IDs of progress-bars to initialize
// max   -	Upper progress-bar range  (Default is 0x03ff0000)
// mib   -	Low progress-bar range  (Default is 0)
void  SppTab::InitBars(HWND hDlg, const DWORD Color, std::vector< int> vBars,ULONG max,ULONG min)
{
	HWND hCh;

	for (auto id : vBars)
	{
		hCh = GetDlgItem(hDlg,  id);
		SetWindowTheme(hCh, L" ", L" ");
		SetWindowPos(hCh, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		SendMessage(hCh, PBM_SETRANGE ,min, max);	
		SendMessage(hCh, PBM_SETPOS, 0, 0);				// Reset
		SendMessage(hCh, PBM_SETBARCOLOR , 0, Color);	
	};

}

// Create one central  Tooltip object
HWND SppTab::CreateToolTip(HWND hDlg, const int arr[], int size)
{
	LRESULT  added, active;

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
	   m_vControls.assign(arr, arr+size);
	   for (auto ctrl : m_vControls)
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
// TitleStr:The title string
// Icon:	Icon to display. Possible values are TTI_NONE (default), TTI_INFO, TTI_WARNING, TTI_ERROR
void SppTab::DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, LPCTSTR TitleStr , int Icon)
{
	TCHAR ControlText[MAX_MSG_SIZE] ={0};
	TCHAR TitleText[MAX_MSG_SIZE] ={0};

	LoadString(m_hInstance, TxtID,  ControlText, MAX_MSG_SIZE);
	lpttt->lpszText = ControlText;

	SendMessage(m_hwndToolTip, TTM_SETTITLE, Icon, (LPARAM) TitleStr);
}

// Display callback-type tooltip
// lpttt:	Pointer to tooltip structure
// TxtID:	The resource ID of the text to display
// TitleID:	The resource ID of the title - default is "no title"
// Icon:	Icon to display. Possible values are TTI_NONE (default), TTI_INFO, TTI_WARNING, TTI_ERROR
void SppTab::DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, int TitleID, int Icon)
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

/*
	Should be never be called - only inhereted methods should be called
	Called every time mouse hovers over a control that was previously registered for tool tip
	Registration was done in CreateToolTip()
	The Control ID (CtrlId) of the control is extracted from the input 'param' 
	The correct text is displayed according to the Control ID
*/
void SppTab::UpdateToolTip(LPVOID param)
{
	LPNMTTDISPINFO lpttt = (LPNMTTDISPINFO)param;
	TCHAR ControlText[MAX_MSG_SIZE] ={0};
	TCHAR TitleText[MAX_MSG_SIZE] ={0};
	int ControlTextSize = 0;

	// Since the id field of the control in the tooltip was defined as a handle - it has to be converted back
	int CtrlId = GetDlgCtrlID((HWND)lpttt->hdr.idFrom);

	// Handle to the tooltip window
	HWND hToolTip = lpttt->hdr.hwndFrom;
	DisplayToolTip(lpttt, IDS_W_NOT_IMP, L"OOOPS!", TTI_WARNING);
	return;

}

INT_PTR CALLBACK MsgHndlTabDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppDlg * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppDlg *)lParam;
		return (INT_PTR)TRUE;

	case WM_NOTIFY:
		// Tooltips
		if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
		{
			DialogObj->UpdateToolTip((LPVOID)lParam);
			return  (INT_PTR)TRUE;
		};

		return (INT_PTR)TRUE;

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
