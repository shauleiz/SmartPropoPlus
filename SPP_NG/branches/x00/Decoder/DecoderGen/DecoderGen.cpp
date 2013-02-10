/*
	Decoder for SmartPropoPlus (NG)

	This is the source for the Decoder base class
	All SPP decoders are derived from this class

*/
#include "stdafx.h"
#include "DecoderGen.h"
#include <WinMessages.h>

SPPINTERFACE_API	CDecoderGen::CDecoderGen(HWND hTarget, LPVOID Params) : 
	m_DecoderVersion(0xFFFFFFFF),
	m_DecoderFriendlyName(L"Error"),
	m_DecoderGuid(GUID_NULL),
	m_ValidPosition(false),
	m_hTargetWin(NULL)
{
	m_hTargetWin = hTarget;
}

SPPINTERFACE_API	CDecoderGen::~CDecoderGen(void)
{
}

SPPINTERFACE_API	bool CDecoderGen::isSppDecoder(void)
{
	return true;
}

SPPINTERFACE_API	LPCTSTR CDecoderGen::GetDecoderFriendlyName(void)
{
	return m_DecoderFriendlyName;
}

SPPINTERFACE_API	DWORD	CDecoderGen::GetDecoderVersion(void)
{
	return m_DecoderVersion;
}

SPPINTERFACE_API	GUID	CDecoderGen::GetDecoderGuid(void)
{
	return m_DecoderGuid;
}

SPPINTERFACE_API	bool	CDecoderGen::GetPosition(const POS & position)
{
	return m_ValidPosition;
}



void CDecoderGen::PostPositionData(void)
{
	// If no target then Do Nothing
	if (!m_hTargetWin)
		return;

	// Post message with WMAPP_POS_READY and data (And GUID)
	PostMessage(m_hTargetWin, WMAPP_POS_READY, (WPARAM)(&m_ChannelPos), (LPARAM)(&m_DecoderGuid));
}