#include "stdafx.h"
#include <string.h>
#include "Windowsx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabFltr.h"

//Globals
const int g_TitleId[] = {IDC_TXT_CH1,IDC_TXT_CH2,IDC_TXT_CH3,IDC_TXT_CH4,IDC_TXT_CH5,IDC_TXT_CH6,IDC_TXT_CH7,IDC_TXT_CH8};
const int g_oTitleId[] = {IDC_TXT_CHPP1,IDC_TXT_CHPP2,IDC_TXT_CHPP3,IDC_TXT_CHPP4,IDC_TXT_CHPP5,IDC_TXT_CHPP6,IDC_TXT_CHPP7,IDC_TXT_CHPP8};
static const int g_Controls[] = {
		IDC_EDIT_FILTERFILE, IDC_COMBO_FILTERS, IDC_BTN_FILTERBROWSE, IDC_CH_FILTER, 
		IDC_CH1, IDC_CH2, IDC_CH3, IDC_CH4, IDC_CH5, IDC_CH6, IDC_CH7, IDC_CH8,
		IDC_CHPP1, IDC_CHPP2, IDC_CHPP3, IDC_CHPP4, IDC_CHPP5, IDC_CHPP6, IDC_CHPP7, IDC_CHPP8
	};

INT_PTR CALLBACK	MsgHndlTabFltrDlg(HWND, UINT, WPARAM, LPARAM);


SppTabFltr::SppTabFltr(void)
{
}

SppTabFltr::SppTabFltr(HINSTANCE hInstance, HWND TopDlgWnd) : SppTab( hInstance,  TopDlgWnd,  IDD_FILTER, MsgHndlTabFltrDlg)
{
}


SppTabFltr::~SppTabFltr(void)
{
}

// Request to reset this tab to its default values
// Bitrate = Auto
// Channel = Auto
void SppTabFltr::Reset(void)
{

	// Clear edit-box IDC_EDIT_FILTERFILE
	HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
	const char * emptyStr = "";
	Edit_SetText(hFilterFile, TEXT(""));
	UpdateWindow(hFilterFile);

	// Clear Comb-box IDC_COMBO_FILTERS
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	ComboBox_ResetContent(hCombo);

	//// UnCheck "Filter Active" IDC_CH_FILTER
	//HWND hFilterCB = GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	//Button_SetCheck(hFilterCB, BST_UNCHECKED);

	// Call InitFilter() with empty filter file and 0 filters (Might need modifications)
	InitFilter(0, NULL);
	EnableFilter(IDC_CH_FILTER);

}

#pragma region Transmitter & Post-processed (filtered) channel data Progress bars
// Init Raw channel progress bar
void SppTabFltr::MonitorCh(HWND hDlg)
{
	InitBars(hDlg, 0xFF00, m_vRawBarId);
}

// Init the pospprocessed channel progress bars
void SppTabFltr::MonitorPpCh(HWND hDlg)
{
	InitBars(hDlg, 0xFF0000, m_vPpBarId);
}

void SppTabFltr::SetNumberProcCh(UINT nCh)
{
	m_nProcCh = nCh;
}

// Update data in one of the transmitter channel progress bars
void SppTabFltr::SetRawChData(UINT iCh, UINT data)
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

// Update the position of the progress bar that corresponds to the channel
void SppTabFltr::SetProcessedChData(UINT iCh, UINT data)
{

	// Check if this channel is supported
	size_t count =  m_vPpBarId.size();
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  m_vPpBarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}


void SppTabFltr::ShowChannelArea(HWND hDlg, int nCmdShow)
{
	ShowWindow(GetDlgItem(hDlg,  IDC_RAW_CHANNELS), nCmdShow);
	ShowWindow(GetDlgItem(hDlg,  IDC_OUT_CHANNELS), nCmdShow);
	ShowArrayOfItems( hDlg, nCmdShow, m_vRawBarId);
	ShowArrayOfItems( hDlg, nCmdShow, g_TitleId, sizeof(g_TitleId)/sizeof(int));
	ShowArrayOfItems( hDlg, nCmdShow, m_vPpBarId);
	ShowArrayOfItems( hDlg, nCmdShow, g_oTitleId, sizeof(g_TitleId)/sizeof(int));
	ResetArrayOfBars(hDlg, m_vRawBarId);
	ResetArrayOfBars(hDlg, m_vPpBarId);
}

