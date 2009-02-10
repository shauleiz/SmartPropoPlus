// Wave2Joystick.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "SmartPropoPlus.h"
#include "WaveIn.h"
#include "PpjoyEx.h"
#include "PpjoyIf.h"
#include "FmsIf.h"
#include "FilterIf.h"
#include "wave2joystick.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CWave2JoystickApp

void _PrintChannels(int * ch, int nCh)
{
	static FILE * pFile;

	if (!pFile)
	{
		pFile = fopen("C:\\Documents and Settings\\Shaul\\Local Settings\\TEMP\\W2JData.log", "w+");
		if (!pFile) return;
		fprintf(pFile, "N=\tCh1\t\tCh2\t\tCh3\t\tCh4\t\tCh5\t\tCh6\n");
	};

	fprintf(pFile, "%d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\n", nCh, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5]);
}

BEGIN_MESSAGE_MAP(CWave2JoystickApp, CWinApp)
END_MESSAGE_MAP()

// CWave2JoystickApp construction

CWave2JoystickApp::CWave2JoystickApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CWave2JoystickApp object
CWave2JoystickApp theApp;


// CWave2JoystickApp initialization
BOOL CWave2JoystickApp::InitInstance()
{
	m_CurrMixerDeviceName	= NULL;
	m_WaveInList			= NULL;
	m_ModulationType		= PpmAuto;		// Default: PPM
	m_ModulationShift		= I_AUT_SHIFT;	// Default: Auto
	m_Filters				= NULL;			// Default: No filters
	m_fms					= NULL;
	m_nPostFilterPos		= 0;			// Number of post-filter joystick positions
	m_PostFilterPos			= NULL;			// Post-filter joystick positions


	// Create the memory mapped file from where winmm.dll will read joystick and other data
	m_fms = new CFmsIf();
	m_fms->InitInstance();

	CWinApp::InitInstance();
	return TRUE;
}

// Stop the interfaces with FMS & PPJoy
BOOL CWave2JoystickApp::StopInterfaces(void)
{
	if (m_fms)
		delete m_fms;
	if (m_PpjoyObj)
		delete m_PpjoyObj;

	return TRUE;
}


// Generic function to set working parameters such as modulation type and audio device
// Serves as entry point for interface function
// Calls other member functions according to 'param'
bool CWave2JoystickApp::SetParam(const int param, const void * pValue)
{
	switch (param)
	{
	case MOD:
		return SetModulation((const TCHAR *)pValue);
	case SHIFT:	// Shift type
		return SetShift(*(const int *)pValue);
	case JITTER:	// Anti-jitter
		return SetAntiJitter(*(const bool *)pValue);
	case DEVICE:	// Audio Device
		return SetActiveAudioDevice((const TCHAR *)pValue);
	case INPUT:	// Audio Input
		return SetSelectedInput((const TCHAR *)pValue);
	case MIXER:	// Mixer selector
		return SetMixerSelector(*(const bool *)pValue);
	case WAVSTART: // Start logging
		return StartWaveLog((TCHAR *)pValue);
	case WAVSTOP:  // Stop logging
		return StopWaveLog();
	case WAVPLAY:  // Play Wav file
		return PlayWaveLog((TCHAR *)pValue);
	case WAVSTAT:  // Status of logger
		return StatWaveLog((int *)pValue);
	case AHSTART: // Start logging
		return StartAudioHeaderLog((TCHAR *)pValue);
	case AHSTOP:  // Stop logging
		return StopAudioHeaderLog();
	case AHSTAT:  // Status of logger
		return StatAudioHeaderLog((int *)pValue);
	case PLSSTART: // Start logging
		return StartPulseLog((TCHAR *)pValue);
	case PLSSTOP:  // Stop logging
		return StopPulseLog();
	case PLSSTAT:  // Status of logger
		return StatPulseLog((int *)pValue);
	case RPLSSTART: // Start logging
		return StartRawPulseLog((TCHAR *)pValue);
	case RPLSSTOP:  // Stop logging
		return StopRawPulseLog();
	case RPLSSTAT:  // Status of logger
		return StatRawPulseLog((int *)pValue);
	case AUDIOLEVEL:  // Monitoring: Get audio level
		return GetAudioLevel((int *)pValue);
	case NJOYPOS:  // Monitoring: Get the number of joystick positions (Pre-filter)
		return GetNumJoyPos((int *)pValue);
	case JOYPOS:  // Monitoring: Get the joystick positions (Pre-filter)
		return GetJoyPos((int *)pValue);
	case POSTFLTR:  // Monitoring: Get the joystick positions (Post-filter)
		return GetFilteredPos((int *)pValue);

	default:
		return false;
	};
}
/***************** Set basic operation mode (Modulation / Audio source) *****************/

