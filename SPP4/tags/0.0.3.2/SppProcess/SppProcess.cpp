// SppProcess.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <StrSafe.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <Windowsx.h>
#include "SmartPropoPlus.h"
#include "vJoyInterface.h"
#include "public.h"
#include "SppAudio.h"
#include "PulseScope.h"
#include "WinMessages.h"
#include "SppProcess.h"

/* Globals */
int gDebugLevel = 0;
FILE * gCtrlLogFile = NULL;


SPPMAIN_API CSppProcess::CSppProcess() :
	m_PropoStarted(false),
	m_pSharedBlock(NULL),
	m_MixerName(NULL),
	m_JsChPostProc_selected(-1),
	m_hMutexStartStop(NULL),
	m_closeRequest(FALSE),
	m_tCapture(NULL),
	m_waveRecording(FALSE),
	m_tCaptureActive(FALSE),
	m_chMonitor(FALSE),
	m_Audio(NULL),
	m_ChangeCapture(FALSE),
	m_hParentWnd(NULL),
	m_vJoyReady(false),
	m_vJoyDeviceId(1),
	m_DbgPulse(FALSE),
	ProcessChannels(NULL),
	m_Mapping(0x12345678),
	m_iActiveProcessPulseFunc(0),
	m_WaveNChannels(2),
	m_WaveBitsPerSample(8),
	m_WaveRate(192000), 
	m_SelectedMod(TEXT("PPM")),
	m_CurrentPP( [=] (int width, BOOL input) {NULL;}),
	m_fPulseMonitor( [=] (int index, int length, bool low, LPVOID timestamp, LPVOID Param) {NULL;}),
	m_WaveInputChannel(0),
	m_nChannels(0),
	m_PulseMonitor(NULL),
	m_PulseScopeObj(NULL),
	m_PosUpdateCounter(0),
	m_PosUpdateCounterFactor(25),
	m_JoyQual(0),
	m_vPulsesIndex(0)
{
	UINT nCh = sizeof(m_Position)/sizeof(m_Position[0]);
	for (UINT i=0; i<nCh; i++)
		m_Position[i] = 0;

	SetDefaultBtnMap(m_BtnMapping);
	m_vJoyPosition.bDevice = 0;

	m_vPulses[0].clear();
	m_vPulses[1].clear();

	m_DecoderStruct.start = m_DecoderStruct.stop = m_DecoderStruct.running = m_DecoderStruct.forever = FALSE;
	m_DecoderStruct.timeout = FALSE;
}

SPPMAIN_API CSppProcess::~CSppProcess()
{
	Stop();
	StopCaptureAudio();
	m_ModulationMap.clear();
}


SPPMAIN_API bool CSppProcess::RegisterPulseMonitor(int index, bool Register)
{
/* Registration of callback function that dispays the pulse data on the monitor*/
	if (Register)
	{
		m_PulseScopeObj = InitPulseScope(m_hParentWnd);
		m_fPulseMonitor = (SCP)Pulse2Scope; // cast to SCP
		m_PulseMonitor = m_PulseScopeObj;
		m_iPulseMonitor = index;
	}
	else if (m_iPulseMonitor == index)
	{
		m_fPulseMonitor = [=] (int index, int length, bool low, LPVOID timestamp, LPVOID Param){NULL;}; // cast to SCP
		m_PulseMonitor =  NULL;
		DeletePulseScope(m_PulseScopeObj);
	};

	return true;
}


SPPMAIN_API void CSppProcess::SelectMod(LPCTSTR ModType)
{
	if (!ModType)
		return;

	MODMAP::iterator it;
	it = m_ModulationMap.find(ModType);
	if (it == m_ModulationMap.end())
		return;

	MOD tmp = it->second;
	m_CurrentPP =  tmp.func;
	m_PosUpdateCounterFactor = tmp.Qreset;
	return;
}

SPPMAIN_API void CSppProcess::StartDbgPulse(void)
{
	m_DbgPulse = TRUE;
}

SPPMAIN_API void CSppProcess::StopDbgPulse(void)
{
	m_DbgPulse = FALSE;
}

SPPMAIN_API void CSppProcess::vJoyReady(bool ready)
{
	m_vJoyReady = ready;
}

SPPMAIN_API void CSppProcess::vJoyDeviceId(UINT rID)
{
	m_vJoyDeviceId=rID;
}

// Called to inform SPP that a filter has been selected or diselected
// iFilter is the filter index
// If ifilter==-1 the no filter selected
SPPMAIN_API void CSppProcess::SelectFilter(int iFilter, LPVOID pProcessChannels)
{
	if (iFilter == m_JsChPostProc_selected)
		return; // Filter not changed - NOOP

	// Update selected filter
	m_JsChPostProc_selected = iFilter;

	if (m_JsChPostProc_selected == -1)
	{
		ProcessChannels = NULL;
		return;
	};

	ProcessChannels = (PJS_CHANNELS (WINAPI * )(PJS_CHANNELS, int max, int min))pProcessChannels;
	LogMessage(INFO, IDS_I_FILTERSELOK);

}

SPPMAIN_API bool CSppProcess::Stop(void)
{
	// Nothing to kill?
	if (!m_PropoStarted)
		return false;

	// Stop Monitoring
	m_closeRequest = TRUE;

	// Wait for thread to stop
	if (m_tMonitorCapture->joinable())
		m_tMonitorCapture->join();
	delete(m_tMonitorCapture);
	m_tMonitorCapture = NULL;

	// Inform parent
	PostMessage(m_hParentWnd, WMSPP_PRCS_ALIVE, FALSE ,0);
	return true;
}

SPPMAIN_API bool CSppProcess::Init(HWND hParentWnd)
{
	m_hParentWnd = hParentWnd;
	return (m_hParentWnd != NULL);
}

SPPMAIN_API bool CSppProcess::Start(void)
{
	LPCTSTR AudioId = NULL;

	// Start only once
	if (m_PropoStarted)
		return false;
	m_PropoStarted = true;

	if (!m_hParentWnd)
		return false;


	// Get list of modulation types: PPM/PCM(JR) ....
	// Mark the selected modulation type
	// Initialize the database (map) of all modilation types along with the associated functions
	int n	 =	InitModulationMap();
	bool ini =	InitModulationSelect();


	//// Audio system from registry
	//// Commented out becaus currently SPP uses default device
	//if (GetCurrentAudioState())			// Get  Mixer Device (selected or preferred) 
	//	m_MixerName = GetCurrentMixerDevice();	// Selected

	// Get selected audio capture endpoint id from the parent window
	AudioId = (LPCTSTR)SendMessage(m_hParentWnd, WMSPP_PRCS_GETID, 0, 0);


	// Create shared memory block and fill it with:
	// - List of modulations as acquired above
	// - Other default values
	//m_pSharedBlock = CreateSharedDataStruct(NULL);
	//if (!m_pSharedBlock)
	//	return false;

	// Pass to the parent window the list modulations
	SendModInfoToParent(m_hParentWnd);

	// Get Debug level from the registry (if exists) and start debugging 
	// TODO: Implement later - gDebugLevel = GetDebugLevel();


	//// Initialize the mutex than prevents simultaneous Start/Stop streaming
	//m_hMutexStartStop = CreateMutex(NULL, FALSE, MUTEX_STOP_START);
	//if (!m_hMutexStartStop)
	//	return false;


	// Start a thread that listens to the GUI
	m_closeRequest = FALSE;
	m_tMonitorCapture = new thread(MonitorCaptureStatic, this);
	if (!m_tMonitorCapture)
		return false;


	return true;

}

SPPMAIN_API void CSppProcess::SetAudioObj(class CSppAudio * Audio)
{
	m_Audio = Audio;
}

// Return the calculated value of the Quality of Position data
// 0 means that the data is worthless
// 100  means that the data is excellent
// Called every 109-110mS (Meassured)
// Calculate a new value once in 10 calls
SPPMAIN_API int  CSppProcess::GetPositionDataQuality(void)
{
	static UINT CallCounter=0, quality = 100;

	// Increment counter - on tenth call: Reset counter and m_PosUpdateCounter and update value;
	CallCounter++;
	if (CallCounter>2)
	{
		// Update value
		quality = m_PosUpdateCounter * m_PosUpdateCounterFactor / 2 ; 
		if (quality>100)
			quality=100;

		// Reset
		CallCounter = m_PosUpdateCounter=0;
	}
	
	return quality;
}

// Return the calculated value of the Quality of Joystick communication
// 0 means that the data is worthless
// 100  means that the data is excellent
SPPMAIN_API int  CSppProcess::GetJoystickCommQuality(void)
{
	if (m_JoyQual)
		m_JoyQual-=10;

	if (m_JoyQual>100)
		m_JoyQual=100;
 		
	return m_JoyQual;
}

// Start (TRUE), search until detection (FALSE), or until timeout (3000mS)

// Start/Stop scanning for encoding type
// Parameters:
//	Start: TRUE(start)/FALSE(stop)
//	Forever: TRUE(Scan until requested to stop)/False(Scan until detection or timeout)
//  Timeout: 0(No timeout)/Positive number(Timeout in milliSeconds)
SPPMAIN_API void  CSppProcess::SetDecoderScanning(BOOL Start, BOOL Forever, int Timeout)
{
	// It is assumed that PollChannels() is running
	if (!m_chMonitor) // TODO: Warning message
		return;

	// Stop?
	if (!Start && m_DecoderStruct.running)
	{
		m_DecoderStruct.start = FALSE;
		m_DecoderStruct.stop = TRUE;
		m_DecoderStruct.rstbuf = TRUE;
		return;
	};

	// Start?
	if (Start && !m_DecoderStruct.running)
	{
		if (Forever)
		{
			m_DecoderStruct.forever = TRUE;
			m_DecoderStruct.timeout = FALSE;
		} // Forever
		else
		{ // Not Forever 
			if (Timeout)
			{
				std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
				m_DecoderStruct.expire = now + std::chrono::milliseconds(Timeout);			
				m_DecoderStruct.timeout = TRUE;
			} // Timeout
			else
			{  // No timeout
				m_DecoderStruct.timeout = FALSE;
			}; // No timeout

		m_DecoderStruct.forever = FALSE;
		};// Not Forever

		m_DecoderStruct.rstbuf = TRUE;
		m_DecoderStruct.stop   = FALSE;
		m_DecoderStruct.start  = TRUE;
	}; // Start
}


