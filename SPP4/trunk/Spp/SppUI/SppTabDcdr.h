#pragma once
#include "spptab.h"

class SppTabDcdr :
	public SppTab
{
public:
	SppTabDcdr(void);
	SppTabDcdr(HINSTANCE hInstance, HWND TopDlgWnd);
	virtual ~SppTabDcdr(void);

public:
	void SetRawChData(UINT iCh, UINT data);
	void MonitorCh(HWND hDlg);
	void AddLine2DcdrList(MOD * mod, LPCTSTR SelType);
	void SelChanged(WORD ListBoxId, HWND hListBox);
	void SelectDecoder(LPCTSTR Decoder);
	void DecoderAuto(bool automode);
	void AutoDecParams(void);
	void ScanEncoding(void);
	void SetNumberRawCh(UINT nCh);
};

