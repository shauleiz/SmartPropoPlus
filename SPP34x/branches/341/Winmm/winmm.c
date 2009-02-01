///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                     WINMM.DLL (for SmartPropoPlus)                                                //
//  Operation:                                                                                                       //
//		This DLL is placed in the same directory that FMS.EXE is located                                             //
//      FMS.EXE loads this DLL instead the original MS namesake DLL                                                  //
//      For most functions, this DLL deligates execution to the original DLL by calling the original function        //
//      The first time FMS.EXE calls an interface function from this DLL, Macro INITWINMM is executed                //
//      This Macro loads the original DLL                                                                            //
//                                                                                                                   //
//  Special interface functions:                                                                                     //
//      joyGetDevCapsA(): For joystick0 it initializes to SPP data.                                                  //
//                        For joystickN (N>0) it executes the original function with parameter (N-1)                 //
//      joyGetNumDevs() : Executes the original function and returns result+1                                        //
//      joyGetPosEx()   : For joystick0 it reads data from memory mapped file where SPP plaeced axes data            //
//                        For joystickN (N>0) it executes the original function with parameter (N-1)                 //
//      joyGetPos()     : For joystick0 it reads data from memory mapped file where SPP plaeced axes data            //
//                        For joystickN (N>0) it executes the original function with parameter (N-1)                 //
//                                                                                                                   //
//                                                                                                                   //
//                                                                                                                   //
//                                                                                                                   //
//                                                                                                                   //
//                                                                                                                   //
//                                                                                                                   //
//                                                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mmsystem.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "SmartPropoPlus.h"
#include "winmm.h"


/***********************************************************************************************/
int WINAPI  DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			TlsIndex = TlsAlloc();
			break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
			WinmmExit();
            break;
    }
			
   return 1;
}

void _PrintChannels(int * ch, int nCh, int stat)
{
	static FILE * pFile;

	if (!pFile)
	{
		pFile = fopen("C:\\Documents and Settings\\Shaul\\Local Settings\\TEMP\\ChannelData.log", "w+");
		if (!pFile) return;
		fprintf(pFile, "Stat\tN\tCh1\t\tCh2\t\tCh3\t\tCh4\t\tCh5\t\tCh6\n");
	};

	fprintf(pFile, "%2d\t%d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\n", stat, nCh, ch[0], ch[1], ch[2], ch[3], ch[4], ch[5]);
}

/* SmartPropoPlus replacement interface functions */
extern __declspec(dllexport) UINT __stdcall     joyGetDevCapsA(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc) 
{
	/* 
		If the GUI console has not been started yet by the simulator then start it
		Assumption - FMS (and any other simulator) will use this function but the GUI will not
		Not assumed - the GUI console is not active yet.*/
	char  jc_name[50] = "Joystick name Not available", *pName;
	int n = 123;

	if (!console_started)
		console_started = StartSppConsole();


	if (uJoyID == 0) {
        pjc->wMid = 0;
        pjc->wPid = 0;
        pjc->wXmin = pjc->wYmin = pjc->wZmin = pjc->wRmin = pjc->wUmin = pjc->wVmin = 0;
        pjc->wXmax = pjc->wYmax = pjc->wZmax = pjc->wRmax = pjc->wUmax = pjc->wVmax = 1023; /* Bug fix by Bruce Page */
        pjc->wNumButtons = pjc->wMaxButtons = 6;
		/* Get number of channels & version string */
		pName = &(jc_name[0]);
		joyGetDataFromSpp(&n, NULL,&pName);
        pjc->wMaxAxes = pjc->wNumAxes = n;
        strcpy((char*)pjc->szPname, jc_name);

        pjc->wPeriodMin = pjc->wPeriodMax = 0;
        pjc->wCaps = JOYCAPS_HASZ | JOYCAPS_HASR | JOYCAPS_HASU | JOYCAPS_HASV;
        pjc->szRegKey[0] = 0;
        pjc->szOEMVxD[0] = 0;
        return JOYERR_NOERROR;
    }

    pjc->wMid = 0;
    pjc->wPid = 0;
    strcpy((char*)pjc->szPname, "");
    pjc->wXmin = pjc->wYmin = pjc->wZmin = pjc->wRmin = pjc->wUmin = pjc->wVmin = 0;
    pjc->wXmax = pjc->wYmax = pjc->wZmax = pjc->wRmax = pjc->wUmax = pjc->wVmax = 0;
    pjc->wNumButtons = pjc->wMaxButtons = 0;
    pjc->wMaxAxes = pjc->wNumAxes = 0;
    pjc->wPeriodMin = pjc->wPeriodMax = 0;
    pjc->wCaps = JOYCAPS_HASV;
    pjc->szRegKey[0] = 0;
    pjc->szOEMVxD[0] = 0;
    //return JOYERR_UNPLUGGED;
	return pjoyGetDevCapsA( uJoyID-1,  pjc,  cbjc);

}

extern __declspec(dllexport) UINT __stdcall     joyGetNumDevs(void)
{
	UINT res;
	res = pjoyGetNumDevs( );
	return 1;
	return res+1;
}


extern __declspec(dllexport) UINT __stdcall     joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
	UINT res;
	int ch_array[MAX_JS_CH], *ch, n, stat;

	if (uJoyID == 0) {
		ch = &(ch_array[0]);
		stat = joyGetDataFromSpp(&n, &ch, NULL);

        pji->dwXpos = ch[3];
        pji->dwYpos = ch[2];
        pji->dwZpos = ch[1];
        pji->dwRpos = ch[0];
        pji->dwUpos = ch[4];
        pji->dwVpos = ch[5];

		pji->dwButtonNumber = 0;
		pji->dwButtons = 0;
		pji->dwPOV = 0;

        res = JOYERR_NOERROR;
	} 
	else
		res =  pjoyGetPosEx(uJoyID-1, pji);
	return res;
}

extern __declspec(dllexport) UINT __stdcall     joyGetPos(UINT uJoyID, LPJOYINFO pji)
{
	UINT res;
	int ch_array[MAX_JS_CH], *ch, n;

	if (uJoyID == 0) {
		ch = &(ch_array[0]);
		joyGetDataFromSpp(&n, &ch, NULL);
						
        pji->wXpos = ch[0];
        pji->wYpos = ch[1];
        pji->wZpos = ch[2];
        res = JOYERR_NOERROR;
	} 
	else
		res = pjoyGetPos(uJoyID-1, pji);
	return res;
}


/* 
	Start the SppConsole GUI application if it exists
*/
int StartSppConsole()
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	int errorcode/*, res*/;
	static UINT NEAR WM_INTERSPPAPPS;	
	char msg[1000], Path[MAX_PATH+20];
	int retry, PathStat;
	
	/* Mark this DLL as running */
	if (!OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXWINMM))
		CreateMutex(NULL, FALSE, MUTXWINMM);

	/* Get the location of the executable */
	PathStat = GetSppConsoleFullPath(Path,MAX_PATH);
	if (PathStat==1)
		sprintf(Path,"%s -i",Path);
	else
		sprintf(Path,"SppConsole.exe -i");


	/* Start the executable */
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	if (!CreateProcess(NULL,Path, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		return 0;
	}
	else
	{
		DWORD rc = WaitForInputIdle(ProcessInformation.hProcess, 10000); // 10 second time out
		if (rc) /* Version 3.4.0 - Problem on slow machines */
		{ /* 3.4.0 - Give a second chance */
			sprintf(msg, "Failed to connect to SmartPropoPlus Console (Error %x)\nDo you want to retry?", rc);
			retry = MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL|MB_YESNO);
			if (!retry)
				return 0;
		};
	};
		
	/* Send starting message to the GUI */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "StartSppConsole(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};
	PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTARTING, 0);
	return 1;
}


