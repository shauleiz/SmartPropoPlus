/* Generic SmartPropoPlus source  file */
#include <stdio.h>
#include "SmartPropoPlus.h"
//#include "GlobalMemory.h"
#include "SppRegistry.h"

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

void SetLocationDialogWindow(int x, int y)
{
	SetLocationDialogWindowToRegistry(x,y);
}

BOOL GetLocationDialogWindow(int * x, int * y)
{
	return GetLocationDialogWindowFromRegistry(x,y);
}

//void SetNumberOfFilters(const int n)
//{
//	SetNumberOfFiltersToGlobalMemory(n);
//}

//int GetNumberOfFilters()
//{
//	return GetNumberOfFiltersFromGlobalMemory();
//};
//
//void SetSelectedFilterIndex(const int i)
//{
//	char * name;
//
//	//SetSelectedFilterIndexToGlobalMemory(i);
//	name  = GetFilterNameByIndexFromGlobalMemory(i);
//
//	if (i>=0 && name && strlen(name) )
//		SetSelectedFilterNameToRegistry(name);
//	else
//		SetSelectedFilterNameToRegistry("");
//}
//
void SetSelectedFilter(const char * FilterName)
{
	SetSelectedFilterNameToRegistry(FilterName);
}
//int GetSelectedFilterIndex()
//{
//	char * name;
//	int index;
//
//
//	/* Not found - get name of selected filter from registry and convert to index */
//	name = GetSelectedFilterNameFromRegistry();
//	if (name && strlen(name))
//		index  = GetFilterIndexByNameFromGlobalMemory(name);
//	else
//		/* Get the index of the selected filter from global memory */
//		index  = GetSelectedFilterIndexFromGlobalMemory();
//
//	/* Update the global memory */
//	SetSelectedFilterIndexToGlobalMemory(index);
//	return index;
//}
//
//
//void SetFilterNames(const char ** name)
//{
//	SetFilterNamesToGlobalMemory(name);
//}
//char * GetFilterNameByIndex(const int i)
//{
//	return GetFilterNameByIndexFromGlobalMemory(i);
//}



/*
	Set the current active modulation type
*/
int SetActiveMode(const char * selected)
{
	//SetActiveModeToGlobalMemory(selected);
	return SetActiveModeToRegistry(selected);
}

/*
	Set the current shift auto-detect value
*/
void SetShiftAutoDetect(const int sel)
{
	//SetShiftAutoDetectToGlobalMemory(sel);
	SetShiftAutoDetectToRegistry(sel);
}

/*
	Set the current shift polarity value
*/
void SetPositiveShift(const int sel)
{
	//SetPositiveShiftToGlobalMemory(sel);
	SetPositiveShiftToRegistry(sel);
}


/*
	Get modulation data.
	First try the global data. If does not exist get it from the registry
*/
struct Modulations * GetModulation(int Create)
{
	//struct Modulations * Out;

	//Out =  GetModulationFromGlobalMemory();
	//if (Out)
	//	return Out;
	//else
		return GetModulationFromRegistry(Create);
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

/* Test existence of Audi2Joystick registry keys */
int isBaseRegistryExist()
{

	long res;
	HKEY hkResult;

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_BASE, 0, KEY_READ, &hkResult);
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
	int AudioState;
	//AudioState = GetCurrentAudioStateFromGlobalMemory();
	//if (AudioState>=0)
	//	return AudioState;
	//else
	//{
		AudioState =  GetCurrentAudioStateFromRegistry();
	//	SetCurrentAudioStateToGlobalMemory(AudioState);
		return AudioState;
	//}
}

void SetCurrentAudioState(int Active)
{
	SetCurrentAudioStateToRegistry(Active);
	//SetCurrentAudioStateToGlobalMemory(Active);
}

int GetCurrentPpjoyState()
{
	return GetCurrentPpjoyStateFromRegistry();
}

void SetCurrentPpjoyState(int Active)
{
	SetCurrentPpjoyStateToRegistry(Active);
}

char * GetCurrentMixerDevice()
{
	char * MixerName;
	int AudioState;

	AudioState = GetCurrentAudioState();

	if (AudioState)
	{ /* Checkbox "Enable Audio input selection" is checked */
		//MixerName = GetCurrentMixerDeviceFromGlobalMemory();
		//if (!MixerName || !strlen(MixerName))
		MixerName =  GetCurrentMixerDeviceFromRegistry();
		return MixerName;
	}
	else
		return "";
}

int GetCurrentInputLine(unsigned int *SrcID)
{
	return GetCurrentInputLineFromRegistry(SrcID);
}

int GetWaveInParams(int *nSamples, int *nBits, int *nBuffers, int *BufferSize)
{
	return GetWaveInParamsFromRegistry(nSamples, nBits, nBuffers, BufferSize);
}
void SetCurrentMixerDevice(const char * MixerName)
{
	SetCurrentMixerDeviceToRegistry(MixerName);
	//SetCurrentMixerDeviceToGlobalMemory(MixerName);
}

void SetCurrentInputLine(const char * MixerName, unsigned int SrcID)
{
	SetCurrentInputLineToRegistry(MixerName, SrcID);
}

//far void * CreateDataBlock(struct Modulations * data)
//{
//	return  CreateSharedDataStruct(data);
//}

int GetAntiJitterState()
{
	return GetAntiJitterStateFromRegistry();
}

void SetAntiJitterState(int enable)
{
	SetAntiJitterStateToRegistry(enable);
}

int GetSppConsoleFullPath(char * Path, UINT PathSize)
{
	return GetSppConsoleFullPathFromRegistry(Path,PathSize);
}

/* 
	Get the Fms.exe full path in 'Path'
	Return:
		1	If Both Fms.exe & Winmm.dll are located in the FMS folder
		0	If FMS not installed
		-1	If Winmm.dll is missing
		-2	If FMS instaled but Fms.exe is missing
*/
int GetFmsFullPath(char * Path, UINT PathSize)
{
	int GetFmsStat;
	char TmpPath[MAX_PATH+1] = "";
	HANDLE  hFile;
	WIN32_FIND_DATA FileData;

	if (!Path)
		return 0;

	/* Get the FMS directory from the registry */
	GetFmsStat = GetFmsFullPathFromRegistry(&(TmpPath[0]),PathSize);
	if (!GetFmsStat)
		return 0;

	/* Check that file winmm.dll exists in the FMS directory */
	sprintf(Path,"%s\\Winmm.dll", TmpPath);
	hFile = FindFirstFile((LPCTSTR)Path, &FileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return -1;
	FindClose(hFile);

	/* Check that file Fms.exe exists in the FMS directory */
	sprintf(Path,"%s\\Fms.exe", TmpPath);
	hFile = FindFirstFile((LPCTSTR)Path, &FileData);
	if (hFile == INVALID_HANDLE_VALUE)
		return -2;
	FindClose(hFile);


	return 1;
}
 