// Set Modulation type and sub-type
// Call CWaveIn::SetModulation
// If successful - updates member variable m_ModulationType and return true
bool CWave2JoystickApp::SetModulation(const TCHAR * Type)
{
	MODE tmp_ModulationType;

	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	// Convert from string 'Type' to enum MODE
		if (!strcmp(Type, MOD_NAME_PPM))
		{
			if (m_ModulationShift == I_NEG_SHIFT)
				tmp_ModulationType = PpmNeg;
			else if (m_ModulationShift == I_POS_SHIFT)
				tmp_ModulationType = PpmPos;
			else 
				tmp_ModulationType = PpmAuto;
		}
		else if (!strcmp(Type, MOD_NAME_JR))
				tmp_ModulationType = PcmJr;
		else if (!strcmp(Type, MOD_NAME_FUT))
				tmp_ModulationType = PcmFut;
		else if (!strcmp(Type, MOD_NAME_AIR1))
				tmp_ModulationType = PcmAir1;
		else if (!strcmp(Type, MOD_NAME_AIR2))
				tmp_ModulationType = PcmAir2;
		else if (!strcmp(Type, MOD_NAME_WAL))
				tmp_ModulationType = PcmWalkera;
		else return false;

	// Get the index of the active interface and call its CWaveIn::SetModulation
	int iDevice		=	m_WaveInList->index;
	bool success	=	m_WaveInList->pDevice[iDevice]->SetModulation(tmp_ModulationType);
	
	// If managed to change mode of the active CWaveIn object - update 'm_ModulationType'
	if (success)
	{
		m_ModulationType = tmp_ModulationType;
		return true;
	}
	else
		return false;
}

// Set shift type (Negative/Positive/Auto) and set modulation to PPM+shift
bool CWave2JoystickApp::SetShift(const int Shift)
{
	m_ModulationShift = Shift;
	if (m_ModulationType == PpmNeg || m_ModulationType == PpmPos || m_ModulationType == PpmAuto)
		return SetModulation(MOD_NAME_PPM);
	else
		return true;
}

// Set anti-jitter to the selected device
bool CWave2JoystickApp::SetAntiJitter(const bool Enable)
{
	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	// Get the index of the active interface and call its CWaveIn::SetModulation
	int iDevice		=	m_WaveInList->index;
	return m_WaveInList->pDevice[iDevice]->SetAntiJitter(Enable);
}

// TODO - SetActiveAudioDevice (Realy needed?)
bool CWave2JoystickApp::SetActiveAudioDevice(const TCHAR * AudioDevice)
{
	if (m_CurrMixerDeviceName)
		free(m_CurrMixerDeviceName);
	m_CurrMixerDeviceName = strdup(AudioDevice);

	if (m_CurrMixerDeviceName && strlen(m_CurrMixerDeviceName))
		return true;
	else
		return false;
}

// TODO - SetSelectedInput (Realy needed?)
bool CWave2JoystickApp::SetSelectedInput(const TCHAR * Input)
{
	return false;
}
// TODO - Enable/Disable native SPP mixer manipulation (Realy needed?)
bool CWave2JoystickApp::SetMixerSelector(const bool Enable)
{
	return false;
}

/****************************************************************************************/

/***************** Wave recording - Start/Stop/Play/Status *****************/

// Start recording input audio into a WAV file (Log file is OUTPUT)
bool CWave2JoystickApp::StartWaveLog(TCHAR * LogFileName)
{
	if (!LogFileName)
		return false;

	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	int iDevice		=	m_WaveInList->index;
	TCHAR * Name	=	m_WaveInList->pDevice[iDevice]->StartWaveFileLog();


	strncpy(LogFileName, Name, MAX_PATH);
	free(Name);
	return true;

}

