#include "stdafx.h"
#include "Wave2Joystick.h"
#include "SmartPropoPlus.h"
#include ".\logaudiohdrs.h"

CLogAudioHdrs::CLogAudioHdrs(void)
{
	m_MsgNoAbortRec		= "Cannot abort while Logging";
	m_FilterStr			= "Log File\0*.log\0\0";
	m_DefaultFileExt	= "Log";
}

CLogAudioHdrs::~CLogAudioHdrs(void)
{
}

void CLogAudioHdrs::UpdateControls(void)
{
	GetDlgItem(IDC_REC_START)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_SEC)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_STOP)->EnableWindow(m_Recording || m_Playing);
}

void CLogAudioHdrs::PostMessageStop()
{
	GetParent()->PostMessage(MSG_AUDIO_HDRS_STOP,0,0);
}

void CLogAudioHdrs::PostMessageStart()
{
	GetParent()->PostMessage(MSG_AUDIO_HDRS_START,0,0);
}
void CLogAudioHdrs::PostMessageCancel()
{
	GetParent()->PostMessage(MSG_AUDIO_HDRS_CANCEL,0,0);
}

// Encasulates call to external function
BOOL CLogAudioHdrs::SppLoggerStop(void)				{ return SppAudioHdrLoggerStop();}
BOOL CLogAudioHdrs::SppLoggerStart(TCHAR * FileName) { return SppAudioHdrLoggerStart(FileName);}
BOOL CLogAudioHdrs::SppLoggerStat(int * stat)		{ return SppAudioHdrLoggerGetStat(stat);}

