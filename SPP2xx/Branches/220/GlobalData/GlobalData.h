#include "windows.h"

/** Definitions **/
#define REG_FMS "Software\\Flying-Model-Simulator"
#define REG_SPP "Software\\Flying-Model-Simulator\\SmartPropoPlus"
#define REG_MOD "Software\\Flying-Model-Simulator\\SmartPropoPlus\\Modulation-Types"

#define MOD_TYPE_PPM	"PPM"
#define MOD_TYPE_JR 	"JR"
#define MOD_TYPE_FUT	"FUT"
#define MOD_TYPE_AIR1	"AIR1"
#define MOD_TYPE_AIR2	"AIR2"

#define MOD_NAME_PPM	"PPM (Generic)"
#define MOD_NAME_JR 	"JR (PCM)"
#define MOD_NAME_FUT	"Futaba (PCM)"
#define MOD_NAME_AIR1	"Sanwa/Air (PCM1)"
#define MOD_NAME_AIR2	"Sanwa/Air (PCM2)"

#define MOD_DEF_STR  {\
						MOD_TYPE_PPM, MOD_NAME_PPM,\
						MOD_TYPE_JR,  MOD_NAME_JR,\
						MOD_TYPE_FUT, MOD_NAME_FUT,\
						MOD_TYPE_AIR1,MOD_NAME_AIR1,\
						MOD_TYPE_AIR2,MOD_NAME_AIR2,\
						NULL}

#define	MNM_POS		"&Positive"
#define	MNM_NEG		"&Negative"
#define	MNM_POS_HID	"Positive"
#define	MNM_NEG_HID	"Negative"

#define MOD_ACTIVE	"Active"
#define SHIFT_POS	"Positive-Shift"
#define SHIFT_AUTO	"Shift-Auto-Detect"
#define DEBUG_LEVEL	"Debug-Level"

#define MUTEX_LABEL	"SPP Global Shared memory Mutex"
#define BLOCK_LABEL	"SPP Global Shared memory Block"
#define	VER_DLL		0x00020200
#define	VER_GUI		0x00020200

#define MAX_VAL_NAME 254
#define MAX_MODS     127

#define CTRL_LOG_FILE	"SPP_ctrl.log"
#define DATA_LOG_FILE	"SPP_data.log"
#define SEPARATOR		"*************************************************************************"

#define CONSOLE_TT			"SmartPropoPlus Console"
#define CONSOLE_TT_OFF		"SmartPropoPlus is OFF"
#define CONSOLE_TT_ON		"SmartPropoPlus is ON"
#define CONSOLE_BALOON_TTL	"SmartPropoPlus Console"
#define CONSOLE_BALOON_TXT	"Use this application to control:\n- Modulation Type\n- Modulation Shift\n\nChanges are effective Immediately!"
#define BALOON_DLL_STOPPED	"SmartPropoPlus stopped"
#define BALOON_DLL_STARTING	"SmartPropoPlus is starting"
#define BALOON_DLL_MODTYPE	"Modulation type:\t%s"
#define BALOON_DLL_AUTOON	"Polarity Auto-Detect:\tOn"
#define BALOON_DLL_AUTOOFF	"Polarity Auto-Detect:\tOff"
#define BALOON_DLL_SHIFTPOS	"Shift Polarity:\t\tPositive"
#define BALOON_DLL_SHIFTNEG	"Shift Polarity:\t\tNegative"

#define SPP_HOME_URL		"http://www.geocities.com/shaul_ei/SmartPropoPlus.html"
#define SPP_HOME_TITLE		"SmartPropoPlus Home"
#define	SPP_MAIN_TEXT		"\
Version %d.%d.%d - Build %s\n\
Copyright (c) 2005-2006 by Shaul Eizikovich\n\n\
This software is based on SmartPropo by Tsutomu SEKI\n\
You may use and destribute this program freely.\n\n\
SmartPropoPlus enables you to connect your R/C transmitter to your computer through \
one of the sound-card inputs (e.g. Microphone, Line-in) so it can be used as a joystick.\n\
SmartPropoPlus is intended to be used by users of FMS simulator, but you can try to use it \
for other purposes if you wish\n\n\
For further details please go to the SmartPropoPlus Home using the link below."

#define CONSOLE_TITLE		"SmartPropoPlus Console"
#define	MUTEX1				"SPPconsole Exists Mutex 1"
#define	MUTEX2				"WINMM Dll is running"
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
#define MSG_DLLSTOPPING		72	// WPARAM: Inform GUI that DLL is stopping
#define MSG_DLLSTARTING		73	// WPARAM: Inform GUI that DLL is starting

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

extern struct SharedDataBlock* gpSharedBlock;
/*
	Structure that hold all that the inter-process communication might require
	- DLL & GUI versions (For future compatibility tests)
	- Sub-Structure holding data about the currently selected modulation type, shift etc.
	- Array of pointers to ProcessPulsXXX functions (For DLL usage only - may be located elsewhere)
	- Number of modulation types 
	- Array of nModulations+1 offsets to (internal) names of modulations. Last value must be 0.
	  The names of modulations are located immediately after the structure.
	- Number of audio sources
	- Array of nSrcName+1 offsets to names of audio sources. Last value must be 0.
	  The source names are located immediately after the names of modulations.
	- Index of currently active audio source
*/
struct SharedDataBlock
{
	unsigned int VersionDll; //Version of DLL file
	unsigned int VersionGui; //Version of GUI file
	char GuiDialogBoxTitle[128]; // Title text of the GUI dialog box
	struct ActiveModulationData    //Modulation Data
	{
		int		iModType; // Index of active modulation
		BOOL		ActiveModShift; // Positive shift?
		BOOL		AutoDetectModShift; // Shift Auto-detect on?
	} ActiveModulation;
/*	far void  __based( gpSharedBlock ) ** ListProcessPulseFunc; */
	int nModulations;
	char __based( gpSharedBlock ) *pInternalModName[MAX_MODS];
	int nSrcName;
	char __based( gpSharedBlock ) *pSrcName[MAX_MODS];
	int iActiveSrc; // Index of active source	
};

/*** Prototypes ***/
struct Modulations * GetModulation(int Create);
int SetActiveMode(const char * selected);
void SetShiftAutoDetect(const int sel);
void SetPositiveShift(const int sel);
far void * CreateSharedDataStruct(struct Modulations * data);
far void * OpenSharedDataStruct(void);
int isFmsRegistryExist();
const char * ModeDisplayFromInternalName(const char * internal);
int SetGuiDataToGlobalMemory();


/*/////////////////////////////////////////////////////////////////////////*/
/* Global Variables */

// This is the global pointer to the shared data memory block.
// It will be initialized when winmm DLL initializes.
extern struct SharedDataBlock * gpSharedBlock;

// This mutex will protect the data so that only one
// process can access it at a time. It would be much
// better to use a readers/writers lock, but this
// is simpler for this example.
extern HANDLE	ghDataLock;


// Global Debug Level
extern int gDebugLevel;
//extern FILE * gCtrlLogFile;