// Start recording/Playback of audio into/from a WAV file
bool CWave2JoystickApp::StopWaveLog(void)
{
	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	// Stop logging
	int iDevice		=	m_WaveInList->index;
	bool out = 	m_WaveInList->pDevice[iDevice]->StopWaveFileLog();

	return out;
}


// Start playback audio from a WAV file (Log file is INPUT)
bool CWave2JoystickApp::PlayWaveLog(TCHAR * LogFileName)
{
	if (!LogFileName)
		return false;

	// We attach the playback of a file to the selected device for simplicity
	// though for playing a wav file there's no need for a device
	int iDevice		=	m_WaveInList->index;
	return m_WaveInList->pDevice[iDevice]->PlayWaveFileLog(LogFileName);
}

// Get status of playback (stat is OUTPUT)
// PLAYING=101 ; STOPPED=102
bool CWave2JoystickApp::StatWaveLog(int * stat)
{
	if (!stat)
		return false;

	int iDevice		=	m_WaveInList->index;
	return m_WaveInList->pDevice[iDevice]->StatWaveFileLog(stat);
}

/***************************************************************************/

/***************** Audio Header log - Start/Stop/Status *****************/
/** 
	Logs the audio headers (WAVEHDR) of every audio buffer
	For every audio buffer it prints:
	* Sound card serial number (Constant for a given sound card)
	* Current buffer serial number - to find a dropped buffer
	* Current buffer flags - should indicate buffer's status
	* Buffer flags for first 8 buffers - to detect processing problems
**/

// Start recording Audio Header into a log file (LogFileName is OUTPUT)
bool CWave2JoystickApp::StartAudioHeaderLog(TCHAR * LogFileName)
{
	if (!LogFileName)
		return false;

	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	int iDevice		=	m_WaveInList->index;
	TCHAR * Name	=	m_WaveInList->pDevice[iDevice]->StartAudioHeaderLog();

	if (!Name)
		return false;

	strncpy(LogFileName, Name, MAX_PATH);
	free(Name);
	return true;

}
// Stop recording Audio Header into a log file
bool CWave2JoystickApp::StopAudioHeaderLog(void)
{
	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	// Stop logging
	int iDevice		=	m_WaveInList->index;
	bool out = 	m_WaveInList->pDevice[iDevice]->StopAudioHeaderLog();

	return out;
}

// Status of recording Audio Header (stat is OUTPUT -> Idle=0, Started=1,	Printing=2,	Stopping=3,	Finish=4)
bool CWave2JoystickApp::StatAudioHeaderLog(int * stat)
{
	if (!stat)
		return false;

	int iDevice		=	m_WaveInList->index;
	return m_WaveInList->pDevice[iDevice]->StatAudioHeaderLog(stat);
}


/************************************************************************/

/******************** Log pulse width and first 8 joystick positions ********************/
/**
	Logs the pulse width of the input data (in number of samples),
	the number of joystick positions and the value of the first 8 joystick positions
	Joystick data refers to that AFTER the filter
**/

// Start recording pulse & position data into a log file (LogFileName is OUTPUT)
bool CWave2JoystickApp::StartPulseLog(TCHAR * LogFileName)
{
	if (!LogFileName)
		return false;

	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	int iDevice		=	m_WaveInList->index;
	TCHAR * Name	=	m_WaveInList->pDevice[iDevice]->StartPulseLog();

	if (!Name)
		return false;

	strncpy(LogFileName, Name, MAX_PATH);
	free(Name);
	return true;

}
// Stop recording pulse & position data 
bool CWave2JoystickApp::StopPulseLog(void)
{
	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	// Stop logging
	int iDevice		=	m_WaveInList->index;
	bool out = 	m_WaveInList->pDevice[iDevice]->StopPulseLog();

	return out;
}

// Status of recording pulse & position data (stat is OUTPUT -> Idle=0, Started=1,	Printing=2,	Stopping=3,	Finish=4)
bool CWave2JoystickApp::StatPulseLog(int * stat)
{
	if (!stat)
		return false;

	int iDevice		=	m_WaveInList->index;
	return m_WaveInList->pDevice[iDevice]->StatPulseLog(stat);
}


