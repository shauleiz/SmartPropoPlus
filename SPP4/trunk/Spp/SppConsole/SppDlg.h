#pragma once
class SppDlg
{
public:
	SppDlg(void);
	SppDlg(HINSTANCE hInstance, HWND	ConsoleWnd);
	virtual ~SppDlg(void);
	void Show();
	void Hide();
	void RegisterEndEvent(HANDLE * h);
	bool MsgLoop(void);
	HWND GetHandle(void);

public: // Called from window procedure
	void CleanAudioList(void);
	void AddLine2AudioList(jack_info * jack);
	void AddLine2ModList(MOD_STRUCT * mod);
	void SetRawChData(UINT iCh, UINT data);
	void SetProcessedChData(UINT iCh, UINT data);
	void SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SelChanged(WORD ListBoxId, HWND hListBox);
	void MonitorRawCh(WORD cb);
	void CfgJoyMonitor(HWND);
	void MonitorPrcCh(WORD cb);
	void ShowLogWindow(WORD cb);
	void RecordInSignal(WORD cb);
	void RecordPulse(WORD cb);
	void FilterListEvent(WPARAM wParam, LPARAM lParam);
	void AddLine2FilterListA(int iFilter, const char * FilterName);
	void UpdateFilter(void);


private:
	bool TaskBarAddIcon(UINT uID, LPTSTR lpszTip);
	//DWORD WINAPI  StartDlg(LPVOID hInstance);

private:
	MSG m_msg;
	HACCEL m_hAccelTable;
	HWND m_hDlg;
	HICON m_hIcon;
	HINSTANCE m_hInstance;
	NOTIFYICONDATA m_tnid;
	HANDLE	m_hEndEvent;
	HANDLE	m_hThread;
	HWND	m_ConsoleWnd;
};

