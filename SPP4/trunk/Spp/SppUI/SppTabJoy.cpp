#include "stdafx.h"
#include "Windowsx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "public.h"
#include "SppBtnsDlg.h"
#include "SppTabJoy.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabJoyDlg(HWND, UINT, WPARAM, LPARAM);
const int g_RawTitleId[] = {IDC_TXT_CH1,IDC_TXT_CH2,IDC_TXT_CH3,IDC_TXT_CH4,IDC_TXT_CH5,IDC_TXT_CH6,IDC_TXT_CH7,IDC_TXT_CH8};
const int g_PpTitleId[] = {IDC_TXT_CHPP1,IDC_TXT_CHPP2,IDC_TXT_CHPP3,IDC_TXT_CHPP4,IDC_TXT_CHPP5,IDC_TXT_CHPP6,IDC_TXT_CHPP7,IDC_TXT_CHPP8};


SppTabJoy::SppTabJoy(void)
{
}

SppTabJoy::SppTabJoy(HINSTANCE hInstance, HWND TopDlgWnd) : SppTab( hInstance,  TopDlgWnd,  IDD_JOY, MsgHndlTabJoyDlg)
{
	m_nProcCh = 100; // Improbable number of Post-processed channels
}

SppTabJoy::~SppTabJoy(void)
{
}

#pragma region Joystick Combo box
// Remove all vJoy Entries
void  SppTabJoy::vJoyRemoveAll()
{
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	SendMessage(hCombo,(UINT) CB_RESETCONTENT ,(WPARAM) 0,(LPARAM)0); 
}

// Add vJoy device entry to combo box
// Set the id as item data
void  SppTabJoy::vJoyDevAdd(UINT id)
{
	wstring vjoyid = L"vJoy " + to_wstring(id);
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	int index = (int)SendMessage(hCombo,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)(vjoyid.data()) ); 
	SendMessage(hCombo,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)id ); 
}

// Set the selected vJoy device
void  SppTabJoy::vJoyDevSelect(UINT id)
{
	wstring vjoyid = L"vJoy " + to_wstring(id);
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	int index = (int)SendMessage(hCombo,(UINT) CB_FINDSTRINGEXACT ,(WPARAM) -1,(LPARAM)(vjoyid.data()) ); 
	if (index == CB_ERR)
		return;
	index =  (int)SendMessage(hCombo,(UINT) CB_SETCURSEL ,(WPARAM) index, 0); 
}

// Get the selected vJoy device
// Extract the device id from the item's data
// Send device id to CU
void  SppTabJoy::vJoySelected(HWND hCb)
{
	// Get the index of the selected vJoy device
	int index = (int)SendMessage(hCb,(UINT) CB_GETCURSEL  ,(WPARAM) 0,(LPARAM)0); 
	if (index == CB_ERR)
		return;

	// Extract the device id from the item's data
	int id = (int)SendMessage(hCb,(UINT) CB_GETITEMDATA   ,(WPARAM) index,(LPARAM)0);
	if (id == CB_ERR)
		return;

	// Send device id to CU
	SendMessage(m_TopDlgWnd, WMSPP_DLG_VJOYSEL, (WPARAM)id, 0);
}

#pragma endregion


#pragma region Progress Bars

// Init the progress bars that monitor the feedback from the joystick
void SppTabJoy::InitJoyMonitor(HWND hDlg)
{	
	InitBars(hDlg, 0xFF, m_vJoyBarId);
}

// Init the pospprocessed channel progress bars
void SppTabJoy::MonitorPpCh(HWND hDlg)
{
	InitBars(hDlg, 0xFF0000, m_vPpBarId);
}

// Init Raw channel progress bar
void SppTabJoy::MonitorCh(HWND hDlg)
{
	InitBars(hDlg, 0xFF00, m_vRawBarId);
}

