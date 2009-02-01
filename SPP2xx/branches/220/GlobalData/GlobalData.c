/** Registry and Global memory area management for SPP **/
/** Commonly used by the SPPconsole and the winmm.DLL  **/

#include "GlobalData.h"

struct SharedDataBlock * gpSharedBlock;
HANDLE	ghDataLock;

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

int isGlobalDataValid()
{
	struct Modulations *  Modulation;

	/* Download configuration from the registry (if exists) */
	Modulation= GetModulation(0);

	/* Create the Global memory area (a.k.a. Shared Data) and upload configuration to it */
	CreateSharedDataStruct(Modulation);

	if (gpSharedBlock)
		return 1;
	else
		return 0;

}


/*********************** Registry access functions ***********************/
/* Test existence of FMS registry keys */
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

/* Test existence of SPP registry keys */
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

/*
	Get modulation data from SPP registry keys
	First test existence - if does not exist then create with default values
	Then get shift-related data, active modulation and all existing modulation types
	Exit with all these data in 'Modulation' structure
*/
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


int isGlobalMemoryExist()
{
	if (!gpSharedBlock)
	{
		OpenSharedDataStruct();
/*		SetGuiDataToGlobalMemory();*/
	};

	if (gpSharedBlock)
		return 1;
	else
		return 0;
}

int SetGuiDataToGlobalMemory()
{
	if (!isGlobalMemoryExist())
		return 0;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	/* GUI Version */
	gpSharedBlock->VersionGui = VER_GUI;

	/* Release mutex */
	ReleaseMutex(ghDataLock);

	return 1;

}




struct Modulations * GetModulationFromGlobalMemory()
{
	struct Modulation ** Mod;
	struct Modulations * out;
	unsigned int VersionDll;
	int index;
//	char tmp[MAX_VAL_NAME];

	if (!isGlobalMemoryExist())
		return NULL;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	VersionDll = gpSharedBlock->VersionDll;

	Mod = (struct Modulation **)calloc(gpSharedBlock->nModulations+1, sizeof(struct Modulation *));
	index = 0;
	
	do 
	{
		Mod[index] = (struct Modulation *)malloc(sizeof(struct Modulation));
		Mod[index]->index = index;
		Mod[index]->ModTypeInternal =  strdup((char *)gpSharedBlock->pInternalModName[index]);
		index++;
	}while ((char *)gpSharedBlock->pInternalModName[index]);

	out = (struct Modulations *)calloc(1, sizeof(struct Modulations));
	out->ModulationList = Mod;
	out->Active = gpSharedBlock->ActiveModulation.iModType;
	out->PositiveShift = gpSharedBlock->ActiveModulation.ActiveModShift;
	out->ShiftAutoDetect = gpSharedBlock->ActiveModulation.AutoDetectModShift;
	
	ReleaseMutex(ghDataLock);

	return out;
}

