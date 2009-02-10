/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CWaveRec Class - GUI class
// Represents dialog box that controls the recording and replay of Wave files
// Calls interface Functions from DLL file (Wave2Joystick.dll)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SmartPropoPlus.h"
#include "Wave2Joystick.h"
#include "SppConsole.h"
#include "WaveRec.h"
#include ".\waverec.h"


IMPLEMENT_DYNAMIC(CWaveRec, CDialog)
CWaveRec::CWaveRec(CWnd* pParent /*=NULL*/)
	: CDialog(CWaveRec::IDD, pParent)
	, m_Recording(false)
	, m_Playing(false)
	, m_Duration(15)
{
	m_WaveFileName[0] = NULL;
	m_MsgNoAbortRec		= "Cannot abort while recording";
	m_MsgNoAbortPlay	= "Cannot abort while playback";
	m_FilterStr			= "Wave File\0*.wav\0\0";
	m_DefaultFileExt	= "Wav";
}

CWaveRec::~CWaveRec()
{
}

void CWaveRec::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LED_G, m_dynLEDGreen);
}


BEGIN_MESSAGE_MAP(CWaveRec, CDialog)
	ON_BN_CLICKED(IDC_REC_STOP, OnBnClickedRecStop)
	ON_BN_CLICKED(IDC_REC_START, OnBnClickedRecStart)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_REC_PLAY, OnBnClickedRecPlay)
	ON_WM_NCDESTROY()
END_MESSAGE_MAP()


// CWaveRec message handlers

void CWaveRec::OnBnClickedRecStop()
{
	// Stop countdown timer
	KillTimer(m_Timer1);

	// Stop logger
	SppLoggerStop();

	// Rename output file
	GetDlgItem(IDC_REC_STOP)->EnableWindow(false);
	if (m_Recording)
		RenameWaveFile();

	// Remove green 'Play' LED
	if (m_Playing)
	{
		m_dynLEDGreen.SwitchOff();
		GetDlgItem(IDC_LED_G)->ShowWindow(SW_HIDE);
	};

	// Update controls
	TCHAR StrDuration[5];
	m_Recording = false;
	m_Playing = false;
	UpdateControls();
	GetDlgItem(IDC_REC_START)->SetFocus();
	sprintf(StrDuration,"%d",m_Duration);
	GetDlgItem(IDC_REC_SEC)->SetWindowText((LPCTSTR)StrDuration);

	// Inform the calling dialog box that Recording is off
	PostMessageStop();
}

void CWaveRec::OnBnClickedRecStart()
{

	// Get the duration in seconds
	TCHAR StrDuration[5];
	if (GetDlgItem(IDC_REC_SEC)->GetWindowText(StrDuration, 4))
		_stscanf(StrDuration,"%d", &m_TimeLeft);
	else
		m_TimeLeft = 15;
	m_Duration = m_TimeLeft;

	// Inform the calling dialog box that Recording is on
	PostMessageStart();

	// Start Wave logger - output file name will be in m_WaveFileName
	if (SppLoggerStart(&m_WaveFileName[0]))
	{
		m_Playing = false;
		m_Recording = true;
	}
	else
		m_Recording = false;

	// Update controls
	UpdateControls();
	GetDlgItem(IDC_REC_STOP)->SetFocus();

	// Start countdown
	m_Timer1 = SetTimer(TIMER_REC,1000,(TIMERPROC)NULL);
}

// Enables/Disables control according to recording status
void CWaveRec::UpdateControls(void)
{
	CWnd * StartBtn = GetDlgItem(IDC_REC_START);
	if (StartBtn)
		StartBtn->EnableWindow(!m_Recording && !m_Playing);

	CWnd * Seconds = GetDlgItem(IDC_REC_SEC);
	if (Seconds)
		Seconds->EnableWindow(!m_Recording && !m_Playing);

	CWnd * StopBtn = GetDlgItem(IDC_REC_STOP);
	if (StopBtn)
		StopBtn->EnableWindow(m_Recording || m_Playing);

	CWnd * PlayBtn = GetDlgItem(IDC_REC_PLAY);
	if (PlayBtn)
		PlayBtn->EnableWindow(!m_Recording && !m_Playing);
}

