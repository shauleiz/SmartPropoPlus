// AudioInputW7.cpp : Defines the class CAudioInputW7 that enumerates all capture endpoints
//

#include "stdafx.h"
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
DWORD WINAPI CaptureAudio(LPVOID param)
/*
	Capture loop - running on a dedicated thread
*/
{
	// Initialize capture thread

	// Capture loop
	do
	{
	}while (g_CaptureAudioThreadRunnig);

	// Exit capture thread
	SetEvent(g_hEventStopCaptureAudioThread);
	return 0;
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
	m_hAudioBufferReady = NULL;
	m_pCaptureClient = NULL;
	m_pAudioClient = NULL;
	m_hCaptureAudioThread = NULL;
	g_hEventStopCaptureAudioThread = NULL;
	g_CaptureAudioThreadRunnig = false;

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
	m_hAudioBufferReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!m_hAudioBufferReady)
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


SPPINTERFACE_API HRESULT	CAudioInputW7::Enumerate(void)
// Fill in structure m_CaptureDevices that specifies the current state of all capture endpoint devices
// Assumption: 
//	1. m_nEndPoints is valid
//	2. m_pCaptureDeviceCollect is already initialized
{
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	HRESULT hr = S_OK;

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

	HRESULT hr = m_pEnumerator->GetDevice((LPWSTR)Id, &pDevice);
	if (FAILED(hr))
		EXIT_ON_ERROR(hr);
	hr = pDevice->QueryInterface(__uuidof(IMMEndpoint), (VOID **)&pEndpoint);
	if (FAILED(hr))
		EXIT_ON_ERROR(hr);
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
	IMMEndpoint * pEndpoint = NULL;
	IMMDevice *pDevice = NULL;
    IConnector *pConnFrom = NULL;
    IConnector *pConnTo = NULL;
    IPart *pPart = NULL;
	ConnectorType Type = Unknown_Connector;
	bool result = false;

	HRESULT hr = m_pEnumerator->GetDevice((LPWSTR)Id, &pDevice);
	EXIT_ON_ERROR(hr);

    // Get the endpoint device's IDeviceTopology interface.
    hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDeviceTopology);
    EXIT_ON_ERROR(hr);	
		
    // The device topology for an endpoint device always
    // contains just one connector (connector number 0).
    hr = pDeviceTopology->GetConnector(0, &pConnFrom);
    EXIT_ON_ERROR(hr);

    // Step across the connection to the jack on the adapter.
    hr = pConnFrom->GetConnectedTo(&pConnTo);
    if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr)
    {
        // The adapter device is not currently active.
        hr = E_NOINTERFACE;
    }
    EXIT_ON_ERROR(hr);

	hr = pConnTo->GetType(&Type);
    EXIT_ON_ERROR(hr);

	if (Type == Physical_External)
		result = true;

