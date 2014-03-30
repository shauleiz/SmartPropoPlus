#include "stdafx.h"
#include "resource.h"
#include "SppTabAudio.h"

// GLobals
INT_PTR CALLBACK	MsgHndlTabAudioDlg(HWND, UINT, WPARAM, LPARAM);

SppTabAudio::SppTabAudio(void)
{
}

SppTabAudio::SppTabAudio(HINSTANCE hInstance, HWND TopDlgWnd) : SppTab( hInstance,  TopDlgWnd,  IDD_AUDIO, MsgHndlTabAudioDlg)
{
}

SppTabAudio::~SppTabAudio(void)
{
}


INT_PTR CALLBACK MsgHndlTabAudioDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static SppTabAudio * DialogObj = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowPos(hDlg, HWND_TOP,20,40, 0, 0,SWP_NOSIZE); 
		DialogObj = (SppTabAudio *)lParam;
		return (INT_PTR)TRUE;

	default:
		break;

	};

	return (INT_PTR)FALSE;
}