int GetModulationIndexFromGlobalMemory(const char * InternalModName)
{
	int index;

	if (!isGlobalMemoryExist() || !InternalModName)
		return 0;
	
	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
	for (index = 0 ; index < gpSharedBlock->nModulations ; index++)
	{
		if (strcmp((char *)gpSharedBlock->pInternalModName[index], InternalModName))
			continue;
		ReleaseMutex(ghDataLock);
		return index;
	}

	ReleaseMutex(ghDataLock);
	return -1;
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

/*
	Set the current active modulation type in the registry
*/
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

int SetActiveModeToGlobalMemory(const char * selected)
{
	const char *DefMods[] = MOD_DEF_STR;
	int nMod=0;
	int iMod;

	if (!isGlobalMemoryExist())
		return 0;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

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

	iMod  = GetModulationIndexFromGlobalMemory(DefMods[nMod*2]);

	if (iMod >= 0)
		gpSharedBlock->ActiveModulation.iModType = iMod;
	else
		iMod = nMod;

	ReleaseMutex(ghDataLock);

	return iMod;
}


int SetShiftAutoDetectToGlobalMemory(const int sel)
{
	if (!isGlobalMemoryExist())
		return 0;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->ActiveModulation.AutoDetectModShift = sel;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);

	return sel;

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
	Set the current shift auto-detect value in the registry
*/
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


/*
	Set the current shift auto-detect value
*/
void SetShiftAutoDetect(const int sel)
{
	SetShiftAutoDetectToGlobalMemory(sel);
	SetShiftAutoDetectToRegistry(sel);
}


/*
	Set the current shift polarity value in the registry
*/
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

void SetPositiveShiftToGlobalMemory(const int sel)
{
	if (!isGlobalMemoryExist())
		return;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->ActiveModulation.ActiveModShift = sel;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);

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
	Create the global shared memory that is used by the DLL and the GUI
	> Open an existing Mutex or create initially owned Mutex named MUTEX_LABEL ("SPP Global Shared memory Mutex")
	> Create or get a File Mapping Object for an operating-system paging file. Object name is BLOCK_LABEL
	> If error from creation of the  File Mapping Object is ERROR_ALREADY_EXISTS then it means that it already exists
	> Map the whole of the Object into a File View that is casted into a global pointer.
	> If this is the CREATION of the Object then the global structure is populated
	> The Mutex is released just before the function returns
	Return: File View
*/
far void * CreateSharedDataStruct(struct Modulations * data)
{
    HANDLE hFileMapping;
	DWORD dwMapErr;
	int index;
	const char *DefMods[] = MOD_DEF_STR;
	int nModulations;
	char * pCurrentModName;
	
	/* Wait for Mutex */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);

	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
    hFileMapping = CreateFileMapping
		(
		INVALID_HANDLE_VALUE,// File handle
		NULL,                // Security attributes
		PAGE_READWRITE,      // Protection
		0,                   // Size - high 32 bits
		1<<16 ,               // Size - low 32 bits
		BLOCK_LABEL); // Name
	
    dwMapErr = GetLastError();
	
    gpSharedBlock = (struct SharedDataBlock *)MapViewOfFile
		(
		hFileMapping,        // File mapping object
		FILE_MAP_ALL_ACCESS, // Read/Write
		0,                  // Offset - high 32 bits
		0,                  // Offset - low 32 bits
		0);                 // Map the whole thing
	
	if (!gpSharedBlock)
	{
		ReleaseMutex(ghDataLock);
		return NULL;
	};

    // Only initialize shared memory if we actually created.
    if (dwMapErr != ERROR_ALREADY_EXISTS)
	{
		/* Get all available modulation types */
		index = 0;
		if (data)
		{	/* Data from registry */

			/* Get number of modulations */
			nModulations=0;
			while (data->ModulationList[nModulations])
				nModulations++;
			gpSharedBlock->nModulations = nModulations;

			/* Get starting point for list of (internal) modulation names */
			pCurrentModName = (char *)gpSharedBlock + sizeof(struct SharedDataBlock);

			for  (index=0 ; index<=nModulations+1 ; index++) 
			{
				if (!data->ModulationList[index] || index == MAX_MODS)
				{
					gpSharedBlock->pInternalModName[index] = NULL;
					break;
				};

				strcpy(pCurrentModName,  data->ModulationList[index]->ModTypeInternal);
				gpSharedBlock->pInternalModName[index] = (char __based( gpSharedBlock ) *)pCurrentModName;

				/* Mark selected Modulation type */
				if (index == data->Active)
					gpSharedBlock->ActiveModulation.iModType = index;

				pCurrentModName+=strlen(data->ModulationList[index]->ModTypeInternal)+1;
			};

			gpSharedBlock->ActiveModulation.ActiveModShift		= data->PositiveShift;
			gpSharedBlock->ActiveModulation.AutoDetectModShift	= data->ShiftAutoDetect;
		}
		else
		{	/* Default data */

			/* Get number of modulations */
			nModulations=0;
			while (DefMods[nModulations*2])
				nModulations++;
			gpSharedBlock->nModulations = nModulations;

			/* Get starting point for list of (internal) modulation names */
			pCurrentModName = (char *)gpSharedBlock + sizeof(struct SharedDataBlock);

			for  (index=0 ; index<=nModulations+1 ; index++) 
			{
				if (!DefMods[index*2] || index == MAX_MODS)
				{
					gpSharedBlock->pInternalModName[index] = NULL;
					break;
				};

				strcpy(pCurrentModName,  DefMods[index*2]);
				gpSharedBlock->pInternalModName[index] = (char __based( gpSharedBlock ) *)pCurrentModName;

				/* Mark selected Modulation type */
				if (!strcmp(gpSharedBlock->pInternalModName[index], MOD_TYPE_PPM))
					gpSharedBlock->ActiveModulation.iModType = index;

				pCurrentModName+=strlen( DefMods[index*2])+1;
			};			
			
			/* Get shift-related info */
			gpSharedBlock->ActiveModulation.ActiveModShift		= 1;
			gpSharedBlock->ActiveModulation.AutoDetectModShift	= 1;
		};
			
		/* Store version of THIS file */
		gpSharedBlock->VersionDll = VER_DLL;
		gpSharedBlock->GuiDialogBoxTitle[0] = '\0';

		/* Default values */
		gpSharedBlock->VersionGui = 0;

	};
	
    ReleaseMutex(ghDataLock);

	return (far void *)gpSharedBlock;
}

