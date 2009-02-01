// JsChPostProc.h : main header file for the JSCHPOSTPROC DLL
//

#if !defined(AFX_JSCHPOSTPROC_H__C4398AC3_C0EE_482C_9A3C_62C118250D4D__INCLUDED_)
#define AFX_JSCHPOSTPROC_H__C4398AC3_C0EE_482C_9A3C_62C118250D4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#define FILTER_VER	0x00030100

typedef struct _JS_CHANNELS	// Joystick channel data
{
	int ch;			// Number of channels
	int * value;	// Pointer to data buffer ; ch>n>=0 ; value[n] holds channel (n+1) 
} JS_CHANNELS, * PJS_CHANNELS;

struct FilterItem {
	int	id;				// Filter ID
	void * Function;	// Filter Function
	char * DisplayName;	// Filter name as displayed in the menu
} ;

typedef  PJS_CHANNELS ( * CC)(PJS_CHANNELS, int max, int min);

extern "C" LONG PASCAL EXPORT GetDllVersion();
extern "C" PJS_CHANNELS  PASCAL EXPORT ProcessChannels(PJS_CHANNELS, int max, int min);
extern "C" int  PASCAL EXPORT GetNumberOfFilters(void);
extern "C" const char *  PASCAL EXPORT GetFilterNameByIndex(const int iFilter);
extern "C" const int  PASCAL EXPORT GetFilterIdByIndex(const int iFilter);
extern "C" const int  PASCAL EXPORT SelectFilterByIndex(const int iFilter);
extern "C" const int  PASCAL EXPORT GetIndexOfSelectedFilter(void);

CC ConvertChannels;
PJS_CHANNELS ConvertEskyCCPM(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertEskyCCPM_1(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertEskyCCPM_2(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertDummy(PJS_CHANNELS, int max, int min);

/////////////////////////////////////////////////////////////////////////////
// CJsChPostProcApp
// See JsChPostProc.cpp for the implementation of this class
//
class CJsChPostProcApp : public CWinApp
{
public:
	CJsChPostProcApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CJsChPostProcApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CJsChPostProcApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JSCHPOSTPROC_H__C4398AC3_C0EE_482C_9A3C_62C118250D4D__INCLUDED_)
