/* Generic SmartPropoPlus source  file */
#include "SmartPropoPlus.h"
#include "GlobalMemory.h"
#include "SppRegistry.h"
#include <stdio.h>

void SppMessageBoxWithErrorCode(void)
{
	int error_code;
	LPVOID lpMsgBuf;
	char msg[1000];

	/* Get error code and convert it to string */
	error_code = GetLastError();
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
	sprintf(msg, "Error(%d): %s", error_code, (LPCTSTR)lpMsgBuf);
	MessageBox(NULL,msg, SPP_ERROR_MSG , MB_SYSTEMMODAL);
}

int w2char(LPWSTR wIn, char * cOut, int size)
{
	int i;
	for (i=0; i<size ; i++)
	{
		cOut[i] = (char)wIn[i];
		if (!cOut[i])
			break;
	};
	return i;
}

void SetNumberOfFilters(const int n)
{
	SetNumberOfFiltersToGlobalMemory(n);
}

int GetNumberOfFilters()
{
	return GetNumberOfFiltersFromGlobalMemory();
};

void SetSelectedFilterIndex(const int i)
{
	char * name;

	SetSelectedFilterIndexToGlobalMemory(i);
	name  = GetFilterNameByIndexFromGlobalMemory(i);

	if (i>=0 && name && strlen(name) )
		SetSelectedFilterNameToRegistry(name);
	else
		SetSelectedFilterNameToRegistry("");

	free(name);
}


int GetSelectedFilterIndex()
{
	char * name;
	int index;


	/* Not found - get name of selected filter from registry and convert to index */
	name = GetSelectedFilterNameFromRegistry();
	if (name && strlen(name))
		index  = GetFilterIndexByNameFromGlobalMemory(name);
	else
		/* Get the index of the selected filter from global memory */
		index  = GetSelectedFilterIndexFromGlobalMemory();

	/* Update the global memory */
	SetSelectedFilterIndexToGlobalMemory(index);

	free(name);
	return index;
}


//void SetFilterNameByIndexToGlobalMemory(const int i, const char * name);

void SetFilterNames(const char ** name)
{
	SetFilterNamesToGlobalMemory(name);
}
char * GetFilterNameByIndex(const int i)
{
	return GetFilterNameByIndexFromGlobalMemory(i);
}



/*
	Set the current active modulation type
*/
int SetActiveMode(const char * selected)
{

	SetActiveModeToGlobalMemory(selected);
	return SetActiveModeToRegistry(selected);
}

/*
	Set the current shift auto-detect value
*/
void SetShiftAutoDetect(const int sel)
{
	SetShiftAutoDetectToGlobalMemory(sel);
	SetShiftAutoDetectToRegistry(sel);
}

/*
	Set the current shift polarity value
*/
void SetPositiveShift(const int sel)
{
	SetPositiveShiftToGlobalMemory(sel);
	SetPositiveShiftToRegistry(sel);
}


/*
	Get modulation data.
	First try the global data. If does not exist get it from the registry
*/
struct Modulations * GetModulation(int Create)
{
	struct Modulations * Out;

	Out =  GetModulationFromGlobalMemory();
	if (Out)
		return Out;
	else
		return GetModulationFromRegistry(Create);
}
void FreeModulation(struct Modulations * modulation)
{
	int iModulation=0;

	do
	{
		free(modulation->ModulationList[iModulation]->ModTypeDisplay);
		free(modulation->ModulationList[iModulation]->ModTypeInternal);
		free(modulation->ModulationList[iModulation]);
		iModulation++;
	} while (modulation->ModulationList[iModulation]) ;

	free(modulation->ModulationList[iModulation]);
	free(modulation->ModulationList);
	free(modulation);
}

/*
	Get debug level
*/
int GetDebugLevel(void)
{
		return GetDebugLevelFromRegistry();
}


const char * ModeDisplayFromInternalName(const char * internal)
{
	int i=0;
	const char *DefMods[] = MOD_DEF_STR;

	while (DefMods[i*2])
	{
		if (!strcmp(DefMods[i*2], internal))
			return DefMods[i*2 + 1];
		i++;
	};

	return "******";
}

/* Test existence of FMS registry keys */
int isFmsRegistryExist()
{

	long res;
	HKEY hkResult;

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_FMS, 0, KEY_READ, &hkResult);
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		return 0;
	};

	RegCloseKey(hkResult);
	return 1;
}


