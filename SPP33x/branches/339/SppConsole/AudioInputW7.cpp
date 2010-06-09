#include "stdafx.h"
#include ".\audioinputw7.h"
#include <wchar.h>
#include "smartpropoplus.h"

	//{A95664D2-9614-4F35-A746-DE8DB63617E6}
	static GUID const CLSID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

	//BCDE0395-E52F-467C-8E3D-C4579291692E
	static GUID const CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };


/***************************************************************************************
	Class CAudioInputW7 (Derived from CAudioInput)

	Object represents the Vista/Windows7 audio system (single object)
	members consist of list of Endpoint devices & index of current Endpoint device
	Endpoint devices are also called Mixer (devices)
***************************************************************************************/
/////////////////// Helper functions //////////////////////////////////////////////////
#ifdef STANDALONE
int w2char(LPWSTR wIn, char * cOut, int size)
{
	int i;
	for (i=0; i<size ; i++)
	{
		cOut[i] = (char)wIn[i];
		if (!cOut[i])
			break;
	};
	return i;
}
#endif

///////////////////////////////////////////////////////////////////////////////////////

CAudioInputW7::CAudioInputW7(void) 
#ifndef STANDALONE
: CAudioInput(false)
#endif
{
	HRESULT hr = S_OK;
	m_nMixers = 0;

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	bool created = Create();
	return;


}


CAudioInputW7::~CAudioInputW7(void)
{
    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pDeviceCollect);
	delete[] m_ArrayOfEndPointNames;
}

bool CAudioInputW7::Create(void)
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pDeviceCollect = NULL;
	m_nEndPoints = 0;
	m_ArrayOfEndPointNames = NULL;
	HRESULT hr = S_OK;

	/* Create a device enumarator then a collection of endpoints and finally get the number of endpoints */
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
	if (FAILED(hr))
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not create audio Endpoint enumerator \r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create a list of  Capture Endpoints
	hr = m_pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &m_pDeviceCollect);
	if (FAILED(hr))
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not enumerate audio Endpoint\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	hr = m_pDeviceCollect->GetCount(&m_nEndPoints);
	if (FAILED(hr))
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not count audio Endpoints\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create an array of Mixer Device Names
	LPWSTR* ListMixerDeviceNames = new LPWSTR[m_nEndPoints+1];
	CreateListDeviceNames(ListMixerDeviceNames);

	// Create array of mixer devices
	m_MixerDevices = new CMixerDevice*[m_nMixers];
	for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
	{
		m_MixerDevices[iMixer] = new CMixerDevice(ListMixerDeviceNames[iMixer]);
		m_MixerDevices[iMixer]->Init(m_pDeviceCollect);
	}

	delete [] ListMixerDeviceNames;

    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pDeviceCollect);
	return true;

	Exit:
    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pDeviceCollect);
	return false;
}

/*
	Create a list of Mixer-Device names
	Return: Number of Mixer-Devices

	Assumption: 
		1. m_nEndPoints is already initialized
		2. ListMixerDeviceNames is already allocated
		3. m_pDeviceCollect is already initialized
*/
int CAudioInputW7::CreateListDeviceNames(LPWSTR * ListMixerDeviceNames)
{
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	HRESULT hr = S_OK;

	for (UINT iEndPoint=0; iEndPoint<m_nEndPoints; iEndPoint++)
	{
		
		ListMixerDeviceNames[iEndPoint]=NULL;

		// Get pointer to endpoint number i.
		hr = m_pDeviceCollect->Item(iEndPoint, &pDevice);
		EXIT_ON_ERROR(hr);

		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the Device's friendly-name property.
		hr = pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);

		// Search for identical entry
		for (UINT i=0; i<=iEndPoint; i++)
		{
			if (!ListMixerDeviceNames[i])
			{
				ListMixerDeviceNames[i] = _wcsdup(varName.pwszVal);
				m_nMixers++;
				break;
			};

			if (!wcscmp(varName.pwszVal, ListMixerDeviceNames[i]))
				break;
		};

	};
	Exit:
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pDevice);

	return m_nMixers;
}

int CAudioInputW7::GetCountMixerDevice()
{
	return m_nMixers;
};

LPCWSTR CAudioInputW7::GetMixerDeviceName(int index)
{
	// Sanity checks
	if ((UINT)index >= m_nMixers || index<0 || m_nMixers<=0 || !m_MixerDevices)
		return GetDefaultMixerDeviceName();

	return m_MixerDevices[index]->GetName();
}

