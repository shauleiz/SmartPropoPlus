#include "stdafx.h"
#include "Windowsx.h"
#include "SmartPropoPlus.h"
#include "WinMessages.h"
#include "public.h"
#include "Commctrl.h"
#include "resource.h"
#include "vJoyMonitor.h"
#include "SppProcess.h"
#include "SppBtnsDlg.h"
#include "SppDlg.h"



#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	MsgHndlDlg(HWND, UINT, WPARAM, LPARAM);
DWORD	WINAPI		StartDlg(LPVOID hInstance);

SppDlg::SppDlg(void)
{
}

SppDlg::SppDlg(HINSTANCE hInstance, HWND	ConsoleWnd)
{
	m_hInstance = hInstance;
	m_ConsoleWnd = ConsoleWnd;

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg, (LPARAM)this);	

	// Add icon to system tray
	m_tnid.cbSize = 0;
	TaskBarAddIcon(IDI_SPPCONSOLE, CONSOLE_BALOON_TTL);
	return;
}

SppDlg::~SppDlg(void)
{
	// Remove notification icon
	if (m_tnid.cbSize)
		Shell_NotifyIcon(NIM_DELETE, &m_tnid);

}

void SppDlg::Show()
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}

void SppDlg::Hide()
{
	ShowWindow(m_hDlg, SW_HIDE);
}

void SppDlg::RegisterEndEvent(HANDLE * h)
{
	m_hEndEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	*h = m_hEndEvent;
}


DWORD WINAPI StartDlg(LPVOID hInstance)
{
	return (DWORD)DialogBox((HINSTANCE)hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg);
}

bool SppDlg::MsgLoop(void)
{
	MSG msg;
	BOOL bRet;

	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) 
	{ 
		if (bRet == -1)
		{
			return false;
		}

		else if (!IsWindow(m_hDlg))
			return true;

		else if (!IsDialogMessage(m_hDlg, &msg)) 
		{ 
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		} 
	} 

	return true;
}

// TaskBarAddIcon - adds an icon to the notification area. 
// Returns TRUE if successful, or FALSE otherwise. 
// uID - identifier of the icon 
// hicon - handle to the icon to add 
// lpszTip - tooltip text 
bool SppDlg::TaskBarAddIcon(UINT uID, LPTSTR lpszTip)
{
	bool res; 
    NOTIFYICONDATA tnid;
	HRESULT hr;
 
 	m_hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(uID));
    tnid.cbSize = sizeof(NOTIFYICONDATA); 
    tnid.hWnd = m_hDlg; 
    tnid.uID = uID; 
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
    tnid.uCallbackMessage = NULL /*MYWM_NOTIFYICON*/; 
    tnid.hIcon =	m_hIcon;// Load system tray icon

    if (lpszTip) 
        hr = StringCbCopyN(tnid.szTip, sizeof(tnid.szTip), lpszTip, 
                           sizeof(tnid.szTip));
        // TODO: Add error handling for the HRESULT.
    else 
        tnid.szTip[0] = (TCHAR)'\0'; 
 
    res = (TRUE == Shell_NotifyIcon(NIM_ADD, &tnid));
	if (res)
		m_tnid = tnid;
 
    if (m_hIcon) 
        DestroyIcon(m_hIcon); 
 
    return res; 
}

void  SppDlg::CleanAudioList(void)
{
	HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
	ListView_DeleteAllItems(hAudioList);
}

void SppDlg::InitAudioDisplay(HWND hDlg)
{
	CleanAudioList();

	// Change style
	HWND hAudioList = GetDlgItem(hDlg,  IDC_LIST_AUDIOSRC);
	ListView_SetExtendedListViewStyle(hAudioList, LVS_EX_FULLROWSELECT);

	// Set columns
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	// Add Device colum (0)
	lvc.iSubItem = 0;
    lvc.pszText = TEXT("Device");
    lvc.cx = 250;               // Width of column in pixels.
	lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.
	ListView_InsertColumn(hAudioList, 0, &lvc);

	// Add audio level colum (1)
	lvc.iSubItem = 1;
    lvc.pszText = TEXT("L/R Levels");
    lvc.cx = 80;               // Width of column in pixels.
	lvc.fmt = LVCFMT_CENTER;  // Left-aligned column.
	ListView_InsertColumn(hAudioList, 1, &lvc);
}

