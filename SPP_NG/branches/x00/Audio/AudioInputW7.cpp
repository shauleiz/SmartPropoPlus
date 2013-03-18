// AudioInputW7.cpp : Defines the class CAudioInputW7 that enumerates all capture endpoints
//

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include <Audioclient.h>
#include <devicetopology.h>
#include <Mmdeviceapi.h>
#include "WinMessages.h"
#include "AudioInputW7.h"
#include "NotificationClient.h"
#include <Functiondiscoverykeys_devpkey.h>
#include <endpointvolume.h>
#include <Mmdeviceapi.h>
#include "PolicyConfig.h"
#include "PulseData.h"

#define MAX_LOADSTRING 100

//{A95664D2-9614-4F35-A746-DE8DB63617E6}
static GUID const CLSID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

//BCDE0395-E52F-467C-8E3D-C4579291692E
static GUID const CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };

// Globals
void   DbgPopUp(int Line, DWORD Error)
{
#ifdef _DEBUG
	char Combined[1023];

	sprintf_s(Combined,1000,"Audio Error: 0x%x in line %d",Error, Line);

	MessageBoxA(NULL, Combined, "Error",  MB_SYSTEMMODAL|MB_ICONERROR);
#endif
}
bool   g_CaptureAudioThreadRunnig;		// If false - signal to capture audio thread to stop
HANDLE g_hEventStopCaptureAudioThread;	// Handle to event that is set when the audio thread is stopping
HANDLE g_hAudioBufferReady = NULL;

DWORD WINAPI CaptureAudio(LPVOID param)
/*
	Capture loop - running on a dedicated thread
	param: Pointer to the calling CAudioInputW7 object
*/
{
	HRESULT hr = S_OK;
	// Initialize capture thread
	CAudioInputW7 * parent = (CAudioInputW7 *)param;
	//CPulseData * pPulseDataObj = new CPulseData;
	parent->InitPulseDataObj(parent->m_pPulseDataObj);

	// Capture loop
	do
	{
		hr = parent->ProcessAudioPacket(parent->m_pPulseDataObj);
	}while (g_CaptureAudioThreadRunnig);

	// Exit capture thread
	delete parent->m_pPulseDataObj;
	parent->m_pPulseDataObj = NULL;
	SetEvent(g_hEventStopCaptureAudioThread);
	return 0;
}

void Log(int Code, int Severity, LPVOID Data)
{
	// Default (NO-OP) logger
}

void AudioLog(int Code,  int size, LPVOID Data, LPVOID Param)
{
	// Default (NO-OP) logger
}

void DefProcPulse(int length,  bool low, LPVOID Param)
{
	// Default (NO-OP) logger
}

LPWSTR GetWasapiText(DWORD code)
/* Translate WASAPI error codes to English */
{
	switch (code)
	{
	case	AUDCLNT_E_NOT_INITIALIZED:
		return L"The audio stream has not been successfully initialized";
	case	AUDCLNT_E_ALREADY_INITIALIZED:
		return L"The IAudioClient object is already initialized";

	case	AUDCLNT_E_WRONG_ENDPOINT_TYPE:
		return L"Wrong Endpoint Type";

	case	AUDCLNT_E_DEVICE_INVALIDATED:
		return L"Device Invalidated";

	case	AUDCLNT_E_NOT_STOPPED:
		return L"The audio stream was not stopped";

	case	AUDCLNT_E_BUFFER_TOO_LARGE:
		return L"The number of requested frames exceeds the available buffer space ";

	case	AUDCLNT_E_OUT_OF_ORDER:
		return L"Out of Order";

	case	AUDCLNT_E_UNSUPPORTED_FORMAT:
		return L"Unsupported Format";

	case	AUDCLNT_E_INVALID_SIZE:
		return L"Invalid Size";

	case	AUDCLNT_E_DEVICE_IN_USE:
		return L"The endpoint device is already in use";

	case	AUDCLNT_E_BUFFER_OPERATION_PENDING:
		return L"Buffer cannot be accessed because a stream reset is in progress";

	case	AUDCLNT_E_THREAD_NOT_REGISTERED:
		return L"Thread not registered";

	case	AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED:
		return L"Exclusive mode not allowed";

	case	AUDCLNT_E_ENDPOINT_CREATE_FAILED:
		return L"Creation of Endpoint failed";

	case	AUDCLNT_E_SERVICE_NOT_RUNNING:
		return L"Service not Running";

	case	AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED:
		return L"The audio stream was not initialized for event-driven buffering";

	case	AUDCLNT_E_EXCLUSIVE_MODE_ONLY:
		return L"Exclusive Mode Only";

	case	AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL:
		return L"parameters Buffer Duration and Periodicity are not equal";

	case	AUDCLNT_E_EVENTHANDLE_NOT_SET:
		return L"Eventhandle Not Set";

	case	AUDCLNT_E_INCORRECT_BUFFER_SIZE:
		return L"Incorrect Buffer Size";

	case	AUDCLNT_E_BUFFER_SIZE_ERROR:
		return L"Buffer Size Error";

	case	AUDCLNT_E_CPUUSAGE_EXCEEDED:
		return L"CPU Usage Exceeded";

	case	E_INVALIDARG:
		return L"Invalid Argument";

	case	AUDCLNT_E_BUFFER_ERROR:
		return L" GetBuffer failed to retrieve a data buffer and *ppData points to NULL.";

	default:
		LPWSTR errcode = new WCHAR[100];
		swprintf(errcode, 100, L"Unknown error code 0x%x", code);
		return errcode;
	};
		return NULL;

}

BOOL IsWindows7OrLater(void)
{
    // Initialize the OSVERSIONINFOEX structure to Windows7.
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osvi.dwMajorVersion = 6;
    osvi.dwMinorVersion = 1;

    // Initialize the condition mask for Windows7.
    DWORDLONG dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

    // Perform the test.
    if (VerifyVersionInfo(	&osvi,  VER_MAJORVERSION | VER_MINORVERSION ,	dwlConditionMask))
		return TRUE; // Windows 7

	return VerifyVersionInfo(	&osvi, VER_MAJORVERSION | VER_MINORVERSION, dwlConditionMask);
}


////////////////////// Class CAudioInputW7 //////////////////////
SPPINTERFACE_API CAudioInputW7::CAudioInputW7(HWND hWnd)
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pNotifyChange = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
	m_nEndPoints = 0;
	HRESULT hr = S_OK;
	m_nMixers = 0;
	m_CaptureDevices.clear();
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Save handle to parent window - will be used fpr notifications
	m_hPrntWnd = hWnd;

	// Set default callback functions
	RegisterLog(Log);
	RegisterAudioLog(AudioLog);

	// Create a vector of capture endpoint
	bool created = Create();
	// First Enumeration
	Enumerate();
	return;

}