// Wrapper to DetectCurrentEncoding()
// Called repetedly, and calls DetectCurrentEncoding() according to external requests and internal status
inline void  CSppProcess::GetEncoding(void)
{
	TCHAR * Decoder = NULL;
	HRESULT res = S_OK;

	// If request to start (and not running) then set 'running' and remove request
	if (!m_DecoderStruct.running && m_DecoderStruct.start)
	{
		m_DecoderStruct.start = FALSE;
		m_DecoderStruct.running = TRUE;
	} else

	// If request to stop and running then reset 'running' and instruct to clear buffers
	if (m_DecoderStruct.running && m_DecoderStruct.stop)
	{
		m_DecoderStruct.rstbuf = TRUE;
		m_DecoderStruct.running = FALSE;
		m_DecoderStruct.stop = FALSE;
		return;
	}

	// Here we do the Detection
	if (m_DecoderStruct.running)
		res = DetectCurrentEncoding(Decoder);
	else
		return;

	// Detected - inform higher levels
	// Stop if not to run forever
	if (res == S_OK)
	{
		SendMessage(m_hParentWnd, WMSPP_PRCS_DCDR,  (WPARAM)Decoder, 0);
		if (m_DecoderStruct.forever)
			m_DecoderStruct.rstbuf = TRUE;
		else
			m_DecoderStruct.stop = TRUE;

	}

	// Timeout? - Inform CU
	if (m_DecoderStruct.timeout && m_DecoderStruct.expire < std::chrono::system_clock::now())
	{
		m_DecoderStruct.stop = TRUE;
		SendMessage(m_hParentWnd, WMSPP_PRCS_DCDR,  (WPARAM)NULL, 0);
	};
}

// Store pulse in pulse buffer for future analysis
// Select buffer according to value of member m_vPulsesIndex
void CSppProcess::StorePulse(UINT PulseLength, bool Negative)
{
	static UINT vPulsesIndex;

	
	if (m_DecoderStruct.rstbuf)
	{
		m_vPulses[0].clear();
		m_vPulses[1].clear();
		m_DecoderStruct.rstbuf = FALSE;
	};

	if (!m_DecoderStruct.running)
		return;

	// Case we need to start a new buffer
	if (vPulsesIndex != m_vPulsesIndex)
	{
		vPulsesIndex = m_vPulsesIndex;
		m_vPulses[vPulsesIndex].clear();
	};

	// If negative pulse then negate value
	if (Negative)
		m_vPulses[vPulsesIndex].push_back(0-PulseLength);
	else
		m_vPulses[vPulsesIndex].push_back(PulseLength);

	// Safety valve - if buffer becomes too large just switch buffers
	if (m_vPulses[vPulsesIndex].size() > MAX_BUF_SIZE)
	{
		if (m_vPulsesIndex) 
			m_vPulsesIndex=0;
		else
			m_vPulsesIndex=1;
	};
}

// Detect the current encoding of the transmitter signal (Modulation)
// Changes the Type of the encoding (e.g. MOD_TYPE_PPM) if detected or NULL if failed to detect
// Return values:
//	S_OK: Type was found and valid
//  ERROR_INVALID_DATA: Data is invalid of insufficient - Type is invalid
//  
 HRESULT CSppProcess::DetectCurrentEncoding(TCHAR*& Type)
{
#define MAX_N_SYNCS 4
	std::vector<int> buf;
	UINT iPrevBuf, size, SyncPulseSize=0;
	UINT SyncPulse[MAX_N_SYNCS] = {0};

	// Make an internal copy of the previous buffer to work on.
	iPrevBuf = (m_vPulsesIndex-1)*(m_vPulsesIndex-1);
	if (m_vPulses[iPrevBuf].size()<100)
		return ERROR_INVALID_DATA; // Not enough data to work on
	else
		buf = m_vPulses[iPrevBuf];

	// Last sanity check
	if (buf.size()<100)
		return ERROR_INVALID_DATA; // Not enough data to work on
	size = (UINT)buf.size();

	///// Here we start the detection (Pulse length is normalized to 192K sampling rate).
	//
#pragma region Loop: Get the length of the sync pulse
	// Find the size of the sync pulse
	for (UINT i=0; i<size; i++)
	{
		// Absolute values only
		UINT pulse = abs(buf[i]);

		// Ignore if pulse is terribly long
		if (pulse > 4224)
			continue;

		// Detect longest pulses that are under 22mS (4224 ticks)
		// If pulse  longer than the length of the sync pulse size then replace. 
		if (pulse > SyncPulseSize)
			SyncPulseSize = pulse;

	}; // for loop: Find the size of the sync pulse
#pragma endregion

#pragma region Find the first and second sync pulses
		UINT iSyncPulse[2]={0};
		// This will enable us to constract a frame and extract all data from it
		for (UINT i=0; i<size; i++)
		{
			// Absolute values only
			UINT pulse = abs(buf[i]);

			// The synch pulse size changes with the movement of the stick
			if (pulse >= SyncPulseSize*0.95)
			{
				if (!iSyncPulse[0])
					iSyncPulse[0] = i;
				else if (!iSyncPulse[1])
					iSyncPulse[1] = i;
				else
					break;
			};
		}; // for loop

#pragma endregion

	/// If this is a long (Over 3mS) sync pulse then it is a PPM encoding
	if (SyncPulseSize>576)
	{  // PPM
		
		if (!iSyncPulse[1])
			return ERROR_INVALID_DATA; // Sync pulses are NOT of the same polarity

#pragma region Make sure that this is really a PPM signal
		// Make sure that this is really a PPM signal by verifying that
		// 1. Sync pulses are of the same polarity
		// 2. The Separators are of the same size and around 0.5mS

		// 1.
		if ((buf[iSyncPulse[0]] * buf[iSyncPulse[1]]) < 0)
			return ERROR_INVALID_DATA; // Sync pulses are NOT of the same polarity
		// 2.
		UINT separator[3] = {abs(buf[iSyncPulse[1] -1]), abs(buf[iSyncPulse[1] -3]), abs(buf[iSyncPulse[1] -5])};
		UINT avrage_separator = (separator[0] + separator[1] + separator[2])/3;
		if (separator[0] > avrage_separator*1.1)
			return ERROR_INVALID_DATA; // separator pulses are NOT of the same size
		if (separator[1] > avrage_separator*1.1)
			return ERROR_INVALID_DATA; // separator pulses are NOT of the same size
		if (separator[2] > avrage_separator*1.1)
			return ERROR_INVALID_DATA; // separator pulses are NOT of the same size
		if (separator[0] < avrage_separator*0.9)
			return ERROR_INVALID_DATA; // separator pulses are NOT of the same size
		if (separator[1] < avrage_separator*0.9)
			return ERROR_INVALID_DATA; // separator pulses are NOT of the same size
		if (separator[2] < avrage_separator*0.9)
			return ERROR_INVALID_DATA; // separator pulses are NOT of the same size

#pragma endregion

		// Walkera PPM or normal PPM?
		if (avrage_separator > 90)
			Type = MOD_TYPE_PPMW;
		else
			Type = MOD_TYPE_PPM;
		
		return S_OK;// PPM OK

	}  // PPM
	else
	{  // PCM

		// Calculate frame period
		UINT Acc=0;
		for (UINT i=iSyncPulse[0]+1; i<=iSyncPulse[1]; i++)
			Acc += abs(buf[i]);


#pragma region Futaba PCM
		// Futaba PCM
		if 
			(
			//Sync Pulse is 2.7mS  +/- 5%
			(SyncPulseSize <= 1.05*518) &&
			(SyncPulseSize >= 0.95*518) &&

			//Sync pulses polarity alternate
			//((buf[iSyncPulse[0]] * buf[iSyncPulse[1]]) < 0) &&

			//Period is 28mS +/- 5%
			(Acc <= 5376*1.05) &&
			(Acc >= 5376*0.95)
			)
		{
			Type = MOD_TYPE_FUT;
			return S_OK;
		}
#pragma endregion

#pragma region Sanwa/Airtronics PCM1
		// Sanwa/Airtronics PCM1
		else if 
			(
			// Search Sync Pulse (length:  0.78mS) for the entire buffer
			(SyncPulseSize <= 1.05*150) &&
			(SyncPulseSize >= 0.95*150) && 

			//Period is 14mS +/- 5%
			(Acc <= 2688*1.05) &&
			(Acc >= 2688*0.95)
			)
		{
			Type = MOD_TYPE_AIR1;
			return S_OK;
		}
#pragma endregion

#pragma region Sanwa/Airtronics PCM2
		// Sanwa/Airtronics PCM2
		else if 
			(
			// Search Sync Pulse (length:  2.1mS) for the entire buffer
			(SyncPulseSize <= 1.05*403) &&
			(SyncPulseSize >= 0.95*403) && 

			//Period is 22mS +/- 5%
			(Acc <= 4224*1.05) &&
			(Acc >= 4224*0.95)
			)
		{
			Type = MOD_TYPE_AIR2;
			return S_OK;
		}
#pragma endregion

#pragma region JR PCM
		// JR PCM
		// The sync pulse is not the longest so we look for sync pulses of  0.4125mS
		// This will enable us to constract a frame and extract all data from it
		iSyncPulse[0] = iSyncPulse[1] = Acc =0;
		for (UINT i=0; i<size; i++)
		{
			// Absolute values only
			UINT pulse = abs(buf[i]);

			// The synch pulse size changes with the movement of the stick
			if ((pulse >= 79*0.95) && (pulse <= 79*1.05))
			{
				if (!iSyncPulse[0])
					iSyncPulse[0] = i;
				else if (!iSyncPulse[1])
					iSyncPulse[1] = i;
				else
					break;
			};
		}; // for loop

		// Calculate frame period
		for (UINT i=iSyncPulse[0]+1; i<=iSyncPulse[1]; i++)
			Acc += abs(buf[i]);

		if 
			//Period is 44mS +/- 5%
			(
				(Acc <= 8448*1.05) &&
				(Acc >= 8448*0.95)
			)
		{
			Type = MOD_TYPE_JR;
			return S_OK;
		};  
#pragma endregion


	}; //PCM

	// DEBUG
	return ERROR_INVALID_DATA;
}

