#pragma once
class SppBtnsDlg;

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
	void AddLine2ModList(MOD * mod, LPCTSTR SelType);
	void SetRawChData(UINT iCh, UINT data);
	void SetProcessedChData(UINT iCh, UINT data);
	void SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SetJoystickDevFrame(UCHAR iDev);
	void SelChanged(WORD ListBoxId, HWND hListBox);
	void CfgJoyMonitor(HWND);
	void MonitorCh(bool cb);
	void MonitorRawCh(WORD cb);
	void MonitorPrcCh(WORD cb);
	void ShowLogWindow(WORD cb);
	void RecordInSignal(WORD cb);
	void RecordPulse(WORD cb);
	void vJoyMapping(void);
	void SetAxesMappingData(DWORD Map, UINT nAxes);
	void SetButtonsMappingData(array<BYTE, 128>* aButtonMap, UINT nButtons);
	void SendButtonsMappingData(array<BYTE, 128>* aButtonMap, UINT nButtons);
	void InitButtonMap(HWND);
	void AudioChannelParams(void);
	void AudioChannelParams(UINT Bitrate, WCHAR Channel);
	void vJoyDevAdd(UINT id);
	void vJoyDevSelect(UINT id);
	void vJoyRemoveAll();
	void vJoySelected(HWND hCb);
	void CreateBtnsDlg(HWND hDlg);

	//void FilterListEvent(WPARAM wParam, LPARAM lParam);
	void AddLine2FilterListA(int iFilter, const char * FilterName);
	void UpdateFilter(void);
	void InitFilterDisplay(HWND);
	void OnFilterFileBrowse(void);
	void InitFilter(int nFilters, LPTSTR FilterName);
	void SelFilter(int FilterId);
	void EnableFilter(BOOL enable);




private:
	bool TaskBarAddIcon(UINT uID, LPTSTR lpszTip);
	void ClearChDisplay(UINT FirstChBar, UINT LastChBar, DWORD Color);

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
	SppBtnsDlg * m_BtnsDlg;
};

