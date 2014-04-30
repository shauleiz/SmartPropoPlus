#pragma once
#include "spptab.h"

class SppBtnsDlg;

class SppTabJoy :
	public SppTab
{
public:
	SppTabJoy(void);
	SppTabJoy(HINSTANCE hInstance, HWND TopDlgWnd);
	virtual ~SppTabJoy(void);

public:
	void vJoyRemoveAll(void);
	void vJoyDevAdd(UINT id);
	void vJoyDevSelect(UINT id);
	void vJoySelected(HWND hCb);
	void EnableControls(UINT id, controls * ctrl);
	void CreateBtnsDlg(HWND hDlg);
	void SetJoystickAxisData(UCHAR iDev, UINT Axis, UINT32 AxisValue);
	void SetJoystickDevFrame(UCHAR iDev);
	void InitJoyMonitor(HWND hDlg);
	void MonitorPpCh(HWND hDlg);
	void MonitorCh(HWND hDlg);
	void SetProcessedChData(UINT iCh, UINT data);	
	void SetMappingData(Mapping * Map);
	void SetButtonsMappingData(BTNArr* aButtonMap, UINT nButtons);
	void vJoyMapping(void);
	void SendMappingData(BTNArr* aButtonMap, UINT nButtons);
	void SetJoystickBtnData(UCHAR iDev, BTNArr * BtnValue);
	void ShowButtonMapWindow(void);
	void SetNumberProcCh(UINT nCh);
	void SetNumberRawCh(UINT nCh);
	void SetRawChData(UINT iCh, UINT data);

protected:
	void UpdateChannelView(bool Raw, UINT nCh);

	SppBtnsDlg * m_BtnsDlg;
	UINT m_nProcCh;
	UINT m_nRawCh;
	UINT m_CurJoy;
};

