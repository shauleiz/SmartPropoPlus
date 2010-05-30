#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
//#include <MMReg.h>  //must be before other Wasapi headers
//#include <strsafe.h>
//#include <mmdeviceapi.h>
////#include <Avrt.h>
//#include <audioclient.h>
////#include <Endpointvolume.h>
//
//#include <KsMedia.h>
//#include <functiondiscoverykeys.h>  // PKEY_Device_FriendlyName
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>
#ifndef STANDALONE
#include "audioinput.h"
#endif
#include <devicetopology.h>
#include "smartpropoplus.h"

#ifndef GUID_DEF
#define GUID_DEF

	// //{1CB9AD4C-DBFA-4C32-B178-C2F568A703B2}
	//static GUID const IID_IAudioClient = {
 //    0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2} };

	////{C8ADBD64-E71E-48A0-A4DE-185C395CD317}
	//static GUID const IID_IAudioCaptureClient = {
 //    0xC8ADBD64, 0xE71E, 0x48A0, {0xA4,0xDE,0x18,0x5C,0x39,0x5C,0xD3,0x17} };
#endif

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

class CAudioInputW7 
#ifndef STANDALONE
	: public CAudioInput
#endif

{
protected: 
	class CMixerDevice;
public:
	CAudioInputW7(void);
	virtual ~CAudioInputW7(void);
	int GetCountMixerDevice(void);
	int GetMixerDeviceIndex(const char * mixer);
	int GetDefaultMixerDeviceIndex(void);
	const char * GetMixerDeviceName(int index);
	const char * GetDefaultMixerDeviceName();
	const char * GetDefaultEndpointName();
	const char * GetMixerDeviceInputLineName(int Mixer, int Line);
	bool GetMixerDeviceSelectInputLine(int Mixer, unsigned int * iLine);
	bool SetMixerDeviceSelectInputLine(int Mixer, int Line);
	const char * GetMixerDeviceUniqueName(int iMixer);
	bool GetMixerDeviceInputLineSrcID(int Mixer, unsigned int * SrcID, unsigned int Index);
	bool GetMixerDeviceInputLineIndex(int Mixer, unsigned int SrcID, unsigned int * Index);
	//bool MuteSelectedInputLine(int Mixer, unsigned int line, bool mute=true, bool temporary=false);

protected:
	IMMDeviceEnumerator *m_pEnumerator;
	IMMDeviceCollection * m_pDeviceCollect;
	UINT m_nEndPoints;
	char ** m_ArrayOfEndPointNames;
	UINT m_nMixers;
	CMixerDevice ** m_MixerDevices;

protected:
	int		CreateListDeviceNames(LPWSTR * ListMixerDeviceNames);
	bool	Create(void);

protected: 
	class CMixerDevice
	{
	public:
		virtual ~CMixerDevice();
		CMixerDevice();
		CMixerDevice(LPWSTR Name);
		bool Init(IMMDeviceCollection * pDevCollect);
		char * GetNameA(void);
		const char * GetInputLineName(int Line);
		const char * GetInputLineEPName(int Line);
		int GetInputLineSrcID(int iLine);
		int GetInputLineIndex(unsigned int  SrcID);


	protected:
		int CreateArrayOfInputLines(void);
		LPWSTR GetName(void);
		IPart * FindMuteControl(IPart * pIn, IPart * pPartMute = NULL );
		bool GetMuteStat(IPart * pMute);

	protected:
		struct LineMute {
			IPart * p;
			bool OrigStatus;
			bool CurrentStatus;
		} ;
		struct InputLine { // Represents entry in array of inputs to the audio device(s)
			IPart * p;
			LPWSTR Name;
			char * NameA;
			LPWSTR EndPointName;
			char * EndPointNameA;
			LPWSTR GlobalId;
			LineMute lMute;
		};

	protected:
		LPWSTR m_Name;
		char * m_NameA;
		int m_nInputLines;
		InputLine * m_ArrayInputLines;
	}; // class CMixerDevice
};
