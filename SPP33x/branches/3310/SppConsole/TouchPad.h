#if !defined(AFX_TOUCHPAD_H__9906D4AD_4899_4B40_8201_28E5E7D7CBDB__INCLUDED_)
#define AFX_TOUCHPAD_H__9906D4AD_4899_4B40_8201_28E5E7D7CBDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TouchPad.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTouchPad window

class CTouchPad : public CStatic
{
// Construction
public:
	CTouchPad();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTouchPad)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTouchPad();

	// Generated message map functions
protected:
     DECLARE_DYNAMIC(CTouchPad)
	//{{AFX_MSG(CTouchPad)
	afx_msg UINT OnNcHitTest(CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOUCHPAD_H__9906D4AD_4899_4B40_8201_28E5E7D7CBDB__INCLUDED_)
