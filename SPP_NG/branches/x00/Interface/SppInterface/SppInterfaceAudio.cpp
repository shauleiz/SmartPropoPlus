#include "StdAfx.h"
#include "Common.h"
#include "SppInterfaceAudio.h"

CSppInterfaceAudio::CSppInterfaceAudio(void)
{
}

CSppInterfaceAudio::CSppInterfaceAudio(ID2D1HwndRenderTarget* pRenderTarget) : CBaseUnit(pRenderTarget), selected_id(NULL)
{
	m_Jacks.clear();
	m_header_text = L"Audio Jacks";
}
CSppInterfaceAudio::~CSppInterfaceAudio(void)
{
}

bool CSppInterfaceAudio::AddJack(LPCWSTR id, LPCWSTR FriendlyName, COLORREF color)
{
	// If id exists then return false
	int size = m_Jacks.size();
	for (int i=0; i<size; i++)
	{
		if (!wcscmp(id, m_Jacks[i]->id))
			return false;
	};

	Jack * j = new(Jack);
	m_Jacks.push_back(j);
	j->id = _wcsdup(id);
	j->color = color;
	j->FriendlyName = _wcsdup(FriendlyName);
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color,0.5f), &(j->m_pRectFillColor));
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &(j->m_pRectLineColor));

	return true;
}



bool CSppInterfaceAudio::RemoveJack(LPCWSTR id)
{
	// If id exists then remove it and return true
	int size = m_Jacks.size();
	for (int i=0; i<size; i++)
	{
		if (!wcscmp(id, m_Jacks[i]->id))
		{
			delete m_Jacks[i]->FriendlyName;
			delete m_Jacks[i]->id;
			SafeRelease(&m_Jacks[i]->m_pRectFillColor);
			SafeRelease(&m_Jacks[i]->m_pRectLineColor);
			m_Jacks.erase(m_Jacks.begin()+i);
			return true;
		};
	};

	// Not found
	return false;
}

bool CSppInterfaceAudio::RemoveAllJack(void)
{
	int size = m_Jacks.size();
	if (!size)
		return false;

	for (int i=0; i<size; i++)
	{
		delete m_Jacks[i]->FriendlyName;
		delete m_Jacks[i]->id;
		SafeRelease(&m_Jacks[i]->m_pRectFillColor);
		SafeRelease(&m_Jacks[i]->m_pRectLineColor);
	};

	m_Jacks.clear();
	return true;
}

void CSppInterfaceAudio::Display(float left, float top, float right, float bottom)
{
	CBaseUnit::Display(left,  top,  right,  bottom);
	RenderJacks();
}