/*
	Tests status of the global data block 10 times a second
	If changed the make the required change in the operation of this unit
*/
void CSppProcess::MonitorCapture(void)
{
	HRESULT hr = S_OK;

	// Loop while close request has not arrived
	while (!m_closeRequest)
	{
		// Test every 100mSec
		Sleep(100);


		// Conditions that meen that there's nothing to do so keep on listening
		if (!m_ChangeCapture)
			continue;

		/* Request to change device - kill capture thread */
		LogMessage(INFO, IDS_I_AUDIOCHANGE);
		if (m_tCapture && m_ChangeCapture)
		{
			//SetSwitchMixerRequestStat(SharedDataBlock::MDSTAT::STOPPING);
			StopCaptureAudio();
			continue;
		};


		// >> Capture audio device changed - start streaming
		if ( !m_tCapture && m_ChangeCapture)
		{
			LogMessage(INFO, IDS_I_STARTSTREAM);
			LPCTSTR Id = (LPCTSTR)SendMessage(m_hParentWnd, WMSPP_PRCS_GETID, 0, 0);
			m_WaveInputChannel = (int)SendMessage(m_hParentWnd, WMSPP_PRCS_GETLR, 0, 0);
			m_waveRecording = TRUE;
			m_ChangeCapture = FALSE;
			if (!Id || !m_Audio->StartStreaming((PVOID)Id))
				LogMessage(ERR, IDS_E_STREAMFAIL);
			else
			{
				// TODO: ReportChange();
				m_tCapture =  new thread(CaptureAudioStatic, this);
				if (!m_tCapture)
					LogMessage(ERR, IDS_E_STREAMTHRDFAIL);
				//else
					LogMessage(INFO, IDS_I_STREAMTHRDOK);
			};
		}; // << Capture audio device changed - start streaming

	}; // While  loop

	StopCaptureAudio();
	m_PropoStarted = false;
}


void CSppProcess::CaptureAudio(void)
{
	PBYTE	buffer=NULL;
	UINT	bSize=0;
	HRESULT	hr = S_OK;
	UINT	bMax = 8000; // TODO: Change to dynamic size

	buffer = new BYTE[bMax];

	// Get the wave rate for this audio source
	m_WaveRate			=	(UINT)SendMessage(m_hParentWnd, WMSPP_PRCS_GETSPR, 0, 0);
	m_WaveBitsPerSample	=	(UINT)SendMessage(m_hParentWnd, WMSPP_PRCS_GETBPS, 0,0);
	m_WaveNChannels		=	(UINT)SendMessage(m_hParentWnd, WMSPP_PRCS_GETNCH, 0,0);
	m_WaveInputChannel	=	(int)SendMessage(m_hParentWnd, WMSPP_PRCS_GETLR, 0,0);

	LogMessage(INFO, IDS_I_CAPTURESTART);
	while (m_waveRecording)
	{
		m_tCaptureActive = TRUE;
		hr = m_Audio->GetAudioPacket(buffer, &bSize, bMax);
		if (hr != S_OK)
		{
			//if (hr = AUDCLNT_E_DEVICE_INVALIDATED)
			//	break;
			continue;
		}


		// Here the processing of the audio is done - report processing to upper levels
		hr = ProcessWave(buffer, bSize);
		if (hr == S_OK)
			PostMessage(m_hParentWnd, WMSPP_PRCS_ALIVE, TRUE ,0);
		else
			break;

		// Test buffer size and adjust if needed
		UINT currsize = bSize*m_WaveBitsPerSample*m_WaveNChannels/8;
		if (currsize != bMax && currsize)
		{
			delete [] (buffer);
			bMax = currsize;
			buffer = new BYTE[bMax];
		}
	}; // While loop - packet get&process 

	delete [] (buffer);
	//SetSwitchMixerRequestStat(SharedDataBlock::MDSTAT::STOPPED);
	m_tCaptureActive = FALSE;
	LogMessage(INFO, IDS_I_CAPTUREABORT);
}

void CSppProcess::StopCaptureAudio(void)
{
	if (!m_tCapture)
		return;

	m_waveRecording = FALSE;
	if (m_tCapture->joinable())
		m_tCapture->join();
	delete(m_tCapture);
	m_tCapture = NULL;
}

void CSppProcess::PollChannels(void)
{
	if (!m_hParentWnd)
		return;

	// Create shadow array of the channel data
	UINT nCh = sizeof(m_Position)/sizeof(m_Position[0]);
	static std::vector<int> vRawChannels (nCh, 0);
	static std::vector<int> vPrcChannels (nCh, 0);
	static UINT StaleCount=0;

	// Poll the channel values and send them over to the parent window
	while(m_chMonitor)
	{
		 //For every Raw channel, check if changed
		 //If changed, POST message at the parent
		for (UINT i=0; i<nCh; i++)
		{
			if ((vRawChannels[i] != m_Position[i]))
			{
				vRawChannels[i] = m_Position[i];
				PostMessage(m_hParentWnd, WMSPP_PRCS_RCHMNT, i,vRawChannels[i]);
			}
		}; // For loop (Raw)

		// Send the number of actual raw channels
		PostMessage(m_hParentWnd, WMSPP_PRCS_NRCHMNT, m_nChannels, 0);

		 //For every Processed channel, check if changed
		 //If changed, POST message at the parent
		for  (UINT i=0; i<nCh; i++)
		{
			if (vPrcChannels[i] != m_PrcChannel[i])
			{
				vPrcChannels[i] = m_PrcChannel[i];
				PostMessage(m_hParentWnd, WMSPP_PRCS_PCHMNT, i,vPrcChannels[i]);
			};
		}; // For loop (Processed)

	
		// Evaliate the pulses to determine the correct encoder
		 GetEncoding();

	Sleep_For(20); // MilliSec

	// Once in a while reset value of channels to ensure refresh of the GUI
	if (StaleCount>100)
	{
		vRawChannels.assign(nCh,0);
		vPrcChannels.assign(nCh,0);
		StaleCount=0;
	}
	else
		StaleCount++;



	}; // While loop
}


void CSppProcess::AudioChanged(void)
{
	m_ChangeCapture = TRUE;
	//SetSwitchMixerRequestStat(SharedDataBlock::MDSTAT::CHANGE_REQ);
}

void CSppProcess::SetAudioChannel(bool Left)
{
	Left ? m_WaveInputChannel=0 : m_WaveInputChannel=1;
}


/*
	Start/Stop sperate thread that monitors the channel (Position[]) values
	The thread polls the channels and sends periodic updates to the parent window
*/
void CSppProcess::MonitorChannels(BOOL Start)
{
	static thread * tMonitor = NULL;
	static bool isRunning = false;
	static std::timed_mutex mtx; // Sync - access to this function is sequential
	std::chrono::milliseconds timeout(100);

	// Sanity check
	if (!m_hParentWnd)
		return;

	// Acquiring the lock for this function
	if (!mtx.try_lock_for(timeout))
		return;

	// Cannot stop if not running and cannot start if already running
	if ((Start && isRunning) || (!Start && !isRunning))
		goto End;

	
	if (Start)
	{// Starting a monitoring thread
		m_chMonitor = true;
		tMonitor = new thread(PollChannelsStatic, this);
		if (!tMonitor)
			goto End;
		isRunning = true;
	}
	else
	{
		m_chMonitor = false;
		if (tMonitor && tMonitor->joinable())
			tMonitor->join();
		isRunning = false;
		delete(tMonitor);
		tMonitor = NULL;
	};

End:
	mtx.unlock();
	return;
}

HRESULT	CSppProcess::ProcessWave(BYTE * pWavePacket, UINT32 packetLength)
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
	UINT rawPulseLength, PulseLength = 0;
	bool negative;

#ifdef _DEBUG
		static std::vector<UINT> raw_pulses;
		static std::vector<UINT> nrm_pulses;
		static int raw_count=0;
#endif

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
		rawPulseLength = PulseLength;
		PulseLength = NormalizePulse(PulseLength);

		// Debug pulse
		if (m_DbgPulse && m_hParentWnd)
			SendDbgPulse(sample,  negative,  rawPulseLength,  PulseLength);

#ifdef _DEBUG
		if (PulseLength)
		{
			if (raw_count<200)
				raw_count++;
			else
			{
				raw_count=0;
				raw_pulses.clear();
				nrm_pulses.clear();
			};
			raw_pulses.push_back(rawPulseLength);
			nrm_pulses.push_back(PulseLength);
		};
#endif

		// If valid pulse the process the pulse
		// TODO (?): Very short pulses are ignored (Glitch)
		if (PulseLength/*>3*/)
		{

			// Call the correct function ProcessPulseXXX() 
			m_CurrentPP(PulseLength, negative);

		// Store pulse in buffer for analysis and debug
		StorePulse(PulseLength, negative);
		}


		if (m_fPulseMonitor && PulseLength>3)
			m_fPulseMonitor(m_iPulseMonitor, PulseLength, negative, 0, m_PulseMonitor);
	};



	EXIT_ON_ERROR(hr);
Exit:
	return hr;
}


// Request the type of selected modulation from CU
// Assign PP function accordingly
bool  CSppProcess::InitModulationSelect(void)
{
	LPTSTR Type = (LPTSTR)SendMessage(m_hParentWnd, WMSPP_PRCS_GETMOD, 0 , 0);
	if (!Type || !wcslen(Type))
		Type = L"PPM";

	MODMAP::iterator it;
	it = m_ModulationMap.find(Type);
	if (it == m_ModulationMap.end())
		return false;

	MOD tmp = it->second;
	m_SelectedMod = Type;
	m_CurrentPP = tmp.func;
	m_PosUpdateCounterFactor = tmp.Qreset;

	return true;
}

