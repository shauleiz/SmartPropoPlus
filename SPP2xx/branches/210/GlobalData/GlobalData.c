/** Registry and Global memory area management for SPP **/
/** Commonly used by the SPPconsole and the winmm.DLL  **/

#include "GlobalData.h"

struct SharedDataBlock * gpSharedBlock;
HANDLE	ghDataLock;

int isGlobalDataValid()
{
	return 1;
}

int isFmsRegistryExist()
{

	long res;
	HKEY hkResult;

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_FMS, 0, KEY_READ, &hkResult);
	if (res != ERROR_SUCCESS)
		return 0;

	RegCloseKey(hkResult);
	return 1;
}

int isGlobalMemoryExist()
{
	return 0;
}

int isSppRegistryExist()
{
	LONG res;
	HKEY hkResult;

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_READ, &hkResult);
	if (res != ERROR_SUCCESS)
		return 0;

	RegCloseKey(hkResult);
	return 1;

}

/* Create an empty FMS registry key */
int CreateEmptyFmsRegistry()
{
	LONG res;
	HKEY hkFms;

	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_FMS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkFms, NULL);		
	if (res != ERROR_SUCCESS)
		return 0;
	RegCloseKey(hkFms);
	return 1;
}

/* Create a default SPP registry key, subkeys and values */
int CreateDefaultSppRegistry()
{
	LONG res;
	HKEY hkSpp, hkMod;
	const char *DefMods[] = MOD_DEF_STR;
	int nMod = 0;
	int AutoMode = 1;
	int PositiveShift = 1;

	/* Create the SPP key */
	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_SPP, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkSpp, NULL);		
	if (res != ERROR_SUCCESS)
		return 0;

	/* Insert default shift values */
	res =  RegSetValueEx(hkSpp, SHIFT_POS  ,0, REG_BINARY, (const BYTE* )&PositiveShift, 4);
	res =  RegSetValueEx(hkSpp, SHIFT_AUTO ,0, REG_BINARY, (const BYTE* )&AutoMode, 4);


	/* Create the Modulation-Types key */
	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_MOD, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkMod, NULL);		
	if (res != ERROR_SUCCESS)
		return 0;

	/* Insert default modulation values */
	while (DefMods[nMod*2])
	{
		RegSetValueEx(hkMod, DefMods[nMod*2],0, REG_SZ, (const BYTE* )DefMods[nMod*2 +1], 1+strlen(DefMods[nMod*2 +1]));
		nMod++;
	};

	/* Mark the default ACTIVE modulation (PPM) */
	RegSetValueEx(hkMod, MOD_ACTIVE ,0, REG_SZ, (const BYTE* )MOD_TYPE_PPM, 1+strlen(MOD_TYPE_PPM));

	/* Create Shift related values */

	RegCloseKey(hkSpp);
	RegCloseKey(hkMod);
	return 1;
}

struct Modulations * GetModulationFromRegistry(int Create)
{
	struct Modulation ** Mod;
	struct Modulations * Out;
	LONG res;
	HKEY hkResult, hkSpp;
	char ValueName[MAX_VAL_NAME];
	unsigned long	ValueNameSize;
	unsigned char	ValueData[MAX_VAL_NAME];
	unsigned long	ValueDataSize;
	unsigned int i=0, index=0;
	int iActive = -1;
	char Active[MAX_VAL_NAME] = "";
	unsigned long nValues, MaxValueNameLength, MaxValueDataLength;
	int ShiftAutoDetect, PositiveShift;

	/* Test Registry - Create default entries if does not exist */
	if (Create && !isFmsRegistryExist())
		CreateEmptyFmsRegistry();
		
	if (Create && !isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP  key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE , &hkSpp);
	if (res != ERROR_SUCCESS)
		return NULL;

	/* Get Shift data */	
	ValueDataSize = 4;
	res = RegQueryValueEx(hkSpp, SHIFT_POS,  NULL, NULL, (unsigned char *)&PositiveShift,  &ValueDataSize);
	res = RegQueryValueEx(hkSpp, SHIFT_AUTO, NULL, NULL, (unsigned char *)&ShiftAutoDetect,  &ValueDataSize);



	/* Open SPP Modulation key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_MOD, 0, KEY_QUERY_VALUE , &hkResult);
	if (res != ERROR_SUCCESS)
		return NULL;
	res = RegQueryInfoKey(hkResult, NULL, NULL, NULL, NULL, NULL, NULL, &nValues, &MaxValueNameLength, &MaxValueDataLength, NULL, NULL);
	if (res != ERROR_SUCCESS)
		return NULL;

	/* Get the active modulation */
	ValueDataSize = MaxValueDataLength+1;
	res = RegQueryValueEx(hkResult, MOD_ACTIVE, NULL, NULL, (unsigned char *)Active,  &ValueDataSize);

	Mod = (struct Modulation **)malloc(nValues*sizeof(struct Modulation *));
	for (i=0 ; i<nValues ; i++)
	{
		if (res != ERROR_SUCCESS)
			break;

		/* Get next value */
		ValueNameSize = MaxValueNameLength+1;
		ValueDataSize = MaxValueDataLength+1;
		res = RegEnumValue(hkResult, i, ValueName, &ValueNameSize, 0, NULL, ValueData, &ValueDataSize);
		
		/* Is this is value a normal entry?  */
		if (strcmp(ValueName, MOD_ACTIVE))
		{ /* Not "Active"  - normal entry */
			Mod[index] = (struct Modulation *)malloc(sizeof(struct Modulation));
			Mod[index]->ModTypeDisplay = strdup((const char *)ValueData);
			Mod[index]->ModTypeInternal = strdup(ValueName);
			Mod[index]->index = index;
			if (!strcmp(Mod[index]->ModTypeInternal, &(Active[0])))
				iActive = index;
			index++;
		};
	};

