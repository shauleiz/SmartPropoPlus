#include "stdafx.h"
#include ".\audioinputw7.h"



/***************************************************************************************
	Class CAudioInputW7 (Derived from CAudioInput)

	Object represents the Vista/Windows7 audio system (single object)
	members consist of list of Endpoint devices & index of current Endpoint device
	Endpoint devices are also called Mixer (devices)
***************************************************************************************/

CAudioInputW7::CAudioInputW7(void)
{
	HRESULT hr = S_OK;


	/* Initializations */
	m_pEnumerator = NULL;
	m_pDeviceCollect = NULL;
	m_nEndPoints = 0;

	//BCDE0395-E52F-467C-8E3D-C4579291692E
	static GUID const CLSID_MMDeviceEnumerator = {
     0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };


	 //{A95664D2-9614-4F35-A746-DE8DB63617E6}
	static GUID const CLSID_IMMDeviceEnumerator = {
     0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

	 //{1CB9AD4C-DBFA-4C32-B178-C2F568A703B2}
	static GUID const IID_IAudioClient = {
     0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2} };

	//{C8ADBD64-E71E-48A0-A4DE-185C395CD317}
	static GUID const IID_IAudioCaptureClient = {
     0xC8ADBD64, 0xE71E, 0x48A0, {0xA4,0xDE,0x18,0x5C,0x39,0x5C,0xD3,0x17} };

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	//_ASSERTE(0);

	/* Create a device enumarator then a collection of endpoints and finally get the number of endpoints */
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
	if (FAILED(hr))
	{
		MessageBox(NULL,"WASAPI (CAudioInputW7): Could not find default audio Endpoint\r\nStopping audio capture", "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create a list of Endpoints
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

	return;

	Exit:
    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pDeviceCollect);


}

CAudioInputW7::~CAudioInputW7(void)
{
    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pDeviceCollect);

}

int CAudioInputW7::GetCountMixerDevice()
{
	return m_nEndPoints;
};

const char * CAudioInputW7::GetMixerDeviceName(int index)
{
	HRESULT hr = S_OK;
	IMMDevice *pDeviceIn = NULL;
	PROPVARIANT varName;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;

	
	// Sanity check
	if (!m_pDeviceCollect)
		return NULL;

	// Get pointer to endpoint number i.
	hr = m_pDeviceCollect->Item(index, &pDeviceIn);
	EXIT_ON_ERROR(hr);

	// Get the endpoint ID string.
	hr = pDeviceIn->GetId( &pwszID);
	EXIT_ON_ERROR(hr);

	hr = pDeviceIn->OpenPropertyStore(STGM_READ, &pProps);
	EXIT_ON_ERROR(hr);

	// Initialize container for property value.
	PropVariantInit(&varName);

	// Get the endpoint's friendly-name property.
	hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
	EXIT_ON_ERROR(hr);

	Exit:
    SAFE_RELEASE(pDeviceIn);
    SAFE_RELEASE(pProps);
	return NULL;
}