// InitModulationMap - Initialize the modulation data base (which is implemented as a map)
// For each modulation supported by SPP, insert an entry and a search key:
//	Search Key is the modulation type (unique)
//	Entry includes:
//	- func: Modulation function
//	- Name: Friendly name of the modulation type (To be displayed)
//	- Type: Modulation type (unique)
//	- Subtype: PPM/PCM
int CSppProcess::InitModulationMap(void)
{
	MOD tmp;
	m_ModulationMap.clear();

	// Generic PPM
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulsePpm(width, input);};
	tmp.Name = MOD_NAME_PPM;
	tmp.Subtype =  _T("PPM");
	tmp.Type = MOD_TYPE_PPM;
	tmp.Qreset = 25;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// PPM (Positive)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseJrPpm(width, input);};
	tmp.Name = MOD_NAME_PPMP;
	tmp.Subtype =  _T("PPM");
	tmp.Type = MOD_TYPE_PPMP;
	tmp.Qreset = 25;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// PPM (Negative)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseFutabaPpm(width, input);};
	tmp.Name = MOD_NAME_PPMN;
	tmp.Subtype =  _T("PPM");
	tmp.Type = MOD_TYPE_PPMN;
	tmp.Qreset = 25;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// PPM (Walkera)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseWK2401Ppm(width, input);};
	tmp.Name = MOD_NAME_PPMW;
	tmp.Subtype =  _T("PPM");
	tmp.Type = MOD_TYPE_PPMW;
	tmp.Qreset = 25;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// JR (PCM)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseJrPcm(width, input);};
	tmp.Name = MOD_NAME_JR;
	tmp.Subtype =  _T("PCM");
	tmp.Type = MOD_TYPE_JR;
	tmp.Qreset = 50;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// Futaba (PCM)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseFutabaPcm(width, input);};
	tmp.Name = MOD_NAME_FUT;
	tmp.Subtype =  _T("PCM");
	tmp.Type = MOD_TYPE_FUT;
	tmp.Qreset = 25;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// Sanwa/Air (PCM1)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseAirPcm1(width, input);};
	tmp.Name = MOD_NAME_AIR1;
	tmp.Subtype =  _T("PCM");
	tmp.Type = MOD_TYPE_AIR1;
	tmp.Qreset = 25;
	m_ModulationMap.emplace(tmp.Type, tmp);

	// Sanwa/Air (PCM2)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseAirPcm2(width, input);};
	tmp.Name = MOD_NAME_AIR2;
	tmp.Subtype =  _T("PCM");
	tmp.Type = MOD_TYPE_AIR2;
	tmp.Qreset = 35;
	m_ModulationMap.emplace(tmp.Type, tmp);


	// Walkera (PCM)
	tmp.func =  [=] (int width, BOOL input) {this->ProcessPulseWalPcm(width, input);};
	tmp.Name = MOD_NAME_WAL;
	tmp.Subtype =  _T("PCM");
	tmp.Type = MOD_TYPE_WAL;
	tmp.Qreset = 50;
	m_ModulationMap.emplace(tmp.Type, tmp);

	return (int)m_ModulationMap.size();
}

inline UINT CSppProcess::Sample2Pulse(short sample, bool * negative)
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

inline UINT CSppProcess::NormalizePulse(UINT Length)
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

/***********************************************************************/
/* ProcessPulse functions - One function for each supported modulation */
/***********************************************************************/
/*
	Process PPM pulse
	A long (over 200 samples) leading high, then a short low, then up to six highs followed by short lows.
	The size of a high pulse may vary between 30 to 70 samples, mapped to joystick values of 1024 to 438
	where the mid-point of 50 samples is translated to joystick position of 731.
*/
 void  CSppProcess::ProcessPulsePpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	char tbuffer [9];
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */

	if (width < PPM_GLITCH)
		return;

	if (gDebugLevel>=2 && gCtrlLogFile && !(_strtime_s( tbuffer, 10 ))/*&& !(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulsePpm(width=%d, input=%d)", tbuffer, width, input);

	/* If pulse is a separator then go to the next one */
	if (width < PPM_SEP|| former_sync)
	{
		former_sync = 0;
		return;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (/*sync == 0 && */width > PPM_TRIG) {
        sync = 1;
		if (datacount)
			m_PosUpdateCounter++;
		m_nChannels = datacount;
        datacount = 0;
		former_sync = 1;
		return;
    }

    if (!sync) 
		return; /* still waiting for sync */

	// Cancel jitter
	if (abs(PrevWidth[datacount] - width) < PPM_JITTER)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input|| m_JsChPostProc_selected!=-1)
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

	
	//if (input|| m_JsChPostProc_selected!=-1)
		m_Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	//else
	//	switch (datacount)
	//{ // Futaba
	//case 0: 	m_Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 1: 	m_Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 2: 	m_Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 3: 	m_Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 4: 	m_Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 5: 	m_Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 6: 	m_Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 7: 	m_Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 8: 	m_Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 9: 	m_Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 10: 	m_Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	//case 11: 	m_Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	//};
				
	// Send Position and number of channels to the virtual joystick
	SendPPJoy(m_nChannels, m_Position);

	if (gDebugLevel>=3 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile," data[%d]=%d", datacount, data[datacount]);

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return;
}

 void  CSppProcess::ProcessPulseFutabaPpm(int width, BOOL input)
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

	if (gDebugLevel>=2 && gCtrlLogFile  && !(_strtime_s( tbuffer, 9 )) /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseFutabaPpm(width=%d, input=%d)", tbuffer , width, input);

	/* If pulse is a separator then go to the next one */
	if (!input || width < PPM_SEP || former_sync)
	{
		former_sync = 0;
		return;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (input &&  width > PPM_TRIG) {
        sync = 1;
		if (datacount)
			m_PosUpdateCounter++;
		m_nChannels = datacount;
        datacount = 0;
		former_sync = 1;
		return;
    }

    if (!sync) return; /* still waiting for sync */

	// Cancel jitter /* Version 3.3.3 */
	if (abs(PrevWidth[datacount] - width) < PPM_JITTER)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input|| m_JsChPostProc_selected!=-1)
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

	
	//if (input|| m_JsChPostProc_selected!=-1)
		m_Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	//else
	//	switch (datacount)
	//{ // Futaba
	//case 0: 	m_Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 1: 	m_Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 2: 	m_Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 3: 	m_Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 4: 	m_Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 5: 	m_Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 6: 	m_Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 7: 	m_Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 8: 	m_Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 9: 	m_Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 10: 	m_Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	//case 11: 	m_Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	//};
			
	// Send Position and number of channels to the virtual joystick
	SendPPJoy(11, m_Position);

	if (gDebugLevel>=3 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile," data[%d]=%d", datacount, data[datacount]);
	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return;
}
 void  CSppProcess::ProcessPulseJrPpm(int width, BOOL input)
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

	if (gDebugLevel>=2 && gCtrlLogFile && !(_strtime_s( tbuffer , 10))/*!(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseJrPpm(width=%d, input=%d)", tbuffer , width, input);

	/* If pulse is a separator then go to the next one */
	if (input || width < PPM_SEP || former_sync)
	{
		former_sync = 0;
		return;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (!input && width > PPM_TRIG) {
        sync = 1;
		if (datacount)
			m_PosUpdateCounter++;
		m_nChannels = datacount;
        datacount = 0;
		former_sync = 1;
		return;
    }

    if (!sync) return; /* still waiting for sync */

	// Cancel jitter /* Version 3.3.3 */
	if (abs(PrevWidth[datacount] - width) < PPM_JITTER)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input || m_JsChPostProc_selected!=-1)
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

	
	//if (input|| m_JsChPostProc_selected!=-1)
		m_Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	//else
	//	switch (datacount)
	//{ // Futaba
	//case 0: 	m_Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 1: 	m_Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 2: 	m_Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 3: 	m_Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 4: 	m_Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 5: 	m_Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 6: 	m_Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 7: 	m_Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 8: 	m_Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 9: 	m_Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	//case 10: 	m_Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	//case 11: 	m_Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	//};
				
	// Send Position and number of channels to the virtual joystick
	SendPPJoy(11, m_Position);

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return ;
}
  //#endif
/*
	Process Pulse for Walkera WK-2401 PPM
	This is just a permiscuous PPM that does not follow the PPM standard

	This is how it works:
	1. Minimal pulse width is 5
	2. Any pulse of over PPMW_TRIG (=200) is considered as a sync pulse. 
	3. Polarity ('input') of the Sync pulse is the polarity of the following data pulses
*/
void  CSppProcess::ProcessPulseWK2401Ppm(int width, BOOL input)
{
    static int sync = 0;
	static BOOL Polarity;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	char tbuffer [9];
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */

	if (width < 5)
		return;

	if (gDebugLevel>=2 && gCtrlLogFile && !(_strtime_s( tbuffer, 9 )))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseWK2401Ppm(width=%d, input=%d)", tbuffer, width, input);

	/* sync is detected at the end of a very long pulse (over  4.5mSec) */
    if (width > PPMW_TRIG) {
        sync = 1;
		if (datacount)
			m_PosUpdateCounter++;
		m_nChannels = datacount;
        datacount = 0;
		Polarity = input;
		return;
    }

    if (!sync) 
		return; /* still waiting for sync */

	// If this pulse is a separator - read the next pulse
	if (Polarity != input)
		return;

	// Cancel jitter /* Version 3.3.3 */
	if (abs(PrevWidth[datacount] - width) < PPM_JITTER)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;


	/* convert pulse width in samples to joystick Position values (newdata)
	joystick Position of 0 correspond to width over 100 samples (2.25mSec)
	joystick Position of 1023 correspond to width under 30 samples (0.68mSec)*/
	newdata = (int)((width - PPMW_MIN) / (PPMW_MAX - PPMW_MIN) * 1024);

	/* Trim values into 0-1023 boundries */
    if (newdata < 0) newdata = 0;
    else if (newdata > 1023) newdata = 1023;

	/* Update data - do not allow abrupt change */
    if (data[datacount] - newdata > 100) data[datacount] -= 100;
    else if (newdata - data[datacount] > 100) data[datacount] += 100;
    else data[datacount] = (data[datacount] + newdata) / 2;

	
	m_Position[datacount] = data[datacount];	/* Assign data to joystick channels */

	// Send Position and number of channels to the virtual joystick
	SendPPJoy(11, m_Position);

	if (gDebugLevel>=3 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile," data[%d]=%d", datacount, data[datacount]);

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return;
}

  //---------------------------------------------------------------------------
