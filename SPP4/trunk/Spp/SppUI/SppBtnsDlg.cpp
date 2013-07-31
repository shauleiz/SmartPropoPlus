#include "stdafx.h"
#include "WinMessages.h"
#include "Commctrl.h"
#include "resource.h"
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
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
			hDlg = NULL;
			return (INT_PTR)TRUE;
		}


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

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_BUTTONSDLG), NULL, MsgHndlBtnDlg, (LPARAM)this);	

	// Populate dialog box
	CreateControls(32);
	// TODO - Remove later
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	

	return;
}

SppBtnsDlg::~SppBtnsDlg(void)
{
}

// Create controls on dialog box
// Four colums of 32 button-controls
// For each button dispaly Button number and an edit box
// The edit box will accept channel number (1-24)
void SppBtnsDlg::CreateControls(UINT nButtons)
{
	// Set the upper-left corner as starting point
	POINT OrigPt;
	OrigPt.x = OrigPt.y = 10;

	for (UINT i=1; i<=128; i++)
	{
		CreateButtonLable(i);
		CreateChannelEdit(i);
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
	CreateStatics(m_hDlg, m_hInstance, SS_SIMPLE, rc, 123456, caption.c_str());
}



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

	CreateEdit(m_hDlg, m_hInstance ,0, rc,122457+iButton,ch.c_str());
}