/*****************************************************************************************/

/******************** Log  from within function ProcessPulseXXX **************************/
/**
	Logs the pulse width of the input data (in number of samples),
	from within function ProcessPulseXXX hence - before any joystick post-processing (filters)
	The printout deffers from ProcessPulseXXX function to function but always bears the function name
	This is the way to get evidence on which ProcessPulseXXX function was actually called
**/

// Start recording pulse & position data into a log file (LogFileName is OUTPUT)
bool CWave2JoystickApp::StartRawPulseLog(TCHAR * LogFileName)
{
	if (!LogFileName)
		return false;

	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	int iDevice		=	m_WaveInList->index;
	TCHAR * Name	=	m_WaveInList->pDevice[iDevice]->StartRawPulseLog();

	if (!Name)
		return false;

	strncpy(LogFileName, Name, MAX_PATH);
	free(Name);
	return true;

}
// Stop recording pulse & position data 
bool CWave2JoystickApp::StopRawPulseLog(void)
{
	if (!(m_WaveInList && m_WaveInList->index>=0))
		return false;

	// Stop logging
	int iDevice		=	m_WaveInList->index;
	bool out = 	m_WaveInList->pDevice[iDevice]->StopRawPulseLog();

	return out;
}

// Status of recording pulse & position data (stat is OUTPUT -> Idle=0, Started=1,	Printing=2,	Stopping=3,	Finish=4)
bool CWave2JoystickApp::StatRawPulseLog(int * stat)
{
	if (!stat)
		return false;

	int iDevice		=	m_WaveInList->index;
	return m_WaveInList->pDevice[iDevice]->StatRawPulseLog(stat);
}


/*****************************************************************************************/



/*****************************************************************************************/
// Get the audio level (volume) of the currently selected input.
bool CWave2JoystickApp::GetAudioLevel(int * level)
{
	if (!level || !m_WaveInList)
		return false;

	int out;
	int iDevice		=	m_WaveInList->index;
	out =  m_WaveInList->pDevice[iDevice]->GetAudioLevel();
	*level = out;
	return true;
}

// Get the number of joystick positions of the currently selected input.
bool CWave2JoystickApp::GetNumJoyPos(int * nPos)
{
	int iDevice		=	m_WaveInList->index;
	int out = m_WaveInList->pDevice[iDevice]->GetNumJoyPos();
	if (out >0)
	{
		*nPos = out;
		return true;
	}
	else
		return false;
}

// Get the joystick positions (PRE-filter) of the currently selected input.
// If number of positions is over zero then 
// fill-in the pointer that is passed by the caller as follows
// pos[0] = Number of positions (1 - MAX_JS_CH)
// pos[i] = Value of position n+1 ( 0<=n<=MAX_JS_CH)
//
// If the the joystick positions are invalid return false
bool CWave2JoystickApp::GetJoyPos(int * pos)
{
	int * pPos;
	int nPos=0;

	int iDevice		=	m_WaveInList->index;
	if (!m_WaveInList->pDevice[iDevice]->isJoyPosValid())
		return false;

	nPos = m_WaveInList->pDevice[iDevice]->GetJoyPos(&pPos);
	if (nPos>0)
	{
		pos[0] = nPos;
		memcpy(pos+1, pPos, nPos*sizeof(int));
		return true;
	}
	else
		return false;
}

// Get the joystick positions (POST-filter) as sent to FMS
// If number of positions is over zero then 
// fill-in the pointer that is passed by the caller as follows
// pos[0] = Number of positions (1 - MAX_JS_CH)
// pos[i] = Value of position n+1 ( 0<=n<=MAX_JS_CH)
//
// If the the joystick positions are invalid return false
bool CWave2JoystickApp::GetFilteredPos(int * pos)
{
	int iDevice		=	m_WaveInList->index;
	if (!m_WaveInList->pDevice[iDevice]->isJoyPosValid())
		return false;

	if (m_nPostFilterPos>0)
	{
		pos[0] = m_nPostFilterPos;
		memcpy(pos+1, m_PostFilterPos, m_nPostFilterPos*sizeof(int));
		return true;
	}
	else
		return false;
}

