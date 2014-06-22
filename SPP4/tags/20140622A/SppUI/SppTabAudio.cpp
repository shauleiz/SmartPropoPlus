#include "stdafx.h"
#include "Commctrl.h"
#include "Uxtheme.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabAudio.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabAudioDlg(HWND, UINT, WPARAM, LPARAM);

static const int g_Controls[] = {
		/* Audio Tab (IDD_AUDIO ) */
		IDC_AUD_AUTO, IDC_AUD_8, IDC_AUD_16, IDC_CH_AUTO, IDC_LEFT, IDC_RIGHT, IDC_LEVEL_L, IDC_LEVEL_R, IDC_LEVEL_M,
	};

SppTabAudio::SppTabAudio(void)
{
}

SppTabAudio::SppTabAudio(HINSTANCE hInstance, HWND TopDlgWnd) : SppTab( hInstance,  TopDlgWnd,  IDD_AUDIO, MsgHndlTabAudioDlg)
{
}

SppTabAudio::~SppTabAudio(void)
{
}

// Display the audio levels of channels (Left/Right)
// Levels are in the range 0-100
void SppTabAudio::DisplayAudioLevels(PVOID Id, UINT Left, UINT Right)
{
	HWND hLabelL = GetDlgItem(m_hDlg, IDS_LEFT);
	HWND hLabelR = GetDlgItem(m_hDlg, IDS_RIGHT);
	HWND hLabelM = GetDlgItem(m_hDlg, IDS_MONO);
	HWND hLevelL = GetDlgItem(m_hDlg, IDC_LEVEL_L);
	HWND hLevelR = GetDlgItem(m_hDlg, IDC_LEVEL_R);
	HWND hLevelM = GetDlgItem(m_hDlg, IDC_LEVEL_M);

	// Fix progress bar problem
	SetWindowTheme(hLevelL, L" ", L" ");
	SetWindowTheme(hLevelR, L" ", L" ");
	SetWindowTheme(hLevelM, L" ", L" ");
	SetWindowPos(hLevelL, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	SetWindowPos(hLevelR, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	SetWindowPos(hLevelM, HWND_TOP, 1,1,1,1, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

	// Mono?
	if (Right>100) // If Right channel is over 100 then this is a mono device
	{
		ShowWindow(hLabelR, SW_HIDE);
		ShowWindow(hLevelR, SW_HIDE);
		ShowWindow(hLabelL, SW_HIDE);
		ShowWindow(hLevelL, SW_HIDE);

		ShowWindow(hLevelM, SW_SHOW);
		ShowWindow(hLabelM, SW_SHOW);
		SendMessage(hLevelM, PBM_SETPOS, Left, 0);
	}
	else
	{ // Stereo
		ShowWindow(hLabelR, SW_SHOW);
		ShowWindow(hLevelR, SW_SHOW);
		ShowWindow(hLabelL, SW_SHOW);
		ShowWindow(hLevelL, SW_SHOW);


		ShowWindow(hLevelM, SW_HIDE);
		ShowWindow(hLabelM, SW_HIDE);
		SendMessage(hLevelL, PBM_SETPOS, Left, 0);
		SendMessage(hLevelR, PBM_SETPOS, Right, 0);
	};
}

// Called when one of the 'Auto' checkboxs for audio are changed
// ctrl is the ID of the checkbox
// Updates CU of the current audio setup
// Gets the new value of the checkbox and sends it to the CU
void SppTabAudio::AutoParams(WORD ctrl)
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
		SendMessage(m_TopDlgWnd, WMSPP_DLG_AUTO, mask, AUTOBITRATE|AUTOCHANNEL);
	else
		SendMessage(m_TopDlgWnd, WMSPP_DLG_AUTO, mask, 0);
}

// Get the parameters of the audio (8/16 bits Left/Right/Mono)
// Send them over to the application
// Default will be 8bit/Left channel
void SppTabAudio::AudioChannelParams(void)
{
	UCHAR bits;
	TCHAR Channel = TEXT('L');

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_AUD_16))
		bits = 16;
	else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_AUD_8))
		bits = 8;

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_RIGHT))
		Channel = TEXT('R');
	else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_LEFT))
		Channel = TEXT('L');

	// Send message: wParam: Number of bits, lParam: channel L/R/M
	SendMessage(m_TopDlgWnd, WMSPP_DLG_CHNL, bits, Channel);

}

// Set the parameters of the audio (8/16 bits Left/Right/Mono)
// If Bitrate = 0 then don't change
// If Channel="" or Channel=NULL then don't change
void SppTabAudio::AudioChannelParams(UINT Bitrate, WCHAR Channel)
{
	if (Bitrate == 8)
		CheckRadioButton(m_hDlg, IDC_AUD_8, IDC_AUD_16, IDC_AUD_8);
	else if (Bitrate == 16)
		CheckRadioButton(m_hDlg, IDC_AUD_8, IDC_AUD_16, IDC_AUD_16);

	if (Channel)
	{
		if (Channel == TEXT('M'))
		{
			ShowWindow(GetDlgItem(m_hDlg,  IDC_LEFT), SW_HIDE);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_RIGHT), SW_HIDE);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_CH_AUTO), SW_HIDE);
			ShowWindow(GetDlgItem(m_hDlg,  IDD_AUDIO_SRC), SW_HIDE);
		}
		else
		{
			ShowWindow(GetDlgItem(m_hDlg,  IDC_LEFT), SW_SHOW);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_RIGHT), SW_SHOW);
			ShowWindow(GetDlgItem(m_hDlg,  IDC_CH_AUTO), SW_SHOW);
			ShowWindow(GetDlgItem(m_hDlg,  IDD_AUDIO_SRC), SW_SHOW);
			if (Channel == TEXT('L'))
				CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_LEFT);
			else if (Channel == TEXT('R'))
				CheckRadioButton(m_hDlg, IDC_LEFT, IDC_RIGHT, IDC_RIGHT);
		}
	};
}