LPCWSTR CAudioInputW7::GetDefaultMixerDeviceName(void)
{
	HRESULT hr = S_OK;
	IMMDevice * pDefaultDevice = NULL;
	IPropertyStore * pProps = NULL;
	PROPVARIANT varName;
	IMMDeviceEnumerator * pEnum = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&pEnum);
	EXIT_ON_ERROR(hr);

	pEnum->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDefaultDevice);
	EXIT_ON_ERROR(hr);

	hr = pDefaultDevice->OpenPropertyStore(STGM_READ, &pProps);
	EXIT_ON_ERROR(hr);

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the Device's friendly-name property.
	hr = pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
	EXIT_ON_ERROR(hr);

	// Device's Name (ASCII)
	//size_t len = wcslen(varName.pwszVal);
	//char * out = (char *)calloc(len+1, sizeof(char));
	//w2char(varName.pwszVal, out, (int)len);

Exit:
	SAFE_RELEASE(pDefaultDevice);
	SAFE_RELEASE(pProps);
	SAFE_RELEASE(pEnum);

	return varName.pwszVal;
}

LPCWSTR CAudioInputW7::GetDefaultEndpointName(void)
{
	HRESULT hr = S_OK;
	IMMDevice * pDefaultDevice = NULL;
	IPropertyStore * pProps = NULL;
	PROPVARIANT varName;
	IMMDeviceEnumerator * pEnum = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&pEnum);
	EXIT_ON_ERROR(hr);

	pEnum->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDefaultDevice);
	EXIT_ON_ERROR(hr);

	hr = pDefaultDevice->OpenPropertyStore(STGM_READ, &pProps);
	EXIT_ON_ERROR(hr);

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the Endpoint's friendly-name property.
	hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
	EXIT_ON_ERROR(hr);

	// Device's Name (ASCII)
	//size_t len = wcslen(varName.pwszVal);
	//char * out = (char *)calloc(len+1, sizeof(char));
	//w2char(varName.pwszVal, out, (int)len);

Exit:
	SAFE_RELEASE(pDefaultDevice);
	SAFE_RELEASE(pProps);
	SAFE_RELEASE(pEnum);

	return varName.pwszVal;
}

LPWSTR CAudioInputW7::GetDefaultEndpointID(void)
{
	HRESULT hr = S_OK;
	IMMDevice * pDefaultDevice = NULL;
	IPropertyStore * pProps = NULL;
	IMMDeviceEnumerator * pEnum = NULL;
	LPWSTR ID = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&pEnum);
	EXIT_ON_ERROR(hr);

	pEnum->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDefaultDevice);
	EXIT_ON_ERROR(hr);

	pDefaultDevice->GetId(&ID);

Exit:
	SAFE_RELEASE(pDefaultDevice);
	SAFE_RELEASE(pProps);
	SAFE_RELEASE(pEnum);

	return ID;
}
int CAudioInputW7::GetMixerDeviceIndex(LPCWSTR mixer)
{
	if (!m_MixerDevices)
		return -1;

	// Special case - Default device

	for (UINT index=0; index<m_nMixers ; index++)
	{
		LPWSTR MixerName = m_MixerDevices[index]->GetName();
		if (!wcscmp(MixerName, mixer))
			return index;
	}

	return -1;
}

int CAudioInputW7::GetDefaultMixerDeviceIndex(void)
{
	LPCWSTR DefaultMixerDeviceName = GetDefaultMixerDeviceName();
	int index = GetMixerDeviceIndex(DefaultMixerDeviceName);
	return index;
}


const char * CAudioInputW7::GetMixerDeviceInputLineName(int iMixer, int iLine)
{
	// Sanity checks
	if ((UINT)iMixer >= m_nMixers || iMixer<0 || m_nMixers<=0 || !m_MixerDevices)
		return NULL;

	CMixerDevice * Mixer = m_MixerDevices[iMixer];
	return Mixer->GetInputLineName(iLine);
}