//---------------------------------------------------------------------------

void WinmmExit(void)
{
	static UINT NEAR WM_INTERSPPAPPS;
	static count;
	char  msg[1000];

	/* Send closing message to the GUI */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);

	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "WinmmExit(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};

	if (console_started)
	{
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTOPPING, 0);
		console_started = 0;
	};}


/*
	Get data from SPP file mapping
	nPos:	Pointer to 'number of positions (=channels)
	Pos:	Pointer to first element in array of positions.
	Return 
		TRUE if data is valid
		FALSE if data in nPos/Pos is invelid
	Note:
		Invalid data serves for debug purpose
*/
int joyGetDataFromSpp(int * nPos, int ** Pos, char ** VersionStr)
{
	static HANDLE hMutexSharedFile=NULL, hFileMapping=NULL, hMutexConsole=NULL;
	DWORD Cont, ConsoleState;
	static struct Spp2FmsStruct * data;
	int i;
	static int  TimeoutCounter, PostedId;
	static UINT NEAR WM_INTERSPPAPPS;

	if (data)
	{
		*nPos=data->nChannels; // Number of channels
		if ( *nPos>MAX_JS_CH)
			*nPos=MAX_JS_CH;
		if (Pos)				// Channels' data
			for (i=0 ; i<6 ; i++)
				(*Pos)[i] = data->Channel[i];
//		return 3;
	};

	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);

	/* 
		Check if SPP console is running
		if not return false, reset all handles and mutexes and 'No GUI' Position pattern 
	*/
	if (hMutexConsole)
	{
		ConsoleState = WaitForSingleObject(hMutexConsole,0);
		if (ConsoleState != WAIT_TIMEOUT)
		{
			if (hMutexConsole) ResetHandle(&hMutexConsole);
			if (hMutexSharedFile) ResetHandle(&hMutexSharedFile);
			if (data) ResetViewOfFile(&data);
			if (hFileMapping) ResetHandle(&hFileMapping);
			return -1;
		}
	} else
	{
		hMutexConsole = OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXCONSOLE);
		if (!hMutexConsole)
		{
			CreatePatternNoGui(nPos, Pos);
			if (VersionStr)
			{
				if (data) ResetViewOfFile(&data);
				if (data)
					strncpy(*VersionStr, (data->VersionStr), MAX_VERSTR);
				else
					strncpy(*VersionStr, "Cannot connect to SppConsole", MAX_VERSTR);
			};
			return -2;
		} else
		{
			/* Send starting message to the GUI - this will make 'exit' button grey */
			console_started = 1;
			PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTARTING, 0);
		};
	} 


	/* 
		Check if SPP console has initiated the file mapping
		if not send it a 'start file mapping' message
		return false with 'No file mapping' Position pattern 
		*/
	if (!data || !hFileMapping || !hMutexSharedFile)
	{
		hMutexSharedFile = OpenMutex(MUTEX_ALL_ACCESS, TRUE, SHARED_FILE_MUTEX);
		hFileMapping = OpenFileMapping(PAGE_READONLY, FALSE, SHARED_FILE_NAME);
		if (hFileMapping)
			data = (struct Spp2FmsStruct *)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!data || !hFileMapping  || !hMutexSharedFile)
		{
			if (hMutexSharedFile) ResetHandle(&hMutexSharedFile);
			if (data) ResetViewOfFile(&data);
			if (hFileMapping) ResetHandle(&hFileMapping);
			CreatePatternNoSharedFile(nPos, Pos);
			return -3;
		};
	}


	/* 
		Wait for file mapping mutex to be signaled
		When signaled lock file mapping memory and read it, reset timeout counter and return true.
		If timeout then raise the timeout counter
		If many consecutive timeout then return false with 'very long wait'  Position pattern.
		Otherwise return true with previous Position data
	*/
	Cont = WaitForSingleObject(hMutexSharedFile,50);
	if (Cont == WAIT_OBJECT_0) // Mutex Signaled, free to read the data
	{
		*nPos=data->nChannels; // Number of channels
		if ( *nPos>MAX_JS_CH)
			*nPos=MAX_JS_CH;
		if (Pos)				// Channels' data
			for (i=0 ; i<6 ; i++)
				(*Pos)[i] = data->Channel[i];
		if (VersionStr)
			strncpy(*VersionStr, (data->VersionStr), MAX_VERSTR);
		ReleaseMutex(hMutexSharedFile);
		TimeoutCounter=0;
		//Beep(800,20);
		if (data->Identifier - PostedId > 180)
		{
			PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLTIMESTAMP, data->Identifier); // Sending timestamp back to GUI
			PostedId = data->Identifier;
		};
		return 1;
	} 
	else if (Cont == WAIT_TIMEOUT) // Mutex not Signaled, timeout expired
	{
		TimeoutCounter++;
		if (TimeoutCounter == VERYLONGWAIT)
		{
			CreatePatternVeryLongWait(nPos, Pos);
			return -4;
		}
		else
		{
			return 2;
		};
	} else
	{
		data = NULL;
		return -5;
	};

	return -6;
}

BOOL LoadWinmm(int line)
{

	char msg[4000];


    if (!GetSystemDirectory(path, MAX_PATH)) 
	{
		if (path)
			sprintf(msg, "LoadWinmm(%d): Cannot find path for file (NULL)", line);
		else
			sprintf(msg, "LoadWinmm(%d): Cannot find path for file %s", line, path);
		MESSAGE(msg, MB_ICONERROR);
		return FALSE;
	};
    strcat(path, "\\winmm.dll");
    hWinmm = LoadOrigDll(path, line);
    if (!hWinmm) 
		return FALSE;

	GetPointerToOriginalFunc();

	return TRUE;
}

/****************** Helper functions ******************/

/* FMS Blue-bar status patterns                        */
/*
	Debug channel pattern for FMS
	Pattern: Alternate Low/High for 8 channels
*/
void CreatePatternNoGui(int * nPos, int ** Pos)
{
	static int n=8, ch[8] = {100,900,100,900,900,100,100,900};
	*nPos = n;
	if (Pos)
		*Pos = ch;
}

/*
	Debug channel pattern for FMS
	Pattern: 
		channels moving simultaneously  (8 channels)
		Moving slowly up and then abruptly dropping down

*/
void CreatePatternNoSharedFile(int * nPos, int ** Pos)
{
	static int n=8, ch[8] = {900,900,900,900,900,900,900,900};
	int value;

	*nPos = n;
	*Pos = ch;
	value = ch[0];
	if (value < 100) value = 900;
	else value = value-10;
	ch[0] = ch[1] = ch[2] = ch[3] = ch[4] = ch[5] = ch[6] = ch[7] = value;
}

/*
	Debug channel pattern for FMS
	Pattern: Channels form a slope going down (L to R)
*/
void CreatePatternVeryLongWait(int * nPos, int ** Pos)
{
	static int n=8, ch[8] = {400,300,200,100,500,600,700,800};
	*nPos = n;
	*Pos = ch;
}


/** Error Message dialog box **/
int Message(const char * in_msg, UINT type ,const char * file_name,  int line_number)
{
	int out, errNum;
	LPVOID lpMsgBuf;
	char msg[4000];
	
	errNum = GetLastError();
	if (errNum)
	{
		FormatMessage( 	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,errNum,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,0,NULL );
		sprintf(msg, "%s\nError: %d %s\n\r\rFile: %s(Line %d)", in_msg, errNum, lpMsgBuf, file_name, line_number);
	}
	else
		sprintf(msg, "%s\n\r\rFile: %s(Line %d)", in_msg, file_name, line_number);

	
	out = MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL | type);
	return out;
}

