#include "stdafx.h"
#include "Windowsx.h"
#include "WinMessages.h"
#include "Commctrl.h"
#include "resource.h"
#include "SmartPropoPlus.h"
#include "SppBtnsDlg.h"

// Message handler for spp dialog box.
INT_PTR CALLBACK MsgHndlBtnDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppBtnsDlg * DialogObj = NULL;

	switch (message)
	{

	case WM_NOTIFY:

		if (((LPNMHDR)lParam)->idFrom  == IDC_BTN_MAP)
		{
			return  (INT_PTR)TRUE;
		}

		return (INT_PTR)TRUE;


	case WM_INITDIALOG:
		DialogObj = (SppBtnsDlg *)lParam;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			ShowWindow(DialogObj->GetHandle(), SW_HIDE);

			// If OK -> Send new button mapping to parent window for further processing
			if (LOWORD(wParam) == IDOK)
				DialogObj->SendButtonsMappingData();

			//DestroyWindow(hDlg);
			//hDlg = NULL;
			//return (INT_PTR)TRUE;
		};

		if (LOWORD(wParam) == ID_APPLY)
		{
			DialogObj->SendButtonsMappingData();
			break;
		}

		break;

	//case WM_CTLCOLOREDIT:
	//		
	//	break;


	case WMSPP_MAPBTN_UPDT:
		DialogObj->SetButtonsMappingData((BTNArr*)wParam, (UINT)lParam);
		break;

	case WMSPP_MAPBTN_SEND:
		DialogObj->SendButtonsMappingData();
		break;

	case VJOYDEV_SETAVAIL:
		DialogObj->EnableControls((UINT)wParam, (controls*)lParam);
		break;


	case WMSPP_JMON_BTN:
		DialogObj->SetButtonValues((UINT)wParam, (BTNArr *)lParam);
		break;

	}
	return (INT_PTR)FALSE;
}


SppBtnsDlg::SppBtnsDlg(void)
{
}

SppBtnsDlg::SppBtnsDlg(HINSTANCE hInstance, HWND	ParentWnd)
{
	m_hInstance = hInstance;
	m_ParentWnd = ParentWnd;
	m_ahEdtBtn.fill(NULL);
	m_BtlVals.fill(0);

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_BUTTONSDLG), NULL, MsgHndlBtnDlg, (LPARAM)this);	

	// Populate dialog box
	CreateControls(MAX_BUTTONS);

	// Test
	array<BYTE,MAX_BUTTONS> map = {0};
	int s = CreateBtnMap(map);

	//::ShowWindow(m_hDlg, SW_SHOW);
	//UpdateWindow(m_hDlg);	

	return;
}

SppBtnsDlg::~SppBtnsDlg(void)
{
}

void SppBtnsDlg::Show(void)
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}

HWND SppBtnsDlg::GetHandle(void)
{
	return m_hDlg;
}

// Create an array of button mapping data and send it to the parent window
// Return the number of buttons in map
void SppBtnsDlg::SendButtonsMappingData(void)
{
	TCHAR text[5];
	HWND hEditCh;
	UINT count=0;

	BTNArr aButtonMap;
	aButtonMap.fill(0);

	auto size = m_ahEdtBtn.size();
	for (UINT i=0; i<size; i++)
	{
		hEditCh = m_ahEdtBtn[i];
		if (!hEditCh)
			break;
				
		// Get data from edit box
		Edit_GetText(m_ahEdtBtn[i], text, 5);
		aButtonMap[i] = (UINT)_ttoi(text);
		count++;
	};

	// Send message to parent window
	SendMessage(m_ParentWnd, WMSPP_DLG_MAPBTN, (WPARAM)&aButtonMap, count);
}

// Fill-in the actual button-mapping data
void SppBtnsDlg::SetButtonsMappingData(BTNArr* aButtonMap, UINT nButtons)
{
	HWND hEditCh;

	for (UINT i=0; i<nButtons; i++)
	{
		hEditCh = m_ahEdtBtn[i];
		if (!hEditCh)
			break;

		// if entry is zero then ignore it
		if (!(*aButtonMap)[i])
			continue;

		// Set the new string
		wstring ch = to_wstring((*aButtonMap)[i]);
		Edit_SetText(m_ahEdtBtn[i], ch.c_str());
	};

}

