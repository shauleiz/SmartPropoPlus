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
    SAFE_RELEASE(m_pCaptureDeviceCollect);
	Restore();
	delete[] m_ArrayOfEndPointNames;

	for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
		delete m_MixerDevices[iMixer];
	delete[] m_MixerDevices;
}

bool CAudioInputW7::Create(void)
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
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

	// Create a list of ACTIVE Capture Endpoints
	hr = m_pEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE, &m_pCaptureDeviceCollect);
	if (FAILED(hr))
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not enumerate capture audio Endpoint\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create a list of ACTIVE Render Endpoints
	hr = m_pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &m_pRenderDeviceCollect);
	EXIT_ON_ERROR(hr);


	hr = m_pCaptureDeviceCollect->GetCount(&m_nEndPoints);
	if (FAILED(hr))
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not count audio Endpoints\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create an array of Mixer Device Names
	LPWSTR* ListMixerDeviceNames = new LPWSTR[m_nEndPoints+1];
	int nMixerDev = CreateListDeviceNames(ListMixerDeviceNames);
	if (!nMixerDev)
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not create a list of Mixer Device Names\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		goto Exit;
	};

	// Create array of mixer devices
	m_MixerDevices = new CMixerDevice*[m_nMixers];
	for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
	{
		m_MixerDevices[iMixer] = new CMixerDevice(ListMixerDeviceNames[iMixer]);
		m_MixerDevices[iMixer]->Init(m_pCaptureDeviceCollect, m_pRenderDeviceCollect);
	}

	// Cleaning up
	for (UINT i=0; i<m_nMixers; i++)
		free(ListMixerDeviceNames[i]);
	delete [] ListMixerDeviceNames;
	return true;

	Exit:
    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pCaptureDeviceCollect);
    SAFE_RELEASE(m_pRenderDeviceCollect);
	return false;
}

