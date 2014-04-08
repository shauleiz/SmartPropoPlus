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
	UINT count = m_vRawBarId.size();
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  m_vRawBarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}

#pragma endregion

INT_PTR CALLBACK MsgHndlTabDcdrDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabDcdr * DialogObj = NULL;

	switch (message)
	{

	case WM_INITDIALOG:
		DialogObj = (SppTabDcdr *)lParam;
		DialogObj->SetPosition(hDlg) ;
		DialogObj->MonitorCh(hDlg) ;
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

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