// Display the audio levels of channels (Left/Right)
// Levels are in the range 0-100
void SppDlg::DisplayAudioLevels(HWND hDlg, PVOID Id, UINT Left, UINT Right)
{
	HWND hAudioList = GetDlgItem(hDlg,  IDC_LIST_AUDIOSRC);

	// Get item index of by Id
	wstring str = to_wstring(Left) + L"/" +  to_wstring(Right);
	int i = FindItemById(hAudioList, (LPCTSTR)Id);
	if (i<0)
		return;

	// Set text in the format L/R (e.g. 98/3)
	ListView_SetItemText(hAudioList, i , 1, (LPTSTR)str.c_str());

}

// Find audio item in list view by its id
// Returns index to item if found or -1 if not found
int SppDlg::FindItemById(HWND hListView, LPCTSTR Id)
{
	// Get number of items
	int ItemCount = ListView_GetItemCount(hListView);
	if (ItemCount<1)
		return -1;

	LVITEM lvi;
	lvi.iSubItem=0;
	lvi.mask=LVIF_PARAM;
	for (int i=0; i<ItemCount;i++)
	{
		lvi.iItem = i;
		ListView_GetItem(hListView, &lvi);
		if (!lstrcmpi((LPCTSTR)lvi.lParam, Id))
			return i;
	}; // For loop

	// Not found
	return -1;
}

// Update the position of the progress bar that corresponds to the channel
void  SppDlg::SetRawChData(UINT iCh, UINT data)
{
	// Check if this channel is supported
	if (iCh > (IDC_CH8-IDC_CH1))
		return;

	HWND hCh = GetDlgItem(m_hDlg,  IDC_CH1+iCh);
	SendMessage(hCh, PBM_SETPOS, data, 0);

}

// Update the number of raw channels
void SppDlg::SetNumberRawCh(UINT nCh)
{
	static UINT prevVal=100;

	// Prevent flicker
	if (prevVal == nCh)
		return;
	prevVal = nCh;

	// Update text of static frame
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_RAW_CHANNELS);
	wstring txt = L"Raw Channels (" + to_wstring(nCh) + L")";
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

// Update the position of the progress bar that corresponds to the channel
void  SppDlg::SetProcessedChData(UINT iCh, UINT data)
{
	// Check if this channel is supported
	if (iCh > (IDC_CHPP8-IDC_CHPP1))
		return;

	HWND hCh = GetDlgItem(m_hDlg,  IDC_CHPP1+iCh);
	SendMessage(hCh, PBM_SETPOS, data, 0);

}

// Update the frame text of the vJoy device vJoy axes
void SppDlg::SetJoystickDevFrame(UCHAR iDev)
{
	static UINT id=0;
	if (id == iDev)
		return;
	
	id = iDev;
	HWND hFrame = GetDlgItem(m_hDlg,  IDC_VJOY_AXES);
	wstring txt = L"vJoy device " + to_wstring(iDev) + L" - Axis data";

	SendMessage(hFrame, WM_SETTEXT, 0, (LPARAM)txt.data());

}

void SppDlg::SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_JMON_BTN, iDev, (LPARAM)BtnValue);
}


// Update the position of the  progress bar that corresponds to the vJoy axis
void SppDlg::SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue)
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

void SppDlg::EnableFilter(BOOL cb)
{
	// Get check state
	HWND hCB = GetDlgItem(m_hDlg,  cb);
	int Enable = Button_GetCheck(hCB);

	// If checked then this is equivalent to selecting the current selected
	// If Un-Checked then send -1 as selected filter
	if (Enable)
		UpdateFilter();
	else
		SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
}

// Set the value of the "Start/Stop" streaming button
void SppDlg::SetStreamingButton(BOOL isProcessingAudio)
{
	static int state = -1;
	LPTSTR text;

	if (state == (int)isProcessingAudio)
		return;
	else
		state = (int)isProcessingAudio;

	HWND hStream = GetDlgItem(m_hDlg,  IDC_STREAM);
	if (isProcessingAudio)
		text = L"Stop";
	else
		text = L"Start";

	SetWindowText(hStream, text);
}