bool CAudioInputW7::GetMixerDeviceSelectInputLine(int iMixer, unsigned int * iLine)
{
	// Sanity checks
	if ((UINT)iMixer >= m_nMixers || iMixer<0 || m_nMixers<=0 || !m_MixerDevices)
		return false;

	if (GetDefaultMixerDeviceIndex() != iMixer)
		return false;

	LPCWSTR DefaultEndpointName = GetDefaultEndpointName();
	if (!DefaultEndpointName)
		return false;

	CMixerDevice * Mixer = m_MixerDevices[iMixer];
	*iLine = Mixer->GetInputLineIndexByEP(DefaultEndpointName);
	if (iLine>=0)
		return true;
	else
		return false;
}

bool  CAudioInputW7::SetMixerDeviceSelectInputLine(int Mixer, int Line)
{
	return false;
}

LPCWSTR CAudioInputW7::GetMixerDeviceUniqueName(int iMixer)
{
	// Sanity checks
	if ((UINT)iMixer >= m_nMixers || iMixer<0 || m_nMixers<=0 || !m_MixerDevices)
		return GetDefaultEndpointID();

	CMixerDevice * Mixer = m_MixerDevices[iMixer];

	/* Convert Source Line ID into Index */
	UINT SrcID;
	int res = ::GetInputLineSrcId(Mixer->GetName(), &SrcID);
	if (!res)
		return  Mixer->GetInputLineEPID(0);

	UINT iLine;
	GetMixerDeviceInputLineIndex(iMixer, SrcID, &iLine);

	if ((int)iLine <0)
		iLine=0;
	return Mixer->GetInputLineEPID(iLine);

}

bool CAudioInputW7::GetMixerDeviceInputLineSrcID(int iMixer, unsigned int * SrcID, unsigned int iLine)
{
	bool Out=false;
	int id=-1;

	// Sanity checks
	if ((UINT)iMixer >= m_nMixers || iMixer<0 || m_nMixers<=0 || !m_MixerDevices)
		return NULL;

	CMixerDevice * Mixer = m_MixerDevices[iMixer];
	id = Mixer->GetInputLineSrcID(iLine);
	*SrcID = (UINT)id;
	if (id>=0)
		Out=true;
	return Out;
}

bool CAudioInputW7::GetMixerDeviceInputLineIndex(int iMixer, unsigned int SrcID, unsigned int * iLine)
{
	bool Out=false;

	// Sanity checks
	if ((UINT)iMixer >= m_nMixers || iMixer<0 || m_nMixers<=0 || !m_MixerDevices)
		return NULL;

	CMixerDevice * Mixer = m_MixerDevices[iMixer];
	int index = Mixer->GetInputLineIndex(SrcID);
	*iLine = index;
	if (index>=0)
		Out=true;
	return Out;

}

//////////////////// Mixer Device ////////////////////////////////////////////////////////////////////////

// Create an object that represents an Audio Mixer Device
CAudioInputW7::CMixerDevice::CMixerDevice(LPWSTR name)
{
	m_Name = _wcsdup(name);

	size_t len = wcslen(m_Name);
	m_NameA = (char *)calloc(len+1, sizeof(char));
	w2char(m_Name, m_NameA, (int)len); 
}

CAudioInputW7::CMixerDevice::CMixerDevice()
{
	CMixerDevice(NULL);
}

