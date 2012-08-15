// Audio.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <devicetopology.h>
#include <Mmdeviceapi.h>
#include "Audio.h"
#include <tchar.h>
#include <Functiondiscoverykeys_devpkey.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
CAudioInputW7 * g_audio;						// Audio interface object (Only one for the moment)

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);






int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR    lpCmdLine, int  nCmdShow)
// Standard WinMain that creates single audio object
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	// Create an audio object
	// This object holds all the info about the computer audio endpoints (capture only)
	// After initializing, the object can receive quiries and issue notifications regarding
	// endpoints
	g_audio = new(CAudioInputW7);

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_AUDIO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
		return FALSE;

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUDIO));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUDIO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_AUDIO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		g_audio->~CAudioInputW7();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}



////////////////////// Class CAudioInputW7 //////////////////////
CAudioInputW7::CAudioInputW7(void) 
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pNotifyChange = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
	m_nEndPoints = 0;
	HRESULT hr = S_OK;
	m_nMixers = 0;
	m_CaptureDevices.nDev = 0;
	m_CaptureDevices.DeviceName = NULL;
	m_CaptureDevices.state = NULL;
	m_CaptureDevices.id = NULL;
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	bool created = Create();
	return;
}

bool CAudioInputW7::Create(void)
{
	/* Initializations */
	m_pEnumerator = NULL;
	m_pNotifyChange = NULL;
	m_pCaptureDeviceCollect = NULL;
	m_pRenderDeviceCollect = NULL;
	m_nEndPoints = 0;
	// m_ArrayOfEndPointNames = NULL;
	HRESULT hr = S_OK;

	/* Create a device enumarator then a collection of endpoints and finally get the number of endpoints */
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, CLSID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
	if (FAILED(hr))
	{
		MessageBox(NULL, TEXT("WASAPI (CAudioInputW7): Could not create audio Endpoint enumerator \r\nStopping audio capture"), L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// Create a list of all Capture Endpoints
	DWORD dwStateMask = DEVICE_STATE_ACTIVE | DEVICE_STATE_DISABLED | DEVICE_STATE_UNPLUGGED;
	hr = m_pEnumerator->EnumAudioEndpoints(eCapture,  dwStateMask, &m_pCaptureDeviceCollect);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not enumerate capture audio Endpoint\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};


	// Register endpoint notofication callback
	m_pNotifyChange = new(CMMNotificationClient);
	hr = m_pEnumerator->RegisterEndpointNotificationCallback(m_pNotifyChange);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not register notification callback\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};


	hr = m_pCaptureDeviceCollect->GetCount(&m_nEndPoints);
	if (FAILED(hr))
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not count audio Endpoints\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		EXIT_ON_ERROR(hr);
	};

	// First Enumeration
	hr = Enumerate();

	// Create an array of Mixer Device Names
	LPWSTR* ListMixerDeviceNames = new LPWSTR[m_nEndPoints+1];
	int nMixerDev = CreateListDeviceNames(ListMixerDeviceNames);
	if (!nMixerDev)
	{
		MessageBox(NULL,L"WASAPI (CAudioInputW7): Could not create a list of Mixer Device Names\r\nStopping audio capture", L"SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_ICONERROR);
		goto Exit;
	};

	return true;

	Exit:
	if (m_pEnumerator && m_pNotifyChange)
		m_pEnumerator->UnregisterEndpointNotificationCallback(m_pNotifyChange);
    SAFE_RELEASE(m_pNotifyChange);
    SAFE_RELEASE(m_pEnumerator);
    SAFE_RELEASE(m_pCaptureDeviceCollect);
    SAFE_RELEASE(m_pRenderDeviceCollect);
	return false;
}

CAudioInputW7::~CAudioInputW7(void)
{
	if (m_pEnumerator && m_pNotifyChange)
		m_pEnumerator->UnregisterEndpointNotificationCallback(m_pNotifyChange);
	SAFE_RELEASE(m_pEnumerator);
	SAFE_RELEASE(m_pNotifyChange);
	SAFE_RELEASE(m_pCaptureDeviceCollect);
	//Restore();
	//delete[] m_ArrayOfEndPointNames;

	//for (UINT iMixer=0; iMixer<m_nMixers; iMixer++)
	//	delete m_MixerDevices[iMixer];
	//delete[] m_MixerDevices;
}



