// TestReg4SPPDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CTestReg4SPPDlg dialog
class CTestReg4SPPDlg : public CDialog
{
// Construction
public:
	CTestReg4SPPDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTREG4SPP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	// Control variable: Log File Name
	CEdit m_ctrl_Log_File_Name;
protected:
	// Open log file for writing. Return pointer to file. Put full path in 'path'
	FILE * OpenLogfile(CString * path);
	CString m_LogPath;
	FILE * m_pLogFile;
public:
	afx_msg void OnBnClickedCancel();
protected:
	// Test existence of key. Create if does not exist
	int CreateKey(const char * KeyName);
};
