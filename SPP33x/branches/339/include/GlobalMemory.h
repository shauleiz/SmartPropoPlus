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
*/
struct SharedDataBlock
{
	unsigned int VersionDll; //Version of DLL file
	unsigned int VersionGui; //Version of GUI file
	char GuiDialogBoxTitle[128]; // Title text of the GUI dialog box
	struct ActiveModulationData    //Modulation Data
	{
		int			iModType; // Index of active modulation
		BOOL		ActiveModShift; // Positive shift?
		BOOL		AutoDetectModShift; // Shift Auto-detect on?
	} ActiveModulation;
/*	far void  __based( gpSharedBlock ) ** ListProcessPulseFunc; */
	int nModulations;
	char __based( gpSharedBlock ) *pInternalModName[MAX_MODS];
	//int nSrcName;
	char SrcName[MAX_MODS];
	//int iActiveSrc; // Index of active source
	enum MDSTAT 
	{
		RUNNING=0, 
		CHANGE_REQ,
		STOPPING,
		STOPPED,
		STARTING,
		STARTED,
		FAILED,
	}  MixerDeviceStatus; // Status of Mixer Device changing process
	// Joystick channels postprocessor data
	int n_fltr;				// Number of supported filters (1 or more)
	int	i_sel_fltr;				// Index of selected filter (0 based, -1 means nothing)
	char __based( gpSharedBlock ) *name_fltr[MAX_FLTRS];	// list of display name of filters
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

void SetNumberOfFiltersToGlobalMemory(const int n);
int GetNumberOfFiltersFromGlobalMemory();
void SetSelectedFilterIndexToGlobalMemory(const int i);
int GetSelectedFilterIndexFromGlobalMemory();
void SetFilterNameByIndexToGlobalMemory(const int i, const char * name);
void SetFilterNamesToGlobalMemory(const char ** name);
char * GetFilterNameByIndexFromGlobalMemory(const int i);
void SetFilterNamesToGlobalMemory(const char ** name);

char * GetFilterIdByIndexFromGlobalMemory(const int i);
int GetFilterIndexByNameFromGlobalMemory(const char * name);

void SwitchMixerRequestViaGlobalMemory(const char * MixerName);
void SetSwitchMixerRequestStatToGlobalMemory(enum MDSTAT Stat);

far void * CreateSharedDataStruct(struct Modulations * data);
