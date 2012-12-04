// NotificationClient.cpp : Defines class CMMNotificationClient
//

#include "stdafx.h"
#include <vector>
#include <devicetopology.h>
#include <Mmdeviceapi.h>
#include "WinMessages.h"
#include <Audioclient.h>
#include "AudioInputW7.h"
#include "NotificationClient.h"
#include <Functiondiscoverykeys_devpkey.h>

//--------------------------------------------------------------------------------------------------
//-----------------------------------------------------------
// Example implementation of IMMNotificationClient interface.
// When the status of audio endpoint devices change, the
// MMDevice module calls these methods to notify the client.
// Based on  http://msdn.microsoft.com/en-us/library/windows/desktop/dd370810(v=vs.85).aspx
//-----------------------------------------------------------

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }


////////////////////// Class CMMNotificationClient //////////////////////
CMMNotificationClient::CMMNotificationClient()
{
	_cRef=1;
	_pEnumerator=NULL;
}

CMMNotificationClient::~CMMNotificationClient()
{
	SAFE_RELEASE(_pEnumerator)
}

void STDMETHODCALLTYPE CMMNotificationClient::GetParent(CAudioInputW7 * Parent)
{
	_Parent = Parent;
}

// IUnknown methods -- AddRef, Release, and QueryInterface

ULONG STDMETHODCALLTYPE CMMNotificationClient::AddRef()
{
	return InterlockedIncrement(&_cRef);
}

ULONG STDMETHODCALLTYPE CMMNotificationClient::Release()
{
	ULONG ulRef = InterlockedDecrement(&_cRef);
	if (0 == ulRef)
	{
		delete this;
	}
	return ulRef;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::QueryInterface(REFIID riid, VOID **ppvInterface)
{
	if (IID_IUnknown == riid)
	{
		AddRef();
		*ppvInterface = (IUnknown*)this;
	}
	else if (__uuidof(IMMNotificationClient) == riid)
	{
		AddRef();
		*ppvInterface = (IMMNotificationClient*)this;
	}
	else
	{
		*ppvInterface = NULL;
		return E_NOINTERFACE;
	}
	return S_OK;
}

// Callback methods for device-event notifications.

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDefaultDeviceChanged(EDataFlow flow, ERole role,	LPCWSTR pwstrDeviceId)
{
	// Notify caller of the change
	_Parent->DefaultDeviceChanged( flow,  role,	 pwstrDeviceId);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{


	// Notify caller of the change
	_Parent->DeviceAdded(pwstrDeviceId);
	return S_OK;
};

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
	// Notify caller of the change
	_Parent->DeviceRemoved(pwstrDeviceId);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
	// Notify caller of the change
	_Parent->DeviceStateChanged(pwstrDeviceId, dwNewState);
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
	// Notify caller of the change
	_Parent->PropertyValueChanged(pwstrDeviceId, key);
	return S_OK;
}

//--------------------------------------------------------------------------------------------------