/*
	Create a list of Mixer-Device names
	Return: Number of Mixer-Devices

	Assumption: 
		1. m_nEndPoints is already initialized
		2. ListMixerDeviceNames is already allocated
		3. m_pCaptureDeviceCollect is already initialized
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
		hr = m_pCaptureDeviceCollect->Item(iEndPoint, &pDevice);
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

	
	// Initialize container for property value.
	PropVariantInit(&varName);
	varName.pwszVal = NULL;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&pEnum);
	EXIT_ON_ERROR(hr);

	hr = pEnum->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDefaultDevice);
	EXIT_ON_ERROR(hr);

	hr = pDefaultDevice->OpenPropertyStore(STGM_READ, &pProps);
	EXIT_ON_ERROR(hr);

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

	hr = pEnum->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDefaultDevice);
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

	hr = pEnum->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDefaultDevice);
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

bool  CAudioInputW7::SetMixerDeviceSelectInputLine(int iMixer, int iLine)
{

	_ASSERTE(m_MixerDevices);
	_ASSERTE(m_nMixers);

	bool res = false;

	for (UINT i=0; i<m_nMixers; i++)
	{
		CMixerDevice * md = m_MixerDevices[i];
		if (!md)
			continue;

		if (i==iMixer)
			res = md->SetSelectedInputLine(iLine);
		else
		{
			md->MuteSelectedInputLine(-1);
			md->MuteCaptureEndpoint(-1);
		}
	};

	return res;
}

/*
	Set the specified Input Line to be (un)muted
	The role of parameter restore:
		true:	Mute value for all other lines (on all mixers) is restored
		false:	All other lines (on all mixers) are temporarily muted

	Return former mute value
*/
bool CAudioInputW7::MuteSelectedInputLine(int Mixer, unsigned int line, bool restore, bool mute)
{
	_ASSERTE(m_MixerDevices);
	_ASSERTE(m_nMixers);

	bool res = false;

	for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
	{
		CMixerDevice * md = m_MixerDevices[iMixer];
		if (!md)
			continue;

		if (iMixer==Mixer)
			res = md->MuteSelectedInputLine(line, restore, mute);
		else
			md->MuteSelectedInputLine(-1, restore, mute);
	};

	return res;
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

void CAudioInputW7::Restore()
{
	if (!m_nMixers || !m_MixerDevices || !m_MixerDevices[0])
		return;

	for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
		m_MixerDevices[iMixer]->Restore();
}

int CAudioInputW7::SetSpeakers(int iMixer, bool restore, bool mute)
{
	// Sanity checks
	if ((UINT)iMixer >= m_nMixers || iMixer<0 || m_nMixers<=0 || !m_MixerDevices)
		return NULL;

	CMixerDevice * Mixer = m_MixerDevices[iMixer];
	return Mixer->SetSpeakers(restore,  mute);

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
	for (int i=0; i<m_nInputLines; i++)
	{
		free(m_ArrayInputLines[i].NameA);
		//free(m_ArrayInputLines[i].Name);
		free(m_ArrayInputLines[i].EndPointNameA);
		free(m_ArrayInputLines[i].EndPointName);

		SAFE_RELEASE(m_ArrayInputLines[i].lMute.p);
		SAFE_RELEASE(m_ArrayInputLines[i].lSelect.p);
		SAFE_RELEASE(m_ArrayInputLines[i].p);

	};
	
	free(m_ArrayInputLines);
	free(m_Name);
	free(m_NameA);
}
bool CAudioInputW7::CMixerDevice::Init(IMMDeviceCollection * pCaptureCollect, IMMDeviceCollection * pRenderCollect)
{
	HRESULT hr = S_OK;
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	IDeviceTopology *pDT = NULL;
    IConnector *pConnEndpoint = NULL;
    IConnector *pConnDevice = NULL;
	IPart *pPart = NULL;
	IAudioEndpointVolume *pEndptVol = NULL;

	// Sanity check
	if (!pCaptureCollect)
		goto Exit;

	// Get the number of endpoints
	UINT	nCaptureEndPoints=0;
	hr = pCaptureCollect->GetCount(&nCaptureEndPoints);
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
		//SAFE_RELEASE(pEndptVol);

		// Get pointer to endpoint number i.
		hr = pCaptureCollect->Item(iEndPoint, &pDevice);
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

		// Get Render/Capture feature
		IMMEndpoint * pEndPoint = NULL;
		hr = pDevice->QueryInterface(__uuidof(IMMEndpoint), (void**)&pEndPoint);
		EXIT_ON_ERROR(hr);	
		EDataFlow DataFlow;
		hr = pEndPoint->GetDataFlow(&DataFlow);
		EXIT_ON_ERROR(hr);

		// Get Master Mute Control then initialize it
		if (DataFlow == eRender)
		{
			FindMasterMute(pDevice);
			continue;
		};

		///// Identical, current endpoint belongs to this device
		///// FIll-up data in InputLine[m_nInputLines]

		// get device topology object for that endpoint
		hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDT);
		EXIT_ON_ERROR(hr);

#ifdef _DEBUG
		int iConn=1;
		LPWSTR DevId;
		pDT->GetConnector(iConn, &pConnEndpoint);
		pDT->GetDeviceId(&DevId);
#endif

		// get the single connector for that endpoint
		hr = pDT->GetConnector(0, &pConnEndpoint);
		EXIT_ON_ERROR(hr);

		// get the connector on the device that is connected to the connector on the endpoint
		hr = pConnEndpoint->GetConnectedTo(&pConnDevice);
		EXIT_ON_ERROR(hr);

		// QI on the device's connector for IPart
		hr = pConnDevice->QueryInterface(__uuidof(IPart), (void**)&pPart);
		EXIT_ON_ERROR(hr);

#ifdef _DEBUG
		IDeviceTopology *pPartDT = NULL;
		pPart->GetTopologyObject(&pPartDT);
		pPartDT->GetDeviceId(&DevId);
#endif

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

		//// Mute ////
		// Find IPart of the mute control 
		m_ArrayInputLines[m_nInputLines].lMute.p = FindMuteControl(pPart);

		// Initialize lmute structure (Mute status)
		bool Muted = false;
		if (m_ArrayInputLines[m_nInputLines].lMute.p)
			Muted = GetMuteStat(m_ArrayInputLines[m_nInputLines].lMute.p);
		m_ArrayInputLines[m_nInputLines].lMute.OrigStatus = m_ArrayInputLines[m_nInputLines].lMute.CurrentStatus = Muted;

		//// Input Selector ////
		bool selected=FALSE;
		UINT FeederID=0;
		m_ArrayInputLines[m_nInputLines].lSelect.p = FindInputSelectorControl(pPart, &selected, &FeederID);
		m_ArrayInputLines[m_nInputLines].lSelect.FeederID = FeederID;
		m_ArrayInputLines[m_nInputLines].lSelect.OrigStatus = selected;
		m_ArrayInputLines[m_nInputLines].lSelect.CurrentStatus = selected;

		//// Endpoint Volume (epVolume) and Endpoint Mute (epMute) ////
		// get device topology object for that endpoint
		hr = pDevice->Activate(__uuidof(IAudioEndpointVolume),CLSCTX_ALL, NULL, (void**)&pEndptVol);
		m_ArrayInputLines[m_nInputLines].epVolume.p = pEndptVol;
		if (pEndptVol)
		{
			float epVolume=0;
			BOOL epMute;

			// Get the current volume level (dB)
			hr = pEndptVol->GetMasterVolumeLevel(&epVolume);
			m_ArrayInputLines[m_nInputLines].epVolume.OrigVolume = epVolume;
			m_ArrayInputLines[m_nInputLines].epVolume.CurrentVolume = epVolume;

			// Get the current Mute state
			hr = pEndptVol->GetMute(&epMute);
			if (epMute)
			{
				m_ArrayInputLines[m_nInputLines].epVolume.OrigMuteStatus = true;
				m_ArrayInputLines[m_nInputLines].epVolume.CurrentMuteStatus = true;
			}
			else
			{
				m_ArrayInputLines[m_nInputLines].epVolume.OrigMuteStatus = false;
				m_ArrayInputLines[m_nInputLines].epVolume.CurrentMuteStatus = false;
			}
		};


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
	//SAFE_RELEASE(pEndptVol);

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

IPart * CAudioInputW7::CMixerDevice::FindMasterMuteControl(IPart * pIn, IPart * pPartMute )
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

    // get the list of incoming parts
   IPartsList *pConnectedParts = NULL;
   hr = pIn->EnumPartsIncoming(&pConnectedParts);

   // If reached the end of path 
   if (E_NOTFOUND == hr) 
	   return pPartMute;
   EXIT_ON_ERROR(hr);

   // This part is nither MUTE nor end of path - so let's continue
   UINT nParts = 0;
   hr = pConnectedParts->GetCount(&nParts);
   EXIT_ON_ERROR(hr);

   IPart *pConnectedPart = NULL;
   // walk the tree on each incoming part recursively
   for (UINT n = 0; n < nParts; n++) {
	   hr = pConnectedParts->GetPart(n, &pConnectedPart);
	   EXIT_ON_ERROR(hr);

	   pFoundPartMute = FindMasterMuteControl(pConnectedPart, pPartMute);
	   SAFE_RELEASE(pConnectedPart);

	   if (pFoundPartMute)
		   break;

   }

Exit:
    SAFE_RELEASE(pConnectedParts);
    SAFE_RELEASE(pMute);

	return pFoundPartMute;

}


