#pragma once

#ifdef SPPINTERFACE_EXPORTS
#define SPPINTERFACE_API __declspec(dllexport)
#else
#define SPPINTERFACE_API __declspec(dllimport)
#endif

#include <INITGUID.H>	// Definitions for controlling GUID initialization
struct POS {
	LONG	AnalogChannel[15];
	bool	DigitalChannel[32];
	UINT	m_nAnalogChannels;
	UINT	m_nDigitalChannels;
};

class CDecoderGen
{
public:
	SPPINTERFACE_API	CDecoderGen(HWND hTarget = NULL, LPVOID Params = NULL);
	SPPINTERFACE_API	virtual ~CDecoderGen(void);

	// Common interface
	SPPINTERFACE_API	bool	isSppDecoder(void);
	SPPINTERFACE_API	LPCTSTR GetDecoderFriendlyName(void);
	SPPINTERFACE_API	DWORD	GetDecoderVersion(void);
	SPPINTERFACE_API	GUID	GetDecoderGuid(void);
	SPPINTERFACE_API	bool	GetPosition(const POS & position);
	SPPINTERFACE_API	virtual void ProcessPulse(int length, bool low) = 0;

protected:
	// Helper functions
	void PostPositionData(void);

protected:
	LPCTSTR m_DecoderFriendlyName;
	DWORD	m_DecoderVersion;
	GUID	m_DecoderGuid;
	POS		m_ChannelPos;
	bool	m_ValidPosition;
	HWND	m_hTargetWin;
};