BOOL CWaveRec::OnInitDialog(void)
{
	UpdateControls();
	CString StrDuration;
	StrDuration.Format("%d",m_Duration);
	GetDlgItem(IDC_REC_SEC)->SetWindowText((LPCTSTR)StrDuration);
	return CDialog::OnInitDialog();
}

void CWaveRec::PostMessageStop()
{
	GetParent()->PostMessage(MSG_WAVE_REC_STOP,0,0);
}

void CWaveRec::PostMessageStart()
{
	GetParent()->PostMessage(MSG_WAVE_REC_START,0,0);
}
void CWaveRec::PostMessageCancel()
{
	GetParent()->PostMessage(MSG_WAVE_REC_CANCEL,0,0);
}
void CWaveRec::PostMessagePlay()
{
	GetParent()->PostMessage(MSG_WAVE_REC_PLAY,0,0);
}

void CWaveRec::OnBnClickedCancel()
{
	if (m_Recording)
	{
		MessageBox(m_MsgNoAbortRec, NULL , MB_ICONERROR);
		return;
	};

	if ( m_Playing)
	{
		MessageBox(m_MsgNoAbortPlay, NULL , MB_ICONERROR);
		return;
	};

	// Inform the calling dialog box that Recording is off
	PostMessageStop();
	PostMessageCancel();


	//OnNcDestroy();
	DestroyWindow();
}


void CWaveRec::RenameWaveFile(void)
{
	OPENFILENAME OpenFileNameStruct;
	const TCHAR *Filter = {m_FilterStr};
	TCHAR OldName[MAX_PATH];
	BOOL Res;

	_tcsncpy(OldName, m_WaveFileName, MAX_PATH);

	// Prepare structure for 'SaveAs' dialog box
	OpenFileNameStruct.lpstrFile = m_WaveFileName;
	OpenFileNameStruct.lStructSize =  sizeof (OPENFILENAME);
	OpenFileNameStruct.hwndOwner = NULL;
	OpenFileNameStruct.lpstrFilter = Filter;
	OpenFileNameStruct.lpstrCustomFilter = NULL;
	OpenFileNameStruct.nMaxFile = MAX_PATH;
	OpenFileNameStruct.lpstrFileTitle = NULL;
	OpenFileNameStruct.lpstrTitle = NULL;
	OpenFileNameStruct.Flags = OFN_OVERWRITEPROMPT;
	OpenFileNameStruct.lpstrDefExt = m_DefaultFileExt;
	OpenFileNameStruct.FlagsEx = NULL;
	OpenFileNameStruct.pvReserved = NULL;
	OpenFileNameStruct.dwReserved = 0;

	// Display 'SaveAs' dialog box - get file name
	Res = GetSaveFileName(&OpenFileNameStruct);
	if (!Res)
		return;

	// Rename
	Res = MoveFileEx(OldName,m_WaveFileName, MOVEFILE_COPY_ALLOWED);
	if (!Res)
		_tcsncpy(m_WaveFileName, OldName, MAX_PATH);
}

