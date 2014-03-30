#include "stdafx.h"
#include "resource.h"
#include "SppTabGen.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabGenDlg(HWND, UINT, WPARAM, LPARAM);


SppTabGen::SppTabGen(void)
{
}

SppTabGen::SppTabGen(HINSTANCE hInstance, HWND TopDlgWnd) : 
	SppTab( hInstance,  TopDlgWnd,  IDD_GENERAL,  MsgHndlTabGenDlg)
{
}


SppTabGen::~SppTabGen(void)
{
}

INT_PTR CALLBACK MsgHndlTabGenDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabGen * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP,20,40, 0, 0,SWP_NOSIZE); 
		DialogObj = (SppTabGen *)lParam;

		return (INT_PTR)TRUE;

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
