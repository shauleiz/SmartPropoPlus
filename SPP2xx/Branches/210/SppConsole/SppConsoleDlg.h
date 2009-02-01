// SppConsoleDlg.h : header file
//

#if !defined(AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_)
#define AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg dialog

class CSppConsoleDlg : public CDialog
{
// Construction
public:
	CSppConsoleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSppConsoleDlg)
	enum { IDD = IDD_SPPCONSOLE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSppConsoleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ShowShiftRB(bool show=true);
	int PopulateModulation();
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSppConsoleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeModType();
	afx_msg void OnShiftAuto();
	afx_msg void OnShiftPos();
	afx_msg void OnShiftNeg();
	afx_msg void OnHide();
	afx_msg void OnSelchangeAudioSrc();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_)
