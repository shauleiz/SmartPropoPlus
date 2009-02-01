#include "stdafx.h"
#include "Wave2Joystick.h"
#include "SmartPropoPlus.h"
#include ".\logfmsconn.h"

CLogFmsConn::CLogFmsConn(void)
{
	m_MsgNoAbortRec		= "Cannot abort while Logging";
	m_FilterStr			= "Log File\0*.log\0\0";
	m_DefaultFileExt	= "Log";
	_tcscpy(m_WaveFileName,"FmsConnecion");
}

CLogFmsConn::~CLogFmsConn(void)
{
}

void CLogFmsConn::UpdateControls(void)
{
	GetDlgItem(IDC_REC_START)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_SEC)->EnableWindow(!m_Recording && !m_Playing);
	GetDlgItem(IDC_REC_STOP)->EnableWindow(m_Recording || m_Playing);
}

void CLogFmsConn::PostMessageStop()
{
	GetParent()->PostMessage(MSG_FMS_CONN_STOP,0,0);
}

void CLogFmsConn::PostMessageStart()
{
	GetParent()->PostMessage(MSG_FMS_CONN_START,0,0);
}
void CLogFmsConn::PostMessageCancel()
{
	GetParent()->PostMessage(MSG_FMS_CONN_CANCEL,0,0);
}

// Encasulates call to external function
BOOL CLogFmsConn::SppLoggerStop(void)				{ PostMessageStop(); return TRUE;}
BOOL CLogFmsConn::SppLoggerStart(TCHAR * FileName) { PostMessageStart(); return TRUE;}
