#include "stdafx.h"
#include "Windowsx.h"
#include "WinMessages.h"
#include "..\SppMain\SppMain.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "Commctrl.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"

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

SppConsoleDlg::SppConsoleDlg(void)
{
}

SppConsoleDlg::SppConsoleDlg(HINSTANCE hInstance, HWND	ConsoleWnd)
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

SppConsoleDlg::~SppConsoleDlg(void)
{
	// Remove notification icon
	if (m_tnid.cbSize)
		Shell_NotifyIcon(NIM_DELETE, &m_tnid);

}

void SppConsoleDlg::Show()
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}

void SppConsoleDlg::Hide()
{
	ShowWindow(m_hDlg, SW_HIDE);
}

void SppConsoleDlg::RegisterEndEvent(HANDLE * h)
{
	m_hEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	*h = m_hEndEvent;
}


DWORD WINAPI StartDlg(LPVOID hInstance)
{
	return (DWORD)DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg);
}

bool SppConsoleDlg::MsgLoop(void)
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
bool SppConsoleDlg::TaskBarAddIcon(UINT uID, LPTSTR lpszTip)
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

void  SppConsoleDlg::CleanAudioList(void)
{
	HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
	ListView_DeleteAllItems(hAudioList);
}

// Update the position of the progress bar that corresponds to the channel
void  SppConsoleDlg::SetRawChData(UINT iCh, UINT data)
{
	// Check if this channel is supported
	if (iCh > (IDC_LAST-IDC_CH1))
		return;

	HWND hCh = GetDlgItem(m_hDlg,  IDC_CH1+iCh);
	SendMessage(hCh, PBM_SETPOS, data, 0);

}

void SppConsoleDlg::AddLine2ModList(MOD_STRUCT * mod)
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
void  SppConsoleDlg::MonitorRawCh(WORD cb)
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
	} while (ch<=IDC_LAST);

	// Pass request
	SendMessage(m_ConsoleWnd, CH_MONITOR , start, 0);
}

void  SppConsoleDlg::AddLine2AudioList(jack_info * jack)
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

void SppConsoleDlg::SelChanged(WORD ListBoxId, HWND hListBox)
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
		SendMessage(m_ConsoleWnd, MOD_CHANGED, (WPARAM)mod, 0);

	};
}

// Message handler for spp dialog box.
INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppConsoleDlg * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppConsoleDlg *)lParam;
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

	case SET_MOD_INFO:
		DialogObj->AddLine2ModList((MOD_STRUCT *)(wParam));
		break;

	case WMAPP_CH_MNTR:
		DialogObj->SetRawChData(wParam, lParam);
		break;

	}
	return (INT_PTR)FALSE;
}



HWND SppConsoleDlg::GetHandle(void)
{
	return m_hDlg;
}