void SppDlg::OnStreamStopStart(void)
{
	HWND hStream = GetDlgItem(m_hDlg,  IDC_STREAM);
	TCHAR buff[10];
	BOOL Stream = TRUE;
	GetWindowText(hStream, buff, 10);
	if (!_tcscmp(buff, L"Stop"))
		Stream = FALSE;

	SendMessage(m_ConsoleWnd, WMSPP_DLG_STREAM , (WPARAM)Stream, 0);
}

// Set the selected filter to be displayed in the filter Combo Box
void SppDlg::SelFilter(int FilterId)
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

void SppDlg::InitFilter(int nFilters, LPTSTR FilterName)
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

void SppDlg::AddLine2FilterListA(int FilterID, const char * FilterName)
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

void SppDlg::AddLine2ModList(MOD * mod, LPCTSTR SelType)
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

void SppDlg::ShowButtonMapWindow(void)
{
	if (!m_BtnsDlg)
		return;

	vJoySelected(GetDlgItem(m_hDlg,IDC_VJOY_DEVICE));
	m_BtnsDlg->Show();
}

// Tell the parent window (Main application)
// to show/hide log window
void  SppDlg::ShowLogWindow(WORD cb)
{
	// Get data
	HWND hLogChCB = GetDlgItem(m_hDlg,  cb);
	int show = Button_GetCheck(hLogChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_LOG , show, 0);
}

void  SppDlg::RecordPulse(WORD cb)
{
	// Get data
	HWND hRecordPulseChCB = GetDlgItem(m_hDlg,  cb);
	int record = Button_GetCheck(hRecordPulseChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_PULSE , record, 0);

}


void  SppDlg::RecordInSignal(WORD cb)
{
	// Get data
	HWND hRecordInSignalChCB = GetDlgItem(m_hDlg,  cb);
	int record = Button_GetCheck(hRecordInSignalChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_INSIG , record, 0);

}

// Mapping button clicked
// Send all mapping info to the control unit
void  SppDlg::vJoyMapping(void)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_MAPBTN_SEND,0, 0);
	
}

// Set the parameters of the audio (8/16 bits Left/Right/Mono)
// If Bitrate = 0 then don't change
// If Channel="" or Channel=NULL then don't change
void SppDlg::AudioChannelParams(UINT Bitrate, WCHAR Channel)
{
	if (Bitrate == 8)
		CheckRadioButton(m_hDlg, IDC_AUD_8, IDC_AUD_16, IDC_AUD_8);
	else if (Bitrate == 16)
		CheckRadioButton(m_hDlg, IDC_AUD_8, IDC_AUD_16, IDC_AUD_16);

	if (Channel == TEXT('L'))
		CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_LEFT);
	else if (Channel == TEXT('R'))
		CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_RIGHT);
	else if (Channel == TEXT('M'))
		CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_MONO);
}

void SppDlg::AudioAutoParams(WORD Mask, WORD Flags)
{
	if (Mask&AUTOCHANNEL)
	{
		HWND hLeft = GetDlgItem(m_hDlg,  IDC_LEFT);
		HWND hMono = GetDlgItem(m_hDlg,  IDC_MONO);
		HWND hRight = GetDlgItem(m_hDlg,  IDC_RIGHT);
		if (Flags&AUTOCHANNEL)
		{
			EnableWindow(hLeft, FALSE);
			EnableWindow(hMono, FALSE);
			EnableWindow(hRight, FALSE);
			CheckDlgButton(m_hDlg,  IDC_CH_AUTO, BST_CHECKED);
		}
		else
		{
			EnableWindow(hLeft, TRUE);
			EnableWindow(hMono, TRUE);
			EnableWindow(hRight, TRUE);
			CheckDlgButton(m_hDlg,  IDC_CH_AUTO, BST_UNCHECKED);
		}
	}
}

// Get the parameters of the audio (8/16 bits Left/Right/Mono)
// Send them over to the application
// Default will be 8bit/Left channel
void SppDlg::AudioChannelParams(void)
{
	UCHAR bits = 8;
	TCHAR Channel = TEXT('L');

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_AUD_16))
		bits = 16;

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_RIGHT))
		Channel = TEXT('R');
	else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_MONO))
		Channel = TEXT('M');

	// Send message: wParam: Number of bits, lParam: channel L/R/M
	SendMessage(m_ConsoleWnd, WMSPP_DLG_CHNL, bits, Channel);

}

