#include "stdafx.h"
#include <vector>
#include "Windowsx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabDcdr.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabDcdrDlg(HWND, UINT, WPARAM, LPARAM);

static const int g_Controls[] = {
		/* Transmitter Tab (IDD_DECODE) */
		IDC_DEC_AUTO, IDC_BTN_SCAN, IDC_LIST_PPM, IDC_LIST_PCM, 
		IDC_CH1, IDC_CH2, IDC_CH3, IDC_CH4, IDC_CH5, IDC_CH6, IDC_CH7, IDC_CH8, IDC_CH9, IDC_CH10, IDC_CH11, IDC_CH12, IDC_CH13, IDC_CH14, IDC_CH15, IDC_CH16
	};


SppTabDcdr::SppTabDcdr(void)
{
}

SppTabDcdr::SppTabDcdr(HINSTANCE hInstance, HWND TopDlgWnd) : 
	SppTab( hInstance,  TopDlgWnd,  IDD_DECODE, MsgHndlTabDcdrDlg)
{	
	
}

SppTabDcdr::~SppTabDcdr(void)
{
}

// Request to reset this tab to its default values
// Auto checked
void SppTabDcdr::Reset(void)
{

	// Check IDC_AUD_AUTO
	CheckDlgButton(m_hDlg,  IDC_DEC_AUTO , BST_CHECKED);
	AutoDecParams();
}


#pragma region Encoding detection and auto selection
// CU informed of state of Auto-detection of decoder
// If Automode true then:
// - Check the checkbox (IDC_DEC_AUTO)
// - Hide scan button (IDC_BTN_SCAN)
// If Automode false then:
// - Un-Check the checkbox
// - Show scan button
void SppTabDcdr::DecoderAuto(bool automode)
{
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
void SppTabDcdr::AutoDecParams(void)
{
	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_DEC_AUTO))
		SendMessage(m_TopDlgWnd, WMSPP_DLG_AUTO, AUTODECODE, 1);
	else
		SendMessage(m_TopDlgWnd, WMSPP_DLG_AUTO, AUTODECODE, 0);
}

//  Button SCAN was pressed
void SppTabDcdr::ScanEncoding(void)
{
	// Notify CU that it SCAN button was pressed.
	SendMessage(m_TopDlgWnd, WMSPP_DLG_SCAN , 0, 0);
}

// Update the decoder quality value in the GUI
void SppTabDcdr::SetDecoderQuality(UINT Quality)
{
	static UINT prevVal=100;

	// Prevent flicker
	if (prevVal == Quality)
		return;
	prevVal = Quality;

	// Update text of static frame
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_SIG_DEC);
	wstring txt = L"Signal Decoder ( Quality: " + to_wstring(Quality) + L")";
	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());
}


#pragma endregion


#pragma region PPM/PCM Lists of decoders
// Populate the lists of decoders: PPM list & PCM list
void SppTabDcdr::AddLine2DcdrList(MOD * mod, LPCTSTR SelType)
{
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
			SendMessage(hPPMList, LB_SETCURSEL , pos, 0); 
	}
	else
	{ // PCM
		HWND hPCMList = GetDlgItem(m_hDlg,  IDC_LIST_PCM);
		if (SendMessage(hPCMList, LB_FINDSTRINGEXACT , -1, (LPARAM)mod->Name) != LB_ERR)
			return;

		int pos = (int)SendMessage(hPCMList, LB_ADDSTRING, 0, (LPARAM)mod->Name); 
		SendMessage(hPCMList, LB_SETITEMDATA, pos, (LPARAM) mod->Type); 
		if (!wcscmp(SelType,mod->Type))
			SendMessage(hPCMList, LB_SETCURSEL , pos, 0); 
	};
}

void SppTabDcdr::SelChanged(WORD ListBoxId, HWND hListBox)
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

		// Set new decoder
		SendMessage(m_TopDlgWnd, WMSPP_DLG_MOD, (WPARAM)mod, 0);

		// Make sure decoder selection is manual
		SendMessage(m_TopDlgWnd, WMSPP_DLG_AUTO, AUTODECODE, 0);
	};

}

