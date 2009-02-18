/** SmartPropoPlus Registry Interface **/
#include <windows.h>
#include <windows.h>
#include "SmartPropoPlus.h"
#include "SppRegistry.h"

#ifndef __SPPREGISTRY
#define __SPPREGISTRY
/*
	Get default value of volume for a given source type
	The source type is parameter SrcType
	The volume value is put in parameter VolumeValue
	Only if the value is valid then the function returns '1'
*/
int GetDefaultVolumeValueFromRegistry(unsigned long SrcType, unsigned long * VolumeValue)
{

	LONG res;
	HKEY hkAud;
	unsigned long  ValueDataSize = MAX_VAL_NAME;
	const char * type;
	int out;

	/* Get the registry entry from source type */
	if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
		type = DEF_VOL_MIC;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_LINE)
		type = DEF_VOL_LIN;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY)
		type = DEF_VOL_AUX;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_ANALOG)
		type = DEF_VOL_ANL;
	else
		type = DEF_VOL_UNK;

	/* Open SPP Audio Sources key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_AUD, 0, KEY_QUERY_VALUE , &hkAud);
	if (res != ERROR_SUCCESS)
		return 0;

	/* Get the default volume value */
	res = RegQueryValueEx(hkAud, type, NULL, NULL, (unsigned char *)(VolumeValue),  &ValueDataSize);
	if (res != ERROR_SUCCESS)
		out =0;
	else
		out =1;

	RegCloseKey(hkAud);
	return out;
}


/*
	Set default value of volume for a given source type 
	The source type is parameter SrcType
	The volume value is parameter VolumeValue
	The function returns '1' if successful
*/
int SetDefaultVolumeValueToRegistry(unsigned long SrcType, unsigned long  VolumeValue)
{

	LONG res;
	HKEY hkAud;
	unsigned long  ValueDataSize = MAX_VAL_NAME;
	const char * type;
	int out;

	/* Get the registry entry from source type */
	if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
		type = DEF_VOL_MIC;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_LINE)
		type = DEF_VOL_LIN;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY)
		type = DEF_VOL_AUX;
	else if (SrcType == MIXERLINE_COMPONENTTYPE_SRC_ANALOG)
		type = DEF_VOL_ANL;
	else
		type = DEF_VOL_UNK;

	/* Open SPP Audio Sources key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_AUD, 0, KEY_ALL_ACCESS , &hkAud);
	if (res != ERROR_SUCCESS)
		return 0;

	/* Set the default volume value */
	res = RegSetValueEx(hkAud, type,0, REG_DWORD, (const unsigned char *)&VolumeValue, 4);
	if (res != ERROR_SUCCESS)
		out =0;
	else
		out =1;

	RegCloseKey(hkAud);
	return out;
}



int GetCurrentAudioStateFromRegistry()
{
	LONG res;
	HKEY hkSpp;
	int Active;
	unsigned long ValueDataSize;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP  key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE , &hkSpp);
	if (res != ERROR_SUCCESS)
		return 0; /* Version 3.3.3 */

	/* Get  data */	
	ValueDataSize = 4;
	res = RegQueryValueEx(hkSpp, AUDIO,  NULL, NULL, (unsigned char *)&Active,  &ValueDataSize);
	if (res != ERROR_SUCCESS)
		return 0; /* Version 3.3.3 */

	RegCloseKey(hkSpp);
	return Active;
}

int GetCurrentPpjoyStateFromRegistry()
{
	LONG res;
	HKEY hkSpp;
	int Active;
	unsigned long ValueDataSize;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP  key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE , &hkSpp);
	if (res != ERROR_SUCCESS)
		return 0;

	/* Get  data */	
	ValueDataSize = 4;
	res = RegQueryValueEx(hkSpp, PPJ_EXT,  NULL, NULL, (unsigned char *)&Active,  &ValueDataSize);
	if (res != ERROR_SUCCESS)
		return 0;

	RegCloseKey(hkSpp);
	return Active;
}


/* Create an empty FMS registry key */
int CreateEmptyFmsRegistry()
{
	LONG res;
	HKEY hkFms;

	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_FMS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkFms, NULL);		
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		return 0;
	};
	RegCloseKey(hkFms);
	return 1;
}


