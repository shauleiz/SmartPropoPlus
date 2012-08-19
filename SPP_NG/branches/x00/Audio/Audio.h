#pragma once

#include "resource.h"

// User-defined messages
UINT const WMAPP_DEFDEV_CHANGED =	WM_APP + 100;			// Default Device changed
UINT const WMAPP_DEV_ADDED =		WM_APP + 101;			// Device added
UINT const WMAPP_DEV_REM =			WM_APP + 102;			// Device removed
UINT const WMAPP_DEV_CHANGED =		WM_APP + 103;			// Device changed
UINT const WMAPP_DEV_PROPTY =		WM_APP + 104;			// Device property changed


//{A95664D2-9614-4F35-A746-DE8DB63617E6}
static GUID const CLSID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

//BCDE0395-E52F-467C-8E3D-C4579291692E
static GUID const CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

//typedef  int(CALLBACK *CBF)(void);
struct CapDev {LPWSTR id; LPWSTR DeviceName; DWORD	state;};


int CALLBACK TestEvent(void)
{
	return 0;
}

class CAudioInputW7 
{

protected: 
	bool Create(void);
	int CreateListDeviceNames(LPWSTR * ListMixerDeviceNames);
	int FindCaptureDevice(PVOID Id);
	bool RemoveCaptureDevice(PVOID Id);
	bool AddCaptureDevice(PVOID Id);
	bool ChangeStateCaptureDevice(PVOID Id, DWORD state);

public:
	CAudioInputW7(void);
	CAudioInputW7(HWND hWnd);
	virtual	~CAudioInputW7(void);
	HRESULT	Enumerate(void);
	int		CountCaptureDevices(void);
	HRESULT	GetCaptureDeviceId(int nDevice, int *size, PVOID *Id);
	HRESULT	GetCaptureDeviceName(PVOID Id, LPWSTR * DeviceName);
	bool	IsCaptureDeviceActive(PVOID Id);
	//bool	RegisterChangeNotification(CBF f);

public: // Called asynchronuously when change occurs
	HRESULT DefaultDeviceChanged(EDataFlow flow, ERole role,LPCWSTR pwstrDeviceId);
	HRESULT DeviceAdded(LPCWSTR pwstrDeviceId);
	HRESULT DeviceRemoved(LPCWSTR pwstrDeviceId);
	HRESULT DeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
	HRESULT PropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

protected:
	IMMDeviceEnumerator * m_pEnumerator;
	IMMDeviceCollection * m_pCaptureDeviceCollect;
	IMMDeviceCollection * m_pRenderDeviceCollect;
	class CMMNotificationClient *m_pNotifyChange;
	std::vector<CapDev *>  m_CaptureDevices;

	UINT m_nEndPoints;
	UINT m_nMixers;
	HWND m_hPrntWnd;

public:
	// CBF m_ChangeEventCB;

};

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