//---------------------------------------------------------------------------
//  Stop all WaveIn devices
BOOL CWave2JoystickApp::Stop()
{
	// If no list - do nothing
	if (!m_WaveInList)
		return FALSE;

	/* Loop on all devices until got to end or found (100 is only to prevent endless looping ) */
	for  (int index = 0; index<100; index++)
	{
		if (!m_WaveInList->pDevice[index])
			break;
		m_WaveInList->pDevice[index]->Stop();
		delete (m_WaveInList->pDevice[index]);
		m_WaveInList->pDevice[index] = NULL;
	};
	delete (m_WaveInList->pDevice);
	m_WaveInList->pDevice = NULL;
	delete (m_WaveInList);
	m_WaveInList = NULL;

	return TRUE;
}


//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
//  Initialize all WaveIn devices and start selected device
BOOL CWave2JoystickApp::Start()
{
	/* Start WaveIn */
	if (!m_WaveInList)
		m_WaveInList = InitAllDevices();
	return StartSelectedDevice();
}

/*
	InitAllDevices - Initialize all WAVE IN capture devices

	Loop on all WaveIn devices and create a pointer to WaveInListStruct structure
	Set the current device to WAVE_MAPPER (-1), index 0

	Return:
		Pointer to WaveInList if created or already exists
		NULL if error
*/
WaveInListStruct * CWave2JoystickApp::InitAllDevices(void)
{
	int nWavein, i, id, index;
	WaveInListStruct * WaveInlist;

	/* Get the number of WaveIn Devices */
	nWavein = waveInGetNumDevs();
	if (!nWavein)
		return NULL;

	/* Create the structure */
	WaveInlist = (WaveInListStruct *)calloc(1, sizeof(WaveInListStruct));
	if (!WaveInlist)
		return NULL;
	/* Allocate the array of pointers to the WaveIn structures */
	WaveInlist->pDevice = (CWaveIn **)calloc(nWavein+2, sizeof(CWaveIn*));

	/* Put the default values */
	WaveInlist->id = WAVE_MAPPER; /* Preferred device */
	WaveInlist->index = 0;		/* Index of the WAVE_MAPPER */
	WaveInlist->pDevice[0] = new CWaveIn();
	if (!WaveInlist->pDevice[0]->Init(WAVE_MAPPER))
	{
		delete WaveInlist->pDevice[0];
		return NULL;
	};

	/* Populate the stucture with pointers to the WaveIn structures */
	index =1;
	for (i=0; i<nWavein; i++)
	{
		id = i;
		CWaveIn * WaveIn = new CWaveIn();
		if (WaveIn->Init(id))
		{
			WaveInlist->pDevice[index] = WaveIn;	/* Insert WaveIn with id into the next array item*/
			index++;
		}
		else
			delete WaveIn;

	};

	return WaveInlist;
}


//---------------------------------------------------------------------------
/*
	StartDevice - Start one of the WaveIn devices
	Select one of the WaveIn devices that is listed in WaveInList and start it.

	Parameters:
	*	DeviceId:	WaveIn Device ID (WAVE_MAPPER is a valid parameter)

	Return:
	*	TRUE - if success
	*	FALSE - if Failed

*/
bool CWave2JoystickApp::StartDevice(int DeviceId)
{
	int index;
	if (!m_WaveInList)
		return false;

	/* Loop on all devices until got to end or found (100 is only to prevent endless looping ) */
	for  (index = 0; index<100; index++)
	{
		if (!m_WaveInList->pDevice[index])
			return FALSE; /* Fall off the edge */
		if (m_WaveInList->pDevice[index]->GetId() == DeviceId)
		{
			m_WaveInList->pDevice[index]->Start();
			m_WaveInList->pDevice[index]->SetJoystickEventCallback((void *)SendPositionCallback, (void *)this);
			break;
		}
	};

	m_WaveInList->id = DeviceId;
	m_WaveInList->index = index;
	return true;
}

