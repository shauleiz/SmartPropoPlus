// PulseScope.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <StrSafe.h>
#include "PulseScope.h"


// This is an example of an exported variable
PULSESCOPE_API int nPulseScope=0;


// This is the constructor of a class that has been exported.
// see PulseScope.h for the class definition
CPulseScope::CPulseScope() :
m_hwnd(NULL),
m_pDirect2dFactory(NULL),
m_pRenderTarget(NULL),
m_pLightSlateGrayBrush(NULL),
m_pCornflowerBlueBrush(NULL),
m_pTextFormat(NULL),
m_pDWriteFactory(NULL),
m_hWinThread(NULL),
m_points(NULL), 
m_npoints(0),
m_offset(0)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	return;
}

CPulseScope::~CPulseScope()
{
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
    SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pDWriteFactory);
}


HRESULT CPulseScope::Initialize()
// Creates the application window and device-independent
// resources.
{
	HRESULT hr;

	// Initialize device-indpendent resources, such
	// as the Direct2D factory.
	hr = CreateDeviceIndependentResources();

	if (SUCCEEDED(hr))
	{
		// Register the window class.
		WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc   = CPulseScope::WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = sizeof(LONG_PTR);
		wcex.hInstance     = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName  = NULL;
		wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"D2DPulseScope";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


		// Create the window.
		m_hwnd = CreateWindow(
			L"D2DPulseScope",
			L"Pulse Scope for SmartPropoPlus",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
			NULL,
			NULL,
			HINST_THISCOMPONENT,
			this
			);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
		}

		// Add a refresh timer
		SetTimer(m_hwnd, 1, 10, NULL);

	}

	return hr;
}




// Creates resources that are not bound to a particular device.
// Their lifetime effectively extends for the duration of the
// application.
HRESULT CPulseScope::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;
    static const WCHAR msc_fontName[] = L"TimesNewRoman";
    static const FLOAT msc_fontSize = 8;

	// Create a Direct2D factory.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }

    if (SUCCEEDED(hr))
    {
         hr = m_pDWriteFactory->CreateTextFormat(
            msc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(msc_fontSize),
			L"", // locale
			&m_pTextFormat
			);
	}

	if (SUCCEEDED(hr))
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

	{
		// Start window main thread
		DWORD dwThreadId;
		m_hWinThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WinThread,0,0,&dwThreadId);
		if (!m_hWinThread)
			hr = ERROR_ACCESS_DENIED;
	};

	return hr;
}



