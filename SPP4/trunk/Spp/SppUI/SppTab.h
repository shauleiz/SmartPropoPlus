#pragma once

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

protected:
	HWND m_hDlg;	// Handle to the tab dialog box
	int	m_DlgId;	// ID of the dialog box resource
	HINSTANCE m_hInstance;
	HWND m_TopDlgWnd;

};