//---------------------------------------------------------------------------
/*
	StartSelectedDevice - Start the WaveIn device that is selected 
	or the WAVE_MAPPER (Preferred) if none is selected

	Return:
	*	TRUE - if success
	*	FALSE - if Failed
*/
bool CWave2JoystickApp::StartSelectedDevice(void)
{
	int  iWaveInDevice;

	/* Get the ID of the selected WaveIn device*/
	if (!m_CurrMixerDeviceName || !strlen(m_CurrMixerDeviceName))
		iWaveInDevice = WAVE_MAPPER;
	else
		iWaveInDevice = MixerName2WaveInId(m_CurrMixerDeviceName);			/* Replace mixer device Name with wave in ID */

	/* Stop running device */
	bool stopped = StopCurrentDevice();
	if (!stopped)
		return false;

	/* Start the selected (or preferred if none selected) device */
	return StartDevice(iWaveInDevice);
}

bool CWave2JoystickApp::StopCurrentDevice(void)
{
	int index;
	if (!m_WaveInList)
		return false;

	index = m_WaveInList->index;
	bool stopped = m_WaveInList->pDevice[index]->Stop();
	if (!stopped)
		return false;

	m_WaveInList->id = -1;
	m_WaveInList->index = -1;
	return true;
}

/*
	Given a Mixer name return the corresponding WaveIn ID
	Any undefined return -1
*/
int CWave2JoystickApp::MixerName2WaveInId(const TCHAR * MixerName)
{
	__int64  iWaveIn;
	int cWaveIn;
	UINT FAR iMixer;
	MMRESULT res;
	MIXERCAPS caps;

	cWaveIn = waveInGetNumDevs();
	for (iWaveIn=0; iWaveIn<cWaveIn; iWaveIn++)
	{
		res = mixerGetID((HMIXEROBJ)iWaveIn, &iMixer, MIXER_OBJECTF_WAVEIN);
		if (res == MMSYSERR_NOERROR)
		{/* Get Mixer name and compare it with the given one */
			res = mixerGetDevCaps(iMixer, &caps, sizeof(MIXERCAPS));
			if (res == MMSYSERR_NOERROR && !strcmp(MixerName, caps.szPname))
				return (int)iWaveIn;
		};
	};
	return -1;
}

// Send joysick position to Ppjoy device driver
bool CWave2JoystickApp::SendPosition2Ppjoy()
{
	DWORD PpjoySendError;
	bool  success;
	static bool PrevSuccess;

	if (!m_PpjoyObj)
		return false;

	int iDevice = m_PpjoyObj->GetDevId();
	success =  m_PpjoyObj->Send(m_PostFilterPos, m_nPostFilterPos, &PpjoySendError);
	if (!success)
	{		
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, (iDevice<<16)+PpjoySendError);
		m_PpjoyObj->Start(iDevice, &PpjoySendError);
	}
	else
		if (!PrevSuccess)
			PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, (iDevice<<16));

	PrevSuccess = success;
	return success;
}

// Send joysick position to data block from which FMS (winmm.dll) will read the data
bool CWave2JoystickApp::SendPosition2Fms()
{
	if (!m_fms)
		return false;

	return m_fms->SetJoystickPos(m_nPostFilterPos, m_PostFilterPos);
}

// Wrapper function to SendPosition2Ppjoy() & SendPosition2Fms()
bool CWave2JoystickApp::SendPositionCallback(void * ThisObj, int * Position, int * nPositions)
{
	if (!ThisObj)
		return false;

	bool SuccessPpjoy, SuccessFms=FALSE;
	if (!((CWave2JoystickApp *)ThisObj)->ApplyFilter(Position, *nPositions))
		return false;
	SuccessPpjoy = ((CWave2JoystickApp *)ThisObj)->SendPosition2Ppjoy();
	SuccessFms   = ((CWave2JoystickApp *)ThisObj)->SendPosition2Fms();

	return (SuccessFms && SuccessPpjoy);
}

// Apply filter, if needed to the raw joystick data
// This function updates the following members:
//		m_nPostFilterPos:	Number of post-filter joystick positions
//		m_PostFilterPos:	Post-filter joystick positions
bool  CWave2JoystickApp::ApplyFilter(int * Position, int nPositions)
{
	m_nPostFilterPos = nPositions;
	m_PostFilterPos = m_Filters->Filter(Position, &m_nPostFilterPos, 1023, 0);
	if (!m_PostFilterPos)
		return false;
	else
		return true;
}