// Create controls on dialog box
// Two colums of 8 button-controls
// For each button dispaly Button number and an edit box
// The edit box will accept channel number (1-24)
// Unimplemented buttons will be seen but disabled
void SppBtnsDlg::CreateControls(UINT nButtons)
{
	// Set the upper-left corner as starting point
	POINT OrigPt;
	OrigPt.x = OrigPt.y = 10;

	for (UINT i=1; i<=MAX_BUTTONS; i++)
	{
		CreateButtonLable(i);
		CreateChannelEdit(i);
		CreateIndicator(i);
	};
}

// BAsed on http://winapi.foosyerdoos.org.uk/code/usercntrls/htm/createstatics.php
HWND SppBtnsDlg::CreateStatics(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle,
							   const RECT& rc,const int id,const wstring& caption)
{
	dwStyle|=WS_CHILD|WS_VISIBLE;
	return CreateWindowEx(0,          //extended styles
		_T("static"),                 //control 'class' name
		(LPCTSTR)caption.c_str(),     //control caption
		dwStyle,                      //control style 
		rc.left,                      //position: left
		rc.top,                       //position: top
		rc.right-rc.left,              //width
		rc.bottom,                    //height
		hParent,                      //parent window handle
		//control's ID
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
		hInst,                        //application instance
		0);                           //user defined info
}

// Based on http://winapi.foosyerdoos.org.uk/code/usercntrls/htm/createedits.php
HWND SppBtnsDlg::CreateEdit(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption)
{
	dwStyle|=WS_CHILD|WS_VISIBLE;
	return CreateWindowEx(
		WS_EX_CLIENTEDGE,             //extended styles
		_T("edit"),                   //control 'class' name
		caption.c_str(),              //control caption
		dwStyle,                      //control style 
		rc.left,                      //position: left
		rc.top,                       //position: top
		rc.right-rc.left,                     //width
		rc.bottom-rc.top,                    //height
		hParent,                      //parent window handle
		//control's ID
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
		hInst,                        //application instance
		0);                           //user defined info
}
// Create a static text control of type "Buttonxx"
// Place control in columns of 32
void SppBtnsDlg::CreateButtonLable(UINT iButton)
{
	// Constants
	UINT RowSpace = ROWSPACE;			// Space between rows
	UINT ColSpace = COLSPACE;		// Space between columns
	RECT rc = {10,10,80,30};	// Text rectangle

	// Location
	UINT iCol = (iButton-1)/32; // Zero-based column index
	UINT iRow = (iButton-1)%32; // Zero-based row index
	rc.top+=iRow*RowSpace;
	rc.bottom+=iRow*RowSpace;
	rc.left+=iCol*ColSpace;
	rc.right+=iCol*ColSpace;

	wstring caption = TEXT("Button") + to_wstring(iButton);
	CreateStatics(m_hDlg, m_hInstance, SS_SIMPLE, rc, ID_BASE_STATIC+iButton, caption.c_str());
}

void SppBtnsDlg::CreateIndicator(UINT iButton)
{
		// Constants
	UINT RowSpace = ROWSPACE;			// Space between rows
	UINT ColSpace = COLSPACE;		// Space between columns
	RECT rc = {0,13,9,30};	// Text rectangle

	// Location
	UINT iCol = (iButton-1)/32; // Zero-based column index
	UINT iRow = (iButton-1)%32; // Zero-based row index
	rc.top+=iRow*RowSpace;
	rc.bottom+=iRow*RowSpace;
	rc.left+=iCol*ColSpace;
	rc.right+=iCol*ColSpace;

	// Green
	HWND hGreenImage= CreateStatics(m_hDlg, m_hInstance, SS_BITMAP, rc, ID_BASE_GREENDOT+iButton, L"");
	HANDLE hGreenImage1 =  LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_GREYDOT), IMAGE_BITMAP,0, 0,  LR_DEFAULTSIZE );
	SendMessage(hGreenImage,STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hGreenImage1);

	//Red
	HWND hRedImage= CreateStatics(m_hDlg, m_hInstance, SS_BITMAP, rc, ID_BASE_REDDOT+iButton, L"");
	HANDLE hRedImage1 =  LoadImage(m_hInstance, MAKEINTRESOURCE(IDB_REDDOT), IMAGE_BITMAP,0, 0,  LR_DEFAULTSIZE );
	SendMessage(hRedImage,STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hRedImage1);
}