// Called when on of the 'Auto' checkboxs are changed
// ctrl is the ID of the checkbox
// Updates CU of the current audio setup
// Gets the new value of the checkbox and sends it to the CU
void SppDlg::AutoParams(WORD ctrl)
{
	WORD mask=0;
	if (ctrl == IDC_CH_AUTO)
		mask=AUTOCHANNEL;
	else if  (ctrl == IDC_AUD_AUTO)
		mask=AUTOBITRATE;
	else 
		return;

	AudioChannelParams();

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   ctrl))
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, mask, AUTOBITRATE|AUTOCHANNEL);
	else
		SendMessage(m_ConsoleWnd, WMSPP_DLG_AUTO, mask, 0);
}


// Clear channel display
void  SppDlg::ClearChDisplay(UINT FirstChBar, UINT LastChBar, DWORD Color)
{
	HWND hCh;
	UINT ch= FirstChBar;
	do 
	{
		hCh = GetDlgItem(m_hDlg,  ch);
		SendMessage(hCh, PBM_SETRANGE ,0, 0x03ff0000); // Range: 0-1023
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		SendMessage(hCh, PBM_SETBARCOLOR , 0, Color);
		ch++;
	} while (ch<=LastChBar);

};

// Start/Stop monitoring Eaw & processed channels
void  SppDlg::MonitorCh(bool cb)
{
	
	// Set checkbox
	HWND hChkBox = GetDlgItem(m_hDlg,  IDC_CH_MONITOR);
	if (!hChkBox)
		return;

	if (cb)
		Button_SetCheck(hChkBox, BST_CHECKED);
	else
		Button_SetCheck(hChkBox, BST_UNCHECKED);

	ClearChDisplay(IDC_CHPP1, IDC_CHPP8, RGB(0,0,0xFF));
	ClearChDisplay(IDC_CH1, IDC_CH8, RGB(0,0xFF,0));

}

// Tell the parent window (Main application)
// to stop/start monitoring the processed channel data
void  SppDlg::MonitorPrcCh(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);

	// Clear display
	ClearChDisplay(IDC_CHPP1, IDC_CHPP8, RGB(0,0,0xFF));

	// Pass request
	int start = Button_GetCheck(hMonitorChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_MONITOR , start, 0);
}

// Tell the parent window (Main application)
// to stop/start monitoring the raw channel data
void  SppDlg::MonitorRawCh(WORD cb)
{
	// Get data
	HWND hMonitorChCB = GetDlgItem(m_hDlg,  cb);

	// Clear display
	ClearChDisplay(IDC_CH1, IDC_CH8, RGB(0,0xFF,0));

	// Pass request
	int start = Button_GetCheck(hMonitorChCB);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_MONITOR , start, 0);
}

// Configure the vJoy Axes progress bars
void SppDlg::CfgJoyMonitor(HWND hDlg)
{
	// Clear display
	UINT ch= IDC_X;
	HWND hCh;
	do 
	{
		hCh = GetDlgItem(hDlg,  ch);
		SendMessage(hCh, PBM_SETRANGE ,0, 0xFFFF0000); // Range: 0-64K
		SendMessage(hCh, PBM_SETPOS, 0, 0);
		SendMessage(hCh, PBM_SETBARCOLOR , 0, RGB(0xFF,0,0));

		ch++;
	} while (ch<=IDC_SL1);

}

// Get selected filter fro GUI (if any) and send its filter index to parent window
void SppDlg::UpdateFilter(void)
{
	// Send the ID (in data) of the selected item to the parent window
	HWND hFilterList = GetDlgItem(m_hDlg,  IDC_COMBO_FILTERS);
	int FilterId; 
	int iCurSel = ComboBox_GetCurSel(hFilterList);
	if (iCurSel==CB_ERR)
	{
		SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)-1, 0);
		return;
	}

	FilterId = (int)ComboBox_GetItemData (hFilterList, iCurSel);
	SendMessage(m_ConsoleWnd, WMSPP_DLG_FILTER, (WPARAM)FilterId, 0);

	// Checks the checkbox
	HWND hFilterCB		= GetDlgItem(m_hDlg,  IDC_CH_FILTER);
	Button_SetCheck(hFilterCB, BST_CHECKED);
}


