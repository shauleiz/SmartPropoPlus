#pragma once
#include "spptab.h"
class SppTabGen :
	public SppTab
{
public:
	SppTabGen(void);
	SppTabGen(HINSTANCE hInstance, HWND TopDlgWnd);
	virtual ~SppTabGen(void);
};