int CSppInterfaceAudio::RenderJacks(void)
{
	// Render an audio jack as a colored ring around a black circle.
	// The jack is placed in the middle of a colored rectangle (slot)

	// Get the frame of the audio unit
	D2D1_RECT_F  rect = m_roundedRect.rect;

	// Leave a margin at the top and at the bottom and split between all jacks
	float margin_top = rect.top + (rect.bottom-rect.top)/20.f;
	float margin_bottom = rect.bottom - (rect.bottom-rect.top)/20.f;

	// Calculate the number and size of slots (jacks)
	// If none then print warning
	int	n_slots = m_Jacks.size();
	if (!n_slots)
	{
		DisplayVerticalMessage(L"No inputs");
		return 0;
	};

	// Render jack slots one by one
	float slot_size = (margin_bottom-margin_top)/n_slots;
	for (int i=0 ; i<n_slots; i++)
	{
		// Draw a slot as a white rectange (selected with thick line)
		float width = 1.f;
		m_pRectFillColor->SetOpacity(0.3f);
		D2D1_RECT_F slot_rect = D2D1::RectF(rect.left+2, margin_top+i*slot_size+2, rect.right-2,margin_top+(i+1)*slot_size-2);
		D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect( slot_rect, 3.f, 3.f);
		m_pRenderTarget->DrawRoundedRectangle(roundedRect, m_Jacks[i]->m_pRectLineColor, width);
		if (selected_id && !wcscmp(selected_id, m_Jacks[i]->id))
		{	// Selected device
			m_pRenderTarget->FillRoundedRectangle(&roundedRect,  m_Jacks[i]->m_pRectLineColor);
			m_pRenderTarget->DrawRoundedRectangle(roundedRect,  m_pRectLineColor, width);
		}

		// Draw jack as a black hole inside a set of color rings
		D2D1_ELLIPSE ring;
		m_pRectFillColor->SetOpacity(1.0f);
		float radius = min(slot_size, (rect.left+rect.right)/2)/6;
		D2D1_POINT_2F centre = D2D1::Point2F((slot_rect.left+radius+10) , (slot_rect.bottom+slot_rect.top)/2);

		ring = D2D1::Ellipse(centre, radius, radius);
		m_pRenderTarget->FillEllipse(ring, m_Jacks[i]->m_pRectFillColor);
		m_pRenderTarget->DrawEllipse(ring, m_pBlackBrush);

		ring = D2D1::Ellipse(centre, radius+1, radius+1);
		m_pRenderTarget->DrawEllipse(ring, m_pBeigeBrush);

		ring = D2D1::Ellipse(centre, radius-5, radius-5);
		m_pRenderTarget->DrawEllipse(ring, m_pBeigeBrush);
		m_pRenderTarget->FillEllipse(ring, m_pBlackBrush);

		// Print the friendly name of the jack
		DisplayFriendlyName(m_Jacks[i]->FriendlyName, centre);

	};

	return n_slots;
}

void CSppInterfaceAudio::DisplayFriendlyName(LPCWSTR Msg, D2D1_POINT_2F jack_hole, ID2D1SolidColorBrush* pTextBrush)
{

	D2D1_RECT_F  textrect;
	
	if (!pTextBrush)
		pTextBrush = m_pBlackBrush;

	// Set delimitting rectangle
	textrect.bottom = jack_hole.y + 15;
	textrect.top =    jack_hole.y - 15;
	textrect.right =  m_roundedRect.rect.right-3;
	textrect.left =   jack_hole.x +25;

	// Trimming
	IDWriteInlineObject *inlineObject = NULL;
	m_pDWriteFactory->CreateEllipsisTrimmingSign(m_pTextFormatInfo, &inlineObject);
	DWRITE_TRIMMING trimming = 
	{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
	m_pTextFormatInfo->SetTrimming(&trimming, inlineObject);

	// Drawing text
	D2D1_POINT_2F origin = D2D1::Point2F(static_cast<FLOAT>(textrect.left), static_cast<FLOAT>(textrect.top));
	IDWriteTextLayout * textLayout;
	m_pDWriteFactory->CreateTextLayout(Msg, wcslen( Msg ), m_pTextFormatInfo, textrect.right-textrect.left, textrect.bottom - textrect.top, &textLayout);
	m_pRenderTarget->DrawTextLayout(origin, textLayout,pTextBrush);

	SafeRelease(&inlineObject);
	SafeRelease(&textLayout);

	return;
}

void CSppInterfaceAudio::SetSelected(LPCWSTR id)
{
	if (selected_id)
	{
		delete(selected_id);
		selected_id = NULL;
	}
	selected_id = _wcsdup(id);
}

void CSppInterfaceAudio::Message2ui(DWORD msg, PVOID payload)
{
	// This is the entry point for messages comming from the control unit
	// this GUI element
	
	jack_info * jack;
	switch (msg)
	{
	// Add a new jack 
	case POPULATE_JACKS:
		jack = (jack_info *)payload;
		AddJack(jack->id, jack->FriendlyName, jack->color);
		break;

	// Set the default endpoint
	case SET_DEF_JACKS:
		SetSelected((LPCWSTR)payload);
		break;

	// Remove an endpoint by its ID
	case REM_JACK:
		RemoveJack((LPCWSTR)payload);
		break;

	// Remove an endpoint by its ID
	case REM_ALL_JACK:
		RemoveAllJack();
		break;

	default:
		return;
	};
}

