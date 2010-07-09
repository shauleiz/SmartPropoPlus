// SppConsole.cpp : Defines the class behaviors for the application.
//
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#include "stdafx.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"
#include "AboutDlg.h"
//#include "..\\GlobalData\\GlobalData.c"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleApp

BEGIN_MESSAGE_MAP(CSppConsoleApp, CWinApp)
	//{{AFX_MSG_MAP(CSppConsoleApp)
	ON_COMMAND(IDC_ABOUT, OnAbout)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleApp construction

CSppConsoleApp::CSppConsoleApp()
{
	_CrtDumpMemoryLeaks();
	int i=-1;
	_CrtSetBreakAlloc(i);

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSppConsoleApp object

CSppConsoleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSppConsoleApp initialization

BOOL CSppConsoleApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	bool StartIconified = doStartIconified();

	CSppConsoleDlg dlg;
	m_pMainWnd = &dlg;
	dlg.StartIconified(StartIconified);
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

bool CSppConsoleApp::doStartIconified()
{

	char * CmdLine;

	CmdLine = (char *)m_lpCmdLine;

	if (!CmdLine || CmdLine[0]== '\0')
		return false;

	if (strstr(CmdLine,"-i") || strstr(CmdLine,"-I"))
		return true;
	else
		return false;
}


void CSppConsoleApp::OnAbout() 
{
	CAboutDlg dlg;
	dlg.DoModal();
}