// Find the master mute control
// Initialize the Mixer Device's MasterMute structure
void CAudioInputW7::CMixerDevice::FindMasterMute(IMMDevice  * pDevice)
{
	// Initializations
	HRESULT hr = S_OK;
	UINT nRenderEndPoints = 0;
	MasterMute.p = NULL;
	IDeviceTopology * pDT = NULL;
	IConnector * pConnEndpoint = NULL;
	IConnector * pConnDevice = NULL;
	IPart * pPart = NULL;

	// get device topology object for that endpoint
	hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&pDT);
	EXIT_ON_ERROR(hr);

#ifdef _DEBUG
	LPWSTR id;
	IPropertyStore * pProps = NULL;
	PROPVARIANT varName;

	LPWSTR DevId;
	pDT->GetDeviceId(&DevId);

	pDevice->GetId(&id);
	pDevice->OpenPropertyStore(STGM_READ, &pProps);

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the Device's friendly-name property.
	pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
	pProps->GetValue(PKEY_Device_FriendlyName, &varName);


	SAFE_RELEASE(pProps);

#endif

	// get the single connector for that endpoint
	hr = pDT->GetConnector(0, &pConnEndpoint);
	EXIT_ON_ERROR(hr);

	// get the connector on the device that is connected to the connector on the endpoint
	hr = pConnEndpoint->GetConnectedTo(&pConnDevice);
	EXIT_ON_ERROR(hr);

	// QI on the device's connector for IPart
	hr = pConnDevice->QueryInterface(__uuidof(IPart), (void**)&pPart);
	EXIT_ON_ERROR(hr);

