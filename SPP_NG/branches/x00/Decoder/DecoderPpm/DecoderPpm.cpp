#include "..\\DecoderGen\\stdafx.h"
#include "..\\DecoderGen\\DecoderGen.h"
#include "DecoderPpm.h"

SPPINTERFACE_API	CDecoderPpm::CDecoderPpm(HWND hTarget, LPVOID Params)
{
	// Defaul values
	m_DecoderFriendlyName = L"PPM (Generic)";
	m_DecoderVersion = 0x01000001;
	m_DecoderGuid = GUID_DECODER_PPM;

	// Init target window
	m_hTargetWin = hTarget;

	// Initialize PPM parameters
	if (!Params)
	{ // Initialize to default
		m_param_separate	= PPM_SEP*192/44.1 + 7;
		m_param_trigger		= PPM_TRIG*192/44.1;
		m_param_min			= PPM_MIN*192/44.1;
		m_param_max			= PPM_MAX*192/44.1;
	};
}

SPPINTERFACE_API	CDecoderPpm::~CDecoderPpm(void)
{
}

SPPINTERFACE_API	void CDecoderPpm::ProcessPulse(int width, bool low)
// This is the actual decoder logic
// Based on winmm.c:ProcessPulsePpm()
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */

	if (width < 5)
		return;

	/* If pulse is a separator then go to the next one */
	if (width < m_param_separate || former_sync)
	{
		former_sync = 0;
		return;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (width > m_param_trigger) {
        sync = 1;
		m_ChannelPos.m_nAnalogChannels = datacount;
		m_ValidPosition = true;
		PostPositionData();
        datacount = 0;
		former_sync = 1;
		return;
    }

    if (!sync) 
		return; /* still waiting for sync */

	// Cancel jitter /* Version 3.3.3 */
	if (abs(PrevWidth[datacount] - width) < 2)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;


	/* convert pulse width in samples to joystick position values (newdata)*/
	if (low)
		newdata = (int)(1024 - (width - m_param_min) / (m_param_max - m_param_min) * 1024); /* JR */
	else
		newdata = (int)((width - m_param_min) / (m_param_max - m_param_min) * 1024);		/* Futaba */


	/* Trim values into 0-1023 boundries */
    if (newdata < 0) newdata = 0;
    else if (newdata > 1023) newdata = 1023;

	/* Update data - do not allow abrupt change */
    if (data[datacount] - newdata > 100) data[datacount] -= 100;
    else if (newdata - data[datacount] > 100) data[datacount] += 100;
    else data[datacount] = (data[datacount] + newdata) / 2;

	
	if (low)
		m_ChannelPos.AnalogChannel[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
		case 0: 	m_ChannelPos.AnalogChannel[1]  = data[datacount];	break;/* Assign data to joystick channels */
		case 1: 	m_ChannelPos.AnalogChannel[2]  = data[datacount];	break;/* Assign data to joystick channels */
		case 2: 	m_ChannelPos.AnalogChannel[0]  = data[datacount];	break;/* Assign data to joystick channels */
		case 3: 	m_ChannelPos.AnalogChannel[3]  = data[datacount];	break;/* Assign data to joystick channels */
		case 4: 	m_ChannelPos.AnalogChannel[4]  = data[datacount];	break;/* Assign data to joystick channels */
		case 5: 	m_ChannelPos.AnalogChannel[5]  = data[datacount];	break;/* Assign data to joystick channels */
		case 6: 	m_ChannelPos.AnalogChannel[6]  = data[datacount];	break;/* Assign data to joystick channels */
		case 7: 	m_ChannelPos.AnalogChannel[7]  = data[datacount];	break;/* Assign data to joystick channels */
		case 8: 	m_ChannelPos.AnalogChannel[8]  = data[datacount];	break;/* Assign data to joystick channels */
		case 9: 	m_ChannelPos.AnalogChannel[9]  = data[datacount];	break;/* Assign data to joystick channels */
		case 10: 	m_ChannelPos.AnalogChannel[10] = data[datacount];	break;/* Assign data to joystick channels */
		case 11: 	m_ChannelPos.AnalogChannel[11] = data[datacount];	break;/* Assign data to joystick channels */
	};


	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return;
}