CAudioInputW7::CAudioInputW7(void) 
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pNotifyChange = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
	m_nEndPoints = 0;
	HRESULT hr = S_OK;
	m_nMixers = 0;
	m_CaptureDevices.clear();
	// m_ChangeEventCB = NULL;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	m_hPrntWnd = NULL;

	// Set default callback functions
	RegisterLog(Log);
	RegisterAudioLog(AudioLog);

	bool created = Create();
	
	// First Enumeration
	Enumerate();

	return;
}

bool CAudioInputW7::Create(void)
/*
	Call this function once to initialize audio-related structures
	Create a device enumarator (m_pEnumerator)
	Create collection of capture endpoints including disabled and unplugged (m_pCaptureDeviceCollect)
	Get the number of endpoints in the collection m_nEndPoints
	Register endpoint notofication callback
	Pass pointer to this CAudioInputW7object as parent of the notification object
*/
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pNotifyChange = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
	m_nEndPoints = 0;
	HRESULT hr = S_OK;
	m_pCaptureClient = NULL;
	m_pAudioClient = NULL;
	m_hCaptureAudioThread = NULL;
	g_hEventStopCaptureAudioThread = NULL;
	g_CaptureAudioThreadRunnig = false;
	m_CurrentWaveFormat.cbSize = 0;
	m_CurrentWaveFormat.nAvgBytesPerSec = 0;
	m_CurrentWaveFormat.nBlockAlign = 0;
	m_CurrentWaveFormat.nChannels = 0;
	m_CurrentWaveFormat.nSamplesPerSec = 0;
	m_CurrentWaveFormat.wBitsPerSample = 0;
	m_CurrentWaveFormat.wFormatTag = 0;
	m_CurrentChannelIsRight = false;
	m_pPulseDataObj = NULL;
	ProcessPulse = DefProcPulse;
	m_ProcPulseParam = NULL;


	/* Create a device enumarator then a collection of endpoints and finally get the number of endpoints */
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
	if (FAILED(hr))
	{
		MessageBox(NULL, TEXT("WASAPI (CAudioInputW7): Could not create audio Endpoint enumerator \r\nStopping audio capture"), L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create a list of all Capture Endpoints
	DWORD dwStateMask = DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_UNPLUGGED;
	hr = m_pEnumerator->EnumAudioEndpoints(eCapture,  dwStateMask, &m_pCaptureDeviceCollect);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not enumerate capture audio Endpoint\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Get the number of Capture Endpoints
	hr = m_pCaptureDeviceCollect->GetCount(&m_nEndPoints);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not count audio Endpoints\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create an event handle for buffer-event notifications.
	g_hAudioBufferReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!g_hAudioBufferReady)
    {
        hr = E_FAIL;
        goto Exit;
    };


	// Register endpoint notofication callback & pass pointer to this object as its parent
	hr = RegisterNotification();
	if (FAILED(hr))
		EXIT_ON_ERROR(hr);
	return true;

Exit:
	if (m_pEnumerator && m_pNotifyChange)
		m_pEnumerator->UnregisterEndpointNotificationCallback(m_pNotifyChange);
	SAFE_RELEASE(m_pNotifyChange);
	SAFE_RELEASE(m_pEnumerator);
	SAFE_RELEASE(m_pCaptureDeviceCollect);
	SAFE_RELEASE(m_pRenderDeviceCollect);
	return false;
}

CAudioInputW7::~CAudioInputW7(void)
{
	if (m_pEnumerator && m_pNotifyChange)
		m_pEnumerator->UnregisterEndpointNotificationCallback(m_pNotifyChange);
	SAFE_RELEASE(m_pEnumerator);
	SAFE_RELEASE(m_pNotifyChange);
	SAFE_RELEASE(m_pCaptureDeviceCollect);
}


HRESULT	CAudioInputW7::Enumerate(void)
// Fill in structure m_CaptureDevices that specifies the current state of all capture endpoint devices
// Assumption: 
//	1. m_nEndPoints is valid
//	2. m_pCaptureDeviceCollect is already initialized
{
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	HRESULT hr = S_OK;

	LogStatus(GEN_STATUS,INFO,ENUM1,m_LogParam);

	// Initialize data staructure
	while (!m_CaptureDevices.empty())
	{
		delete m_CaptureDevices.back()->DeviceName;
		delete m_CaptureDevices.back()->id;
		m_CaptureDevices.pop_back();
	};

	// Loop on all endpoints
	for (UINT iEndPoint=0; iEndPoint<m_nEndPoints; iEndPoint++)
	{
		// Get pointer to endpoint number iEndPoint.
		hr = m_pCaptureDeviceCollect->Item(iEndPoint, &pDevice);
		EXIT_ON_ERROR(hr);

		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the Device's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);

		// Get the state of the device:
		// DEVICE_STATE_ACTIVE / DEVICE_STATE_DISABLED / DEVICE_STATE_NOTPRESENT / DEVICE_STATE_UNPLUGGED
		DWORD state;
		hr = pDevice->GetState(&state);
		EXIT_ON_ERROR(hr);

		// Get device unique id
		LPWSTR id;
		hr = pDevice->GetId(&id);
		EXIT_ON_ERROR(hr);

		// Copy data to vector member
		m_CaptureDevices.push_back(new CapDev);
		m_CaptureDevices.back()->DeviceName = _wcsdup(varName.pwszVal);
		m_CaptureDevices.back()->state = state;
		m_CaptureDevices.back()->id = _wcsdup(id);
		//LogStatus(ENUM_UID,INFO,(LPVOID)varName.pwszVal);
		//LogStatus(ENUM_FRND,INFO,(LPVOID)id);

		CoTaskMemFree(id);
		id = NULL;
		PropVariantClear(&varName);
		SAFE_RELEASE(pProps)
		SAFE_RELEASE(pDevice)

	};


Exit:
	SAFE_RELEASE(pProps);
	SAFE_RELEASE(pDevice);

	return hr;

};

