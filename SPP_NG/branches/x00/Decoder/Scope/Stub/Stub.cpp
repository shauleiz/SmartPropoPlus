// Stub.cpp : Defines the exported functions for the DLL application.
//
// Stub - Connects the Scope to the audio engine
// Supplies a callback function of type ProcessPulse and feeds the Scope with dots to connect

#include "stdafx.h"
#include "Stub.h"


// This is an example of an exported variable
STUB_API int nStub=0;

// This is an example of an exported function.
STUB_API int fnStub(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see Stub.h for the class definition
CStub::CStub()
{
	return;
}

STUB_API void Pulse2Scope(int length, bool low, LPVOID Param)
/* 
	Starting point - This is the ProcessPulse callback function

	This function is called for each pulse detected by the audio unit
	Every time it is called, it is called with pulse length and pulse polarity (low/high)
	
	Pulse2Scope() fills ulternate buffers with pulse data until a limit of 100mS (approx.)
	or until a buffer is full.
	When a buffer is ready (100mS/Full) Pulse2Scope() posts a message to the scope window with
	a pointer to the buffer and the number of entries.
*/
{
	// Define two sets of X/Y buffers, Buffer index (0/1) and pulse index
	static float	X0Buff[PULSE_BUF_SIZE], Y0Buff[PULSE_BUF_SIZE];
	static float	X1Buff[PULSE_BUF_SIZE], Y1Buff[PULSE_BUF_SIZE];
	static float	*pXBuff=&(X0Buff[0]), *pYBuff=(&Y0Buff[0]);
	static UCHAR	iBuff = 0;	// X0/Y0 or X1/Y1
	static UINT		iPulse = 0;	// Index of pulse in the buffer

	// Clip very log pulses to 100mS
	if (length > PULSE_MAX_SIZE)
		length = PULSE_MAX_SIZE;

	// For each buffer, accumulate the total length of pulses.
	// When the accumulated length passes 100mS the buffer is ready to be posted
	static  UINT acc = 0;

	// Fill-in the next entry in the buffers.
	if(iPulse<PULSE_BUF_SIZE)
	{
		acc+=length;

		pXBuff[iPulse] = (float)acc;
		if (low)
			pYBuff[iPulse] = LO;
		else
			pYBuff[iPulse] = HI;

		iPulse++;
	};

	// Check if buffer ready to be posted 
	if (acc>=PULSE_MAX_SIZE || iPulse>=PULSE_BUF_SIZE)
	{
		// Post Buffer
		PULSE_DATA pulses;
		pulses.nPulses = iPulse-1;
		pulses.pXBuff = pXBuff;
		pulses.pYBuff = pYBuff;
		PostMessage(g_hScopeWnd, WM_BUFF_READY, (WPARAM)(&pulses), NULL);

		// Reset counters
		iPulse=acc=0;

		// Swap buffers
		if (iBuff)
		{
			iBuff=0;
			pXBuff=&(X0Buff[0]);
			pYBuff=(&Y0Buff[0]);
		}
		else
		{
			iBuff=1;
			pXBuff=&(X1Buff[0]);
			pYBuff=(&Y1Buff[0]);
		};


	};
}

STUB_API void SetSampleRate(int rate)
// We expose this function so that the control unit will set the sample rate
// Possible values are 192KHz (Default), 96KHz, 48KHz and 44.1KHz
{
	g_rate = rate;
}

STUB_API void InitDecoder(int rate)
// Initialize the decoder
{
	g_rate = rate;
}