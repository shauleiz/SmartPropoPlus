/*
	CPulseData - Pulse Data Class
	Convert input from SppAudio into pulse data
	A pulse is a two-entry object:
		1. Pulse duration in 192K sample units (1,000,000/192,000 = 5.208333uSec)
		2. Pulse Polarity (Up/Down)
*/

#include "stdafx.h"
#include "WinMessages.h"
#include "math.h"
#include "PulseData.h"

//////////// Globals /////////////////////////////////////////////////////
void DefaultProcPulse(int length,  bool low, LPVOID Param)
{
	// Default (NO-OP) logger
}

//////////// Class CPulseData ////////////////////////////////////////////
CPulseData::CPulseData()
{
	m_PulseDuration = 0;		// Pulse duration in 192K sampling rate
	m_PulsePolarity = 0;		// Up=1, Down=-1, Unknown=0
	m_WaveRate = 0;				// Sample rate (44100, 48000, 96000, 192000 ....)
	m_WaveNChannels = 0;        // number of channels (i.e. mono, stereo...)
	m_WaveBitsPerSample = 0;    // Number of bits per sample of mono data
	m_WaveInputChannel = 0;		// Input channel is LEFT by default
	ProcessPulse = NULL;

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

HRESULT	CPulseData::ProcessWave(BYTE * pWavePacket, UINT32 packetLength)
/*
	ProcessWave processes a wave packet
	Return value:
		S_OK:		Processing of the data was completed - ready to get the next packet
		E_xxx:		Error

	Input parameters:
		[IN] pWavePacket:	Pointer to the beginning of the wave packet
		[IN] packetLength:	Length of packet
		[IN] right_channel:	Process right channel(Second channel) - Default is left channel (Ignored for mono waves)
*/
{


	HRESULT hr = S_OK;
	UINT PulseLength = 0;
	bool negative;

	// for every sample in the packet, read data carrying channels
	// If mono (m_WaveNChannels==1) then increment sample index by 1
	// If stereo (m_WaveNChannels==2) then increment sample index by 2
	// If stereo then read the first channel if you need the left channel or the second if it is the right channel you want
	unsigned char sample8;
	signed short sample;
	for (UINT32 i=0; i<packetLength*m_WaveNChannels;i+=m_WaveNChannels)
	{
		// Get one sample
		if (m_WaveBitsPerSample == 8)
		{// 8-bit PCM, ranging 0-255, mid-point is 128
			sample8 =	(unsigned char)pWavePacket[i+m_WaveInputChannel];
			sample	=	(signed short)sample8;
		} else if (m_WaveBitsPerSample == 16)
			// 16-bit PCM, ranging from -32768 to 32767, mid-point is 0
			sample = ((signed short *)pWavePacket)[i+m_WaveInputChannel];
		else break;


		// Here the samples are assemblied into pulses
		// negative is true when pulse is LOW
		PulseLength = Sample2Pulse(sample, &negative);

		// Normalize pulse length to 192K sampling rate
		PulseLength = NormalizePulse(PulseLength);

		// If valid pulse the process the pulse
		// TODO (?): Very short pulses are ignored (Glitch)
		if (PulseLength/*>3*/)
			ProcessPulse(PulseLength, negative,m_ProcPulseParam);
	};



	EXIT_ON_ERROR(hr);
Exit:
	return hr;
}

void CPulseData::GetPulseValues(UINT * PulseDuration, INT * PulsePolarity)
{
}


void CPulseData::SelectInputChannel(bool RightChannel)
{
	if (m_WaveNChannels<2)
	{
		m_WaveInputChannel = 0;
		return;
	};

	if (RightChannel) 
		m_WaveInputChannel = 1;
	else
		m_WaveInputChannel = 0;
}

inline UINT CPulseData::Sample2Pulse(short sample, bool * negative)
{

	static double min = 0;	/* Sticky minimum sample value */
    static double max = 0;	/* Sticky maximum sample value */
    static int high = 0;	/* Number of contingious above-threshold samples */
    static int low = 0;		/* Number of contingious below-threshold samples */
    double threshold;		/* calculated mid-point */
	UINT pulse = 0;

	/* Initialization of the min/max vaues */
    max -= 0.1;
    min += 0.1;
    if (max < min) max = min + 1;

    if (sample> max) max = sample;				/* Update max value */
    else if (sample < min) min = sample;		/* Update min value */
    //threshold = (min + max) / 2;				/* Update mid-point value */
	threshold = CalcThreshold(sample);			/* Version 3.3.3 */

	/* Update the width of the number of the low/high samples */
	/* If edge, then call ProcessPulse() to process the previous high/low level */
	if (sample > threshold) 
	{
		high++;
		if (low) 
		{
			pulse = low;
			*negative = true;
			low = 0;
		}
	} else 
	{
		low++;
		if (high) 
		{
			pulse = high;
			*negative = false;
			high = 0;
		}
	};

	// Case of very long (20000) pulses
	if (high >= 20000)
	{
		pulse = high;
		*negative = false;
		high = 0;
	}
	else if (low  >= 20000)
	{
		pulse = low;
		*negative = true;
		low = 0;
	};

	return pulse;
}
//inline void CPulseData::ProcessPulse(UINT PulseLength, bool negative)
//{
//	static UINT pulse[1000] = {0};
//	static int i=0;
//}

inline double CPulseData::CalcThreshold(int value)
/*
	Calculate audio threshold
	____________________________________________________________________
	Based on RCAudio V 3.0 and original Smartpropo
	copyright (C) Philippe G.De Coninck 2007
	
	Copied from: http://www.rcuniverse.com/forum/m_3413991/tm.htm
	____________________________________________________________________
*/
{
	// RCAudio V 3.0 : (C) Philippe G.De Coninck 2007

	static double aud_max_val, aud_min_val;
	double delta_max = fabs(value - aud_max_val);
	double delta_min = fabs(value - aud_min_val);

	if (delta_max > delta_min) aud_min_val = (4*aud_min_val + value)/5;
	else aud_max_val = (4*aud_max_val + value)/5;

	if (aud_max_val < aud_min_val + 2) {
		aud_max_val = aud_min_val + 1;
		aud_min_val = aud_min_val - 1;
	}

	return((aud_max_val + aud_min_val)/2); 
}

bool	CPulseData::RegisterProcessPulse(LPVOID f, LPVOID param)
{
/* Registration of callback function that processes the pulse data */
	if (f)
	{
		ProcessPulse = (PROCPULSEFUNC)f;
		m_ProcPulseParam = param;
	}
	else
	{
		ProcessPulse = (PROCPULSEFUNC)DefaultProcPulse;
		m_ProcPulseParam = NULL;
	};

	return true;
}

inline UINT CPulseData::NormalizePulse(UINT Length)
// Normalize pulse length to 192K sampling rate
{
	switch(m_WaveRate)
	{
	case 192000:
	case 0:
		return Length;
	case 96000:
		return Length*2;
	case 48000:
		return Length*4;
	case 44100:
		return (UINT)(Length*4.35);
	default:
		return Length*192000/m_WaveRate;
	};

	return 0;
}