HRESULT CAudioInputW7::SetDefaultAudioDevice(PVOID devID)
{	
	IPolicyConfigVista *pPolicyConfigVista;
	IPolicyConfig *pPolicyConfig;
	ERole reserved = eConsole;
	HRESULT hr;


	// WIndows 7 and up
	if (IsWindows7OrLater())
	{
		hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_ALL, __uuidof(IPolicyConfig), (LPVOID *)&pPolicyConfig);
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfig->SetEndpointVisibility((LPCWSTR)devID, true);
			hr = pPolicyConfig->SetDefaultEndpoint((LPCWSTR)devID, reserved);
			pPolicyConfig->Release();
		};
	}
	else
	{ // Vista SP2
		hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
			NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfigVista);
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfigVista->SetEndpointVisibility((LPCWSTR)devID, true);
			hr = pPolicyConfigVista->SetDefaultEndpoint((LPCWSTR)devID, reserved);
			pPolicyConfigVista->Release();
		};
	};

	// Logging
	if (SUCCEEDED(hr))
		LogStatus(CHANGE_DEFDEV,INFO,devID,m_LogParam);
	else
		LogStatus(CHANGE_DEFDEV,WARN,GetWasapiText(hr),m_LogParam);

	return hr;
}

HRESULT CAudioInputW7::EnableAudioDevice(PVOID devID, bool Enable)
{
	IPolicyConfigVista *pPolicyConfigVista;
	IPolicyConfig *pPolicyConfig;
	ERole reserved = eConsole;
	HRESULT hr;


	// WIndows 7 and up
	if (IsWindows7OrLater())
	{
		hr = CoCreateInstance(__uuidof(CPolicyConfigClient), NULL, CLSCTX_ALL, __uuidof(IPolicyConfig), (LPVOID *)&pPolicyConfig);
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfig->SetEndpointVisibility((LPCWSTR)devID, Enable);
			pPolicyConfig->Release();
		};
	}
	else
	{ // Vista SP2
		hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
			NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfigVista);
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfigVista->SetEndpointVisibility((LPCWSTR)devID, Enable);
			pPolicyConfigVista->Release();
		};
	};

	// Logging
	if (SUCCEEDED(hr))
		LogStatus(CHANGE_ENDEV,INFO,devID,m_LogParam);
	else
		LogStatus(CHANGE_ENDEV,WARN,GetWasapiText(hr),m_LogParam);

	return hr;
}


int		CAudioInputW7::CountCaptureDevices(void)
{		
	return (int)m_CaptureDevices.size();
}

HRESULT	CAudioInputW7::GetCaptureDeviceId(int nDevice, int *size, PVOID *Id)
{
	// Given the serial number (1-based) of a capture device (Which is meaningless),
	// the function passes a pointer to the id (which uniquely identifies the capture device).
	// The function also passes the size in bytes of the id

	// Sanity check
	if (nDevice<1 || nDevice> CountCaptureDevices())
		return FWP_E_OUT_OF_BOUNDS;

	if (m_CaptureDevices[nDevice-1]->id)
	{
		*Id = (PVOID)m_CaptureDevices[nDevice-1]->id;
		*size = (int)wcslen(m_CaptureDevices[nDevice-1]->id)*sizeof(WCHAR);
		return S_OK;
	}
	else
		return S_FALSE;
}

HRESULT	CAudioInputW7::GetCaptureDeviceName(PVOID Id, LPWSTR * DeviceName)
// Given a pointer to the id (which uniquely identifies the capture device),
// the function passes the Capture Device friendly name
{
	for (UINT  i = 0; i<m_CaptureDevices.size(); i++)
	{
		if (!Id)
			continue;
		if (!wcscmp(m_CaptureDevices[i]->id, (LPWSTR)Id))
		{
			*DeviceName = m_CaptureDevices[i]->DeviceName;
			return S_OK;
		}
	};

	*DeviceName = NULL;
	return S_FALSE;
}

bool	CAudioInputW7::IsCaptureDeviceActive(PVOID Id)
{
	// Given a pointer to the id (which uniquely identifies the capture device),
	// the function returns 'true' if the capture device is active
	// This function must be executed only after Enumerate()
	for (UINT i = 0; i<m_CaptureDevices.size(); i++)
	{
		if (!wcscmp(m_CaptureDevices[i]->id, (LPWSTR)Id))
			return m_CaptureDevices[i]->state & DEVICE_STATE_ACTIVE;
	};

	return false;
}

