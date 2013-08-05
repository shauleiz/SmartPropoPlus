#pragma once

#define ROWSPACE 25			// Space between rows
#define COLSPACE 130		// Space between columns
#define ID_BASE_STATIC	123432
#define ID_BASE_CH		133432

class SppBtnsDlg
{
public:
	SppBtnsDlg(void);
	SppBtnsDlg(HINSTANCE hInstance, HWND	ConsoleWnd);
	virtual ~SppBtnsDlg(void);
	HWND GetHandle(void);
	void SetButtonsMappingData(array<BYTE, 128>* aButtonMap, UINT nButtons);
	void SendButtonsMappingData(void);

private:
	HWND CreateStatics(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption);
	HWND CreateEdit(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption);
	void CreateControls(UINT nButtons);
	void CreateButtonLable(UINT iButton);
	void CreateChannelEdit(UINT iButton);
	int	 CreateBtnMap(array<BYTE,MAX_BUTTONS>& BtnMap);

private:
	HWND		m_hDlg;
	HINSTANCE	m_hInstance;
	HWND		m_ParentWnd;
	array<HWND,MAX_BUTTONS> m_ahEdtBtn;
};

