// PulseScope.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <Windowsx.h>
#include "PulseScope.h"
#include <StrSafe.h>
#include <WinMessages.h>


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
m_pBtnTextFormat(NULL),
m_pMsrTextFormat(NULL),
m_pDWriteFactory(NULL),
m_hWinThread(NULL),
m_points(NULL), 
m_npoints(0),
m_offset(0),
m_isMeasuring(false),
m_isPlaying(true),
m_PlayPauseRect(D2D1::RectF(10,30,100,60)),
m_close_button_rect(D2D1::RectF()),
m_left_button_rect(D2D1::RectF()),
m_right_button_rect(D2D1::RectF()),
m_info_pane_rect(D2D1::RectF()),
m_manual_shift(0),
m_manual_shift_pressed_r(false),
m_manual_shift_pressed_l(false)	,
m_InfoText(NULL),
m_InfoTextSize(0)
{
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
	return;
}

CPulseScope::~CPulseScope()
{
	m_isPlaying = false;
	//SendMessage(m_hwnd, WM_DESTROY, 0, 0);
	SafeRelease(&m_pDirect2dFactory);
	SafeRelease(&m_pRenderTarget);
	SafeRelease(&m_pLightSlateGrayBrush);
	SafeRelease(&m_pCornflowerBlueBrush);
    SafeRelease(&m_pTextFormat);
    SafeRelease(&m_pBtnTextFormat);
    SafeRelease(&m_pMsrTextFormat);
	SafeRelease(&m_pDWriteFactory);
}


HRESULT CPulseScope::Initialize(HWND hWndParent )
// Creates the application window and device-independent
// resources.
{
	HRESULT hr;
	m_hWndParent = hWndParent;

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
			WS_OVERLAPPED | WS_THICKFRAME /*| WS_HSCROLL*/,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
			static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
			hWndParent ,
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

		::SetWindowLongPtr(
			hwnd,
			GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pPulseScope)
			);

		result = 1;
	}
	else
	{
		CPulseScope *pPulseScope = reinterpret_cast<CPulseScope *>(static_cast<LONG_PTR>(
			::GetWindowLongPtr(
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

			//case WM_DESTROY:
			//	{
			//		PostQuitMessage(0);
			//	}
			//	result = 1;
			//	wasHandled = true;
			//	break;

			case WM_LBUTTONUP:
				pPulseScope->m_manual_shift_pressed_r=false;
				pPulseScope->m_manual_shift_pressed_l=false;
				pPulseScope->m_close_button_pressed = false;
				break;

			case WM_LBUTTONDOWN:
				if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_left_button_rect))
				{/*Left scroll key*/
					pPulseScope->m_manual_shift_pressed_l=true;
				}
				else if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_right_button_rect))
				{/* Right scroll key */
					pPulseScope->m_manual_shift_pressed_r=true;
				}
				else if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_close_button_rect))
				{/* Close button */
					SendMessage(pPulseScope->m_hWndParent, WMSPP_DLG_PLSSCOP, FALSE, 0);
				}
				else if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_PlayPauseRect))
				{
					if (pPulseScope->m_isPlaying)
						pPulseScope->m_isPlaying = false;
					else
						pPulseScope->m_isPlaying = true;
				}
				else
					pPulseScope->StartMeasure(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				break;

			case WM_MOUSEMOVE:
				if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_left_button_rect))
					pPulseScope->m_manual_shift_pressed_l = false;
				if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_right_button_rect))
					pPulseScope->m_manual_shift_pressed_r = false;
 				if (inRect(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) , pPulseScope->m_close_button_rect))
					pPulseScope->m_close_button_pressed = false;

				if (wParam & MK_LBUTTON)
					pPulseScope->Measure(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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
void CPulseScope::Measure(int x, int y)
{
	m_measureEndPoint = D2D1::Point2F(static_cast<FLOAT>(x),static_cast<FLOAT>(y));

}

void CPulseScope::StartMeasure(int x, int y)
{
	// Start measuring distance between two points
	// Initialize start point and m_isMeasuring flag 
	if (!m_pRenderTarget)
		return;

	m_isMeasuring = true;
	m_measureStartPoint = D2D1::Point2F(static_cast<FLOAT>(x),static_cast<FLOAT>(y));
	m_measureEndPoint = m_measureStartPoint;

}

void CPulseScope::OnResize(UINT width, UINT height)
//  If the application receives a WM_SIZE message, this method
//  resizes the render target appropriately.
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
	float sqSize = 50;			// Absolute square size
	float scale = sqSize/192;	// Scale wave to 1milliSec per square (X-axis)
	ID2D1GeometrySink *pSink = NULL;

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
		WCHAR textBuffer[20] = { 0 };
		tMaxLen = sizeof(textBuffer)/sizeof(WCHAR);

		for (float x = 0; x < width; x += sqSize)
		{
			m_pRenderTarget->DrawLine(D2D1::Point2F(x, 0.0f), D2D1::Point2F(x, rtSize.height), m_pLightSlateGrayBrush, 0.5f);

		// Add text - Time and Voltage
		hr = StringCchPrintf(textBuffer, tMaxLen, L"%umS", static_cast<UINT>(x/sqSize));
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

		// Information Pane
		DisplayWaveInfo();

		// Play/Pause button
		DisplayPausePlayButton(!m_isPlaying, m_PlayPauseRect);

		// Close Button
		DisplayCloseButton();


		/////////  Draw wave form //////////////////////////////////////////////////////
		// Implement as Path Geometry
		hr = m_pDirect2dFactory->CreatePathGeometry(&m_pWaveGeometry);
		hr = m_pWaveGeometry->Open(&pSink);
		if (hr != S_OK)
			return hr;
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
			if (fabs(delta) >= 0.05)
			{
				hr = StringCchPrintf(textBuffer, tMaxLen, L"%.2fmS", delta);
				m_pRenderTarget->DrawText(
					textBuffer,
					static_cast<UINT>(ARRAYSIZE(textBuffer)),
					m_pMsrTextFormat,
					D2D1::RectF(20.0f, 10.0f, 80.0f, 35.0f),
					m_pCornflowerBlueBrush,
					D2D1_DRAW_TEXT_OPTIONS_NONE
					);
			}
		}


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
	SafeRelease(&m_pDarkViolet);
	SafeRelease(&m_pMeasureBrush);
	SafeRelease(&m_pArrowColor);
	SafeRelease(&m_pButtonColor);
}	


