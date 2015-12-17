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
		IDC_CH_LOG, IDC_CH_INSIG, IDC_CH_PULSE, IDC_PLS_SCOPE, IDC_BTN_RESET,
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

// Update check box
void  SppTabAdv::SetPulseScope(bool cb)
{
	// Set checkbox
	HWND hChkBox = GetDlgItem(m_hDlg, IDC_PLS_SCOPE);
	if (!hChkBox)
		return;

	if (cb)
		Button_SetCheck(hChkBox, BST_CHECKED);
	else
		Button_SetCheck(hChkBox, BST_UNCHECKED);
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


// Reset button was pressed pass it to the parent
void  SppTabAdv::ResetRequest(void)
{
	SendMessage(m_TopDlgWnd, WMSPP_DLG_RST , 0, 0);
}

void SppTabAdv::Reset()
{
	//Clear check box IDC_PLS_SCOPE
	HWND hLogChCB = GetDlgItem(m_hDlg,  IDC_PLS_SCOPE);
	Button_SetCheck(hLogChCB, BST_UNCHECKED);
 
	PulseScope(IDC_PLS_SCOPE);
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

/*
	Called every time mouse hovers over a control that was previously registered for tool tip
	Registration was done in CreateToolTip()
	The Control ID (CtrlId) of the control is extracted from the input 'param' 
	The correct text is displayed according to the Control ID
*/
void SppTabAdv::UpdateToolTip(LPVOID param)
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
	case IDC_CH_LOG:
		DisplayToolTip(lpttt, IDS_I_CH_LOG, IDS_T_CH_LOG);
		break;

	case IDC_CH_INSIG:
		DisplayToolTip(lpttt, IDS_I_CH_INSIG, IDS_T_CH_INSIG);
		break;

	case IDC_CH_PULSE:
		DisplayToolTip(lpttt, IDS_I_CH_PULSE, IDS_T_CH_PULSE);
		break;

	case IDC_PLS_SCOPE:
		DisplayToolTip(lpttt, IDS_I_PLS_SCOPE, IDS_T_PLS_SCOPE);
		break;

	case IDC_BTN_RESET:
		DisplayToolTip(lpttt, IDS_I_BTN_RESET, IDS_T_BTN_RESET);
		break;

	default:
		DisplayToolTip(lpttt, IDS_W_NOT_IMP, L"OOOPS", TTI_WARNING);
		break;
	}

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

		if (LOWORD(wParam) == IDC_BTN_RESET  && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->ResetRequest();
			break;
		};

	break;

	case WM_NOTIFY:
		// Tooltips
		if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
		{
			DialogObj->UpdateToolTip((LPVOID)lParam);
			return  (INT_PTR)TRUE;
		};
 		break;

	default:
		break;

	};

	return (INT_PTR)FALSE;
}

