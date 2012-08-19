// AudioInputW7.cpp : Defines the class CAudioInputW7 that enumerates all capture endpoints
//

#include "stdafx.h"
#include <vector>
#include <devicetopology.h>
#include <Mmdeviceapi.h>
#include "WinMessages.h"
#include "AudioInputW7.h"
#include "NotificationClient.h"
#include <Functiondiscoverykeys_devpkey.h>

#define MAX_LOADSTRING 100

//{A95664D2-9614-4F35-A746-DE8DB63617E6}
static GUID const CLSID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

//BCDE0395-E52F-467C-8E3D-C4579291692E
static GUID const CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };


////////////////////// Class CAudioInputW7 //////////////////////
SPPINTERFACE_API CAudioInputW7::CAudioInputW7(HWND hWnd)
{
	m_hPrntWnd = hWnd;
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
	m_hPrntWnd = hWnd;

	bool created = Create();
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
	return;
}

bool CAudioInputW7::Create(void)
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pNotifyChange = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
	m_nEndPoints = 0;
	// m_ArrayOfEndPointNames = NULL;
	HRESULT hr = S_OK;

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

	hr = m_pCaptureDeviceCollect->GetCount(&m_nEndPoints);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not count audio Endpoints\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};



	// Register endpoint notofication callback & pass pointer to this object as its parent
	m_pNotifyChange = new(CMMNotificationClient);
	hr = m_pEnumerator->RegisterEndpointNotificationCallback(m_pNotifyChange);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not register notification callback\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};
	m_pNotifyChange->GetParent(this);



	// First Enumeration
	hr = Enumerate();

	// TEST
	PVOID id;
	int size;
	LPWSTR DevName;
	bool active;

	for (int i=0; i<15; i++)
	{
		hr = GetCaptureDeviceId(i, &size, &id);
		if (FAILED(hr))continue;
		hr = GetCaptureDeviceName(id, &DevName);
		active = IsCaptureDeviceActive(id);
	}

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
	//Restore();
	//delete[] m_ArrayOfEndPointNames;

	//for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
	//	delete m_MixerDevices[iMixer];
	//delete[] m_MixerDevices;
}



int CAudioInputW7::CreateListDeviceNames(LPWSTR * ListMixerDeviceNames)
/*
Create a list of Mixer-Device names
Return: Number of Mixer-Devices

Assumption: 
1. m_nEndPoints is already initialized
2. ListMixerDeviceNames is already allocated
3. m_pCaptureDeviceCollect is already initialized
*/
{
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	HRESULT hr = S_OK;

	for (UINT iEndPoint=0; iEndPoint<m_nEndPoints; iEndPoint++)
	{

		ListMixerDeviceNames[iEndPoint]=NULL;

		// Get pointer to endpoint number i.
		hr = m_pCaptureDeviceCollect->Item(iEndPoint, &pDevice);
		EXIT_ON_ERROR(hr);

		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the Device's friendly-name property.
		//hr = pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);

		ListMixerDeviceNames[iEndPoint] = _wcsdup(varName.pwszVal);
		m_nMixers++;

		// Get the state of the device:
		// DEVICE_STATE_ACTIVE / DEVICE_STATE_DISABLED / DEVICE_STATE_NOTPRESENT / DEVICE_STATE_UNPLUGGED
		DWORD state;
		hr = pDevice->GetState(&state);
		EXIT_ON_ERROR(hr);

		LPWSTR id;
		hr = pDevice->GetId(&id);
		EXIT_ON_ERROR(hr);
		CoTaskMemFree(id);




		// Search for identical entry
		//for (UINT i=0; i<=iEndPoint; i++)
		//{
		//	if (!ListMixerDeviceNames[i])
		//	{
		//		ListMixerDeviceNames[i] = _wcsdup(varName.pwszVal);
		//		m_nMixers++;
		//		break;
		//	};

		//	if (!wcscmp(varName.pwszVal, ListMixerDeviceNames[i]))
		//		break;
		//};

	};

Exit:
	SAFE_RELEASE(pProps);
	SAFE_RELEASE(pDevice);

	return m_nMixers;
}



SPPINTERFACE_API HRESULT	CAudioInputW7::Enumerate(void)
// Fill in structure m_CaptureDevices that specifies the current state of all capture endpoint devices
// Assumption: 
//	1. m_nEndPoints is valid
//	2. m_pCaptureDeviceCollect is already initialized
// Call this function only once during initialization - updates will be done by callback routines
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


		m_CaptureDevices.push_back(new CapDev);
		m_CaptureDevices.back()->DeviceName = _wcsdup(varName.pwszVal);
		m_CaptureDevices.back()->state = state;
		m_CaptureDevices.back()->id = _wcsdup(id);

		CoTaskMemFree(id);
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
	for (UINT i = 0; i<m_CaptureDevices.size(); i++)
	{
		if (!wcscmp(m_CaptureDevices[i]->id, (LPWSTR)Id))
		{
			return m_CaptureDevices[i]->state & DEVICE_STATE_ACTIVE;
		}
	};

	return false;

}

//bool CAudioInputW7::RegisterChangeNotification(CBF func)
//// Register callback function to be called everytime there's a change in one or more input devices
//// This function must be simple and used for notification.
//// It is recommended to run Enumerate after every call to this function
//{
//	if (!func || !m_pNotifyChange)
//		return false;
//
//	m_ChangeEventCB = func;
//	m_pNotifyChange->GetParent(this);
//	return true;
//}

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
// Given Capture Device ID this function removes its entry
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


HRESULT	CAudioInputW7::DefaultDeviceChanged(EDataFlow flow, ERole role,LPCWSTR pwstrDeviceId)
// Called (asynch) when Default device was changed
{
	if (flow != eCapture)
		return S_OK;
	// Send message to calling window indicating what happend
	SendMessage(m_hPrntWnd, WMAPP_DEFDEV_CHANGED, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}
HRESULT	CAudioInputW7::DeviceAdded(LPCWSTR pwstrDeviceId)
// Called (asynch) when device added
{
	// Send message to calling window indicating what happend
	SendMessage(m_hPrntWnd, WMAPP_DEV_ADDED, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

HRESULT	CAudioInputW7::DeviceRemoved(LPCWSTR pwstrDeviceId)
// Called (asynch) when device removed
{
	// Send message to calling window indicating what happend
	SendMessage(m_hPrntWnd, WMAPP_DEV_REM, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

HRESULT	CAudioInputW7::DeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
// Called (asynch) when device state changed
{
	// Send message to calling window indicating what happend
	SendMessage(m_hPrntWnd, WMAPP_DEV_CHANGED, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

HRESULT	CAudioInputW7::PropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
// Called (asynch) when device property changed
{
	// Send message to calling window indicating what happend
	SendMessage(m_hPrntWnd, WMAPP_DEV_PROPTY, (WPARAM)pwstrDeviceId, NULL);
	return S_OK;
}