/*
	Futaba PCM1024Z format
	Based on the following documents:
		http://www.cg.its.tudelft.nl/~wouter/publications/pasman03k.pdf
		http://www.cg.its.tudelft.nl/~wouter/publications/pasman04b.pdf

	The data is incapsulated in alternating odd/even frames of 160 bits of RAW data
	Before every FRAME there is a 18-bit sync pulse that is followed by a PARITY MARKER.
	ODD parity marker is 00000011. EVEN parity marker is 000011.

	Every 10-bits of raw data are mapped into a 6-bit data SEXTET.
	Every 4 consequtive SEXTETS form a 24-bit PACKET.
	Every PACKET is built as follows:
	 packet[0:1]   - AUX:   Auxilary bits used to modify the meaning of the data
	 packet[2:5]   - DELTA: used to calculate position delta (not used here).
	 packet[6:15]  - POS:   Channel position data (10 bit data)
	 packet[16:23] - ECC:   Error correction code (not used here).

   The first 8 channels (Analog) are mapped like this:
	Ch1: POS[5] (Odd Frame)  Provided AUX[4] != 0x1
	Ch2: POS[4] (Odd Frame)  Provided AUX[4] != 0x1
	Ch3: POS[0] (Even Frame) Provided AUX[0] != 0x0
	Ch4: POS[1] (Even Frame) Provided AUX[0] != 0x0
	Ch5: POS[2] (Even Frame) Provided AUX[0] != 0x0
	Ch6: POS[6] (Odd Frame)  Provided AUX[4] != 0x1
	Ch7: POS[3] (Even Frame) Provided AUX[0] != 0x0
	Ch8: POS[7] (Odd Frame)  Provided AUX[4] != 0x1

   The next channels (binary)  are mapped like this:
    CH9:  
		AUX[3] lower bit = 0 Provided AUX[2] = 0x0
		AUX[11] lower bit = 0 Provided AUX[10] = 0x1
    CH10: 
		AUX[5] lower bit = 0 Provided AUX[4] = 0x0
		AUX[13] lower bit = 0 Provided AUX[12] = 0x1

*/
void CSppProcess::ProcessPulseFutabaPcm(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bit = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;

    static int data[32];
    static int datacount = 0;
	char tbuffer [9];
	static int i = 0;

#ifdef _DEBUG
	static int _d = 0;
	static std::array<int, 1000> _arWidth;
	if (_d<1000)
	{
		_arWidth[_d] = width;
		_d++;
	}
	else
	{
		_d=0;
		_arWidth.fill(-1);
	};
#endif

    width = (int)floor(width / PW_FUTABA);

	/* 
		Sync is determined as 18-bit wide pulse 
		If detected, move state machine to sync=1 (wait for parity marker) 
		and initialize all static params
	*/
    if (sync == 0 && width == 18) {
        sync = 1;
		if (datacount)
			m_PosUpdateCounter++;
        bit = 0;
        bitstream = 0;
        bitcount = 0;
        datacount = 0;		
			if (gDebugLevel>=2 && gCtrlLogFile &&  !( _strtime_s( tbuffer, 9 ))/*!(i++%50)*/)
				fprintf(gCtrlLogFile,"\n%s - ProcessPulseFutabaPcm(%d) %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d - %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d", tbuffer, width,\
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],\
				data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15],\
				data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23],\
				data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31]\
				);
        return;
    }
	
    if (!sync) return;

    bitstream = (bitstream << width) | (bit >> (32 - width));
    bit ^= 0xFFFFFFFF;
    bitcount += width;

	/* 
		Parity marker must follow the sync pulse (sync==1)
		It might take one of two forms:
		- EVEN parity marker is 000011.
		- ODD parity marker is 00000011.
		If either is found mode state machine to sync=2 (read raw data)
	*/
    if (sync == 1) {
        if (bitcount >= 6) {
            bitcount -= 6;
            if (((bitstream >> bitcount) & 0x3F) == 0x03) { /* Even? */
                sync = 2;
                datacount = 0; /* Even: Reset sextet counter to read the first (even) 16 ones */
            } else if (((bitstream >> bitcount) & 0x3F) == 0x00) { /* Odd? */
                sync = 2;
                datacount = 16; /* Odd: Set sextet counter to 16  to read the last (odd) 16 ones */
                bitcount -= 2;
            } else {
                sync = 0;
            }
        }
        return;
    }

	/* 
		Read the next ten bits of raw data 
		Convert then into a sextet
		Increment sextet counter
		If data is illegal reset state machine to sync=0
	*/
    if (bitcount >= 10) {
        bitcount -= 10;
        if ((data[datacount++] = futaba_symbol[(bitstream >> bitcount) & 0x3FF]) < 0) {
            sync = 0;
            return;
        }
    }

	/* 
		Convert sextet data into channel (m_Position) data
		Every channel is 10-bit, copied from POS in the corresponding packet.
		Every channel is calculated only after the corresponding packet is ready (forth sextet is ready).
	*/
    switch (datacount) {
		/* Even frame */
        case 3:  /* 4th sextet of first packet is ready */
			if ((data[0] >> 4) != 0)  
				m_Position[2] = (data[1] << 4) | (data[2] >> 2);    /* Ch3: m_Position: packet[6:15] */
			break;

        case 7:  /* 4th sextet of second packet is ready */                            
			if ((data[0] >> 4) != 0)  
				m_Position[3] = (data[5] << 4) | (data[6] >> 2);    /* Ch4: m_Position: packet[6:15] */
			m_Position[9] = ((data[20]>>4)&1)*512;  /* Ch 10: One of the auxilliary bits */
			break;

        case 11:  /* 4th sextet of 3rd packet is ready */ 
			if ((data[0] >> 4) != 0)  
				m_Position[4] = (data[9] << 4) | (data[10] >> 2);   /* Ch5: m_Position: packet[6:15] */
			break;

        case 15:   /* 4th sextet of 4th packet is ready */
			if ((data[0] >> 4) != 0)  
				m_Position[6] = (data[13] << 4) | (data[14] >> 2);   /* Ch7: m_Position: packet[6:15] */
			m_Position[8] = ((data[12]>>4)&1)*512; /* Ch 9: One of the auxilliary bits */
			sync = 0; /* End of even frame. Wait for sync */
			break;

		/* Odd frame */
        case 19:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				m_Position[1] = (data[17] << 4) | (data[18] >> 2);     /* Ch2: m_Position: packet[6:15] */
			break;
        case 23:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				m_Position[0] = (data[21] << 4) | (data[22] >> 2);     /* Ch1: m_Position: packet[6:15] */
			m_Position[9] = ((data[20]>>4)&1)*512; /* Ch 10: One of the auxilliary bits */
			break;
        case 27:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				m_Position[5] = (data[25] << 4) | (data[26] >> 2);     /* Ch6: m_Position: packet[6:15] */
			break;
        case 31:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				m_Position[7] = (data[29] << 4) | (data[30] >> 2);     /* Ch8: m_Position: packet[6:15] */
			m_Position[8] = ((data[28]>>4)&1)*512; /* Ch 9: One of the auxilliary bits */
			break;
        case 32: sync = 0;/* End of odd frame. Wait for sync */
			
		m_nChannels = 10;  // Fixed number of channels
		SendPPJoy(m_nChannels, m_Position);
    };

	return;
}
void CSppProcess::ProcessPulseJrPcm(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
    static int data[30];
    static int datacount = 0;
	char tbuffer [9];
	static int i = 0;

	if (gDebugLevel>=2 && gCtrlLogFile && !(i++%50) && !(_strtime_s( tbuffer, 9 )))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseJrPcm(%d)", tbuffer, width);


    if (sync == 0 && (int)floor(2.0 * width / PW_JR + 0.5) == 5) {
        sync = 1;
		if (datacount >= 8)
			m_PosUpdateCounter++;
        bitstream = 0;
        bitcount = -1;
        datacount = 0;
        return;
    }

    if (!sync) return;

    width = (int)floor((double)width / PW_JR + 0.5);
    bitstream = ((bitstream << 1) + 1) << (width - 1);
    bitcount += width;

    if (bitcount >= 8) {
        bitcount -= 8;
        if ((data[datacount++] = jr_symbol[(bitstream >> bitcount) & 0xFF]) < 0) {
            sync = 0;
            return;
        }
    }

    switch (datacount) {
        case 3:  m_Position[2] = 1023 - ((data[1] << 5) | data[2]); break;
        case 6:  m_Position[0] = 1023 - ((data[4] << 5) | data[5]); break;
        case 11: m_Position[5] = 1023 - ((data[9] << 5) | data[10]); break;
        case 14: m_Position[7] = 1023 - ((data[12] << 5) | data[13]); break;
        case 18: m_Position[3] = 1023 - ((data[16] << 5) | data[17]); break;
        case 21: m_Position[1] = 1023 - ((data[19] << 5) | data[20]); break;
        case 26: m_Position[4] = 1023 - ((data[24] << 5) | data[25]); break;
        case 29: m_Position[6] = 1023 - ((data[27] << 5) | data[28]); break;
        case 30: sync = 0;
			
		m_nChannels = 8; // Fixed number of channels
		SendPPJoy(m_nChannels, m_Position);
    };

	 return;
}



