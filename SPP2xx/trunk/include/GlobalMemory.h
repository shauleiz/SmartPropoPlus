/* Global memory header file */
#include <windows.h>

/* Structures */
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

/* Parameter Declarations */
struct SharedDataBlock * gpSharedBlock;
HANDLE	ghDataLock;


/* Function prototypes */
int isGlobalMemoryExist();
int GetModulationIndexFromGlobalMemory(const char * InternalModName);
far void * OpenSharedDataStruct(void);

int SetActiveModeToGlobalMemory(const char * selected);
int SetShiftAutoDetectToGlobalMemory(const int sel);
void SetPositiveShiftToGlobalMemory(const int sel);
struct Modulations * GetModulationFromGlobalMemory();

far void * CreateSharedDataStruct(struct Modulations * data);