CAudioInputW7::CMixerDevice::~CMixerDevice()
{
}
bool CAudioInputW7::CMixerDevice::Init(IMMDeviceCollection * pDevCollect)
{
	HRESULT hr = S_OK;
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	IDeviceTopology *pDT = NULL;
    IConnector *pConnEndpoint = NULL;
    IConnector *pConnDevice = NULL;
	IPart *pPart = NULL;

	// Sanity check
	if (!pDevCollect)
		goto Exit;

	// Get the number of capture endpoints
	UINT	nCaptureEndPoints=0;
	hr = pDevCollect->GetCount(&nCaptureEndPoints);
	EXIT_ON_ERROR(hr);


	m_ArrayInputLines = new InputLine[nCaptureEndPoints];
	m_nInputLines = 0;

	for (UINT iEndPoint=0; iEndPoint<nCaptureEndPoints; iEndPoint++)
	{
		// Init before loop starts
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pProps);
		SAFE_RELEASE(pDT);
		SAFE_RELEASE(pConnEndpoint);
		SAFE_RELEASE(pConnDevice);

		// Get pointer to endpoint number i.
		hr = pDevCollect->Item(iEndPoint, &pDevice);
		EXIT_ON_ERROR(hr);

		// Get endpoint ID
		hr = pDevice->GetId(&(m_ArrayInputLines[m_nInputLines].EndPointID));
		EXIT_ON_ERROR(hr);

		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the Device's friendly-name property.
		hr = pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);

		// Compare to this device name
		if (wcscmp(varName.pwszVal, GetName()))
			continue;

		///// Identical, current endpoint belongs to this device
		///// FIll-up data in InputLine[m_nInputLines]

		// get device topology object for that endpoint
		hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDT);
		EXIT_ON_ERROR(hr);

		// get the single connector for that endpoint
		hr = pDT->GetConnector(0, &pConnEndpoint);
		EXIT_ON_ERROR(hr);

		// get the connector on the device that is connected to the connector on the endpoint
		hr = pConnEndpoint->GetConnectedTo(&pConnDevice);
		EXIT_ON_ERROR(hr);

		// QI on the device's connector for IPart
		hr = pConnDevice->QueryInterface(__uuidof(IPart), (void**)&pPart);
		EXIT_ON_ERROR(hr);

		////////////// Now, that pPart points to the input pin of the Mixer, we can collect the data we need
		
		// Input pin name - If no name then continue
		LPWSTR PartName;
		size_t len;
		hr = pPart->GetName(&PartName);
		EXIT_ON_ERROR(hr);
		if (!wcslen(PartName))
			continue;
		m_ArrayInputLines[m_nInputLines].Name = PartName;

		// Input pin Name (ASCII)
		len = wcslen(m_ArrayInputLines[m_nInputLines].Name);
		m_ArrayInputLines[m_nInputLines].NameA = (char *)calloc(len+1, sizeof(char));
		w2char(m_ArrayInputLines[m_nInputLines].Name, m_ArrayInputLines[m_nInputLines].NameA, (int)len);

		// Corresponding Endpoint name
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		m_ArrayInputLines[m_nInputLines].EndPointName = _wcsdup(varName.pwszVal);

		// Corresponding Endpoint name (ASCII)
		len = wcslen(m_ArrayInputLines[m_nInputLines].EndPointName);
		m_ArrayInputLines[m_nInputLines].EndPointNameA = (char *)calloc(len+1, sizeof(char));
		w2char(m_ArrayInputLines[m_nInputLines].EndPointName, m_ArrayInputLines[m_nInputLines].EndPointNameA, (int)len);

		// Get Global ID
		hr = pPart->GetGlobalId(&(m_ArrayInputLines[m_nInputLines].GlobalId));
		EXIT_ON_ERROR(hr);

		// Init Mute structure
		m_ArrayInputLines[m_nInputLines].lMute.p = FindMuteControl(pPart);

		// Initialize lmute structure (Mute status)
		bool Muted = false;
		if (m_ArrayInputLines[m_nInputLines].lMute.p)
			Muted = GetMuteStat(m_ArrayInputLines[m_nInputLines].lMute.p);
		m_ArrayInputLines[m_nInputLines].lMute.OrigStatus = m_ArrayInputLines[m_nInputLines].lMute.CurrentStatus = Muted;

		// Keep a pointer to the interface (Just in case ...)
		m_ArrayInputLines[m_nInputLines].p = pPart;

		// Increment number of inputs
		m_nInputLines++;
	};

Exit:
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pProps);
	SAFE_RELEASE(pDT);
	SAFE_RELEASE(pConnEndpoint);
	SAFE_RELEASE(pConnDevice);

	if (FAILED(hr))
		return false;
	else
		return true;
}