// Start PPJoy interface
// Send a system-wide status message 
bool CWave2JoystickApp::StartPPJoyInterface(int iDevice)
{
	DWORD PpjoyCreateError;
	bool  success;
	
	/* Make sure we could open the device! */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);

	if (m_PpjoyObj)
		delete m_PpjoyObj;
	m_PpjoyObj = new CPpjoyIf();
	success =  m_PpjoyObj->Start(iDevice, &PpjoyCreateError);
	if (!success)
	{
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, (iDevice<<16)+PpjoyCreateError);
		delete m_PpjoyObj;
		m_PpjoyObj = NULL;
	}
	else
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, iDevice<<16);

	return success;
}

// Stop PPJoy interface
// Send a system-wide status message 

bool CWave2JoystickApp::StopPPJoyInterface(int iDevice)
{
	DWORD PpjoyStopError;
	bool  success;

	if (!m_PpjoyObj)
		return false;

	success =  m_PpjoyObj->Stop(&PpjoyStopError);
	if (!success)
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, (iDevice<<16)+MSG_DPPJSTAT_NOTCN);
	else
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, (iDevice<<16)+MSG_DPPJSTAT_DISCN);

	return success;
}
bool CWave2JoystickApp::TestPPJoyInterface(int iDevice)
{
	if (!m_WaveInList)
		return false;

	int index = m_WaveInList->index;
	CWaveIn * device = m_WaveInList->pDevice[index];
	if (!device)
		return false;
	if (!m_PpjoyObj)
	{
			if (!StartPPJoyInterface(iDevice))
				return false;
	};

	return device->SendJoystickData(true);
}


bool CWave2JoystickApp::SetFilterInterface(void * object)
{
	m_Filters = (CFilterIf *)object;
	if (!m_Filters)
		return false;
	else
		return true;
}

CWave2JoystickApp::~CWave2JoystickApp(void)
{
	int index=0;

	if (m_WaveInList)
	{
		while (m_WaveInList->pDevice[index])
			delete m_WaveInList->pDevice[index++];
	};

	if (m_CurrMixerDeviceName)
		free(m_CurrMixerDeviceName);
}


/**********************************************************************************************
							DLL Interface

	All interface functions are of type PASCAL
	Every function begins with 'AFX_MANAGE_STATE(AfxGetStaticModuleState());' 
	(See beginning of this file for clarifications)
	The interface functions must also be declared in file Wave2Joystick.def

	Most functions call parallel functions in object 'theApp' of class CWave2JoystickApp
	Function 'SppSetParam(const int param, const void * pValue)' serves as a 
	generic entry point to this module:
	 'param':	Function
	 'pValue':	Pointer to parameter of above Function

**********************************************************************************************/
LONG PASCAL TestTest()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return 0;
}

// Get a pointer to the CWave2JoystickApp object
CWave2JoystickApp * PASCAL GetWave2JoystickObject()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return &theApp;
}

// Stop PPJoy+FMS interfaces
BOOL PASCAL StopInterfaces(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.StopInterfaces();
}

// InitAllDevices - Initialize all WAVE IN capture devices
// Set the current device to WAVE_MAPPER (-1)  = index 0
BOOL PASCAL SppStart(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.Start();
}

// Stop all WAVE IN capture devices
BOOL PASCAL SppStop(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.Stop();
}


////////// Interface to GUI - Functions that reflect state of GUI //////////
// Generic function - to be called by all other GUI-interface functions
// Not intended for use by external modules
//
// After initialization - call the parallel function in object 'theApp' of class CWave2JoystickApp
BOOL PASCAL SppSetParam(const int param, const void * pValue)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!theApp)
		return FALSE;
	return theApp.SetParam(param, pValue);
}

// Set the modulation type: PPM, JR(PCM) etc
BOOL PASCAL SppSetModulation(const TCHAR * Modulation)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(MOD, (void *)Modulation);
}

