/* Global memory source code  file */
#include <assert.h> 
#include "SmartPropoPlus.h"
#include "GlobalMemory.h"


void SetNumberOfFiltersToGlobalMemory(const int n)
{
	if (!isGlobalMemoryExist())
		return;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->n_fltr = n;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

	return;
}

int GetNumberOfFiltersFromGlobalMemory()
{
	int n;

	if (!isGlobalMemoryExist())
		return 0;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	n = gpSharedBlock->n_fltr;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

	return n;
}


void SetSelectedFilterIndexToGlobalMemory(const int i)
{
	if (!isGlobalMemoryExist())
		return;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->i_sel_fltr = i;

	/* Release acces lock */
	ReleaseMutex(ghDataLock); 
	CloseHandle(ghDataLock);

	return;
}


int GetSelectedFilterIndexFromGlobalMemory()
{
	int i;

	if (!isGlobalMemoryExist())
		return -1;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	i = gpSharedBlock->i_sel_fltr;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

	return i;
}


//void SetFilterNameByIndexToGlobalMemory(const int i, const char * name);
void SetFilterNamesToGlobalMemory(const char ** name)
{
	int index;
	size_t total_length=0;
	char * mode;
	char * pCurrentFilterName;


	if (!isGlobalMemoryExist())
		return;
	
	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
	// Find the begining of the list (just after the list of modulation names)
	for (index = 0 ; index < gpSharedBlock->nModulations ; index++)
	{
		mode = (char *)gpSharedBlock->pInternalModName[index];
		if (mode)
			total_length+=strlen(mode)+1;
		else
			break;
	};
	pCurrentFilterName = (char *)gpSharedBlock + sizeof(struct SharedDataBlock) + total_length + 1;
	
	// Take the filternames one by and Place them on the global memory
	index=0;
	while (name[index])
	{
		strcpy(pCurrentFilterName,name[index] );
		gpSharedBlock->name_fltr[index] = (char __based( gpSharedBlock ) *)pCurrentFilterName;
		pCurrentFilterName+=strlen(name[index])+1;
		index++;
	};
	gpSharedBlock->name_fltr[index] = NULL;
	

	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

}

char * GetFilterNameByIndexFromGlobalMemory(const int i)
{
	char * out;

	if (!isGlobalMemoryExist())
		return NULL;

	if (i>=gpSharedBlock->n_fltr || i<0)
		return NULL;


	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
	out = strdup((char *)gpSharedBlock->name_fltr[i]);

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

	return out;
}

int GetFilterIndexByNameFromGlobalMemory(const char * name)
{
	int i=0, out=-1;
	char * current;

	if (!isGlobalMemoryExist())
		return -1;

	if (i>=gpSharedBlock->n_fltr || i<0)
		return -1;
	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
	/* Go over all entries and compair to input string */
	for (i=0; i<MAX_FLTRS; i++)
	{
		current = (char *)gpSharedBlock->name_fltr[i];
		if (!strcmp(current, name))
		{
			out = i;
			break;
		};
	};

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);
	return out;
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
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

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
	CloseHandle(ghDataLock);

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
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->ActiveModulation.AutoDetectModShift = sel;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

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
	HANDLE hLock;

	if (!isGlobalMemoryExist() || !InternalModName)
		return 0;
	
	/* Open Mutex and Wait for it */
	hLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(hLock, INFINITE);
	
	/* Lock access to global memory */
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	for (index = 0 ; index < gpSharedBlock->nModulations ; index++)
	{
		if (strcmp((char *)gpSharedBlock->pInternalModName[index], InternalModName))
			continue;
		ReleaseMutex(hLock);
		CloseHandle(hLock);
		return index;
	}

	ReleaseMutex(hLock);
	CloseHandle(hLock);

	return -1;
}

far void * OpenSharedDataStruct(void)
{
    HANDLE hFileMapping;
	const char *DefMods[] = MOD_DEF_STR;

	gpSharedBlock = NULL;
	
	/* Wait for Mutex */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);

	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);
	
    hFileMapping = OpenFileMapping
		(
		FILE_MAP_ALL_ACCESS, 
		FALSE,
		BLOCK_LABEL); // Name
	
	/* Version 3.3.2 */
	if (!hFileMapping)
	{ 		
		ReleaseMutex(ghDataLock);
		CloseHandle(ghDataLock);

		return NULL;
	};
	
    gpSharedBlock = (struct SharedDataBlock *)MapViewOfFile
		(
		hFileMapping,        // File mapping object
		FILE_MAP_ALL_ACCESS, // Read/Write
		0,                  // Offset - high 32 bits
		0,                  // Offset - low 32 bits
		0);                 // Map the whole thing
	

    ReleaseMutex(ghDataLock);

	if (!gpSharedBlock)
	{
		/* Version 3.3.2 */
		SppMessageBoxWithErrorCode();
		return NULL;
	};

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
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->ActiveModulation.ActiveModShift = sel;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

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
	// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

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
	CloseHandle(ghDataLock);

	return out;
}


LPWSTR GetMixerNameFromGlobalMemory(void)
{	
	LPWSTR MixerName;

	if (!isGlobalMemoryExist())
		return NULL;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	//// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	MixerName = wcsdup(gpSharedBlock->SrcName);

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

	return MixerName;

}

void SwitchMixerRequestViaGlobalMemory(LPCWSTR MixerName)
{
	if (!isGlobalMemoryExist())
		return;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	//// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	wcsncpy(gpSharedBlock->SrcName, MixerName, MAX_MODS);
	gpSharedBlock->MixerDeviceStatus = CHANGE_REQ;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);

}

void SwitchMixerAckViaGlobalMemory(LPWSTR MixerName)
{
	if (!isGlobalMemoryExist())
		return;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	//// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	wcsncpy(gpSharedBlock->SrcName, MixerName, MAX_MODS);
	//gpSharedBlock->MixerDeviceStatus = RUNNING;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);
}


void SetSwitchMixerRequestStatToGlobalMemory(enum MDSTAT Stat)
{
	if (!isGlobalMemoryExist())
		return;

	/* Open Mutex and Wait for it */
	ghDataLock = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTEX_LABEL);
	WaitForSingleObject(ghDataLock, INFINITE);
	
	/* Lock access to global memory */
	//// --> ghDataLock = CreateMutex(NULL, TRUE, MUTEX_LABEL);

	gpSharedBlock->MixerDeviceStatus = Stat;

	/* Release acces lock */
	ReleaseMutex(ghDataLock);
	CloseHandle(ghDataLock);
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

	gpSharedBlock = NULL;
	
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

	/* Version 3.3.2 */
	if (!hFileMapping)
	{ 
		SppMessageBoxWithErrorCode();
		ReleaseMutex(ghDataLock);
		CloseHandle(ghDataLock);
		return NULL;
	};

	
    gpSharedBlock = (struct SharedDataBlock *)MapViewOfFile
		(
		hFileMapping,        // File mapping object
		FILE_MAP_ALL_ACCESS, // Read/Write
		0,                  // Offset - high 32 bits
		0,                  // Offset - low 32 bits
		0);                 // Map the whole thing
	
	if (!gpSharedBlock)
	{
		/* Version 3.3.2 */
		SppMessageBoxWithErrorCode();
		ReleaseMutex(ghDataLock);
		CloseHandle(ghDataLock);
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
		gpSharedBlock->i_sel_fltr = -1;
		gpSharedBlock->MixerDeviceStatus = RUNNING;
		wcscpy(gpSharedBlock->SrcName, L"");

	};
	
    ReleaseMutex(ghDataLock);

	return (far void *)gpSharedBlock;
}
