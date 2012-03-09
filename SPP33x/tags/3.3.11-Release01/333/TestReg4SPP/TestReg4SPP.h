// TestReg4SPP.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CTestReg4SPPApp:
// See TestReg4SPP.cpp for the implementation of this class
//
#define REG_LOG_FILE	"SPP_reg.log"

class CTestReg4SPPApp : public CWinApp
{
public:
	CTestReg4SPPApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CTestReg4SPPApp theApp;