// Wave2Joystick.h : main header file for the Wave2Joystick DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "SmartPropoPlus.h"
#include "GenWave.h"

class CFilterIf;
class CFmsIf;

/* Structure holding data relevant for a WaveIn device */
struct WaveInStruct {
	int				id;								// WaveIn ID (Default = -1)
	char *			DeviceName;						// Name of the MIXER device than owns this WaveIn device
	HWAVEIN			waveIn;							// Handle to Wave In device
	WAVEFORMATEX	waveFmt;						// WAVE FORMAT (IN)
	WAVEHDR			*waveBuf[N_WAVEIN_BUF];			// Array of poiters to audio buffers /* Version 3.3.5 - Vista compatability */
} /*WaveInStruct, *pWaveInStruct*/;

/* Structure holding audio buffer related data */
struct BufferStruct {
	int				Index;							// Buffer index
	WaveInStruct	*pWaveInStruct;					// Wavin Device
};

/* 
	Structure holding a list of structures type _WaveInStruct and the ID of the current device
	The first item (index 0) in the list is the WAVE_MAPPER (-1) WaveIn device
	The second item (index 1) in the list is WaveIn device ID=0 and so on
	If there is only one WaveIn device then there are two items in the array but they are identicle
*/
struct WaveInListStruct {
	CWaveIn			**pDevice;						// Array WaveInStruct's
	int				id;								// ID of the currently selected device (Not index!)
	int				index;							// Index of the currently selected device (in the array )
} /*WaveInListStruct, *pWaveInListStruct*/;


// Global
DWORD WINAPI waveInThreadProc(LPVOID arg);

// CWave2JoystickApp
// See Wave2Joystick.cpp for the implementation of this class
//

class CWave2JoystickApp : public CWinApp
{
public:
	CWave2JoystickApp();
	virtual ~CWave2JoystickApp(void);
	BOOL StopInterfaces(void);

// Overrides
public:
	virtual BOOL InitInstance();
	virtual BOOL Start();

private:
	TCHAR *				m_CurrMixerDeviceName;
	WaveInListStruct *	m_WaveInList;
	MODE				m_ModulationType;	// PPM/JR(PCM)/Futaba(PCM)...
	int					m_ModulationShift;	// Auto/Negative/Positive ...

	WaveInListStruct * InitAllDevices(void);

	CPpjoyIf *			m_PpjoyObj;
	UINT	NEAR		WM_INTERSPPAPPS;
	CFilterIf *			m_Filters;
	CFmsIf *			m_fms;

	int					m_nPostFilterPos;	// Number of post-filter joystick positions
	int				*	m_PostFilterPos;	// Pointer to array of Post-filter joystick positions

	//CWaveIn * InitWaveIn(int DeviceId);

	bool StartSelectedDevice(void);
	bool StopCurrentDevice(void);
	bool SetShift(const int Shift);// Set the modulation shift type: Auto, Negative or Positive
	bool SetAntiJitter(const bool Enable);// Enable/Disable anti-jitter functionality status (On/Off)
	bool SetActiveAudioDevice(const TCHAR * AudioDevice);// Set the active audio device to which the Tx is connected
	bool SetSelectedInput(const TCHAR * Input);// Set the selected audio input (in the selected audio device) to which the Tx is connected
	bool SetMixerSelector(const bool Enable);
	bool StartWaveLog(TCHAR * LogFileName);
	bool PlayWaveLog(TCHAR * LogFileName);
	bool StopWaveLog(void);
	bool StatWaveLog(int * stat);
	bool StartAudioHeaderLog(TCHAR * LogFileName);
	bool StopAudioHeaderLog(void);
	bool StatAudioHeaderLog(int * stat);
	bool StartPulseLog(TCHAR * LogFileName);
	bool StopPulseLog(void);
	bool StatPulseLog(int * stat);
	bool StartRawPulseLog(TCHAR * LogFileName);
	bool StopRawPulseLog(void);
	bool StatRawPulseLog(int * stat);
	bool GetAudioLevel(int * level);
	bool GetNumJoyPos(int * nPos);
	bool GetJoyPos(int * pos);
	bool GetFilteredPos(int * pos);