/* Improved DLL loader */
HINSTANCE LoadOrigDll(const char * dll, int line)
{
	HINSTANCE h;
	BOOL attribsOK;
	WIN32_FILE_ATTRIBUTE_DATA Attribs;
	char msg[4000];
	int res;
	
	if (!dll)
	{
		MESSAGE("LoadOrigDll(): File to load is (NULL)", MB_ICONERROR);
		return NULL;
	};

	attribsOK = GetFileAttributesEx(dll, GetFileExInfoStandard, &Attribs);
	if (!attribsOK)
	{
		sprintf(msg, "LoadOrigDll(): Cannot get attributes for File %s", dll);
		MESSAGE(msg, MB_ICONERROR);
	};

	h = LoadLibrary(dll);
	while (!h)
	{
		h = LoadLibraryEx(dll, NULL, 0);
		if (!h)
		{

			sprintf(msg, "LoadOrigDll(%d):\n\rCannot load File %s, Retry?", line, dll);
			res = MESSAGE(msg, MB_YESNO | MB_ICONQUESTION );
			if (res != IDYES)
				break;
		};
	};

	return h;
}

_inline BOOL ResetHandle(HANDLE * ph)
{
	BOOL out;

	out = CloseHandle(*ph);
	*(ph) = NULL;
	return out;
}

_inline BOOL ResetViewOfFile(LPCVOID * pv)
{
	BOOL out;

	out = UnmapViewOfFile(*pv);
	*(pv)=NULL;
	return out;
}




