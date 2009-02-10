/* Generic SmartPropoPlus header file */
#include "windows.h"
#include "Ver.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#ifndef __SMARTPROPOPLUS
#define __SMARTPROPOPLUS
/** Definitions **/
#define REG_FMS		"Software\\Flying-Model-Simulator"
#define REG_BASE	"Software\\Audio2Joystick"
#define REG_SPP		REG_BASE"\\SmartPropoPlus"
#define REG_MOD		REG_BASE"\\Modulation-Types"
#define REG_AUD		REG_BASE"\\Audio-Sources"
#define REG_UNINST	"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\SmartPropoPlus"

#define UNINST_SPP		"UninstallString"
#define INSTPATH_FMS	"InstallationPath"

#define MOD_TYPE_PPM	"PPM"
#define MOD_TYPE_JR 	"JR"
#define MOD_TYPE_FUT	"FUT"
#define MOD_TYPE_AIR1	"AIR1"
#define MOD_TYPE_AIR2	"AIR2"
#define MOD_TYPE_WAL	"WAL"

#define MOD_NAME_PPM	"PPM (Generic)"
#define MOD_NAME_JR 	"JR (PCM)"
#define MOD_NAME_FUT	"Futaba (PCM)"
#define MOD_NAME_AIR1	"Sanwa/Air (PCM1)"
#define MOD_NAME_AIR2	"Sanwa/Air (PCM2)"
#define MOD_NAME_WAL	"Walkera (PCM)"

#define MOD_DEF_STR  {\
						MOD_TYPE_PPM, MOD_NAME_PPM,\
						MOD_TYPE_JR,  MOD_NAME_JR,\
						MOD_TYPE_FUT, MOD_NAME_FUT,\
						MOD_TYPE_AIR1,MOD_NAME_AIR1,\
						MOD_TYPE_AIR2,MOD_NAME_AIR2,\
						MOD_TYPE_WAL,MOD_NAME_WAL,\
						NULL}

#define	MNM_POS		"&Positive"
#define	MNM_NEG		"&Negative"
#define	MNM_POS_HID	"Positive"
#define	MNM_NEG_HID	"Negative"

#define MOD_ACTIVE	"Active"
#define SHIFT_POS	"Positive-Shift"
#define SHIFT_AUTO	"Shift-Auto-Detect"
#define DEBUG_LEVEL	"Debug-Level"
#define	MIXER_DEV	"Mixer-Device"
//#define	MIXER_DEV_I	"Mixer-Device-Index"
#define	AUDIO		"Audio"

#define	SEL_FLTR	"Selected-Filter"

#define LOCATIONX	"SppConsole location - X"
#define LOCATIONY	"SppConsole location - Y"

#define	PPJ_EXT		"PPJoy_Extension"

#define	ANTI_JITTER	"Anti_Jitter"

#define GUI_LOCAT	"Location_of_SppConsole"

#define	DEF_VOL_MIC	"Default Volume Level - Microphone"
#define	DEF_VOL_AUX	"Default Volume Level - Aux"
#define	DEF_VOL_LIN	"Default Volume Level - Line"
#define	DEF_VOL_ANL	"Default Volume Level - Analog"
#define	DEF_VOL_UNK	"Default Volume Level - Unknown"

// WaveIn override values
#define	WAVEIN_NSAMPLES	"WaveIn nSamples"
#define	WAVEIN_NBITS	"WaveIn nBits"
#define	WAVEIN_NBUFFERS	"WaveIn nBuffs"
#define	WAVEIN_BUFSIZE	"WaveIn BufSize"

#define MUTEX_LABEL	"SPP Global Shared memory Mutex"
#define BLOCK_LABEL	"SPP Global Shared memory Block"
#define SHARED_FILE_NAME	"SPP memory Block - as a shared file"
#define SHARED_FILE_MUTEX	"SPP memory Block - as a shared file - Mutex preventing SPP-FMS mutual access"


#define TX_VOLUME_MIC		5
#define TX_VOLUME_LINE		5
#define TX_VOLUME_AUX		7
#define TX_VOLUME_ANALOG	10
#define TX_VOLUME_UNKNOWN	10


#define MAX_VAL_NAME 254
#define MAX_MODS    127
#define MAX_FLTRS   127
#define MAX_MIXER   511
#define MAX_JS_CH	12
#define MAX_VERSTR  50