// Handles window messages.
LRESULT CALLBACK CPulseScope::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		CPulseScope *pPulseScope = (CPulseScope *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pPulseScope)
			);

		result = 1;
	}
	else
	{
		CPulseScope *pPulseScope = reinterpret_cast<CPulseScope *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (pPulseScope)
		{
			switch (message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pPulseScope->OnResize(width, height);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_DISPLAYCHANGE:
				{
					InvalidateRect(hwnd, NULL, FALSE);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_BUFF_READY:
			case WM_PAINT:
				{
					pPulseScope->OnRender();
					ValidateRect(hwnd, NULL);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_DESTROY:
				{
					PostQuitMessage(0);
				}
				result = 1;
				wasHandled = true;
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}

//  If the application receives a WM_SIZE message, this method
//  resizes the render target appropriately.
void CPulseScope::OnResize(UINT width, UINT height)
{
	if (m_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.

#if 1
		// When this code is commented out - resizing the window resizes the data
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
#endif
	}
}

HRESULT CPulseScope::OnRender()
{
// This method discards device-specific
// resources if the Direct3D device dissapears during execution and
// recreates the resources the next time it's invoked.
	HRESULT hr = S_OK;
	float sqSize = 30;			// Absolute square size
	float scale = sqSize/192;	// Scale wave to 1milliSec per square (X-axis)

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw a grid background.
		float width = rtSize.width;
		float height = rtSize.height;
		float low = (rtSize.height/2 + sqSize*2.5f);
		float hi  = (rtSize.height/2 - sqSize*2.5f);
		UINT tMaxLen;
		WCHAR textBuffer[20];
		tMaxLen = sizeof(textBuffer)/sizeof(WCHAR);

		for (float x = 0; x < width; x += sqSize)
		{
			m_pRenderTarget->DrawLine(D2D1::Point2F(x, 0.0f), D2D1::Point2F(x, rtSize.height), m_pLightSlateGrayBrush, 0.5f);

		// Add text - Time and Voltage
		hr = StringCchPrintf(textBuffer, tMaxLen, L"%dmS", static_cast<UINT>(x/sqSize));
		m_pRenderTarget->DrawText(
			textBuffer,
			static_cast<UINT>(wcsnlen(textBuffer, ARRAYSIZE(textBuffer))),
			m_pTextFormat,
            D2D1::RectF(static_cast<FLOAT>(x)-20.0f, rtSize.height/2+10, static_cast<FLOAT>(x)+20.0f, rtSize.height/2+20.0f),
            m_pCornflowerBlueBrush,
            D2D1_DRAW_TEXT_OPTIONS_NONE
            );

		}

		for (float y = height/2; y < height; y += sqSize)
		{
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(0.0f, static_cast<FLOAT>(y+sqSize)),
				D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y+sqSize)),
				m_pLightSlateGrayBrush,
				0.5f
				);

			m_pRenderTarget->DrawLine(
				D2D1::Point2F(0.0f, static_cast<FLOAT>(height - y-sqSize)),
				D2D1::Point2F(rtSize.width, static_cast<FLOAT>(height - y-sqSize)),
				m_pLightSlateGrayBrush,
				0.5f
				);
		}

		// Draw a heavy line to mark the zero voltage line
		m_pRenderTarget->DrawLine(
			D2D1::Point2F(0.0f, static_cast<FLOAT>(height/2)),
			D2D1::Point2F(rtSize.width, static_cast<FLOAT>(height/2)),
			m_pLightSlateGrayBrush,
			1.5f
			);


		// Draw wave form
		// Implement as Path Geometry
		hr = m_pDirect2dFactory->CreatePathGeometry(&m_pWaveGeometry);
		ID2D1GeometrySink *pSink = NULL;
		hr = m_pWaveGeometry->Open(&pSink);
		pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		pSink->BeginFigure(D2D1::Point2F(0,low),D2D1_FIGURE_BEGIN_HOLLOW );
		if (m_points && m_npoints>3)
			pSink->AddLines(m_points, m_npoints-3);
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		hr = pSink->Close();
		SafeRelease(&pSink);

		// Scale & Shift
		D2D1_MATRIX_3X2_F xFormScale= D2D1::Matrix3x2F::Scale(D2D1::Size(scale, 1.0f),D2D1::Point2F(0.0f, 0.0f));
		D2D1_MATRIX_3X2_F xFormShift=D2D1::Matrix3x2F::Translation(10, 0);
		m_pRenderTarget->SetTransform(xFormScale * xFormShift);

		m_pRenderTarget->DrawGeometry(m_pWaveGeometry, m_pDarkViolet, 1.f);








//		goto L1;
//
//		// Draw two rectangles.
//		D2D1_RECT_F rectangle1 = D2D1::RectF(
//			rtSize.width/2 - 50.0f,
//			rtSize.height/2 - 50.0f,
//			rtSize.width/2 + 50.0f,
//			rtSize.height/2 + 50.0f
//			);
//
//		D2D1_RECT_F rectangle2 = D2D1::RectF(
//			rtSize.width/2 - 100.0f,
//			rtSize.height/2 - 100.0f,
//			rtSize.width/2 + 100.0f,
//			rtSize.height/2 + 100.0f
//			);
//
//		D2D1_POINT_2F  center = {rtSize.height/4, rtSize.width/4};
//		D2D1_ELLIPSE elllipse1 = D2D1::Ellipse(
//			center,
//			(rtSize.width+i)/10,
//			(rtSize.width+i)/10
//			);
//
//		i++;
//		if (i>500) i=0;
//
//		// Draw a filled rectangle.
//		m_pRenderTarget->FillRectangle(&rectangle1, m_pLightSlateGrayBrush);
//
//		// Draw the outline of a rectangle.
//		m_pRenderTarget->DrawRectangle(&rectangle2, m_pCornflowerBlueBrush);
//
//		m_pRenderTarget->DrawEllipse(elllipse1, m_pDarkViolet, 2.0f);
//L1:
		hr = m_pRenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}


HRESULT CPulseScope::CreateDeviceResources()
// Creates resources that are bound to a particular
// Direct3D device. These resources need to be recreated
// if the Direct3D device dissapears, such as when the display
// changes, the window is remoted, etc.
{
	HRESULT hr = S_OK;

	if (!m_pRenderTarget)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(
			rc.right - rc.left,
			rc.bottom - rc.top
			);

		// Create a Direct2D render target.
		hr = m_pDirect2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, size),
			&m_pRenderTarget
			);

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::DarkViolet),
				&m_pDarkViolet
				);
		}


		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray),
				&m_pLightSlateGrayBrush
				);
		}
		if (SUCCEEDED(hr))
		{
			// Create a blue brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
				&m_pCornflowerBlueBrush
				);
		}

	}

	return hr;
}

void CPulseScope::DiscardDeviceResources()
// Discards device-dependent resources. These resources must be
// recreated when the Direct3D device is lost.
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

void CPulseScope::DisplayPulseData(UINT nPulses, float *Length, float *Value)
{
	float offset = 0;			// Offset to the beginning of the longest pulse
	float maxlength = 0;		// Size of the longest pulse (in the first half of the buffer)
	UINT arrsize = nPulses*2+1;	// Size of array of points

	// Release old array of points and assign a new one
	if (m_points)
	{
		delete m_points;
		m_points = NULL;
	};
	m_points = new D2D1_POINT_2F[arrsize];

	// Populate each point and calculate overall offset
	m_points[0].x = 0;
	m_points[0].y = Value[0];
	for (UINT i=1; i<nPulses; i++)
	{
		// Convert from pulse data to points
		m_points[2*i].x	  = m_points[2*i-1].x	= Length[i-1] + m_points[2*i-2].x;
		m_points[2*i-1].y = m_points[2*i-2].y = Value[i-1];

		// find the longest pulse in the first half buffer
		if (m_points[2*i].x < MID_BUF)
		{
			if (Length[i-1] > maxlength)
			{	// Longist pulse (so far) found
				maxlength = Length[i-1];
				offset = m_points[2*i].x;
			}
		} // if still in first half of buffer
	} // for loop

	m_npoints = arrsize;
	m_offset = offset;

	PostMessage(m_hwnd, WM_BUFF_READY, (WPARAM)arrsize, (LPARAM)offset);
}


// Initialize Pulse Scope object and return pointer to object
PULSESCOPE_API CPulseScope * InitPulseScope(void)
{
	CPulseScope * PulseScopeObj = new  CPulseScope;
	PulseScopeObj->Initialize();
	return PulseScopeObj;
}

void	WINAPI WinThread(void)
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
}
