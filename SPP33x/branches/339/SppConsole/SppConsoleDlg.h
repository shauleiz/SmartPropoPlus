// SppConsoleDlg.h : header file
//

#if !defined(AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_)
#define AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AudioInput.h"

typedef HRESULT (CALLBACK* DLLFUNC1)();

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
	int  SetMixerSelectionByName(const char * MixerName);
	BOOL TaskBarIconToolTip(UINT uID, const char *ToolTipText);
	bool isDllActive(void);
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
	CAudioInput * m_AudioInput;
	HANDLE m_hMuxex;
	CWnd * m_pSpkPic;
	int m_iSelLine; // Index of the selected line
	int m_iSelMixer; // Index of the selected mixer device
	long m_PpJoyExVer;	// Version of PPJoyEx.dll file
	HINSTANCE m_hPpJoyExDll; // Handle to PPJoyEx.dll
	bool m_JsChPostProcEvail;
	int m_iSelFilter;	// index of selected filter
	HMENU m_hFilterMenu; // Handle to the filter menu


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
	//}}AFX_MSG
	virtual void OnCancel();
	afx_msg void OnShow();
	DECLARE_MESSAGE_MAP()
private:
	LRESULT OnHideMsg(WPARAM wParam , LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_SPPCONSOLEDLG_H__164DAAB9_C3B2_4686_A604_0038C1E9D0F4__INCLUDED_)
