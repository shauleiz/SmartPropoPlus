#pragma once
#include "vjoymonitor.h"
class CvJoyMonitorMM :
	public CvJoyMonitor
{
public:
	CvJoyMonitorMM(void);
	CvJoyMonitorMM(HINSTANCE hInstance, HWND	ParentWnd);
	virtual ~CvJoyMonitorMM(void);
};