IPart * CAudioInputW7::CMixerDevice::FindMuteControl(IPart * pIn, IPart * pPartMute )
{
	HRESULT hr = S_OK;
	IAudioMute *pMute = NULL;
	 IPart * pFoundPartMute = NULL;

	// If mute part not already found - keep on looking for it
	if (!pPartMute)
	{	// see if this is a mute node part
		hr = pIn->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**)&pMute);
		if (E_NOINTERFACE == hr) { /* not a mute node */} 
		else if (FAILED(hr)) {
			EXIT_ON_ERROR(hr);  }   
		else 
			pPartMute = pIn;// it's a mute node...	Save it
	}; // if (!pPartMute)

    // get the list of incoming/outgoing parts
   IPartsList *pConnectedParts = NULL;
   hr = pIn->EnumPartsOutgoing(&pConnectedParts);

   // If reached the end of path - verify that this is a rendering device
   if (E_NOTFOUND == hr) 
   {
		GUID pSubType;
        // not an error... we've just reached the end of the path
		pIn->GetSubType(&pSubType);
		if (IsEqualGUID(KSNODETYPE_SPEAKER, pSubType)
			|| IsEqualGUID(KSNODETYPE_HEADPHONES, pSubType)
			|| IsEqualGUID(KSNODETYPE_DESKTOP_SPEAKER, pSubType))
			return pPartMute;
		else
			return NULL;
    };
   EXIT_ON_ERROR(hr);

   // This part is nither MUTE nor a rendering device - so let's continue
   UINT nParts = 0;
   hr = pConnectedParts->GetCount(&nParts);
   EXIT_ON_ERROR(hr);

   IPart *pConnectedPart = NULL;
   // walk the tree on each incoming part recursively
   for (UINT n = 0; n < nParts; n++) {
	   hr = pConnectedParts->GetPart(n, &pConnectedPart);
	   EXIT_ON_ERROR(hr);

	   pFoundPartMute = FindMuteControl(pConnectedPart, pPartMute);
	   SAFE_RELEASE(pConnectedPart);

	   if (pFoundPartMute)
		   break;

   }

Exit:
    SAFE_RELEASE(pConnectedParts);
    SAFE_RELEASE(pMute);

	return pFoundPartMute;

}

 bool CAudioInputW7::CMixerDevice::GetMuteStat(IPart * pMutePart)
 {
    IAudioMute *pMute = NULL;
	BOOL out = false;

    HRESULT hr = pMutePart->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**)&pMute);
	if (E_NOINTERFACE == hr || FAILED(hr)) 
	{// not a mute node        
	} 
	else 
	{   // it's a mute node...
		pMute->GetMute(&out);
	};

	SAFE_RELEASE(pMute);

	if (out)
		return true;
	else
		return false;
 }

LPWSTR CAudioInputW7::CMixerDevice::GetName(void)
{
	return m_Name;
}
 char * CAudioInputW7::CMixerDevice::GetNameA(void)
 {
	 return m_NameA;
 }
 const char * CAudioInputW7::CMixerDevice::GetInputLineName(int Line)
 {
	 if (!m_nInputLines || Line<0 || Line>=m_nInputLines)
		 return NULL;

	 return m_ArrayInputLines[Line].NameA;
 }

 LPCWSTR CAudioInputW7::CMixerDevice::GetInputLineEPName(int Line)
 {
	 if (!m_nInputLines || Line<0 || Line>=m_nInputLines)
		 return NULL;

	 return m_ArrayInputLines[Line].EndPointName;
 }

 LPWSTR CAudioInputW7::CMixerDevice::GetInputLineEPID(int Line)
 {
	 if (!m_nInputLines || Line<0 || Line>=m_nInputLines)
		 return NULL;

	 return m_ArrayInputLines[Line].EndPointID;
 }


 int CAudioInputW7::CMixerDevice::GetInputLineSrcID(int iLine)
 {
	 // Sanity Check
	 if (m_nInputLines <= iLine || iLine<0)
		 return -1;

	 int out;
	 LPWSTR  StrId = wcsrchr(m_ArrayInputLines[iLine].GlobalId, (int)'/');
	 swscanf(++StrId, L"%d", &out);
	 return out;

 }

 int CAudioInputW7::CMixerDevice::GetInputLineIndex(unsigned int  SrcID)
 {
	 int CurrentSrcId;

	 for (int i=0; i<m_nInputLines; i++)
	 {
		 CurrentSrcId = GetInputLineSrcID(i);
		 if (CurrentSrcId == SrcID)
			 return i;
	 };
	 return -1;
 }

  int CAudioInputW7::CMixerDevice::GetInputLineIndexByEP(LPCWSTR EndpointName)
 {
	 LPCWSTR CurEndpointName;
	 int out = -1;

	 for (int i=0; i<m_nInputLines; i++)
	 {
		 CurEndpointName = GetInputLineEPName(i);
		 if (!wcscmp(CurEndpointName, EndpointName))
		 {
			 out = i;
			 break;
		 };
	 };

	 return out;
 }


//////////////////// Mixer Device  (End)//////////////////////////////////////////////////////////////////