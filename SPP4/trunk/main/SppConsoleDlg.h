#pragma once
class SppConsoleDlg
{
public:
	SppConsoleDlg(void);
	SppConsoleDlg(HINSTANCE hInstance);
	virtual ~SppConsoleDlg(void);
	void Show();
	void Hide();

private:


private:
	MSG m_msg;
	HACCEL m_hAccelTable;
	HWND m_hDlg;
};

