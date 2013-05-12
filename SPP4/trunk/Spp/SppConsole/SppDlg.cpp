#include "stdafx.h"
#include "Windowsx.h"
#include "WinMessages.h"
#include "..\SppProcess\SppProcess.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "Commctrl.h"
#include "SppControl.h"
#include "SppDlg.h"

#define MAX_LOADSTRING 100

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

	// Create the dialog box (Hidden)
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg, (LPARAM)this);	

	// Add icon to system tray
	m_tnid.cbSize = 0;
	TaskBarAddIcon(IDI_SPPCONSOLE, CONSOLE_BALOON_TTL);
	return;
}

SppDlg::~SppDlg(void)
{
	// Remove notification icon
	if (m_tnid.cbSize)
		Shell_NotifyIcon(NIM_DELETE, &m_tnid);

}

void SppDlg::Show()
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}

void SppDlg::Hide()
{
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

// TaskBarAddIcon - adds an icon to the notification area. 
// Returns TRUE if successful, or FALSE otherwise. 
// uID - identifier of the icon 
// hicon - handle to the icon to add 
// lpszTip - tooltip text 
bool SppDlg::TaskBarAddIcon(UINT uID, LPTSTR lpszTip)
{
	bool res; 
    NOTIFYICONDATA tnid;
	HRESULT hr;
 
 	m_hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(uID));
    tnid.cbSize = sizeof(NOTIFYICONDATA); 
    tnid.hWnd = m_hDlg; 
    tnid.uID = uID; 
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
    tnid.uCallbackMessage = NULL /*MYWM_NOTIFYICON*/; 
    tnid.hIcon =	m_hIcon;// Load system tray icon

    if (lpszTip) 
        hr = StringCbCopyN(tnid.szTip, sizeof(tnid.szTip), lpszTip, 
                           sizeof(tnid.szTip));
        // TODO: Add error handling for the HRESULT.
    else 
        tnid.szTip[0] = (TCHAR)'\0'; 
 
    res = (TRUE == Shell_NotifyIcon(NIM_ADD, &tnid));
	if (res)
		m_tnid = tnid;
 
    if (m_hIcon) 
        DestroyIcon(m_hIcon); 
 
    return res; 
}

void  SppDlg::CleanAudioList(void)
{
	HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
	ListView_DeleteAllItems(hAudioList);
}

// Update the position of the progress bar that corresponds to the channel
void  SppDlg::SetRawChData(UINT iCh, UINT data)
{
	// Check if this channel is supported
	if (iCh > (IDC_CH8-IDC_CH1))
		return;

	HWND hCh = GetDlgItem(m_hDlg,  IDC_CH1+iCh);
	SendMessage(hCh, PBM_SETPOS, data, 0);

}

void SppDlg::AddLine2FilterListA(int iFilter, const char * FilterName)
{
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_LIST_FILTERS);
	//int pos = (int)SendMessage(hFilterList, LB_ADDSTRING, 0, (LPARAM)FilterName);
	//SendMessage(hFilterList, LB_SETITEMDATA, pos, (LPARAM) iFilter);

	DWORD exStyles = LVS_EX_CHECKBOXES;
	ListView_SetExtendedListViewStyleEx(hFilterList, exStyles, exStyles); // TODO: Move to initialization of dialog box

	// Convert to a wchar_t*
    size_t origsize = strlen(FilterName) + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t FilterNameW[newsize];
	mbstowcs_s(&convertedChars, FilterNameW, origsize, FilterName, _TRUNCATE); // Filter names are converted from ASCII to UNICODE

	// Insert filter name
	LV_ITEM item;
	item.mask = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE | LVIF_PARAM;
	item.iItem = 0;
	item.iSubItem = 0;
	item.pszText =  FilterNameW;
    item.stateMask = 0;
    item.iSubItem  = 0;
    item.state     = 0;
	item.lParam    = iFilter;
	int pos = ListView_InsertItem(hFilterList, &item);
	///

	//ListView_SetItemText(hFilterList, i, 1, TEXT("SI")); // TODO: Replace later with real stuff

}

void SppDlg::AddLine2ModList(MOD_STRUCT * mod)
{
	if (!mod)
		return;

	if (mod->isPpm)
	{ // PPM
		HWND hPPMList = GetDlgItem(m_hDlg,  IDC_LIST_PPM);
		int pos = (int)SendMessage(hPPMList, LB_ADDSTRING, 0, (LPARAM)mod->ModName);
		SendMessage(hPPMList, LB_SETITEMDATA, pos, (LPARAM) mod->ModType); 
		if (mod->ModSelect)
			SendMessage(hPPMList, LB_SETCURSEL , pos, 0); 
	}
	else
	{ // PCM
		HWND hPCMList = GetDlgItem(m_hDlg,  IDC_LIST_PCM);
		int pos = (int)SendMessage(hPCMList, LB_ADDSTRING, 0, (LPARAM)mod->ModName); 
		SendMessage(hPCMList, LB_SETITEMDATA, pos, (LPARAM) mod->ModType); 
		if (mod->ModSelect)
			SendMessage(hPCMList, LB_SETCURSEL , pos, 0); 
	};
}