// Update the position of the  progress bar that corresponds to the vJoy axis
void SppTabJoy::SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue)
{
	int IdItem;

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

// Update the position of the progress bar that corresponds to the processed channel
void SppTabJoy::SetProcessedChData(UINT iCh, UINT data)
{
	// Check if this channel is supported
	UINT count = m_vPpBarId.size();
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  m_vPpBarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}

// Update the position of the progress bar that corresponds to the raw channel
void SppTabJoy::SetRawChData(UINT iCh, UINT data)
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

// Get the number of Post-processed (Filtered) channels
// Zero means - filter not used
void SppTabJoy::SetNumberProcCh(UINT nCh)
{
	// If Number of Post-processed (Filtered) channels did not change - Do Nothing
	if (m_nProcCh == nCh)
		return;

	// If nCh is 0 then hide PP bars and frame and show Raw bars and frame
	// If m_nProcCh is 0 then do the oposite
	if (!nCh)
	{
		ShowWindow(GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS), SW_SHOW);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vRawBarId);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vRawTitleId);
		ShowWindow(GetDlgItem(m_hDlg,  IDC_SPP_OUT), SW_HIDE);
		ShowArrayOfItems( m_hDlg, SW_HIDE, m_vPpBarId);
		ShowArrayOfItems( m_hDlg, SW_HIDE, m_vPpTitleId);
	}
	else
	{
		ShowWindow(GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS), SW_HIDE);
		ShowArrayOfItems( m_hDlg, SW_HIDE, m_vRawBarId);
		ShowArrayOfItems( m_hDlg, SW_HIDE, m_vRawTitleId);
		ShowWindow(GetDlgItem(m_hDlg,  IDC_SPP_OUT), SW_SHOW);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vPpBarId);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vPpTitleId);
	}
	ResetArrayOfBars(m_hDlg, m_vRawBarId);
	ResetArrayOfBars(m_hDlg, m_vPpBarId);

	// Change
	m_nProcCh = nCh;
}

// Update the frame text of the vJoy device vJoy axes
void SppTabJoy::SetJoystickDevFrame(UCHAR iDev)
{
	static UINT id=0;
	if (id == iDev)
		return;

	id = iDev;
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_VJOY_AXES);
	wstring txt = L"vJoy device " + to_wstring(iDev) + L" - Axis data";

	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());
}

#pragma endregion

#pragma region Mapping of Joystick buttons and axes
// Fill-in the actual mapping data
void SppTabJoy::SetMappingData(Mapping * Map)
{

	UINT id = IDC_SRC_SL1;
	HWND hEdtBox;
	UINT channel;
	TCHAR buffer[4];
	UINT& nAxes = Map->nAxes;
	DWORD& AxisMap = *Map->pAxisMap;

	// Go through the map and read nibble by nibble
	// Every nibble read goes to the corresponding edit box
	for (UINT i=0; i<nAxes; i++)
	{
		hEdtBox = GetDlgItem(m_hDlg,  id-i);
		channel = ((AxisMap>>(i*4))&0xF);
		_itot_s(channel, buffer, 2, 10);
		Edit_SetText(hEdtBox, buffer);
	};

	SetButtonsMappingData(Map->ButtonArray, Map->nButtons);
}

// Fill-in the actual button-mapping data - pass message to button-mapping dialog
void SppTabJoy::SetButtonsMappingData(BTNArr* aButtonMap, UINT nButtons)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_MAPBTN_UPDT,(WPARAM)aButtonMap, nButtons);
}

// Mapping button clicked
// Send all mapping info to the control unit
void  SppTabJoy::vJoyMapping(void)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_MAPBTN_SEND,0, 0);	
}

