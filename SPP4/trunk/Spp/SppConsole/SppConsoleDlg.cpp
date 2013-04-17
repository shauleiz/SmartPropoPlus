#include "stdafx.h"
#include "WinMessages.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
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

SppConsoleDlg::SppConsoleDlg(HINSTANCE hInstance)
{
	m_hInstance = hInstance;

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
}

void  SppConsoleDlg::AddLine2AudioList(jack_info * jack)
{
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
		break;
		
	case REM_ALL_JACK:
		DialogObj->CleanAudioList();
		break;

	case POPULATE_JACKS:
		DialogObj->AddLine2AudioList((jack_info *)(wParam));
		break;

	}
	return (INT_PTR)FALSE;
}



HWND SppConsoleDlg::GetHandle(void)
{
	return m_hDlg;
}
