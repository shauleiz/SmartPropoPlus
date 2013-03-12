// This is the Control Unit State Machine
#include "stdafx.h"
#include "config.h"
#include "control.h"
using namespace std;
#include "StateMachine.h"
#include <tinyxml.h>
#include <tinystr.h>
#include "WinMessages.h"


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
	m_state = UNDEF;

	// Read cofiguration file
	// Get file C:\Users\[User]\AppData\Roaming\SmartPropoPlus\Control.xml (if exists)
	// If does not exist move to state STATE_CONF and read the default configuration file
	// The  default configuration file is .\Control.xml
	HRESULT hr  = LoadConfigFromLocalFile();
	if (hr != S_OK)
	{	// Configuration file not found-
		// Read default (pre-installed) XML file and copy it to C:\Users\[User]\AppData\Roaming\SmartPropoPlus
		hr = LoadConfigFromDefaultFile();
		if (hr == S_OK)
		{
			m_state = CONF;
			SaveConfigToLocalFile();
			NotifyParent(WMAPP_SM_INIT, m_state);
			return TRUE;
		}
		else
		{
			m_state = ERR;
			NotifyParent(WMAPP_SM_INIT, m_state);
			return FALSE;
		}
	}

	// According to the current configuration deside on innitial state
	// 


	// Notify at the end of the inialization
	NotifyParent(WMAPP_SM_INIT, m_state);
	return TRUE;
}

CStateMachine::~CStateMachine()
{
}

LRESULT CStateMachine::NotifyParent(UINT message, WPARAM wParam, LPARAM lParam)
{
	return SendMessage(hWnd, message,  wParam,  lParam);
}

HRESULT CStateMachine::LoadConfigFromLocalFile(void)
{
	// Get the path to Get file C:\Users\[User]\AppData\Roaming
	TCHAR path[MAX_PATH];
	BOOL GotSpecialFolder = SHGetSpecialFolderPath(NULL, (LPTSTR)&path, CSIDL_APPDATA , FALSE);
	if (!GotSpecialFolder)
	{ // Strange error - can't find the user's roaming folder
		return ERROR_PATH_NOT_FOUND;
	};

	// Get the path to the configuration file
	errno_t concat = wcscat_s((LPTSTR)&path, MAX_PATH, STR_CTRL_XML);
	if (concat)
	{ // Cannot create the correct string
		return ERROR_BUFFER_OVERFLOW;
	};

	return LoadConfigFromFile(path);
}

void CStateMachine::SaveConfigToLocalFile(void)
{
	// Get the path to Get file C:\Users\[User]\AppData\Roaming
	TCHAR path[MAX_PATH];
	BOOL GotSpecialFolder = SHGetSpecialFolderPath(NULL, (LPTSTR)&path, CSIDL_APPDATA , FALSE);
	if (!GotSpecialFolder)
	{ // Strange error - can't find the user's roaming folder
		return;
	};

	// Get the path to the configuration file
	errno_t concat = wcscat_s((LPTSTR)&path, MAX_PATH, STR_CTRL_XML);
	if (concat)
	{ // Cannot create the correct string
		return;
	};

	SaveConfigToFile(path);
}

HRESULT CStateMachine::LoadConfigFromDefaultFile(void)
{
	return LoadConfigFromFile(STR_DEFLT_CTRL_XML);
}

HRESULT CStateMachine::LoadConfigFromFile(LPCWSTR FileName)
// Given XML configuration file the function read the file (Using TinyXML)
// Then updates state machine state and SPP configuration structure
{
	MessageMap  messages;
	CaptureEndPoint EndPoint;
	Decoder dcdr;

	if (m_config.LoadConfigFile(FileName))
	{
		bool found = m_config.ExistDecoder();
		found = m_config.GetDecoderbyIndex(&dcdr,0);
		found = m_config.GetDecoderbyIndex(&dcdr,1);
		found = m_config.GetDecoderbyIndex(&dcdr,2);
		found = m_config.GetSelectedDecoder(&dcdr);

		found = m_config.GetSelectedCaptureEP(&EndPoint);
		found = m_config.GetSelectedCaptureEP(&EndPoint, false);
		bool existEP = m_config.ExistCaptureEP();
		bool FirstEP = m_config.GetFirstCaptureEP(&EndPoint);
		FirstEP = m_config.GetCaptureEPbyIndex(&EndPoint, 0);
		FirstEP = m_config.GetCaptureEPbyIndex(&EndPoint, 1);
		FirstEP = m_config.GetCaptureEPbyIndex(&EndPoint, 2);
		FirstEP = m_config.GetCaptureEPbyIndex(&EndPoint, 3);
		FirstEP = m_config.GetCaptureEPbyIndex(&EndPoint, 4);
		FirstEP = m_config.GetCaptureEPbyIndex(&EndPoint, 5);
		return S_OK;
	}
	else
		return S_FALSE;


	return S_OK;
}


void CStateMachine::SaveConfigToFile(LPCWSTR FileName)
{
	LPWSTR t= _wcsdup(FileName);
	m_config.SaveConfigFile(t);
	delete(t);
}

