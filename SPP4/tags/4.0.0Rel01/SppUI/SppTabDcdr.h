#pragma once
#include "spptab.h"

class SppTabDcdr :
	public SppTab
{
public:
	virtual ~SppTabDcdr(void);
	SppTabDcdr(void);
	SppTabDcdr(HINSTANCE hInstance, HWND TopDlgWnd);

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
	void Init(HWND hDlg);
	void SetDecoderQuality(UINT Quality);
	void UpdateToolTip(LPVOID param);
	void Reset();
};