/* Create an empty Audio registry key */
int CreateEmptyAudioRegistry()
{
	LONG res;
	HKEY hkAudio;

	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_AUD, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkAudio, NULL);		
	if (res != ERROR_SUCCESS)
		return 0;
	RegCloseKey(hkAudio);
	return 1;
}


/* Create a default SPP registry key, subkeys and values */
int CreateDefaultSppRegistry()
{
	LONG res;
	HKEY hkSpp;

	int AutoMode = 1;
	int PositiveShift = 1;

	/* Create the SPP key */
	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_SPP, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkSpp, NULL);		
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		return 0;
	};

	/* Insert default shift values */
	res =  RegSetValueEx(hkSpp, SHIFT_POS  ,0, REG_BINARY, (const BYTE* )&PositiveShift, 4);
	res =  RegSetValueEx(hkSpp, SHIFT_AUTO ,0, REG_BINARY, (const BYTE* )&AutoMode, 4);

	CreateDefaultModRegistry();

	RegCloseKey(hkSpp);
	return 1;
}

/* Create a default modulation registry key, subkeys and values */
int CreateDefaultModRegistry()
{
	HKEY  hkMod;
	LONG res;
	const char *DefMods[] = MOD_DEF_STR;
	int nMod = 0;

	/* Create the Modulation-Types key */
	res =  RegCreateKeyEx(HKEY_CURRENT_USER, REG_MOD, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,&hkMod, NULL);		
	if (res != ERROR_SUCCESS)
		return 0;

	/* Insert default modulation values */
	while (DefMods[nMod*2])
	{
		RegSetValueEx(hkMod, DefMods[nMod*2],0, REG_SZ, (const BYTE* )DefMods[nMod*2 +1], 1+(DWORD)strlen(DefMods[nMod*2 +1]));
		nMod++;
	};

	/* Mark the default ACTIVE modulation (PPM) */
	RegSetValueEx(hkMod, MOD_ACTIVE ,0, REG_SZ, (const BYTE* )MOD_TYPE_PPM, 1+(DWORD)strlen(MOD_TYPE_PPM));

	RegCloseKey(hkMod);
	return 1;
}


void SetCurrentAudioStateToRegistry(int Active)
{
	LONG res;
	HKEY hkMixer;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS , &hkMixer);
	if (res != ERROR_SUCCESS)
		return ;

	/* Set Active entry */
	RegSetValueEx(hkMixer, AUDIO,0, REG_BINARY, (const unsigned char *)&Active, 4);
	if (res != ERROR_SUCCESS)
		return;

	RegCloseKey(hkMixer);
}


void SetCurrentPpjoyStateToRegistry(int Active)
{
	LONG res;
	HKEY hkMixer;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS , &hkMixer);
	if (res != ERROR_SUCCESS)
		return ;

	/* Set Active entry */
	RegSetValueEx(hkMixer, PPJ_EXT,0, REG_BINARY, (const unsigned char *)&Active, 4);
	if (res != ERROR_SUCCESS)
		return;

	RegCloseKey(hkMixer);
}



char * GetCurrentMixerDeviceFromRegistry()
{
	LONG res;
	HKEY hkSpp;
	char Active[MAX_VAL_NAME] = "";
	unsigned long ValueDataSize;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP  key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE , &hkSpp);
	if (res != ERROR_SUCCESS)
		return NULL;

	/* Get  data */	
	ValueDataSize = MAX_VAL_NAME;
	res = RegQueryValueEx(hkSpp, MIXER_DEV,  NULL, NULL, (unsigned char *)&(Active[0]),  &ValueDataSize);
	if (res != ERROR_SUCCESS || ValueDataSize<=1)
		return NULL;

	return strdup(Active);
}


