// SppMain.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "GlobalMemory.h"
#include "SmartPropoPlus.h"
#include "SppMain.h"


CSppMain::CSppMain() :
	m_PropoStarted(false),
	m_pSharedBlock(NULL),
	m_MixerName(NULL),
	m_ListProcessPulseFunc(NULL)
{}
CSppMain::~CSppMain() {}


bool CSppMain::Start()
{
	// Start only once
	if (m_PropoStarted)
		return false;
	m_PropoStarted = true;


	// Get list of modulation types: PPM/PCM(JR) ....
	// Mark the selected modulation type
	struct Modulations *  Modulation = GetModulation(0);

	//// Audio system from registry
	//// Commented out becaus currently SPP uses default device
	//if (GetCurrentAudioState())			// Get  Mixer Device (selected or preferred) 
	//	m_MixerName = GetCurrentMixerDevice();	// Selected

	// Create shared memory block and fill it with:
	// - List of modulations as acquired above
	// - Other default values
	m_pSharedBlock = CreateSharedDataStruct(Modulation);

	/* SetActiveProcessPulseFunction(DataBlock); */
	int nActiveModulations = LoadProcessPulseFunctions();

	return true;
}

/*
	Create a list of pointers to functions (ProcessPulseXXX)
	The order is acording to the order of the modulation types in the Global Memory.
	The number of entries is 3 times the number of modulations + 1
	The first set will consist of the default (autodetect) version of the functions
	The second set will consist of the negative-shift version of the functions
	The third set will consist of the positive-shift version of the functions
	The addional input is a final NULL entry
*/
int CSppMain::LoadProcessPulseFunctions()
{
	int index=0;
	TCHAR tmp[MAX_VAL_NAME];
	int nMod;


	/* Version 3.3.2 */
	if (m_pSharedBlock && ((SharedDataBlock *)m_pSharedBlock)->nModulations > 0)
		nMod = ((SharedDataBlock *)m_pSharedBlock)->nModulations;
	else
	{	
		MessageBox(NULL,MM_NO_NUM_MOD, MM_STD_HDR , MB_SYSTEMMODAL);
		nMod = 1;
	};

	m_ListProcessPulseFunc = (PP *)calloc(3*nMod+1, sizeof(far void *));

	/* Loop on list of modulation types */
	while(index < nMod && index<MAX_MODS)
	{
		/* Get the internal name of the modulation */
		if (m_pSharedBlock && ((SharedDataBlock *)m_pSharedBlock)->pInternalModName)/* Version 3.3.2 */
			_tcscpy_s(tmp, MAX_MODS, (TCHAR *)((SharedDataBlock *)m_pSharedBlock)->pInternalModName[index]);
		/* Version 3.3.2 */
		else
		{
			/* TODO: Take it from the registry */
			_tcscpy_s(tmp,10,  _T("PPM"));
		};


		if (!_tcscmp(tmp, MOD_TYPE_PPM))
		{	
			m_ListProcessPulseFunc[0*nMod+index]  =   (&CSppMain::ProcessPulsePpm);			/* Auto  Detect*/
			//m_ListProcessPulseFunc[1*nMod+index] = (void *)ProcessPulseFutabaPpm;	/* Negative  Detect*/
			//m_ListProcessPulseFunc[2*nMod+index] = (void *)ProcessPulseJrPpm;		/* Positive  Detect*/
		}
#if 0
		else if (!_tcscmp(tmp, MOD_TYPE_PPMW))
		{
			m_ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseWK2401Ppm;
			m_ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseWK2401Ppm;
			m_ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseWK2401Ppm;
		}
		else if (!_tcscmp(tmp, MOD_TYPE_JR))
		{
			m_ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseJrPcm;
			m_ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseJrPcm;
			m_ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseJrPcm;
		}
		else if (!_tcscmp(tmp, MOD_TYPE_FUT))
		{
			m_ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseFutabaPcm;
			m_ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseFutabaPcm;
			m_ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseFutabaPcm;
		}
		else if (!_tcscmp(tmp, MOD_TYPE_AIR1))
		{
			m_ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseAirPcm1;
			m_ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseAirPcm1;
			m_ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseAirPcm1;
		}
		else if (!_tcscmp(tmp, MOD_TYPE_AIR2))
		{
			m_ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseAirPcm2;
			m_ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseAirPcm2;
			m_ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseAirPcm2;
		}
		else if (!_tcscmp(tmp, MOD_TYPE_WAL))
		{
			m_ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseWalPcm;
			m_ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseWalPcm;
			m_ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseWalPcm;
		}
#endif
		index++;
	};


	m_ListProcessPulseFunc[2*nMod+index] = NULL;
	return index;
}

/*
	Process PPM pulse
	A long (over 200 samples) leading high, then a short low, then up to six highs followed by short lows.
	The size of a high pulse may vary between 30 to 70 samples, mapped to joystick values of 1024 to 438
	where the mid-point of 50 samples is translated to joystick position of 731.
*/
void CSppMain::ProcessPulsePpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	char tbuffer [9];
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */

	if (width < 5)
		return;

	if (gDebugLevel>=2 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulsePpm(width=%d, input=%d)", _strtime( tbuffer ), width, input);

	/* If pulse is a separator then go to the next one */
	if (width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (/*sync == 0 && */width > PPM_TRIG) {
        sync = 1;
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


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input|| JsChPostProc_selected!=-1)
		newdata = (int)(1024 - (width - PPM_MIN) / (PPM_MAX - PPM_MIN) * 1024); /* JR */
	else
		newdata = (int)((width - PPM_MIN) / (PPM_MAX - PPM_MIN) * 1024);		/* Futaba */

	/* Trim values into 0-1023 boundries */
    if (newdata < 0) newdata = 0;
    else if (newdata > 1023) newdata = 1023;

	/* Update data - do not allow abrupt change */
    if (data[datacount] - newdata > 100) data[datacount] -= 100;
    else if (newdata - data[datacount] > 100) data[datacount] += 100;
    else data[datacount] = (data[datacount] + newdata) / 2;

	
	if (input|| JsChPostProc_selected!=-1)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	case 6: 	Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	case 7: 	Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	case 8: 	Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	case 9: 	Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	case 10: 	Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	case 11: 	Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	};
#ifdef PPJOY
				SendPPJoy(datacount, Position);
#endif

	if (gDebugLevel>=3 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile," data[%d]=%d", datacount, data[datacount]);

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return;
}
