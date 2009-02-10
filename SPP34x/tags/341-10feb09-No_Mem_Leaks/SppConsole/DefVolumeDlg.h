#if !defined(AFX_DEFVOLUMEDLG_H__AEDA9B15_8280_4749_B555_2723E8C7D7BD__INCLUDED_)
#define AFX_DEFVOLUMEDLG_H__AEDA9B15_8280_4749_B555_2723E8C7D7BD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DefVolumeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDefVolumeDlg dialog

class CDefVolumeDlg : public CDialog
{
// Construction
public:
	CDefVolumeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDefVolumeDlg)
	enum { IDD = IDD_DEF_VOL };
	CSliderCtrl	m_sldrAnl;
	CSliderCtrl	m_sldrAux;
	CSliderCtrl	m_sldrLin;
	CEdit	m_edtLin;
	CEdit	m_edtAux;
	CEdit	m_edtAnl;
	CEdit	m_edtMic;
	CSliderCtrl	m_sldrMic;
	//}}AFX_DATA

protected:
	int m_volMic;
	int m_volLin;
	int m_volAux;
	int m_volAnl;
	
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDefVolumeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void GetDefaultVolumeValues(void);
	int ChangeEdt(int Volume, CEdit * edt, CSliderCtrl * slider);

	// Generated message map functions
	//{{AFX_MSG(CDefVolumeDlg)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnChangeEdtMic();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEdtLin();
	afx_msg void OnChangeEdtAux();
	afx_msg void OnChangeEdtAnl();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEFVOLUMEDLG_H__AEDA9B15_8280_4749_B555_2723E8C7D7BD__INCLUDED_)