	static int	MixerName2WaveInId(const TCHAR * MixerName);
	bool StartDevice(int DeviceId);


	DECLARE_MESSAGE_MAP()
public:
	// Generic function to set working parameters such as modulation type and audio device
	virtual bool SetParam(const int param, const void * pValue);
	// Wrapper function to SendPosition2Ppjoy()
	static bool SendPositionCallback(void * ThisObj, int * Position, int * nPositions);

protected:
	// Set the modulation type
	bool SetModulation(const TCHAR * Type);
	// Send joysick position to Ppjoy device driver
	bool SendPosition2Ppjoy();
	// Send joysick position to data block from which FMS (winmm.dll) will read the data
	bool SendPosition2Fms();
	// Store the post-filter joystick positions and number of positions in the appropriate members
	void  SetPostFilterPosition(const int * Position, const int nPositions);
	// Apply filter, if needed to the raw joystick data
	bool  ApplyFilter(int * Position, int nPositions);


public:
	bool StartPPJoyInterface(int iDevice);
	bool StopPPJoyInterface(int iDevice);
	bool TestPPJoyInterface(int iDevice);

public:
	bool SetFilterInterface(void * object);


};
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */
#define W2JAPI        DECLSPEC_IMPORT

//////////// Interface functions ////////////////////
LONG PASCAL EXPORT TestTest();
CWave2JoystickApp * PASCAL GetWave2JoystickObject();
BOOL PASCAL SppStart(void);
BOOL PASCAL StopInterfaces(void);

// Set parameters
BOOL PASCAL SppSetParam(const int param, const void * pValue);
BOOL PASCAL SppSetModulation(const TCHAR * Modulation);// Set the modulation type: PPM, JR(PCM) etc
BOOL PASCAL SppSetShift(const int Shift);// Set the modulation shift type: Auto, Negative or Positive
BOOL PASCAL SppSetAntiJitter(const BOOL Enable);// Enable/Disable anti-jitter functionality status (On/Off)
BOOL PASCAL SppSetActiveAudioDevice(const TCHAR * AudioDevice);// Set the active audio device to which the Tx is connected
BOOL PASCAL SppSetSelectedInput(const TCHAR * Input);// Set the selected audio input (in the selected audio device) to which the Tx is connected
BOOL PASCAL SppSetMixerSelector(const BOOL Enable);

// PPJoy interface
BOOL PASCAL StartPPJoyInterface(int index);
BOOL PASCAL StopPPJoyInterface(int index);
BOOL PASCAL TestPPJoyInterface(int index);

// Filter (Post processor) interface
BOOL PASCAL SetFilterInterface(void * object); // Pointer to the filter object

// Wave Recorder Interface
BOOL PASCAL SppWaveLoggerStart(TCHAR * FileName);
BOOL PASCAL SppWaveLoggerStop(void);
BOOL PASCAL SppWaveLoggerPlay(TCHAR * FileName);
BOOL PASCAL SppWaveLoggerGetStat(int * stat);

// Audio Header logger Interface
BOOL PASCAL SppAudioHdrLoggerStart(TCHAR * FileName);
BOOL PASCAL SppAudioHdrLoggerStop(void);
BOOL PASCAL SppAudioHdrLoggerGetStat(int * stat);

// Pulse logger Interface
BOOL PASCAL SppPulseLoggerStart(TCHAR * FileName);
BOOL PASCAL SppPulseLoggerStop(void);
BOOL PASCAL SppPulseLoggerGetStat(int * stat);

// Raw Pulse logger Interface
BOOL PASCAL SppRawPulseLoggerStart(TCHAR * FileName);
BOOL PASCAL SppRawPulseLoggerStop(void);
BOOL PASCAL SppRawPulseLoggerGetStat(int * stat);

// Monitoring
BOOL PASCAL SppGetAudioLevel(int * AudioLevel);
BOOL PASCAL SppGetNumJoyPos(int * nPos);
BOOL PASCAL SppGetJoyPos(int * Pos);
BOOL PASCAL SppGetFilteredPos(int * Pos);

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