void CPulseScope::DisplayPulseData(UINT nPulses, float *Length, float *Value)
// Based on
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd756686(v=vs.85).aspx
{
	if (nPulses<1)
		return;

	FLOAT offset = 0;			// Offset to the beginning of the longest pulse
	FLOAT maxlength = 0;		// Size of the longest pulse (in the first half of the buffer)
	UINT arrsize = nPulses*2+1;	// Size of array of points
	FLOAT fTmp;

	// Pause state
	if (!m_isPlaying)
	{
		PostMessage(m_hwnd, WM_BUFF_READY, (WPARAM)arrsize, (LPARAM)offset);
		return;
	}

	// Release old array of points and assign a new one
	if (m_points)
	{
		delete m_points;
		m_points = NULL;
	};

	m_points = new D2D1_POINT_2F[arrsize + 2];

	// Populate each point and calculate overall offset
	m_points[0].x = 0;
	m_points[0].y = Value[0];
	for (UINT i=1; i<nPulses; i++)
	{
		// Convert from pulse data to points
		fTmp = Length[i-1] + m_points[2*i-2].x;
		m_points[2*i].x	    = fTmp;
		m_points[2*i-1].x	= fTmp;
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

void CPulseScope::SetWaveInfo(WCHAR * strOut, size_t size)
{
	m_InfoText = strOut;
	m_InfoTextSize = size;

}


HRESULT CPulseScope::LoadResourceBitmap(
    ID2D1RenderTarget *pRenderTarget,
    IWICImagingFactory *pIWICFactory,
    PCWSTR resourceName,
    PCWSTR resourceType,
    UINT destinationWidth,
    UINT destinationHeight,
    ID2D1Bitmap **ppBitmap
    )
{
    IWICBitmapDecoder *pDecoder = NULL;
    IWICBitmapFrameDecode *pSource = NULL;
    IWICStream *pStream = NULL;
    IWICFormatConverter *pConverter = NULL;
    IWICBitmapScaler *pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResourceW(HINST_THISCOMPONENT, resourceName, resourceType);
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);

		hr = imageResDataHandle ? S_OK : E_FAIL;
	}

	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImageFile = LockResource(imageResDataHandle);

		hr = pImageFile ? S_OK : E_FAIL;
	}
    if (SUCCEEDED(hr))
    {
        // Calculate the size.
        imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);

        hr = imageFileSize ? S_OK : E_FAIL;        
    }

    if (SUCCEEDED(hr))
    {
          // Create a WIC stream to map onto the memory.
        hr = pIWICFactory->CreateStream(&pStream);
    }

    if (SUCCEEDED(hr))
    {
        // Initialize the stream with the memory pointer and size.
        hr = pStream->InitializeFromMemory(
            reinterpret_cast<BYTE*>(pImageFile),
            imageFileSize
            );
    }

    if (SUCCEEDED(hr))
    {
        // Create a decoder for the stream.
        hr = pIWICFactory->CreateDecoderFromStream(
            pStream,
            NULL,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
            );
    }

    if (SUCCEEDED(hr))
    {
        // Create the initial frame.
        hr = pDecoder->GetFrame(0, &pSource);
    }

    if (SUCCEEDED(hr))
    {
        // Convert the image format to 32bppPBGRA
        // (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }

	if (SUCCEEDED(hr))
	{          
		hr = pConverter->Initialize(
        pSource,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.f,
        WICBitmapPaletteTypeMedianCut
        );
	}
    if (SUCCEEDED(hr))
    {
        //create a Direct2D bitmap from the WIC bitmap.
        hr = pRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            NULL,
            ppBitmap
            );
    }

    SafeRelease(&pDecoder);
    SafeRelease(&pSource);
    SafeRelease(&pStream);
    SafeRelease(&pConverter);
    SafeRelease(&pScaler);

    return hr;
}


