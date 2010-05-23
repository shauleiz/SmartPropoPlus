#include "stdafx.h"
#include ".\audioinputw7.h"

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

	CreateArrayOfEndPointNames();

	// Creating an array of real mixer devices (Derived from the topology)
	int nNumRealMixerDevices = GetNumRealMixerDevices();

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
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not find default audio Endpoint\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
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
	return m_nEndPoints;
};

const char * CAudioInputW7::GetMixerDeviceName(int index)
{
	// Sanity checks
	if ((UINT)index >= m_nEndPoints || index<0 || !m_ArrayOfEndPointNames)
		return "";

	return m_ArrayOfEndPointNames[index];
}


bool CAudioInputW7::CreateArrayOfEndPointNames(void)
{
	HRESULT hr = S_OK;
	IMMDevice *pDeviceIn = NULL;
	PROPVARIANT varName;
	IPropertyStore *pProps = NULL;
	IAudioClient * pClientIn = NULL;
	LPWSTR pwszID = NULL;
	bool Ret = false;

	
	// Sanity check
	if (!m_pDeviceCollect)
		return false;

	// Create an array of Endpoint names only if 
	//	Array does not exist
	//	There is at least one endpoint
	if (m_ArrayOfEndPointNames || !m_nEndPoints)
		return false;

	m_ArrayOfEndPointNames = new char*[m_nEndPoints];

	for (UINT index=0; index<m_nEndPoints ; index++)
	{
		// Get pointer to endpoint number i.
		hr = m_pDeviceCollect->Item(index, &pDeviceIn);
		EXIT_ON_ERROR(hr);

		// Get the endpoint ID string.
		hr = pDeviceIn->GetId( &pwszID);
		EXIT_ON_ERROR(hr);

		// TODO
		//// Test if device is usable
		// Activate
		//hr = pDeviceIn->Activate(&IID_IAudioClient, CLSCTX_ALL,  NULL, (void**)&pClientIn);
		//EXIT_ON_ERROR(hr);



		hr = pDeviceIn->OpenPropertyStore(STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);

		// Convert to char
		size_t size = wcslen(varName.pwszVal);
		m_ArrayOfEndPointNames[index] = new char[size+2];
		w2char(varName.pwszVal, m_ArrayOfEndPointNames[index], (int)size+1);
	};

	Ret=true;

Exit:
	SAFE_RELEASE(pDeviceIn);
    SAFE_RELEASE(pProps);
	return Ret;
}

int CAudioInputW7::GetMixerDeviceIndex(char * mixer)
{
	if (!m_ArrayOfEndPointNames)
		return -1;

	for (UINT index=0; index<m_nEndPoints ; index++)
	{
		if (!strcmp(m_ArrayOfEndPointNames[index], mixer))
			return index;
	}

	return -1;
}

const char * CAudioInputW7::GetMixerDeviceInputLineName(int Mixer, int Line)
{
	return NULL;
}

bool CAudioInputW7::GetMixerDeviceSelectInputLine(int Mixer, unsigned int * iLine)
{
	*iLine = -1;
	return false;
}

bool  CAudioInputW7::SetMixerDeviceSelectInputLine(int Mixer, int Line)
{
	return false;
}

int CAudioInputW7::GetNumRealMixerDevices()
{
	return -1;
}


//////////////////// Mixer Device ////////////////////////////////////////////////////////////////////////

// Create an object that represents an Audio Mixer Device
CAudioInputW7::CMixerDevice::CMixerDevice(LPWSTR name)
{
	m_Name = _wcsdup(name);
}

LPWSTR CAudioInputW7::CMixerDevice::GetName(void)
{
	return m_Name;
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

		// Now, that pPart points to the input pin of the Mixer, we can collect the data we need
		
		// Keep a pointer to the interface (Just in case ...)
		m_ArrayInputLines[m_nInputLines].p = pPart;

		// Input pin name
		hr = pPart->GetName(&(m_ArrayInputLines[m_nInputLines].Name));
		EXIT_ON_ERROR(hr);

		// Get Global ID
		hr = pPart->GetGlobalId(&(m_ArrayInputLines[m_nInputLines].GlobalId));
		EXIT_ON_ERROR(hr);

		// Init Mute structure
		m_ArrayInputLines[m_nInputLines].lMute.p = FindMuteControl(pPart);

		// Initialize lmute structure (Mute status)
		// bool Muted = GetMuteStat(m_ArrayInputLines[m_nInputLines].lMute.p);
		// m_ArrayInputLines[m_nInputLines].lMute.OrigStatus = m_ArrayInputLines[m_nInputLines].lMute.CurrentStatus = Muted;

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

int CreateArrayOfInputLines(void)
{
	return -1;
}
//////////////////// Mixer Device  (End)//////////////////////////////////////////////////////////////////