void SppTabFltr::ShowChannelArea(HWND hDlg, bool Enable)
{
	EnableWindow(GetDlgItem(hDlg, IDC_RAW_CHANNELS), Enable);
	EnableWindow(GetDlgItem(hDlg, IDC_OUT_CHANNELS), Enable);
	ShowArrayOfItems( hDlg, Enable, m_vRawBarId);
	ShowArrayOfItems( hDlg, Enable, g_TitleId, sizeof(g_TitleId)/sizeof(int));
	ShowArrayOfItems( hDlg, Enable, m_vPpBarId);
	ShowArrayOfItems( hDlg, Enable, g_oTitleId, sizeof(g_TitleId)/sizeof(int));
	ResetArrayOfBars(hDlg, m_vRawBarId);
	ResetArrayOfBars(hDlg, m_vPpBarId);
}


#pragma endregion


#pragma region Selection of Filter file and Filter
// Respond to Browse button
// If file selected then sends message WMSPP_DLG_FLTRFILE with full path to selected file
// CU tests file - if valid then file name (NOT full path) is displayed
void SppTabFltr::OnFilterFileBrowse(void)
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
	info = SendMessage(m_TopDlgWnd, WMSPP_DLG_FLTRFILE , (WPARAM)ofn.lpstrFile, 0);
	if (!info)
	{
		MessageBox(m_hDlg, TEXT("Illegal DLL File"), TEXT("Filter File"), MB_ICONERROR); // TODO: Replace strings
		return;
	}

	// Display File Name
	HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
	Edit_SetText(hFilterFile, (LPTSTR)info);
	_tcscpy_s(&(m_FilterFileName[0]), sizeof(m_FilterFileName)/sizeof(TCHAR),(LPTSTR)info);
	UpdateWindow(hFilterFile);
	delete[] (LPVOID)info;
}

// Set the selected filter to be displayed in the filter Combo Box
void SppTabFltr::SelFilter(int FilterId)
{
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
			EnableWindow(hFilterCB, true);
			Button_SetCheck(hFilterCB, BST_CHECKED);
			ShowChannelArea( m_hDlg, true);
			
			// Inform Parent
			ComboBox_GetLBText (hCombo,i,m_FilterName);
			m_FilterActive = true;
			break;
		};
		i++;
	};

	SentFilterInfo2Parent();
}

void SppTabFltr::InitFilter(int nFilters, LPTSTR FilterName)
{

	// Clear Filter display
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	SendMessage(hCombo,(UINT) CB_RESETCONTENT ,(WPARAM) 0,(LPARAM)0); 

	HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	// If there are filters then prepare data for selection
	if (nFilters)
	{
		// Bring "-- Select Filter --" to top
		ComboBox_SetText(hCombo, TEXT("-- Select Filter --"));

		// Display File name
		HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
		_tcscpy_s(&(m_FilterFileName[0]), sizeof(m_FilterFileName)/sizeof(TCHAR),FilterName);
		Edit_SetText(hFilterFile, FilterName);
		UpdateWindow(hFilterFile);
	}
	else
	{
		ComboBox_Enable(hCombo, FALSE);		
		Button_SetCheck(hFilterCB, BST_UNCHECKED);
		ShowChannelArea( m_hDlg, false);
		m_FilterActive = false;
		SentFilterInfo2Parent();
	};

	EnableWindow(hFilterCB, false);
}

void SppTabFltr::AddLine2FilterListA(int FilterID, const char * FilterName)
{
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);

	ShowWindow(hFilterList, SW_SHOW);
	ShowWindow(GetDlgItem(m_hDlg,  IDC_CH_FILTER), SW_SHOW);

	ComboBox_Enable(hFilterList, TRUE);
	// Convert to a wchar_t*
	size_t origsize = strlen(FilterName) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t FilterNameW[newsize];
	mbstowcs_s(&convertedChars, FilterNameW, origsize, FilterName, _TRUNCATE); // Filter names are converted from ASCII to UNICODE

	int index = (int)SendMessage(hFilterList,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)FilterNameW ); 
	SendMessage(hFilterList,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)FilterID ); 
}

void SppTabFltr::AddLine2FilterListW(int FilterID, LPCWSTR FilterName)
{
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	ShowWindow(hFilterList, SW_SHOW);
	ShowWindow(GetDlgItem(m_hDlg,  IDC_CH_FILTER), SW_SHOW);

	ComboBox_Enable(hFilterList, TRUE);

	int index = (int)SendMessage(hFilterList,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)FilterName ); 
	SendMessage(hFilterList,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)FilterID ); 
}