void GetPointerToOriginalFunc(void)
{
    pCloseDriver = GetProcAddress(hWinmm, "CloseDriver");
    pDefDriverProc = GetProcAddress(hWinmm, "DefDriverProc");
    pDriverCallback = GetProcAddress(hWinmm, "DriverCallback");
    pDrvGetModuleHandle = GetProcAddress(hWinmm, "DrvGetModuleHandle");
    pGetDriverModuleHandle = GetProcAddress(hWinmm, "GetDriverModuleHandle");
    pMigrateAllDrivers = GetProcAddress(hWinmm, "MigrateAllDrivers");
//    pMigrateMidiUser = GetProcAddress(hWinmm, "MigrateMidiUser");
    pMigrateSoundEvents = GetProcAddress(hWinmm, "MigrateSoundEvents");
    pNotifyCallbackData = GetProcAddress(hWinmm, "NotifyCallbackData");
    pOpenDriver = GetProcAddress(hWinmm, "OpenDriver");
    pPlaySound = GetProcAddress(hWinmm, "PlaySound");
    pPlaySoundA = GetProcAddress(hWinmm, "PlaySoundA");
    pPlaySoundW = GetProcAddress(hWinmm, "PlaySoundW");
    pSendDriverMessage = GetProcAddress(hWinmm, "SendDriverMessage");
    pWOW32DriverCallback = GetProcAddress(hWinmm, "WOW32DriverCallback");
    pWOW32ResolveMultiMediaHandle = GetProcAddress(hWinmm, "WOW32ResolveMultiMediaHandle");
    pWOWAppExit = GetProcAddress(hWinmm, "WOWAppExit");
    pWinmmLogoff = GetProcAddress(hWinmm, "WinmmLogoff");
    pWinmmLogon = GetProcAddress(hWinmm, "WinmmLogon");
    paux32Message = GetProcAddress(hWinmm, "aux32Message");
    pauxGetDevCapsA = GetProcAddress(hWinmm, "auxGetDevCapsA");
    pauxGetDevCapsW = GetProcAddress(hWinmm, "auxGetDevCapsW");
    pauxGetNumDevs = GetProcAddress(hWinmm, "auxGetNumDevs");
    pauxGetVolume = GetProcAddress(hWinmm, "auxGetVolume");
    pauxOutMessage = GetProcAddress(hWinmm, "auxOutMessage");
    pauxSetVolume = GetProcAddress(hWinmm, "auxSetVolume");
    pjoy32Message = GetProcAddress(hWinmm, "joy32Message");
    pjoyConfigChanged = GetProcAddress(hWinmm, "joyConfigChanged");
    pjoyGetDevCapsA = GetProcAddress(hWinmm, "joyGetDevCapsA");
    pjoyGetDevCapsW = GetProcAddress(hWinmm, "joyGetDevCapsW");
    pjoyGetNumDevs = GetProcAddress(hWinmm, "joyGetNumDevs");
//    pjoyGetPos = GetProcAddress(hWinmm, "joyGetPos");
    pjoyGetThreshold = GetProcAddress(hWinmm, "joyGetThreshold");
    pjoyReleaseCapture = GetProcAddress(hWinmm, "joyReleaseCapture");
    pjoySetCapture = GetProcAddress(hWinmm, "joySetCapture");
    pjoySetThreshold = GetProcAddress(hWinmm, "joySetThreshold");
    pmci32Message = GetProcAddress(hWinmm, "mci32Message");
    pmciDriverNotify = GetProcAddress(hWinmm, "mciDriverNotify");
    pmciDriverYield = GetProcAddress(hWinmm, "mciDriverYield");
    pmciExecute = GetProcAddress(hWinmm, "mciExecute");
    pmciFreeCommandResource = GetProcAddress(hWinmm, "mciFreeCommandResource");
    pmciGetCreatorTask = GetProcAddress(hWinmm, "mciGetCreatorTask");
    pmciGetDeviceIDA = GetProcAddress(hWinmm, "mciGetDeviceIDA");
    pmciGetDeviceIDFromElementIDA = GetProcAddress(hWinmm, "mciGetDeviceIDFromElementIDA");
    pmciGetDeviceIDFromElementIDW = GetProcAddress(hWinmm, "mciGetDeviceIDFromElementIDW");
    pmciGetDeviceIDW = GetProcAddress(hWinmm, "mciGetDeviceIDW");
    pmciGetDriverData = GetProcAddress(hWinmm, "mciGetDriverData");
    pmciGetErrorStringA = GetProcAddress(hWinmm, "mciGetErrorStringA");
    pmciGetErrorStringW = GetProcAddress(hWinmm, "mciGetErrorStringW");
    pmciGetYieldProc = GetProcAddress(hWinmm, "mciGetYieldProc");
    pmciLoadCommandResource = GetProcAddress(hWinmm, "mciLoadCommandResource");
    pmciSendCommandA = GetProcAddress(hWinmm, "mciSendCommandA");
    pmciSendCommandW = GetProcAddress(hWinmm, "mciSendCommandW");
    pmciSendStringA = GetProcAddress(hWinmm, "mciSendStringA");
    pmciSendStringW = GetProcAddress(hWinmm, "mciSendStringW");
    pmciSetDriverData = GetProcAddress(hWinmm, "mciSetDriverData");
    pmciSetYieldProc = GetProcAddress(hWinmm, "mciSetYieldProc");
    pmid32Message = GetProcAddress(hWinmm, "mid32Message");
    pmidiConnect = GetProcAddress(hWinmm, "midiConnect");
    pmidiDisconnect = GetProcAddress(hWinmm, "midiDisconnect");
    pmidiInAddBuffer = GetProcAddress(hWinmm, "midiInAddBuffer");
    pmidiInClose = GetProcAddress(hWinmm, "midiInClose");
    pmidiInGetDevCapsA = GetProcAddress(hWinmm, "midiInGetDevCapsA");
    pmidiInGetDevCapsW = GetProcAddress(hWinmm, "midiInGetDevCapsW");
    pmidiInGetErrorTextA = GetProcAddress(hWinmm, "midiInGetErrorTextA");
    pmidiInGetErrorTextW = GetProcAddress(hWinmm, "midiInGetErrorTextW");
    pmidiInGetID = GetProcAddress(hWinmm, "midiInGetID");
    pmidiInGetNumDevs = GetProcAddress(hWinmm, "midiInGetNumDevs");
    pmidiInMessage = GetProcAddress(hWinmm, "midiInMessage");
    pmidiInOpen = GetProcAddress(hWinmm, "midiInOpen");
    pmidiInPrepareHeader = GetProcAddress(hWinmm, "midiInPrepareHeader");
    pmidiInReset = GetProcAddress(hWinmm, "midiInReset");
    pmidiInStart = GetProcAddress(hWinmm, "midiInStart");
    pmidiInStop = GetProcAddress(hWinmm, "midiInStop");
    pmidiInUnprepareHeader = GetProcAddress(hWinmm, "midiInUnprepareHeader");
    pmidiOutCacheDrumPatches = GetProcAddress(hWinmm, "midiOutCacheDrumPatches");
    pmidiOutCachePatches = GetProcAddress(hWinmm, "midiOutCachePatches");
    pmidiOutClose = GetProcAddress(hWinmm, "midiOutClose");
    pmidiOutGetDevCapsA = GetProcAddress(hWinmm, "midiOutGetDevCapsA");
    pmidiOutGetDevCapsW = GetProcAddress(hWinmm, "midiOutGetDevCapsW");
    pmidiOutGetErrorTextA = GetProcAddress(hWinmm, "midiOutGetErrorTextA");
    pmidiOutGetErrorTextW = GetProcAddress(hWinmm, "midiOutGetErrorTextW");
    pmidiOutGetID = GetProcAddress(hWinmm, "midiOutGetID");
    pmidiOutGetNumDevs = GetProcAddress(hWinmm, "midiOutGetNumDevs");
    pmidiOutGetVolume = GetProcAddress(hWinmm, "midiOutGetVolume");
    pmidiOutLongMsg = GetProcAddress(hWinmm, "midiOutLongMsg");
    pmidiOutMessage = GetProcAddress(hWinmm, "midiOutMessage");
    pmidiOutOpen = GetProcAddress(hWinmm, "midiOutOpen");
    pmidiOutPrepareHeader = GetProcAddress(hWinmm, "midiOutPrepareHeader");
    pmidiOutReset = GetProcAddress(hWinmm, "midiOutReset");
    pmidiOutSetVolume = GetProcAddress(hWinmm, "midiOutSetVolume");
    pmidiOutShortMsg = GetProcAddress(hWinmm, "midiOutShortMsg");
    pmidiOutUnprepareHeader = GetProcAddress(hWinmm, "midiOutUnprepareHeader");
    pmidiStreamClose = GetProcAddress(hWinmm, "midiStreamClose");
    pmidiStreamOpen = GetProcAddress(hWinmm, "midiStreamOpen");
    pmidiStreamOut = GetProcAddress(hWinmm, "midiStreamOut");
    pmidiStreamPause = GetProcAddress(hWinmm, "midiStreamPause");
    pmidiStreamPosition = GetProcAddress(hWinmm, "midiStreamPosition");
    pmidiStreamProperty = GetProcAddress(hWinmm, "midiStreamProperty");
    pmidiStreamRestart = GetProcAddress(hWinmm, "midiStreamRestart");
    pmidiStreamStop = GetProcAddress(hWinmm, "midiStreamStop");
    pmixerClose = GetProcAddress(hWinmm, "mixerClose");
    pmixerGetControlDetailsA = GetProcAddress(hWinmm, "mixerGetControlDetailsA");
    pmixerGetControlDetailsW = GetProcAddress(hWinmm, "mixerGetControlDetailsW");
    pmixerGetDevCapsA = GetProcAddress(hWinmm, "mixerGetDevCapsA");
    pmixerGetDevCapsW = GetProcAddress(hWinmm, "mixerGetDevCapsW");
    pmixerGetID = GetProcAddress(hWinmm, "mixerGetID");
    pmixerGetLineControlsA = GetProcAddress(hWinmm, "mixerGetLineControlsA");
    pmixerGetLineControlsW = GetProcAddress(hWinmm, "mixerGetLineControlsW");
    pmixerGetLineInfoA = GetProcAddress(hWinmm, "mixerGetLineInfoA");
    pmixerGetLineInfoW = GetProcAddress(hWinmm, "mixerGetLineInfoW");
    pmixerGetNumDevs = GetProcAddress(hWinmm, "mixerGetNumDevs");
    pmixerMessage = GetProcAddress(hWinmm, "mixerMessage");
    pmixerOpen = GetProcAddress(hWinmm, "mixerOpen");
    pmixerSetControlDetails = GetProcAddress(hWinmm, "mixerSetControlDetails");
    pmmDrvInstall = GetProcAddress(hWinmm, "mmDrvInstall");
    pmmGetCurrentTask = GetProcAddress(hWinmm, "mmGetCurrentTask");
    pmmTaskBlock = GetProcAddress(hWinmm, "mmTaskBlock");
    pmmTaskCreate = GetProcAddress(hWinmm, "mmTaskCreate");
    pmmTaskSignal = GetProcAddress(hWinmm, "mmTaskSignal");
    pmmTaskYield = GetProcAddress(hWinmm, "mmTaskYield");
    pmmioAdvance = GetProcAddress(hWinmm, "mmioAdvance");
    pmmioAscend = GetProcAddress(hWinmm, "mmioAscend");
    pmmioClose = GetProcAddress(hWinmm, "mmioClose");
    pmmioCreateChunk = GetProcAddress(hWinmm, "mmioCreateChunk");
    pmmioDescend = GetProcAddress(hWinmm, "mmioDescend");
    pmmioFlush = GetProcAddress(hWinmm, "mmioFlush");
    pmmioGetInfo = GetProcAddress(hWinmm, "mmioGetInfo");
    pmmioInstallIOProcA = GetProcAddress(hWinmm, "mmioInstallIOProcA");
    pmmioInstallIOProcW = GetProcAddress(hWinmm, "mmioInstallIOProcW");
    pmmioOpenA = GetProcAddress(hWinmm, "mmioOpenA");
    pmmioOpenW = GetProcAddress(hWinmm, "mmioOpenW");
    pmmioRead = GetProcAddress(hWinmm, "mmioRead");
    pmmioRenameA = GetProcAddress(hWinmm, "mmioRenameA");
    pmmioRenameW = GetProcAddress(hWinmm, "mmioRenameW");
    pmmioSeek = GetProcAddress(hWinmm, "mmioSeek");
    pmmioSendMessage = GetProcAddress(hWinmm, "mmioSendMessage");
    pmmioSetBuffer = GetProcAddress(hWinmm, "mmioSetBuffer");
    pmmioSetInfo = GetProcAddress(hWinmm, "mmioSetInfo");
    pmmioStringToFOURCCA = GetProcAddress(hWinmm, "mmioStringToFOURCCA");
    pmmioStringToFOURCCW = GetProcAddress(hWinmm, "mmioStringToFOURCCW");
    pmmioWrite = GetProcAddress(hWinmm, "mmioWrite");
    pmmsystemGetVersion = GetProcAddress(hWinmm, "mmsystemGetVersion");
    pmod32Message = GetProcAddress(hWinmm, "mod32Message");
    pmxd32Message = GetProcAddress(hWinmm, "mxd32Message");
    psndPlaySoundA = GetProcAddress(hWinmm, "sndPlaySoundA");
    psndPlaySoundW = GetProcAddress(hWinmm, "sndPlaySoundW");
    ptid32Message = GetProcAddress(hWinmm, "tid32Message");
    ptimeBeginPeriod = GetProcAddress(hWinmm, "timeBeginPeriod");
    ptimeEndPeriod = GetProcAddress(hWinmm, "timeEndPeriod");
    ptimeGetDevCaps = GetProcAddress(hWinmm, "timeGetDevCaps");
    ptimeGetSystemTime = GetProcAddress(hWinmm, "timeGetSystemTime");
    ptimeGetTime = GetProcAddress(hWinmm, "timeGetTime");
    ptimeKillEvent = GetProcAddress(hWinmm, "timeKillEvent");
    ptimeSetEvent = GetProcAddress(hWinmm, "timeSetEvent");
    pwaveInGetDevCapsA = GetProcAddress(hWinmm, "waveInGetDevCapsA");
    pwaveInGetDevCapsW = GetProcAddress(hWinmm, "waveInGetDevCapsW");
    pwaveInGetErrorTextA = GetProcAddress(hWinmm, "waveInGetErrorTextA");
    pwaveInGetErrorTextW = GetProcAddress(hWinmm, "waveInGetErrorTextW");
    pwaveInGetID = GetProcAddress(hWinmm, "waveInGetID");
    pwaveInGetNumDevs = GetProcAddress(hWinmm, "waveInGetNumDevs");
    pwaveInGetPosition = GetProcAddress(hWinmm, "waveInGetPosition");
    pwaveInMessage = GetProcAddress(hWinmm, "waveInMessage");
    pwaveInReset = GetProcAddress(hWinmm, "waveInReset");
    pwaveOutBreakLoop = GetProcAddress(hWinmm, "waveOutBreakLoop");
    pwaveOutClose = GetProcAddress(hWinmm, "waveOutClose");
    pwaveOutGetDevCapsA = GetProcAddress(hWinmm, "waveOutGetDevCapsA");
    pwaveOutGetDevCapsW = GetProcAddress(hWinmm, "waveOutGetDevCapsW");
    pwaveOutGetErrorTextA = GetProcAddress(hWinmm, "waveOutGetErrorTextA");
    pwaveOutGetErrorTextW = GetProcAddress(hWinmm, "waveOutGetErrorTextW");
    pwaveOutGetID = GetProcAddress(hWinmm, "waveOutGetID");
    pwaveOutGetNumDevs = GetProcAddress(hWinmm, "waveOutGetNumDevs");
    pwaveOutGetPitch = GetProcAddress(hWinmm, "waveOutGetPitch");
    pwaveOutGetPlaybackRate = GetProcAddress(hWinmm, "waveOutGetPlaybackRate");
    pwaveOutGetPosition = GetProcAddress(hWinmm, "waveOutGetPosition");
    pwaveOutGetVolume = GetProcAddress(hWinmm, "waveOutGetVolume");
    pwaveOutMessage = GetProcAddress(hWinmm, "waveOutMessage");
    pwaveOutOpen = GetProcAddress(hWinmm, "waveOutOpen");
    pwaveOutPause = GetProcAddress(hWinmm, "waveOutPause");
    pwaveOutPrepareHeader = GetProcAddress(hWinmm, "waveOutPrepareHeader");
    pwaveOutReset = GetProcAddress(hWinmm, "waveOutReset");
    pwaveOutRestart = GetProcAddress(hWinmm, "waveOutRestart");
    pwaveOutSetPitch = GetProcAddress(hWinmm, "waveOutSetPitch");
    pwaveOutSetPlaybackRate = GetProcAddress(hWinmm, "waveOutSetPlaybackRate");
    pwaveOutSetVolume = GetProcAddress(hWinmm, "waveOutSetVolume");
    pwaveOutUnprepareHeader = GetProcAddress(hWinmm, "waveOutUnprepareHeader");
    pwaveOutWrite = GetProcAddress(hWinmm, "waveOutWrite");
    pwid32Message = GetProcAddress(hWinmm, "wid32Message");
    pwinmmDbgOut = GetProcAddress(hWinmm, "winmmDbgOut");
    pwinmmSetDebugLevel = GetProcAddress(hWinmm, "winmmSetDebugLevel");
    pwod32Message = GetProcAddress(hWinmm, "wod32Message");

    pgfxAddGfx = GetProcAddress(hWinmm, "gfxAddGfx");
    pgfxBatchChange = GetProcAddress(hWinmm, "gfxBatchChange");
    pgfxCreateGfxFactoriesList = GetProcAddress(hWinmm, "gfxCreateGfxFactoriesList");
    pgfxCreateZoneFactoriesList = GetProcAddress(hWinmm, "gfxCreateZoneFactoriesList");
    pgfxDestroyDeviceInterfaceList = GetProcAddress(hWinmm, "gfxDestroyDeviceInterfaceList");
    pgfxEnumerateGfxs = GetProcAddress(hWinmm, "gfxEnumerateGfxs");
    p_gfxLogoff = GetProcAddress(hWinmm, "_gfxLogoff@0");
    p_gfxLogon = GetProcAddress(hWinmm, "_gfxLogon@4");
    pgfxModifyGfx = GetProcAddress(hWinmm, "gfxModifyGfx");
    pgfxOpenGfx = GetProcAddress(hWinmm, "gfxOpenGfx");
    pgfxRemoveGfx = GetProcAddress(hWinmm, "gfxRemoveGfx");


    pwaveInAddBuffer =  (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddress(hWinmm, "waveInAddBuffer");
    pwaveInClose = (UINT ( WINAPI *)(HWAVEIN))GetProcAddress(hWinmm, "waveInClose");
    pwaveInOpen = (UINT (WINAPI*)(LPHWAVIN, UINT, LPCWAVEFORMATEX, DWORD, DWORD, DWORD))GetProcAddress(hWinmm, "waveInOpen");
    pwaveInPrepareHeader = (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddress(hWinmm, "waveInPrepareHeader");
    pwaveInStart = (UINT ( WINAPI *)(HWAVEIN))GetProcAddress(hWinmm, "waveInStart");
    pwaveInStop = (UINT ( WINAPI *)(HWAVEIN))GetProcAddress(hWinmm, "waveInStop");
    pwaveInUnprepareHeader = (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddress(hWinmm, "waveInUnprepareHeader");
	pjoyGetPosEx = (UINT ( WINAPI *)(UINT uJoyID, LPJOYINFOEX pji))GetProcAddress(hWinmm, "joyGetPosEx");
	pjoyGetPos = (UINT ( WINAPI *)(UINT uJoyID, LPJOYINFO pji))GetProcAddress(hWinmm, "joyGetPos");
}


/* 
	The following functions are interface functions of the DLL that are implemented
	as a call to the corresponding functions in the original DLL file

	The call is a "naked" call to an assembly code that jumps 
	to the location of the original function

	#define NAKED __declspec(naked)
*/
void NAKED joyConfigChanged(void) {  INITWINMM __asm jmp dword ptr pjoyConfigChanged  }
void NAKED joyGetThreshold(void)   {   INITWINMM __asm  jmp dword ptr   pjoyGetThreshold}
void NAKED joyReleaseCapture(void) {   INITWINMM __asm jmp dword ptr   pjoyReleaseCapture}
void NAKED joySetCapture(void) {   INITWINMM __asm jmp dword ptr   pjoySetCapture}
void NAKED joySetThreshold(void) {   INITWINMM __asm jmp dword ptr   pjoySetThreshold}
void NAKED mci32Message(void) {   INITWINMM __asm jmp dword ptr   pmci32Message}
void NAKED mciDriverNotify(void) {   INITWINMM __asm jmp dword ptr   pmciDriverNotify}
void NAKED mciDriverYield(void) {   INITWINMM __asm jmp dword ptr   pmciDriverYield}
void NAKED mciExecute(void) {   INITWINMM __asm jmp dword ptr   pmciExecute}
void NAKED mciFreeCommandResource(void) {   INITWINMM __asm jmp dword ptr   pmciFreeCommandResource}
void NAKED mciGetCreatorTask(void) {   INITWINMM __asm jmp dword ptr   pmciGetCreatorTask}
void NAKED mciGetDeviceIDA(void) {   INITWINMM __asm jmp dword ptr   pmciGetDeviceIDA}
void NAKED mciGetDeviceIDFromElementIDA(void) {   INITWINMM __asm jmp dword ptr   pmciGetDeviceIDFromElementIDA}
void NAKED mciGetDeviceIDFromElementIDW(void) {   INITWINMM __asm jmp dword ptr   pmciGetDeviceIDFromElementIDW}
void NAKED mciGetDeviceIDW(void) {   INITWINMM __asm jmp dword ptr   pmciGetDeviceIDW}
void NAKED mciGetDriverData(void) {   INITWINMM __asm jmp dword ptr   pmciGetDriverData}
void NAKED mciGetErrorStringA(void) {   INITWINMM __asm jmp dword ptr   pmciGetErrorStringA}
void NAKED mciGetErrorStringW(void) {   INITWINMM __asm jmp dword ptr   pmciGetErrorStringW}
void NAKED mciGetYieldProc(void) {   INITWINMM __asm jmp dword ptr   pmciGetYieldProc}
void NAKED mciLoadCommandResource(void) {   INITWINMM __asm jmp dword ptr   pmciLoadCommandResource}
void NAKED mciSendCommandA(void) {   INITWINMM __asm jmp dword ptr   pmciSendCommandA}
void NAKED mciSendCommandW(void) {   INITWINMM __asm jmp dword ptr   pmciSendCommandW}
void NAKED mciSendStringA(void) {   INITWINMM __asm jmp dword ptr   pmciSendStringA}
void NAKED mciSendStringW(void) {   INITWINMM __asm jmp dword ptr   pmciSendStringW}
void NAKED mciSetDriverData(void) {   INITWINMM __asm jmp dword ptr   pmciSetDriverData}
void NAKED mciSetYieldProc(void) {   INITWINMM __asm jmp dword ptr   pmciSetYieldProc}
void NAKED mid32Message(void) {   INITWINMM __asm jmp dword ptr   pmid32Message}
void NAKED midiConnect(void) {   INITWINMM __asm jmp dword ptr   pmidiConnect}
void NAKED midiDisconnect(void) {   INITWINMM __asm jmp dword ptr   pmidiDisconnect}
void NAKED midiInAddBuffer(void) {   INITWINMM __asm jmp dword ptr   pmidiInAddBuffer}
void NAKED midiInClose(void) {  INITWINMM __asm jmp dword ptr   pmidiInClose}
void NAKED midiInGetDevCapsA(void) {   INITWINMM __asm jmp dword ptr   pmidiInGetDevCapsA}
void NAKED midiInGetDevCapsW(void) {   INITWINMM __asm jmp dword ptr   pmidiInGetDevCapsW}
void NAKED midiInGetErrorTextA(void) {   INITWINMM __asm jmp dword ptr   pmidiInGetErrorTextA}
void NAKED midiInGetErrorTextW(void) {   INITWINMM __asm jmp dword ptr   pmidiInGetErrorTextW}
void NAKED midiInGetID(void) {   INITWINMM __asm jmp dword ptr   pmidiInGetID}
void NAKED midiInGetNumDevs(void) {   INITWINMM __asm jmp dword ptr   pmidiInGetNumDevs}
void NAKED midiInMessage(void) {   INITWINMM __asm jmp dword ptr   pmidiInMessage}
void NAKED midiInOpen(void) {   INITWINMM __asm jmp dword ptr   pmidiInOpen}
void NAKED midiInPrepareHeader(void) {   INITWINMM __asm jmp dword ptr   pmidiInPrepareHeader}
void NAKED midiInReset(void) {   INITWINMM __asm jmp dword ptr   pmidiInReset}
void NAKED midiInStart(void) {   INITWINMM __asm jmp dword ptr   pmidiInStart}
void NAKED midiInStop(void) {   INITWINMM __asm jmp dword ptr   pmidiInStop}
void NAKED midiInUnprepareHeader(void) {   INITWINMM __asm jmp dword ptr   pmidiInUnprepareHeader}
void NAKED midiOutCacheDrumPatches(void) {   INITWINMM __asm jmp dword ptr   pmidiOutCacheDrumPatches}
void NAKED midiOutCachePatches(void) {   INITWINMM __asm jmp dword ptr   pmidiOutCachePatches}
void NAKED midiOutClose(void) {   INITWINMM __asm jmp dword ptr   pmidiOutClose}
void NAKED midiOutGetDevCapsA(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetDevCapsA}
void NAKED midiOutGetDevCapsW(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetDevCapsW}
void NAKED midiOutGetErrorTextA(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetErrorTextA}
void NAKED midiOutGetErrorTextW(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetErrorTextW}
void NAKED midiOutGetID(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetID}
void NAKED midiOutGetNumDevs(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetNumDevs}
void NAKED midiOutGetVolume(void) {   INITWINMM __asm jmp dword ptr   pmidiOutGetVolume}
void NAKED midiOutLongMsg(void) {   INITWINMM __asm jmp dword ptr   pmidiOutLongMsg}
void NAKED midiOutMessage(void) {   INITWINMM __asm jmp dword ptr   pmidiOutMessage}
void NAKED midiOutOpen(void) {   INITWINMM __asm jmp dword ptr   pmidiOutOpen}
void NAKED midiOutPrepareHeader(void) {   INITWINMM __asm jmp dword ptr   pmidiOutPrepareHeader}
void NAKED midiOutReset(void) {   INITWINMM __asm jmp dword ptr   pmidiOutReset}
void NAKED midiOutSetVolume(void) {   INITWINMM __asm jmp dword ptr   pmidiOutSetVolume}
void NAKED midiOutShortMsg(void) {   INITWINMM __asm jmp dword ptr   pmidiOutShortMsg}
void NAKED midiOutUnprepareHeader(void) {   INITWINMM __asm jmp dword ptr   pmidiOutUnprepareHeader}
void NAKED midiStreamClose(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamClose}
void NAKED midiStreamOpen(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamOpen}
void NAKED midiStreamOut(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamOut}
void NAKED midiStreamPause(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamPause}
void NAKED midiStreamPosition(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamPosition}
void NAKED midiStreamProperty(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamProperty}
void NAKED midiStreamRestart(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamRestart}
void NAKED midiStreamStop(void) {   INITWINMM __asm jmp dword ptr   pmidiStreamStop}
void NAKED mixerGetDevCapsA(void) {   INITWINMM __asm jmp dword ptr   pmixerGetDevCapsA}
void NAKED mixerGetDevCapsW(void) {   INITWINMM __asm jmp dword ptr   pmixerGetDevCapsW}
void NAKED mixerGetID(void) {   INITWINMM __asm jmp dword ptr   pmixerGetID}
void NAKED mixerGetNumDevs(void) {   INITWINMM __asm jmp dword ptr   pmixerGetNumDevs}
void NAKED mixerMessage(void) {   INITWINMM __asm jmp dword ptr   pmixerMessage}
void NAKED mmDrvInstall(void) {   INITWINMM __asm jmp dword ptr   pmmDrvInstall}
void NAKED mmGetCurrentTask(void) {   INITWINMM __asm jmp dword ptr   pmmGetCurrentTask}
void NAKED mmTaskBlock(void) {   INITWINMM __asm jmp dword ptr   pmmTaskBlock}
void NAKED mmTaskCreate(void) {   INITWINMM __asm jmp dword ptr   pmmTaskCreate}
void NAKED mmTaskSignal(void) {   INITWINMM __asm jmp dword ptr   pmmTaskSignal}
void NAKED mmTaskYield(void) {   INITWINMM __asm jmp dword ptr   pmmTaskYield}
void NAKED mmioAdvance(void) {   INITWINMM __asm jmp dword ptr   pmmioAdvance}
void NAKED mmioAscend(void) {   INITWINMM __asm jmp dword ptr   pmmioAscend}
void NAKED mmioClose(void) {   INITWINMM __asm jmp dword ptr   pmmioClose}
void NAKED mmioCreateChunk(void) {   INITWINMM __asm jmp dword ptr   pmmioCreateChunk}
void NAKED mmioDescend(void) {   INITWINMM __asm jmp dword ptr   pmmioDescend}
void NAKED mmioFlush(void) {   INITWINMM __asm jmp dword ptr   pmmioFlush}
void NAKED mmioGetInfo(void) {   INITWINMM __asm jmp dword ptr   pmmioGetInfo}
void NAKED mmioInstallIOProcA(void) {   INITWINMM __asm jmp dword ptr   pmmioInstallIOProcA}
void NAKED mmioInstallIOProcW(void) {   INITWINMM __asm jmp dword ptr   pmmioInstallIOProcW}
void NAKED mmioOpenA(void) {   INITWINMM __asm jmp dword ptr   pmmioOpenA}
void NAKED mmioOpenW(void) {   INITWINMM __asm jmp dword ptr   pmmioOpenW}
void NAKED mmioRead(void) {  INITWINMM __asm jmp dword ptr   pmmioRead}
void NAKED mmioRenameA(void) {   INITWINMM __asm jmp dword ptr   pmmioRenameA}
void NAKED mmioRenameW(void) {   INITWINMM __asm jmp dword ptr   pmmioRenameW}
void NAKED mmioSeek(void) {   INITWINMM __asm jmp dword ptr   pmmioSeek}
void NAKED mmioSendMessage(void) {   INITWINMM __asm jmp dword ptr   pmmioSendMessage}
void NAKED mmioSetBuffer(void) {   INITWINMM __asm jmp dword ptr   pmmioSetBuffer}
void NAKED mmioSetInfo(void) {   INITWINMM __asm jmp dword ptr   pmmioSetInfo}
void NAKED mmioStringToFOURCCA(void) {   INITWINMM __asm jmp dword ptr   pmmioStringToFOURCCA}
void NAKED mmioStringToFOURCCW(void) {   INITWINMM __asm jmp dword ptr   pmmioStringToFOURCCW}
void NAKED mmioWrite(void) {   INITWINMM __asm jmp dword ptr   pmmioWrite}
void NAKED mmsystemGetVersion(void) {   INITWINMM __asm jmp dword ptr   pmmsystemGetVersion}
void NAKED mod32Message(void) {   INITWINMM __asm jmp dword ptr   pmod32Message}
void NAKED mxd32Message(void) {   INITWINMM __asm jmp dword ptr   pmxd32Message}
void NAKED sndPlaySoundA(void) {   INITWINMM __asm jmp dword ptr   psndPlaySoundA}
void NAKED sndPlaySoundW(void) {   INITWINMM __asm jmp dword ptr   psndPlaySoundW}
void NAKED tid32Message(void) {   INITWINMM __asm jmp dword ptr   ptid32Message}
void NAKED timeGetDevCaps(void) {   INITWINMM __asm jmp dword ptr   ptimeGetDevCaps}
void NAKED timeGetSystemTime(void) {   INITWINMM __asm jmp dword ptr   ptimeGetSystemTime}
void NAKED timeKillEvent(void) {   INITWINMM __asm jmp dword ptr   ptimeKillEvent}
void NAKED timeSetEvent(void) {   INITWINMM __asm jmp dword ptr   ptimeSetEvent}
void NAKED waveInGetDevCapsA(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetDevCapsA}
void NAKED waveInGetDevCapsW(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetDevCapsW}
void NAKED waveInGetErrorTextA(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetErrorTextA}
void NAKED waveInGetErrorTextW(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetErrorTextW}
void NAKED waveInGetID(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetID}
void NAKED waveInGetNumDevs(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetNumDevs}
void NAKED waveInMessage(void) {   INITWINMM __asm jmp dword ptr   pwaveInMessage}
void NAKED waveOutBreakLoop(void) {   INITWINMM __asm jmp dword ptr   pwaveOutBreakLoop}
void NAKED waveOutClose(void) {   INITWINMM __asm jmp dword ptr   pwaveOutClose}
void NAKED waveOutGetDevCapsA(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetDevCapsA}
void NAKED waveOutGetDevCapsW(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetDevCapsW}
void NAKED waveOutGetErrorTextA(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetErrorTextA}
void NAKED waveOutGetErrorTextW(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetErrorTextW}
void NAKED waveOutGetID(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetID}
void NAKED waveOutGetNumDevs(void) {   INITWINMM __asm jmp dword ptr pwaveOutGetNumDevs }
void NAKED waveOutGetPitch(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetPitch}
void NAKED waveOutGetPlaybackRate(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetPlaybackRate}
void NAKED waveOutGetVolume(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetVolume}
void NAKED waveOutMessage(void) {   INITWINMM __asm jmp dword ptr   pwaveOutMessage }
void NAKED waveOutOpen(void)    {   INITWINMM __asm jmp dword ptr   pwaveOutOpen }
void NAKED waveOutPause(void) {   INITWINMM __asm jmp dword ptr   pwaveOutPause}
void NAKED waveOutReset(void) {   INITWINMM __asm jmp dword ptr   pwaveOutReset }
void NAKED waveOutRestart(void) {   INITWINMM __asm jmp dword ptr   pwaveOutRestart}
void NAKED waveOutSetPitch(void) {   INITWINMM __asm jmp dword ptr   pwaveOutSetPitch}
void NAKED waveOutSetPlaybackRate(void) {   INITWINMM __asm jmp dword ptr   pwaveOutSetPlaybackRate}
void NAKED waveOutSetVolume(void) {   INITWINMM __asm jmp dword ptr   pwaveOutSetVolume}
void NAKED waveOutWrite(void) {   INITWINMM __asm jmp dword ptr   pwaveOutWrite}
void NAKED wid32Message(void) {   INITWINMM __asm jmp dword ptr   pwid32Message}
void NAKED winmmDbgOut(void) {   INITWINMM __asm jmp dword ptr   pwinmmDbgOut}
void NAKED winmmSetDebugLevel(void) {   INITWINMM __asm jmp dword ptr   pwinmmSetDebugLevel}
void NAKED wod32Message(void) {   INITWINMM __asm jmp dword ptr   pwod32Message}
/**/

void NAKED waveInGetPosition(void) {   INITWINMM __asm jmp dword ptr   pwaveInGetPosition}
void NAKED timeBeginPeriod(void) {   INITWINMM __asm jmp dword ptr   ptimeBeginPeriod}
void NAKED timeEndPeriod(void) {   INITWINMM __asm jmp dword ptr   ptimeEndPeriod}

void NAKED mixerSetControlDetails(void) {   INITWINMM __asm jmp dword ptr   pmixerSetControlDetails}
void NAKED mixerOpen(void) {   INITWINMM __asm jmp dword ptr   pmixerOpen}
void NAKED mixerClose(void) {   INITWINMM __asm jmp dword ptr   pmixerClose}
void NAKED mixerGetLineInfoA(void) {   INITWINMM __asm jmp dword ptr   pmixerGetLineInfoA}
void NAKED mixerGetLineInfoW(void) {   INITWINMM __asm jmp dword ptr   pmixerGetLineInfoW}
void NAKED mixerGetLineControlsA(void) {   INITWINMM __asm jmp dword ptr   pmixerGetLineControlsA}
void NAKED mixerGetLineControlsW(void) {   INITWINMM __asm jmp dword ptr   pmixerGetLineControlsW}
void NAKED mixerGetControlDetailsA(void) {   INITWINMM __asm jmp dword ptr   pmixerGetControlDetailsA}
void NAKED mixerGetControlDetailsW(void) {   INITWINMM __asm jmp dword ptr   pmixerGetControlDetailsW}

void NAKED waveOutGetPosition(void) {   INITWINMM __asm jmp dword ptr   pwaveOutGetPosition}
void NAKED waveOutPrepareHeader(void) {   INITWINMM __asm jmp dword ptr   pwaveOutPrepareHeader}
void NAKED waveOutUnprepareHeader(void) {   INITWINMM __asm jmp dword ptr   pwaveOutUnprepareHeader}

void NAKED CloseDriver(void) { INITWINMM __asm jmp dword ptr pCloseDriver  }
void NAKED DefDriverProc(void) { INITWINMM __asm jmp dword ptr pDefDriverProc  }
void NAKED DriverCallback(void) { INITWINMM __asm jmp dword ptr pDriverCallback  }
void NAKED DrvGetModuleHandle(void) { INITWINMM __asm jmp dword ptr pDrvGetModuleHandle  }
void NAKED GetDriverModuleHandle(void) { INITWINMM __asm jmp dword ptr pGetDriverModuleHandle  }
void NAKED MigrateAllDrivers(void) { INITWINMM __asm jmp dword ptr pMigrateAllDrivers  }
//void NAKED MigrateMidiUser(void) { INITWINMM __asm jmp dword ptr pMigrateMidiUser  }
void NAKED MigrateSoundEvents(void) { INITWINMM __asm jmp dword ptr pMigrateSoundEvents  }
void NAKED NotifyCallbackData(void) { INITWINMM __asm jmp dword ptr pNotifyCallbackData  }
void NAKED OpenDriver(void) { INITWINMM __asm jmp dword ptr pOpenDriver  }
void NAKED PlaySound(void) { INITWINMM __asm jmp dword ptr pPlaySound  }
void NAKED PlaySoundA(void) { INITWINMM __asm jmp dword ptr pPlaySoundA  }
void NAKED PlaySoundW(void) { INITWINMM __asm jmp dword ptr pPlaySoundW  }
void NAKED SendDriverMessage(void) { INITWINMM __asm jmp dword ptr pSendDriverMessage  }
void NAKED WOW32DriverCallback(void) { INITWINMM __asm jmp dword ptr pWOW32DriverCallback  }
void NAKED WOW32ResolveMultiMediaHandle(void) { INITWINMM __asm jmp dword ptr pWOW32ResolveMultiMediaHandle  }
void NAKED WOWAppExit(void) { INITWINMM __asm jmp dword ptr pWOWAppExit  }
void NAKED WinmmLogoff(void) { INITWINMM __asm jmp dword ptr pWinmmLogoff  }
void NAKED WinmmLogon(void) { INITWINMM __asm jmp dword ptr pWinmmLogon  }
void NAKED aux32Message(void) { INITWINMM __asm jmp dword ptr paux32Message  }
void NAKED auxGetDevCapsA(void) { INITWINMM __asm jmp dword ptr pauxGetDevCapsA  }
void NAKED auxGetDevCapsW(void) { INITWINMM __asm jmp dword ptr pauxGetDevCapsW  }
void NAKED auxGetNumDevs(void) { INITWINMM __asm jmp dword ptr pauxGetNumDevs  }
void NAKED auxGetVolume(void) { INITWINMM __asm jmp dword ptr pauxGetVolume  }
void NAKED auxOutMessage(void) { INITWINMM __asm jmp dword ptr pauxOutMessage  }
void NAKED auxSetVolume(void) { INITWINMM __asm jmp dword ptr pauxSetVolume  }
void NAKED joy32Message(void) { INITWINMM __asm jmp dword ptr pjoy32Message  }
void NAKED timeGetTime(void) {   INITWINMM __asm jmp dword ptr   ptimeGetTime}
void NAKED joyGetDevCapsW(void) {   INITWINMM __asm jmp dword ptr   pjoyGetDevCapsW}
//void NAKED joyGetPos(void) {   INITWINMM __asm jmp dword ptr   pjoyGetPos}

void NAKED gfxAddGfx(void) {   INITWINMM __asm jmp dword ptr   pgfxAddGfx}
void NAKED gfxBatchChange(void) {   INITWINMM __asm jmp dword ptr   pgfxBatchChange}
void NAKED gfxCreateGfxFactoriesList(void) {   INITWINMM __asm jmp dword ptr   pgfxCreateGfxFactoriesList}
void NAKED gfxCreateZoneFactoriesList(void) {   INITWINMM __asm jmp dword ptr   pgfxCreateZoneFactoriesList}
void NAKED gfxDestroyDeviceInterfaceList(void) {   INITWINMM __asm jmp dword ptr   pgfxDestroyDeviceInterfaceList}
void NAKED gfxEnumerateGfxs(void) {   INITWINMM __asm jmp dword ptr   pgfxEnumerateGfxs}
void NAKED _gfxLogoff(void) {   INITWINMM __asm jmp dword ptr   p_gfxLogoff}
void NAKED _gfxLogon(void) {   INITWINMM __asm jmp dword ptr   p_gfxLogon}
void NAKED gfxModifyGfx(void) {   INITWINMM __asm jmp dword ptr   pgfxModifyGfx}
void NAKED gfxOpenGfx(void) {   INITWINMM __asm jmp dword ptr   pgfxOpenGfx}
void NAKED gfxRemoveGfx(void) {   INITWINMM __asm jmp dword ptr   pgfxRemoveGfx}


UINT NAKED waveInAddBuffer(HWAVEIN a, void* b, UINT c) {  INITWINMM __asm jmp dword ptr   pwaveInAddBuffer}
UINT NAKED waveInClose(HWAVEIN a) { INITWINMM  __asm jmp dword ptr   pwaveInClose}
UINT NAKED waveInOpen(void* a, UINT b, void* c, DWORD d, DWORD e, DWORD f) { INITWINMM  __asm jmp dword ptr   pwaveInOpen}
UINT NAKED waveInPrepareHeader(HWAVEIN a, void* b, UINT c) { INITWINMM  __asm jmp dword ptr   pwaveInPrepareHeader}
UINT NAKED waveInStart(HWAVEIN a ) { INITWINMM  __asm jmp dword ptr   pwaveInStart}
UINT NAKED waveInStop(HWAVEIN a) { INITWINMM  __asm jmp dword ptr   pwaveInStop}
UINT NAKED waveInUnprepareHeader(HWAVEIN a, void* b, UINT c) { INITWINMM  __asm jmp dword ptr   pwaveInUnprepareHeader}
UINT NAKED waveInReset(HWAVEIN a) { INITWINMM  __asm jmp dword ptr   pwaveInReset}