/*
	Process Airtronics/Sanwa PCM1 pulse (10-bit)
	
	Pulse width may last 1 to 4 bits. Every bit consists an avarege of 9 samples
	This is why the width is devided by 8.

	The data consists of 8 channels in two chunks:
	First Chunk begins with a 4-bit low pulse, second Chunk begins with a 4-bit high pulse
	Every channel data starts at the 10th bit of a chunk and consists of 15bit word. No delimiter between channel data

	Channel data is converted using function "Convert15bits()" (and inverted) into a joystick posision.
*/
void CSppProcess::ProcessPulseAirPcm1(int width, BOOL input)
{
	static int pulse;
	static int sync;
	static int datacount = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
	static int chunk = -1;
	unsigned int		shift;
    static unsigned int data[10];
	char tbuffer [9];
	static int i = 0;
	const int fixed_n_channel = 8;
	m_nChannels = 8;

	if (gDebugLevel>=2 && gCtrlLogFile && i++%10 && !( _strtime_s( tbuffer, 9 )))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseAirPcm1(%d)", tbuffer, width);


	if (width<25)
		return;
	else if (width<50)
		pulse=1;
	else if (width<90)
		pulse=2;
	else if (width<130)
		pulse=3;
	else
		pulse=4;


	if (pulse == 4)  // 4-bit pulse marks a bigining of a data chunk
		{
			if (!input)
			{
				// First data chunk - clear chunnel counter
				datacount = 0;
			}
			else
			{	// Second data chunk - get joystick m_Position from channel data
				m_Position[0] = smooth(m_Position[0], Convert15bits(data[8])); // Elevator (Ch1)
				m_Position[1] = smooth(m_Position[1], Convert15bits(data[7])); // Ailron (Ch2)
				m_Position[2] = smooth(m_Position[2], Convert15bits(data[6])); // Throttle (Ch3)
				m_Position[3] = smooth(m_Position[3], Convert15bits(data[9])); // Rudder (Ch4)
				m_Position[4] = smooth(m_Position[4], Convert15bits(data[1])); // Gear (Ch5)
				m_Position[5] = smooth(m_Position[5], Convert15bits(data[2])); // Flaps (Ch6)
				m_Position[6] = smooth(m_Position[6], Convert15bits(data[3])); // Aux1 (Ch7)
				m_Position[7] = smooth(m_Position[7], Convert15bits(data[4])); // Aux2 (Ch8)

				
				SendPPJoy(fixed_n_channel-1, m_Position);

				
			};
			sync = 1;		// Sync bit is set for the first 10 bits of the chunk (No channel data here)
			bitstream = 0;
			bitcount = -1;
			chunk = input;	// Mark chunk polarity - 0: Low channels, 1: High channels
			//return 0;
				
			if (datacount == 5 && width<160)
				m_PosUpdateCounter++;
		};
		
		if (sync) 
		{
			shift = 9;	// Read the first 10 bits
		}
		else
		{
			shift =15; // Read a channel data
		};

		bitstream = ((bitstream << 1) + 1) << (pulse - 1);
		bitcount += pulse;
		
		if (bitcount >=shift) 
		{
			bitcount -= shift;
			data[datacount] = (bitstream >> bitcount) & 0x7FFF; // Put raw 15-bit channel data
			datacount++;
			sync = 0;
			if (datacount>=fixed_n_channel+2)
				datacount = 0;
		};
		return;
}

/*
	Process Airtronics/Sanwa PCM2 pulse (9-bit)
	
	Pulse width may last 1 to 7 bits. Every bit consists an avarege of 13 samples
	This is why the width is devided by 12.

	The data consists of 6 channels in two chunks:
	First Chunk begins with a 7-bit high pulse, second Chunk begins with a 7-bit low pulse
	Every channel data starts at the 9th bit of a chunk and consists of 20bit word. No delimiter between channel data

	Channel data is converted using function "Convert20bits()" (and inverted) into a joystick posision. 
*/
void CSppProcess::ProcessPulseAirPcm2(int width, BOOL input)
{
	int pulse;
	static int sync;
	static int datacount = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
	static int chunk = -1;
	unsigned int		shift;
    static unsigned int data[10];
	char tbuffer [9];
	static int i = 0;

	if (gDebugLevel>=2 && gCtrlLogFile && !(i++%50) && !(_strtime_s( tbuffer, 9 )))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseAirPcm2(Width=%d, input=%d)", tbuffer, width, input);

	if (width<10)
		return;
	if (width<70)
		pulse =1;
	else if (width<140)
		pulse=2;
	else
		pulse = 7;

	if (pulse == 7)  // 4-bit pulse marks a biginind of a data chunk
		{
			if (!input)
			{
				// First data chunk - clear chunnel counter
				datacount = 0;
			}
			else
			{	// Second data chunk - get joystick m_Position from channel data
				m_Position[0] = smooth(m_Position[0], Convert20bits(data[2])); // Elevator	(Ch1)
				m_Position[1] = smooth(m_Position[1], Convert20bits(data[3])); // Ailerons	(Ch2)
				m_Position[2] = smooth(m_Position[2], Convert20bits(data[6])); // Throtle	(Ch3)
				m_Position[3] = smooth(m_Position[3], Convert20bits(data[7])); // Rudder	(Ch4)
				m_Position[4] = smooth(m_Position[4], Convert20bits(data[1])); // Gear		(Ch5)
				m_Position[5] = smooth(m_Position[5], Convert20bits(data[5])); // Flaps		(Ch6)

				m_nChannels = 6;
				SendPPJoy(m_nChannels, m_Position);

				if (width<420 && width>390)
					m_PosUpdateCounter++;
			};

			sync = 1;		// Sync bit is set for the first 10 bits of the chunk (No channel data here)
			bitstream = 0;
			bitcount = -1;
			chunk = input;	// Mark chunk polarity - 0: Low channels, 1: High channels
			//return 0;
		};
		
		if (sync) 
		{
			shift = 7;	// Read the first 10 bits
		}
		else
		{
			shift =20; // Read a channel data
		};

		bitstream = ((bitstream << 1) + 1) << (pulse - 1);
		bitcount += pulse;
		
		if (bitcount >=shift) 
		{
			bitcount -= shift;
			data[datacount] = (bitstream >> bitcount) & 0xFFFFF; // Put raw 20-bit channel data
			datacount++;
			sync = 0;
			if (datacount>=8)
				datacount = 0;
		};

		return;
}
/*
	Process Walkera PCM pulse (Tested with Walkera WK-0701)

	Major changes in version 3.3.1
	
	Pulse width may be from 11 to 58 samples.
	Pulse of over 56 samples is considered to be a sync pulse.
	A sync pulse is always followed by 49 data pulses.

	Data pulses are of two types:
	Binary pulses and Octal pulses.
	Binary pulses may be:
		0:	11-14 samples
		1:	19-21 samples
	Octal pulses may be:
		0:	11-14 samples
		1:	17-18 samples
		3:	19-21 samples
		4:	24-25 samples
		5:	27-28 samples
		6:	30-32 samples
		7:	34-35 samples
		8:	38-39 samples
*/
void CSppProcess::ProcessPulseWalPcm(int width, BOOL input)
{
	static int nPulse;
	static unsigned char cycle[50];
	int Elevator=0, Ailerons=0,Throttle=0,Rudder=0,Gear=0, Pitch=0, Gyro=0, Ch8=0;
	const int fixed_n_channel = 8;
	m_nChannels = 8;
	int vPulse;
	int * cs;

	width = (int)(width*44.1/192); // Normalize to 44.1K

	/* Detect Sync pulse - if detected then reset pulse counter and return */
	if (width>56)
	{
		nPulse = 1;
		return;
	};

	if (width < 11)
		return;

	/* Even pulses are binary, Odd pulses are Octal */
	if ((nPulse&1))
		vPulse = WalkeraConvert2Bin(width);
	else
		vPulse = WalkeraConvert2Oct(width);
	if (vPulse<8)
		cycle[nPulse] = vPulse;

	nPulse++;

	/* At the end of the 50-pulse cycle - calculate the channels */
	if (nPulse==50)
	{
		/* Channels */
		Elevator	= WalkeraElevator(cycle);	/* Ch1: Elevator */		
		Ailerons	= WalkeraAilerons(cycle);	/* Ch2: Ailerons */		
		Throttle	= WalkeraThrottle(cycle);	/* Ch3: Throttle */		
		Rudder		= WalkeraRudder(cycle);		/* Ch4: Rudder   */		
		Gear		= WalkeraGear(cycle);		/* Ch5: Gear     */
		Pitch		= WalkeraPitch(cycle);		/* Ch6: Pitch    */		
		Gyro		= WalkeraGyro(cycle);		/* Ch7: Gyro     */	/* version 3.3.1 */
		Ch8			= WalkeraChannel8(cycle);	/* Ch8: Not used */ /* version 3.3.1 */

		/* Checksum */
		cs = WalkeraCheckSum(cycle); /* version 3.3.1 */

		/* Copy data to joystick positions if checksum is valid (ch1-ch4) */
		if ((cs[0] == cycle[21]) && (cs[1] == cycle[22]))
		{
			m_Position[0] = smooth(m_Position[0], Elevator);
			m_Position[1] = smooth(m_Position[1], Ailerons);
			m_Position[2] = smooth(m_Position[2], Throttle);
			m_Position[3] = smooth(m_Position[3], Rudder);
		};

		/* Copy data to joystick positions if checksum is valid (ch5-ch8) */
		if ((cs[2] == cycle[47]) && (cs[3] == cycle[48]))
		{
			m_Position[4] = smooth(m_Position[4], Gear);
			m_Position[5] = smooth(m_Position[5], Pitch);
			m_Position[6] = smooth(m_Position[6], Gyro);
			m_Position[7] = smooth(m_Position[7], Ch8);
		};
		nPulse = 0;

				SendPPJoy(fixed_n_channel-1, m_Position);
	};
	return;
}

/************** Walkera PCM helper functions **************/
/* Convert pulse width to binary value */
unsigned char  CSppProcess::WalkeraConvert2Bin(int width)
{
	switch (width)
	{
		case 11:
		case 12:
		case 13:
		case 14: return 0;
		break;


		case 19:
		case 20:
		case 21: return 1;
		break;

		default:
			return 0;
	}
};

/* Convert pulse width to octal value */
unsigned char  CSppProcess::WalkeraConvert2Oct(int width)
{
	switch (width)
	{
		case 11:
		case 12:
		case 13:
		case 14: return 0;
		break;

		case 17:
		case 18: return 1;
		break;

		case 19:
		case 20:
		case 21: return 2;
		break;

		case 24:
		case 25: return 3;
		break;

		case 27:
		case 28:
		case 29: return 4;
		break;

		case 30:
		case 31:
		case 32: return 5;
		break;

		case 34:
		case 35:
		case 36: return 6;
		break;

		case 38:
		case 39: return 7;
		break;

		default:
			return 8; /* Illegal value */
	};	

};

