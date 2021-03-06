/***************************************************************************
	Log class for SPP

	This class contains a log window (Hidden by default)
	The window visibility is controlled by SppControl

	When visible:
	Log message displays:
	- Message string
	- Severity
	- Source module

***************************************************************************/
#include "stdafx.h"
#include <Richedit.h>
#include "WinMessages.h"
#include "Resource.h"
#include "SppLog.h"

INT_PTR CALLBACK  DlgAudioLog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

SppLog::SppLog(void) : m_hLogDlg(NULL)
{
}

SppLog::SppLog(HINSTANCE hInstance, HWND	ConsoleWnd)
{
	m_hInstance = hInstance;
	m_ConsoleWnd = ConsoleWnd;
	m_hLogDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_LOGDLG), NULL, DlgAudioLog,  (LPARAM)this);
	//SetWindowLong(m_hLogDlg, GWL_STYLE, WS_CHILD);

}

SppLog::~SppLog(void)
{
}

void SppLog::Show()
{
	ShowWindow(m_hLogDlg, SW_SHOW);
	UpdateWindow(m_hLogDlg);	
}

void SppLog::Hide()
{
	ShowWindow(m_hLogDlg, SW_HIDE);
}

void SppLog::LogAudioUnit(int Code, int source, int Severity, LPVOID Data)
{
	TCHAR pBuf[1000] = { NULL };
	int len;

	if (!m_hLogDlg)
		return;

	// Initialize
	LRESULT lr;
	SYSTEMTIME SystemTime;
	HWND hEdit=NULL;
	WCHAR prefix[6], prtcode[20], src[20], PrtTime[30];
	GETTEXTLENGTHEX tl;
	CHARFORMAT cf;
	tl.codepage =  CP_ACP;
	tl.flags = GTL_DEFAULT;
	hEdit = GetDlgItem(m_hLogDlg, IDC_EDIT_LOG);
	SendMessage(hEdit,EM_SHOWSCROLLBAR    , (WPARAM)SB_VERT, TRUE);

	// Severity to colour and Prefix
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = 0;
	switch (Severity)
	{
	case ERR:
		cf.crTextColor = RGB(180,0,0);
		wmemcpy(prefix,L"[E] ",5);
		break;
	case FATAL:
		cf.crTextColor = RGB(250,0,0);
		wmemcpy(prefix,L"[F] ",5);
		break;
	case WARN:
		cf.crTextColor = RGB(255,140,0);
		wmemcpy(prefix,L"[W] ",5);
		break;
	default:
		cf.dwEffects = CFE_AUTOCOLOR;
		wmemcpy(prefix,L"[I] ",5);
	}

	switch (source)
	{
	case WMSPP_LOG_CNTRL:
		wmemcpy(src,L"<Cntrl ",sizeof(L"<Cntrl ")/sizeof(TCHAR));
		break;
	case WMSPP_LOG_AUDIO:
		wmemcpy(src, L"<Audio ", sizeof(L"<Audio ") / sizeof(TCHAR));
		break;
	case WMSPP_LOG_CONFIG:
		wmemcpy(src, L"<Confg ", sizeof(L"<Confg ") / sizeof(TCHAR));
		break;
	case WMSPP_LOG_PRSC:
		wmemcpy(src,L"<Prcss ",sizeof(L"<Prcss ")/sizeof(TCHAR));
		break;
	default:
		wmemcpy(src,L"<????? ",sizeof(L"<????? ")/sizeof(TCHAR));
	}


	SendMessage(hEdit,EM_SETCHARFORMAT, (WPARAM)SCF_SELECTION, (LPARAM)&cf);


	// Calculate Time
	GetLocalTime(&SystemTime);
	swprintf_s(PrtTime, 30, L"%02u:%02u:%02u.%03u : ", SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);

	// Print one Line

	// Time
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)PrtTime);

	// Prefix
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)prefix);
	// Source
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)src);
	// Code
	swprintf_s(prtcode, 10,L"%d>: ", Code);
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)prtcode);
	// Code String
	len = LoadString(m_hInstance, Code, reinterpret_cast< LPWSTR >(&pBuf), sizeof(pBuf) / sizeof(TCHAR));
	if (len)
	{
		lr = SendMessage(hEdit, EM_GETTEXTLENGTHEX, (WPARAM)&tl, 0);
		SendMessage(hEdit, EM_SETSEL, lr, lr);
		SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)(LPCWSTR)pBuf);
	}
	// Data	
	if (Data)
	{
		lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
		SendMessage(hEdit,EM_SETSEL    , lr, lr);
		SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)(LPCWSTR)Data); 
	}
	
	// New line
	lr = SendMessage(hEdit,EM_GETTEXTLENGTHEX   , (WPARAM)&tl,0);
	SendMessage(hEdit,EM_SETSEL    , lr, lr);
	SendMessage(hEdit,EM_REPLACESEL     , TRUE, (LPARAM)L"\r\n");

	// Scroll to bottom
	SendMessage(hEdit,EM_SCROLLCARET       , 0, 0);
}

HWND SppLog::GetWndHandle(void)
{
	return m_hLogDlg;
}

INT_PTR CALLBACK  DlgAudioLog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static SppLog * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppLog *)lParam;
		return (INT_PTR)TRUE;

	case WM_DESTROY:
		EndDialog(hDlg, IDCANCEL);
		hDlg = NULL;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDC_BTN_CLOSELOG)
		{
			
			SendMessage(DialogObj->m_ConsoleWnd, WMSPP_DLG_LOG, 0, 0);
			return (INT_PTR)TRUE;
		}
		break;

	case WMSPP_LOG_CNTRL+INFO:
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	case WMSPP_LOG_CNTRL+WARN:
	case WMSPP_LOG_CNTRL+ERR:
	case WMSPP_LOG_CNTRL+FATAL:
		DialogObj->LogAudioUnit((int)wParam, WMSPP_LOG_CNTRL, message-WMSPP_LOG_CNTRL, (LPVOID)lParam);
		break;

	case WMSPP_LOG_PRSC+INFO:
	case WMSPP_LOG_PRSC+WARN:
	case WMSPP_LOG_PRSC+ERR:
	case WMSPP_LOG_PRSC+FATAL:
		DialogObj->LogAudioUnit((int)wParam, WMSPP_LOG_PRSC, message-WMSPP_LOG_PRSC, (LPVOID)lParam);
		break;

	case WMSPP_LOG_AUDIO + INFO:
	case WMSPP_LOG_AUDIO + WARN:
	case WMSPP_LOG_AUDIO + ERR:
	case WMSPP_LOG_AUDIO + FATAL:
		DialogObj->LogAudioUnit((int)wParam, WMSPP_LOG_AUDIO, message - WMSPP_LOG_AUDIO, (LPVOID)lParam);
		break;

	case WMSPP_LOG_CONFIG + INFO:
	case WMSPP_LOG_CONFIG + WARN:
	case WMSPP_LOG_CONFIG + ERR:
	case WMSPP_LOG_CONFIG + FATAL:
		DialogObj->LogAudioUnit((int)wParam, WMSPP_LOG_CONFIG, message - WMSPP_LOG_CONFIG, (LPVOID)lParam);
		break;

	}
	return (INT_PTR)FALSE;
}