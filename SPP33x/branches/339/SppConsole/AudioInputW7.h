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
public:
	CAudioInputW7(void);
	virtual ~CAudioInputW7(void);
	int GetCountMixerDevice(void);
	int GetMixerDeviceIndex(char * mixer);
	const char * GetMixerDeviceName(int index);


protected:
	IMMDeviceEnumerator *m_pEnumerator;
	IMMDeviceCollection * m_pDeviceCollect;
	UINT m_nEndPoints;
	char ** m_ArrayOfEndPointNames;

protected:
	bool CreateArrayOfEndPointNames(void);

};
