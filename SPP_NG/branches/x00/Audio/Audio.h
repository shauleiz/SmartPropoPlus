#pragma once

#include "resource.h"

//{A95664D2-9614-4F35-A746-DE8DB63617E6}
static GUID const CLSID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

//BCDE0395-E52F-467C-8E3D-C4579291692E
static GUID const CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

typedef  int(CALLBACK *CBF)(void);

int CALLBACK TestEvent(void)
{
	return 0;
}

class CAudioInputW7 
{

protected: 
	bool Create(void);
	int CreateListDeviceNames(LPWSTR * ListMixerDeviceNames);


public:
	CAudioInputW7(void);
	virtual	~CAudioInputW7(void);
	HRESULT	Enumerate(void);
	int		CountCaptureDevices(void);
	HRESULT	GetCaptureDeviceId(int nDevice, int *size, PVOID *Id);
	HRESULT	GetCaptureDeviceName(PVOID Id, LPWSTR * DeviceName);
	bool	IsCaptureDeviceActive(PVOID Id);
	bool	RegisterChangeNotification(CBF f);

protected:
	IMMDeviceEnumerator * m_pEnumerator;
	IMMDeviceCollection * m_pCaptureDeviceCollect;
	IMMDeviceCollection * m_pRenderDeviceCollect;
	class CMMNotificationClient *m_pNotifyChange;
	struct CapDev {
		int		nDev;
		std::vector<LPWSTR> id;
		std::vector<LPWSTR> DeviceName;
		std::vector<DWORD>	state;
	} m_CaptureDevices;

	UINT m_nEndPoints;
	UINT m_nMixers;

public:
	CBF m_ChangeEventCB;
};

class CMMNotificationClient : public IMMNotificationClient
{
    LONG _cRef;
    IMMDeviceEnumerator *_pEnumerator;
	CAudioInputW7 * _Parent;

    // Private function to print device-friendly name
    HRESULT _PrintDeviceName(LPCWSTR  pwstrId);

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