/*
	Data: cycle[1:5]
	cycle[1]:			0/1 for stick above/below middle point
	cycle[2]:			Octal - 0 at middle point (MSB)
	cycle[3]:			0/1
	cycle[4]:			Octal
	cycle[5]:			0/1
	MSBit of cycle[6]:	0/1
*/
int CSppProcess::WalkeraElevator(const unsigned char * cycle)
{
	int value;

	value = cycle[2]*64+cycle[3]*32+cycle[4]*4+cycle[5]*2+(cycle[6]>>2);

	/* Mid-point is 511 */
	if(cycle[1])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};


/*
	Data: cycle[6:10]
	cycle[6]:	Bit[1]: 0/1 for stick Left/Right
	cycle[6]:	Bit[0]: (MSB)
	cycle[7]:	Binary - 0 at middle point
	cycle[8]:	Octal
	cycle[9]:	Binary
	cycle[10]:	Octal (LSB)
*/
int CSppProcess::WalkeraAilerons(const unsigned char * cycle)
{
	int value, msb;

	msb = cycle[6]&1;

	/* Offset from mid-point */
	value = msb*256+cycle[7]*128+cycle[8]*16+cycle[9]*8+cycle[10];

	/* Mid-point is 511 */
	if(cycle[6]&2)
		value = 511-value; /* Left */
	else
		value = 511+value; /* Right */

	return value;
};

/*
	Data: cycle[11:15]
	cycle[11]:	0/1 for stick above/below middle point
	cycle[12]:	Octal - 0 at middle point (MSB)
	cycle[13]:	0/1
	cycle[14]:	Octal
	cycle[15]:	Binary (LSB)
*/
int CSppProcess::WalkeraThrottle(const unsigned char * cycle)
{
	int value;

	value = cycle[12]*64+cycle[13]*32+cycle[14]*4+cycle[15]*2+((cycle[16]&4)>>2);

	/* Mid-point is 511 */
	if(cycle[11])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};


/*
	Data: cycle[16:21]
	cycle[16]:	Bit[1]: 0/1 for stick Left/Right
	cycle[16]:	Bit[0]: (MSB)
	cycle[17]:	Binary - 0 at middle point
	cycle[18]:	Octal
	cycle[19]:	Binary
	cycle[20]:	Octal (LSB)
*/
int CSppProcess::WalkeraRudder(const unsigned char * cycle)
{
	int value,  msb;

	msb = cycle[16]&1;

	/* Offset from mid-point */
	value = msb*256+cycle[17]*128+cycle[18]*16+cycle[19]*8+cycle[20];

	/* Mid-point is 511 */
	if(cycle[16]&2)
		value = 511-value; /* Left */
	else
		value = 511+value; /* Right */

	return value;
};


/*
	Data: cycle[23]
*/
int CSppProcess::WalkeraGear(const unsigned char * cycle)
{
	int value;

	value = cycle[24]*64+cycle[25]*32+cycle[26]*4+cycle[27]*2+((cycle[28]&4)>>2);

	/* Mid-point is 511 */
	if(cycle[23])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};



/*
	Data: cycle[29:33]
	cycle[28]:	Bit[1]: 0/1 for stick Left/Right
	cycle[28]:	Bit[0]: (MSB)
	cycle[29]:	Binary - 0 at middle point
	cycle[30]:	Octal
	cycle[31]:	Binary
	cycle[32]:	Octal (LSB)
*/
int CSppProcess::WalkeraPitch(const unsigned char * cycle)
{
	int value,  msb;

	msb = cycle[28]&1;


	/* Offset from mid-point */
	value = msb*256+cycle[29]*128+cycle[30]*16+cycle[31]*8+cycle[32];

	/* Mid-point is 511 */
	if(cycle[28]&2)
		value = 511+value; /* Left */
	else
		value = 511-value; /* Right */

	return value;
};


int CSppProcess::WalkeraGyro(const unsigned char * cycle)
{
	int value;

	value = cycle[34]*64+cycle[35]*32+cycle[36]*4+cycle[37]*2+((cycle[38]&4)>>2);

	/* Mid-point is 511 */
	if(cycle[33])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};

int CSppProcess::WalkeraChannel8(const unsigned char * cycle)
{
	int value,  msb;

	msb = cycle[38]&1;


	/* Offset from mid-point */
	value = msb*256+cycle[39]*128+cycle[40]*16+cycle[41]*8+cycle[42];

	/* Mid-point is 511 */
	if(cycle[38]&2)
		value = 511+value; /* Left */
	else
		value = 511-value; /* Right */

	return value;
};
/*
	Calculates the four Checksum values:
	CS1,CS2: For channels 1-4
	CS3,CS4: For Channels 5-8
*/
int * CSppProcess::WalkeraCheckSum(const unsigned char * cycle)
{
	int static cs[4]= {-1,-1,-1,-1};

	/* CS2 */
	cs[1] = cycle[2]+cycle[4]+cycle[6]+cycle[8]+cycle[10]+cycle[12]+cycle[14]+cycle[16]+cycle[18]+cycle[20];

	/* CS1 */
	cs[0] = cycle[1]+cycle[3]+cycle[5]+cycle[7]+cycle[9]+cycle[11]+cycle[13]+cycle[15]+cycle[17]+cycle[19]+(cs[1]>>3);

	/* CS4 */
	cs[3] = cycle[24]+cycle[26]+cycle[28]+cycle[30]+cycle[32]+cycle[34]+cycle[36]+cycle[38]+cycle[40]+cycle[42]+cycle[44]+cycle[46];

	/* CS3 */
	cs[2] = cycle[23]+cycle[25]+cycle[27]+cycle[29]+cycle[31]+cycle[33]+cycle[35]+cycle[37]+cycle[39]+cycle[41]+cycle[43]+cycle[45]+(cs[3]>>3);

	cs[0]&= 0x1;
	cs[1]&= 0x7;
	cs[2]&= 0x1;
	cs[3]&= 0x7;
	return cs;
}

