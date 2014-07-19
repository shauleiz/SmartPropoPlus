#pragma once

#define MAX_FILTER_TXT 100

#include "spptab.h"
class SppTabFltr :
	public SppTab
{
public:
	SppTabFltr(void);
	SppTabFltr(HINSTANCE hInstance, HWND TopDlgWnd);
	virtual ~SppTabFltr(void);

public:
	void SelFilter(int FilterId);
	void InitFilter(int nFilters, LPTSTR FilterName);
	void AddLine2FilterListA(int FilterID, const char * FilterName);
	void AddLine2FilterListW(int FilterID, LPCWSTR FilterName);
	void InitFilterDisplay(HWND hDlg);
	void OnFilterFileBrowse(void);
	void EnableFilter(int cb);
	void UpdateFilter(void);
	void SetRawChData(UINT iCh, UINT data);
	void SetProcessedChData(UINT iCh, UINT data);
	void MonitorCh(HWND hDlg);
	void MonitorPpCh(HWND hDlg);
	void SetNumberProcCh(UINT nCh);
	void UpdateToolTip(LPVOID param);
	void Reset();

protected:
	void ShowRawCh(HWND hDlg, int nCmdShow);
	void ShowRawChTitle(HWND hDlg, int nCmdShow);
	void ShowChannelArea(HWND hDlg, int nCmdShow);
	void ShowRawCh(HWND hDlg, bool enable);
	void ShowRawChTitle(HWND hDlg, bool enable);
	void ShowChannelArea(HWND hDlg, bool enable);
	void SentFilterInfo2Parent(void);

protected:
	UINT	m_nProcCh;
	TCHAR	m_FilterFileName[MAX_FILTER_TXT];
	TCHAR	m_FilterName[MAX_FILTER_TXT];
	bool	m_FilterActive;
};

