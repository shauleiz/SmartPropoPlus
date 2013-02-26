// This is the Control Unit State Machine
#include "stdafx.h"
#include "control.h"
using namespace std;
#include "StateMachine.h"
#include <tinyxml.h>
#include <tinystr.h>


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

	// Read cofiguration file
	// Get file C:\Users\[User]\AppData\Roaming\SmartPropoPlus\Control.xml (if exists)
	// If does not exist move to state STATE_CONF and read the default configuration file
	// The  default configuration file is .\Control.xml
	HRESULT hr  = LoadConfigFromLocalFile();
	if (hr != S_OK)
		hr = LoadConfigFromDefaultFile();

	// Notify at the end of the inialization
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

	// Open configuration file
	HANDLE hConfigFile = CreateFile(path, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL ,NULL);
	if (hConfigFile == INVALID_HANDLE_VALUE)
	{ // Cannot open the configuration file
		return (HRESULT)GetLastError();
	};
	 
	// Now load configuration file
	CloseHandle(hConfigFile);
	return LoadConfigFromFile(path);
}

HRESULT CStateMachine::LoadConfigFromDefaultFile(void)
{
	// Open configuration file
	HANDLE hConfigFile = CreateFile(STR_DEFLT_CTRL_XML, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL ,NULL);
	if (hConfigFile == INVALID_HANDLE_VALUE)
	{ // Cannot open the configuration file
		return (HRESULT)GetLastError();
	};
	 
	// Now load configuration file
	CloseHandle(hConfigFile);
	return LoadConfigFromFile(STR_DEFLT_CTRL_XML);
}

HRESULT CStateMachine::LoadConfigFromFile(LPCWSTR FileName)
// Given XML configuration file the function read the file (Using TinyXML)
// Then updates state machine state and SPP configuration structure
{
	FILE * f;
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);
	MessageMap  messages;

	errno_t err = _wfopen_s(&f, FileName, L"r+b, ccs=UTF-8");
	if (err)
		return S_FALSE;

	// Open & load XML file with TunyXML 
	HRESULT hr = S_OK;
	TiXmlDocument * doc = new(TiXmlDocument);
	bool loaded = doc->LoadFile(f, TIXML_ENCODING_UTF8);
	if (!loaded)
	{
		const char * desc = doc->ErrorDesc();
		hr = ERROR_OPEN_FAILED;
		goto end;
	};

	TiXmlHandle hDoc(doc);

	pElem=hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem) 
	{
		hr = ERROR_OPEN_FAILED;
		goto end;
	};

	// Rootname is expected to be "SmartPropoPlus"
	const char * root_name = pElem->Value();
	if(!root_name)
	{
		hr = ERROR_OPEN_FAILED;
		goto end;
	};

	if (!IsIdentical(XML_ROOT_ELEM, root_name))
	{
		hr = GetLastError();
		goto end;
	};


	// OK - Rootname is indeed "SmartPropoPlus" 
	// save this for later
	hRoot=TiXmlHandle(pElem);

	/////////////// Now get capture endpoints ////////
	pElem=hRoot.FirstChild( "Audio" ).FirstChild().Element();
	for( pElem; pElem; pElem=pElem->NextSiblingElement())
	{
		const char *pKey=pElem->Value(); // <CaptureEndpoint></CaptureEndpoint>
		if (IsIdentical(L"CaptureEndPoint", pKey))
		{
			if (pElem->FirstChildElement("id")) const char *id_text = pElem->FirstChildElement("id")->GetText(); // id
			if (pElem->FirstChildElement("color")) const char *color_text = pElem->FirstChildElement("color")->GetText(); // jack colour
			if (pElem->FirstChildElement("friendlyName")) const char *color_frnm = pElem->FirstChildElement("friendlyName")->GetText(); // Friendly Name
		};
		int dummy = 5;
		//if (pKey && pText) 
		//{
		//	messages[pKey]=pText;
		//}
	}




end:
	delete(doc);
	fclose(f);
	return hr;
}

bool CStateMachine::IsIdentical(LPCWSTR wStr, const char * utf8)
{
	if (!wStr || !utf8)
		return false;

	WCHAR w_utf8[MAX_PATH];
	int mb = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w_utf8,0); 
	mb = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w_utf8,mb); 
	DWORD lasterr = GetLastError();
	if (lasterr)
		return false;

	if (wcscmp(wStr, w_utf8))
		return false;
	else
		return true;
}
bool CStateMachine::IsIdentical(const char * utf8, LPCWSTR wStr)
{
	return IsIdentical(wStr, utf8);
}