// Header file for Class CPulseData

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SPPINTERFACE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// SPPINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SPPINTERFACE_EXPORTS
#define SPPINTERFACE_API __declspec(dllexport)
#else
#define SPPINTERFACE_API __declspec(dllimport)
#endif

#pragma once

#include "resource.h"

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { DbgPopUp(__LINE__, hres); goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }



void   DbgPopUp(int Line, DWORD Error);

class CPulseData
{
public:
	CPulseData(void);
	virtual	~CPulseData(void);
	HRESULT	Initialize(UINT rate, UINT nChannels, UINT BitsPerSample);


protected:
	// Output Pulse
	unsigned int	m_PulseDuration;		// Pulse duration in 192K sampling rate
	int				m_PulsePolarity;		// Up=1, Down=-1, Unknown=0

	// Input wave
	unsigned int	m_WaveRate;				// Sample rate (44100, 48000, 96000, 192000 ....)
	unsigned int	m_WaveNChannels;        // number of channels (i.e. mono, stereo...)
	unsigned int	m_WaveBitsPerSample;    // Number of bits per sample of mono data
};