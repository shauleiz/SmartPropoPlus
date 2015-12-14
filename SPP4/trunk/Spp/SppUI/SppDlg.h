#pragma once
//
//#include "SppTabGen.h"
#include "Commctrl.h"
#include "Uxtheme.h"
#include "SppTabAudio.h"
#include "SppTabDcdr.h"
#include "SppTabFltr.h"
#include "SppTabJoy.h"
#include "SppTabAdv.h"
#pragma comment( linker, "/manifestdependency:\"type='win32' \
    name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
    processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
    language='*'\"") // TODO: This causes ugly Tabs & progress bars
#pragma comment( lib, "comctl32.lib") 

#define STOP	L"Sleep"
#define START	L"Wake-up"

class SppDlg
{

typedef struct tag_dlghdr {  // Based on http://msdn.microsoft.com/en-us/library/windows/desktop/hh298366(v=vs.85).aspx
    HWND hwndTab;       // tab control 
    SppTab * Display;   // current child dialog box 
    RECT rcDisplay;     // display rectangle for the tab control 
	SppTab /** TabGen,*/ * TabAudio, * TabDcdr, * TabFltr, * TabJoy, * TabAdvnc;
} DLGHDR; 

enum TABS_INDEX {IAUDIO=0, IDECODER, IFILTER, IJOYSTICK, IADVANCED, ILAST};

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
	void SetNumberProcCh(UINT nCh);
	void SetDecoderQuality(UINT Quality);
	void SetProcessedChData(UINT iCh, UINT data);
	void SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue);
	void ResetJoystick(void);
	void SetJoystickDevFrame(UCHAR iDev);
	void SelChanged(WORD ListBoxId, HWND hListBox);
	void CfgJoyMonitor(HWND);
	void SetPulseScope(bool cb);
	void MonitorCh(bool cb);
	void MonitorRawCh(WORD cb);
	void MonitorPrcCh(WORD cb);
	void ShowButtonMapWindow(void);
	void PulseScope(WORD cb);
	void ShowLogWindow(WORD cb);
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
	void SetStreamingState(BOOL isProcessingAudio);
	//void FilterListEvent(WPARAM wParam, LPARAM lParam);
	void AddLine2FilterListA(int iFilter, const char * FilterName);
	void AddLine2FilterListW(int iFilter, LPCWSTR FilterName);
	void UpdateFilter(void);
	void InitFilterDisplay(HWND);
	void OnFilterFileBrowse(void);
	void InitFilter(int nFilters, LPTSTR FilterName);
	void SelFilter(int FilterId);
	void EnableFilter(BOOL enable);
	void OnStreamStopStart(void);
	void ExitWithOK(bool);
	void Iconified(bool);
	void OnNotificationIcon( WPARAM wParam, LPARAM lParam);
	void SppStatusChanged( WPARAM wParam, LPARAM lParam);
	void ScanMod(void);
	void SelectDecoder(LPCTSTR);
	void SelectDecoderFailed(void);
	void AutoDecParams(void);
	void DecoderAuto(bool);
	int InitTabs(HWND);
	void OnSelChanged(HWND hDlg);
	LRESULT RelayToConsoleWnd(UINT message, WPARAM wParam, LPARAM lParam);
	LPCTSTR GetDecoderFullName(LPCTSTR Type);
	void SetFilterInfo(LPTSTR,  LPTSTR);
	HWND CreateToolTip(HWND hDlg);
	void UpdateToolTip(LPVOID);
	VOID OnPaint(void);
	void InitBackgroundImage(HWND hDlg);
	LRESULT Reset(void);

private:
	void DisplayBackgroundImage(BOOL Display);
	bool TaskBarAddIcon(UINT uID, LPTSTR lpszTip, LPTSTR lpszInfo);
	void ClearChDisplay(UINT FirstChBar, UINT LastChBar, DWORD Color);
	int FindItemById(HWND hListView, LPCTSTR Id);
	DLGTEMPLATE* DoLockDlgRes(LPCTSTR lpszResName);
	HIMAGELIST CreateTabsImageList(void);
	void DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, int TitleID=-1, int Icon= TTI_NONE);
	void DisplayToolTip(LPNMTTDISPINFO lpttt, int TxtID, LPCTSTR TitleStr, int Icon= TTI_NONE);

protected:
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
	DLGHDR m_hrsrc;
	HWND m_hwndToolTip;
	BOOL m_StreamingState;
	BOOL m_WizMinimized;
	DWORD m_color;
	HWND  m_hTarget;
};

