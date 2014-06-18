#include "stdafx.h"
#include "resource.h"
#include "JoyMonitorDlg.h"

INT_PTR CALLBACK	MsgHndlDlg(HWND, UINT, WPARAM, LPARAM);

CJoyMonitorDlg::CJoyMonitorDlg(void)
{
}

CJoyMonitorDlg::CJoyMonitorDlg(HINSTANCE hInstance, HWND	ConsoleWnd)
{
	m_hInstance = hInstance;
	m_ConsoleWnd = ConsoleWnd;

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_JOY), NULL, MsgHndlDlg, (LPARAM)this);	
	CreateControls(32);


	return;
}


CJoyMonitorDlg::~CJoyMonitorDlg(void)
{
}


// Create controls on dialog box
// Two colums of 8 button-controls
// For each button dispaly Button number and an edit box
// The edit box will accept channel number (1-24)
// Unimplemented buttons will be seen but disabled
void CJoyMonitorDlg::CreateControls(UINT nButtons)
{
	// Set the upper-left corner as starting point
	POINT OrigPt;
	OrigPt.x = 10;
	OrigPt.y = 100;

	for (UINT i=1; i<=MAX_BUTTONS; i++)
	{
		CreateButtonLable(i);
		CreateIndicator(i);
	};
}

// Create a static text control of type "Buttonxx"
// Place control in columns of 32
void CJoyMonitorDlg::CreateButtonLable(UINT iButton)
{

	// Constants
	UINT RowSpace = ROWSPACE;			// Space between rows
	UINT ColSpace = COLSPACE;		// Space between columns
	RECT rc = {20,130,100,160};	// Text rectangle

	// Location
	UINT iCol = (iButton-1)/16; // Zero-based column index
	UINT iRow = (iButton-1)%16; // Zero-based row index
	rc.top+=iRow*RowSpace;
	rc.bottom+=iRow*RowSpace;
	rc.left+=iCol*ColSpace;
	rc.right+=iCol*ColSpace;

	wstring caption = TEXT("") + to_wstring(iButton);
	CreateStatics(m_hDlg, m_hInstance, 0 , rc, ID_BASE_STATIC+iButton, caption.c_str());
}

void CJoyMonitorDlg::CreateIndicator(UINT iButton)
{
		// Constants
	UINT RowSpace = ROWSPACE;			// Space between rows
	UINT ColSpace = COLSPACE;		// Space between columns
	RECT rc = {10,133,19,140};	// Text rectangle

	// Location
	UINT iCol = (iButton-1)/16; // Zero-based column index
	UINT iRow = (iButton-1)%16; // Zero-based row index
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

#if 0
// Create a edit control for channel number
// Place control in columns of 32 to the right of the button number
void CJoyMonitorDlg::CreateChannelEdit(UINT iButton)
{
	// Constants
	UINT RowSpace = ROWSPACE;			// Space between rows
	UINT ColSpace = COLSPACE;		// Space between columns
	RECT rc = {85,10,120,30};	// Text rectangle

	// Location
	UINT iCol = (iButton-1)/8; // Zero-based column index
	UINT iRow = (iButton-1)%8; // Zero-based row index
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

#endif // 0

// BAsed on http://winapi.foosyerdoos.org.uk/code/usercntrls/htm/createstatics.php
HWND CJoyMonitorDlg::CreateStatics(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle,
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
		rc.bottom-rc.top,                    //height
		hParent,                      //parent window handle
		//control's ID
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
		hInst,                        //application instance
		0);                           //user defined info
}


INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CJoyMonitorDlg * DialogObj = NULL;


	switch (message)
	{

	//case WM_PAINT:
	//	DialogObj->OnPaint();
	//break;

	case WM_DESTROY:
			DestroyWindow(hDlg);
			hDlg = NULL;
//			PostQuitMessage(0);
			return (INT_PTR)TRUE;

	case WM_INITDIALOG:
		DialogObj = (CJoyMonitorDlg *)lParam;
	}
	return (INT_PTR)FALSE;
}