/*
	If the correct registry structure exists:
	1. Get the current Mixer Device
	2. Get the Input Line assosoated with this device
*/
int GetCurrentInputLineFromRegistry(unsigned int *SrcID)
{
	LONG res;
	HKEY hkAud;
	unsigned long  ValueDataSize = MAX_VAL_NAME;

	char * mdName = GetCurrentMixerDeviceFromRegistry();
	if (!mdName)
		return 0;


	/* Open SPP Audio Sources key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_AUD, 0, KEY_QUERY_VALUE , &hkAud);
	if (res != ERROR_SUCCESS)
		return 0;

	
	res = RegQueryValueEx(hkAud, mdName, NULL, NULL, (unsigned char *)SrcID,  &ValueDataSize);
	free (mdName);
	if (res != ERROR_SUCCESS)
		return 0;

	RegCloseKey(hkAud);
	return 1;
}



void SetCurrentMixerDeviceToRegistry(const char * MixerName)
{
	LONG res;
	HKEY hkMixer;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS , &hkMixer);
	if (res != ERROR_SUCCESS)
		return ;

	/* Set Active entry */
	RegSetValueEx(hkMixer, MIXER_DEV,0, REG_SZ, (const unsigned char *)MixerName, 1+(DWORD)strlen(MixerName));

	RegCloseKey(hkMixer);
}


void SetCurrentInputLineToRegistry(const char * MixerName, unsigned int SrcID)
{
	LONG res;
	HKEY hkAudio;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	if (!isAudioRegistryExist())
		CreateEmptyAudioRegistry();

	/* Now create an entry with the inputs */
	/* Open SPP key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_AUD, 0, KEY_ALL_ACCESS , &hkAudio);
	if (res != ERROR_SUCCESS)
		return ;

	/* Set Active entry */
	RegSetValueEx(hkAudio, MixerName,0, REG_DWORD, (const unsigned char *)&SrcID, 4);

	RegCloseKey(hkAudio);
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
	RegSetValueEx(hkResult, MOD_ACTIVE,0, REG_SZ, (const BYTE* )DefMods[nMod*2], 1+(DWORD)strlen(DefMods[nMod*2]));

	RegCloseKey(hkResult);
	return nMod;
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


/* Test existence of debug level entry */
int isDebugEntryExist()
{
	LONG res;
	HKEY hSpp;

	if (!isFmsRegistryExist())
		return 0;

	if (!isSppRegistryExist())
		return 0;

	/* Open the SPP key for read */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE, &hSpp);
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		return 0;
	};

	/* Get debug entry */	
	res = RegQueryValueEx(hSpp, DEBUG_LEVEL,  NULL, NULL, NULL,  NULL);
	RegCloseKey(hSpp);

	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		return 0;
	}
	else
		return 1;

}

/*
	Get debug level from SPP registry keys
	First test existence - if does not exist then create with default value (0)
	Then get the debug level
*/
//#define REG_TEST 1
int GetDebugLevelFromRegistry(void)
{
	int res;
	HKEY hSpp;
	int ValueDataSize, DebugLevel=0;
#ifdef REG_TEST
#include <stdio.h>
#include <time.h>
#include <assert.h>
	LPVOID lpMsgBuf;
	char chPath[1000], msg[1000];
	FILE * pfDebug;
	char dbuffer [9], tbuffer [9];
	DWORD pId;

	pId = GetCurrentProcessId();
	sprintf(chPath, "%s\\%s_%d%s", getenv("TEMP"), "SPP_REG", pId,".LOG");
	pfDebug = fopen(chPath, "w");
	if (!pfDebug)
		sprintf(msg,"Cannot open log file %s", chPath);
	else
	{
		sprintf(msg,"Log file %s opened", chPath);
		fprintf(pfDebug,"*******************  Registry Test Utility  *******************\n");
		fprintf(pfDebug, "%s - %s\n\n", _strdate( dbuffer ), _strtime( tbuffer ));
	};
	MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
#endif


	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	if (!isDebugEntryExist())
	{	/* Create default debug entry */
#ifdef REG_TEST
		if (pfDebug)
		{
			fprintf(pfDebug, "1> %s entry was not found\n", DEBUG_LEVEL);
			fflush(pfDebug);
			fprintf(pfDebug, "2> Creating registry key %s\n", DEBUG_LEVEL);
		};
#endif
		res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_WRITE, &hSpp);
		if (res != ERROR_SUCCESS)
#ifdef REG_TEST
		{
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(pfDebug, "3> Cannot Open %s registry key for writing: error number %d  - %s",REG_SPP, GetLastError(), (LPCTSTR)lpMsgBuf);
#endif
			return -1;
#ifdef REG_TEST
		};
#endif
		res =  RegSetValueEx(hSpp, DEBUG_LEVEL  ,0, REG_DWORD, (const BYTE *)&DebugLevel, 4);
		if (res != ERROR_SUCCESS)
#ifdef REG_TEST
		{
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(pfDebug, "4> Cannot create %s registry value: error number %d  - %s",DEBUG_LEVEL, GetLastError(), (LPCTSTR)lpMsgBuf);
#endif
			return -1;
#ifdef REG_TEST
		};
#endif
		RegCloseKey(hSpp);
	}
