#pragma once

#include "resource.h"


#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { DbgPopUp(__LINE__, hres); goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }



class CMMNotificationClient : public IMMNotificationClient
{
    LONG _cRef;
    IMMDeviceEnumerator *_pEnumerator;
	CAudioInputW7 * _Parent;

public:
	CMMNotificationClient() ;
	virtual ~CMMNotificationClient(void);
	void	STDMETHODCALLTYPE GetParent(CAudioInputW7 * Parent);
	HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role,LPCWSTR pwstrDeviceId);
	HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);
	HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);
	HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
	HRESULT STDMETHODCALLTYPE OnPropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key);
	ULONG STDMETHODCALLTYPE Release();

protected:
	ULONG STDMETHODCALLTYPE AddRef();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface);
};