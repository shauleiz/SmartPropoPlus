#pragma once

#include <math.h>
#include "SmartPropoPlus.h"
#include "GenWave.h"
#include "jr.h"
#include "futaba.h"
#include "airtronics.h"
#include "Logger.h"

#define	POLLING_PERIOD	20	// in Milliseconds
#define FLG(i) m_waveBuf[i]->dwFlags


DWORD WINAPI g_WaveInThreadProc(LPVOID arg);
DWORD WINAPI g_ReadWaveFileThreadProc(LPVOID arg);
VOID CALLBACK TimerCompletionRoutine(LPVOID lpArg,   DWORD dwTimerLowValue,   DWORD dwTimerHighValue );

class CWaveIn
{
public:
	CWaveIn(void);
	virtual ~CWaveIn(void);

	/* Interface with CWave2Joystick*/
	bool Init(int id);
	bool			Start();
	bool			Stop();
	int				GetId();
	bool			SetModulation(const MODE Type); 
	void			SetJoystickEventCallback(void FAR * callbackfunc, void FAR * Obj);
	bool SendJoystickData(bool force=false);// Send Joystick data (if channged). Returnes true if sent or false if not sent
	LONG			GetWaveDataFromLogger(void * buffer, int * size);
	int				GetAudioLevel(void);
	int				GetNumJoyPos(void);
	int				GetJoyPos(int ** pos);
	bool			isJoyPosValid(void);


	/* Interface with the thread that called by the driver  */
	void			FAR ProcessBuffer(WAVEHDR * hdr, HWAVEIN hDevice);
	void			FAR ResetBuffer(WAVEHDR * hdr);
	void			__fastcall ProcessData(int i);



	WAVEHDR			*m_waveBuf[N_WAVEIN_BUF];			// Array of poiters to audio buffers /* Version 3.3.5 - Vista compatability */
private:
	int				m_id;								// WaveIn ID (Default = -1)
	TCHAR *			m_DeviceName;						// Name of the MIXER device than owns this WaveIn device
	HWAVEIN			m_hWaveIn;							// Handle to Wave In device
	WAVEFORMATEX	m_waveFmt;							// WAVE FORMAT (IN)
	WAVEFORMATEX	m_waveFmtRd;						// WAVE FORMAT (Reading WAV file)
	DWORD			m_waveInThreadId;					// Id of the processing thread
	int				m_Position[MAX_JS_CH];				// Array of joystick position values
	int				m_nPositions;						// Number of active channels
	bool			m_ValidPositions;					// True only if the data in m_Position is valid
	int				m_waveBufSize ;
	class Wave2Joystick *	m_JoystickTargetObj;		// Target Object to from which the joystick callback is executed
	void *			m_JoystickTargetFunc;				// Pointer to joystick callback
	DWORD			m_ThreadId;							// ID of the current thread
	CLogger *		m_LogWr;							// Object that handles logging (Write Operation)
	CLogger *		m_LogRd;							// Object that handles logging (Read Operation)
	int				m_AudioLevel;						// Audio level (volume) of the raw audio signal
	FILE *			m_pAudioHeaderLogFile;				// Pointer to audio header log file
	LOGSTAT			m_AudioHeaderLogStat;				// Status of the audio header logger
	FILE *			m_pPulseLogFile;					// Pointer to pulse log file
	LOGSTAT			m_PulseLogStat;						// Status of the pulse logger
	FILE *			m_pRawPulseLogFile;					// Pointer to raw pulse log file
	LOGSTAT			m_RawPulseLogStat;					// Status of the raw pulse logger


private:
	// All ProcessPulseXXX functions + one placeholder function
	int		(CWaveIn::*m_ProcessPulse)(int, BOOL);
	int		ProcessPulsePpm(int width, BOOL input);
	int		ProcessPulseJrPpm(int width, BOOL input);
	int		ProcessPulseFutabaPpm(int width, BOOL input);
	int		ProcessPulseWalPcm(int width, BOOL input);
	int		ProcessPulseAirPcm1(int width, BOOL input);
	int		ProcessPulseAirPcm2(int width, BOOL input);
	int		ProcessPulseJrPcm(int width, BOOL input);
	int		ProcessPulseFutabaPcm(int width, BOOL input);

	// Walkera PCM helper functions
	unsigned char	WalkeraConvert2Bin(int width);
	unsigned char	WalkeraConvert2Oct(int width);
	int				WalkeraElevator(const unsigned char * cycle);
	int				WalkeraAilerons(const unsigned char * cycle);
	int				WalkeraThrottle(const unsigned char * cycle);
	int				WalkeraRudder(const unsigned char * cycle);
	int				WalkeraGear(const unsigned char * cycle);
	int				WalkeraPitch(const unsigned char * cycle);
	int				WalkeraGyro(const unsigned char * cycle);
	int				WalkeraChannel8(const unsigned char * cycle);
	int	*			WalkeraCheckSum(const unsigned char * cycle);

	// PP Helper functions
	int  __fastcall Convert15bits(unsigned int in);
	int  __fastcall Convert20bits(int in);

	// Debug functions
	void DebugPrintChannels(bool changed);
	void inline DebugPrintPulses(int Length, double Threshold);

	// Volume
	int CalcAudioLevel(int Value, int Size);

	// Jitter
	bool m_AntiJitterEnabled;

public:
	// Create a WAV file for logging purpose and return its name
	TCHAR * StartWaveFileLog(void);
	bool StopWaveFileLog(void);
	bool PlayWaveFileLog(TCHAR * LogFileName);
	bool StatWaveFileLog(int * stat);
	bool m_PlayStopped, m_PlayEnded;

	// Craete a log file that keeps data from the WaveIn headers
	TCHAR * StartAudioHeaderLog(void);
	bool StopAudioHeaderLog(void);
	bool StatAudioHeaderLog(int * stat);

	// Craete a log file that saves the pulse width info
	TCHAR * StartPulseLog(void);
	bool StopPulseLog(void);
	bool StatPulseLog(int * stat);

	// Craete a log file that saves info from within functions ProcessPulseXXX
	TCHAR * StartRawPulseLog(void);
	bool StopRawPulseLog(void);
	bool StatRawPulseLog(int * stat);

	// Jitter
	bool SetAntiJitter(const bool Enable);
	// Print WaveIn header data
	inline void DebugPrintWaveHdr(WAVEHDR * hdr, LONG MsgTime=0);
	inline void DebugPrintRawPulses(const char * FunctionName, const int width, const unsigned char * data, const int nData, int size=1);

};