void CPulseScope::DisplayCloseButton(void)
{
	// Get render target size
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

	// Create button rectangle
	m_close_button_rect = D2D1::RectF(rtSize.width - 60, 30, rtSize.width - 5, 60);
	D2D1_ROUNDED_RECT buttonRect = D2D1::RoundedRect(m_close_button_rect, 3, 3);

	// Draw a filled rectangle.
	m_pRenderTarget->FillRoundedRectangle(&buttonRect, m_pButtonColor);

	// Write text (Close)
	WCHAR textBuffer[20] = { 0 };
	UINT tMaxLen = sizeof(textBuffer) / sizeof(WCHAR);
	StringCchPrintf(textBuffer, tMaxLen, L"Close");

	m_pRenderTarget->DrawText(
		textBuffer,
		static_cast<UINT>(ARRAYSIZE(textBuffer)),
		m_pBtnTextFormat,
		m_close_button_rect,
		m_pMeasureBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE
		);
}

void CPulseScope::DisplayPausePlayButton(bool Play, D2D1_RECT_F rect1)
{
	// Get render target size
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

	// Create button rectangle
	D2D1_ROUNDED_RECT buttonRect = D2D1::RoundedRect(rect1, 3, 3);

	// Draw a filled rectangle.
	m_pRenderTarget->FillRoundedRectangle(&buttonRect, m_pButtonColor);

	// Write text (Pause/Play)
	WCHAR textBuffer[20] = { 0 };
	UINT tMaxLen = sizeof(textBuffer) / sizeof(WCHAR);
	if (Play)
		StringCchPrintf(textBuffer, tMaxLen, L"Play");
	else
		StringCchPrintf(textBuffer, tMaxLen, L"Pause");

	m_pRenderTarget->DrawText(
		textBuffer,
		static_cast<UINT>(ARRAYSIZE(textBuffer)),
		m_pBtnTextFormat,
		rect1,
		m_pCornflowerBlueBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE
		);
}

void CPulseScope::DisplayRightScrollButton(void)
// Draw right arrow button
{
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
	m_right_button_rect = D2D1::RectF(rtSize.width-30,rtSize.height-45,rtSize.width-5,rtSize.height-15);

	ID2D1GeometrySink *pSink = NULL;
	ID2D1PathGeometry * TriangleRightGeometry;
	HRESULT hr = m_pDirect2dFactory->CreatePathGeometry(&TriangleRightGeometry);
	if (hr != S_OK)
		return;
	D2D1_POINT_2F TriangleRightPoints[3] = {
		D2D1::Point2F(m_right_button_rect.right-5,  (m_right_button_rect.top+m_right_button_rect.bottom)/2),
		D2D1::Point2F(m_right_button_rect.right-15, (m_right_button_rect.top+m_right_button_rect.bottom)/2-10),
		D2D1::Point2F(m_right_button_rect.right-15, (m_right_button_rect.top+m_right_button_rect.bottom)/2+10)
	};
	hr = TriangleRightGeometry->Open(&pSink);
	if (hr != S_OK)
		return;
	pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
	pSink->BeginFigure(
		D2D1::Point2F(m_right_button_rect.right-5, (m_right_button_rect.bottom+m_right_button_rect.top)/2),
		D2D1_FIGURE_BEGIN_FILLED);
	pSink->AddLines(TriangleRightPoints,3);
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();
	SafeRelease(&pSink);
	m_pRenderTarget->FillGeometry(TriangleRightGeometry, m_pArrowColor);
	m_pRenderTarget->DrawRectangle(m_right_button_rect, m_pArrowColor);
}

