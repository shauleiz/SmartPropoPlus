#include "stdafx.h"
#include "Windowsx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabFltr.h"

//Globals
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
	};
}

void SppTabFltr::AddLine2FilterListA(int FilterID, const char * FilterName)
{
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);

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
		UpdateFilter();
	else
		SendMessage(m_TopDlgWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
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
}


// Initialize Filters section
void SppTabFltr::InitFilterDisplay(HWND hDlg)
{
	// Get handles to the controls
	HWND hFilterFile	= GetDlgItem(hDlg,  IDC_EDIT_FILTERFILE);
	HWND hFilterCB		= GetDlgItem(hDlg,  IDC_CH_FILTER);
	HWND hFilters		= GetDlgItem(hDlg,  IDC_COMBO_FILTERS);

	// Clear Filter File, Unselect checkbox and gray-out Selected Filters
	Edit_SetText(hFilterFile, TEXT("Select Filter File"));
	Button_SetCheck(hFilterCB, BST_UNCHECKED);
	ComboBox_Enable(hFilters, FALSE);
}

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
		return;

	// Display File Name
	HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
	Edit_SetText(hFilterFile, (LPTSTR)info);
	UpdateWindow(hFilterFile);
	delete[] (LPVOID)info;
}


INT_PTR CALLBACK MsgHndlTabFltrDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabFltr * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabFltr *)lParam;
		DialogObj->SetPosition(hDlg) ;
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
