#include "stdafx.h"
#include "Wave2Joystick.h"
#include "SmartPropoPlus.h"
#include ".\LogRawPulse.h"

CLogRawPulse::CLogRawPulse(void)
{
	m_MsgNoAbortRec		= "Cannot abort while Logging";
	m_FilterStr			= "Log File\0*.log\0\0";
	m_DefaultFileExt	= "Log";
	_tcscpy(m_WaveFileName,"RawPulses");
}

CLogRawPulse::~CLogRawPulse(void)
{
}

void CLogRawPulse::UpdateControls(void)
{
	GetDlgItem(IDC_REC_START)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_SEC)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_STOP)->EnableWindow(m_Recording || m_Playing);
}

void CLogRawPulse::PostMessageStop()
{
	GetParent()->PostMessage(MSG_RAW_PULSE_STOP,0,0);
}

void CLogRawPulse::PostMessageStart()
{
	GetParent()->PostMessage(MSG_RAW_PULSE_START,0,0);
}
void CLogRawPulse::PostMessageCancel()
{
	GetParent()->PostMessage(MSG_RAW_PULSE_CANCEL,0,0);
}

// Encasulates call to external function
BOOL CLogRawPulse::SppLoggerStop(void)				{ return SppRawPulseLoggerStop();}
BOOL CLogRawPulse::SppLoggerStart(TCHAR * FileName) { return SppRawPulseLoggerStart(FileName);}
BOOL CLogRawPulse::SppLoggerStat(int * stat)		{ return SppRawPulseLoggerGetStat(stat);}