// Fill-in the actual button-mapping data - pass message to button-mapping dialog
void SppDlg::SetButtonsMappingData(BTNArr* aButtonMap, UINT nButtons)
{
	SendMessage(m_BtnsDlg->GetHandle(), WMSPP_MAPBTN_UPDT,(WPARAM)aButtonMap, nButtons);
}


// Enable/disable controls according to vJoy device settings
void SppDlg::EnableControls(UINT id, controls * ctrl)
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


// Relay the actual button-mapping data - pass message to parent
void SppDlg::SendMappingData(BTNArr* aButtonMap, UINT nButtons)
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

	SendMessage(m_ConsoleWnd, WMSPP_DLG_MAP, (WPARAM)&m, NULL);
}

// Fill-in the actual mapping data
void SppDlg::SetMappingData(Mapping * Map)
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


void  SppDlg::AddLine2AudioList(jack_info * jack)
{
	// Audio jack must have at least one audio channel (mono)
	HWND hAudioList = GetDlgItem(m_hDlg,  IDC_LIST_AUDIOSRC);
	if (!jack->nChannels)
		return;

	// Insert audio jack name
	LV_ITEM item;
	item.mask = LVIF_TEXT | LVIF_IMAGE |LVIF_STATE |LVIF_PARAM;
	item.iItem = 0;
	item.iSubItem = 0;
	item.pszText = jack->FriendlyName;
    item.stateMask = 0;
    item.iSubItem  = 0;
    item.state     = 0;
	item.lParam = (LPARAM)jack->id;
	int i = ListView_InsertItem(hAudioList, &item);

	ListView_SetItemText(hAudioList, i, 1, TEXT("0/0")); // TODO: Replace later with real stuff

	// Set the default jack as focused (and selected)
	if (jack->Default)
		ListView_SetItemState(hAudioList, i, 0xF|LVIS_FOCUSED, 0xF|LVIS_FOCUSED);


}

// Get the selected vJoy device
// Extract the device id from the item's data
// Send device id to CU
void  SppDlg::vJoySelected(HWND hCb)
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
	SendMessage(m_ConsoleWnd, WMSPP_DLG_VJOYSEL, (WPARAM)id, 0);
}

// Remove all vJoy Entries
void  SppDlg::vJoyRemoveAll()
{
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	SendMessage(hCombo,(UINT) CB_RESETCONTENT ,(WPARAM) 0,(LPARAM)0); 
}

// Add vJoy device entry to combo box
// Set the id as item data
void  SppDlg::vJoyDevAdd(UINT id)
{
	wstring vjoyid = L"vJoy " + to_wstring(id);
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	int index = (int)SendMessage(hCombo,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM)(vjoyid.data()) ); 
	SendMessage(hCombo,(UINT) CB_SETITEMDATA ,(WPARAM) index,(LPARAM)id ); 
}

// Set the selected vJoy device
void  SppDlg::vJoyDevSelect(UINT id)
{
	wstring vjoyid = L"vJoy " + to_wstring(id);
	HWND hCombo = GetDlgItem(m_hDlg,  IDC_VJOY_DEVICE);
	int index = (int)SendMessage(hCombo,(UINT) CB_FINDSTRINGEXACT ,(WPARAM) -1,(LPARAM)(vjoyid.data()) ); 
	if (index == CB_ERR)
		return;
	index =  (int)SendMessage(hCombo,(UINT) CB_SETCURSEL ,(WPARAM) index, 0); 
}

// Informs Parent window (CU) that the user pressed OK or Cancel button
void SppDlg::ExitWithOK(bool OkSelected)
{
	SendMessage(m_ConsoleWnd, WMSPP_DLG_OK, (WPARAM)OkSelected, 0);
}

void SppDlg::SelChanged(WORD ListBoxId, HWND hListBox)
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
		SendMessage(m_ConsoleWnd, WMSPP_DLG_MOD, (WPARAM)mod, 0);

	};
}

