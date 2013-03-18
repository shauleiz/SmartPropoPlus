#include "StdAfx.h"
#include "BaseUnit.h"

CBaseUnit::CBaseUnit(void) 
// Not used;
{
}

// Initialize default values and get the Render Target
CBaseUnit::CBaseUnit(ID2D1HwndRenderTarget* pRenderTarget) : 
	m_margin(20.0f), m_rect_color(D2D1::ColorF::LightSlateGray), m_line_color(D2D1::ColorF::Red), m_line_width(2.f),
	m_opacity_sel(1.0f), m_opacity_nosel(0.6f), m_pRenderTarget(NULL), m_selected(false), m_pDWriteFactory(NULL), 
	m_pTextFormat(NULL), m_pBeigeBrush(NULL), m_pBlackBrush(NULL), m_pTextFormatInfo(NULL), m_header_text(L"Your text"),
	m_pPinionBitmap(NULL)
{
	m_pRenderTarget = pRenderTarget;
}

CBaseUnit::~CBaseUnit(void)
{
	SafeRelease(&m_pRectFillColor);
	SafeRelease(&m_pRectLineColor);
	SafeRelease(&m_pBeigeBrush);
	SafeRelease(&m_pBlackBrush);
	SafeRelease(&m_pHeaderTextColor);
	SafeRelease(&m_pWICFactory);
	SafeRelease(&m_pPinionBitmap);

}

void CBaseUnit::Initialize(DWORD rect_color, DWORD line_color)
{
	// Initiallization of the device resources  
	if (m_pRenderTarget)
	{
		// Create a fill brush - unselected.
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rect_color), &m_pRectFillColor);
		m_pRectFillColor->SetOpacity(m_opacity_nosel);
		// Create a line brush - unselected.
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(line_color), &m_pRectLineColor);
		m_pRectLineColor->SetOpacity(m_opacity_nosel);
		// Create other brushes
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Beige, 1.0f),&m_pBeigeBrush);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 1.0f),&m_pBlackBrush);
		m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlueViolet, 1.0f),&m_pHeaderTextColor);


		// Create text formats
		static const WCHAR msc_fontName[] = L"Verdana";
		static const FLOAT msc_fontSize = 50;
		static const FLOAT msc_header_fontSize = 20;

		HRESULT hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
			);

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
			// Center the text horizontally and vertically.
			m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			
		}

		if (SUCCEEDED(hr))
		{
			hr = m_pDWriteFactory->CreateTextFormat(
				msc_fontName,
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				static_cast<FLOAT>(msc_header_fontSize),
				L"", // locale
				&m_pHeaderTextFormat
				);
		}
		if (SUCCEEDED(hr))
		{
			// Center the text vertically.
			// Text to the left
			m_pHeaderTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			m_pHeaderTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			
		}


		if (SUCCEEDED(hr))
		{
			hr = m_pDWriteFactory->CreateTextFormat(
				msc_fontName,
				NULL,
				DWRITE_FONT_WEIGHT_NORMAL,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				static_cast<FLOAT>(msc_fontSize/5),
				L"", // locale
				&m_pTextFormatInfo
				);
		}

		if (SUCCEEDED(hr))
		{
			// Center the text horizontally and vertically.
			m_pTextFormatInfo->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			m_pTextFormatInfo->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		}

		if (SUCCEEDED(hr))
		{
			// Create WIC factory.
			hr = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory,
				reinterpret_cast<void **>(&m_pWICFactory)
				);
		}

		if (SUCCEEDED(hr))
		{
			hr = LoadResourceBitmap(
				m_pRenderTarget,
				m_pWICFactory,
				L"IDB_PINION",
				L"Image",
				30,
				0,
				&m_pPinionBitmap
				);
		}

	};
}



void CBaseUnit::DisplaySelected(float left, float top, float right, float bottom)
{
	// Get location and draw the rounded rectangle with the 'selected' attributes
	m_left=left;
	m_top= top;
	m_right= right;
	m_bottom= bottom;

	// Create a rounded rectangle
	m_roundedRect = D2D1::RoundedRect( D2D1::RectF(m_left, m_top, m_right, m_bottom), 10.f, 10.f);

	m_pRectFillColor->SetOpacity(m_opacity_sel);
	m_pRectLineColor->SetOpacity(m_opacity_sel);
	m_pRenderTarget->DrawRoundedRectangle(m_roundedRect, m_pRectLineColor, 2.f);
	m_pRenderTarget->FillRoundedRectangle(m_roundedRect, m_pRectFillColor);

	// Create header
	DisplayHeader();
}


