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
	void DisplayAudioLevels(HWND hDlg, PVOID Id, UINT Left, UINT Right);
	void CleanAudioList(void);
	void InitAudioDisplay(HWND hDlg);
	void AddLine2AudioList(jack_info * jack);
	void AddLine2ModList(MOD * mod, LPCTSTR SelType);
	void SetRawChData(UINT iCh, UINT data);
	void SetNumberRawCh(UINT nCh);
	void SetProcessedChData(UINT iCh, UINT data);
	void SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue);
	void SetJoystickDevFrame(UCHAR iDev);
	void SelChanged(WORD ListBoxId, HWND hListBox);
	void CfgJoyMonitor(HWND);
	void SetPulseScope(bool cb);
	void MonitorCh(bool cb);
	void MonitorRawCh(WORD cb);
	void MonitorPrcCh(WORD cb);
	void PulseScope(WORD cb);
	void ShowLogWindow(WORD cb);
	void ShowButtonMapWindow(void);
	void RecordInSignal(WORD cb);
	void RecordPulse(WORD cb);
	void vJoyMapping(void);
	void SetMappingData(Mapping * Map);
	void SetButtonsMappingData(BTNArr* aButtonMap, UINT nButtons);
	void SendMappingData(BTNArr* aButtonMap, UINT nButtons);
	void InitButtonMap(HWND);
	void AudioChannelParams(void);
	void AutoParams(WORD ctrl);
	void AudioChannelParams(UINT Bitrate, WCHAR Channel);
	void AudioAutoParams(WORD, WORD);
	void vJoyDevAdd(UINT id);
	void vJoyDevSelect(UINT id);
	void vJoyRemoveAll();
	void vJoySelected(HWND hCb);
	void CreateBtnsDlg(HWND hDlg);
	void EnableControls(UINT id, controls * ctrl);
	void SetStreamingButton(BOOL isProcessingAudio);
	//void FilterListEvent(WPARAM wParam, LPARAM lParam);
	void AddLine2FilterListA(int iFilter, const char * FilterName);
	void UpdateFilter(void);
	void InitFilterDisplay(HWND);
	void OnFilterFileBrowse(void);
	void InitFilter(int nFilters, LPTSTR FilterName);
	void SelFilter(int FilterId);
	void EnableFilter(BOOL enable);
	void OnStreamStopStart(void);
	void ExitWithOK(bool);




private:
	bool TaskBarAddIcon(UINT uID, LPTSTR lpszTip);
	void ClearChDisplay(UINT FirstChBar, UINT LastChBar, DWORD Color);
	int FindItemById(HWND hListView, LPCTSTR Id);

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

