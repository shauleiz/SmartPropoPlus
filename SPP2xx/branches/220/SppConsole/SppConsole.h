// SppConsole.h : main header file for the SPPCONSOLE application
//

#if !defined(AFX_SPPCONSOLE_H__2F47B91B_1A9D_49A2_B6B5_E299A349477C__INCLUDED_)
#define AFX_SPPCONSOLE_H__2F47B91B_1A9D_49A2_B6B5_E299A349477C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleApp:
// See SppConsole.cpp for the implementation of this class
//
class CGlobalData;

class CSppConsoleApp : public CWinApp
{
public:
	CSppConsoleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSppConsoleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSppConsoleApp)
	afx_msg void OnAbout();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
protected:
	bool doStartIconified(void);
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPPCONSOLE_H__2F47B91B_1A9D_49A2_B6B5_E299A349477C__INCLUDED_)
