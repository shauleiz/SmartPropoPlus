// SppInterface.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Windowsx.h>
#include "SppInterface.h"
#include "SppInterfaceAudio.h"

#pragma  comment(lib, "windowscodecs.lib")
#pragma  comment(lib, "Dwrite.lib")
#pragma  comment(lib, "D2d1.lib")

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

// This is an example of an exported variable
SPPINTERFACE_API int nSppInterface=0;

// This is an example of an exported function.
SPPINTERFACE_API int fnSppInterface(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see SppInterface.h for the class definition
CSppInterface::CSppInterface() :
m_pDirect2dFactory(NULL),
m_pRenderTarget(NULL),
m_pLightSlateGrayBrush(NULL),
m_pCornflowerBlueBrush(NULL),
m_pMeasureBrush(NULL),
m_pDarkViolet(NULL),
m_pTextFormat(NULL),
m_pBtnTextFormat(NULL),
m_pMsrTextFormat(NULL),
m_pButtonColor(NULL),
m_pButtonPauseBitmap(NULL),
m_pButtonPlayBitmap(NULL),
m_pDWriteFactory(NULL),
m_hWinThread(NULL),
m_ChildWin(NULL)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	return;
}

HRESULT CSppInterface::Initialize(HWND hWndParent )
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
		wcex.lpfnWndProc   = CSppInterface::WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = sizeof(LONG_PTR);
		wcex.hInstance     = HINST_THISCOMPONENT;
		wcex.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		wcex.lpszMenuName  = NULL;
		wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
		wcex.lpszClassName = L"D2DSppInterface";

		RegisterClassEx(&wcex);


		// Because the CreateWindow function takes its size in pixels,
		// obtain the system DPI and use it to scale the window size.
		FLOAT dpiX, dpiY;

		// The factory returns the current system DPI. This is also the value it will use
		// to create its own windows.
		m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


		// Create the window.
		m_hwnd = CreateWindow(
			L"D2DSppInterface",
			L"SmartPropoPlus Configuration",
			WS_OVERLAPPED /*| WS_SIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME| WS_HSCROLL*/,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(960.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(720.f * dpiY / 96.f)),
			hWndParent ,
			NULL,
			HINST_THISCOMPONENT,
			this
			);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			////Make this window 50% alpha
			//SetWindowLong(m_hwnd, GWL_EXSTYLE, GetWindowLong(m_hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
			//SetLayeredWindowAttributes(m_hwnd, 0, (255 * 50) / 100, LWA_ALPHA);

			// Show main window
			ShowWindow(m_hwnd, SW_SHOWNORMAL);
			UpdateWindow(m_hwnd);
			
			//// Create Chind-windows
			//m_ChildWin = new(CSppInterfaceChildWin);
			//m_ChildWin->Initialize(m_hwnd);
		}

	}

	return hr;
}

HRESULT CSppInterface::CreateDeviceIndependentResources()
// Creates resources that are not bound to a particular device.
// Their lifetime effectively extends for the duration of the
// application.
{
	HRESULT hr = S_OK;
    static const WCHAR msc_fontName[] = L"TimesNewRoman";
    static const FLOAT msc_fontSize = 8;
    static const FLOAT msc_btnfontSize = 12;
    static const FLOAT msc_msrfontSize = 12;

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

    if (SUCCEEDED(hr))
    {
         hr = m_pDWriteFactory->CreateTextFormat(
            msc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(msc_btnfontSize),
			L"", // locale
			&m_pBtnTextFormat
			);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pBtnTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		hr = m_pBtnTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	};

    if (SUCCEEDED(hr))
    {
         hr = m_pDWriteFactory->CreateTextFormat(
            msc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            static_cast<FLOAT>(msc_msrfontSize),
			L"", // locale
			&m_pMsrTextFormat
			);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pMsrTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		hr = m_pMsrTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	};

	if (SUCCEEDED(hr))
	{
		// Start window main thread
		DWORD dwThreadId;
		m_hWinThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&WinThread,0,0,&dwThreadId);
		if (!m_hWinThread)
			hr = ERROR_ACCESS_DENIED;
	};

	if (SUCCEEDED(hr))
    {
        // Create WIC factory.
		CoInitialize(NULL);
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void **>(&m_pWICFactory)
            );
    }

	return hr;
}

