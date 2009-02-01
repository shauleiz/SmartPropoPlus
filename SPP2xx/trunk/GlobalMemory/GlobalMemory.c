/* Global memory source code  file */
#include "SmartPropoPlus.h"
#include "GlobalMemory.h"


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



int isGlobalMemoryExist()
{
	if (!gpSharedBlock)
		OpenSharedDataStruct();

	if (gpSharedBlock)
		return 1;
	else
		return 0;
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
