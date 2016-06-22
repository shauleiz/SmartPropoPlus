#pragma once
#include "spptab.h"
class SppTabAdv : public SppTab

{
public:
	SppTabAdv(void);
	virtual ~SppTabAdv(void);
	SppTabAdv(HINSTANCE hInstance, HWND TopDlgWnd);

	void SetAudioLog(bool cb);
	void SetPulseScope(bool cb);
	void PulseScope(WORD cb);
	void ShowLogWindow(WORD cb);
	void RecordInSignal(WORD cb);
	void RecordPulse(WORD cb);
	void UpdateToolTip(LPVOID param);
	void ResetRequest(void);
	void Reset();
	void DisplayVersionText(UINT SppVer);
};