bool CWaveRec::GetWaveFileName(void)
{
	OPENFILENAME OpenFileNameStruct;
	const TCHAR *Filter = {m_FilterStr};
	TCHAR OldName[MAX_PATH];
	BOOL Res;

	_tcsncpy(OldName, m_WaveFileName, MAX_PATH);

	// Prepare structure for 'SaveAs' dialog box
	OpenFileNameStruct.lpstrFile = m_WaveFileName;
	OpenFileNameStruct.lStructSize =  sizeof (OPENFILENAME);
	OpenFileNameStruct.hwndOwner = NULL;
	OpenFileNameStruct.lpstrFilter = Filter;
	OpenFileNameStruct.lpstrCustomFilter = NULL;
	OpenFileNameStruct.nMaxFile = MAX_PATH;
	OpenFileNameStruct.lpstrFileTitle = NULL;
	OpenFileNameStruct.lpstrTitle = NULL;
	OpenFileNameStruct.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
	OpenFileNameStruct.lpstrDefExt = m_DefaultFileExt;
	OpenFileNameStruct.FlagsEx = NULL;
	OpenFileNameStruct.pvReserved = NULL;
	OpenFileNameStruct.dwReserved = 0;

	// Display 'Open' dialog box - get file name
	Res = GetOpenFileName(&OpenFileNameStruct);
	if (Res) return true;
	else return false;
}

void CWaveRec::OnTimer(UINT nIDEvent)
{
	TCHAR StrDuration[5];
	int Status;
	BOOL Valid;

	// Count down timer and stop recording when time ellapsed
	if (nIDEvent == TIMER_REC)
	{
		if ( m_TimeLeft != 0)
		{
			sprintf(StrDuration,"%d",--m_TimeLeft);
			GetDlgItem(IDC_REC_SEC)->SetWindowText((LPCTSTR)StrDuration);
			if (m_TimeLeft == 0)
				OnBnClickedRecStop();
		};
		return;

	}
	else if (nIDEvent == TIMER_STAT)
	{
		Valid = SppWaveLoggerGetStat(&Status);
		if (Valid && Status == STOPPED)
			OnBnClickedRecStop();
		return;
	};

	CDialog::OnTimer(nIDEvent);
}


// PLAY button was clicked
// Select an input WAV file and execute the play routine
void CWaveRec::OnBnClickedRecPlay()
{
	// Inform the calling dialog box that Recording is on
	PostMessagePlay();

	// Blinking green LED - show (off state)
	CWnd *pWndGreen = (CWnd *)GetDlgItem(IDC_LED_G);
	pWndGreen->ShowWindow(SW_SHOW);
	m_dynLEDGreen.InitLed(pWndGreen,ID_LED_GREEN, ID_SHAPE_ROUND);
	m_dynLEDGreen.SwitchOff();

	// Get the name of the Wav file to play
	CWnd * pLed = GetDlgItem(IDC_REC_PLAY);
	if (pLed)
		pLed->EnableWindow(false);
	if (!GetWaveFileName() && pLed)
	{
		GetDlgItem(IDC_REC_PLAY)->EnableWindow(true);
		m_dynLEDGreen.SwitchOff();
		GetDlgItem(IDC_LED_G)->ShowWindow(SW_HIDE);
		return;
	};


	// Start Wave playbak - input file name will be in m_WaveFileName
	if (SppLoggerPlay(&m_WaveFileName[0]))
	{
		m_Playing = true;
		m_Recording = false;
	}
	else
	{
		m_Playing = false;
		m_dynLEDGreen.SwitchOff();
		GetDlgItem(IDC_LED_G)->ShowWindow(SW_HIDE);
	};


	// Update controls
	UpdateControls();
	GetDlgItem(IDC_REC_STOP)->SetFocus();

	// Blinking green LED - on
	m_dynLEDGreen.Blink(500);

	// Start polling for status
	m_Timer1 = SetTimer(TIMER_STAT,100,(TIMERPROC)NULL);

}


// Encasulates call to external function
BOOL CWaveRec::SppLoggerStop(void) { return SppWaveLoggerStop();}
BOOL CWaveRec::SppLoggerPlay(TCHAR * FileName)	{ return SppWaveLoggerPlay(FileName);}
BOOL CWaveRec::SppLoggerStart(TCHAR * FileName) { return SppWaveLoggerStart(FileName);}
BOOL CWaveRec::SppLoggerStat(int * stat)		{ return SppWaveLoggerGetStat(stat);}

void CWaveRec::OnNcDestroy()
{
	delete (this);
}