// Create a edit control for channel number
// Place control in columns of 32 to the right of the button number
void SppBtnsDlg::CreateChannelEdit(UINT iButton)
{
	// Constants
	UINT RowSpace = ROWSPACE;			// Space between rows
	UINT ColSpace = COLSPACE;		// Space between columns
	RECT rc = {85,10,120,30};	// Text rectangle

	// Location
	UINT iCol = (iButton-1)/32; // Zero-based column index
	UINT iRow = (iButton-1)%32; // Zero-based row index
	rc.top+=iRow*RowSpace;
	rc.bottom+=iRow*RowSpace;
	rc.left+=iCol*ColSpace;
	rc.right+=iCol*ColSpace;

	wstring ch;
	if (iButton<=24)
		ch = to_wstring(iButton+8);
	else
		ch = to_wstring(9);

	HWND hCh = CreateEdit(m_hDlg, m_hInstance ,0, rc,ID_BASE_CH+iButton,ch.c_str());
	m_ahEdtBtn[iButton-1] = hCh;
}

// Go over the edit boxes and get the data
// Create button-map out of them
// Return the size of the array or -1 if error
int SppBtnsDlg::CreateBtnMap(array<BYTE,MAX_BUTTONS>& BtnMap)
{
	auto size = BtnMap.size();
	HWND hEditCh;
	int textsize, n=0;
	TCHAR text[5];

	for (UINT i=0; i<size; i++)
	{
		hEditCh = m_ahEdtBtn[i];
		if (!hEditCh)
			break;

		n++;
		textsize = Edit_GetText(hEditCh, (LPTSTR)&text, 5);
		BtnMap[i] = _tstoi(text);;
	};

	return n;
}


void SppBtnsDlg::SetButtonValues(UINT id, BTNArr * BtnVals)
{
	HWND  hRedDot, hGreenDot, hEdit;

	//return;

	for (UINT i=0; i<MAX_BUTTONS; i++)
	{
		hGreenDot = GetDlgItem(m_hDlg,ID_BASE_GREENDOT+i+1);
		hRedDot = GetDlgItem(m_hDlg,ID_BASE_REDDOT+i+1);
		hEdit = GetDlgItem(m_hDlg,ID_BASE_CH+i+1);
		LONG StyleEdit = GetWindowLong( hEdit, GWL_STYLE);
		if (!(StyleEdit & WS_VISIBLE))
			continue;

		if ((*BtnVals)[i])
		{
			ShowWindow(hGreenDot, SW_HIDE);
			ShowWindow(hRedDot, SW_SHOW);
		}
		else
		{
			ShowWindow(hGreenDot, SW_SHOW);
			ShowWindow(hRedDot, SW_HIDE);
		};
	};

}

void SppBtnsDlg::EnableControls(UINT id, controls * ctrl)
{
	HWND hEdit, hLable, hRedDot, hGreenDot;

	for (UINT i=0; i<=MAX_BUTTONS; i++)
	{
		hEdit = GetDlgItem(m_hDlg,ID_BASE_CH+i);
		hLable = GetDlgItem(m_hDlg,ID_BASE_STATIC+i);
		hGreenDot = GetDlgItem(m_hDlg,ID_BASE_GREENDOT+i);
		hRedDot = GetDlgItem(m_hDlg,ID_BASE_REDDOT+i);
		if (ctrl->nButtons < i)
		{
			ShowWindow(hEdit, SW_HIDE);
			ShowWindow(hLable, SW_HIDE);
			ShowWindow(hGreenDot, SW_HIDE);
			ShowWindow(hRedDot, SW_HIDE);
		}
		else
		{
			ShowWindow(hEdit, SW_SHOW);
			ShowWindow(hLable, SW_SHOW);
			ShowWindow(hGreenDot, SW_SHOW);
			ShowWindow(hRedDot, SW_SHOW);
		};
	}
}