LRESULT CALLBACK CSppInterface::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
// Handles window messages.
	LRESULT result = 0;
	static float orig_opacity = 1;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		CSppInterface *pSppInterface = (CSppInterface *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pSppInterface)
			);

		result = 1;
	}
	else
	{
		CSppInterface *pSppInterface = reinterpret_cast<CSppInterface *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA
			)));

		bool wasHandled = false;

		if (pSppInterface)
		{
			switch (message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pSppInterface->OnResize(width, height);
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

			case WM_PAINT:
				{
					pSppInterface->OnRender();
					ValidateRect(hwnd, NULL);
				}
				result = 0;
				wasHandled = true;
				break;

			//case WM_DESTROY:
			//	{
			//		PostQuitMessage(0);
			//	}
			//	result = 1;
			//	wasHandled = true;
			//	break;


			case WM_MOUSEMOVE:
				if ((inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , *pSppInterface->m_AudioUnit->GetRect() )))
				{
					pSppInterface->m_AudioUnit->Select();
				}
				else
				{
					pSppInterface->m_AudioUnit->UnSelect();				
				}
				pSppInterface->OnRender();
				//wasHandled = true;
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

void CSppInterface::OnResize(UINT width, UINT height)
//  If the application receives a WM_SIZE message, this method
//  resizes the render target appropriately.
{
	if (m_pRenderTarget)
	{
		// Note: This method can fail, but it's okay to ignore the
		// error here, because the error will be returned again
		// the next time EndDraw is called.

		// When this code is commented out - resizing the window resizes the data
		m_pRenderTarget->Resize(D2D1::SizeU(width, height));
	}
}



HRESULT CSppInterface::OnRender()
{
// This method discards device-specific
// resources if the Direct3D device dissapears during execution and
// recreates the resources the next time it's invoked.
	HRESULT hr = S_OK;
	float sqSize = 50;			// Absolute square size
	float scale = sqSize/192;	// Scale wave to 1milliSec per square (X-axis)
	ID2D1GeometrySink *pSink = NULL;

	hr = CreateDeviceResources();

	if (SUCCEEDED(hr))
	{
		m_pRenderTarget->BeginDraw();

		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRenderTarget->Clear(D2D1::ColorF(0.3f, 0.3f, 0.3f, 0.8f ));

		D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

		// Draw a rounded rectangle on along the left edge
		// The recangle should have an edge and margin
		float margin = 20.0f;
		if (m_AudioUnit)
			m_AudioUnit->Display(margin, margin,rtSize.width / 5 - 2*margin, rtSize.height - 2*margin);

#if 0
		float RectWidth = rtSize.width / 5 - 2*margin;
		float RectHeight = rtSize.height - 2*margin;
 
		// Define a rounded rectangle.
        m_roundedRect = D2D1::RoundedRect( D2D1::RectF(margin, margin, RectWidth, RectHeight), 10.f, 10.f);
		
		// Draw and fill the rectangle.
        m_pRenderTarget->DrawRoundedRectangle(m_roundedRect, m_pDarkViolet, 2.f);
		m_pRenderTarget->FillRoundedRectangle(m_roundedRect, m_pLightSlateGrayBrush);


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

		///////// Controls ////////////////////////////////////////////////////
		// Shift Right
		DisplayRightScrollButton();
		if (m_manual_shift_pressed_r)
			m_manual_shift-=50;

		// Shift Left
		DisplayLeftScrollButton();
		if (m_manual_shift_pressed_l)
			m_manual_shift+=50;

		// Play/Pause button
		DisplayPausePlayButtonBM(!m_isPlaying, m_PlayPauseRect);


		/////////  Draw wave form //////////////////////////////////////////////////////
		// Implement as Path Geometry
		hr = m_pDirect2dFactory->CreatePathGeometry(&m_pWaveGeometry);
		hr = m_pWaveGeometry->Open(&pSink);
		pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
		if (m_points && m_npoints>3)
		{
			pSink->BeginFigure(m_points[0],D2D1_FIGURE_BEGIN_HOLLOW );
			pSink->AddLines(m_points, m_npoints-3);
			pSink->EndFigure(D2D1_FIGURE_END_OPEN);
		}
		hr = pSink->Close();
		SafeRelease(&pSink);

		// Scale & Shift wave form (Mark the triger point)
		D2D1_MATRIX_3X2_F xFormScale= D2D1::Matrix3x2F::Scale(D2D1::Size(scale, 1.0f),D2D1::Point2F(0.0f, 0.0f));
		D2D1_MATRIX_3X2_F xFormShift=D2D1::Matrix3x2F::Translation(2*sqSize/scale-m_offset-m_manual_shift, 0);
		m_pRenderTarget->SetTransform(xFormShift*xFormScale); // Order of multiplection is crucial
		m_pRenderTarget->DrawGeometry(m_pWaveGeometry, m_pDarkViolet, 1.f);
		// Trigger point marked as two close lines
		m_pRenderTarget->DrawLine(D2D1::Point2F(m_offset-10,rtSize.height/2+10),D2D1::Point2F(m_offset-10,rtSize.height/2-10),m_pArrowColor,7.0);
		m_pRenderTarget->DrawLine(D2D1::Point2F(m_offset+10,rtSize.height/2+10),D2D1::Point2F(m_offset+10,rtSize.height/2-10),m_pArrowColor,7.0);
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		////////////////////////////////////////////////////////////////////////////////

		// Draw measurement line and write measurment (in millisecs)
		// Test if measurment is on and draw measurement line
		// If delta is more than 0.05 milli then write value on the upper left corner
		if (m_isMeasuring)
		{
			m_pRenderTarget->DrawLine(m_measureStartPoint, m_measureEndPoint, m_pMeasureBrush, 0.3f);
			float delta = (m_measureEndPoint.x - m_measureStartPoint.x)/sqSize;
			if (abs(delta) >= 0.05)
			{
				hr = StringCchPrintf(textBuffer, tMaxLen, L"%.2fmS", delta);
				m_pRenderTarget->DrawText(
					textBuffer,
					static_cast<UINT>(wcsnlen(textBuffer, ARRAYSIZE(textBuffer))),
					m_pMsrTextFormat,
					D2D1::RectF(20.0f, 10.0f, 80.0f, 35.0f),
					m_pCornflowerBlueBrush,
					D2D1_DRAW_TEXT_OPTIONS_NONE
					);
			}
		}
#endif

		hr = m_pRenderTarget->EndDraw();
	}

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_OK;
		DiscardDeviceResources();
	}

	return hr;
}


