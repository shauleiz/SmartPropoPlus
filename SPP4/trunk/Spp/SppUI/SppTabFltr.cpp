#include "stdafx.h"
#include <vector>
#include "Windowsx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabFltr.h"

//Globals
const int g_BarId[] = {IDC_CH1,IDC_CH2,IDC_CH3,IDC_CH4,IDC_CH5,IDC_CH6,IDC_CH7,IDC_CH8};
const int g_TitleId[] = {IDC_TXT_CH1,IDC_TXT_CH2,IDC_TXT_CH3,IDC_TXT_CH4,IDC_TXT_CH5,IDC_TXT_CH6,IDC_TXT_CH7,IDC_TXT_CH8};
const int g_oBarId[] = {IDC_CHPP1,IDC_CHPP2,IDC_CHPP3,IDC_CHPP4,IDC_CHPP5,IDC_CHPP6,IDC_CHPP7,IDC_CHPP8};
const int g_oTitleId[] = {IDC_TXT_CHPP1,IDC_TXT_CHPP2,IDC_TXT_CHPP3,IDC_TXT_CHPP4,IDC_TXT_CHPP5,IDC_TXT_CHPP6,IDC_TXT_CHPP7,IDC_TXT_CHPP8};

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

#pragma region Transmitter & Post-processed (filtered) channel data Progress bars
// Init Raw channel progress bar
void SppTabFltr::MonitorCh(HWND hDlg)
{
	const DWORD Color = 0xFF00; // Green
	HWND hCh;

	for (auto id : g_BarId)
	{
		hCh = GetDlgItem(hDlg,  id);
		SendMessage(hCh, PBM_SETRANGE ,0, 0x03ff0000);	// Range: 0-1023
		SendMessage(hCh, PBM_SETPOS, 0, 0);				// Reset
		SendMessage(hCh, PBM_SETBARCOLOR , 0, Color);	// Green
	};
}

// Init the pospprocessed channel progress bars
void SppTabFltr::MonitorPpCh(HWND hDlg)
{
	const DWORD Color = 0xFF0000; // Blue
	HWND hCh;

	for (auto id : g_oBarId)
	{
		hCh = GetDlgItem(hDlg,  id);
		SendMessage(hCh, PBM_SETRANGE ,0, 0x03ff0000);	// Range: 0-1023
		SendMessage(hCh, PBM_SETPOS, 0, 0);				// Reset
		SendMessage(hCh, PBM_SETBARCOLOR , 0, Color);	// Green
	};
}


// Update data in one of the transmitter channel progress bars
void SppTabFltr::SetRawChData(UINT iCh, UINT data)
{

	// Check if this channel is supported
	UINT count = sizeof(g_BarId)/sizeof(int);
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  g_BarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}

// Update the position of the progress bar that corresponds to the channel
void SppTabFltr::SetProcessedChData(UINT iCh, UINT data)
{

	// Check if this channel is supported
	UINT count = sizeof(g_oBarId)/sizeof(int);
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  g_oBarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}

void SppTabFltr::ShowArrayOfItems(HWND hDlg, int nCmdShow, const int items[], UINT size)
{
	for (UINT i=0; i<size; i++)
		ShowWindow(GetDlgItem(hDlg,  items[i]), nCmdShow);
}

void SppTabFltr::ShowArrayOfItems(HWND hDlg, bool Enable, const int items[], UINT size)
{
	for (UINT i=0; i<size; i++)
		EnableWindow(GetDlgItem(hDlg,  items[i]), Enable);
}

void SppTabFltr::ShowChannelArea(HWND hDlg, int nCmdShow)
{
	ShowWindow(GetDlgItem(hDlg,  IDC_RAW_CHANNELS), nCmdShow);
	ShowWindow(GetDlgItem(hDlg,  IDC_OUT_CHANNELS), nCmdShow);
	ShowArrayOfItems( hDlg, nCmdShow, g_BarId, sizeof(g_BarId)/sizeof(int));
	ShowArrayOfItems( hDlg, nCmdShow, g_TitleId, sizeof(g_TitleId)/sizeof(int));
	ShowArrayOfItems( hDlg, nCmdShow, g_oBarId, sizeof(g_BarId)/sizeof(int));
	ShowArrayOfItems( hDlg, nCmdShow, g_oTitleId, sizeof(g_TitleId)/sizeof(int));
}

void SppTabFltr::ShowChannelArea(HWND hDlg, bool Enable)
{
	EnableWindow(GetDlgItem(hDlg, IDC_RAW_CHANNELS), Enable);
	EnableWindow(GetDlgItem(hDlg, IDC_OUT_CHANNELS), Enable);
	ShowArrayOfItems( hDlg, Enable, g_BarId, sizeof(g_BarId)/sizeof(int));
	ShowArrayOfItems( hDlg, Enable, g_TitleId, sizeof(g_TitleId)/sizeof(int));
	ShowArrayOfItems( hDlg, Enable, g_oBarId, sizeof(g_BarId)/sizeof(int));
	ShowArrayOfItems( hDlg, Enable, g_oTitleId, sizeof(g_TitleId)/sizeof(int));
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
		MessageBox(m_hDlg, TEXT("Illegal DLL File"), TEXT("Filter File"), MB_ICONERROR); // TODO: Re[lace strings
		return;
	}

	// Display File Name
	HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
	Edit_SetText(hFilterFile, (LPTSTR)info);
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
			Button_SetCheck(hFilterCB, BST_CHECKED);
			ShowChannelArea( m_hDlg, true);
			break;
		};
		i++;
	};
}

void SppTabFltr::InitFilter(int nFilters, LPTSTR FilterName)
{

	// Clear Filter display
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	SendMessage(hCombo,(UINT) CB_RESETCONTENT ,(WPARAM) 0,(LPARAM)0); 

	// If there are filters then prepare data for selection
	if (nFilters)
	{
		// Bring "-- Select Filter --" to top
		ComboBox_SetText(hCombo, TEXT("-- Select Filter --"));

		// Display File name
		HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
		Edit_SetText(hFilterFile, FilterName);
		UpdateWindow(hFilterFile);
	}
	else
	{
		ComboBox_Enable(hCombo, FALSE);
		HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
		Button_SetCheck(hFilterCB, BST_UNCHECKED);
		ShowChannelArea( m_hDlg, false);
	};
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

void SppTabFltr::EnableFilter(BOOL cb)
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
\
	};
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

	// Checks the checkbox
	HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	Button_SetCheck(hFilterCB, BST_CHECKED);
	ShowChannelArea( m_hDlg, true);
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


	default:
		break;

	};

	return (INT_PTR)FALSE;
}
