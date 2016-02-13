#pragma once
/////// Log related definitions ///////
// Severity
#define	INFO	0
#define	WARN	1
#define	ERR		2
#define	FATAL	3

class SppLog
{
public:
	SppLog(void);
	SppLog(HINSTANCE hInstance, HWND	ConsoleWnd);
	virtual ~SppLog(void);
	void Show(void);
	void Hide(void);
	HWND GetWndHandle(void);
	void LogAudioUnit(int Code, int source, int Severity, LPVOID Data=NULL);


private:
	HWND m_hLogDlg;
	HINSTANCE m_hInstance;
};

