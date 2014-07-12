#include "stdafx.h"
#include "Windowsx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabAdv.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabAdvDlg(HWND, UINT, WPARAM, LPARAM);

static const int g_Controls[] = {
		/* Advanced Tab (IDD_ADVANCED ) */
		IDC_CH_LOG, IDC_CH_INSIG, IDC_CH_PULSE, IDC_PLS_SCOPE,
	};


SppTabAdv::SppTabAdv(void)
{
}


SppTabAdv::~SppTabAdv(void)
{
}


SppTabAdv::SppTabAdv(HINSTANCE hInstance, HWND TopDlgWnd) :  SppTab( hInstance,  TopDlgWnd,  IDD_ADVANCED, MsgHndlTabAdvDlg)
{
}

// Tell the parent window (Main application)
// to stop/start displaying pulse scope
void  SppTabAdv::PulseScope(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);

	// Pass request
	int start = Button_GetCheck(hMonitorChCB);
	SendMessage(m_TopDlgWnd, WMSPP_DLG_PLSSCOP , start, 0);
}

// Tell the parent window (Main application)
// to show/hide log window
void  SppTabAdv::ShowLogWindow(WORD cb)
{
	// Get data
	HWND hLogChCB = GetDlgItem(m_hDlg,  cb);
	int show = Button_GetCheck(hLogChCB);
	SendMessage(m_TopDlgWnd, WMSPP_DLG_LOG , show, 0);
}

void  SppTabAdv::RecordPulse(WORD cb)
{
	// Get data
	HWND hRecordPulseChCB = GetDlgItem(m_hDlg,  cb);
	int record = Button_GetCheck(hRecordPulseChCB);
	SendMessage(m_TopDlgWnd, WMSPP_DLG_PULSE , record, 0);

}

void  SppTabAdv::RecordInSignal(WORD cb)
{
	// Get data
	HWND hRecordInSignalChCB = GetDlgItem(m_hDlg,  cb);
	int record = Button_GetCheck(hRecordInSignalChCB);
	SendMessage(m_TopDlgWnd, WMSPP_DLG_INSIG , record, 0);

}

INT_PTR CALLBACK MsgHndlTabAdvDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabAdv * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabAdv *)lParam;
		DialogObj->SetPosition(hDlg) ;
		DialogObj->CreateToolTip(hDlg, g_Controls, sizeof(g_Controls)/sizeof(int)); // Initialize tooltip object
		return (INT_PTR)TRUE;

	case WM_COMMAND:
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

	default:
		break;

	};

	return (INT_PTR)FALSE;
}

