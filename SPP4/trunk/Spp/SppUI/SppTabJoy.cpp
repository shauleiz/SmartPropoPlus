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
static const int g_Controls[] = {
	/* Joystick Tab (IDD_JOY ) */
	IDC_CHPP1, IDC_CHPP2, IDC_CHPP3, IDC_CHPP4, IDC_CHPP5, IDC_CHPP6, IDC_CHPP7, IDC_CHPP8, 
	IDC_TXT_CHPP1, IDC_TXT_CHPP2, IDC_TXT_CHPP3, IDC_TXT_CHPP4, IDC_TXT_CHPP5, IDC_TXT_CHPP6, IDC_TXT_CHPP7, IDC_TXT_CHPP8, 
	IDC_CH1, IDC_CH2, IDC_CH3, IDC_CH4, IDC_CH5, IDC_CH6, IDC_CH7, IDC_CH8, 
	IDC_TXT_CH1, IDC_TXT_CH2, IDC_TXT_CH3, IDC_TXT_CH4, IDC_TXT_CH5, IDC_TXT_CH6, IDC_TXT_CH7, IDC_TXT_CH8,
	IDC_X, IDC_Y, IDC_Z, IDC_RX, IDC_RY, IDC_RZ, IDC_SL0, IDC_SL1,
	IDC_TXT_X, IDC_TXT_Y, IDC_TXT_Z, IDC_TXT_RX, IDC_TXT_RY, IDC_TXT_RZ, IDC_TXT_SL0, IDC_TXT_SL1,
	IDC_SRC_X, IDC_SRC_Y, IDC_SRC_Z, IDC_SRC_RX, IDC_SRC_RY, IDC_SRC_RZ, IDC_SRC_SL0, IDC_SRC_SL1,
	IDC_BTN_MAP, IDC_VJOY_DEVICE, IDC_BTN_MAPBTNS,
	};


SppTabJoy::SppTabJoy(void)
{
}

SppTabJoy::SppTabJoy(HINSTANCE hInstance, HWND TopDlgWnd) : SppTab( hInstance,  TopDlgWnd,  IDD_JOY, MsgHndlTabJoyDlg)
{
	m_nProcCh = 100; // Improbable number of Post-processed channels
}

SppTabJoy::~SppTabJoy(void)
{
	delete(m_BtnsDlg);
}

/* Called when vJoy is stopped
 Clears/resets joystick-related controls for the tab
 *	vJoy device frame : "vJoy device - Axis data" instead of device #
 *	vJoy bars titles disabled
 *	vJoy bars at 0
 *	Mapping : edit boxes cleared
 *	Combo box : Empty
*/
void  SppTabJoy::ClearAll(void)
{
	// Remove all entries from combo box leading to 
	// vJoy device frame : "vJoy device - Axis data" instead of device #
	vJoyRemoveAll();

	// Clear mapping data
	ClearMappingData();


	// Mapping : edit boxes disabled
	// vJoy bars titles disabled
	// Set vJoy bars at 0
	controls ctrl;
	ctrl.nButtons = 0;
	for (auto& x : ctrl.axis)
		x = FALSE;
	EnableControls(0, &ctrl);

}
void  SppTabJoy::Reset()
{
	// Select index 0 in combo IDC_VJOY_DEVICE
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	SendMessage(hCombo,(UINT) CB_SETCURSEL ,(WPARAM) 0, 0);

	// Reset Mapping of axes and Buttons
	SetMappingData();
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

// Write the version of the installed vJoy driver
void  SppTabJoy::vJoySetVer(UINT Ver)
{

	// Test if the handle to the frame is OK
	HWND hFrame = GetDlgItem(m_hDlg, IDC_VJOYNAME_FRM);
	if (!hFrame)
		return;

	// Break the version value to components and
	UINT VerComp[3] = { 0 };
	VerComp[0] = (Ver & 0x000000FF) >> 0;
	VerComp[1] = (Ver & 0x0000FF00) >> 8;
 	VerComp[2] = (Ver & 0x00FF0000) >> 16;

	// create a version string
	TCHAR BasicText[MAX_MSG_SIZE] = { 0 };
	LoadString(m_hInstance, IDS_VJOY_FRM_TXT, BasicText, MAX_MSG_SIZE);
	wstring VerStr = BasicText + to_wstring(VerComp[2]) + L"."  + to_wstring(VerComp[1])  + L"."  + to_wstring(VerComp[0]);
	
	// Update frame text
	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)VerStr.data());
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

