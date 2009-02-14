#pragma once
#include "afxwin.h"


// ScanInputs dialog

class CScanInputs : public CDialog
{
	DECLARE_DYNAMIC(CScanInputs)

public:
	CScanInputs(CWnd* pParent = NULL);   // standard constructor
	virtual ~CScanInputs();

// Dialog Data
	enum { IDD = IDD_SCAN_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void LogAudioLevel(int iMixer, int iLine, int Level);
	void LogModulation(int iModeType, int nPos);
	int ScanModulationTypes(void);

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedScan();
	afx_msg void OnBnClickedSaveLog();
	// Log control
	CEdit m_LogEdt;

protected:
	CSppConsoleDlg*		m_pWndParent;
	CComboBox*			m_MixerDeviceCtrl;
	int					m_iOrigMixerDevice;
	CButton*			m_EnableAudioCtrl;
	int					m_iOrigEnableAudio;
	CListBox*			m_AudioLinesCtrl;
	int					m_iOrigAudioLines;
	CListBox*			m_ModeTypeCtrl;
	int					m_iOrigModeType;
	CButton*			m_ShiftAutoCtrl;
	int					m_iOrigShiftAuto;
	CButton*			m_ShiftPosCtrl;
	int					m_iOrigShiftPos;
	bool				m_FirstIdle;
	bool				m_Found;
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	int OnIdle ( void );

};

