// ScanInputs.cpp : implementation file
//

#include "stdafx.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "ScanInputs.h"
#include ".\scaninputs.h"


// ScanInputs dialog

IMPLEMENT_DYNAMIC(CScanInputs, CDialog)
CScanInputs::CScanInputs(CWnd* pParent /*=NULL*/)
	: CDialog(CScanInputs::IDD, pParent)
{
	m_pWndParent = (CSppConsoleDlg *)pParent;

	//// Audio source checkbox
	//m_EnableAudioCtrl = (CButton*)m_pWndParent->GetCheckAudioCtrl();
	//m_iOrigEnableAudio = m_EnableAudioCtrl->GetCheck();

	// Mixer device & audio lines
	m_MixerDeviceCtrl = (CComboBox*)m_pWndParent->GetMixerDeviceCtrl();
	m_iOrigMixerDevice = m_MixerDeviceCtrl->GetCurSel();
	m_AudioLinesCtrl = (CListBox*)m_pWndParent->GetAudioLinesCtrl();
	m_iOrigAudioLines = m_AudioLinesCtrl->GetCurSel();

	// Mode - Type & subtypes
	m_ModeTypeCtrl = (CListBox*)m_pWndParent->GetModeTypeCtrl();
	m_iOrigModeType = m_ModeTypeCtrl->GetCurSel();
	m_ShiftAutoCtrl = (CButton*)m_pWndParent->GetShiftAutoCtrl();
	m_iOrigShiftAuto = m_ShiftAutoCtrl->GetCheck();
	m_ShiftPosCtrl = (CButton*)m_pWndParent->GetShiftPosCtrl();
	m_iOrigShiftPos = m_ShiftPosCtrl->GetCheck();
}

CScanInputs::~CScanInputs()
{
}

void CScanInputs::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);DDX_Control(pDX, IDC_LOG_EDT, m_LogEdt);
}