void SppTabFltr::EnableFilter(int cb)
{
	// Get check state
	HWND hCB = GetDlgItem(m_hDlg,  cb);
	int Enable = Button_GetCheck(hCB);

	// If checked then this is equivalent to selecting the current selected
	// If Un-Checked then send -1 as selected filter
	if (Enable)
	{
		UpdateFilter();
		ShowChannelArea( m_hDlg, true);
	}
	else
	{
		ShowChannelArea( m_hDlg, false);
		SendMessage(m_TopDlgWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
	};

	m_FilterActive = (Enable != 0);
	SentFilterInfo2Parent();

}

// Get selected filter fro GUI (if any) and send its filter index to parent window
void SppTabFltr::UpdateFilter(void)
{
	// Send the ID (in data) of the selected item to the parent window
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	int FilterId; 
	int iCurSel = ComboBox_GetCurSel(hFilterList);
	if (iCurSel==CB_ERR)
	{
		SendMessage(m_TopDlgWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
		return;
	}

	FilterId = (int)ComboBox_GetItemData (hFilterList, iCurSel);
	SendMessage(m_TopDlgWnd, WMSPP_DLG_FILTER, (WPARAM)FilterId, 0);

	// Inform Parent
	ComboBox_GetLBText (hFilterList,iCurSel,m_FilterName);


	// Checks the checkbox
	HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	EnableWindow(hFilterCB, true);
	Button_SetCheck(hFilterCB, BST_CHECKED);
	ShowChannelArea( m_hDlg, true);
	m_FilterActive = true;
	SentFilterInfo2Parent();
}

#pragma endregion


// Initialize Filters section
void SppTabFltr::InitFilterDisplay(HWND hDlg)
{
	// Get handles to the controls
	HWND hFilterFile	= GetDlgItem(hDlg,  IDC_EDIT_FILTERFILE);
	HWND hFilterCB		= GetDlgItem(hDlg,  IDC_CH_FILTER);
	HWND hFilters		= GetDlgItem(hDlg,  IDC_COMBO_FILTERS);
	HWND hInputCh		= GetDlgItem(hDlg,  IDC_RAW_CHANNELS);
	HWND hOutputCh		= GetDlgItem(hDlg,  IDC_OUT_CHANNELS);

	// Clear Filter File, Unselect checkbox and gray-out Selected Filters
	Edit_SetText(hFilterFile, TEXT("Select Filter File"));
	Button_SetCheck(hFilterCB, BST_UNCHECKED);
	ComboBox_Enable(hFilters, FALSE);

	// Just remove everything
	ShowWindow(hFilterCB, SW_HIDE);
	ShowWindow(hFilters, SW_HIDE);
	ShowChannelArea( hDlg, false);

	// Initialize status
	m_FilterActive = false;
	m_FilterFileName[0] = NULL;
	m_FilterName[0] = NULL;

}

// Inform parent window of the currently selected filter
void SppTabFltr::SentFilterInfo2Parent(void)
{
	if (m_FilterActive)
		SendMessage(m_TopDlgWnd, WMSPP_DLG_FLTR , (WPARAM)m_FilterFileName, (LPARAM)m_FilterName);
	else
		SendMessage(m_TopDlgWnd, WMSPP_DLG_FLTR , (WPARAM)NULL, (LPARAM)NULL);
}

/*
	Called every time mouse hovers over a control that was previously registered for tool tip
	Registration was done in CreateToolTip()
	The Control ID (CtrlId) of the control is extracted from the input 'param' 
	The correct text is displayed according to the Control ID
*/
void SppTabFltr::UpdateToolTip(LPVOID param)
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
	case IDC_EDIT_FILTERFILE: 
		DisplayToolTip(lpttt, IDS_I_EDIT_FILTERFILE, IDS_T_EDIT_FILTERFILE);
		break;

	case IDC_BTN_FILTERBROWSE: 
		DisplayToolTip(lpttt, IDS_I_BTN_FILTERBROWSE, IDS_T_BTN_FILTERBROWSE);
		break;

	case IDC_CH_FILTER: 
		DisplayToolTip(lpttt, IDS_I_CH_FILTER);
		break;

	case IDC_COMBO_FILTERS:
		DisplayToolTip(lpttt, IDS_I_COMBO_FILTERS, IDS_T_COMBO_FILTERS);
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


	default:
		DisplayToolTip(lpttt, IDS_W_NOT_IMP, L"OOOPS", TTI_WARNING);
		break;
	}
}

INT_PTR CALLBACK MsgHndlTabFltrDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabFltr * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabFltr *)lParam;
		DialogObj->SetPosition(hDlg) ;
		DialogObj->MonitorCh(hDlg) ;
		DialogObj->MonitorPpCh(hDlg) ;
		DialogObj->CreateToolTip(hDlg, g_Controls, sizeof(g_Controls)/sizeof(int)); // Initialize tooltip object
		DialogObj->InitFilterDisplay(hDlg); // Initialize Filter section of the GUI
		return (INT_PTR)TRUE;

	case WM_COMMAND:

		if (LOWORD(wParam)  == IDC_BTN_FILTERBROWSE && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->OnFilterFileBrowse();
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