#define PPJDLL_NAME		TEXT("PPJoyEx")
#define SPP_PROD_NAME	TEXT("SmartPropoPlus %d.%d.%d")
#define CTRL_LOG_FILE	"SPP_ctrl.log"
#define CHNL_LOG_FILE	"SPP_chnl.log"
#define DATA_LOG_FILE	"SPP_data.log"
#define SEPARATOR		"*************************************************************************"

#define	SPP_ERROR_MSG	"SmartPropoPlus Error"

#define CONSOLE_TT			"SmartPropoPlus Console"
#define CONSOLE_TT_OFF		"SmartPropoPlus is OFF"
#define CONSOLE_TT_ON		"SmartPropoPlus is ON"
#define CONSOLE_BALOON_TTL	"SmartPropoPlus Console"
#define CONSOLE_BALOON_TXT	"Use this application to control:\n- Modulation Type\n- Modulation Shift\n\nChanges are effective Immediately!"
#define BALOON_DLL_STOPPED	"SmartPropoPlus stopped"
#define BALOON_DLL_STARTING	"SmartPropoPlus is starting"
#define BALOON_DLL_MODTYPE	"\nModulation type:\t%s"
#define BALOON_DLL_AUTOON	"\nPolarity Auto-Detect:\tOn"
#define BALOON_DLL_AUTOOFF	"\nPolarity Auto-Detect:\tOff"
#define BALOON_DLL_SHIFTPOS	"\nShift Polarity:\t\tPositive"
#define BALOON_DLL_SHIFTNEG	"\nShift Polarity:\t\tNegative"
#define BALOON_DLL_AUDIO	"\n\nAudio Source:\t%s"
#define BALOON_DLL_PPJOY	"\n\nPPJoy Status:\t%s"
#define BALOON_PPJ_CNCT		"PPJoy Virtual Joystick %d Connected"
#define BALOON_PPJ_DISCNCT	"PPJoy Virtual Joystick %d Disconnected"
#define BALOON_PPJ_NOTCNCT	"PPJoy Virtual Joystick %d not connected"
#define BALOON_PPJ_NOTFND	"PPJoy Virtual Joystick %d cannot be found"
#define BALOON_PPJ_INVHNDL	"PPJoy device handle is invalid"
	


#define SPP_HOME_URL		"http://www.SmartPropoPlus.com"
#define SPP_HOME_TITLE		"SmartPropoPlus Home"
#define	SPP_MAIN_TEXT		"\
Version %d.%d.%d - Build %s\n\
Copyright (c) 2005-2007 by Shaul Eizikovich\n\n\
This software is based on SmartPropo by Tsutomu SEKI\n\
You may use and destribute this program freely.\n\n\
SmartPropoPlus enables you to connect your R/C transmitter to your computer through \
one of the sound-card inputs (e.g. Microphone, Line-in) so it can be used as a joystick.\n\
For further details please go to the SmartPropoPlus Home using the link below."

#define CONSOLE_TITLE		"SmartPropoPlus Console"
#define	MUTXCONSOLE			"SPPconsole Exists Mutex 1"
#define	MUTXWINMM			"WINMM Dll is running"
#define	INTERSPPCONSOLE		"Inter SPPconsole Message 1"
#define	INTERSPPAPPS		"Inter SPP Application Message 2"

/* User Defined Window-Messages */
#define WM_DBLLEFT	WM_APP+1
#define WM_LEFT		WM_APP+2
#define WM_RIGHT	WM_APP+3
#define WM_GENERAL	WM_APP+4
#define WM_SHOWGUI	WM_APP+5
#define WM_HIDEGUI	WM_APP+6

/* User Defined Window-Message parameters */
#define MSG_DLLSTOPPING		1280	// WPARAM: Inform GUI that DLL is stopping
#define MSG_DLLSTARTING		1281	// WPARAM: Inform GUI that DLL is starting
#define MSG_DLLPPJSTAT		1282	// WPARAM: Inform GUI of PPJoy connection status
#define MSG_JSCHPPEVAIL		1283	// WPARAM: Inform GUI that Joystick channel postprocessor is evalable/unevailable
#define MSG_DLLTIMESTAMP	1284	// WPARAM: Inform GUI that DLL is getting the joystick data
#define MSG_TIMESTAMP2DLL	1285	// WPARAM: Inform GUI that joystick data was sent to DLL


