// This is the Control Unit State Machine
#include "stdafx.h"
#include "control.h"
#include "StateMachine.h"

CStateMachine::CStateMachine()
{
	hWnd = NULL ;
	hInst = NULL;
}

BOOL CStateMachine::InitInstance(HWND const hWindow, HINSTANCE const hInstance)
{
	// If already initialized the don't do anything
	if (hWnd || hInst)
		return FALSE;

	// Partial initialization is not permitted
	if (!hWindow || !hInstance)
		return FALSE;

	// Initialize
	hWnd = hWindow;
	hInst = hInstance;
	state = UNDEF;
	NotifyParent(WMAPP_SM_INIT);
	return TRUE;
}

CStateMachine::~CStateMachine()
{
}

LRESULT CStateMachine::NotifyParent(UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(hWnd, message,  wParam,  lParam);
}