HRESULT CSppInterface::CreateDeviceResources()
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
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat( DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED ) ),
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
				D2D1::ColorF(D2D1::ColorF::Red),
				&m_pMeasureBrush
				);
		}

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::DarkOrange),
				&m_pArrowColor
				);
			m_pArrowColor->SetOpacity(0.5f);
		}

		if (SUCCEEDED(hr))
		{
			// Create a brush for buttons
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::DarkGray),
				&m_pButtonColor
				);
			m_pButtonColor->SetOpacity(0.5f);
		}

		if (SUCCEEDED(hr))
		{
			// Create a gray brush.
			hr = m_pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::LightSlateGray, 1.0f),
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

		// Create bitmaps for buttons from files
		// Files must be located in current folder

		m_AudioUnit = new CSppInterfaceAudio(m_pRenderTarget);
		if (!m_AudioUnit)
			hr = S_FALSE;
		else
			m_AudioUnit->Initialize();
		// DEBUG - 
		m_AudioUnit->AddJack(L"ID Number 1", L"Friendly name for Jack number 1", 0x00450033);
		m_AudioUnit->AddJack(L"ID Number 2", L"Friendly name for Jack number 2", 0x00453300);
		m_AudioUnit->AddJack(L"ID Number 3", L"Friendly name for Jack number 3");
		m_AudioUnit->RemoveJack(L"ID Number 1");
		m_AudioUnit->AddJack(L"ID Number 1", L"Friendly name for Jack number 1", 0x00450033);
		// - DEBUG 
	}

	return hr;
}

void CSppInterface::DiscardDeviceResources()
// Discards device-dependent resources. These resources must be
// recreated when the Direct3D device is lost.
{
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
	SafeRelease(&m_pDarkViolet);
	SafeRelease(&m_pMeasureBrush);
	SafeRelease(&m_pArrowColor);
	SafeRelease(&m_pButtonColor);
	SafeRelease(&m_pButtonPauseBitmap);
	SafeRelease(&m_pButtonPlayBitmap);

}

bool CSppInterface::inRect(int x, int y, D2D1_RECT_F rect)
{
	if (rect.bottom >=y && rect.top<=y && rect.left<=x && rect.right>=x)
		return true;
	else
		return false;
}