#ifdef REG_TEST
	else 
	{
		fprintf(pfDebug, "5> Debug level entry was found\n");
		//assert(0);
	};
#endif
	
	/* Open the SPP key for read */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE, &hSpp);
	if (res != ERROR_SUCCESS)
#ifdef REG_TEST
		{
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(pfDebug, "6> Cannot open registry key %s for query: error number %d  - %s",REG_SPP, GetLastError(), (LPCTSTR)lpMsgBuf);
#endif
			return -1;
#ifdef REG_TEST
		};
#endif

	/* Get debug data */
	ValueDataSize = 4;
	res = RegQueryValueEx(hSpp, DEBUG_LEVEL,  NULL, NULL, (unsigned char *)&DebugLevel,  &ValueDataSize);
	RegCloseKey(hSpp);
	if (res != ERROR_SUCCESS)
#ifdef REG_TEST
		{
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf, 0, NULL );
			fprintf(pfDebug, "7> Cannot query registry value %s for query: error number %d  - %s",DEBUG_LEVEL, GetLastError(), (LPCTSTR)lpMsgBuf);
		}
	else
			fprintf(pfDebug, "8> Registry value %s data is : %d\n",DEBUG_LEVEL, DebugLevel);
#endif

#ifdef REG_TEST
		if (pfDebug)
			fclose(pfDebug);
#endif
	if (res != ERROR_SUCCESS)
		return -1;
	
	return DebugLevel;
}

#undef REG_TEST
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
	int iActive = -1, ModUpdated;
	char Active[MAX_VAL_NAME] = "";
	unsigned long nValues, MaxValueNameLength, MaxValueDataLength;
	int ShiftAutoDetect, PositiveShift;

	/* Test Registry - Create default entries if does not exist */
	if (Create && !isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (Create && !isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Test the existence and validity of the list of modulation types */
	ModUpdated = isModRegistryUpdate();
	if (Create && ModUpdated == -1)
		CreateDefaultModRegistry();
	else if (Create && ModUpdated == 0)
		UpdateModRegistry();

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

	Mod = (struct Modulation **)malloc((1+nValues)*sizeof(struct Modulation *));
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

/* Test existence of SPP registry keys */
int isSppRegistryExist()
{
	LONG res;
	HKEY hkResult;

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_READ, &hkResult);
	if (res != ERROR_SUCCESS)
	{
		SetLastError(res);
		return 0;
	};

	RegCloseKey(hkResult);
	return 1;

}

/*	
	Test if modulation registry entries are up to date
	Return:
	1:	Up to date
	0:	Not up to date
	-1:	Does not exist
*/
int isModRegistryUpdate()
{
	LONG res;
	HKEY hkResult;
	const char *DefMods[] = MOD_DEF_STR;
	int iMod;
	unsigned long nValues, MaxValueNameLength, MaxValueDataLength, ValueDataSize;
	char Active[MAX_VAL_NAME] = "";

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_MOD, 0, KEY_QUERY_VALUE, &hkResult);
	if (res != ERROR_SUCCESS)
	{	/* Does not exist */
		RegCloseKey(hkResult);
		return -1;
	};
	res = RegQueryInfoKey(hkResult, NULL, NULL, NULL, NULL, NULL, NULL, &nValues, &MaxValueNameLength, &MaxValueDataLength, NULL, NULL);
	if (res != ERROR_SUCCESS)
	{	/* Cannot open key */
		RegCloseKey(hkResult);
		return -1;
	};

	/* Search for modes in the registry */
	iMod=0;
	ValueDataSize = MaxValueDataLength+1;
	while (DefMods[iMod*2])
	{
		res = RegQueryValueEx(hkResult, DefMods[iMod*2], NULL, NULL, NULL,  NULL);
		if (res != ERROR_SUCCESS)
		{	/* Cannot open key */
			RegCloseKey(hkResult);
			return 0;
		};
		iMod++;
	};

	RegCloseKey(hkResult);
	return 1;
}