// Message handler for spp dialog box.
INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppDlg * DialogObj = NULL;

	switch (message)
	{

	case WM_NOTIFY:

		if (((LPNMHDR)lParam)->idFrom  == IDC_BTN_MAP)
		{
			return  (INT_PTR)TRUE;
		}

		return (INT_PTR)TRUE;


	case WM_INITDIALOG:
		DialogObj = (SppDlg *)lParam;
		DialogObj->CfgJoyMonitor(hDlg); // Initialize vJoy Monitoring
		DialogObj->InitFilterDisplay(hDlg); // Initialize Filter section of the GUI
		DialogObj->CreateBtnsDlg(hDlg); // Create button dialog box
		DialogObj->InitAudioDisplay(hDlg); // Initialize audio source display
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			DialogObj->ExitWithOK(LOWORD(wParam) == IDOK);
			DestroyWindow(hDlg);
			hDlg = NULL;
			PostQuitMessage(0);
			return (INT_PTR)TRUE;
		}

		if (HIWORD(wParam) == LBN_SELCHANGE )
		{
			DialogObj->SelChanged(LOWORD(wParam), (HWND)lParam);
			break;
		}

		if (LOWORD(wParam) == IDC_CH_MONITOR)
		{
			DialogObj->MonitorPrcCh(LOWORD(wParam));
			DialogObj->MonitorRawCh(LOWORD(wParam));
			break;
		};

		if (LOWORD(wParam) == IDC_CH_LOG)
		{
			DialogObj->ShowLogWindow(LOWORD(wParam));
			break;
		};

		if (LOWORD(wParam) == IDC_CH_INSIG)
		{
			DialogObj->RecordInSignal(LOWORD(wParam));
			break;
		};

		if (LOWORD(wParam) == IDC_CH_PULSE)
		{
			DialogObj->RecordPulse(LOWORD(wParam));
			break;
		};
		
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

		if  (LOWORD(wParam)  == IDC_AUD_8 || LOWORD(wParam)  == IDC_AUD_16 ||  LOWORD(wParam)  == IDC_LEFT || LOWORD(wParam)  == IDC_RIGHT || LOWORD(wParam)  == IDC_MONO) 
		{
			DialogObj->AudioChannelParams();
			break;
		}

		if  (LOWORD(wParam)  == IDC_CH_AUTO) 
		{
			DialogObj->AutoParams(IDC_CH_AUTO);
			break;
		}

		if  (LOWORD(wParam)  == IDC_AUD_AUTO) 
		{
			DialogObj->AutoParams(IDC_AUD_AUTO);
			break;
		}

		if (LOWORD(wParam)  == IDC_VJOY_DEVICE && HIWORD(wParam) == CBN_SELENDOK  )
		{
			DialogObj->vJoySelected((HWND)lParam);
			break;
		}

		if (LOWORD(wParam)  == IDC_BTN_FILTERBROWSE && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->OnFilterFileBrowse();
			break;
		}

		if (LOWORD(wParam)  == IDC_STREAM && HIWORD(wParam) == BN_CLICKED )
		{
			DialogObj->OnStreamStopStart();
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

		break; // No match for WM_COMMAND


	case REM_ALL_JACK:
		DialogObj->CleanAudioList();
		break;

	case POPULATE_JACKS:
		DialogObj->AddLine2AudioList((jack_info *)(wParam));
		break;

	case SET_AUDIO_PARAMS:
		DialogObj->AudioChannelParams((UINT)wParam, (WCHAR)lParam);
		break;
		
	case SET_AUDIO_AUTO:
		DialogObj->AudioAutoParams((UINT)wParam, (WCHAR)lParam);
		break;

	case WMSPP_PRCS_SETMOD:
		DialogObj->AddLine2ModList((MOD *)(wParam), (LPCTSTR)(lParam));
		break;

	case WMSPP_PRCS_RCHMNT:
		DialogObj->SetRawChData((UINT)wParam, (UINT)lParam);
		break;

	case WMSPP_PRCS_NRCHMNT:
		DialogObj->SetNumberRawCh((UINT)wParam);
		break;

	case WMSPP_PRCS_ALIVE:
		DialogObj->SetStreamingButton((BOOL)wParam);
		break;


	case WMSPP_PRCS_PCHMNT:
		DialogObj->SetProcessedChData((UINT)wParam, (UINT)lParam);
		break;

	case FILTER_ADDA:
		DialogObj->AddLine2FilterListA((int)wParam, (const char *)lParam);
		break;

	case FILTER_NUM:
		DialogObj->InitFilter((int)wParam, (LPTSTR)lParam);
		break;

	case FILTER_SELCTED:
		DialogObj->SelFilter((int)wParam);
		break;


	case WMSPP_JMON_AXIS:
		DialogObj->SetJoystickAxisData((UCHAR)(wParam&0xFF), (UINT)(wParam>>16), (UINT32)lParam);
		DialogObj->SetJoystickDevFrame((UCHAR)(wParam&0xFF));
		break;

	case WMSPP_JMON_BTN:
		DialogObj->SetJoystickBtnData((UCHAR)wParam, (BTNArr *)lParam);
		break;

	case WMSPP_MAP_UPDT:
		DialogObj->SetMappingData((Mapping *)wParam);
		break;

	//case WMSPP_MAPBTN_UPDT:
	//	DialogObj->SetButtonsMappingData((BTNArr *)wParam, (UINT)lParam);
	//	break;

	case MONITOR_CH:
		DialogObj->MonitorCh(wParam != 0); // Silly way to cast to bool
		break;

	case VJOYDEV_ADD:
		DialogObj->vJoyDevAdd((UINT)wParam);
		if (lParam)
			DialogObj->vJoyDevSelect((UINT)wParam);
		break;

	case VJOYDEV_REMALL:
		DialogObj->vJoyRemoveAll();
		break;

	case WMSPP_DLG_MAPBTN:
		DialogObj->SendMappingData((BTNArr *)wParam, (UINT)lParam);
		break;

	case VJOYDEV_SETAVAIL:
		DialogObj->EnableControls((UINT)wParam, (controls*)lParam);
		break;

	case VJOYDEV_CH_LEVEL:
		DialogObj->DisplayAudioLevels(hDlg, (PVOID)wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	}
	return (INT_PTR)FALSE;
}



