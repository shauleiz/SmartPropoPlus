// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#include "DemoApp.h"
#include <Strsafe.h>

// Provides the application entry point.
int WINAPI WinMain(
				   HINSTANCE /* hInstance */,
				   HINSTANCE /* hPrevInstance */,
				   LPSTR /* lpCmdLine */,
				   int /* nCmdShow */
				   )
{
	// Use HeapSetInformation to specify that the process should
	// terminate if the heap manager detects an error in any heap used
	// by the process.
	// The return value is ignored, because we want to continue running in the
	// unlikely event that HeapSetInformation fails.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			DemoApp app;

			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}

// DemoApp constructor
DemoApp::DemoApp() :
m_hwnd(NULL),
m_pDirect2dFactory(NULL),
m_pRenderTarget(NULL),
m_pLightSlateGrayBrush(NULL),
m_pCornflowerBlueBrush(NULL),
m_pTextFormat(NULL),
m_pDWriteFactory(NULL)

{
}

// DemoApp destructor
// Releases the application's resources.
DemoApp::~DemoApp()
{
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
    SafeRelease(&m_pTextFormat);
	SafeRelease(&m_pDWriteFactory);
}

// Creates the application window and device-independent
// resources.
HRESULT DemoApp::Initialize()
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
		wcex.lpfnWndProc   = DemoApp::WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = sizeof(LONG_PTR);
		wcex.hInstance     = HINST_THISCOMPONENT;
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName  = NULL;
		wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"D2DDemoApp";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


		// Create the window.
		m_hwnd = CreateWindow(
			L"D2DDemoApp",
			L"Direct2D Demo App",
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
HRESULT DemoApp::CreateDeviceIndependentResources()
{
	HRESULT hr = S_OK;
    static const WCHAR msc_fontName[] = L"TimesNewRoman";
    static const FLOAT msc_fontSize = 11;

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
    {
		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	};

	return hr;
}


// Creates resources that are bound to a particular
// Direct3D device. These resources need to be recreated
// if the Direct3D device dissapears, such as when the display
// changes, the window is remoted, etc.
HRESULT DemoApp::CreateDeviceResources()
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

// Discards device-dependent resources. These resources must be
// recreated when the Direct3D device is lost.
void DemoApp::DiscardDeviceResources()
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
}

// Runs the main window message loop.
void DemoApp::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// This method discards device-specific
// resources if the Direct3D device dissapears during execution and
// recreates the resources the next time it's invoked.
HRESULT DemoApp::OnRender()
{
	HRESULT hr = S_OK;
	int millisecs=350;
	static float i=0;

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw a grid background.
		int width = static_cast<int>(rtSize.width);
		int height = static_cast<int>(rtSize.height);
		int sqSize = 50;
		float low = (rtSize.height/2 + sqSize*2.5f);
		float hi  = (rtSize.height/2 - sqSize*2.5f);
		float tic = static_cast<float>(sqSize)/25;
		UINT tMaxLen;
		WCHAR textBuffer[20];
		tMaxLen = sizeof(textBuffer)/sizeof(WCHAR);

		for (int x = 0; x < width; x += sqSize)
		{
			m_pRenderTarget->DrawLine(
				D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
				D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
				m_pLightSlateGrayBrush,
				0.5f
				);

		// Add text - Time and Voltage
		hr = StringCchPrintf(textBuffer, tMaxLen, L"%d", x/25);
		m_pRenderTarget->DrawText(
			textBuffer,
			static_cast<UINT>(wcsnlen(textBuffer, ARRAYSIZE(textBuffer))),
			m_pTextFormat,
            D2D1::RectF(static_cast<FLOAT>(x)-20.0f, rtSize.height/2+10, static_cast<FLOAT>(x)+20.0f, rtSize.height/2+20.0f),
            m_pDarkViolet,
            D2D1_DRAW_TEXT_OPTIONS_NONE
            );

		}

		for (int y = height/2; y < height; y += sqSize)
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


		// Draw a demo wave form
		// Implement as Path Geometry
		hr = m_pDirect2dFactory->CreatePathGeometry(&m_pWaveGeometry);
		ID2D1GeometrySink *pSink = NULL;
		hr = m_pWaveGeometry->Open(&pSink);
		pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		pSink->BeginFigure(D2D1::Point2F(0,low),D2D1_FIGURE_BEGIN_HOLLOW );
		D2D1_POINT_2F points[] = {
			D2D1::Point2F(i+tic*4, low),
			D2D1::Point2F(i+tic*4, hi),
			D2D1::Point2F(i+tic*15, hi),
			D2D1::Point2F(i+tic*15, low),
			D2D1::Point2F(i+tic*30, low), 
			D2D1::Point2F(i+tic*30, hi),
			D2D1::Point2F(i+tic*45, hi),
			D2D1::Point2F(i+tic*45, low),
			D2D1::Point2F(i+tic*1000, low), 
		};
		pSink->AddLines(points, ARRAYSIZE(points));
		pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		hr = pSink->Close();
		SafeRelease(&pSink);
		m_pRenderTarget->DrawGeometry(m_pWaveGeometry, m_pDarkViolet, 1.f);

		// Simulate moving wave
		i+=2;
		if (i>700) i=0;





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


//  If the application receives a WM_SIZE message, this method
//  resizes the render target appropriately.
void DemoApp::OnResize(UINT width, UINT height)
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

// Handles window messages.
LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		DemoApp *pDemoApp = (DemoApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pDemoApp)
			);

		result = 1;
	}
	else
	{
		DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (pDemoApp)
		{
			switch (message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pDemoApp->OnResize(width, height);
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

			case WM_TIMER:
			case WM_PAINT:
				{
					pDemoApp->OnRender();
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

