// SppConsole.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Shellapi.h>
#include "SmartPropoPlus.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"




int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	HANDLE hDlgCLosed=NULL;

	// Read Command line
	// TODO: This is only an example of how to parse the command line
	int argc = 0;
	LPWSTR* argv = NULL;
	argv = CommandLineToArgvW(lpCmdLine, &argc);
	LocalFree(argv);
	// Read Command line (End)

	//Registers a system-wide messages:
	// WM_INTERSPPCONSOLE - to ensure that it is a singleton.
	// WM_INTERSPPAPPS - Messages to the user interface window.
	TCHAR msg[MAX_MSG_SIZE];
	WM_INTERSPPCONSOLE	= RegisterWindowMessage(INTERSPPCONSOLE);
	if (!WM_INTERSPPCONSOLE)
	{
		_stprintf_s(msg, TEXT("wWinMain(): WM_INTERSPPCONSOLE = %d - cannot register window message INTERSPPCONSOLE"), MAX_MSG_SIZE, WM_INTERSPPCONSOLE);
		::MessageBox(NULL,msg, TEXT("SmartPropoPlus Message") , MB_SYSTEMMODAL);
		return -1;
	};
	WM_INTERSPPAPPS		= RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{
		_stprintf_s(msg, TEXT("wWinMain(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS"), MAX_MSG_SIZE, WM_INTERSPPAPPS);
		::MessageBox(NULL,msg, TEXT("SmartPropoPlus Message") , MB_SYSTEMMODAL);
		return -1;
	};

		
	/* Test if another SPPConsole is running */
	HANDLE hMuxex;
	if (hMuxex=OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXCONSOLE))
	{	// another instance is already running and the second instance is NOT 
		// launched iconified then Broadcast a message.
		//TODO: if (!m_Iconified)
			::PostMessage(HWND_BROADCAST, WM_INTERSPPCONSOLE, 0, 0);
		return -2;
	}
	else
		hMuxex = CreateMutex(NULL, FALSE, MUTXCONSOLE);

	// Create Dialog box, initialize it then show it
	SppConsoleDlg * Dialog = new SppConsoleDlg(hInstance);
	Dialog->Show();

	// Loop forever in the dialog box until user kills it
	Dialog->MsgLoop();
	delete(Dialog);

	return 0;
}






