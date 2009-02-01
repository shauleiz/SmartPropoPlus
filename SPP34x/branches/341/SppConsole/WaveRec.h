#include "afxwin.h"
#include "resource.h"		// main symbols
#include "DynamicLED.h"
#pragma once

#define TIMER_REC	1
#define TIMER_STAT	2

// CWaveRec dialog

class CWaveRec : public CDialog
{
	DECLARE_DYNAMIC(CWaveRec)

public:
	CWaveRec(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWaveRec();

// Dialog Data
	enum { IDD = IDD_REC_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRecStop();
	afx_msg void OnBnClickedRecStart();
	afx_msg void OnBnClickedRecPlay();

protected:
	TCHAR m_WaveFileName[MAX_PATH];
	// True while recording
	bool m_Recording;
	// True while Playback
	bool m_Playing;
	// Duration of requested recording
	int m_Duration;
	// Time left for current recording
	int m_TimeLeft;
	// Enables/Disables control according to recording status
	virtual void UpdateControls(void);
	CWnd* m_pParent;
	void RenameWaveFile(void);
	bool GetWaveFileName(void);

	// Timer number 1: Ticks every second
	UINT_PTR m_Timer1;
	// Blinking green LED
	CDynamicLED m_dynLEDGreen;

	virtual void PostMessageStop();
	virtual void PostMessageStart();
	virtual void PostMessageCancel();
	virtual void PostMessagePlay();

	/* Constant strings */
	const TCHAR * m_MsgNoAbortRec;
	const TCHAR * m_MsgNoAbortPlay;
	const TCHAR * m_FilterStr;
	const TCHAR * m_DefaultFileExt;

	// Encasulation functions
	virtual BOOL SppLoggerStop(void);
	virtual BOOL SppLoggerPlay(TCHAR * FileName);
	virtual BOOL SppLoggerStart(TCHAR * FileName);
	virtual BOOL SppLoggerStat(int * stat);

public:
	BOOL OnInitDialog(void);
	afx_msg void OnBnClickedCancel();
	void OnTimer(UINT nIDEvent);
};
