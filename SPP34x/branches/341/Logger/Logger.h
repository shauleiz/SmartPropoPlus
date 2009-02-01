// Logger.h : main header file for the Logger DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CLoggerApp
// See Logger.cpp for the implementation of this class
//

class CLoggerApp : public CWinApp
{
public:
	CLoggerApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