/* Helper function - Airtronic/Sanwa PCM1 data convertor */
int  __fastcall CSppProcess::Convert15bits(unsigned int in)
{
	int quintet[3];

	/* Convert the upper 5-bits to value 0-3 */
	quintet[2] = air1_msb[((in>>10)&0x1F)];
	if (quintet[2]<0)
		return -1;

	/* Convert the mid 5-bits to value 0-15 */
	quintet[1] = air1_symbol[((in>>5)&0x1F)];
	if (quintet[1]<0)
		return -1;

	/* Convert the low 5-bits to value 0-15 */
	quintet[0] = air1_symbol[(in&0x1F)];
	if (quintet[0]<0)
		return -1;

	/* Return the calculated (inverted) channel value of 0-1023 */
	return 1023-(quintet[2]*256+quintet[1]*16+quintet[0]);

}
/* Helper function - Airtronic/Sanwa PCM2 data convertor */
int  __fastcall  CSppProcess::Convert20bits(int in)
{
	int quartet[5];
	int value;

	quartet[4] = air2_symbol[((in>>16)&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[3] = air2_symbol[((in>>12)&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[2] = air2_symbol[((in>>8 )&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[1] = air2_symbol[((in>>4 )&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[0] = air2_symbol[((in>>0 )&0xC)];
	if (quartet[4]<0)
		return -1;

	value = quartet[4] + (quartet[3]<<2) + (quartet[2]<<4) + (quartet[1]<<6) + (quartet[0]<<8);
	return 1023-2*value;
}

__inline  int  CSppProcess::smooth(int orig, int newval)
{
	if ((orig-newval > 100) || (newval-orig > 100))
		return (newval+orig)/2;

	return newval;

}

/*
	Calculate audio threshold
	____________________________________________________________________
	Based on RCAudio V 3.0 and original Smartpropo
	copyright (C) Philippe G.De Coninck 2007
	
	Copied from: http://www.rcuniverse.com/forum/m_3413991/tm.htm
	____________________________________________________________________
*/
_inline double  CSppProcess::CalcThreshold(int value)
{
	// RCAudio V 3.0 : (C) Philippe G.De Coninck 2007

	static double aud_max_val, aud_min_val;
	static int cAboveThr, cBelowThr;
	double threthold;
	double delta_max = fabs(value - aud_max_val);
	double delta_min = fabs(value - aud_min_val);

	if (delta_max > delta_min) aud_min_val = (4*aud_min_val + value)/5;
	else aud_max_val = (4*aud_max_val + value)/5;

	if (aud_max_val < aud_min_val + 2) {
		aud_max_val = aud_min_val + 1;
		aud_min_val = aud_min_val - 1;
	}

	threthold = (aud_max_val + aud_min_val)/2;

	/* Patch: reset threshold if nothing happens */
	if (value > threthold)
	{
		cAboveThr++;
		cBelowThr=0;
	}
	else
	{
		cAboveThr=0;
		cBelowThr++;
	};
	if (cAboveThr>=10000 || cBelowThr>=10000)
		aud_max_val=aud_min_val=0;

	return(threthold); 
}

// TODO: Rename to Send2vJoy
// TODO: Normalize the calling functions to the range 0-32K
void CSppProcess::SendPPJoy(int nChannels, int * Channel)
{
	BOOL writeOk;
	UINT rID = m_vJoyDeviceId;
	int i, k;
	int ch[MAX_JS_CH];
	int n_ch = 0;

	if (!m_vJoyReady)
		return;

	/* Duplicate channel data */
	memcpy(ch, Channel, MAX_JS_CH*sizeof(int));
	n_ch = nChannels; 

	if (ProcessChannels)
	{
		n_ch = RunJsFilter(ch,nChannels+1);
		if (!n_ch)
			n_ch = nChannels;
	}

	// Create a public duplication of processed data for monitoring
	memcpy(m_PrcChannel, ch, MAX_JS_CH*sizeof(int));	

	
	// Fill-in the structure to be fed to the vJoy device - Axes the Buttons
	UINT iMapped;
	for (i=0; /*i<=n_ch &&*/ i<=HID_USAGE_SL1-HID_USAGE_X;i++)
	{
		iMapped	= Map2Nibble(m_Mapping, i);	// Prepare mapping re-indexing
		writeOk =  SetAxisDelayed(32*ch[iMapped-1], HID_USAGE_X+i); // TODO: the normalization to default values should be done in the calling functions
	}

	for (k=0; k<MAX_BUTTONS;k++)
		writeOk =  SetBtnDelayed(ch[m_BtnMapping[k]-1]>511,k+1); // TODO: Replace 511 with some constant definition

	// Feed structure to vJoy device rID
	BOOL updated = UpdateVJD(rID, &m_vJoyPosition);


	// Calculate quality of joystick data
	static UINT count=0;
	static std::chrono::system_clock::time_point tPrev = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point tNow = std::chrono::system_clock::now();
	std::chrono::duration<double> delta = std::chrono::system_clock::duration::zero();
	if (updated)
	{
		count++;
		if (count>=20)
		{
			delta = tNow-tPrev;
			tPrev = tNow;
			count = 0;
			double c = delta.count();
			m_JoyQual = (UINT)(100/c);
		};
	}
	else
		m_JoyQual=0;

}

void CSppProcess::ButtonMappingChanged(BTNArr* BtnMap, UINT nBtn, UINT vJoyId)
{
	static UINT id = 0;

	if (nBtn>MAX_BUTTONS)
		return;

	if (vJoyId != id)
		SetDefaultBtnMap(m_BtnMapping);
	id = vJoyId;

	// Go over the input array and replace every zero with the corresponding entry in m_BtnMapping
	auto size = BtnMap->size();
	if (m_BtnMapping.size() == size)
	{
		for (UINT i=0; i<size; i++)
		{
			if ((*BtnMap)[i])
				m_BtnMapping[i] = (*BtnMap)[i];
		}; // for

		(*BtnMap) = m_BtnMapping;
	};

}


// Change the mapping scheme
// The number of axes currently supported is 1 to 8
// The number of channels currently supported is 1 to 15
// Each map nibble represents a channel to axis mapping. The nibble value is the channel index (1-based). The nibble location is the axis
// Nibbles are set from the upper nibble (X axis) to lower nibble (SL1 axis)
// Special cases:
// - nAxes = 0: Reset mapping to defualt
// - nAxes < 8: map only the nAxes starting from X axis, set the rest to default
// - Map nibble = 0, don't change mapping for this axis.
// - vJoy device changed: Default mapping is reset
// Return: New mapping

void CSppProcess::AxisMappingChanged(DWORD* Map, UINT nAxes,  UINT vJoyId)
{

	DWORD dwMap=0; // Intermediary map
	UCHAR Nibble, PrevNibble, DefNibble;
	static UINT id = 0;

	if (nAxes>8)
		return;

	if (vJoyId != id)
		m_Mapping = 0x12345678;
	id = vJoyId;

	// Axes
	// For every nibble: Normalize index (to 0-based), set mapping if was 0 or set to default if out of range
	for (UINT i=0; i<8; i++)
	{
		Nibble		= Map2Nibble(*Map,i);			// Nibble from map
		PrevNibble	= Map2Nibble(m_Mapping, i);		// Nibble previously used
		DefNibble	= Map2Nibble(0x12345678, i);	// Default nibble

		if (i<nAxes)
		{
			if (Nibble)
				dwMap = (dwMap<<4) | (Nibble);
			else 
				dwMap = (dwMap<<4) | PrevNibble ;
		}
		else
			dwMap = (dwMap<<4) | DefNibble ;

	}

	m_Mapping = dwMap;
	*Map = m_Mapping;
	return;
}

void CSppProcess::MappingChanged(Mapping* m, UINT vJoyId)
{
	ButtonMappingChanged(m->ButtonArray, m->nButtons, vJoyId);
	AxisMappingChanged(m->pAxisMap, m->nAxes, vJoyId);
}

void CSppProcess::SetDefaultBtnMap(array <BYTE, 128>& BtnMap)
{
	auto size = BtnMap.size();
	for (UINT i=0; i<size; i++)
	{
		if (i<24)
			BtnMap[i] = i+9;
		else
			BtnMap[i] = 9;
	}

}

/* Run Joystick post processor filter */
int CSppProcess::RunJsFilter(int * ch, int nChannels)
{
	JS_CHANNELS  * js_filter_out, js_data;
	int n_out_ch=0;

	// Sanity check
	if (!ProcessChannels || m_JsChPostProc_selected ==-1)
		return 0;

		js_data.ch = nChannels;
		js_data.value = ch;
		js_filter_out = ProcessChannels(&js_data, 1023, 0);
		if (js_filter_out && js_filter_out->ch>0 && js_filter_out->ch<=MAX_JS_CH)
		{
			for (int i=0;i<js_filter_out->ch;i++)
			{
				ch[i] = js_filter_out->value[i];
				js_filter_out->value[i] = 0;
			};
			n_out_ch = js_filter_out->ch;
			js_filter_out->ch=0;
		}

	return n_out_ch;
}

DWORD WINAPI CSppProcess::MonitorCaptureStatic(LPVOID obj)
{
	THREAD_NAME(" CSppProcess::MonitorCapture");
	if (obj)
		((CSppProcess *)obj)->MonitorCapture();
	return 0;
}

DWORD WINAPI CSppProcess::CaptureAudioStatic(LPVOID obj)
{
	THREAD_NAME(" CSppProcess::CaptureAudio");

	if (obj)
		((CSppProcess *)obj)->CaptureAudio();
	return 0;
}

DWORD WINAPI CSppProcess::PollChannelsStatic(LPVOID obj)
{
	if (obj)
		((CSppProcess *)obj)->PollChannels();
	return 0;
}

void CSppProcess::SendModInfoToParent(HWND hParentWnd)
{
	if (!hParentWnd || !m_ModulationMap.size())
		return;

	for (MODMAP::iterator  i=m_ModulationMap.begin(); i != m_ModulationMap.end(); i++)
		SendMessage(hParentWnd, WMSPP_PRCS_SETMOD, (WPARAM)(&(*i).second) , (LPARAM)m_SelectedMod);
}

BOOL CSppProcess::SetAxisDelayed(LONG Value,  UINT Axis)
{
		/* Write Value to a given axis defined in the specified VDJ
		Limited to the following axes:
		HID_USAGE_X		0x30
		HID_USAGE_Y		0x31
		HID_USAGE_Z		0x32
		HID_USAGE_RX	0x33
		HID_USAGE_RY	0x34
		HID_USAGE_RZ	0x35
		HID_USAGE_SL0	0x36
		HID_USAGE_SL1	0x37
		HID_USAGE_WHL	0x38

	*/

	if (Axis<HID_USAGE_X || Axis>HID_USAGE_WHL)
		return FALSE;

	switch (Axis)
	{
	case HID_USAGE_X:
		m_vJoyPosition.wAxisX = Value;
		break;
	case HID_USAGE_Y:
		m_vJoyPosition.wAxisY = Value;
		break;
	case HID_USAGE_Z:
		m_vJoyPosition.wAxisZ = Value;
		break;
	case HID_USAGE_RX:
		m_vJoyPosition.wAxisXRot = Value;
		break;
	case HID_USAGE_RY:
		m_vJoyPosition.wAxisYRot = Value;
		break;
	case HID_USAGE_RZ:
		m_vJoyPosition.wAxisZRot = Value;
		break;
	case HID_USAGE_SL0:
		m_vJoyPosition.wSlider = Value;
		break;
	case HID_USAGE_SL1:
		m_vJoyPosition.wDial = Value;
		break;
	case HID_USAGE_WHL:
		m_vJoyPosition.wWheel = Value;
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL CSppProcess::SetBtnDelayed(BOOL Value, UCHAR nBtn)
{
	LONG Mask=0x00000001;

	// Write Value to a given button defined in the specified VDJ
	if (nBtn<1 || nBtn>32)
		return FALSE;

	// If Value=TRUE the the given button is set to 1
	if (Value)
	{
		Mask = Mask<<(nBtn-1);
		m_vJoyPosition.lButtons |= Mask;
	}
	else
	{
		Mask = Mask<<(nBtn-1);
		Mask = ~Mask;
		m_vJoyPosition.lButtons &= Mask;
	};

	return TRUE;
}


void	CSppProcess::LogMessage(int Severity, int Code, LPCTSTR Msg)
{
	if (!m_hParentWnd)
		return;

	PostMessage(m_hParentWnd , WMSPP_LOG_PRSC + Severity, (WPARAM)Code, (LPARAM)Msg);
}

// Called with every input sample - sends data only on pulse end
// Accumulates sample data until pulse end - then sends the data to parent window
// Sample date accumulated in two interleaving arrays
void CSppProcess::SendDbgPulse(USHORT sample, bool negative, UINT rawPulseLength, UINT PulseLength)
{
	// Preparing arrays to accumulate samples
	const UINT ArraySize = 4000;
	static USHORT  Samples[2][ArraySize];
	static UCHAR iArray = 0;
	static UINT iSample=0;
	static DbgPulseInfo info;

	// Send message if new pulse or if the array is going to overrun
	if ( (iSample >= ArraySize) || rawPulseLength)
	{
		// Prepare data
		info.size = sizeof(DbgPulseInfo);
		info.Samples = (LPVOID)(&(Samples[iArray]));
		info.RawPulse = rawPulseLength;
		info.NormPulse = PulseLength;
		info.negative= negative;
		// Post data 
		SendMessage(m_hParentWnd, WMSPP_PROC_PULSE, (WPARAM)&info, (LPARAM)iSample);

		// Prepare to continue
		if (iArray)
			iArray = 0;
		else
			iArray = 1;
		iSample = 0;
	}

	// Put a new sample in current array (Make sure array not overrun)
	Samples[iArray][iSample++] = sample;
}

#ifdef _DEBUG
const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // Must be 0x1000.
   LPCSTR szName; // Pointer to name (in user addr space).
   DWORD dwThreadID; // Thread ID (-1=caller thread).
   DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(char* threadName)
{
   THREADNAME_INFO info;
   info.dwType = 0x1000;
   info.szName = threadName;
   info.dwThreadID = GetCurrentThreadId();
   info.dwFlags = 0;

   __try
   {
      RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
   }
   __except(EXCEPTION_EXECUTE_HANDLER)
   {
   }
}
#endif
