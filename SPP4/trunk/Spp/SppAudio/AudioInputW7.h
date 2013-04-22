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
#include <devicetopology.h>
#include <Mmdeviceapi.h>
#include <Audioclient.h>
#include <vector>


#define EXIT_ON_ERROR(hres) \
              if (FAILED(hres)) { /*DbgPopUp(__LINE__, hres);*/ goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }


struct CapDev {LPWSTR id; LPWSTR DeviceName; DWORD	state;};
class CPulseData;
typedef void (* LOGFUNC)(int Code, int Severity, LPVOID Data, LPVOID Param);
typedef void (* AUDIOLOGFUNC)(int Code,  int size, LPVOID Data, LPVOID Param);
typedef void (* PROCPULSEFUNC)(int length, bool low, LPVOID Param);



/////// Log related definitions ///////
// Severity
#define	INFO	0
#define	WARN	1
#define	ERR		2
#define	FATAL	3

// General Log Message Codes
#define	GEN_STATUS			100
#define	ENUM_FRND			101
#define	ENUM_UID			102
#define	CHANGE_DEFDEV		103
#define	ISCAP_IDNOTFOUND	104
#define	ISCAP_EPNOTFOUND	105
#define	ISEXT_IDNOTFOUND	106
#define	ISEXT_TOPO			107
#define	ISEXT_NOCONN		108
#define	ISEXT_NOTYPE		109
#define	ADDDEV_IDNOTFOUND	110
#define	ADDDEV_STATE		111
#define	ADDDEV_PROP			112
#define	ADDDEV_FRND			113
#define	REGNOT_FAIL			114
#define	CHPEAK_IDNOTFOUND	115
#define	CHPEAK_ACTCLNT		116
#define	CHPEAK_MXFRMT		117
#define	CHPEAK_NOINIT		118
#define	CHPEAK_ACTMTR		119
#define	CHPEAK_MTRCNT		120
#define	CHPEAK_GETVAL		121
#define	DEVPEAK_IDNOTFOUND	122
#define	DEVPEAK_ACTCLNT		123
#define	DEVPEAK_MXFRMT		124
#define	DEVPEAK_NOINIT		125
#define	DEVPEAK_ACTMTR		126
#define	DEVPEAK_MTRCNT		127
#define	DEVPEAK_GETVAL		128
#define	INITEP_IDNOTFOUND	129
#define	INITEP_ACTCLNT		130
#define	INITEP_MXFRMT		131
#define	INITEP_FRMT			132
#define	INITEP_NOINIT		133
#define	INITEP_EVTHND		134
#define	INITEP_NOCAPT		135
#define	STPSTR_NOSTOP		136
#define	STRTSTR_NOSTART		137
#define	INITEP_FRMT1		138
#define	PROCPACK_GETBUF		139
#define	PROCPACK_DISC		140
#define	PROCPACK_PRW		141
#define	PROCPACK_RLS		142
#define	PROCPACK_PADD		143
#define	INITPULSE			144
#define	CHANGE_ENDEV		145

// Audio Log message Codes
#define	ALOG_GETPCK			200
#define	ALOG_PACK			201


// Message Text
#define	ENUM1		(LPVOID)L"Enumerating devices"
#define	STRSTRM1	(LPVOID)L"StartStreaming(): Could not stop current stream"
#define	STRSTRM2	(LPVOID)L"StartStreaming(): Could not set default audio device"
#define	STRSTRM3	(LPVOID)L"StartStreaming(): Could not initialize endpoint"
#define	STRSTRM4	(LPVOID)L"StartStreaming(): Could not start current stream"
#define	STRSTRM5	(LPVOID)L"StartStreaming(): Could not create capture thread"
#define	INITEP1		(LPVOID)L"InitEndPoint(): Function called with ID=NULL "
#define	INITPLSOBJ	(LPVOID)L"Initializing CPulseData Object"