void CBaseUnit::DisplayNotSelected(float left, float top, float right, float bottom)
{
	// Get location and draw the rounded rectangle with the 'NOT selected' attributes
	m_left=left;
	m_top= top;
	m_right= right;
	m_bottom= bottom;

	// Create a rounded rectangle
	m_roundedRect = D2D1::RoundedRect( D2D1::RectF(m_left, m_top, m_right, m_bottom), 10.f, 10.f);
	// Create header
	DisplayHeader();

	m_pRectFillColor->SetOpacity(m_opacity_nosel);
	m_pRectLineColor->SetOpacity(m_opacity_nosel);
	m_pRenderTarget->DrawRoundedRectangle(m_roundedRect, m_pRectLineColor, 2.f);
	m_pRenderTarget->FillRoundedRectangle(m_roundedRect, m_pRectFillColor);
}

void CBaseUnit::DisplayHeader(LPCWSTR text, ID2D1SolidColorBrush * color)
// Display the header of the unit
// Default text is m_header_text
// Default color is m_pHeaderTextColor
{
	D2D1_RECT_F  textrect;
	

	// Use defaults?
	if (!text)
		text = m_header_text;
	if (!color)
		color = m_pHeaderTextColor;
		
	// Define rectangle that surrounds the text
	textrect.top	= m_roundedRect.rect.top;
	textrect.left	= m_roundedRect.rect.left + 10.f;
	textrect.right	= m_roundedRect.rect.right - 10.f;
	textrect.bottom	= m_roundedRect.rect.top + 30;

	// Text
	m_pRenderTarget->DrawText(text, wcslen( m_header_text ), m_pHeaderTextFormat, &textrect, color);

	if (m_selected)
	{
		// Draw a bitmap in the upper-right corner of the window.
		m_iconrect = textrect;
		m_iconrect.left = m_iconrect.right - (textrect.bottom - textrect.top);
		m_pRenderTarget->DrawBitmap(m_pPinionBitmap,m_iconrect);
	};


}

void CBaseUnit::Display(float left, float top, float right, float bottom)
{
	// Get location and draw the rounded rectangle with the correct attributes 
	if (m_selected)
		DisplaySelected(left, top, right, bottom);
	else
		DisplayNotSelected(left, top, right, bottom);
}

void CBaseUnit::Select(void)
{
	m_selected = true;
}

void CBaseUnit::UnSelect(void)
{
	m_selected = false;
}

D2D1_RECT_F * CBaseUnit::GetRect(void)
{
	return &m_roundedRect.rect;
}

void CBaseUnit::DisplayVerticalMessage(LPCWSTR Msg, ID2D1SolidColorBrush* pTextBrush)
{
	// Display a vertical string on the unit's rectangle
	// Msg: The text to display
	// pTextBrush: The brush to use (Default: the rectangle line color

	D2D1_RECT_F  rect = m_roundedRect.rect;
	D2D1_RECT_F  textrect;
	
	if (!pTextBrush)
		pTextBrush = m_pRectLineColor;

	// Print warning 
	textrect.bottom = (rect.bottom + rect.top + rect.right - rect.left)/2;
	textrect.top =    (rect.bottom + rect.top - rect.right + rect.left)/2;
	textrect.right =  (rect.right + rect.left + rect.bottom - rect.top)/2;
	textrect.left =   (rect.right + rect.left - rect.bottom + rect.top)/2;
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(
		90.0f, D2D1::Point2F(((rect.left+rect.right)/2), (rect.top+rect.bottom)/2)));

	m_pRenderTarget->DrawText(
		Msg,
		wcslen( Msg ),
		m_pTextFormat,
		&textrect,
		pTextBrush
		);

	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	return;
}

void CBaseUnit::Message2ui(DWORD msg, PVOID payload)
{/* Place holder*/}


//
// Creates a Direct2D bitmap from a resource in the
// application resource file.
//
HRESULT CBaseUnit::LoadResourceBitmap(
    ID2D1RenderTarget *pRenderTarget,
    IWICImagingFactory *pIWICFactory,
    PCWSTR resourceName,
    PCWSTR resourceType,
    UINT destinationWidth,
    UINT destinationHeight,
    ID2D1Bitmap **ppBitmap
    )
{
    HRESULT hr = S_OK;
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

    hr = imageResHandle ? S_OK : E_FAIL;
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
        // If a new width or height was specified, create an
        // IWICBitmapScaler and use it to resize the image.
        if (destinationWidth != 0 || destinationHeight != 0)
        {
            UINT originalWidth, originalHeight;
            hr = pSource->GetSize(&originalWidth, &originalHeight);
            if (SUCCEEDED(hr))
            {
                if (destinationWidth == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                    destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (destinationHeight == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                    destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }

                hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                if (SUCCEEDED(hr))
                {
                    hr = pScaler->Initialize(
                            pSource,
                            destinationWidth,
                            destinationHeight,
                            WICBitmapInterpolationModeCubic
                            );
                    if (SUCCEEDED(hr))
                    {
                        hr = pConverter->Initialize(
                            pScaler,
                            GUID_WICPixelFormat32bppPBGRA,
                            WICBitmapDitherTypeNone,
                            NULL,
                            0.f,
                            WICBitmapPaletteTypeMedianCut
                            );
                    }
                }
            }
        }
        else
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
