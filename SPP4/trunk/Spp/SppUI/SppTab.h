#pragma once
#include <vector>
#include "Commctrl.h"


class SppTab
{
public:
	SppTab(void);
	SppTab(HINSTANCE hInstance, HWND TopDlgWnd, int Id, DLGPROC MsgHndlDlg);
	virtual ~SppTab(void);
	void Show();
	void Hide();
	HWND GetHandle(void);
	int  GetId(void);
	int  GetTabText(LPTSTR Text, int TextSize);
	void SetPosition(HWND hDlg);
	HWND CreateToolTip(HWND hDlg, const int arr[], int size=0);
	virtual void UpdateToolTip(LPVOID);
	virtual void Reset();


protected:
	void InitBars(HWND hDlg, const DWORD Color, std::vector< int> vBars,ULONG max=0x03ff0000,ULONG min=0);
	void ShowArrayOfItems(HWND hDlg, int nCmdShow, const int items[], UINT size);
	void ShowArrayOfItems(HWND hDlg, int nCmdShow, std::vector< int> items, UINT size=0);
	void ShowArrayOfItems(HWND hDlg, bool enable, const int items[], UINT size);
	void ShowArrayOfItems(HWND hDlg, bool enable, std::vector< int>, UINT size=0);
	void ResetArrayOfBars(HWND hDlg, const int items[], UINT size);
	void ResetArrayOfBars(HWND hDlg, std::vector< int>);
	void DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, int TitleID=-1, int Icon= TTI_NONE);
	void DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, LPCTSTR TitleStr, int Icon= TTI_NONE);


protected:
	HWND m_hDlg;	// Handle to the tab dialog box
	int	m_DlgId;	// ID of the dialog box resource
	HINSTANCE m_hInstance;
	HWND m_TopDlgWnd;
	HWND m_hwndToolTip;
	std::vector< int> m_vRawBarId;
	std::vector< int> m_vPpBarId;
	std::vector< int> m_vJoyBarId;
	std::vector< int> m_vRawTitleId;
	std::vector< int> m_vPpTitleId;
	std::vector< int> m_vJoyTitleId;
	std::vector< int> m_vControls;
};

