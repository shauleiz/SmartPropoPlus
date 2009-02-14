// SppConsoleDlg.h : header file
//

#if !defined(AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_)
#define AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DynamicLED.h"
#include "AudioInput.h"
#include "Wave2Joystick.h"
#include "FilterIf.h"
#include "afxcmn.h"

typedef HRESULT (CALLBACK* DLLFUNC1)();
typedef HRESULT (CALLBACK* DLLFUNC2)(int);

#define     NOTIFYICON_VERSION 3
#define NOTIFYICONDATAA_V1_SIZE     FIELD_OFFSET(NOTIFYICONDATAA, szTip[64])
#define NOTIFYICONDATAW_V1_SIZE     FIELD_OFFSET(NOTIFYICONDATAW, szTip[64])
#ifdef UNICODE
#define NOTIFYICONDATA_V1_SIZE      NOTIFYICONDATAW_V1_SIZE
#else
#define NOTIFYICONDATA_V1_SIZE      NOTIFYICONDATAA_V1_SIZE
#endif

#define NOTIFYICONDATAA_V2_SIZE     FIELD_OFFSET(NOTIFYICONDATAA, guidItem)
#define NOTIFYICONDATAW_V2_SIZE     FIELD_OFFSET(NOTIFYICONDATAW, guidItem)
#ifdef UNICODE
#define NOTIFYICONDATA_V2_SIZE      NOTIFYICONDATAW_V2_SIZE
#else
#define NOTIFYICONDATA_V2_SIZE      NOTIFYICONDATAA_V2_SIZE
#endif

#if (_WIN32_IE >= 0x0500)
#define NIF_STATE       0x00000008
#define NIF_INFO        0x00000010
#endif
#if (_WIN32_IE >= 0x600)
#define NIF_GUID        0x00000020
#endif

#if (_WIN32_IE >= 0x0500)
#define NIM_SETFOCUS    0x00000003
#define NIM_SETVERSION  0x00000004
#define     NOTIFYICON_VERSION 3
#endif

#if (_WIN32_IE >= 0x0500)
#define NIS_HIDDEN              0x00000001
#define NIS_SHAREDICON          0x00000002
#endif

// Notify Icon Infotip flags
#define NIIF_NONE       0x00000000
// icon flags are mutually exclusive
// and take only the lowest 2 bits
#define NIIF_INFO       0x00000001
#define NIIF_WARNING    0x00000002
#define NIIF_ERROR      0x00000003
#define NIIF_ICON_MASK  0x0000000F
#if (_WIN32_IE >= 0x0501)
#define NIIF_NOSOUND    0x00000010
#endif

// Timer ideces
#define TIMER_PPJ		1
#define TIMER_LEVEL		2
#define TIMER_FMS_STAT	3
#define TIMER_JOY		4

// Additional
#define	TOGGLE		-1
#define	DONT_CHANGE	-2

#define N_JOY_BARS	10

#define FMS_STAT_STR	{	"Connected: Data flow is OK", \
							"Connected: Data flow is slow", \
							"Connected: No data", \
							"Connected: On Hold", \
							"Connected: Data problem"\
						}
/////////////////////////////////////////////////////////////////////////////
// CSppConsoleDlg dialog

