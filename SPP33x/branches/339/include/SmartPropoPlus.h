/* Generic SmartPropoPlus header file */
#include "windows.h"
#if defined(__cplusplus)
extern "C"
{
#endif

#ifndef __SMARTPROPOPLUS
#define __SMARTPROPOPLUS
/** Definitions **/
#define REG_FMS "Software\\Flying-Model-Simulator"
#define REG_SPP "Software\\Flying-Model-Simulator\\SmartPropoPlus"
#define REG_MOD "Software\\Flying-Model-Simulator\\SmartPropoPlus\\Modulation-Types"
#define REG_AUD "Software\\Flying-Model-Simulator\\SmartPropoPlus\\Audio-Sources"

#define MOD_TYPE_PPM	"PPM"
#define MOD_TYPE_PPMW	"PPMW"
#define MOD_TYPE_JR 	"JR"
#define MOD_TYPE_FUT	"FUT"
#define MOD_TYPE_AIR1	"AIR1"
#define MOD_TYPE_AIR2	"AIR2"
#define MOD_TYPE_WAL	"WAL"

#define MOD_NAME_PPM	"PPM (Generic)"
#define MOD_NAME_PPMW	"PPM (Walkera)"
#define MOD_NAME_JR 	"JR (PCM)"
#define MOD_NAME_FUT	"Futaba (PCM)"
#define MOD_NAME_AIR1	"Sanwa/Air (PCM1)"
#define MOD_NAME_AIR2	"Sanwa/Air (PCM2)"
#define MOD_NAME_WAL	"Walkera (PCM)"

#define MOD_DEF_STR  {\
						MOD_TYPE_PPM, MOD_NAME_PPM,\
						MOD_TYPE_PPMW, MOD_NAME_PPMW,\
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
#define	AUDIO		"Audio"

#define	SEL_FLTR	"Selected-Filter"

#define	PPJ_EXT		"PPJoy_Extension"

#define	DEF_VOL_MIC	"Default Volume Level - Microphone"
#define	DEF_VOL_AUX	"Default Volume Level - Aux"
#define	DEF_VOL_LIN	"Default Volume Level - Line"
#define	DEF_VOL_ANL	"Default Volume Level - Analog"
#define	DEF_VOL_UNK	"Default Volume Level - Unknown"

#define MUTEX_LABEL	"SPP Global Shared memory Mutex"
#define BLOCK_LABEL	"SPP Global Shared memory Block"
#define EVENT_MIXER	"Mixer Switching Event"
#define	VER_DLL		0x00030307 /* DLL File Version */
#define	VER_GUI		0x00030307 /* GUI File Version */


#define TX_VOLUME_MIC		5
#define TX_VOLUME_LINE		5
#define TX_VOLUME_AUX		7
#define TX_VOLUME_ANALOG	10
#define TX_VOLUME_UNKNOWN	10


#define MAX_VAL_NAME 254
#define MAX_MODS     127
#define MAX_FLTRS    127

#define PPJDLL_NAME		TEXT("PPJoyEx")
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
#define BALOON_PPJ_CNCT		"PPJoy Connected"
#define BALOON_PPJ_DISCNCT	"PPJoy Disconnected"
#define BALOON_PPJ_NOTFND	"PPJoy cannot be found"
#define BALOON_PPJ_INVHNDL	"PPJoy device handle is invalid"
	


#define SPP_HOME_URL		"http://www.SmartPropoPlus.com"
#define SPP_HOME_TITLE		"SmartPropoPlus Home"
#define	SPP_MAIN_TEXT		"\
Version %d.%d.%d - Build %s\n\
Copyright (c) 2005-2008 by Shaul Eizikovich\n\n\
This software is based on SmartPropo by Tsutomu SEKI\n\
You may use and destribute this program freely.\n\n\
SmartPropoPlus enables you to connect your R/C transmitter to your computer through \
one of the sound-card inputs (e.g. Microphone, Line-in) so it can be used as a joystick.\n\
For further details please go to the SmartPropoPlus Home using the link below."

#define CONSOLE_TITLE		"SmartPropoPlus Console"
#define	MUTXCONSOLE			"SPPconsole Exists Mutex 1"
#define	MUTXWINMM			"WINMM Dll is running"
#define	MUTXPROPOSTARTED	"WINMM Dll - Propo Started"
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


#define MSG_DPPJSTAT_DISCN	1984	// LPARAM: Intentional disconnection

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

/* Function Prototypes */
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
char * GetMixerName(void);
char * GetCurrentMixerDevice();
int GetCurrentInputLine(unsigned int *SrcID);
void SetCurrentMixerDevice(const char * MixerName);
void SwitchMixerRequest(const char * MixerName);
void SwitchMixerAck(const char * MixerName);
void SetSwitchMixerRequestStat(enum MDSTAT Stat);
void SetCurrentInputLine(const char * MixerName, unsigned int SrcID);
far void * CreateDataBlock(struct Modulations * data);
int isVista(void);
int isVistaSP1OrHigher(void);


int w2char(LPWSTR wIn, char * cOut, int size);
void SetNumberOfFilters(const int n);
int GetNumberOfFilters();
void SetSelectedFilterIndex(const int i);
int GetSelectedFilterIndex();
char * GetFilterNameByIndex(const int i);
//void SetFilterNameByIndex(const int i, const char * name);
//char * GetFilterNameByIndex(const int i);
void SetFilterNames(const char ** name);

void SppMessageBoxWithErrorCode(void);

#endif // __SMARTPROPOPLUS

#if defined(__cplusplus)
}
#endif