void SppTabAudio::AudioAutoParams(WORD Mask, WORD Flags)
{
	// Auto channel selection
	if (Mask&AUTOCHANNEL)
	{
		HWND hLeft = GetDlgItem(m_hDlg,  IDC_LEFT);
		//HWND hMono = GetDlgItem(m_hDlg,  IDC_MONO);
		HWND hRight = GetDlgItem(m_hDlg,  IDC_RIGHT);
		if (Flags&AUTOCHANNEL)
		{
			EnableWindow(hLeft, FALSE);
			//EnableWindow(hMono, FALSE);
			EnableWindow(hRight, FALSE);
			CheckDlgButton(m_hDlg,  IDC_CH_AUTO, BST_CHECKED);
		}
		else
		{
			EnableWindow(hLeft, TRUE);
			//EnableWindow(hMono, TRUE);
			EnableWindow(hRight, TRUE);
			CheckDlgButton(m_hDlg,  IDC_CH_AUTO, BST_UNCHECKED);
		}
	}

	// Auto bit rate selection
	if (Mask&AUTOBITRATE)
	{
		HWND h8 = GetDlgItem(m_hDlg,  IDC_AUD_8);
		HWND h16 = GetDlgItem(m_hDlg,  IDC_AUD_16);
		if (Flags&AUTOBITRATE)
		{
			EnableWindow(h8, FALSE);
			EnableWindow(h16, FALSE);
			CheckDlgButton(m_hDlg,  IDC_AUD_AUTO, BST_CHECKED);
		}
		else
		{
			EnableWindow(h8, TRUE);
			EnableWindow(h16, TRUE);
			CheckDlgButton(m_hDlg,  IDC_AUD_AUTO, BST_UNCHECKED);
		}
	}}

/*
	Called every time mouse hovers over a control that was previously registered for tool tip
	Registration was done in CreateToolTip()
	The Control ID (CtrlId) of the control is extracted from the input 'param' 
	The correct text is displayed according to the Control ID
*/
void SppTabAudio::UpdateToolTip(LPVOID param)
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
	case IDC_AUD_AUTO:	// Auto bitrate check box
		DisplayToolTip(lpttt, IDS_I_AUD_AUTO, IDS_T_AUD_AUTO);
		break;
	case IDC_AUD_8:		// Bitrate = 8
		DisplayToolTip(lpttt, IDS_I_AUD_8, IDS_T_AUD_8);
		break;
	case IDC_AUD_16:	// Bitrate = 16
		DisplayToolTip(lpttt, IDS_I_AUD_16, IDS_T_AUD_16);
		break;
	case IDC_CH_AUTO:	// Auto channel check box
		DisplayToolTip(lpttt, IDS_I_CH_AUTO, IDS_T_CH_AUTO);
		break;
	case IDC_LEFT:		// Left channel
		DisplayToolTip(lpttt, IDS_I_LEFT, IDS_T_LEFT);
		break;
	case IDC_RIGHT:		// Right channel
		DisplayToolTip(lpttt, IDS_I_RIGHT, IDS_T_RIGHT);
		break;
	case IDC_LEVEL_L:	// Audio level: Left channel
		DisplayToolTip(lpttt, IDS_I_LEVEL_L, IDS_T_LEVEL_L);
		break;
	case IDC_LEVEL_R:	// Audio level: Right channel
		DisplayToolTip(lpttt, IDS_I_LEVEL_R, IDS_T_LEVEL_R);
		break;
	case IDC_LEVEL_M:	// Audio level
		DisplayToolTip(lpttt, IDS_I_LEVEL_M);
		break;



	default:
		DisplayToolTip(lpttt, IDS_W_NOT_IMP, L"OOOPS", TTI_WARNING);
		break;
	}
}


INT_PTR CALLBACK MsgHndlTabAudioDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabAudio * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabAudio *)lParam;
		DialogObj->SetPosition(hDlg) ;
		DialogObj->CreateToolTip(hDlg, g_Controls, sizeof(g_Controls)/sizeof(int)); // Initialize tooltip object
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if  (LOWORD(wParam)  == IDC_AUD_AUTO) 
		{
			DialogObj->AutoParams(IDC_AUD_AUTO);
			break;
		}

		if  (LOWORD(wParam)  == IDC_AUD_8 || LOWORD(wParam)  == IDC_AUD_16 ||  LOWORD(wParam)  == IDC_LEFT || LOWORD(wParam)  == IDC_RIGHT/* || LOWORD(wParam)  == IDC_MONO*/) 
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

	case WM_NOTIFY:
		// Tooltips
		if (((LPNMHDR)lParam)->code == TTN_GETDISPINFO)
		{
			DialogObj->UpdateToolTip((LPVOID)lParam);
			return  (INT_PTR)TRUE;
		};
		
		return (INT_PTR)TRUE;

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
