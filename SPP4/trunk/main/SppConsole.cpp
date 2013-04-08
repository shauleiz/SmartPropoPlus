// SppConsole.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SppConsole.h"
#include "SppConsoleDlg.h"




int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// Read Command line
	// TODO

	//Registers a system-wide messages:
	// WM_INTERSPPCONSOLE - to ensure that it is a singleton.
	// WM_INTERSPPAPPS - Messages to the user interface window.

	SppConsoleDlg Dialog = SppConsoleDlg(hPrevInstance);
	//Dialog.Hide();
	Dialog.Show();


	return 0;
}