	Mod[index] = NULL; /* Final entry */

	/* Pack data in structure */
	Out = (struct Modulations *)malloc(sizeof(struct Modulations));
	Out->ModulationList = Mod;
	Out->Active = iActive;
	Out->PositiveShift = PositiveShift;
	Out->ShiftAutoDetect = ShiftAutoDetect;

	RegCloseKey(hkResult);
	return Out;
}

struct Modulations * GetModulationFromGlobalMemory()
{
	return NULL;
}

struct Modulations * GetModulation(int Create)
{
	struct Modulations * Out;

	Out =  GetModulationFromGlobalMemory();
	if (Out)
		return Out;
	else
		return GetModulationFromRegistry(Create);
}

int SetActiveModeToRegistry(const char * selected)
{
	LONG res;
	HKEY hkResult;
	const char *DefMods[] = MOD_DEF_STR;
	int nMod=0;

	/* Test if registry key exists */
	if (!isSppRegistryExist())
		return -1;
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_MOD, 0, KEY_ALL_ACCESS , &hkResult);
	if (res != ERROR_SUCCESS)
		return -1;

	/* Convert Display Name to Internal Name */
	while (DefMods[nMod*2])
	{
		if (strcmp(DefMods[1+nMod*2], selected))
		{
			nMod++;
			continue;
		};
		break;
	};

	/* Test if entry exists */
	res = RegQueryValueEx(hkResult, DefMods[nMod*2], NULL, NULL, NULL,  NULL);
	if (res != ERROR_SUCCESS)
		return -1;

	/* Set Active entry */
	RegSetValueEx(hkResult, MOD_ACTIVE,0, REG_SZ, (const BYTE* )DefMods[nMod*2], 1+strlen(DefMods[nMod*2]));

	RegCloseKey(hkResult);
	return nMod;
}


int SetActiveMode(const char * selected)
{
	return SetActiveModeToRegistry(selected);
}

void SetShiftAutoDetectToRegistry(const int sel)
{
	LONG res;
	HKEY hkResult;

	/* Test if registry key exists */
	if (!isSppRegistryExist())
		return;
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS , &hkResult);
	if (res != ERROR_SUCCESS)
		return;

	/* Insert default shift values */
	res =  RegSetValueEx(hkResult, SHIFT_AUTO ,0, REG_BINARY, (const BYTE* )&sel, 4);
	RegCloseKey(hkResult);
}


void SetShiftAutoDetect(const int sel)
{
	SetShiftAutoDetectToRegistry(sel);
}


void SetPositiveShiftToRegistry(const int sel)
{
	LONG res;
	HKEY hkResult;

	/* Test if registry key exists */
	if (!isSppRegistryExist())
		return;
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS , &hkResult);
	if (res != ERROR_SUCCESS)
		return;

	/* Insert default shift values */
	res =  RegSetValueEx(hkResult, SHIFT_POS ,0, REG_BINARY, (const BYTE* )&sel, 4);
	RegCloseKey(hkResult);
}


void SetPositiveShift(const int sel)
{
	SetPositiveShiftToRegistry(sel);
}

far void * CreateSharedDataStruct(struct Modulations * data)
{
    HANDLE hFileMapping;
	DWORD dwMapErr;
	int index;
	const char *DefMods[] = MOD_DEF_STR;
	
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);	
	
    hFileMapping = CreateFileMapping
		(
		(HANDLE)0xFFFFFFFF,  // File handle
		NULL,                // Security attributes
		PAGE_READWRITE,      // Protection
		0,                   // Size - high 32 bits
		1<<16,               // Size - low 32 bits
		BLOCK_LABEL); // Name
	
    dwMapErr = GetLastError();
	
    gpSharedBlock = (struct SharedDataBlock *)MapViewOfFile
		(
		hFileMapping,        // File mapping object
		FILE_MAP_ALL_ACCESS, // Read/Write
		0,                  // Offset - high 32 bits
		0,                  // Offset - low 32 bits
		0);                 // Map the whole thing
	
    // Only initialize shared memory if we actually created.
    if (dwMapErr != ERROR_ALREADY_EXISTS)
	{
		/* Get all available modulation types */
		index = 0;
		if (data)
		{	/* Data from registry */
			while (data->ModulationList[index]  && index<MAX_MODS) 
			{
				gpSharedBlock->ModName[index] = (char __based(gpSharedBlock) *)strdup(data->ModulationList[index]->ModTypeInternal);
				/* Mark selected Modulation type */
				if (index == data->Active)
					gpSharedBlock->ActiveModulation.iModType = index;
				index++;
			} ;

			gpSharedBlock->ActiveModulation.ActiveModShift		= data->PositiveShift;
			gpSharedBlock->ActiveModulation.AutoDetectModShift	= data->ShiftAutoDetect;
		}
		else
		{	/* Default data */
			while (DefMods[index*2])
			{
				gpSharedBlock->ModName[index] = (char __based(gpSharedBlock) *)strdup(DefMods[index*2]);
				if (!strcmp(gpSharedBlock->ModName[index], MOD_TYPE_PPM))
					gpSharedBlock->ActiveModulation.iModType = index;
				index++;
			};
			
			
			/* Get shift-related info */
			gpSharedBlock->ActiveModulation.ActiveModShift		= 1;
			gpSharedBlock->ActiveModulation.AutoDetectModShift	= 1;
		};
			
		gpSharedBlock->ModName[index] = NULL;

		/* Store version of THIS file */
		gpSharedBlock->VersionDll = VER_DLL;
	};
	
    ReleaseMutex(ghDataLock);

	return (far void *)gpSharedBlock;
}