class CAudioInputW7 
{
protected: 
	bool Create(void);
	int FindCaptureDevice(PVOID Id);
	bool RemoveCaptureDevice(PVOID Id);
	bool AddCaptureDevice(PVOID Id);
	bool ChangeStateCaptureDevice(PVOID Id, DWORD state);
	float GetChannelPeak(PVOID Id, int iChannel);
	HRESULT	RegisterNotification(void);
	HRESULT StopCurrentStream(void);
	HRESULT StartCurrentStream(void);
	HRESULT InitEndPoint(PVOID Id);
	HRESULT CreateCuptureThread(PVOID Id);
	HRESULT SetDefaultAudioDevice(PVOID Id);
	HRESULT EnableAudioDevice(PVOID devID, bool Enable=true);
	LOGFUNC	LogStatus;
	AUDIOLOGFUNC	LogAudio;
	PROCPULSEFUNC	ProcessPulse;


public:
	CAudioInputW7(void);
	SPPINTERFACE_API CAudioInputW7(HWND hWnd);
	virtual	~CAudioInputW7(void);
	SPPINTERFACE_API HRESULT	Enumerate(void);
	SPPINTERFACE_API double	GetDevicePeak(PVOID Id);
	HRESULT InitPulseDataObj(CPulseData * pPulseDataObj);
	SPPINTERFACE_API int		CountCaptureDevices(void);
	SPPINTERFACE_API int		GetNumberChannels(PVOID Id);
	SPPINTERFACE_API HRESULT	GetCaptureDeviceId(int nDevice, int *size, PVOID *Id);
	SPPINTERFACE_API HRESULT	GetCaptureDeviceName(PVOID Id, LPWSTR * DeviceName);
	SPPINTERFACE_API bool		IsCaptureDeviceActive(PVOID Id);
	SPPINTERFACE_API bool		IsCaptureDeviceDefault(PVOID Id);
	SPPINTERFACE_API bool		IsCaptureDevice(PVOID Id);
	SPPINTERFACE_API bool		IsExternal(PVOID Id);
	SPPINTERFACE_API double		GetLoudestDevice(PVOID * Id);
	SPPINTERFACE_API bool		StartStreaming(PVOID Id, bool RightChannel=false);
	SPPINTERFACE_API HRESULT	ProcessAudioPacket(CPulseData * pPulseDataObj);
	SPPINTERFACE_API HRESULT	GetJackInfo(PVOID Id, KSJACK_DESCRIPTION *pJackDescData);
	SPPINTERFACE_API COLORREF	GetJackColor(PVOID Id);
	SPPINTERFACE_API bool		IsDisconnected(PVOID Id);

	SPPINTERFACE_API bool		RegisterLog(LPVOID,LPVOID=NULL);
	SPPINTERFACE_API bool		RegisterAudioLog(LPVOID,LPVOID=NULL);
	SPPINTERFACE_API bool		RegisterProcessPulse(LPVOID,LPVOID=NULL);
	SPPINTERFACE_API HRESULT	GetAudioPacket(PBYTE pBuffer, PUINT pBufLength, UINT bMax);



	//bool	RegisterChangeNotification(CBF f);

public: // Called asynchronuously when change occurs
	HRESULT DefaultDeviceChanged(EDataFlow flow, ERole role,LPCWSTR pwstrDeviceId);
	HRESULT DeviceAdded(LPCWSTR pwstrDeviceId);
	HRESULT DeviceRemoved(LPCWSTR pwstrDeviceId);
	HRESULT DeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState);
	HRESULT PropertyValueChanged( LPCWSTR pwstrDeviceId, const PROPERTYKEY key);


protected:
	IAudioCaptureClient * m_pCaptureClient;
	IMMDeviceEnumerator * m_pEnumerator;
	IMMDeviceCollection * m_pCaptureDeviceCollect;
	IMMDeviceCollection * m_pRenderDeviceCollect;
	IAudioClient		* m_pAudioClient;
	class CMMNotificationClient *m_pNotifyChange;
	std::vector<CapDev *>  m_CaptureDevices;

	UINT			m_nEndPoints;
	UINT			m_nMixers;
	HWND			m_hPrntWnd;
	HANDLE			m_hAudioBufferReady;
	HANDLE			m_hCaptureAudioThread;
	WAVEFORMATEX	m_CurrentWaveFormat;
	bool			m_CurrentChannelIsRight;
	PVOID			m_CurrentId;
	PVOID			m_LogAudioParam;
	PVOID			m_LogParam;
	PVOID			m_ProcPulseParam;

public:
	CPulseData			* m_pPulseDataObj;

};