Exit:
	SAFE_RELEASE(pEndpoint);
	SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pDeviceTopology)
    SAFE_RELEASE(pConnFrom)
    SAFE_RELEASE(pConnTo)
    SAFE_RELEASE(pPart)
	return result;
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
		goto bad_exit;

	// Get state of device
	DWORD state;
	hr = pDevice->GetState(&state);
	if (FAILED(hr))
		goto bad_exit;

	// Get friendly name of device
	hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
	if (FAILED(hr))
		goto bad_exit;

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the Device's friendly-name property.
	hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
	if (FAILED(hr))
		goto bad_exit;

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
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not register notification callback\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
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
	if (m_hPrntWnd)
		PostMessage(m_hPrntWnd, WMAPP_DEFDEV_CHANGED, (WPARAM)pwstrDeviceId, NULL);
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
		PostMessage(m_hPrntWnd, WMAPP_DEV_REM, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

HRESULT	CAudioInputW7::DeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
// Called (asynch) when device state changed
{
	// Send message to calling window indicating what happend
	if (m_hPrntWnd)
		PostMessage(m_hPrntWnd, WMAPP_DEV_CHANGED, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
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
	EXIT_ON_ERROR(hr);

	///// Activate  and initialize a client
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    EXIT_ON_ERROR(hr);

    hr = pAudioClient->GetMixFormat(&pwfx);
    EXIT_ON_ERROR(hr)

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration,0, pwfx, NULL);
    EXIT_ON_ERROR(hr)
	///////////////////////////////////////


	// Activate Audio Meter Information interface
	hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
	EXIT_ON_ERROR(hr);

	// Get number of channels, if iChannel invalid then exit
	hr = pMeterInfo->GetMeteringChannelCount(&nChannels);
	EXIT_ON_ERROR(hr);
	if (iChannel >= (int)nChannels)
		goto Exit;

	// Get peak volume
	PeakValue = new float[nChannels];
	hr = pMeterInfo->GetChannelsPeakValues(nChannels, PeakValue);
	EXIT_ON_ERROR(hr);

	Value = PeakValue[iChannel];

Exit:
	CoTaskMemFree(pwfx);
	SAFE_RELEASE(pAudioClient);
	SAFE_RELEASE(pMeterInfo);
	SAFE_RELEASE(pDevice);
	delete [] PeakValue;
	return Value;
}

SPPINTERFACE_API double CAudioInputW7::GetDevicePeak(PVOID Id)
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
	Value -= 0.01;
	EXIT_ON_ERROR(hr);

	///// Activate  and initialize a client ///////////////////////
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
	Value -= 0.01;
    EXIT_ON_ERROR(hr);

	hr = pAudioClient->GetMixFormat(&pwfx);
	Value -= 0.01;
	EXIT_ON_ERROR(hr);
	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration,0, pwfx, NULL);
	if (FAILED(hr)){
		if (hr == AUDCLNT_E_DEVICE_IN_USE)
			Value = -0.55;
		else
			Value = -0.75;
	};

	EXIT_ON_ERROR(hr);

	//////////////////////////////////////////////////////////////

	// Activate Audio Meter Information interface
	hr = pDevice->Activate(__uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, (void**)&pMeterInfo);
 	Value -= 0.01;
	EXIT_ON_ERROR(hr);

	// Get number of channels, if iChannel invalid then exit
	hr = pMeterInfo->GetMeteringChannelCount(&nChannels);
 	Value -= 0.01;
	EXIT_ON_ERROR(hr);

	// For each channel get peak volume
	PeakValue = new float[nChannels];
	hr = pMeterInfo->GetChannelsPeakValues(nChannels, PeakValue);
 	Value -= 0.01;
	EXIT_ON_ERROR(hr);
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

	for (UINT i=0; i<m_CaptureDevices.size(); i++)
	{
		Value = GetDevicePeak((PVOID)m_CaptureDevices[i]->id);
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
SPPINTERFACE_API bool CAudioInputW7::StartStreaming(PVOID Id)
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

	// Stop streaming current endpoint
	hr = StopCurrentStream();

	// Set current default endpoint
	hr = SetDefaultAudioDevice(Id);

	// Initialize endpoint
	hr = InitEndPoint(Id);

	// Start capture stream
	hr = StartCurrentStream();

	// Create capturing thread
	hr = CreateCuptureThread(Id);

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
		EXIT_ON_ERROR(hr);
	};

	// Get device from ID
	hr = m_pEnumerator->GetDevice((LPCWSTR)Id, &pDevice);
	EXIT_ON_ERROR(hr);

	///// Activate a client and get it's audio format
	hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_pAudioClient);
	EXIT_ON_ERROR(hr);

	hr = m_pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr);

	// Try to improve the current format
	pwfx->nSamplesPerSec = 192000;
	pwfx->nBlockAlign = pwfx->wBitsPerSample / 8 * pwfx->nChannels;
	pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
	outFormat = new(WAVEFORMATEX);
	hr = m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pwfx, &outFormat);
	if (hr == S_OK)
		outFormat = pwfx;
    EXIT_ON_ERROR(hr);

	// Initialize the endpoint 
	hr = m_pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,			// shared mode
		AUDCLNT_STREAMFLAGS_EVENTCALLBACK,	// stream flags - Event callback
		0,									// buffer duration
		0,									// periodicity (set to buffer duration if AUDCLNT_STREAMFLAGS_EVENTCALLBACK is set)
		outFormat,							// wave format
		NULL);								// session GUID
	EXIT_ON_ERROR(hr);
	//AUDCLNT_E_DEVICE_IN_USE == 0x8889000a;


	//  sets the event handle that the system signals when an audio buffer is ready to be processed by the client.
	hr = m_pAudioClient->SetEventHandle(m_hAudioBufferReady);
	EXIT_ON_ERROR(hr);

		//  Accesses additional services from the audio client object. We'll need the GetNextPacketSize(), GetBuffer() & ReleaseBuffer
	hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient),(void**)&m_pCaptureClient);
	EXIT_ON_ERROR(hr);


Exit:
	CoTaskMemFree(pwfx);
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
	SAFE_RELEASE(m_pAudioClient);

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
	}

	return hr;

}
HRESULT CAudioInputW7::StartCurrentStream(void)
{
	HRESULT hr=S_OK;

	// If no valid capture stream
	if (!m_pAudioClient)
		return S_FALSE;

	hr = m_pAudioClient->Start();  // Start recording.
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

	/* Create capturing thread  */
	m_hCaptureAudioThread = CreateThread(
		NULL,				// no security attribute
		0,					// default stack size
		&CaptureAudio,
		Id,				// thread parameter
		0,					// not suspended
		&dwThreadId);		// returns thread ID

	if (!m_hCaptureAudioThread)
		return ERROR_ACCESS_DENIED;

	return hr;
}


HRESULT CAudioInputW7::SetDefaultAudioDevice(PVOID devID)
{	
	IPolicyConfigVista *pPolicyConfig;
	ERole reserved = eConsole;

    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
		NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
	if (SUCCEEDED(hr))
	{
		hr = pPolicyConfig->SetDefaultEndpoint((LPCWSTR)devID, reserved);
		pPolicyConfig->Release();
	}
	return hr;
}
