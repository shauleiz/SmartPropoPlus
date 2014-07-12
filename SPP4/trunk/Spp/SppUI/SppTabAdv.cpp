#include "stdafx.h"
#include "Commctrl.h"
#include "resource.h"
#include "WinMessages.h"
#include "SmartPropoPlus.h"
#include "SppTabAdv.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabAdvDlg(HWND, UINT, WPARAM, LPARAM);

static const int g_Controls[] = {
		/* Advanced Tab (IDD_ADVANCED ) */
		IDC_CH_LOG, IDC_CH_INSIG, IDC_CH_PULSE, IDC_PLS_SCOPE,
	};


SppTabAdv::SppTabAdv(void)
{
}


SppTabAdv::~SppTabAdv(void)
{
}


SppTabAdv::SppTabAdv(HINSTANCE hInstance, HWND TopDlgWnd) :  SppTab( hInstance,  TopDlgWnd,  IDD_ADVANCED, MsgHndlTabAdvDlg)
{
}

INT_PTR CALLBACK MsgHndlTabAdvDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabAdv * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppTabAdv *)lParam;
		DialogObj->CreateToolTip(hDlg, g_Controls, sizeof(g_Controls)/sizeof(int)); // Initialize tooltip object
		return (INT_PTR)TRUE;
	default:
		break;

	};

	return (INT_PTR)FALSE;
}

