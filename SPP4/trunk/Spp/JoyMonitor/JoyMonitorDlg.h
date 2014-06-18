#pragma once

#define MAX_BUTTONS 128
#define ROWSPACE 25			// Space between rows
#define COLSPACE 60		// Space between columns
#define ID_BASE_STATIC		123332
#define ID_BASE_CH			133432
#define ID_BASE_GREENDOT	143432
#define ID_BASE_REDDOT		153432

class CJoyMonitorDlg
{
public:
	CJoyMonitorDlg(void);
	CJoyMonitorDlg(HINSTANCE hInstance, HWND	ConsoleWnd);
	virtual ~CJoyMonitorDlg(void);

protected:
	void CreateControls(UINT nButtons);
	void CreateButtonLable(UINT);
	void CreateChannelEdit(UINT);
	void CreateIndicator(UINT);
	HWND CreateStatics(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption);

protected:
	HWND m_hDlg;
	HICON m_hIcon;
	HINSTANCE m_hInstance;
	HWND	m_ConsoleWnd;
};