// Enable/disable controls according to vJoy device settings
void SppTabJoy::EnableControls(UINT id, controls * ctrl)
{
	UINT ch= IDC_X;
	UINT edt = IDC_SRC_X;
	HWND hCh, hEdt, hTtl;
	UINT iAxis=0;

	////// Verify correct vJoy device
	HWND hCb = GetDlgItem(m_hDlg, IDC_VJOY_DEVICE);
	// Get the index of the selected vJoy device
	if (id)
	{
		int index = (int)SendMessage(hCb, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		if (index == CB_ERR)
			return;

		// Extract the device id from the item's data
		int SelId = (int)SendMessage(hCb, (UINT)CB_GETITEMDATA, (WPARAM)index, (LPARAM)0);
		if (id != SelId)
			return;
	};
	////// Verified

	// Go over all axes
	do
	{
		// Axis bars
		hCh = GetDlgItem(m_hDlg,  ch);
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		//ShowWindow(hCh, !ctrl->axis[ch - IDC_X] ? SW_HIDE : SW_SHOW);
		EnableWindow(hCh, (ctrl->axis[ch - IDC_X] == TRUE));
		UpdateWindow(hCh);

		// Bar titles
		hTtl = GetDlgItem(m_hDlg,  m_vJoyTitleId[iAxis]);
		EnableWindow(hTtl, (ctrl->axis[edt - IDC_SRC_X] == TRUE));
		UpdateWindow(hTtl);

		// Map edit fields
		hEdt = GetDlgItem(m_hDlg,  edt);
		EnableWindow(hEdt, (ctrl->axis[edt-IDC_SRC_X] == TRUE));
		UpdateWindow(hEdt);

		ch++;
		edt++;
		iAxis++;
	} while (ch<=IDC_SL1);

	SendMessage(m_BtnsDlg->GetHandle(), VJOYDEV_SETAVAIL, id, (LPARAM)ctrl);
}

// Init the progress bars that monitor the feedback from the joystick
void SppTabJoy::InitJoyMonitor(HWND hDlg)
{	
	InitBars(hDlg, 0xFF, m_vJoyBarId,0xFFFF0000);
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

	if (iDev)
	{
		if (m_CurJoy != iDev)
			return;
	};

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
	size_t count = m_vPpBarId.size();
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
	size_t count = m_vRawBarId.size();
	if (iCh >= count)
		return;

	HWND hCh = GetDlgItem(m_hDlg,  m_vRawBarId[iCh]);
	if (IsWindowEnabled(hCh))
		SendMessage(hCh, PBM_SETPOS, data, 0);
	else
		SendMessage(hCh, PBM_SETPOS, 0, 0);
}

// Get the number of raw channels
void SppTabJoy::SetNumberRawCh(UINT nCh)
{
	if (m_nRawCh == nCh)
		return;

	// Change
	m_nRawCh = nCh;

	if (!m_nProcCh)
		UpdateChannelView(true, m_nRawCh);

}

// Get the number of Post-processed (Filtered) channels
// Zero means - filter not used
void SppTabJoy::SetNumberProcCh(UINT nCh)
{
	// If Number of Post-processed (Filtered) channels did not change - Do Nothing
	if (m_nProcCh == nCh)
		return;

	// Change
	m_nProcCh = nCh;

	if (m_nProcCh)
		UpdateChannelView(false, m_nProcCh);
	else
		UpdateChannelView(true, m_nRawCh);

}

// Update the display of the Raw/PP bars and related controls
// Parameters: 
//	Raw	- true: Display Raw channels / false: Display PP Channels
//	Ch	- Number of channels to display
void SppTabJoy::UpdateChannelView(bool Raw, UINT nCh)
{

	// Remove every thing
	ShowWindow(GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS), SW_HIDE);
	ShowArrayOfItems( m_hDlg, SW_HIDE, m_vRawBarId);
	ShowArrayOfItems( m_hDlg, SW_HIDE, m_vRawTitleId);
	ShowWindow(GetDlgItem(m_hDlg,  IDC_SPP_OUT), SW_HIDE);
	ShowArrayOfItems( m_hDlg, SW_HIDE, m_vPpBarId);
	ShowArrayOfItems( m_hDlg, SW_HIDE, m_vPpTitleId);

	// Reset all data
	ResetArrayOfBars(m_hDlg, m_vRawBarId);
	ResetArrayOfBars(m_hDlg, m_vPpBarId);

	// If Raw - Display frame, the required bars and titls
	if (Raw)
	{
		ShowWindow(GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS), SW_SHOW);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vRawBarId, nCh);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vRawTitleId, nCh);
	}

	// If PP - Display frame, the required bars and titls
	else
	{
		ShowWindow(GetDlgItem(m_hDlg,  IDC_SPP_OUT), SW_SHOW);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vPpBarId, nCh);
		ShowArrayOfItems( m_hDlg, SW_SHOW, m_vPpTitleId, nCh);
	};
}


// Update the frame text of the vJoy device vJoy axes
void SppTabJoy::SetJoystickDevFrame(UCHAR iDev)
{
	static UINT id=100;
	wstring txt;

	if (id == iDev)
		return;

	m_CurJoy = id = iDev;
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_VJOY_AXES);

	if (m_CurJoy)
	{
		 txt = L"vJoy device " + to_wstring(iDev) + L" - Axis data";
	}
	else
		 txt = L"vJoy device - Axis data";

	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());
}

#pragma endregion

#pragma region Mapping of Joystick buttons and axes