// Relay the actual button-mapping data - pass message to parent
void SppTabJoy::SendMappingData(BTNArr* aButtonMap, UINT nButtons)
{
	// Buttons
	Mapping m;
	m.nButtons = nButtons;
	m.ButtonArray = aButtonMap;


	// Axes
	TCHAR Buffer[4];
	int nTchar;
	int out=0;
	UINT id = IDC_SRC_X;
	DWORD AxesMap = 0;

	// Go on all entries and get value. Empty is considered as channel 0
	// The value has to be one/two digits - is pusshed onto the DWORD to be sent
	do {
		((WORD *)Buffer)[0]=4;
		out=0;
		HWND hEdtBox = GetDlgItem(m_hDlg,  id);
		nTchar = Edit_GetLine(hEdtBox, 1, &Buffer, 3);
		if (nTchar == 1 || nTchar == 2)
		{
			Buffer[nTchar] = NULL;
			out = _tstoi(Buffer);
		}
		if (out>0xF)
			out=0;
		AxesMap = (AxesMap<<4 | out);
		id++;
	} while (id <= IDC_SRC_SL1);

	m.pAxisMap = &AxesMap;
	m.nAxes = 8;

	SendMessage(m_TopDlgWnd, WMSPP_DLG_MAP, (WPARAM)&m, NULL);
}


// Create Button mapping dialog box
void SppTabJoy::CreateBtnsDlg(HWND hDlg)
{
	m_BtnsDlg = new SppBtnsDlg(m_hInstance, hDlg);
}


// Enable/disable controls according to vJoy device settings
void SppTabJoy::EnableControls(UINT id, controls * ctrl)
{
	UINT ch= IDC_X;
	UINT edt = IDC_SRC_X;
	HWND hCh, hEdt;
	UINT iAxis=0;

	////// Verify correct vJoy device
	HWND hCb = GetDlgItem(m_hDlg,IDC_VJOY_DEVICE);
	// Get the index of the selected vJoy device
	int index = (int)SendMessage(hCb,(UINT) CB_GETCURSEL  ,(WPARAM) 0,(LPARAM)0); 
	if (index == CB_ERR)
		return;

	// Extract the device id from the item's data
	int SelId = (int)SendMessage(hCb,(UINT) CB_GETITEMDATA   ,(WPARAM) index,(LPARAM)0);
	if (id != SelId)
		return;
	////// Verified

	// Go over all axes
	do 
	{
		// Axis bars
		hCh = GetDlgItem(m_hDlg,  ch);
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		ShowWindow(hCh, ctrl->axis[ch-IDC_X]);
		UpdateWindow(hCh);

		// Map edit fields
		hEdt = GetDlgItem(m_hDlg,  edt);
		EnableWindow(hEdt, ctrl->axis[edt-IDC_SRC_X]);
		UpdateWindow(hEdt);

		ch++;
		edt++;
	} while (ch<=IDC_SL1);

	SendMessage(m_BtnsDlg->GetHandle(), VJOYDEV_SETAVAIL, id, (LPARAM)ctrl);
}

void SppTabJoy::SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_JMON_BTN, iDev, (LPARAM)BtnValue);
}

void SppTabJoy::ShowButtonMapWindow(void)
{
	if (!m_BtnsDlg)
		return;

	vJoySelected(GetDlgItem(m_hDlg,IDC_VJOY_DEVICE));
	m_BtnsDlg->Show();
}


#pragma endregion

INT_PTR CALLBACK MsgHndlTabJoyDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabJoy * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabJoy *)lParam;
		DialogObj->SetPosition(hDlg) ;
		DialogObj->InitJoyMonitor(hDlg); // Init joystick monitor progress bar
		DialogObj->MonitorPpCh(hDlg); // Init joystick feeder progress bar
		DialogObj->MonitorCh(hDlg); // Init Raw channels progress bar
		DialogObj->CreateBtnsDlg(hDlg); // Create button dialog box
		return (INT_PTR)TRUE;

	case WMSPP_DLG_MAPBTN:
		DialogObj->SendMappingData((BTNArr *)wParam, (UINT)lParam);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam)  == IDC_VJOY_DEVICE && HIWORD(wParam) == CBN_SELENDOK  )
		{
			DialogObj->vJoySelected((HWND)lParam);
			break;
		}

		if (LOWORD(wParam)  == IDC_BTN_MAP && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->vJoyMapping();
			break;
		}

		if (LOWORD(wParam)  == IDC_BTN_MAPBTNS && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->ShowButtonMapWindow();
			break;
		}

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
