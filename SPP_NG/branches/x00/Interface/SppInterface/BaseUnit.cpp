#include "StdAfx.h"
#include "BaseUnit.h"

CBaseUnit::CBaseUnit(void) 
// Not used;
{
}

// Initialize default values and get the Render Target
CBaseUnit::CBaseUnit(ID2D1HwndRenderTarget* pRenderTarget) : 
	m_margin(20.0f), m_rect_color(D2D1::ColorF::LightSlateGray), m_line_color(D2D1::ColorF::Red), m_line_width(2.f),
	m_opacity_sel(1.0f), m_opacity_nosel(0.6f), m_pRenderTarget(NULL), m_selected(false), m_pDWriteFactory(NULL), m_pTextFormat(NULL)
{
	m_pRenderTarget = pRenderTarget;
}

CBaseUnit::~CBaseUnit(void)
{
	SafeRelease(&m_pRectFillColor);
	SafeRelease(&m_pRectLineColor);
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

		// Create text formats
		static const WCHAR msc_fontName[] = L"Verdana";
		static const FLOAT msc_fontSize = 50;

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

	m_pRectFillColor->SetOpacity(m_opacity_nosel);
	m_pRectLineColor->SetOpacity(m_opacity_nosel);
	m_pRenderTarget->DrawRoundedRectangle(m_roundedRect, m_pRectLineColor, 2.f);
	m_pRenderTarget->FillRoundedRectangle(m_roundedRect, m_pRectFillColor);
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