/*
	Update the contents of the modulation registry key
	Used to add new entries when upgrading
*/
int UpdateModRegistry()
{
	LONG res;
	HKEY hkResult;
	const char *DefMods[] = MOD_DEF_STR;
	int iMod;
	unsigned long nValues, MaxValueNameLength, MaxValueDataLength, ValueDataSize;
	char Active[MAX_VAL_NAME] = "";

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_MOD, 0, KEY_QUERY_VALUE|KEY_WRITE, &hkResult);
	if (res != ERROR_SUCCESS)
	{	/* Does not exist */
		RegCloseKey(hkResult);
		return -1;
	};
	res = RegQueryInfoKey(hkResult, NULL, NULL, NULL, NULL, NULL, NULL, &nValues, &MaxValueNameLength, &MaxValueDataLength, NULL, NULL);
	if (res != ERROR_SUCCESS)
	{	/* Cannot open key */
		RegCloseKey(hkResult);
		return -1;
	};

	/* Search for modes in the registry - if mode not found then create it */
	iMod=0;
	ValueDataSize = MaxValueDataLength+1;
	while (DefMods[iMod*2])
	{
		res = RegQueryValueEx(hkResult, DefMods[iMod*2], NULL, NULL, NULL,  NULL);
		if (res != ERROR_SUCCESS)
			res = RegSetValueEx(hkResult, DefMods[iMod*2],0, REG_SZ, (const BYTE* )DefMods[iMod*2 +1], 1+(DWORD)strlen(DefMods[iMod*2 +1]));
		iMod++;
	};

	RegCloseKey(hkResult);
	return 1;
}

/* Test existence of Audio registry keys */
int isAudioRegistryExist()
{
	LONG res;
	HKEY hkResult;

	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_AUD, 0, KEY_READ, &hkResult);
	if (res != ERROR_SUCCESS)
		return 0;

	RegCloseKey(hkResult);
	return 1;

}

/* Put the name of the selected filter (JsChPostProc) */
int SetSelectedFilterNameToRegistry(const char * selected)
{
	LONG res;
	HKEY hkResult;

	/* Test if registry key exists */
	if (!isSppRegistryExist())
		return -1;
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_ALL_ACCESS , &hkResult);
	if (res != ERROR_SUCCESS)
		return -1;

	/* Insert default shift values */
	res =  RegSetValueEx(hkResult, SEL_FLTR ,0, REG_SZ, (const unsigned char *)selected, 1+(DWORD)strlen(selected));
	RegCloseKey(hkResult);

	return 0;
}

/* Get the name of the selected filter (JsChPostProc) */
char * GetSelectedFilterNameFromRegistry()
{
	LONG res;
	HKEY hkSpp;
	char Active[MAX_VAL_NAME] = "";
	unsigned long ValueDataSize;

	/* Test Registry - Create default entries if does not exist */
	if (!isFmsRegistryExist())
		CreateEmptyFmsRegistry();

	if (!isSppRegistryExist())
		CreateDefaultSppRegistry();

	/* Open SPP  key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE , &hkSpp);
	if (res != ERROR_SUCCESS)
		return NULL;

	/* Get  data */	
	ValueDataSize = MAX_VAL_NAME;
	res = RegQueryValueEx(hkSpp, SEL_FLTR,  NULL, NULL, (unsigned char *)&(Active[0]),  &ValueDataSize);
	if (res != ERROR_SUCCESS || ValueDataSize<=1)
		return NULL;

	return strdup(Active);
}

#endif // __SPPREGISTRY