bool	CAudioInputW7::IsCaptureDevice(PVOID Id)
{
	IMMEndpoint * pEndpoint = NULL;
	IMMDevice *pDevice = NULL;
	EDataFlow dataflow;
	bool result = false;

	// Get device by ID
	HRESULT hr = m_pEnumerator->GetDevice((LPWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(ISCAP_IDNOTFOUND,WARN,Id,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Get endpoint interface
	hr = pDevice->QueryInterface(__uuidof(IMMEndpoint), (VOID **)&pEndpoint);
	if (FAILED(hr))
	{
		LogStatus(ISCAP_EPNOTFOUND,ERR,Id,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Sanity chack
	pEndpoint->GetDataFlow(&dataflow);
	if (dataflow != eCapture)
		EXIT_ON_ERROR(hr);

	result = true;

Exit:
	SAFE_RELEASE(pEndpoint);
	SAFE_RELEASE(pDevice);
	return result;
}


bool	CAudioInputW7::IsExternal(PVOID Id)
/*
	Based on: http://msdn.microsoft.com/en-us/library/windows/desktop/dd371387(v=vs.85).aspx
*/
{
	IDeviceTopology *pDeviceTopology = NULL;
	//IMMEndpoint * pEndpoint = NULL;
	IMMDevice *pDevice = NULL;
    IConnector *pConnFrom = NULL;
    IConnector *pConnTo = NULL;
    IPart *pPart = NULL;
	ConnectorType Type = Unknown_Connector;
	bool result = false;

	HRESULT hr = m_pEnumerator->GetDevice((LPWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(ISEXT_IDNOTFOUND,WARN,Id,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

    // Get the endpoint device's IDeviceTopology interface.
    hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDeviceTopology);
	if (FAILED(hr))
	{
		LogStatus(ISEXT_TOPO,WARN,Id,m_LogParam);
		EXIT_ON_ERROR(hr);
	};
		
    // The device topology for an endpoint device always
    // contains just one connector (connector number 0).
    hr = pDeviceTopology->GetConnector(0, &pConnFrom);
	if (FAILED(hr))
	{
		LogStatus(ISEXT_NOCONN,WARN,Id,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

    // Step across the connection to the jack on the adapter.
    hr = pConnFrom->GetConnectedTo(&pConnTo);
    if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        // The adapter device is not currently active.
        hr = E_NOINTERFACE;
    }
    EXIT_ON_ERROR(hr);

	hr = pConnTo->GetType(&Type);
	if (FAILED(hr))
	{
		LogStatus(ISEXT_NOTYPE,WARN,Id,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	if (Type == Physical_External)
		result = true;

Exit:
	// BUG: UN-COmment when fixed SAFE_RELEASE(pConnTo)
    SAFE_RELEASE(pConnFrom)
    SAFE_RELEASE(pDeviceTopology)
	SAFE_RELEASE(pDevice);
	return result;
}
SPPINTERFACE_API bool CAudioInputW7::IsDisconnected(PVOID Id)
{
	// Return true only if positively the jack is disconnected
	// Returns false if the jack is connected OR if information not available

	// Get Jack info
	KSJACK_DESCRIPTION  JackDesc;
	HRESULT hr = GetJackInfo((PVOID) Id, &JackDesc);
	if (FAILED(hr))
		return false;

	if (!JackDesc.IsConnected)
		return true;
	else 
		return false;

}
SPPINTERFACE_API COLORREF CAudioInputW7::GetJackColor(PVOID Id)
/* Get the colour of the input jack
   Provided that the jack:
   * The sound cart supports Jack Interface Information
   * Is connected
   * The port connection is typed 'ePortConnJack'
   * The colour is valid

   Return Value:
   If valid: RGB Value of the Jack
   If invalid: 0xFFFFFFFF
*/
{
	HRESULT hr = S_OK;
	COLORREF out = 0xFFFFFFFF;

	// Get Jack info
	KSJACK_DESCRIPTION  JackDesc;
	hr = GetJackInfo((PVOID) Id, &JackDesc);
	if (FAILED(hr))
		return out;

	if (JackDesc.IsConnected && JackDesc.PortConnection == ePortConnJack)
		return JackDesc.Color;
	else 
		return out;
}

SPPINTERFACE_API HRESULT CAudioInputW7::GetJackInfo(PVOID Id, KSJACK_DESCRIPTION *pJackDescData)
//-----------------------------------------------------------
// Get the IKsJackDescription interface that describes the
// audio jack or jacks that the endpoint device plugs into.
//
// Based on http://msdn.microsoft.com/en-us/library/windows/desktop/dd371387(v=vs.85).aspx 
//-----------------------------------------------------------
{
    HRESULT hr = S_OK;
    IDeviceTopology *pDeviceTopology = NULL;
    IConnector *pConnFrom = NULL;
    IConnector *pConnTo = NULL;
    IPart *pPart = NULL;
    IKsJackDescription *pJackDesc = NULL;
	IMMDevice *pDevice = NULL;

	// Get device from ID
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_IDNOTFOUND,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_IDNOTFOUND,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};


    if (NULL == pDevice || NULL == pJackDescData)
    {
        return E_POINTER;
    }

    // Get the endpoint device's IDeviceTopology interface.
    hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL,
                           NULL, (void**)&pDeviceTopology);
    EXIT_ON_ERROR(hr)

    // The device topology for an endpoint device always
    // contains just one connector (connector number 0).
    hr = pDeviceTopology->GetConnector(0, &pConnFrom);
    EXIT_ON_ERROR(hr)

    // Step across the connection to the jack on the adapter.
    hr = pConnFrom->GetConnectedTo(&pConnTo);
    if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        // The adapter device is not currently active.
        hr = E_NOINTERFACE;
    }
    EXIT_ON_ERROR(hr)

    // Get the connector's IPart interface.
    hr = pConnTo->QueryInterface(__uuidof(IPart), (void**)&pPart);
    EXIT_ON_ERROR(hr)

    // Activate the connector's IKsJackDescription interface.
    hr = pPart->Activate(CLSCTX_INPROC_SERVER,
                         __uuidof(IKsJackDescription), (void**)&pJackDesc);
    EXIT_ON_ERROR(hr);
	
	hr = pJackDesc->GetJackDescription(0, pJackDescData);
    EXIT_ON_ERROR(hr);


Exit:
    SAFE_RELEASE(pDeviceTopology)
    SAFE_RELEASE(pConnFrom)
    SAFE_RELEASE(pConnTo)
    SAFE_RELEASE(pPart)
    return hr;
}



SPPINTERFACE_API int CAudioInputW7::GetNumberChannels(PVOID Id)
// Return number of channels of an endpoint device
{
	IMMDevice *pDevice = NULL;
	HRESULT hr = S_OK;
	UINT nChannels = 0;
	float *PeakValue = NULL;
	IAudioClient *pAudioClient = NULL;
	WAVEFORMATEX *pwfx = NULL;

	// Get device from ID
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_IDNOTFOUND,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_IDNOTFOUND,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	///// Activate  a client ///////////////////////
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_ACTCLNT,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_ACTCLNT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	///// Get client mix format
	hr = pAudioClient->GetMixFormat(&pwfx);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_MXFRMT,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_MXFRMT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	nChannels = pwfx->nChannels;


Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pDevice);
		
	return nChannels;
}
bool	CAudioInputW7::IsCaptureDeviceDefault(PVOID Id)
{
// Given a pointer to the id (which uniquely identifies the capture device),
// the function returns 'true' if the capture device is default(colsole) capture endpoint

	// Get the default capture console endpoint
	IMMDevice *pDevice = NULL;
	HRESULT hr = m_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
	if (hr != S_OK)
		return false;

	// Get the endpoint's id
	LPWSTR DefaultId;
	hr = pDevice->GetId(&DefaultId);
	if (hr != S_OK)
		return false;

	// Compare to given Id
	return(!wcscmp(DefaultId, (LPWSTR)Id));
}


int 	CAudioInputW7::FindCaptureDevice(PVOID Id)
// Given Capture Device ID this function returns device serial number (1-based)
// Return 0 if not found
{
	if (m_CaptureDevices.empty() || !m_CaptureDevices.size())
		return 0;

	for (UINT i=0; i<m_CaptureDevices.size(); i++)
	{
		if (!wcscmp(m_CaptureDevices[i]->id, (LPWSTR)Id))
			return i+1;
	}; // for loop

	return 0;
}

bool	CAudioInputW7::RemoveCaptureDevice(PVOID Id)
// Given Capture Device ID this function removes its entry from the vector of capture devices
// Return true/false for success/failure
{
	int i = FindCaptureDevice(Id);
	if(--i<0)
		return false;

	delete m_CaptureDevices[i]->id;
	delete m_CaptureDevices[i]->DeviceName;
	m_CaptureDevices.erase(m_CaptureDevices.begin()+i);
	return true;
}

bool 	CAudioInputW7::ChangeStateCaptureDevice(PVOID Id, DWORD state)
// Given Capture Device ID this function change the state
// Return true/false for success/failure
{
	int i = FindCaptureDevice(Id);
	if(--i<0)
		return false;

	m_CaptureDevices[i]->state = state;
	return true;
}


bool 	CAudioInputW7::AddCaptureDevice(PVOID Id)
// Given Capture Device ID this creates an entry in the capture endpoint vector
// Return true/false for success/failure

{
	// Already exists?
	int i = FindCaptureDevice(Id);
	if(i>0)
		return false;

	IPropertyStore *pProps = NULL;
	IMMDevice *pDevice = NULL;
	PROPVARIANT varName;

	// Get device from ID
	HRESULT hr = S_OK;
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(ADDDEV_IDNOTFOUND,WARN,Id,m_LogParam);
		LogStatus(ADDDEV_IDNOTFOUND,WARN,GetWasapiText(hr),m_LogParam);
		goto bad_exit;
	};

	// Get state of device
	DWORD state;
	hr = pDevice->GetState(&state);
	if (FAILED(hr))
	{
		LogStatus(ADDDEV_STATE,WARN,Id,m_LogParam);
		LogStatus(ADDDEV_STATE,WARN,GetWasapiText(hr),m_LogParam);
		goto bad_exit;
	};

	// Get friendly name of device
	hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
	if (FAILED(hr))
	{
		LogStatus(ADDDEV_PROP,WARN,Id,m_LogParam);
		LogStatus(ADDDEV_PROP,WARN,GetWasapiText(hr),m_LogParam);
		goto bad_exit;
	};

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the Device's friendly-name property.
	hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
	if (FAILED(hr))
	{
		LogStatus(ADDDEV_FRND,WARN,Id,m_LogParam);
		LogStatus(ADDDEV_FRND,WARN,GetWasapiText(hr),m_LogParam);
		goto bad_exit;
	};

	//Verify it is a capture device
	if (!IsCaptureDevice(Id))
		goto bad_exit;


	// Create device entry and insert as last entry
	CapDev *dev = new(CapDev);
	dev->DeviceName = _wcsdup(varName.pwszVal);
	dev->id =  _wcsdup((LPCWSTR)Id);
	dev->state = state;
	m_CaptureDevices.push_back(dev);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pProps)

	return true;



bad_exit:
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pProps)

	return false;
}

HRESULT	CAudioInputW7::RegisterNotification(void)
{
	HRESULT hr = S_OK;
	
	if (m_pNotifyChange || !m_pEnumerator)
	{
		hr = S_FALSE;
		goto Exit;
	}

	// Register endpoint notofication callback & pass pointer to this object as its parent
	m_pNotifyChange = new(CMMNotificationClient);
	hr = m_pEnumerator->RegisterEndpointNotificationCallback(m_pNotifyChange);
	if (FAILED(hr))
	{
		LogStatus(REGNOT_FAIL,ERR,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};
	m_pNotifyChange->GetParent(this);

Exit:
	return hr;
}

/* 
	Set of functions called from the equivalent callback functions in the notofication object
	In general, they just send an appropreate message to the parent window
*/
HRESULT	CAudioInputW7::DefaultDeviceChanged(EDataFlow flow, ERole role,LPCWSTR pwstrDeviceId)
// Called (asynch) when Default device was changed
{
	// Send message to calling window indicating what happend
	if (m_hPrntWnd && role == eMultimedia)
		SendMessage(m_hPrntWnd, WMAPP_DEFDEV_CHANGED, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}
HRESULT	CAudioInputW7::DeviceAdded(LPCWSTR pwstrDeviceId)
// Called (asynch) when device added
{
	// Send message to calling window indicating what happend
	if (m_hPrntWnd)
		PostMessage(m_hPrntWnd, WMAPP_DEV_ADDED, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

HRESULT	CAudioInputW7::DeviceRemoved(LPCWSTR pwstrDeviceId)
// Called (asynch) when device removed
{
	// Send message to calling window indicating what happend
	if (m_hPrntWnd)
		SendMessage(m_hPrntWnd, WMAPP_DEV_REM, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

HRESULT	CAudioInputW7::DeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
// Called (asynch) when device state changed
{
	// Send message to calling window indicating what happend
	if (m_hPrntWnd)
	{
		if (dwNewState & DEVICE_STATE_ACTIVE)
		{
			PostMessage(m_hPrntWnd, WMAPP_DEV_ADDED, (WPARAM)pwstrDeviceId, NULL);
			return S_OK;
		};

		if (dwNewState & DEVICE_STATE_UNPLUGGED)
		{
			SendMessage(m_hPrntWnd, WMAPP_DEV_REM, (WPARAM)pwstrDeviceId, NULL);
			return S_OK;
		};
	};
	return S_FALSE;
}

HRESULT	CAudioInputW7::PropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
// Called (asynch) when device property changed
{
	// Send message to calling window indicating what happend
	if (m_hPrntWnd)
		PostMessage(m_hPrntWnd, WMAPP_DEV_PROPTY, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

/*
	Peak Meters:
	Find the peak value of a given capture endpoint channel
	Find the louder channel of a given capture endpoint
	Find the louder channel of all capture endpoints
*/
float CAudioInputW7::GetChannelPeak(PVOID Id, int iChannel)
// Return peak value for a given channel of an endpoint device
// Endpoint device selected by Id
// Channel index is zero-based
// Return value in the range 0.0-1.0
// Non existing channel returns 0.0
{
	IMMDevice *pDevice = NULL;
	float Value = 0;
	HRESULT hr = S_OK;
	IAudioMeterInformation *pMeterInfo = NULL;
	UINT nChannels = 0;
	float *PeakValue = NULL;
	IAudioClient *pAudioClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	REFERENCE_TIME hnsRequestedDuration = 10000000;

	// Get device from ID
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_IDNOTFOUND,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_IDNOTFOUND,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};


	///// Activate  and initialize a client
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_ACTCLNT,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_ACTCLNT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

    hr = pAudioClient->GetMixFormat(&pwfx);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_MXFRMT,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_MXFRMT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration,0, pwfx, NULL);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_NOINIT,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_NOINIT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};
	///////////////////////////////////////


	// Activate Audio Meter Information interface
	hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_ACTMTR,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_ACTMTR,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Get number of channels, if iChannel invalid then exit
	hr = pMeterInfo->GetMeteringChannelCount(&nChannels);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_MTRCNT,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_MTRCNT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};
	if (iChannel >= (int)nChannels)
		goto Exit;

	// Get peak volume
	PeakValue = new float[nChannels];
	hr = pMeterInfo->GetChannelsPeakValues(nChannels, PeakValue);
	if (FAILED(hr))
	{
		LogStatus(CHPEAK_GETVAL,WARN,Id,m_LogParam);
		LogStatus(CHPEAK_GETVAL,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	Value = PeakValue[iChannel];

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pMeterInfo);
	SAFE_RELEASE(pDevice);
	delete [] PeakValue;
	return Value;
}

double CAudioInputW7::GetDevicePeak(PVOID Id)
// Return peak value for the loader channel of an endpoint device
// Endpoint device selected by Id
// Return value in the range 0.0-1.0
// Non existing endpoint returns 0.0
{
	IMMDevice *pDevice = NULL;
	double Value = -0.01, max = 0;
	HRESULT hr = S_OK;
	IAudioMeterInformation *pMeterInfo = NULL;
	UINT nChannels = 0;
	float *PeakValue = NULL;
	IAudioClient *pAudioClient = NULL;
	WAVEFORMATEX *pwfx = NULL;
	REFERENCE_TIME hnsRequestedDuration = 0;

	//// Stop notifications for this function
	//if (m_pEnumerator && m_pNotifyChange)
	//	m_pEnumerator->UnregisterEndpointNotificationCallback(m_pNotifyChange);

	// Get device from ID
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_IDNOTFOUND,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_IDNOTFOUND,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	///// Activate  and initialize a client ///////////////////////
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_ACTCLNT,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_ACTCLNT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	hr = pAudioClient->GetMixFormat(&pwfx);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_MXFRMT,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_MXFRMT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration,0, pwfx, NULL);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_NOINIT,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_NOINIT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	EXIT_ON_ERROR(hr);

	//////////////////////////////////////////////////////////////

	// Activate Audio Meter Information interface
	hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_ACTMTR,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_ACTMTR,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Get number of channels, if iChannel invalid then exit
	hr = pMeterInfo->GetMeteringChannelCount(&nChannels);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_MTRCNT,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_MTRCNT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// For each channel get peak volume
	Sleep(1000);
	PeakValue = new float[nChannels];
	hr = pMeterInfo->GetChannelsPeakValues(nChannels, PeakValue);
	if (FAILED(hr))
	{
		LogStatus(DEVPEAK_GETVAL,WARN,Id,m_LogParam);
		LogStatus(DEVPEAK_GETVAL,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	for (UINT i=0; i<nChannels; i++)
	{
		if (PeakValue[i]>max)
			max = PeakValue[i];
	};

	Value = max;

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pMeterInfo);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pDevice);
	delete [] PeakValue;
		
	//// Resume notifications
	//m_pEnumerator->RegisterEndpointNotificationCallback(m_pNotifyChange);
	return Value;
}
SPPINTERFACE_API double CAudioInputW7::GetLoudestDevice(PVOID * Id)
// Return peak value for the loadest channel of the loudest endpoint device
// Return value in the range 0.0-1.0
// If successful then function sets Id to correct Device ID else to NULL
{
	double Value = 0, max = 0;
	*Id = NULL;

	if (m_CaptureDevices.empty() || !m_CaptureDevices.size())
		return 0;

	// Disable all devices
	for (UINT i=0; i<m_CaptureDevices.size(); i++)
		EnableAudioDevice((PVOID)m_CaptureDevices[i]->id, false);


	// Get Peak value from all devices
	for (UINT i=0; i<m_CaptureDevices.size(); i++)
	{
		SetDefaultAudioDevice((PVOID)m_CaptureDevices[i]->id);
		//Sleep(1000);
		Value = GetDevicePeak((PVOID)m_CaptureDevices[i]->id);
		EnableAudioDevice((PVOID)m_CaptureDevices[i]->id, false);
		if (max <= Value)
		{
			*Id =(PVOID)m_CaptureDevices[i]->id;
			max =Value;
		};
	};

	Value = max;
	return Value;
}

/*
	Streaming the audio device:
	Starting by ID and stopping

	The actual capture of the audio is done in the audio capture thread - started by
	global function CaptureAudio().
	Function CaptureAudio()loops on the audio data as long as global 
	variable g_CaptureAudioThreadRunnig is 'true'

	Starting capture:
	Before starting the audio capture thread the main thread sets g_CaptureAudioThreadRunnig=true
	Another thing: Creates a global event g_hEventStopCaptureAudioThread
	Capture starts by creating the audio capture thread.

	Stopping capture:
	To stop the audio capture thread the main thread sets g_CaptureAudioThreadRunnig=false
	When the audio capture thread is going out it signals this fact using event g_hEventStopCaptureAudioThread
	Then the main thread waits for audio capture thread to signal that it is exiting.
	The main thread can now release the handle to the audio capture thread and the event then continue execution.

	Capturing:
	Capturing done in capture loop inside audio capture thread (global function CaptureAudio()). The details are in the function itself.
	Capture loop goes while g_CaptureAudioThreadRunnig=true
	When signaled from main thread to exit (g_CaptureAudioThreadRunnig=false)
	it signals the main thread that is exiting by signaling event g_hEventStopCaptureAudioThread;
*/
SPPINTERFACE_API bool CAudioInputW7::StartStreaming(PVOID Id, bool RightChannel)
/*
	Given capture endpoint id this function starts streaming the data
	1. Stop streaming current endpoint
	2. Initialize endpoint
	3. Test usability of endpoint
	4. Start capture stream
	5. Create capturing thread
*/
{
	HRESULT hr = S_OK;
	m_CurrentChannelIsRight = RightChannel;

	// Stop streaming current endpoint
	hr = StopCurrentStream();
	if (FAILED(hr))
	{
		LogStatus(GEN_STATUS,WARN,Id,m_LogParam);
		LogStatus(GEN_STATUS,WARN,STRSTRM1,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Set current default endpoint
	hr = SetDefaultAudioDevice(Id);
	if (FAILED(hr))
	{
		LogStatus(GEN_STATUS,WARN,Id,m_LogParam);
		LogStatus(GEN_STATUS,WARN,STRSTRM2,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Initialize endpoint
	hr = InitEndPoint(Id);
	if (FAILED(hr))
	{
		LogStatus(GEN_STATUS,WARN,Id,m_LogParam);
		LogStatus(GEN_STATUS,WARN,STRSTRM3,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Start capture stream
	hr = StartCurrentStream();
	if (FAILED(hr))
	{
		LogStatus(GEN_STATUS,WARN,Id,m_LogParam);
		LogStatus(GEN_STATUS,WARN,STRSTRM4,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Create capturing thread
	hr = CreateCuptureThread(Id);
	if (FAILED(hr))
	{
		LogStatus(GEN_STATUS,WARN,Id,m_LogParam);
		LogStatus(GEN_STATUS,WARN,STRSTRM5,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

Exit:
	return false;
}

HRESULT CAudioInputW7::InitEndPoint(PVOID Id)
{
	HRESULT hr=S_OK;
	IMMDevice * pDevice = NULL;
	WAVEFORMATEX  *outFormat;
	WAVEFORMATEX *pwfx = NULL;

	// Sanity check
	if (!Id)
	{
		hr = E_FAIL;
		LogStatus(GEN_STATUS,ERR,INITEP1,m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Get device from ID
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	if (FAILED(hr))
	{
		LogStatus(INITEP_IDNOTFOUND,WARN,Id,m_LogParam);
		LogStatus(INITEP_IDNOTFOUND,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	///// Activate a client and get it's audio format
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_pAudioClient);
	if (FAILED(hr))
	{
		LogStatus(INITEP_ACTCLNT,WARN,Id,m_LogParam);
		LogStatus(INITEP_ACTCLNT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	hr = m_pAudioClient->GetMixFormat(&pwfx);
	if (FAILED(hr))
	{
		LogStatus(INITEP_MXFRMT,WARN,Id,m_LogParam);
		LogStatus(INITEP_MXFRMT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Try to improve the current format
	pwfx->wFormatTag = WAVE_FORMAT_PCM;
	pwfx->wBitsPerSample = 8;
	pwfx->nBlockAlign = pwfx->wBitsPerSample / 8 * pwfx->nChannels;
	pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
	pwfx->cbSize = 0;

	hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pwfx, &outFormat);
	if (hr == S_OK)
		outFormat = pwfx;


	// If 8-bit failed, try 16-bit samples
	if (FAILED(hr))
	{
		LogStatus(INITEP_FRMT,WARN,Id,m_LogParam);
		LogStatus(INITEP_FRMT,WARN,GetWasapiText(hr),m_LogParam);
		
		pwfx->wBitsPerSample = 16;
		pwfx->nBlockAlign = pwfx->wBitsPerSample / 8 * pwfx->nChannels;
		pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
		hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pwfx, &outFormat);
		if (hr == S_OK)
			outFormat = pwfx;
		if (FAILED(hr))
		{
			LogStatus(INITEP_FRMT1,ERR,Id,m_LogParam);
			LogStatus(INITEP_FRMT1,ERR,GetWasapiText(hr),m_LogParam);
			EXIT_ON_ERROR(hr);
		};
	};

	// Initialize the endpoint 
	hr = m_pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,			// shared mode
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK,	// stream flags - Event callback
		0,									// buffer duration
		0,									// periodicity (set to buffer duration if AUDCLNT_STREAMFLAGS_EVENTCALLBACK is set)
		outFormat,							// wave format
		NULL);								// session GUID
	if (FAILED(hr))
	{
		LogStatus(INITEP_NOINIT,WARN,Id,m_LogParam);
		LogStatus(INITEP_NOINIT,WARN,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};
	// Note: AUDCLNT_E_DEVICE_IN_USE == 0x8889000a;


	//  sets the event handle that the system signals when an audio buffer is ready to be processed by the client.
	hr = m_pAudioClient->SetEventHandle(g_hAudioBufferReady);
	if (FAILED(hr))
	{
		LogStatus(INITEP_EVTHND,ERR,Id,m_LogParam);
		LogStatus(INITEP_EVTHND,ERR,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

		//  Accesses additional services from the audio client object. We'll need the GetNextPacketSize(), GetBuffer() & ReleaseBuffer
	hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient),(void**)&m_pCaptureClient);
	if (FAILED(hr))
	{
		LogStatus(INITEP_NOCAPT,ERR,Id,m_LogParam);
		LogStatus(INITEP_NOCAPT,ERR,GetWasapiText(hr),m_LogParam);
		EXIT_ON_ERROR(hr);
	};

	// Set default Wave format
	m_CurrentWaveFormat.cbSize = outFormat->cbSize;
	m_CurrentWaveFormat.nAvgBytesPerSec = outFormat->nAvgBytesPerSec;
	m_CurrentWaveFormat.nBlockAlign = outFormat->nBlockAlign;
	m_CurrentWaveFormat.nChannels = outFormat->nChannels;
	m_CurrentWaveFormat.nSamplesPerSec = outFormat->nSamplesPerSec;
	m_CurrentWaveFormat.wBitsPerSample = outFormat->wBitsPerSample;
	m_CurrentWaveFormat.wFormatTag = outFormat->wFormatTag;

Exit:
	CoTaskMemFree(pwfx);
	//delete outFormat;
	SAFE_RELEASE(pDevice);
	return hr;
}


HRESULT CAudioInputW7::StopCurrentStream(void)
{
	HRESULT hr=S_OK;

	// If no valid capture stream
	if (!m_pAudioClient)
		return S_FALSE;

	hr = m_pAudioClient->Stop();  // Stop recording.

	// Stop capture thread
	if (g_hEventStopCaptureAudioThread)
	{
		g_CaptureAudioThreadRunnig = false;
		DWORD dwWaitResult = WaitForSingleObject(g_hEventStopCaptureAudioThread, 2000);
		if (dwWaitResult != WAIT_OBJECT_0)
			TerminateThread(m_hCaptureAudioThread,-1);
		CloseHandle(g_hEventStopCaptureAudioThread);
		g_hEventStopCaptureAudioThread = NULL;
		CloseHandle(m_hCaptureAudioThread);
		if (m_pPulseDataObj)
		{
			delete m_pPulseDataObj;
			m_pPulseDataObj=NULL;
		};

		m_hCaptureAudioThread = NULL;
		m_CurrentWaveFormat.cbSize = 0;
		m_CurrentWaveFormat.nAvgBytesPerSec = 0;
		m_CurrentWaveFormat.nBlockAlign = 0;
		m_CurrentWaveFormat.nChannels = 0;
		m_CurrentWaveFormat.nSamplesPerSec = 0;
		m_CurrentWaveFormat.wBitsPerSample = 0;
		m_CurrentWaveFormat.wFormatTag = 0;
	}

	SAFE_RELEASE(m_pAudioClient);
	if (FAILED(hr))
		LogStatus(STPSTR_NOSTOP,WARN,GetWasapiText(hr),m_LogParam);

	return hr;

}
HRESULT CAudioInputW7::StartCurrentStream(void)
{
	HRESULT hr=S_OK;

	// If no valid capture stream
	if (!m_pAudioClient)
		return S_FALSE;

	hr = m_pAudioClient->Start();  // Start recording.
	if (FAILED(hr))
		LogStatus(STRTSTR_NOSTART,WARN,GetWasapiText(hr),m_LogParam);

	return hr;

}


HRESULT CAudioInputW7::CreateCuptureThread(PVOID Id)
{
	HRESULT hr=S_OK;
	DWORD dwThreadId;

	if (m_hCaptureAudioThread)
		return S_FALSE;
	if (!m_pAudioClient)
		return S_FALSE;

	/* Create a PulseData object & init */
	m_pPulseDataObj = new CPulseData;
	InitPulseDataObj(m_pPulseDataObj);
	m_pPulseDataObj->SelectInputChannel(m_CurrentChannelIsRight);

	/* Pass Process Pulse callback function */
	m_pPulseDataObj->RegisterProcessPulse(ProcessPulse,m_ProcPulseParam);


	/* signal the audio capture thread that it is OK to capture */
	g_CaptureAudioThreadRunnig = true;

	/* Create event by which the audio capture thread signals that it exits */
	g_hEventStopCaptureAudioThread = CreateEvent( 
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("Event Stop Capture Audio-Thread")  // object name TODO: Make it through resorces
        ); 
    if (g_hEventStopCaptureAudioThread == NULL)
		return GetLastError();


	/* Create capturing thread  and pass a pointer to THIS object */
	m_hCaptureAudioThread = CreateThread(
		NULL,				// no security attribute
		0,					// default stack size
		&CaptureAudio,
		this,				// thread parameter
		0,					// not suspended
		&dwThreadId);		// returns thread ID

	if (!m_hCaptureAudioThread)
		return ERROR_ACCESS_DENIED;

	return hr;
}

HRESULT CAudioInputW7::ProcessAudioPacket(CPulseData * pPulseDataObj)
{
	UINT32 packetLength=0, packetLengthNext=0;
	HRESULT hr = S_OK;
	DWORD retval, flags;
	BYTE *pDataIn;
	//UINT PulseDuration;
	//int PulsePolarity;

	// Wait for next buffer event to be signaled.
	retval = WaitForSingleObject(g_hAudioBufferReady, 2000);

	//In case of timeout - continue
	if (retval == WAIT_TIMEOUT)
	{
		return S_FALSE;
	};


	LogAudio(ALOG_GETPCK, m_CurrentWaveFormat.wBitsPerSample, NULL, m_LogAudioParam);
	UINT32 pad=0;
	do { // Loop on buffer until empty - Usually only once

		// Get pointer to next data packet in capture buffer.
		pDataIn = 0;
		flags = 0;
		hr = m_pCaptureClient->GetBuffer(&pDataIn, &packetLength, &flags, NULL, NULL);
		if (FAILED(hr) /*|| flags*/)
		{
			m_pCaptureClient->ReleaseBuffer(packetLength);
			LogStatus(PROCPACK_GETBUF,ERR,GetWasapiText(hr),m_LogParam);
			return hr;
		}; 

		// Detect glitch in data (Unused in Vista)
		if (flags == AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY)
		{
			m_pCaptureClient->ReleaseBuffer(packetLength);
			hr = AUDCLNT_E_BUFFER_ERROR;
			LogStatus(PROCPACK_DISC,WARN,GetWasapiText(hr),m_LogParam);
			return 	hr;
		};


		// Log packets - usually just NO-OP
		LogAudio(ALOG_PACK, packetLength*m_CurrentWaveFormat.nChannels, pDataIn, m_LogAudioParam);

		// Process the audio data - Here the actual work is done
		// ProcessWave converts the audio into pulses
		// The pulses are fed (as they are detected) into the decoder
		// ProcessWave exits when the buffer was read.
		hr = pPulseDataObj->ProcessWave(pDataIn,packetLength); 
		if (FAILED(hr))
		{
			// In case of failure - continue
			LogStatus(PROCPACK_PRW,ERR,GetWasapiText(hr),m_LogParam);
			return hr;
		};

		/* Release the buffer*/
		hr = m_pCaptureClient->ReleaseBuffer(packetLength);
		if (FAILED(hr))
		{
			// In case of failure - continue
			LogStatus(PROCPACK_RLS,ERR,GetWasapiText(hr),m_LogParam);
			return hr;
		};

		// Test how many frames where left - usually none.
		// See: http://social.msdn.microsoft.com/Forums/en-US/windowspro-audiodevelopment/thread/0d4a839d-2c10-49f6-965d-094b692af4f0
		hr = m_pAudioClient->GetCurrentPadding(&pad);
		if (FAILED(hr))
		{
			LogStatus(PROCPACK_PADD,ERR,GetWasapiText(hr),m_LogParam);
			return hr;
		};

	} while (pad);

	return hr;
}

HRESULT CAudioInputW7::InitPulseDataObj(CPulseData * pPulseDataObj)
{
	HRESULT hr = S_OK;


	// Init the Pulse data object
	hr = pPulseDataObj->Initialize(m_CurrentWaveFormat.nSamplesPerSec, m_CurrentWaveFormat.nChannels, m_CurrentWaveFormat.wBitsPerSample);
	if (!FAILED(hr))
		LogStatus(INITPULSE,INFO,INITPLSOBJ,m_LogParam);
	else
		LogStatus(INITPULSE,ERR,GetWasapiText(hr),m_LogParam);
	return hr;
}


SPPINTERFACE_API bool	CAudioInputW7::RegisterLog(LPVOID f, LPVOID param)
{
/* Registration of general Log callback function */
	if (f)
	{
		LogStatus = (LOGFUNC)f;
		m_LogParam = param;
	}
	else
	{
		LogStatus = (LOGFUNC)Log;
		m_LogParam = NULL;
	};
	return false;
}

SPPINTERFACE_API bool	CAudioInputW7::RegisterAudioLog(LPVOID f, LPVOID param)
{
/* Registration of Audio Log callback function */
	if (f)
	{
		LogAudio = (AUDIOLOGFUNC)f;
		m_LogAudioParam = param;
	}
	else
	{
		LogAudio = (AUDIOLOGFUNC)AudioLog;
		m_LogAudioParam = NULL;
	};
	return true;
}

SPPINTERFACE_API bool	CAudioInputW7::RegisterProcessPulse(LPVOID f, LPVOID param)
{
/* Registration of callback function that processes the pulse data */
	if (f)
	{
		ProcessPulse = (PROCPULSEFUNC)f;
		m_ProcPulseParam = param;
	}
	else
	{
		ProcessPulse = (PROCPULSEFUNC)DefProcPulse;
		m_ProcPulseParam = NULL;
	};

	if (m_pPulseDataObj)
		m_pPulseDataObj->RegisterProcessPulse(f,param);

	return true;
}