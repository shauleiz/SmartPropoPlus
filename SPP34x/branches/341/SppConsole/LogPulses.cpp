#include "stdafx.h"
#include "Wave2Joystick.h"
#include "SmartPropoPlus.h"
#include ".\logpulse.h"

CLogPulse::CLogPulse(void)
{
	m_MsgNoAbortRec		= "Cannot abort while Logging";
	m_FilterStr			= "Log File\0*.log\0\0";
	m_DefaultFileExt	= "Log";
}

CLogPulse::~CLogPulse(void)
{
}

void CLogPulse::UpdateControls(void)
{
	GetDlgItem(IDC_REC_START)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_SEC)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_STOP)->EnableWindow(m_Recording || m_Playing);
}

void CLogPulse::PostMessageStop()
{
	GetParent()->PostMessage(MSG_PULSE_STOP,0,0);
}

void CLogPulse::PostMessageStart()
{
	GetParent()->PostMessage(MSG_PULSE_START,0,0);
}
void CLogPulse::PostMessageCancel()
{
	GetParent()->PostMessage(MSG_PULSE_CANCEL,0,0);
}

// Encasulates call to external function
BOOL CLogPulse::SppLoggerStop(void)				{ return SppPulseLoggerStop();}
BOOL CLogPulse::SppLoggerStart(TCHAR * FileName) { return SppPulseLoggerStart(FileName);}
BOOL CLogPulse::SppLoggerStat(int * stat)		{ return SppPulseLoggerGetStat(stat);}

