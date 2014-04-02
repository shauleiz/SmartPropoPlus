#pragma once
#include "spptab.h"
class SppTabFltr :
	public SppTab
{
public:
	SppTabFltr(void);
	SppTabFltr(HINSTANCE hInstance, HWND TopDlgWnd);
	virtual ~SppTabFltr(void);

public:
	void SelFilter(int FilterId);
	void InitFilter(int nFilters, LPTSTR FilterName);
	void AddLine2FilterListA(int FilterID, const char * FilterName);
	void AddLine2FilterListW(int FilterID, LPCWSTR FilterName);
	void InitFilterDisplay(HWND hDlg);
	void OnFilterFileBrowse(void);
	void EnableFilter(BOOL cb);
	void UpdateFilter(void);
};

