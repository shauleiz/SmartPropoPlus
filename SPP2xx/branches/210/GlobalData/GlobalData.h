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

#define MOD_ACTIVE	"Active"
#define SHIFT_POS	"Positive-Shift"
#define SHIFT_AUTO	"Shift-Auto-Detect"
#define DEBUG_LEVEL	"Debug-Level"

#define MUTEX_LABEL	"SPP Global Shared memory Mutex"
#define BLOCK_LABEL	"SPP Global Shared memory Block"
#define	VER_DLL		0x00020100

#define MAX_VAL_NAME 254
#define MAX_MODS     127

#define CTRL_LOG_FILE	"SPP_ctrl.log"
#define DATA_LOG_FILE	"SPP_data.log"
#define SEPARATOR		"*************************************************************************"

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

struct SharedDataBlock
{
	unsigned int VersionDll; //Version of DLL file
	unsigned int VersionGui; //Version of GUI file
	far void  __based( gpSharedBlock ) ** ListProcessPulseFunc;
	char __based( gpSharedBlock ) *ModName[MAX_MODS];
	char __based( gpSharedBlock ) *SrcName[MAX_MODS];
	struct ActiveModulationData    //Modulation Data
	{
		int		iModType; // Index of active modulation
		BOOL		ActiveModShift; // Positive shift?
		BOOL		AutoDetectModShift; // Shift Auto-detect on?
	} ActiveModulation;
	int iActiveSrc; // Index of active source	
};

/*** Prototypes ***/
int isGlobalDataValid();
struct Modulations * GetModulation(int Create);
int SetActiveMode(const char * selected);
void SetShiftAutoDetect(const int sel);
void SetPositiveShift(const int sel);
far void * CreateSharedDataStruct(struct Modulations * data);
int isFmsRegistryExist();


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