#ifdef _DEBUG
		IDeviceTopology *pPartDT = NULL;
		pPart->GetTopologyObject(&pPartDT);
		pPartDT->GetDeviceId(&DevId);
#endif

	// Find Master Mute Control IPart interface
	MasterMute.p = FindMasterMuteControl(pPart);
	if (!MasterMute.p)
		goto Exit;

	// Get Master Mute Control state
	MasterMute.CurrentStatus = MasterMute.OrigStatus = GetMuteStat(MasterMute.p);

Exit:
	SAFE_RELEASE(pDT);
	SAFE_RELEASE(pConnEndpoint);
	SAFE_RELEASE(pConnDevice);

	return;
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

 void CAudioInputW7::CMixerDevice::RestoreMute(int iInputLine)
 {
	 if (iInputLine<0 || iInputLine >= m_nInputLines || !m_ArrayInputLines[iInputLine].lMute.p)
		 return;

	 RestoreMute(&m_ArrayInputLines[iInputLine].lMute);
 }

 void CAudioInputW7::CMixerDevice::RestoreCaptureEndpoint(int iInputLine)
 {
	 if (iInputLine<0 || iInputLine >= m_nInputLines || !m_ArrayInputLines[iInputLine].epVolume.p)
		 return;

	 bool OrigMuteStatus = m_ArrayInputLines[iInputLine].epVolume.OrigMuteStatus;
	 HRESULT hr = m_ArrayInputLines[iInputLine].epVolume.p->SetMute(OrigMuteStatus, NULL);
	 if (hr = S_OK)
		 m_ArrayInputLines[iInputLine].epVolume.CurrentMuteStatus = OrigMuteStatus;

 }
 void CAudioInputW7::CMixerDevice::RestoreMute(LineMute * plMute)
 {
	 if (!plMute || !plMute->p)
		 return;

	IAudioMute *pMute = NULL;

    HRESULT hr = plMute->p->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**)&pMute);
	if (E_NOINTERFACE == hr || FAILED(hr)) 
	{// not a mute node        
	} 
	else 
	{   // it's a mute node...
		hr = pMute->SetMute(plMute->OrigStatus,NULL);
		if (!FAILED(hr))
			plMute->CurrentStatus = plMute->OrigStatus;
	};

	SAFE_RELEASE(pMute);

 }