int CAudioInputW7::CreateListDeviceNames(LPWSTR * ListMixerDeviceNames)
/*
	Create a list of Mixer-Device names
	Return: Number of Mixer-Devices

	Assumption: 
		1. m_nEndPoints is already initialized
		2. ListMixerDeviceNames is already allocated
		3. m_pCaptureDeviceCollect is already initialized
*/
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
		//hr = pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);

		ListMixerDeviceNames[iEndPoint] = _wcsdup(varName.pwszVal);
		m_nMixers++;

		// Get the state of the device:
		// DEVICE_STATE_ACTIVE / DEVICE_STATE_DISABLED / DEVICE_STATE_NOTPRESENT / DEVICE_STATE_UNPLUGGED
		DWORD state;
		hr = pDevice->GetState(&state);
		EXIT_ON_ERROR(hr);

		LPWSTR id;
		hr = pDevice->GetId(&id);
		EXIT_ON_ERROR(hr);
		CoTaskMemFree(id);




		// Search for identical entry
		//for (UINT i=0; i<=iEndPoint; i++)
		//{
		//	if (!ListMixerDeviceNames[i])
		//	{
		//		ListMixerDeviceNames[i] = _wcsdup(varName.pwszVal);
		//		m_nMixers++;
		//		break;
		//	};

		//	if (!wcscmp(varName.pwszVal, ListMixerDeviceNames[i]))
		//		break;
		//};

	};

	Exit:
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pDevice);

	return m_nMixers;
}


#if 0
int CAudioInputW7::ListInputs(LPWSTR * ListMixerDeviceNames)
/*
	Create a list of Mixer-Device names
	Return: Number of Mixer-Devices

	Assumption: 
		1. m_nEndPoints is already initialized
		2. ListMixerDeviceNames is already allocated
		3. m_pCaptureDeviceCollect is already initialized
*/
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
		//hr = pProps->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);

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

#endif

HRESULT	CAudioInputW7::Enumerate(void)
// Fill in structure m_CaptureDevices that specifies the current state of all capture endpoint devices
// Assumption: 
//	1. m_nEndPoints is valid
//	2. m_pCaptureDeviceCollect is already initialized
// Call this function after every notification of change because the content of m_CaptureDevices becomes invelid
// with every change in the system
{
	IPropertyStore * pProps = NULL;
	IMMDevice * pDevice = NULL;
	PROPVARIANT varName;
	HRESULT hr = S_OK;

	// Initialize data staructure
	for (int i=0; i<m_CaptureDevices.nDev;i++)
	{
		free(m_CaptureDevices.DeviceName[i]); 
		free(m_CaptureDevices.id[i]); 
	};
	free(m_CaptureDevices.DeviceName);
	free(m_CaptureDevices.id);
	free(m_CaptureDevices.state);

	// Prepare for enumeration
	m_CaptureDevices.nDev = m_nEndPoints;
	m_CaptureDevices.DeviceName = new LPWSTR[m_nEndPoints];
	m_CaptureDevices.id = new LPWSTR[m_nEndPoints];
	m_CaptureDevices.state = new DWORD[m_nEndPoints];

	// Loop on all endpoints
	for (UINT iEndPoint=0; iEndPoint<m_nEndPoints; iEndPoint++)
	{

		// Get pointer to endpoint number iEndPoint.
		hr = m_pCaptureDeviceCollect->Item(iEndPoint, &pDevice);
		EXIT_ON_ERROR(hr);

		hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the Device's friendly-name property.
		hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);
		m_CaptureDevices.DeviceName[iEndPoint] = _wcsdup(varName.pwszVal);

		// Get the state of the device:
		// DEVICE_STATE_ACTIVE / DEVICE_STATE_DISABLED / DEVICE_STATE_NOTPRESENT / DEVICE_STATE_UNPLUGGED
		DWORD state;
		hr = pDevice->GetState(&state);
		EXIT_ON_ERROR(hr);
		m_CaptureDevices.state[iEndPoint] = state;

		// Get device unique id
		LPWSTR id;
		hr = pDevice->GetId(&id);
		EXIT_ON_ERROR(hr);
		m_CaptureDevices.id[iEndPoint] = _wcsdup(id);
		CoTaskMemFree(id);
	};
	Exit:
    SAFE_RELEASE(pProps);
    SAFE_RELEASE(pDevice);

	return hr;

};


//--------------------------------------------------------------------------------------------------
//-----------------------------------------------------------
// Example implementation of IMMNotificationClient interface.
// When the status of audio endpoint devices change, the
// MMDevice module calls these methods to notify the client.
// Based on  http://msdn.microsoft.com/en-us/library/windows/desktop/dd370810(v=vs.85).aspx
//-----------------------------------------------------------

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