BEGIN_MESSAGE_MAP(CScanInputs, CDialog)
	ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
	ON_BN_CLICKED(IDC_SCAN, OnBnClickedScan)
	ON_BN_CLICKED(IDC_SAVE_LOG, OnBnClickedSaveLog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// ScanInputs message handlers

void CScanInputs::OnBnClickedCancel()
{
	// Reset to original state 
	m_MixerDeviceCtrl->SetCurSel(m_iOrigMixerDevice);
	m_AudioLinesCtrl->SetCurSel(m_iOrigAudioLines);
	m_ModeTypeCtrl->SetCurSel(m_iOrigModeType);
	m_ShiftAutoCtrl->SetCheck(m_iOrigShiftAuto);
	m_ShiftPosCtrl->SetCheck(m_iOrigShiftPos);

	m_pWndParent->SomethingChanged();

	OnCancel();
}

void CScanInputs::OnBnClickedScan()
{
	// Set the hourglass cursor
	AfxGetApp()->DoWaitCursor(1); 

	// Loop on all mixer devices
	int MixerCount = m_MixerDeviceCtrl->GetCount();
	int MixerCurSel = m_MixerDeviceCtrl->GetCurSel();
	for (int i=0; i<MixerCount; i++)
	{
		// Loop on all current mixer's input lines
		int AudioLineCount = m_AudioLinesCtrl->GetCount();
		int AudioLineCurSel = m_AudioLinesCtrl->GetCurSel();
		for (int j=0; j<AudioLineCount; j++)
		{
			Sleep(200);
			// Test this line for audio level
			int CurAudioLevel = m_pWndParent->GetAudioLevel();
			LogAudioLevel(MixerCurSel, AudioLineCurSel, CurAudioLevel);
			// If the audio level is high enough - Scan modulations
			if ((CurAudioLevel>200) && (ScanModulationTypes()>0))
				return; // FOUND! Stop searching	

			// get next audio line
			AudioLineCurSel = m_AudioLinesCtrl->SetCurSel((AudioLineCurSel+1)%AudioLineCount);
			m_pWndParent->SomethingChanged();
		};	// Audio Line for-loop
		// Get next mixer;
		MixerCurSel = m_MixerDeviceCtrl->SetCurSel((MixerCurSel+1)%MixerCount);
		m_pWndParent->SomethingChanged();
	}; // Mixer for-loop

	// Remove the hourglass cursor
	AfxGetApp()->DoWaitCursor(-1); 
}

void CScanInputs::OnBnClickedSaveLog()
{
	// TODO: Save the log text in a file
}

void CScanInputs::LogAudioLevel(int iMixer, int iLine, int Level)
{
	CString OldText, NewText, txtCurMixer, txtCurLine;

	// Get Mixer % input line text
	m_MixerDeviceCtrl->GetWindowText(txtCurMixer);
	if (!txtCurMixer.GetLength()) txtCurMixer = "[?]";
	m_AudioLinesCtrl->GetText(iLine, txtCurLine);
	if (!txtCurLine.GetLength()) txtCurLine = "[?]";

	// Print
	NewText.Format("Mixer: %-30s -  %-20s ; Volume: %d\r\n", txtCurMixer, txtCurLine, Level);
	m_LogEdt.GetWindowText(OldText);
	m_LogEdt.SetWindowText(OldText+NewText);

	// Scroll
	int nLines = m_LogEdt.GetLineCount();
	m_LogEdt.LineScroll(nLines-1);
	m_LogEdt.RedrawWindow();
}

int CScanInputs::ScanModulationTypes(void)
{
	const int nTrys = 3; 
	int nPos[nTrys]; // Number of joystick positions X nTrys
	int iTry;

	int ModeTypeCount =  m_ModeTypeCtrl->GetCount();
	int ModeTypeCurSel = m_ModeTypeCtrl->GetCurSel();
	// Loop on all modulation types
	for (int m=0; m<ModeTypeCount; m++)
	{
		Sleep(500);
		// Get joystick information (Several tries)
		for (iTry=0; iTry<nTrys; iTry++)
		{
			nPos[iTry] = m_pWndParent->GetNumJoystickPos();
			LogModulation(ModeTypeCurSel, nPos[iTry]);
			if (!nPos[iTry]) break;
			if (iTry >0 && iTry == nTrys-1 && nPos[iTry] == nPos[iTry-1])
				return nPos[iTry];
		};
		ModeTypeCurSel = m_ModeTypeCtrl->SetCurSel((ModeTypeCurSel+1)%ModeTypeCount);
		m_pWndParent->SomethingChanged();
	};

	return -1;
}

void CScanInputs::LogModulation(int iModeType, int nPos)
{
	CString OldText, NewText,txtModulation;

	// Get Modulation type string
	m_ModeTypeCtrl->GetText(iModeType,txtModulation);

	// Print
	NewText.Format("   Mod: %-30s ; Joystick Pos: %d\r\n", txtModulation, nPos);
	m_LogEdt.GetWindowText(OldText);
	m_LogEdt.SetWindowText(OldText+NewText);

	// Scroll
	int nLines = m_LogEdt.GetLineCount();
	m_LogEdt.LineScroll(nLines-1);
	m_LogEdt.RedrawWindow();
}


void CScanInputs::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// Move the dialog box to (almost) aline with the upper-right corner of the parent dialog box
	RECT RectParent, RectDlg;
	m_pWndParent->GetWindowRect(&RectParent);
	GetWindowRect(&RectDlg);

	int DeltaRight = RectParent.right - RectDlg.right-8;
	RectDlg.right +=DeltaRight;
	RectDlg.left +=DeltaRight;

	int DeltaTop = RectParent.top - RectDlg.top+8;
	RectDlg.top += DeltaTop;
	RectDlg.bottom += DeltaTop;

	MoveWindow(&RectDlg,0);
}
