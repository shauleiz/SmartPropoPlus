#pragma once

class CBaseUnit
{
public:
	CBaseUnit(void);
	CBaseUnit(ID2D1HwndRenderTarget* pRenderTarget);
	virtual ~CBaseUnit(void);
	void Initialize(DWORD rect_color = (D2D1::ColorF::LightSlateGray), DWORD line_color = (D2D1::ColorF::Red));
	void Display(float left, float top, float right, float bottom);
	void Select(void);
	void UnSelect(void);
	D2D1_RECT_F * GetRect();
	void Message2ui(DWORD msg, PVOID payload);

protected:
	ID2D1HwndRenderTarget* m_pRenderTarget;
	D2D1_ROUNDED_RECT m_roundedRect;
	ID2D1SolidColorBrush* m_pRectFillColor;
	ID2D1SolidColorBrush* m_pRectLineColor;
	ID2D1SolidColorBrush* m_pHeaderTextColor;
	ID2D1SolidColorBrush* m_pBeigeBrush;
	ID2D1SolidColorBrush* m_pBlackBrush;
	IDWriteFactory *m_pDWriteFactory;
	IDWriteTextFormat *m_pTextFormat;
	IDWriteTextFormat *m_pHeaderTextFormat;
	IDWriteTextFormat *m_pTextFormatInfo;
    IWICImagingFactory	*m_pWICFactory;
	ID2D1Bitmap			*m_pPinionBitmap;
	D2D1_RECT_F			m_iconrect;


	float m_margin;
	float m_left, m_top, m_right, m_bottom;
	DWORD m_rect_color, m_line_color;
	float m_line_width;
	float m_opacity_sel, m_opacity_nosel;
	bool m_selected;
	LPCWSTR	m_header_text;

protected:
	void DisplaySelected(float left, float top, float right, float bottom);
	void DisplayNotSelected(float left, float top, float right, float bottom);
	void DisplayVerticalMessage(LPCWSTR Msg, ID2D1SolidColorBrush* pTextBrush = NULL);
	void DisplayHeader(LPCWSTR text = NULL, ID2D1SolidColorBrush * color = NULL);
	    HRESULT LoadResourceBitmap(
        ID2D1RenderTarget *pRenderTarget,
        IWICImagingFactory *pIWICFactory,
        PCWSTR resourceName,
        PCWSTR resourceType,
        UINT destinationWidth,
        UINT destinationHeight,
        ID2D1Bitmap **ppBitmap
        );

};