CMMNotificationClient::CMMNotificationClient()
    {
		_cRef=1;
		_pEnumerator=NULL;
    }

    CMMNotificationClient::~CMMNotificationClient()
    {
        SAFE_RELEASE(_pEnumerator)
    }

    // IUnknown methods -- AddRef, Release, and QueryInterface

    ULONG STDMETHODCALLTYPE CMMNotificationClient::AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE CMMNotificationClient::Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE CMMNotificationClient::QueryInterface(
                                REFIID riid, VOID **ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IMMNotificationClient) == riid)
        {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    // Callback methods for device-event notifications.

    HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDefaultDeviceChanged(
                                EDataFlow flow, ERole role,
                                LPCWSTR pwstrDeviceId)
    {
        char  *pszFlow = "?????";
        char  *pszRole = "?????";

        _PrintDeviceName(pwstrDeviceId);

        switch (flow)
        {
        case eRender:
            pszFlow = "eRender";
            break;
        case eCapture:
            pszFlow = "eCapture";
            break;
        }

        switch (role)
        {
        case eConsole:
            pszRole = "eConsole";
            break;
        case eMultimedia:
            pszRole = "eMultimedia";
            break;
        case eCommunications:
            pszRole = "eCommunications";
            break;
        }

        // printf("  -->New default device: flow = %s, role = %s\n",  pszFlow, pszRole);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
    {
        _PrintDeviceName(pwstrDeviceId);

        // printf("  -->Added device\n");
        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
        _PrintDeviceName(pwstrDeviceId);

        // printf("  -->Removed device\n");
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceStateChanged(
                                LPCWSTR pwstrDeviceId,
                                DWORD dwNewState)
    {
        char  *pszState = "?????";

        _PrintDeviceName(pwstrDeviceId);

        switch (dwNewState)
        {
        case DEVICE_STATE_ACTIVE:
            pszState = "ACTIVE";
            break;
        case DEVICE_STATE_DISABLED:
            pszState = "DISABLED";
            break;
        case DEVICE_STATE_NOTPRESENT:
            pszState = "NOTPRESENT";
            break;
        case DEVICE_STATE_UNPLUGGED:
            pszState = "UNPLUGGED";
            break;
        }

        // printf("  -->New device state is DEVICE_STATE_%s (0x%8.8x)\n",   pszState, dwNewState);

        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnPropertyValueChanged(
                                LPCWSTR pwstrDeviceId,
                                const PROPERTYKEY key)
    {
        _PrintDeviceName(pwstrDeviceId);

        // printf("  -->Changed device property "
               //"{%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x}#%d\n",
               //key.fmtid.Data1, key.fmtid.Data2, key.fmtid.Data3,
               //key.fmtid.Data4[0], key.fmtid.Data4[1],
               //key.fmtid.Data4[2], key.fmtid.Data4[3],
               //key.fmtid.Data4[4], key.fmtid.Data4[5],
               //key.fmtid.Data4[6], key.fmtid.Data4[7],
               //key.pid);
        return S_OK;
    }

// Given an endpoint ID string, print the friendly device name.
HRESULT CMMNotificationClient::_PrintDeviceName(LPCWSTR pwstrId)
{
    HRESULT hr = S_OK;
    IMMDevice *pDevice = NULL;
    IPropertyStore *pProps = NULL;
    PROPVARIANT varString;

    CoInitialize(NULL);
    PropVariantInit(&varString);

    if (_pEnumerator == NULL)
    {
        // Get enumerator for audio endpoint devices.
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                              NULL, CLSCTX_INPROC_SERVER,
                              __uuidof(IMMDeviceEnumerator),
                              (void**)&_pEnumerator);
    }
    if (hr == S_OK)
    {
        hr = _pEnumerator->GetDevice(pwstrId, &pDevice);
    }
    if (hr == S_OK)
    {
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    }
    if (hr == S_OK)
    {
        // Get the endpoint device's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varString);
    }
    // printf("----------------------\nDevice name: \"%S\"\n" "  Endpoint ID string: \"%S\"\n",
           //(hr == S_OK) ? varString.pwszVal : L"null device",
           //(pwstrId != NULL) ? pwstrId : L"null ID");

    PropVariantClear(&varString);

    SAFE_RELEASE(pProps)
    SAFE_RELEASE(pDevice)
    CoUninitialize();
    return hr;
}
//--------------------------------------------------------------------------------------------------