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
	bool RemoveAllJack(void);
	void Display(float left, float top, float right, float bottom);
	void SetSelected(LPCWSTR id);
	void Message2ui(DWORD msg, PVOID payload);
	void MouseLeftBtnDown(int x, int y);

private:
	int RenderJacks(void);
	void DisplayFriendlyName(LPCWSTR Msg, D2D1_POINT_2F jack_hole, ID2D1SolidColorBrush* pTextBrush = 0);


private:
	std::vector<Jack *> m_Jacks;
	WCHAR * selected_id;
};