// Clear data related to mapping
void SppTabJoy::ClearMappingData(void)
{
	UINT id = IDC_SRC_SL1;
	UINT n = IDC_SRC_SL1 - IDC_SRC_X + 1;
	HWND hEdtBox;

	// Go through the map and read nibble by nibble
	// Every nibble read goes to the corresponding edit box
	for (UINT i = 0; i<n; i++)
	{
		hEdtBox = GetDlgItem(m_hDlg, id - i);
		Edit_SetText(hEdtBox, TEXT(""));
	};

}


// Fill-in the actual mapping data
void SppTabJoy::SetMappingData(Mapping * Map)
{

	UINT id = IDC_SRC_SL1;
	HWND hEdtBox;
	UINT channel;
	TCHAR buffer[4];

	if (Map == NULL)
		return;

	//UINT& nAxes = Map->nAxes;
	//DWORD& AxisMap = *Map->pAxisMap;

	// Go through the map and read nibble by nibble
	// Every nibble read goes to the corresponding edit box
	for (UINT i=0; i<Map->nAxes; i++)
	{
		hEdtBox = GetDlgItem(m_hDlg,  id-i);
		channel = (((*Map->pAxisMap) >>(i*4))&0xF);
		_itot_s(channel, buffer, 4, 10);
		Edit_SetText(hEdtBox, buffer);
	};

	SetButtonsMappingData(Map->ButtonArray, Map->nButtons);
}

// Reset the mapping data
void SppTabJoy::SetMappingData(void)
{
	Mapping Map;
	DWORD AxisMap = 0x12345678;
	BTNArr buttons;

	Map.nAxes = 8;
	Map.pAxisMap = &AxisMap;
	Map.nButtons = 128;
	buttons.fill(9);
	for (int i=0; i<24; i++)
		buttons[i] = 9+i;
	Map.ButtonArray  = &buttons;
	SetMappingData(&Map);
	vJoyMapping();
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



void SppTabJoy::SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue)
{
	if (iDev == m_CurJoy)
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

/*
	Called every time mouse hovers over a control that was previously registered for tool tip
	Registration was done in CreateToolTip()
	The Control ID (CtrlId) of the control is extracted from the input 'param' 
	The correct text is displayed according to the Control ID
*/
void SppTabJoy::UpdateToolTip(LPVOID param)
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

	case IDC_BTN_MAP:
		DisplayToolTip(lpttt, IDS_I_BTN_MAP);
		break;

	case IDC_VJOY_DEVICE:
		DisplayToolTip(lpttt, IDS_I_VJOY_DEVICE, IDS_T_VJOY_DEVICE);
		break;

	case IDC_BTN_MAPBTNS:
		DisplayToolTip(lpttt, IDS_I_BTN_MAPBTNS, IDS_T_BTN_MAPBTNS);
		break;

	case IDC_SRC_X:
		DisplayToolTip(lpttt, IDS_I_SRC_X, IDS_T_SRC);
		break;

	case IDC_SRC_Y:
		DisplayToolTip(lpttt, IDS_I_SRC_Y, IDS_T_SRC);
		break;

	case  IDC_SRC_Z:
		DisplayToolTip(lpttt, IDS_I_SRC_Z, IDS_T_SRC);
		break;

	case  IDC_SRC_RX:
		DisplayToolTip(lpttt, IDS_I_SRC_RX, IDS_T_SRC);
		break;

	case  IDC_SRC_RY:
		DisplayToolTip(lpttt, IDS_I_SRC_RY, IDS_T_SRC);
		break;

	case  IDC_SRC_RZ:
		DisplayToolTip(lpttt, IDS_I_SRC_RZ, IDS_T_SRC);
		break;

	case  IDC_SRC_SL0:
		DisplayToolTip(lpttt, IDS_I_SRC_SL0, IDS_T_SRC);
		break;

	case  IDC_SRC_SL1:
		DisplayToolTip(lpttt, IDS_I_SRC_SL1, IDS_T_SRC);
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
	
		case IDC_X:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_X);
		break;

	case IDC_Y:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_Y);
		break;

	case IDC_Z:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_Z);
		break;

	case IDC_RX:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_RX);
		break;

	case IDC_RY:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_RY);
		break;

	case IDC_RZ:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_RZ);
		break;

	case IDC_SL0:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_SL0);
		break;

	case IDC_SL1:
		DisplayToolTip(lpttt, IDS_I_AXES, IDS_T_SL1);
		break;
	
	default:
		DisplayToolTip(lpttt, IDS_W_NOT_IMP, L"OOOPS", TTI_WARNING);
		break;
	}
}

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
		DialogObj->CreateToolTip(hDlg, g_Controls, sizeof(g_Controls)/sizeof(int)); // Initialize tooltip object
		return (INT_PTR)TRUE;

	case WM_NOTIFY:
		// Tooltips
		if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
		{
			DialogObj->UpdateToolTip((LPVOID)lParam);
			return  (INT_PTR)TRUE;
		};
		break;

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