/*
	Get default value (normalized) of volume per source type (Mike, Line In etc.)
*/
int GetDefaultVolumeValue(unsigned long SrcType)
{
	unsigned long VolumeValue;

	// Get the needed value from the registry
	if (GetDefaultVolumeValueFromRegistry(SrcType, &VolumeValue))
		return VolumeValue;

	// Calculate the needed volume
	if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
		VolumeValue = TX_VOLUME_MIC;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_LINE)
		VolumeValue = TX_VOLUME_LINE;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY)
		VolumeValue = TX_VOLUME_AUX;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_ANALOG)
		VolumeValue = TX_VOLUME_ANALOG;
	else
		VolumeValue = TX_VOLUME_UNKNOWN;

	// Record this value in the registry
	SetDefaultVolumeValueToRegistry(SrcType, VolumeValue);

	return VolumeValue;
}

/*
	Set default value (normalized) of volume per source type (Mike, Line In etc.)
*/
int SetDefaultVolumeValue(unsigned long SrcType, unsigned long  VolumeValue)
{
	return SetDefaultVolumeValueToRegistry(SrcType, VolumeValue);
}

int GetCurrentAudioState()
{
	return GetCurrentAudioStateFromRegistry();
}

void SetCurrentAudioState(int Active)
{
	SetCurrentAudioStateToRegistry(Active);
}

int GetCurrentPpjoyState()
{
	return GetCurrentPpjoyStateFromRegistry();
}

void SetCurrentPpjoyState(int Active)
{
	SetCurrentPpjoyStateToRegistry(Active);
}

LPWSTR GetMixerName(void)
{
	return GetMixerNameFromGlobalMemory();
}


LPWSTR GetCurrentMixerDevice()
{
	return GetCurrentMixerDeviceFromRegistry();	
}

LPWSTR GetCurrentEndpointDevice()
{
	return GetCurrentEndpointDeviceFromRegistry();	
}

int GetCurrentInputLine(unsigned int *SrcID)
{
	return GetCurrentInputLineFromRegistry(SrcID);
}

int GetInputLineSrcId(LPCWSTR MixerName, unsigned int *SrcID)
{
	return GetInputLineFromRegistry(MixerName, SrcID);
}

void SetCurrentMixerDevice(LPCWSTR MixerName)
{
	SetCurrentMixerDeviceToRegistry(MixerName);
}

void SetCurrentEndpointDevice(LPCWSTR MixerName)
{
	SetCurrentEndpointDeviceToRegistry(MixerName);
}

void SwitchMixerRequest(LPCWSTR MixerName)
{
	SwitchMixerRequestViaGlobalMemory(MixerName);
}

void SwitchMixerAck(LPWSTR MixerName)
{
	SwitchMixerAckViaGlobalMemory(MixerName);
}


void SetSwitchMixerRequestStat(enum MDSTAT Stat)
{
	SetSwitchMixerRequestStatToGlobalMemory(Stat);
}

void SetCurrentInputLine(LPCWSTR MixerName, unsigned int SrcID)
{
	SetCurrentInputLineToRegistry(MixerName, SrcID);
}

far void * CreateDataBlock(struct Modulations * data)
{
	return  CreateSharedDataStruct(data);
}

/* Test if this is a Vista machine or higher */
int isVista(void)
{
	OSVERSIONINFOEX InfoStruct;
	DWORD Mask;
	DWORDLONG ConditionMask=0;

	// Init
	InfoStruct.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	InfoStruct.dwMajorVersion = 6; // Vista
	Mask = VER_MAJORVERSION;
	VerSetConditionMask(ConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfo(&InfoStruct, Mask, ConditionMask);
}

/* Test if this is a Vista SP1 machine  or higher */
int isVistaSP1OrHigher(void)
{
	OSVERSIONINFOEX InfoStruct;
	DWORD Mask;
	DWORDLONG ConditionMask=0;

	// Init
	InfoStruct.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	InfoStruct.dwMajorVersion = 6;		// Vista
	InfoStruct.wServicePackMajor = 1;	// SP1
	InfoStruct.dwMinorVersion = 0;		// Vista
	InfoStruct.wServicePackMinor = 0;	// SP1
	Mask = VER_MAJORVERSION | VER_SERVICEPACKMAJOR | VER_MINORVERSION | VER_SERVICEPACKMINOR;
	VER_SET_CONDITION(ConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION( ConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL );
	VER_SET_CONDITION(ConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION( ConditionMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL );

	return VerifyVersionInfo(&InfoStruct, Mask, ConditionMask);
}
