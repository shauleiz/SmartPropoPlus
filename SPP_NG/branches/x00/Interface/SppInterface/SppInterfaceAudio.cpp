#include "StdAfx.h"
#include "SppInterfaceAudio.h"

CSppInterfaceAudio::CSppInterfaceAudio(void)
{
}

CSppInterfaceAudio::CSppInterfaceAudio(ID2D1HwndRenderTarget* pRenderTarget) : CBaseUnit(pRenderTarget)
{
	m_Jacks.clear();
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
	m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color), &(j->m_pRectFillColor));
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

void CSppInterfaceAudio::Display(float left, float top, float right, float bottom)
{
	CBaseUnit::Display(left,  top,  right,  bottom);
	RenderJacks();
}

int CSppInterfaceAudio::RenderJacks(void)
{
	// Get the frame of the audio unit
	D2D1_RECT_F  rect = m_roundedRect.rect;
	D2D1_RECT_F  textrect;

	// Leave a margin at the top and at the bottom and split between all jacks
	float margin_top = rect.top + (rect.bottom-rect.top)/20.f;
	float margin_bottom = rect.bottom - (rect.bottom-rect.top)/20.f;

	// Calculate the number and size of slots (jacks)
	// If none then print warning
	int	n_slots = m_Jacks.size();
	if (!n_slots)
	{
		// Print warning 
		textrect.bottom = (rect.bottom + rect.top + rect.right - rect.left)/2;
		textrect.top =    (rect.bottom + rect.top - rect.right + rect.left)/2;
		textrect.right =  (rect.right + rect.left + rect.bottom - rect.top)/2;
		textrect.left =   (rect.right + rect.left - rect.bottom + rect.top)/2;
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(
            90.0f,
			D2D1::Point2F(((rect.left+rect.right)/2), (rect.top+rect.bottom)/2))
        );
		m_pRenderTarget->DrawText(
			L"No inputs",
			ARRAYSIZE(L"No inputs") - 1,
			m_pTextFormat,
			&textrect,
			m_pRectLineColor
			);

		m_pRenderTarget->DrawRectangle(&textrect, m_pRectLineColor);
		m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		return 0;
	};

	// Render jack slots one by one
	float slot_size = (margin_bottom-margin_top)/n_slots;
	for (int i=0 ; i<n_slots; i++)
	{
		D2D1_RECT_F slot_rect = D2D1::RectF(rect.left+2, margin_top+i*slot_size-2, rect.right-2,margin_top+(i+1)*slot_size+2);
		m_pRenderTarget->DrawRectangle(&slot_rect, m_Jacks[i]->m_pRectLineColor);
		m_pRenderTarget->FillRectangle(&slot_rect, m_Jacks[i]->m_pRectFillColor);
	};

	return n_slots;
}
