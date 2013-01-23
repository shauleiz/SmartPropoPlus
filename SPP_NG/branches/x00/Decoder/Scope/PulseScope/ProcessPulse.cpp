// PulseScope - Oscilloscope for viewing SPP Pulses
// 
// This file holdes the ProcessPulse callback function and related helper functions
#include "stdafx.h"
#include "PulseScope.h"


PULSESCOPE_API void Pulse2Scope(int length, bool low, LPVOID Param)
/* 
	Starting point - This is the ProcessPulse callback function

	length - Pulse length in 192K sample units
	low    - Pulse polarity (true=LOW, false=HIGH)
	Param  - Pointer to initialized CPulseScope object

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

	// If no CPulseScope object then NO-OP
	if (!Param)
		return;

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

		pXBuff[iPulse] = (float)length;
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
		PostMessage(NULL/*((CPulseScope *)Param)*/, WM_BUFF_READY, (WPARAM)(&pulses), NULL);

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

