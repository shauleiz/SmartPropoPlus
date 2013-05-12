#pragma once

#include "resource.h"

#ifndef EXIT_ON_ERROR
#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { /*DbgPopUp(__LINE__, hres);*/ goto Exit; }
#endif
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }



class CMMNotificationClient : public IMMNotificationClient
{
    LONG _cRef;
    IMMDeviceEnumerator *_pEnumerator;
	CSppAudio * _Parent;

public:
	CMMNotificationClient() ;
	virtual ~CMMNotificationClient(void);
	void	STDMETHODCALLTYPE GetParent(CSppAudio * Parent);
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