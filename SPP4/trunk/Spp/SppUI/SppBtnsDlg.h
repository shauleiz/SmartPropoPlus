#pragma once

#define ROWSPACE 25			// Space between rows
#define COLSPACE 130		// Space between columns
#define ID_BASE_STATIC		123332
#define ID_BASE_CH			133432
#define ID_BASE_GREENDOT	143432
#define ID_BASE_REDDOT		153432

class SppBtnsDlg
{
public:
	SppBtnsDlg(void);
	SppBtnsDlg(HINSTANCE hInstance, HWND	ConsoleWnd);
	virtual ~SppBtnsDlg(void);
	HWND GetHandle(void);
	void SetButtonsMappingData(array<BYTE, MAX_BUTTONS>* aButtonMap, UINT nButtons);
	void SendButtonsMappingData(void);
	void Show(void);
	void EnableControls(UINT id, controls * ctrl);
	void SetButtonValues(UINT id, BTNArr * BtnVals);

private:
	HWND CreateStatics(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption);
	HWND CreateEdit(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption);
	void CreateControls(UINT nButtons);
	void CreateButtonLable(UINT iButton);
	void CreateChannelEdit(UINT iButton);
	void CreateIndicator(UINT iButton);
	int	 CreateBtnMap(array<BYTE,MAX_BUTTONS>& BtnMap);

private:
	HWND		m_hDlg;
	HINSTANCE	m_hInstance;
	HWND		m_ParentWnd;
	array<HWND,MAX_BUTTONS> m_ahEdtBtn;
	BTNArr		m_BtlVals;
};