/*
	Find the Input Selector control (mux) corresponding to a given input

	Parameters:
		pIn:			Pointer to IPart
		isSelected:		Pointer to BOOL value. must be set to to FALSE. Becomes TRUE if mux selects this input
		pFeedingPart:	Pointer to IPart of the part feeding pIn (=NULL)
		pPartMux:		Pointer to IPart of the mux (=NULL)

	Returns
		Pointer to IPart of the mux (If found)

*/
IPart * CAudioInputW7::CMixerDevice::FindInputSelectorControl(IPart * pIn, bool * isSelected, UINT * pFeedingID, IPart * pFeedingPart /*=NULL*/, IPart * pPartMux/*=NULL*/ )
{
	HRESULT hr = S_OK;
	IAudioInputSelector *pMux = NULL;
	IPart * pFoundPartMux = NULL;
	IConnector * pConnector=NULL;

	// If mux part not already found - keep on looking for it
	if (!pPartMux)
	{	// see if this is a mux node part
		hr = pIn->Activate(CLSCTX_ALL, __uuidof(IAudioInputSelector), (void**)&pMux);
		if (E_NOINTERFACE == hr) 
		{ 
			//hr=+0;/* not a mux node */
		} 
		else if (FAILED(hr)) 
		{
			EXIT_ON_ERROR(hr);  
		}   
		else 
		{// it's a mux node...
			pPartMux = pIn;	// Save it
			// Is connected?
			if (pFeedingPart)
			{
				UINT SelectedID=NULL, FeedingID=NULL;
				pMux->GetSelection(&SelectedID);
				pFeedingPart->GetLocalId(&FeedingID);
				*pFeedingID = FeedingID;
				if (SelectedID==FeedingID)
					*isSelected=TRUE;
			};

		}
	}; // if (!pPartMux)

	// get the list of outgoing parts
   IPartsList *pConnectedParts = NULL;
   hr = pIn->EnumPartsOutgoing(&pConnectedParts);

   // If reached the end of path - verify that this is a rendering device
   if (E_NOTFOUND == hr) 
   {		
        // not an error... we've just reached the end of the path
		ConnectorType Type;
		hr = pIn->QueryInterface(__uuidof(IConnector), (void **)&pConnector);
		pConnector->GetType(&Type);
		
		if (Type == Software_IO || Software_Fixed)
			return pPartMux;
		else
			return NULL;
    };
   EXIT_ON_ERROR(hr);

   // This part is nither MUX nor a rendering device - so let's continue
   UINT nParts = 0;
   hr = pConnectedParts->GetCount(&nParts);
   EXIT_ON_ERROR(hr);

   IPart *pConnectedPart = NULL;
   // walk the tree on each outgoing part recursively
   for (UINT n = 0; n < nParts; n++) {
	   hr = pConnectedParts->GetPart(n, &pConnectedPart);
	   EXIT_ON_ERROR(hr);

	   pFoundPartMux = FindInputSelectorControl(pConnectedPart, isSelected, pFeedingID, pIn, pPartMux);
	   SAFE_RELEASE(pConnectedPart);

	   if (pFoundPartMux)
		   break;

   }

Exit:
    SAFE_RELEASE(pConnectedParts);
    SAFE_RELEASE(pMux);
	SAFE_RELEASE(pConnector);

	return pFoundPartMux;

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

bool CAudioInputW7::CMixerDevice::SetSelectedInputLine(int iLine)
{
	// Test scope
	if (!m_nInputLines || iLine >= m_nInputLines )
		return false;

#ifdef _DEBUG
	bool escape = false;
	if (escape)
		return true;
#endif

	bool selected=false, muted, epMuted;

	selected	= SelectInputLine(iLine);
	muted		= MuteOutputLine(iLine);
	epMuted		= MuteCaptureEndpoint(iLine, false);

	return selected;
}

bool CAudioInputW7::CMixerDevice::SelectInputLine(int iLine)
{
	HRESULT hr = S_OK;
	IAudioInputSelector * pMux = NULL;
	UINT sel=0;
	//static const GUID AudioSessionSelInput = { 0x2715279f, 0x4139, 0x4ba0, { 0x9c, 0xb2, 0xb3, 0x52, 0xf2, 0xb5, 0x8a, 0x4a } };
	IPart * pPartMux = m_ArrayInputLines[iLine].lSelect.p;
	
	if (!pPartMux)
		return false;

	hr = pPartMux->Activate(CLSCTX_ALL, __uuidof(IAudioInputSelector), (void**)&pMux);
	if (E_NOINTERFACE == hr || FAILED(hr)) 
	{
		return false;
	} 
	else 
	{   // it's a mux node...
		hr = pMux->SetSelection(m_ArrayInputLines[iLine].lSelect.FeederID, NULL /*&AudioSessionSelInput*/);
		pMux->GetSelection(&sel);
	};

	SAFE_RELEASE(pMux);
	return FAILED(hr);
}

bool CAudioInputW7::CMixerDevice::MuteOutputLine(int iLine, bool mute , bool temporary )
{
    IAudioMute *pMute = NULL;
	BOOL WasMuted = FALSE;
	IPart * pMutePart = NULL;
	HRESULT hr = S_OK;

	// Go over all the lines. Mute line iLine and restore all others
	for (int i=0; i<m_nInputLines; i++)
	{
		pMutePart =  m_ArrayInputLines[i].lMute.p;
		if (!pMutePart)
			return false;

		hr = pMutePart->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**)&pMute);
		if (E_NOINTERFACE == hr || FAILED(hr)) 
		{
			return false;
		} 
		else 
		{   // it's a mute node...
			if (i == iLine)
			{	// Selected line: Save current state, set new mute state and save it is NOT temporary
				hr = pMute->GetMute(&WasMuted);
				hr = pMute->SetMute(mute,NULL);
				if (!FAILED(hr) && !temporary)
					m_ArrayInputLines[i].lMute.CurrentStatus = mute;
			}
			else
			{	// Not the selected line:
				if (!temporary)
					RestoreMute(i); // If not temporary then just restore line to original value
				else
					pMute->SetMute(!mute,NULL);// Temporary: Inverse mute values
			};
		};

		SAFE_RELEASE(pMute);
	}

	if (WasMuted)
		return (true);
	else
		return (false);

}