HWND SppDlg::GetHandle(void)
{
	return m_hDlg;
}

// Create Button mapping dialog box
void SppDlg::CreateBtnsDlg(HWND hDlg)
{
	m_BtnsDlg = new SppBtnsDlg(m_hInstance, hDlg);
}


// Initialize Filters section
void SppDlg::InitFilterDisplay(HWND hDlg)
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
void SppDlg::OnFilterFileBrowse(void)
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
	info = SendMessage(m_ConsoleWnd, WMSPP_DLG_FLTRFILE , (WPARAM)ofn.lpstrFile, 0);
	if (!info)
		return;

	// Display File Name
	HWND hFilterFile	= GetDlgItem(m_hDlg,  IDC_EDIT_FILTERFILE);
	Edit_SetText(hFilterFile, (LPTSTR)info);
	UpdateWindow(hFilterFile);
	delete[] (LPVOID)info;
}

#if 0
// Respond to Browse button
// Open a IFileOpenDialog to get full path of the filters DLL file
// Based on http://msdn.microsoft.com/en-us/library/windows/desktop/bb776913(v=vs.85).aspx#api
void SppDlg::OnFilterFileBrowse(void)
{
	// CoCreate the File Open Dialog object.
    IFileDialog *pfd = NULL;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (FAILED(hr))
		return;

	// No Event handler at the moment
	// Set the options on the dialog.
	DWORD dwFlags;

	// Before setting, always get the options first in order not to override existing options.
	hr = pfd->GetOptions(&dwFlags);
	if (FAILED(hr))
		return;

	// In this case, get shell items only for file system items.
	hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
	if (FAILED(hr))
		return;

	// Set the file types to display only. Notice that, this is a 1-based array.
	hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
	if (FAILED(hr))
		return;

	// Set the selected file type index to dll.
	hr = pfd->SetFileTypeIndex(1);
	if (FAILED(hr))
		return;

	// Set the default extension to be ".dll" file.
	hr = pfd->SetDefaultExtension(L"dll");
	if (FAILED(hr))
		return;

	IKnownFolderManager *pkfm = NULL;
	IKnownFolder * ppkf = NULL;
	hr = CoCreateInstance(CLSID_KnownFolderManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pkfm));
	hr = pkfm->GetFolderByName(L".", &ppkf);

	// Show the dialog
	hr = pfd->Show(NULL);
	if (FAILED(hr))
		return;

}
#endif
