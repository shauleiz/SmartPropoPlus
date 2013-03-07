#pragma once
#include "baseunit.h"

struct Jack
{
	WCHAR * id;
	COLORREF color;
	ID2D1SolidColorBrush* m_pRectLineColor;
	ID2D1SolidColorBrush* m_pRectFillColor;
	WCHAR * FriendlyName;
};

class CSppInterfaceAudio :
	public CBaseUnit
{
public:
	CSppInterfaceAudio(void);
	CSppInterfaceAudio(ID2D1HwndRenderTarget* pRenderTarget);
	virtual ~CSppInterfaceAudio(void);
	bool AddJack(LPCWSTR id, LPCWSTR FriendlyName, COLORREF color=0);
	bool RemoveJack(LPCWSTR id);
	void Display(float left, float top, float right, float bottom);

private:
	int RenderJacks(void);

private:
	std::vector<Jack *> m_Jacks;
};
