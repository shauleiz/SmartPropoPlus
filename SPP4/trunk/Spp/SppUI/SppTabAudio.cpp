#include "stdafx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabAudio.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabAudioDlg(HWND, UINT, WPARAM, LPARAM);

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
	UCHAR bits = 8;
	TCHAR Channel = TEXT('L');

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_AUD_16))
		bits = 16;

	if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_RIGHT))
		Channel = TEXT('R');
	else if (BST_CHECKED == IsDlgButtonChecked(m_hDlg,   IDC_MONO))
		Channel = TEXT('M');

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


INT_PTR CALLBACK MsgHndlTabAudioDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabAudio * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabAudio *)lParam;
		DialogObj->SetPosition(hDlg) ;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if  (LOWORD(wParam)  == IDC_AUD_AUTO) 
		{
			DialogObj->AutoParams(IDC_AUD_AUTO);
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

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
