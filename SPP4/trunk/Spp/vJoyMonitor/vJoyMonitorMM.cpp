#include "stdafx.h"
#include "SmartPropoPlus.h"
#include "vJoyMonitorMM.h"


CvJoyMonitorMM::CvJoyMonitorMM(void)
{
}

CvJoyMonitorMM::CvJoyMonitorMM(HINSTANCE hInstance, HWND	ParentWnd) : CvJoyMonitor(hInstance, 	ParentWnd)
{
	if (!m_ParentWnd || !m_hInstance)
		return;
}


CvJoyMonitorMM::~CvJoyMonitorMM(void)
{
}
