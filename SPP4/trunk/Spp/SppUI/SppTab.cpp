#include "stdafx.h"
#include "Windowsx.h"
#include "SmartPropoPlus.h"
#include "WinMessages.h"
#include "public.h"
#include "Commctrl.h"
#include "resource.h"
#include "vJoyMonitor.h"
#include "SppBtnsDlg.h"
#include "SppDlg.h"
#include "SppTab.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabDlg(HWND, UINT, WPARAM, LPARAM);


SppTab::SppTab(void)
{
}

SppTab::SppTab(HINSTANCE hInstance, HWND TopDlgWnd, int Id, DLGPROC MsgHndlDlg) : 
	m_hDlg(0),
	m_DlgId(-1)
{
	m_hInstance = hInstance;
	m_TopDlgWnd = TopDlgWnd;
	m_DlgId = Id;

	// Create the dialog box (Hidden) 
	m_hDlg = CreateDialogParam((HINSTANCE)hInstance, MAKEINTRESOURCE(m_DlgId), m_TopDlgWnd, MsgHndlDlg, (LPARAM)this);	

	return;
}

SppTab::~SppTab(void)
{
}

HWND SppTab::GetHandle(void)
{
	return m_hDlg;
}


int SppTab::GetId(void)
{
	return m_DlgId;
}

void SppTab::Show()
{
	ShowWindow(m_hDlg, SW_SHOW);
	UpdateWindow(m_hDlg);	
}
 
void SppTab::Hide()	
{
	ShowWindow(m_hDlg, SW_HIDE);
}


INT_PTR CALLBACK MsgHndlTabDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppDlg * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		DialogObj = (SppDlg *)lParam;
		return (INT_PTR)TRUE;

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