void CPulseScope::DisplayLeftScrollButton(void)
// Draw left arrow button
{
	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
	m_left_button_rect = D2D1::RectF(5,rtSize.height-45,30,rtSize.height-15);

	ID2D1GeometrySink *pSink = NULL;
	ID2D1PathGeometry * TriangleLeftGeometry;
	HRESULT hr = m_pDirect2dFactory->CreatePathGeometry(&TriangleLeftGeometry);
	if (hr != S_OK)
		return;
	D2D1_POINT_2F TriangleLeftPoints[3] = {
		D2D1::Point2F(m_left_button_rect.left+5, (m_left_button_rect.top+m_left_button_rect.bottom)/2),
		D2D1::Point2F(m_left_button_rect.left+15, (m_left_button_rect.top+m_left_button_rect.bottom)/2-10),
		D2D1::Point2F(m_left_button_rect.left+15, (m_left_button_rect.top+m_left_button_rect.bottom)/2+10)
	};
	hr = TriangleLeftGeometry->Open(&pSink);
	if (hr != S_OK)
		return;

	pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
	pSink->BeginFigure(
		D2D1::Point2F(m_left_button_rect.left+5, (m_left_button_rect.top+m_left_button_rect.bottom)/2),
		D2D1_FIGURE_BEGIN_FILLED);
	pSink->AddLines(TriangleLeftPoints,3);
	pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
	pSink->Close();
	SafeRelease(&pSink);
	m_pRenderTarget->FillGeometry(TriangleLeftGeometry, m_pArrowColor);
	m_pRenderTarget->DrawRectangle(m_left_button_rect ,m_pArrowColor);

}

// Display Information Pane
void CPulseScope::DisplayWaveInfo() 
{	
	if (!m_InfoText)
		return;

	FLOAT  info_left = m_left_button_rect.right+5;
	FLOAT  info_right = m_right_button_rect.left-5;
	FLOAT  info_top = m_right_button_rect.top;
 	FLOAT  info_bottom = m_right_button_rect.bottom;

	D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
	m_info_pane_rect = D2D1::RectF(info_left, info_top, info_right, info_bottom);

	// Draw a rectangle.
	m_pRenderTarget->DrawRectangle(&m_info_pane_rect, m_pCornflowerBlueBrush);

	// Write text 
	WCHAR textBuffer[500] = { 0 };
	UINT tMaxLen = sizeof(textBuffer) / sizeof(WCHAR);
	StringCchPrintf(textBuffer, tMaxLen, m_InfoText);

	m_pRenderTarget->DrawText(
		textBuffer,
		static_cast<UINT>(ARRAYSIZE(textBuffer)),
		m_pBtnTextFormat,
		m_info_pane_rect,
		m_pCornflowerBlueBrush,
		D2D1_DRAW_TEXT_OPTIONS_NONE | D2D1_DRAW_TEXT_OPTIONS_CLIP
		);

}

////////////////// General functions ////////////////////////////

PULSESCOPE_API CPulseScope * InitPulseScope(HWND hWndParent)
// Initialize Pulse Scope object and return pointer to object
{
	static CPulseScope * PulseScopeObj = NULL;

	if (PulseScopeObj)
		ShowWindow(PulseScopeObj->m_hwnd,SW_SHOW);
	else
	{
		PulseScopeObj = new  CPulseScope;
		PulseScopeObj->Initialize(hWndParent);
	}
	return PulseScopeObj;
}

PULSESCOPE_API void DeletePulseScope(CPulseScope * obj)
{
	ShowWindow(obj->m_hwnd,SW_HIDE);
	//delete obj;
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

bool inRect(int x, int y, D2D1_RECT_F rect)
{
	if (rect.bottom >=y && rect.top<=y && rect.left<=x && rect.right>=x)
		return true;
	else
		return false;
}