// Set the modulation shift type: Auto, Negative or Positive
BOOL PASCAL SppSetShift(const int Shift)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(SHIFT, (void *)&Shift);
}

// Enable/Disable anti-jitter functionality status (On/Off)
BOOL PASCAL SppSetAntiJitter(const BOOL Enable)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(JITTER, (void *)&Enable);
}

// Set the active audio device to which the Tx is connected
BOOL PASCAL SppSetActiveAudioDevice(const TCHAR * AudioDevice)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(DEVICE, (void *)AudioDevice);
}

// Set the selected audio input (in the selected audio device) to which the Tx is connected
BOOL PASCAL SppSetSelectedInput(const TCHAR * Input)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(INPUT, (void *)Input);
}

// Enable/Disable native SPP mixer manipulation
BOOL PASCAL SppSetMixerSelector(const BOOL Enable)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(MIXER, (void *)&Enable);
}

// Start logging into WAVE file. Output file name in supplied buffer
BOOL PASCAL SppWaveLoggerStart(TCHAR * FileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(WAVSTART, (void *)FileName);
}

// Stop logging into WAVE file. Need to supply file name. NULL will result in default value
BOOL PASCAL SppWaveLoggerStop(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(WAVSTOP, NULL);
}

// Start payback a WAVE file. File name in supplied buffer
BOOL PASCAL SppWaveLoggerPlay(TCHAR * FileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(WAVPLAY, (void *)FileName);
}

// Get status of the logger
BOOL PASCAL SppWaveLoggerGetStat(int * stat)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(WAVSTAT, (void *)stat);
}


// Start logging Audio Headers. Output file name in supplied buffer
BOOL PASCAL SppAudioHdrLoggerStart(TCHAR * FileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(AHSTART, (void *)FileName);
}

// Stop logging Audio Headers. Need to supply file name. NULL will result in default value
BOOL PASCAL SppAudioHdrLoggerStop(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(AHSTOP, NULL);
}

// Get status of the Audio Headers log
BOOL PASCAL SppAudioHdrLoggerGetStat(int * stat)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(AHSTAT, (void *)stat);
}

// Start logging Audio Pulses. Output file name in supplied buffer
BOOL PASCAL SppPulseLoggerStart(TCHAR * FileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(PLSSTART, (void *)FileName);
}

// Stop logging Audio Headers. Need to supply file name. NULL will result in default value
BOOL PASCAL SppPulseLoggerStop(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(PLSSTOP, NULL);
}


// Get status of the Audio Headers log
BOOL PASCAL SppPulseLoggerGetStat(int * stat)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(PLSSTAT, (void *)stat);
}

// Start logging Raw Pulses from inside function ProcessPulseXXX. Output file name in supplied buffer
BOOL PASCAL SppRawPulseLoggerStart(TCHAR * FileName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(RPLSSTART, (void *)FileName);
}

// Stop logging Raw Pulses from inside function ProcessPulseXXX. Need to supply file name. NULL will result in default value
BOOL PASCAL SppRawPulseLoggerStop(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(RPLSSTOP, NULL);
}

// Get status of the pulse data from inside function ProcessPulseXXX
BOOL PASCAL SppRawPulseLoggerGetStat(int * stat)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(RPLSSTAT, (void *)stat);
}







BOOL PASCAL StartPPJoyInterface(int index)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.StartPPJoyInterface(index);
}

BOOL PASCAL StopPPJoyInterface(int index)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.StopPPJoyInterface(index);
}

BOOL PASCAL TestPPJoyInterface(int index)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.TestPPJoyInterface(index);
}


BOOL PASCAL SetFilterInterface(void * object)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return theApp.SetFilterInterface(object);
}

// Monitoring
BOOL PASCAL SppGetAudioLevel(int * AudioLevel)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(AUDIOLEVEL, (void *)AudioLevel);
}

BOOL PASCAL SppGetNumJoyPos(int * nPos)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(NJOYPOS, (void *)nPos);
}

BOOL PASCAL SppGetJoyPos(int * Pos)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(JOYPOS, (void *)Pos);
}

BOOL PASCAL SppGetFilteredPos(int * Pos)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return SppSetParam(POSTFLTR, (void *)Pos);
}