far void * OpenSharedDataStruct(void)
{
    HANDLE hFileMapping;
	const char *DefMods[] = MOD_DEF_STR;
	
	/* Wait for Mutex */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);

	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
    hFileMapping = OpenFileMapping
		(
		FILE_MAP_ALL_ACCESS, 
		FALSE,
		BLOCK_LABEL); // Name
	
	
    gpSharedBlock = (struct SharedDataBlock *)MapViewOfFile
		(
		hFileMapping,        // File mapping object
		FILE_MAP_ALL_ACCESS, // Read/Write
		0,                  // Offset - high 32 bits
		0,                  // Offset - low 32 bits
		0);                 // Map the whole thing
	

    ReleaseMutex(ghDataLock);

	if (!gpSharedBlock)
		return NULL;
	return (far void *)gpSharedBlock;
}

#if 0
far struct SharedDataBlock * GetSharedDataStruct(void)
{
    HANDLE hFileMapping;
	DWORD dwMapErr;
	struct SharedDataBlock * out;
	int index;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);

	ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
    hFileMapping = CreateFileMapping
		(
		INVALID_HANDLE_VALUE,// File handle
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
	
    // If shared memory does not exist - just go out
    if (dwMapErr != ERROR_ALREADY_EXISTS)
		return NULL;

	out  = (struct SharedDataBlock *)calloc(1, sizeof(struct SharedDataBlock *));

	out->ModName[0] = (char __based(gpSharedBlock) *)calloc(MAX_MODS,sizeof(char *));
	out->SrcName[0] = (char __based(gpSharedBlock) *)calloc(MAX_MODS,sizeof(char *));

	out->VersionGui =  ((struct SharedDataBlock *)gpSharedBlock)->VersionGui;
	out->VersionDll =  ((struct SharedDataBlock *)gpSharedBlock)->VersionDll;
	out->iActiveSrc = ((struct SharedDataBlock *)gpSharedBlock)->iActiveSrc;
	out->ActiveModulation.ActiveModShift = ((struct SharedDataBlock *)gpSharedBlock)->ActiveModulation.ActiveModShift;
	out->ActiveModulation.AutoDetectModShift = ((struct SharedDataBlock *)gpSharedBlock)->ActiveModulation.AutoDetectModShift;
	out->ActiveModulation.iModType = ((struct SharedDataBlock *)gpSharedBlock)->ActiveModulation.iModType;
	for  (index=0;  index<MAX_MODS ; index++) 
	{
		out->ModName[index] = (char __based(gpSharedBlock) *)strdup(((struct SharedDataBlock *)gpSharedBlock)->ModName[index]);
		if (!((struct SharedDataBlock *)gpSharedBlock)->ModName[index])
			break;
	};
	for  (index=0;  index<MAX_MODS ; index++) 
	{
		out->SrcName[index] = (char __based(gpSharedBlock) *)strdup(((struct SharedDataBlock *)gpSharedBlock)->SrcName[index]);
		if (!((struct SharedDataBlock *)gpSharedBlock)->SrcName[index])
			break;
	};

    ReleaseMutex(ghDataLock);
	return out;
}
#endif
