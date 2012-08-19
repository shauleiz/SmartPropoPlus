// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SPPINTERFACE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SPPINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SPPINTERFACE_EXPORTS
#define SPPINTERFACE_API __declspec(dllexport)
#else
#define SPPINTERFACE_API __declspec(dllimport)
#endif

#pragma once

#include "resource.h"

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

struct CapDev {LPWSTR id; LPWSTR DeviceName; DWORD	state;};


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
	SPPINTERFACE_API CAudioInputW7(HWND hWnd);
	virtual	~CAudioInputW7(void);
	SPPINTERFACE_API HRESULT	Enumerate(void);
	SPPINTERFACE_API int		CountCaptureDevices(void);
	SPPINTERFACE_API HRESULT	GetCaptureDeviceId(int nDevice, int *size, PVOID *Id);
	SPPINTERFACE_API HRESULT	GetCaptureDeviceName(PVOID Id, LPWSTR * DeviceName);
	SPPINTERFACE_API bool		IsCaptureDeviceActive(PVOID Id);
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