// Tell the parent window (Main application)
// to stop/start monitoring the raw channel data
void  SppDlg::MonitorRawCh(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);
	int start = Button_GetCheck(hMonitorChCB);

	// Clear display
	UINT ch= IDC_CH1;
	HWND hCh;
	do 
	{
		hCh = GetDlgItem(m_hDlg,  ch);
		SendMessage(hCh, PBM_SETRANGE ,0, 0x03ff0000); // Range: 0-1023
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		ch++;
	} while (ch<=IDC_CH8);

	// Pass request
	SendMessage(m_ConsoleWnd, WMSPP_DLG_MONITOR , start, 0);
}

// Get selected filter fro GUI (if any) and send its filter index to parent window
void SppDlg::UpdateFilter(void)
{
	// Loop on all entries in filter list
	// Find the first checked entry (Assumption: Only one entry at most can be checked)
	// Send filter index  (-1 means no filter) parent
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_LIST_FILTERS);
	int count = ListView_GetItemCount(hFilterList);
	int iFilter = -1;
	LVITEM item;

	for (int i = 0; i<count; i++)
	{
		if (ListView_GetCheckState(hFilterList, i))
		{
			item.iItem = i;
			item.iSubItem = 0;
			item.mask = LVIF_PARAM;
			if (ListView_GetItem(hFilterList,&item))
				iFilter = item.lParam;
		}; // if
	}; // for

	SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)iFilter, 0);
}

void SppDlg::FilterListEvent(WPARAM wParam, LPARAM lParam)
{
	LPNMLISTVIEW change;
	//BOOL checked;
	HWND hFilterList;
	// int count;
	UINT ItemState = 0;

	switch (((LPNMHDR)lParam)->code)
	{
	case LVN_ITEMCHANGED:
		hFilterList = GetDlgItem(m_hDlg,  IDC_LIST_FILTERS);
		change = (LPNMLISTVIEW)lParam;
		//count = ListView_GetItemCount(hFilterList);

		// If checkbox was clicked then change the select state accordingly
		if (change->uNewState&0x1000 && change->uOldState&0x2000)
				ListView_SetItemState(hFilterList, change->iItem, 0, LVIS_SELECTED | LVIS_FOCUSED)
		else if (change->uNewState&0x2000 && change->uOldState&0x1000)
			ListView_SetItemState(hFilterList, change->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

		// The selected item only is checked
		if (change->uNewState&LVIS_SELECTED)
				ListView_SetCheckState(hFilterList, change->iItem, TRUE)
		else if (!(change->uNewState & (LVIS_SELECTED | 0x2000)))
				ListView_SetCheckState(hFilterList, change->iItem, FALSE);

		UpdateFilter();

		break;
	};
}

void  SppDlg::AddLine2AudioList(jack_info * jack)
{
	// Audio jack must have at least one audio channel (mono)
	HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
	if (!jack->nChannels)
		return;

	// Insert audio jack name
	LV_ITEM item;
	item.mask = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE;
	item.iItem = 0;
	item.iSubItem = 0;
	item.pszText = jack->FriendlyName;
    item.stateMask = 0;
    item.iSubItem  = 0;
    item.state     = 0;
	int i = ListView_InsertItem(hAudioList, &item);

	ListView_SetItemText(hAudioList, i, 1, TEXT("SI")); // TODO: Replace later with real stuff

	// Set the default jack as focused (and selected)
	if (jack->Default)
		ListView_SetItemState(hAudioList, i, 0xF|LVIS_FOCUSED, 0xF|LVIS_FOCUSED);


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

	};
}

// Message handler for spp dialog box.
INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppDlg * DialogObj = NULL;

	switch (message)
	{

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom  == IDC_LIST_FILTERS)
		{
			DialogObj->FilterListEvent(wParam, lParam);
		}
		return (INT_PTR)TRUE;

	case WM_INITDIALOG:
		DialogObj = (SppDlg *)lParam;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
			hDlg = NULL;
			PostQuitMessage(0);
			return (INT_PTR)TRUE;
		}

		if (HIWORD(wParam) == LBN_SELCHANGE )
		{
			DialogObj->SelChanged(LOWORD(wParam), (HWND)lParam);
			break;
		}

		if (LOWORD(wParam) == IDC_CH_MONITOR)
			DialogObj->MonitorRawCh(LOWORD(wParam));




		break;
		
	case REM_ALL_JACK:
		DialogObj->CleanAudioList();
		break;

	case POPULATE_JACKS:
		DialogObj->AddLine2AudioList((jack_info *)(wParam));
		break;

	case WMSPP_PRCS_SETMOD:
		DialogObj->AddLine2ModList((MOD_STRUCT *)(wParam));
		break;

	case WMSPP_PRCS_CHMNTR:
		DialogObj->SetRawChData(wParam, lParam);
		break;

	case FILTER_ADDA:
		DialogObj->AddLine2FilterListA((int)wParam, (const char *)lParam);


	}
	return (INT_PTR)FALSE;
}



HWND SppDlg::GetHandle(void)
{
	return m_hDlg;
}
