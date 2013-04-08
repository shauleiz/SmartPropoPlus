#include "stdafx.h"
#include "SppConsoleDlg.h"
#include "SppConsole.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	MsgHndlDlg(HWND, UINT, WPARAM, LPARAM);

SppConsoleDlg::SppConsoleDlg(void)
{
}

SppConsoleDlg::SppConsoleDlg(HINSTANCE hInstance)
{

	m_hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_SPPDIAG), NULL, MsgHndlDlg);
	return;
}

SppConsoleDlg::~SppConsoleDlg(void)
{
}

void SppConsoleDlg::Show()
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}

void SppConsoleDlg::Hide()
{
	ShowWindow(m_hDlg, SW_HIDE);
}


// Message handler for spp dialog box.
INT_PTR CALLBACK MsgHndlDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