#define MSG_DPPJSTAT_DISCN	1984	// LPARAM: Intentional disconnection
#define MSG_DPPJSTAT_NOTCN	1985	// LPARAM: Not connected

#define MSG_WAVE_REC_START	2001	// Wave recording started
#define MSG_WAVE_REC_STOP	2002	// Wave recording stopped
#define MSG_WAVE_REC_PLAY	2003	// Wave playing started
#define MSG_WAVE_REC_CANCEL	2004	// Wave playing/recording canceled

#define MSG_AUDIO_HDRS_START	2011	// Audio header logging started
#define MSG_AUDIO_HDRS_STOP		2012	// Audio header logging stopped
#define MSG_AUDIO_HDRS_CANCEL	2014	// Audio header logging canceled

#define MSG_PULSE_START			2015	// Pulse logging started
#define MSG_PULSE_STOP			2016	// Pulse logging stopped
#define MSG_PULSE_CANCEL		2018	// Pulse logging canceled

#define MSG_FMS_CONN_START		2019	// Fms connection logging started
#define MSG_FMS_CONN_STOP		2020	// Fms connection logging stopped
#define MSG_FMS_CONN_CANCEL		2022	// Fms connection logging canceled


#define MSG_RAW_PULSE_START		2023	// Raw pulses logging started
#define MSG_RAW_PULSE_STOP		2024	// Raw pulses  logging stopped
#define MSG_RAW_PULSE_CANCEL	2026	// Raw pulses  logging canceled

enum LOGSTAT {
		Idle
	,	Started
	,	Printing
	,	Stopping
	,	Finish
};

/** Data types and structures ***/
struct Modulation
{
	char * ModTypeInternal;
	char * ModTypeDisplay;
	int index;
};

struct Modulations
{
	struct Modulation ** ModulationList;
	int Active;
	int PositiveShift;
	int	ShiftAutoDetect;
};

struct JsChPostProcInfo
{
	int n_fltr;				// Number of supported filters (1 or more)
	int	i_sel;				// Index of selected filter (0 based, -1 means nothing)
	far char ** name_fltr;	// list of display name of filters
} ;

struct Spp2FmsStruct
{
	int nChannels;
	int Channel[MAX_JS_CH];
	char VersionStr[MAX_VERSTR];
	int Identifier;
};

/* Function Prototypes */
void SetLocationDialogWindow(int x, int y);
BOOL GetLocationDialogWindow(int * x, int * y);
int SetActiveMode(const char * selected);
void SetShiftAutoDetect(const int sel);
void SetPositiveShift(const int sel);
struct Modulations * GetModulation(int Create);
int GetDebugLevel(void);
const char * ModeDisplayFromInternalName(const char * internal);
int GetDefaultVolumeValue(unsigned long SrcType);
int SetDefaultVolumeValue(unsigned long SrcType, unsigned long  VolumeValue);
int GetCurrentAudioState();
void SetCurrentAudioState(int Active);
int GetCurrentPpjoyState();
void SetCurrentPpjoyState(int Active);
char * GetCurrentMixerDevice(void);
int GetCurrentInputLine(unsigned int *SrcID);
int GetWaveInParams(int *nSamples, int *nBits, int *nBuffers, int *BufferSize);
void SetCurrentMixerDevice(const char * MixerName);
void SetCurrentInputLine(const char * MixerName, unsigned int SrcID);
far void * CreateDataBlock(struct Modulations * data);


void SetNumberOfFilters(const int n);
//int GetNumberOfFilters();
void SetSelectedFilterIndex(const int i);
void SetSelectedFilter(const char * FilterName);
int GetSelectedFilterIndex();
//char * GetFilterNameByIndex(const int i);
//void SetFilterNameByIndex(const int i, const char * name);
//char * GetFilterNameByIndex(const int i);
void SetFilterNames(const char ** name);

void SppMessageBoxWithErrorCode(void);

int GetAntiJitterState();
void SetAntiJitterState(int enable);

int GetSppConsoleFullPath(char * Path, UINT PathSize);
int GetFmsFullPath(char * Path, UINT PathSize);

#endif // __SMARTPROPOPLUS

#if defined(__cplusplus)
}
#endif