// Button pressed: Reply was received from CU - Now change GUI to match
void SppTabDcdr::SelectDecoder(LPCTSTR Decoder)
{
	int list[2] = {IDC_LIST_PPM, IDC_LIST_PCM};
	HWND hList;
	int count=0;
	LPCTSTR iData;
	int SelList=-1, SelItem=-1;

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

#pragma endregion

#pragma region Transmitter channel data Progress bars

// Init Raw channel progress bar
void SppTabDcdr::MonitorCh(HWND hDlg)
{
	InitBars(hDlg, 0xFF00, m_vRawBarId);
}

// Update data in one of the transmitter channel progress bars
void SppTabDcdr::SetRawChData(UINT iCh, UINT data)
{

	// Check if this channel is supported
	size_t count = m_vRawBarId.size();
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  m_vRawBarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}

// Update the number of raw channels
void SppTabDcdr::SetNumberRawCh(UINT nCh)
{
	static UINT prevVal=100;

	// Prevent flicker
	if (prevVal == nCh)
		return;
	prevVal = nCh;

	// Update text of static frame
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS);
	wstring txt = L"R/C Channels (" + to_wstring(nCh) + L")";
	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());

	// Enable bars and titles for the existing channels only
	ShowArrayOfItems( m_hDlg, true, m_vRawBarId,nCh);
	ShowArrayOfItems( m_hDlg, true, m_vRawTitleId,nCh);
}
#pragma endregion

/*
	Called every time mouse hovers over a control that was previously registered for tool tip
	Registration was done in CreateToolTip()
	The Control ID (CtrlId) of the control is extracted from the input 'param' 
	The correct text is displayed according to the Control ID
*/
void SppTabDcdr::UpdateToolTip(LPVOID param)
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

	case IDC_DEC_AUTO:	// Decoder Type: Auto selection (Recommended)
		DisplayToolTip(lpttt, IDS_I_DEC_AUTO, IDS_T_DEC_AUTO);
		break;

	case IDC_BTN_SCAN:  // Decoder Type: Evaluate signal for correct type
		DisplayToolTip(lpttt, IDS_I_BTN_SCAN, IDS_T_BTN_SCAN);
		break;

	case IDC_LIST_PPM:	// PPM Transmitters: Select a decoder that matches you Transmitter
		DisplayToolTip(lpttt, IDS_I_LIST_PPM, IDS_T_LIST_PPM);
		break;

	case IDC_LIST_PCM:  // PCM Transmitters: Select a decoder that matches you Transmitter
		DisplayToolTip(lpttt, IDS_I_LIST_PCM, IDS_T_LIST_PCM);
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
	case IDC_CH9: 
		DisplayToolTip(lpttt, IDS_I_CH9);
		break;
	case IDC_CH10: 
		DisplayToolTip(lpttt, IDS_I_CH10);
		break;
	case IDC_CH11: 
		DisplayToolTip(lpttt, IDS_I_CH11);
		break;
	case IDC_CH12: 
		DisplayToolTip(lpttt, IDS_I_CH12);
		break;
	case IDC_CH13: 
		DisplayToolTip(lpttt, IDS_I_CH13);
		break;
	case IDC_CH14: 
		DisplayToolTip(lpttt, IDS_I_CH14);
		break;
	case IDC_CH15: 
		DisplayToolTip(lpttt, IDS_I_CH15);
		break;
	case IDC_CH16:
		DisplayToolTip(lpttt, IDS_I_CH16);
		break;


	default:
		DisplayToolTip(lpttt, IDS_W_NOT_IMP, L"OOOPS", TTI_WARNING);
		break;
	}
}

INT_PTR CALLBACK MsgHndlTabDcdrDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabDcdr * DialogObj = NULL;

	switch (message)
	{

	case WM_INITDIALOG:
		DialogObj = (SppTabDcdr *)lParam;
		DialogObj->SetPosition(hDlg) ;
		DialogObj->MonitorCh(hDlg) ;
		DialogObj->CreateToolTip(hDlg, g_Controls, sizeof(g_Controls)/sizeof(int)); // Initialize tooltip object
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == LBN_SELCHANGE )
		{
			DialogObj->SelChanged(LOWORD(wParam), (HWND)lParam);
			break;
		}

		if  (LOWORD(wParam)  == IDC_DEC_AUTO) 
		{
			DialogObj->AutoDecParams();
			break;
		}
		if (LOWORD(wParam)  == IDC_BTN_SCAN && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->ScanEncoding();
			break;
		}

		return (INT_PTR)TRUE;

	case WM_NOTIFY:
		// Tooltips
		if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
		{
			DialogObj->UpdateToolTip((LPVOID)lParam);
			return  (INT_PTR)TRUE;
		};

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
