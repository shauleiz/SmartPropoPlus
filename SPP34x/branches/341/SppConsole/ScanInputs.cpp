// ScanInputs.cpp : implementation file
//

#include "stdafx.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "ScanInputs.h"


// ScanInputs dialog

IMPLEMENT_DYNAMIC(CScanInputs, CDialog)
CScanInputs::CScanInputs(CWnd* pParent /*=NULL*/)
	: CDialog(CScanInputs::IDD, pParent)
{
	m_pWndParent = (CSppConsoleDlg *)pParent;
	m_FirstIdle = true;

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
	CString txt;
	// If the text is too short then abort
	m_LogEdt.GetWindowText(txt);
	if (!txt || txt.IsEmpty() || !txt.GetLength())
		return;

	// Get the name of the destination file
	OPENFILENAME OpenFileNameStruct;
	const TCHAR *Filter = {"Log File\0*.log\0\0"};
	const TCHAR *Ext	= {"Log"};
	TCHAR FileName[MAX_PATH] = "";

	// Prepare structure for 'SaveAs' dialog box
	OpenFileNameStruct.lpstrFile = FileName;
	OpenFileNameStruct.lStructSize =  sizeof (OPENFILENAME);
	OpenFileNameStruct.hwndOwner = NULL;
	OpenFileNameStruct.lpstrFilter = Filter;
	OpenFileNameStruct.lpstrCustomFilter = NULL;
	OpenFileNameStruct.nMaxFile = MAX_PATH;
	OpenFileNameStruct.lpstrFileTitle = NULL;
	OpenFileNameStruct.lpstrTitle = NULL;
	OpenFileNameStruct.Flags = OFN_OVERWRITEPROMPT;
	OpenFileNameStruct.lpstrDefExt = Ext;
	OpenFileNameStruct.FlagsEx = NULL;
	OpenFileNameStruct.pvReserved = NULL;
	OpenFileNameStruct.dwReserved = 0;

	// Display 'SaveAs' dialog box - get file name
	BOOL Res = GetSaveFileName(&OpenFileNameStruct);
	if (!Res)
		return;

	// Open output file and write data into file
	HANDLE hFile = CreateFile(FileName, FILE_WRITE_DATA, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	TCHAR * buffer = txt.GetBuffer();
	DWORD BytesWritten;
	Res = WriteFile(hFile, buffer, (DWORD)strlen(buffer), &BytesWritten, (LPOVERLAPPED)NULL);

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


/*
	This function is called by the parent window every time this dialog box is idle
*/
int  CScanInputs::OnIdle( void )
{

	// If this is the first time the dialog box id idle the start a scan
	if (m_FirstIdle)
	{
		OnBnClickedScan();
		m_FirstIdle=false;
		return 1;
	};

	return 0;
}
