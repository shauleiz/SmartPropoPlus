/*
	CPulseData - Pulse Data Class
	Convert input from AudioInputW7 into pulse data
	A pulse is a two-entry object:
		1. Pulse duration in 192K sample units (1,000,000/192,000 = 5.208333uSec)
		2. Pulse Polarity (Up/Down)
*/

#include "stdafx.h"
#include "WinMessages.h"
#include "PulseData.h"

//////////// Globals /////////////////////////////////////////////////////
// Globals

//////////// Class CPulseData ////////////////////////////////////////////
CPulseData::CPulseData()
{
	m_PulseDuration = 0;		// Pulse duration in 192K sampling rate
	m_PulsePolarity = 0;		// Up=1, Down=-1, Unknown=0
	m_WaveRate = 0;				// Sample rate (44100, 48000, 96000, 192000 ....)
	m_WaveNChannels = 0;        // number of channels (i.e. mono, stereo...)
	m_WaveBitsPerSample = 0;    // Number of bits per sample of mono data

}

CPulseData::~CPulseData(void)
{
}

HRESULT	CPulseData::Initialize(UINT rate, UINT nChannels, UINT BitsPerSample)
{
	HRESULT hr = S_OK;

	// Supports bit-rate of 20K and up
	if (rate <20000)
		hr = E_INVALIDARG;
	EXIT_ON_ERROR(hr);
	m_WaveRate = rate;

	// Supports only Mono/Stereo
	if (nChannels<1 || nChannels>2)
		hr = E_INVALIDARG;
	EXIT_ON_ERROR(hr);
	m_WaveNChannels = nChannels;

	m_WaveBitsPerSample = BitsPerSample;

Exit:
	return hr;
}
