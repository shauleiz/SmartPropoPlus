// PulseScope - Oscilloscope for viewing SPP Pulses
// 
// This file holdes the ProcessPulse callback function and related helper functions
#include "stdafx.h"
#include "PulseScope.h"


PULSESCOPE_API void Pulse2Scope(int index, int length, bool low, LPVOID timestamp, LPVOID Param)
/* 
	Starting point - This is the ProcessPulse callback function

	index     - Index of wave to which this pulse belongs
	length    - Pulse length in 192K sample units
	low       - Pulse polarity (true=LOW, false=HIGH)
	timestamp - Pointer to absolute timestamp. Ignored when 0
	Param     - Pointer to initialized CPulseScope object

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
		// Send Buffer to scope
		((CPulseScope *)Param)->DisplayPulseData(iPulse-1, pXBuff, pYBuff);

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

PULSESCOPE_API void WaveInfo2Scope(PULSE_INFO * Info, LPVOID Param)
{
	/*
		This function calls the Scope function that prints Information
		It creates a string and sends it to the scope for dispaly
	*/

	// If Param is NULL then error because the scope object must be valid - return
	if (!Param)
		return;

	// If Info is NULL - clear the info field of the Scope
	if (!Info)
		// TODO: Call CPulseScope::ClearWaveInfo()
		return;

	// Create a string and calculate size
	// Channel:
	WCHAR strCh[10];
	if (Info->nChannels == 1)
		wsprintf(strCh, L"Mono ");
	else if (Info->isRight)
 		wsprintf(strCh, L"Right ");
	else
		wsprintf(strCh, L"Left ");

	// Bitrate
	WCHAR strBr[3];
	if (Info->BitRate >0)
		wsprintf(strBr, L"%2d", Info->BitRate);
	else
		wsprintf(strBr, L"?");


	// Wave Rate
	WCHAR strWr[8];
	float  fWaveRate = (float)(Info->WaveRate) / 1000;
	swprintf(strWr, 7,L"%3.1f", fWaveRate);

	static WCHAR strOut[40];
	swprintf(strOut, 39,L"%s %sbit %sKb/S", strCh, strBr, strWr);
	size_t size;
	size = wcslen(strOut);
	((CPulseScope *)Param)->SetWaveInfo(strOut, size);

}