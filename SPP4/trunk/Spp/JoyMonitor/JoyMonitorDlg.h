#pragma once

#include "WinMessages.h"

#define MAX_DISP_BUTTONS 128
#define ROWSPACE 25			// Space between rows
#define COLSPACE 60		// Space between columns
#define ID_BASE_STATIC		123332
#define ID_BASE_CH			133432
#define ID_BASE_GREENDOT	143432
#define ID_BASE_REDDOT		153432
#define ID_BASE_RING		163432



class CJoyMonitorDlg
{
public:
	CJoyMonitorDlg(void);
	CJoyMonitorDlg(HINSTANCE hInstance, HWND	ConsoleWnd);
	virtual ~CJoyMonitorDlg(void);
	HWND GetHandle(void);
	void SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SetPovValues(UCHAR iDev, UINT iPov, UINT32 PovValue);
	void InitJoyMonitor(HWND hDlg);
	void EnableControls(UINT id, controls * ctrl);
	void EnableControlsBtn(UINT id, controls * ctrl);
	void SetButtonValues(UINT id, BTNArr * BtnVals);
	void InitDevices(HWND hDlg);
	void JoystickStopped(UCHAR iDev);
	void ReportDeviceSelection(void);
	void AddDevice(int vJoyID, bool Selected);

protected:
	void CreateControls(UINT nButtons);
	void CreateButtonLable(UINT);
	void CreateChannelEdit(UINT);
	void CreateIndicator(UINT);
	HWND CreateStatics(const HWND hParent,const HINSTANCE hInst,DWORD dwStyle, const RECT& rc,const int id,const wstring& caption);
	void InitBars(HWND hDlg, const DWORD Color, std::vector<const int> vBars,ULONG max=0x03ff0000,ULONG min=0);
	void GetExistingDevices(HWND hDlg);
	void CreatePovMeters(UINT nPovs);

protected:
	HWND m_hDlg;
	HICON m_hIcon;
	HINSTANCE m_hInstance;
	HWND	m_ConsoleWnd;
	UINT m_CurJoy;
	UINT m_nRawCh;
	vector<const int> m_vJoyBarId;
	vector<const int> m_vJoyTitleId;
};

class CPovGrph : CJoyMonitorDlg
{
public:
	CPovGrph(void);
	CPovGrph(UINT iPov, LONG Radius, POINT Centre, int ID);
	void PaintRing(HWND hDlg, HINSTANCE hInst, BOOL Enabled = TRUE);
	virtual ~CPovGrph(void);

private:
	UINT  m_iPov;	// 1-based POV index (Legal values are 1-4)
	LONG  m_Radius;	// Radius of rim
	POINT m_Centre;	// Centre of rim (In client coordinates)
	BOOL  m_Valid;  // Object is valid
	int   m_ID;		// ID of the ring window
};