// Mute/Un-mute the capture Endpoint coresponding to a given input line
// Restore all other lines
bool CAudioInputW7::CMixerDevice::MuteCaptureEndpoint(int iLine, bool mute)
{
    IAudioEndpointVolume *pEpVolume = NULL;
	BOOL WasMuted = FALSE;
	HRESULT hr = S_OK;

	// Go over all the lines. Mute line iLine and restore all others
	for (int i=0; i<m_nInputLines; i++)
	{
		pEpVolume =  m_ArrayInputLines[i].epVolume.p;
		if (!pEpVolume)
			return false;

		if (i == iLine)
		{	// Selected line: Save current state, set new mute state and save it is NOT temporary
			hr = pEpVolume->GetMute(&WasMuted);
			hr = pEpVolume->SetMute(mute,NULL);
			if (!FAILED(hr))
				m_ArrayInputLines[i].epVolume.CurrentMuteStatus = mute;
		}
		else
			pEpVolume->SetMute(m_ArrayInputLines[i].epVolume.OrigMuteStatus,NULL);

	};

	if (WasMuted)
		return (true);
	else
		return (false);

}




// Restore original setup of this device including:
//	Selected input
//  Mute setting of every input
void CAudioInputW7::CMixerDevice::Restore()
{
	if (m_nInputLines<1 || !m_ArrayInputLines )
		return;

	// Go to every input line and restore its original status
	for (int iInputLine=0; iInputLine<m_nInputLines; iInputLine++)
	{
		// If this line was originally selected then select it and mark it as currently selected
		if (m_ArrayInputLines[iInputLine].lSelect.OrigStatus)
		{
			if (SelectInputLine(iInputLine))
				m_ArrayInputLines[iInputLine].lSelect.CurrentStatus = true;
		};

		// Restore this line's original mute status
		RestoreMute(iInputLine);
		// Restore the corresponding Endpoint original mute status
		RestoreCaptureEndpoint(iInputLine);
	};
}

void CAudioInputW7::CMixerDevice::Mute(LineMute * plMute, bool mute)
{
	if (!plMute || !plMute->p)
		return;

	IAudioMute *pMute = NULL;
	HRESULT hr = S_OK;

	hr = plMute->p->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**)&pMute);
	if (E_NOINTERFACE == hr || FAILED(hr)) 
		return;
	else 
	{
		hr = pMute->SetMute(mute,NULL);
		if (!FAILED(hr))
			plMute->CurrentStatus = mute;
	};

	SAFE_RELEASE(pMute);
}

int CAudioInputW7::CMixerDevice::SetSpeakers(bool restore, bool mute)
{
	/* Test */
	if (!MasterMute.p)
		return 0;


	if (restore)
		RestoreMute(&MasterMute);
	else
		Mute(&MasterMute, mute);

	return 1;
}

/*
	Set Input Line number 'iLine' to be (un)muted
	If iLine is (-1) then none of the Input Lines is selected to be (un)muted.

	The role of parameter restore:
		true:	Mute value for all other lines is restored
		false:	All other lines are (temporarily) muted

	Return former mute value
*/
bool CAudioInputW7::CMixerDevice::MuteSelectedInputLine(unsigned int iLine, bool restore, bool mute)
{
	int i;
	bool res = false;

	/* Test scope */
	if ((int)iLine >= m_nInputLines)
		return false;


	/* Loop on every line on this mixer device */
	for (i=0; i<m_nInputLines; i++)
	{		
		// Skip if no mute control for this line
		if (!m_ArrayInputLines[i].lMute.p)
			continue;

		if (i==iLine)
		{	// This is the selected line. Save current status and (un)mute
			res = m_ArrayInputLines[i].lMute.CurrentStatus;
			Mute(&m_ArrayInputLines[i].lMute, mute);
		}
		else
		{	// Treat all other line according 'restore': restore value or force temporary mute
			if (restore)
				RestoreMute(&m_ArrayInputLines[i].lMute);
			else
				Mute(&m_ArrayInputLines[i].lMute);
		}

	};

	return res;
}


//////////////////// Mixer Device  (End)//////////////////////////////////////////////////////////////////