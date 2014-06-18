#include "stdafx.h"
#include "Commctrl.h"
#include "resource.h"
#include "public.h"
#include "smartpropoplus.h"
#include "JoyMonitorDlg.h"

// IDs of Joystick monitoring progress bars
static const int g_JoyBarId[] = {IDC_X,IDC_Y,IDC_Z,IDC_RX,IDC_RY,IDC_RZ,IDC_SL0,IDC_SL1};

// IDs of joystick monitoring titles (X...SL1) 
static const int g_JoyTitleId[] = {IDC_TXT_X,IDC_TXT_Y,IDC_TXT_Z,IDC_TXT_RX,IDC_TXT_RY,IDC_TXT_RZ,IDC_TXT_SL0,IDC_TXT_SL1};

INT_PTR CALLBACK	MsgHndlDlg(HWND, UINT, WPARAM, LPARAM);

CJoyMonitorDlg::CJoyMonitorDlg(void)
{
}

CJoyMonitorDlg::CJoyMonitorDlg(HINSTANCE hInstance, HWND	ConsoleWnd) : 
	m_hDlg(0),
	m_vJoyBarId(g_JoyBarId, g_JoyBarId+sizeof(g_JoyBarId)/ sizeof(int)),
	m_vJoyTitleId(g_JoyTitleId, g_JoyTitleId+sizeof(g_JoyTitleId)/ sizeof(int))
{
	m_hInstance = hInstance;
	m_ConsoleWnd = ConsoleWnd;
	m_nRawCh = 0;
	m_CurJoy = 1; // Current displayed joystick: Default is 1

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_JOY), NULL, MsgHndlDlg, (LPARAM)this);	
	CreateControls(128);


	return;
}

// Init the progress bars that monitor the feedback from the joystick
void CJoyMonitorDlg::InitJoyMonitor(HWND hDlg)
{	
	InitBars(hDlg, 0xFF, m_vJoyBarId,0xFFFF0000);
}

// Initialize array of progress-bars that desplay data such as channel position or axes
//
// hDlg  -	Handle to Dialog box (Tab)
// Color -	Color of bars in COLORREF (R,G,B)
// vBars -  Vector of IDs of progress-bars to initialize
// max   -	Upper progress-bar range  (Default is 0x03ff0000)
// mib   -	Low progress-bar range  (Default is 0)
void  CJoyMonitorDlg::InitBars(HWND hDlg, const DWORD Color, std::vector<const int> vBars,ULONG max,ULONG min)
{
	HWND hCh;

	for (auto id : vBars)
	{
		hCh = GetDlgItem(hDlg,  id);
		//SetWindowTheme(hCh, L" ", L" ");
		SetWindowPos(hCh, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		SendMessage(hCh, PBM_SETRANGE ,min, max);	
		SendMessage(hCh, PBM_SETPOS, 0, 0);				// Reset
		SendMessage(hCh, PBM_SETBARCOLOR , 0, Color);	
	};

}

CJoyMonitorDlg::~CJoyMonitorDlg(void)
{
}

HWND CJoyMonitorDlg::GetHandle(void)
{
	return m_hDlg;
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

	for (UINT i=1; i<=MAX_DISP_BUTTONS; i++)
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


// Update the position of the  progress bar that corresponds to the vJoy axis
void CJoyMonitorDlg::SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue)
{
	int IdItem;

	if (m_CurJoy != iDev)
		return;

	switch (Axis)
	{
	case HID_USAGE_X:
		IdItem = IDC_X;
		break;
	case HID_USAGE_Y:
		IdItem = IDC_Y;
		break;
	case HID_USAGE_Z:
		IdItem = IDC_Z;
		break;
	case HID_USAGE_RX:
		IdItem = IDC_RX;
		break;
	case HID_USAGE_RY:
		IdItem = IDC_RY;
		break;
	case HID_USAGE_RZ:
		IdItem = IDC_RZ;
		break;
	case HID_USAGE_SL0:
		IdItem = IDC_SL0;
		break;
	case HID_USAGE_SL1:
		IdItem = IDC_SL1;
		break;

	default:
		return;
	};

	HWND hCh = GetDlgItem(m_hDlg, IdItem);
	SendMessage(hCh, PBM_SETPOS, AxisValue, 0);
}

// Enable/disable controls according to vJoy device settings
void CJoyMonitorDlg::EnableControls(UINT id, controls * ctrl)
{
	UINT ch= IDC_X;
	HWND hCh, hEdt, hTtl;
	UINT iAxis=0;
	UINT edt = 0;

	////// Verify correct vJoy device
	HWND hCb = GetDlgItem(m_hDlg,IDC_VJOY_DEVICE);
	// Get the index of the selected vJoy device
	int index = 0; // TODO: (int)SendMessage(hCb,(UINT) CB_GETCURSEL  ,(WPARAM) 0,(LPARAM)0); 
	if (index == CB_ERR)
		return;

#if 0
	// Extract the device id from the item's data
	int SelId = (int)SendMessage(hCb,(UINT) CB_GETITEMDATA   ,(WPARAM) index,(LPARAM)0);
	if (id != SelId)
		return;

#endif // 0
	////// Verified

	// Go over all axes
	do 
	{
		// Axis bars
		hCh = GetDlgItem(m_hDlg,  ch);
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		ShowWindow(hCh, ctrl->axis[ch-IDC_X]);
		UpdateWindow(hCh);


		// Bar titles
		hTtl = GetDlgItem(m_hDlg,  m_vJoyTitleId[iAxis]);
		EnableWindow(hTtl, ctrl->axis[edt]);
		UpdateWindow(hTtl);

#if 0		
		// Map edit fields
		hEdt = GetDlgItem(m_hDlg,  edt);
		EnableWindow(hEdt, ctrl->axis[edt-IDC_SRC_X]);
		UpdateWindow(hEdt);

#endif // 0

		ch++;
		edt++;
		iAxis++;
	} while (ch<=IDC_SL1);

	// SendMessage(m_BtnsDlg->GetHandle(), VJOYDEV_SETAVAIL, id, (LPARAM)ctrl);
}


INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CJoyMonitorDlg * DialogObj = NULL;


	switch (message)
	{

	case WM_DESTROY:
			DestroyWindow(hDlg);
			hDlg = NULL;
			PostQuitMessage(0);
			return (INT_PTR)TRUE;

	case WM_CLOSE:
		DestroyWindow(hDlg);
		break;

	case WM_INITDIALOG:
		DialogObj = (CJoyMonitorDlg *)lParam;
		DialogObj->InitJoyMonitor(hDlg); // Init joystick monitor progress bar

		break;

	case WMSPP_JMON_AXIS:
		DialogObj->SetJoystickAxisData((UCHAR)(wParam&0xFF), (UINT)(wParam>>16), (UINT32)lParam);
		break;

	case VJOYDEV_SETAVAIL:
		DialogObj->EnableControls((UINT)wParam, (controls*)lParam);		
		break;

	}
	return (INT_PTR)FALSE;
}