class CSppConsoleDlg : public CDialog
{
	
typedef struct _NOTIFYICONDATAA_V6 {
        DWORD cbSize;
        HWND hWnd;
        UINT uID;
        UINT uFlags;
        UINT uCallbackMessage;
        HICON hIcon;
#if (_WIN32_IE < 0x0500)
        CHAR   szTip[64];
#else
        CHAR   szTip[128];
#endif
#if (_WIN32_IE >= 0x0500)
        DWORD dwState;
        DWORD dwStateMask;
        CHAR   szInfo[256];
        union {
            UINT  uTimeout;
            UINT  uVersion;
        } DUMMYUNIONNAME;
        CHAR   szInfoTitle[64];
        DWORD dwInfoFlags;
#endif
#if (_WIN32_IE >= 0x600)
        GUID guidItem;
#endif
} NOTIFYICONDATA_V6, *PNOTIFYICONDATA_V6;
// Construction
public:
	CString GetStatusString(void);
	void StartIconified(bool iconified);
	CSppConsoleDlg(CWnd* pParent = NULL);	// standard constructor
	CWnd * GetMixerDeviceCtrl();
	CWnd * GetCheckAudioCtrl();
	CWnd * GetAudioLinesCtrl();
	CWnd * GetModeTypeCtrl();
	CWnd * GetShiftAutoCtrl();
	CWnd * GetShiftPosCtrl();
	void SomethingChanged();
	int GetAudioLevel();
	int GetNumJoystickPos();

// Dialog Data
	//{{AFX_DATA(CSppConsoleDlg)
	enum { IDD = IDD_SPPCONSOLE_DIALOG };
	CStatic	m_spk;
	BOOL	m_enable_audio;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSppConsoleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnFilterSelect(UINT nID);
	void UpdateFilterMenu(void);
	void UpdateFmsItem(void);
	void PpJoyExShowCtrl(void);
	int GetCurrentMixerDevice();
	void HearTx(bool hear=TRUE);
	void CheckAudioCB(void);
	void SetCurrentInputLine(int iLine);
	void SetCurrentMixerDevice(unsigned int iMixer);
	bool GetCurrentInputLineFromSystem(unsigned int * iLine);
	bool GetSavedCurrentInputLine(unsigned int * iLine);
	int GetCurrentInputLine(void);
	void EnableAudio(int enable=1);
	void PopulateInputLines(void);
	void PopulateAudioSource();
	BOOL TaskBarIconToolTip(UINT uID, const char *ToolTipText);
	bool isDllActive(void);
	void InitJoyPosBars(void);
	void SetJoyPosBars(int * position);
	afx_msg LRESULT OnInterSppConsole(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInterSppApps(WPARAM wParam, LPARAM lParam);
	void ShowTrayIconMenu(UINT uID);
	bool m_Iconified;
	BOOL TaskBarIconBaloon(UINT uID, const char *BaloonText, const char *BaloonTitle=NULL);
	BOOL TaskBarDelIcon(UINT uID);
	BOOL TaskBarAddIcon(UINT uID, HICON hicon, LPSTR lpszTip);
	void ShowShiftRB(bool show=true);
	int PopulateModulation();
	HICON m_hIcon;
	afx_msg LRESULT OnStatusAreaMessage(WPARAM wParam, LPARAM lParam);
	BOOL OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	int GetFmsStatusString(CString &StatStr);
	CAudioInput * m_AudioInput;
	HANDLE m_hMuxex;
	HANDLE m_hConsoleMuxex; // Handle to the mutext that ensures that this application is a singleton
	CWnd * m_pSpkPic;
	int m_iSelLine; // Index of the selected line
	int m_iSelMixer; // Index of the selected mixer device
	long m_PpJoyExVer;	// Version of PPJoyEx.dll file
	bool m_JsChPostProcEvail;
	int m_iSelFilter;	// index of selected filter
	HMENU m_hFilterMenu; // Handle to the filter menu
	CMenu * m_pMainMenu;	// Pointer to the main menu
	CWave2JoystickApp *  m_Wave2Joystick; // Object representing the audio system
	CFilterIf * m_pFilters; // Object representing JS post processing filter
	//bool m_WaveRecording; // True if recording of the input wave.
	bool m_AntiJitter;	// True if anti-jitter mechanism is on
	void InitJitterStatus(void); // Initialize m_AntiJitter and the menu check mark according to registry
	void UpdateJitterStatus(int Stat, BOOL Enable=TRUE); // Set check-mark and m_AntiJitter
	void SetWaveInParams(void); // Override default WaveIn parameters (Override values from the registry)
	void UpdateFmsMenuItem(int enable=1); // Enable/Disable FMS item in File menu
	class CWaveRec * m_RecWaveDialog; // Wave recorder dialog box
	class CLogAudioHdrs * m_LogAudioHdrsDialog; // Audio header logger dialog box
	class CLogPulse * m_LogPulseDialog; // Pulse logger dialog box
	class CLogFmsConn * m_LogFmsConnDialog; // FMS connection logger dialog box
	class CLogRawPulse * m_LogRawPulseDialog; // Raw pulses as recorded inside ProcessPulseXXX()
	FILE * m_pFmsConnLogFile; // FMS connection logger output file 
	LOGSTAT	m_FmsConnLogStat; // State of connection logger state machine
	FILE * m_pRawPulseLogFile; // Raw pulses logger output file 
	LOGSTAT	m_RawPulseLogStat; // State of Raw pulses logger state machine
	class CDynamicLED m_LedValidPos;
	CWnd * m_pScanInputs;

	// Generated message map functions
	//{{AFX_MSG(CSppConsoleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeModType();
	afx_msg void OnShiftAuto();
	afx_msg void OnShiftPos();
	afx_msg void OnShiftNeg();
	afx_msg void OnHide();
	afx_msg void OnSelchangeAudioSrc();
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	virtual void OnOK();
	afx_msg void OnSelchangeMixerdevice();
	afx_msg void OnEnableAudio();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSelSrc();
	afx_msg void OnAudioDst();
	afx_msg void OnAudioVol();
	afx_msg void OnAudioPrint();
	afx_msg void OnPpjoyex();
	afx_msg void OnWaveRec();
	//}}AFX_MSG
	virtual void OnCancel();
	afx_msg void OnShow();
	//afx_msg void OnUpdateWaveRec(CCmdUI *pCmdUI);
	DECLARE_MESSAGE_MAP()
private:
	LRESULT OnHideMsg(WPARAM wParam , LPARAM lParam);
	LRESULT OnMixerLineChange(WPARAM wParam , LPARAM lParam);
	LRESULT OnMixerControlChange(WPARAM wParam , LPARAM lParam);
	LRESULT OnWaveRecStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnWaveRecStop(WPARAM wParam, LPARAM lParam);
	LRESULT OnWaveRecCancel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLogAudioHdrsCancel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLogPulseCancel(WPARAM wParam, LPARAM lParam);
	LRESULT OnFmsConnStart(WPARAM wParam, LPARAM lParam);
	LRESULT OnFmsConnStop(WPARAM wParam, LPARAM lParam);
	LRESULT OnFmsConnCancel(WPARAM wParam, LPARAM lParam);
	//LRESULT OnRawPulseStart(WPARAM wParam, LPARAM lParam);
	//LRESULT OnRawPulseStop(WPARAM wParam, LPARAM lParam);
	LRESULT OnRawPulseCancel(WPARAM wParam, LPARAM lParam);

protected:
	// Sets the initial position of the dialog box
	int LocateDialogWindow(void);
	int StoreLocationDialogWindow(void);
	// Timer number 1: Ticks every second
	UINT_PTR m_Timer1;
	UINT_PTR m_TimerLevel;
	UINT_PTR m_TimerFmsStat;
	UINT_PTR m_TimerJoystick;
	int m_AudioLevel;							// Audio level (volume) of the selected device
	int m_JoystickPos[MAX_JS_CH+1];				// Array of joystick positions
	int m_FmsLatency;
	int m_TimestampFromDll;
	int m_TimestampToDll;
public:
	afx_msg void OnTimer(UINT nIDEvent);
private:
	// Progress bar used to display audio level
	CProgressCtrl m_AudioLevelBar;
	CProgressCtrl m_JoyPosBar[N_JOY_BARS];
public:
	afx_msg void OnAdvancedJitter();
	afx_msg void OnRunFms();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnLogAudioHdrs();
	afx_msg void OnLogPulse();
	afx_msg void OnLogFmsConn();
	afx_msg void OnLogRawPulse();
	afx_msg void OnBnClickedScandlg();
	afx_msg void OnEnterIdle(UINT nWhy, CWnd* pWho);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_)
