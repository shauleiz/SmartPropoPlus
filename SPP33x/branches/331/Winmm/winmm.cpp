// pcpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "mmsystem.h"
#include "winmm.h"
#include "assert.h"
#include "jr.h"

int WINAPI  DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			TlsIndex = TlsAlloc();
            LoadWinmm();
            TlsSetValue(TlsIndex, hWinmm);
			StartPropo();
			break;

        case DLL_THREAD_ATTACH:
            TlsSetValue(TlsIndex, LoadLibrary(path));
            LoadWinmm();
            break;

        case DLL_THREAD_DETACH:
            FreeLibrary((HINSTANCE)TlsGetValue(TlsIndex));
            break;

        case DLL_PROCESS_DETACH:
            FreeLibrary(hWinmm);
			StopPropo();
            break;
    }
			
   return 1;
}

/*
	Process PPM pulse
	A long (over 200 samples) leading high, then a short low, then up to six highs followed by short lows.
	The size of a high pulse may vary between 30 to 70 samples, mapped to joystick values of 1024 to 438
	where the mid-point of 50 samples is translated to joystick position of 731.
*/
static void __fastcall ProcessPulse(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */

	/* If pulse is a separator then go to the next one */
	if (width < PPM_SEP+5)
		return;

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (sync == 0 && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
		return;
    }

    if (!sync) return; /* still waiting for sync */


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input)
		newdata = (int)(1024 - (width - PPM_MIN) / (PPM_MAX - PPM_MIN) * 1024); /* JR */
	else
		newdata = (int)((width - PPM_MIN) / (PPM_MAX - PPM_MIN) * 1024);		/* Futaba */

	/* Trim values into 0-1023 boundries */
    if (newdata < 0) newdata = 0;
    else if (newdata > 1023) newdata = 1023;

	/* Update data - do not allow abrupt change */
    if (data[datacount] - newdata > 100) data[datacount] -= 100;
    else if (newdata - data[datacount] > 100) data[datacount] += 100;
    else data[datacount] = (data[datacount] + newdata) / 2;

	
	Position[datacount] = data[datacount];	/* Assign data to joystick channels */
	if (datacount == 5)	sync = 0;			/* Reset sync after channel 6 */

    datacount++;
}

//---------------------------------------------------------------------------

/*
	ProcessData - process a single audio sample of unknown type (8-16 bits mono)
	The audio sample may be 8-bit PCM, ranging 0-255, mid-point is 128
	The audio sample may be 16-bit PCM, ranging from -32768 to 32767, mid-point is 0
	The minimal step is 1
*/
static void __fastcall ProcessData(int i)
{
    static double min = 0;	/* Sticky minimum sample value */
    static double max = 0;	/* Sticky maximum sample value */
    static int high = 0;	/* Number of contingious above-threshold samples */
    static int low = 0;		/* Number of contingious below-threshold samples */
    double threshold;		/* calculated mid-point */

	/* Initialization of the min/max vaues */
    max -= 0.1;
    min += 0.1;
    if (max < min) max = min + 1;

    if (i> max) max = i;			/* Update max value */
    else if (i < min) min = i;		/* Update min value */
    threshold = (min + max) / 2;	/* Update mid-point value */


	/* Update the width of the number of the low/high samples */
	/* If edge, then call ProcessPulse() to process the previous high/low level */
    if (i > threshold) 
	{
	high++;
        if (low) 
		{
            ProcessPulse(low, FALSE);
            low = 0;
        }
    } else 
	{
        low++;
        if (high) 
		{
            ProcessPulse(high, TRUE);
            high = 0;
        }
    }
}

//---------------------------------------------------------------------------
/*
	Callback function called by the audio device driver when the input buffer is valid
	Ignores the start/stop condition
	For every entry in the audio buffer, calles ProcessData() with the entry as a parameter
*/
static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, void *lpUser, WAVEHDR *buf, DWORD Reserved)
{
    int i;
    if (uMsg == WIM_DATA) /* Sent when the device driver is finished with a data block */
	{
        int Size = waveFmt.nBlockAlign;
        int Length = buf->dwBytesRecorded / Size;
        if (Size == 1) /* 8-bit per sample. Value range: 0-255 */
		{
            for (i = 0; i < Length; i++) 
			{
                ProcessData((unsigned char)buf->lpData[i]);
            }
        } 
		else if (Size == 2)  /* 16-bit per sample. Value range: -32k - +32k */
		{
            for (i = 0; i < Length; i++) 
			{
                ProcessData(((signed short*)(buf->lpData))[i]);
            }
        }

		/* Requests the audio device to refill the current buffer */
        if (waveRecording) 
			pwaveInAddBuffer(waveIn, waveBuf[buf->dwUser], sizeof(WAVEHDR));
    }
}

//---------------------------------------------------------------------------
/*
	StartPropo - Initialize the WAVE IN capture process

	static WAVEFORMATEX  waveFmt: Wave format structure, initialized to:
				Wave format = PCM
				Mono
				44100 samples/second
				16 bits per sample

	static HWAVEIN waveIn: handle to the WAVE IN audio stream
	static WAVEHDR      *waveBuf[2]: Two headers to the audio buffers. Initialized

*/
void StartPropo(void)
{
    int i;
    waveRecording = TRUE;					/* Start recording */

	/* Wave format structure initialization */ 
    waveFmt.wFormatTag = WAVE_FORMAT_PCM;
    waveFmt.nChannels = 1;
    waveFmt.nSamplesPerSec = 44100;
    waveFmt.wBitsPerSample =16;
    waveFmt.nBlockAlign = waveFmt.wBitsPerSample / 8 * waveFmt.nChannels;
    waveFmt.nAvgBytesPerSec = waveFmt.nSamplesPerSec * waveFmt.nBlockAlign;
    waveFmt.cbSize = 0;

	/* Open audio stream, assigning 'waveInProc()' as the WAVE IN callback function*/
    pwaveInOpen(&waveIn, WAVE_MAPPER, &waveFmt, (DWORD)(waveInProc), 0, CALLBACK_FUNCTION);

	/* Initialize the two WAVE IN buffers*/
    for (i = 0; i < 2; i++) {
        waveBuf[i] = (WAVEHDR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEHDR));
        waveBuf[i]->lpData = (char*)HeapAlloc(GetProcessHeap(), 0, waveBufSize);
        waveBuf[i]->dwBufferLength = waveBufSize;
        waveBuf[i]->dwUser = i;
        pwaveInPrepareHeader(waveIn, waveBuf[i], sizeof(WAVEHDR));
        pwaveInAddBuffer(waveIn, waveBuf[i], sizeof(WAVEHDR));
    }

	/* Begin listening to WAVE IN */
    pwaveInStart(waveIn);
}
//---------------------------------------------------------------------------

void StopPropo(void)
{
    int i;

    waveRecording = FALSE;
    pwaveInStop(waveIn);
    for (i = 0; i < 2 ;i++) {
        pwaveInUnprepareHeader(waveIn, waveBuf[i], sizeof(WAVEHDR));
        HeapFree(GetProcessHeap(), 0, waveBuf[i]->lpData);
        HeapFree(GetProcessHeap(), 0, waveBuf[i]);
    }
    pwaveInClose(waveIn);
}

//---------------------------------------------------------------------------
int GetPosition(int ch)
{
    return Position[ch];
}
//---------------------------------------------------------------------------


// Exported function.
extern __declspec(dllexport) UINT __stdcall   joyGetDevCapsA(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc) {
   if (uJoyID == 0) {
        pjc->wMid = 0;
        pjc->wPid = 0;
        strcpy((char*)pjc->szPname, PRODUCT);
        pjc->wXmin = pjc->wYmin = pjc->wZmin = pjc->wRmin = pjc->wUmin = pjc->wVmin = 0;
        pjc->wXmax = pjc->wYmax = pjc->wZmax = pjc->wRmax = pjc->wUmax = pjc->wVmax = 100;
        pjc->wNumButtons = pjc->wMaxButtons = 6;
        pjc->wMaxAxes = pjc->wNumAxes = 6;
        pjc->wPeriodMin = pjc->wPeriodMax = 0;
        pjc->wCaps = JOYCAPS_HASZ | JOYCAPS_HASR | JOYCAPS_HASU | JOYCAPS_HASV;
        pjc->szRegKey[0] = 0;
        pjc->szOEMVxD[0] = 0;
		AfxMessageBox("joyGetDevCapsA");
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
    return JOYERR_UNPLUGGED;
}

extern __declspec(dllexport) UINT __stdcall        joyGetNumDevs(void)
{
    return 1;
}


extern __declspec(dllexport) UINT __stdcall     joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
   if (uJoyID == 0) {
        pji->dwXpos = Position[3];
        pji->dwYpos = Position[2];
        pji->dwZpos = Position[1];
        pji->dwRpos = Position[0];
        pji->dwUpos = Position[4];
        pji->dwVpos = Position[5];
        return JOYERR_NOERROR;
   };

    return JOYERR_UNPLUGGED;
}


BOOL LoadWinmm(void)
{

    if (!GetSystemDirectory(path, MAX_PATH)) 
		return FALSE;
    strcat(path, "\\winmm.dll");
    hWinmm = LoadLibrary(path);
    if (!hWinmm) 
		return FALSE;

	GetPointerToOriginalFunc();

	return TRUE;
}





/* Get pointer to functions in the original DLL */
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
    pjoyGetPos = GetProcAddress(hWinmm, "joyGetPos");
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

    pwaveInAddBuffer =  (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddress(hWinmm, "waveInAddBuffer");
    pwaveInClose = (UINT ( WINAPI *)(HWAVEIN))GetProcAddress(hWinmm, "waveInClose");
    pwaveInOpen = (UINT (WINAPI*)(LPHWAVIN, UINT, LPCWAVEFORMATEX, DWORD, DWORD, DWORD))GetProcAddress(hWinmm, "waveInOpen");
    pwaveInPrepareHeader = (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddress(hWinmm, "waveInPrepareHeader");
    pwaveInStart = (UINT ( WINAPI *)(HWAVEIN))GetProcAddress(hWinmm, "waveInStart");
    pwaveInStop = (UINT ( WINAPI *)(HWAVEIN))GetProcAddress(hWinmm, "waveInStop");
    pwaveInUnprepareHeader = (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddress(hWinmm, "waveInUnprepareHeader");
	pjoyGetPosEx = (UINT ( WINAPI *)(UINT uJoyID, LPJOYINFOEX pji))GetProcAddress(hWinmm, "joyGetPosEx");
}

/* 
	The following functions are interface functions of the DLL that are implemented
	as a call to the corresponding functions in the original DLL file

	The call is a "naked" call to an assembly code that jumps 
	to the location of the original function

	#define NAKED __declspec(naked)
*/
void NAKED joyConfigChanged(void) { __asm jmp dword ptr pjoyConfigChanged  }
void NAKED joyGetThreshold(void)   {  __asm  jmp dword ptr   pjoyGetThreshold}
void NAKED joyReleaseCapture(void) {  __asm jmp dword ptr   pjoyReleaseCapture}
void NAKED joySetCapture(void) {  __asm jmp dword ptr   pjoySetCapture}
void NAKED joySetThreshold(void) {  __asm jmp dword ptr   pjoySetThreshold}
void NAKED mci32Message(void) {  __asm jmp dword ptr   pmci32Message}
void NAKED mciDriverNotify(void) {  __asm jmp dword ptr   pmciDriverNotify}
void NAKED mciDriverYield(void) {  __asm jmp dword ptr   pmciDriverYield}
void NAKED mciExecute(void) {  __asm jmp dword ptr   pmciExecute}
void NAKED mciFreeCommandResource(void) {  __asm jmp dword ptr   pmciFreeCommandResource}
void NAKED mciGetCreatorTask(void) {  __asm jmp dword ptr   pmciGetCreatorTask}
void NAKED mciGetDeviceIDA(void) {  __asm jmp dword ptr   pmciGetDeviceIDA}
void NAKED mciGetDeviceIDFromElementIDA(void) {  __asm jmp dword ptr   pmciGetDeviceIDFromElementIDA}
void NAKED mciGetDeviceIDFromElementIDW(void) {  __asm jmp dword ptr   pmciGetDeviceIDFromElementIDW}
void NAKED mciGetDeviceIDW(void) {  __asm jmp dword ptr   pmciGetDeviceIDW}
void NAKED mciGetDriverData(void) {  __asm jmp dword ptr   pmciGetDriverData}
void NAKED mciGetErrorStringA(void) {  __asm jmp dword ptr   pmciGetErrorStringA}
void NAKED mciGetErrorStringW(void) {  __asm jmp dword ptr   pmciGetErrorStringW}
void NAKED mciGetYieldProc(void) {  __asm jmp dword ptr   pmciGetYieldProc}
void NAKED mciLoadCommandResource(void) {  __asm jmp dword ptr   pmciLoadCommandResource}
void NAKED mciSendCommandA(void) {  __asm jmp dword ptr   pmciSendCommandA}
void NAKED mciSendCommandW(void) {  __asm jmp dword ptr   pmciSendCommandW}
void NAKED mciSendStringA(void) {  __asm jmp dword ptr   pmciSendStringA}
void NAKED mciSendStringW(void) {  __asm jmp dword ptr   pmciSendStringW}
void NAKED mciSetDriverData(void) {  __asm jmp dword ptr   pmciSetDriverData}
void NAKED mciSetYieldProc(void) {  __asm jmp dword ptr   pmciSetYieldProc}
void NAKED mid32Message(void) {  __asm jmp dword ptr   pmid32Message}
void NAKED midiConnect(void) {  __asm jmp dword ptr   pmidiConnect}
void NAKED midiDisconnect(void) {  __asm jmp dword ptr   pmidiDisconnect}
void NAKED midiInAddBuffer(void) {  __asm jmp dword ptr   pmidiInAddBuffer}
void NAKED midiInClose(void) {  __asm jmp dword ptr   pmidiInClose}
void NAKED midiInGetDevCapsA(void) {  __asm jmp dword ptr   pmidiInGetDevCapsA}
void NAKED midiInGetDevCapsW(void) {  __asm jmp dword ptr   pmidiInGetDevCapsW}
void NAKED midiInGetErrorTextA(void) {  __asm jmp dword ptr   pmidiInGetErrorTextA}
void NAKED midiInGetErrorTextW(void) {  __asm jmp dword ptr   pmidiInGetErrorTextW}
void NAKED midiInGetID(void) {  __asm jmp dword ptr   pmidiInGetID}
void NAKED midiInGetNumDevs(void) {  __asm jmp dword ptr   pmidiInGetNumDevs}
void NAKED midiInMessage(void) {  __asm jmp dword ptr   pmidiInMessage}
void NAKED midiInOpen(void) {  __asm jmp dword ptr   pmidiInOpen}
void NAKED midiInPrepareHeader(void) {  __asm jmp dword ptr   pmidiInPrepareHeader}
void NAKED midiInReset(void) {  __asm jmp dword ptr   pmidiInReset}
void NAKED midiInStart(void) {  __asm jmp dword ptr   pmidiInStart}
void NAKED midiInStop(void) {  __asm jmp dword ptr   pmidiInStop}
void NAKED midiInUnprepareHeader(void) {  __asm jmp dword ptr   pmidiInUnprepareHeader}
void NAKED midiOutCacheDrumPatches(void) {  __asm jmp dword ptr   pmidiOutCacheDrumPatches}
void NAKED midiOutCachePatches(void) {  __asm jmp dword ptr   pmidiOutCachePatches}
void NAKED midiOutClose(void) {  __asm jmp dword ptr   pmidiOutClose}
void NAKED midiOutGetDevCapsA(void) {  __asm jmp dword ptr   pmidiOutGetDevCapsA}
void NAKED midiOutGetDevCapsW(void) {  __asm jmp dword ptr   pmidiOutGetDevCapsW}
void NAKED midiOutGetErrorTextA(void) {  __asm jmp dword ptr   pmidiOutGetErrorTextA}
void NAKED midiOutGetErrorTextW(void) {  __asm jmp dword ptr   pmidiOutGetErrorTextW}
void NAKED midiOutGetID(void) {  __asm jmp dword ptr   pmidiOutGetID}
void NAKED midiOutGetNumDevs(void) {  __asm jmp dword ptr   pmidiOutGetNumDevs}
void NAKED midiOutGetVolume(void) {  __asm jmp dword ptr   pmidiOutGetVolume}
void NAKED midiOutLongMsg(void) {  __asm jmp dword ptr   pmidiOutLongMsg}
void NAKED midiOutMessage(void) {  __asm jmp dword ptr   pmidiOutMessage}
void NAKED midiOutOpen(void) {  __asm jmp dword ptr   pmidiOutOpen}
void NAKED midiOutPrepareHeader(void) {  __asm jmp dword ptr   pmidiOutPrepareHeader}
void NAKED midiOutReset(void) {  __asm jmp dword ptr   pmidiOutReset}
void NAKED midiOutSetVolume(void) {  __asm jmp dword ptr   pmidiOutSetVolume}
void NAKED midiOutShortMsg(void) {  __asm jmp dword ptr   pmidiOutShortMsg}
void NAKED midiOutUnprepareHeader(void) {  __asm jmp dword ptr   pmidiOutUnprepareHeader}
void NAKED midiStreamClose(void) {  __asm jmp dword ptr   pmidiStreamClose}
void NAKED midiStreamOpen(void) {  __asm jmp dword ptr   pmidiStreamOpen}
void NAKED midiStreamOut(void) {  __asm jmp dword ptr   pmidiStreamOut}
void NAKED midiStreamPause(void) {  __asm jmp dword ptr   pmidiStreamPause}
void NAKED midiStreamPosition(void) {  __asm jmp dword ptr   pmidiStreamPosition}
void NAKED midiStreamProperty(void) {  __asm jmp dword ptr   pmidiStreamProperty}
void NAKED midiStreamRestart(void) {  __asm jmp dword ptr   pmidiStreamRestart}
void NAKED midiStreamStop(void) {  __asm jmp dword ptr   pmidiStreamStop}
void NAKED mixerGetDevCapsA(void) {  __asm jmp dword ptr   pmixerGetDevCapsA}
void NAKED mixerGetDevCapsW(void) {  __asm jmp dword ptr   pmixerGetDevCapsW}
void NAKED mixerGetID(void) {  __asm jmp dword ptr   pmixerGetID}
void NAKED mixerGetNumDevs(void) {  __asm jmp dword ptr   pmixerGetNumDevs}
void NAKED mixerMessage(void) {  __asm jmp dword ptr   pmixerMessage}
void NAKED mmDrvInstall(void) {  __asm jmp dword ptr   pmmDrvInstall}
void NAKED mmGetCurrentTask(void) {  __asm jmp dword ptr   pmmGetCurrentTask}
void NAKED mmTaskBlock(void) {  __asm jmp dword ptr   pmmTaskBlock}
void NAKED mmTaskCreate(void) {  __asm jmp dword ptr   pmmTaskCreate}
void NAKED mmTaskSignal(void) {  __asm jmp dword ptr   pmmTaskSignal}
void NAKED mmTaskYield(void) {  __asm jmp dword ptr   pmmTaskYield}
void NAKED mmioAdvance(void) {  __asm jmp dword ptr   pmmioAdvance}
void NAKED mmioAscend(void) {  __asm jmp dword ptr   pmmioAscend}
void NAKED mmioClose(void) {  __asm jmp dword ptr   pmmioClose}
void NAKED mmioCreateChunk(void) {  __asm jmp dword ptr   pmmioCreateChunk}
void NAKED mmioDescend(void) {  __asm jmp dword ptr   pmmioDescend}
void NAKED mmioFlush(void) {  __asm jmp dword ptr   pmmioFlush}
void NAKED mmioGetInfo(void) {  __asm jmp dword ptr   pmmioGetInfo}
void NAKED mmioInstallIOProcA(void) {  __asm jmp dword ptr   pmmioInstallIOProcA}
void NAKED mmioInstallIOProcW(void) {  __asm jmp dword ptr   pmmioInstallIOProcW}
void NAKED mmioOpenA(void) {  __asm jmp dword ptr   pmmioOpenA}
void NAKED mmioOpenW(void) {  __asm jmp dword ptr   pmmioOpenW}
void NAKED mmioRead(void) {  __asm jmp dword ptr   pmmioRead}
void NAKED mmioRenameA(void) {  __asm jmp dword ptr   pmmioRenameA}
void NAKED mmioRenameW(void) {  __asm jmp dword ptr   pmmioRenameW}
void NAKED mmioSeek(void) {  __asm jmp dword ptr   pmmioSeek}
void NAKED mmioSendMessage(void) {  __asm jmp dword ptr   pmmioSendMessage}
void NAKED mmioSetBuffer(void) {  __asm jmp dword ptr   pmmioSetBuffer}
void NAKED mmioSetInfo(void) {  __asm jmp dword ptr   pmmioSetInfo}
void NAKED mmioStringToFOURCCA(void) {  __asm jmp dword ptr   pmmioStringToFOURCCA}
void NAKED mmioStringToFOURCCW(void) {  __asm jmp dword ptr   pmmioStringToFOURCCW}
void NAKED mmioWrite(void) {  __asm jmp dword ptr   pmmioWrite}
void NAKED mmsystemGetVersion(void) {  __asm jmp dword ptr   pmmsystemGetVersion}
void NAKED mod32Message(void) {  __asm jmp dword ptr   pmod32Message}
void NAKED mxd32Message(void) {  __asm jmp dword ptr   pmxd32Message}
void NAKED sndPlaySoundA(void) {  __asm jmp dword ptr   psndPlaySoundA}
void NAKED sndPlaySoundW(void) {  __asm jmp dword ptr   psndPlaySoundW}
void NAKED tid32Message(void) {  __asm jmp dword ptr   ptid32Message}
void NAKED timeGetDevCaps(void) {  __asm jmp dword ptr   ptimeGetDevCaps}
void NAKED timeGetSystemTime(void) {  __asm jmp dword ptr   ptimeGetSystemTime}
void NAKED timeKillEvent(void) {  __asm jmp dword ptr   ptimeKillEvent}
void NAKED timeSetEvent(void) {  __asm jmp dword ptr   ptimeSetEvent}
void NAKED waveInGetDevCapsA(void) {  __asm jmp dword ptr   pwaveInGetDevCapsA}
void NAKED waveInGetDevCapsW(void) {  __asm jmp dword ptr   pwaveInGetDevCapsW}
void NAKED waveInGetErrorTextA(void) {  __asm jmp dword ptr   pwaveInGetErrorTextA}
void NAKED waveInGetErrorTextW(void) {  __asm jmp dword ptr   pwaveInGetErrorTextW}
void NAKED waveInGetID(void) {  __asm jmp dword ptr   pwaveInGetID}
void NAKED waveInGetNumDevs(void) {  __asm jmp dword ptr   pwaveInGetNumDevs}
void NAKED waveInMessage(void) {  __asm jmp dword ptr   pwaveInMessage}
void NAKED waveOutBreakLoop(void) {  __asm jmp dword ptr   pwaveOutBreakLoop}
void NAKED waveOutClose(void) {  __asm jmp dword ptr   pwaveOutClose}
void NAKED waveOutGetDevCapsA(void) {  __asm jmp dword ptr   pwaveOutGetDevCapsA}
void NAKED waveOutGetDevCapsW(void) {  __asm jmp dword ptr   pwaveOutGetDevCapsW}
void NAKED waveOutGetErrorTextA(void) {  __asm jmp dword ptr   pwaveOutGetErrorTextA}
void NAKED waveOutGetErrorTextW(void) {  __asm jmp dword ptr   pwaveOutGetErrorTextW}
void NAKED waveOutGetID(void) {  __asm jmp dword ptr   pwaveOutGetID}
void NAKED waveOutGetNumDevs(void) {  __asm jmp dword ptr pwaveOutGetNumDevs }
void NAKED waveOutGetPitch(void) {  __asm jmp dword ptr   pwaveOutGetPitch}
void NAKED waveOutGetPlaybackRate(void) {  __asm jmp dword ptr   pwaveOutGetPlaybackRate}
void NAKED waveOutGetVolume(void) {  __asm jmp dword ptr   pwaveOutGetVolume}
void NAKED waveOutMessage(void) {  __asm jmp dword ptr   pwaveOutMessage }
void NAKED waveOutOpen(void)    {  __asm jmp dword ptr   pwaveOutOpen }
void NAKED waveOutPause(void) {  __asm jmp dword ptr   pwaveOutPause}
void NAKED waveOutReset(void) {  __asm jmp dword ptr   pwaveOutReset }
void NAKED waveOutRestart(void) {  __asm jmp dword ptr   pwaveOutRestart}
void NAKED waveOutSetPitch(void) {  __asm jmp dword ptr   pwaveOutSetPitch}
void NAKED waveOutSetPlaybackRate(void) {  __asm jmp dword ptr   pwaveOutSetPlaybackRate}
void NAKED waveOutSetVolume(void) {  __asm jmp dword ptr   pwaveOutSetVolume}
void NAKED waveOutWrite(void) {  __asm jmp dword ptr   pwaveOutWrite}
void NAKED wid32Message(void) {  __asm jmp dword ptr   pwid32Message}
void NAKED winmmDbgOut(void) {  __asm jmp dword ptr   pwinmmDbgOut}
void NAKED winmmSetDebugLevel(void) {  __asm jmp dword ptr   pwinmmSetDebugLevel}
void NAKED wod32Message(void) {  __asm jmp dword ptr   pwod32Message}
/**/

void NAKED waveInGetPosition(void) {  __asm jmp dword ptr   pwaveInGetPosition}
void NAKED timeBeginPeriod(void) {  __asm jmp dword ptr   ptimeBeginPeriod}
void NAKED timeEndPeriod(void) {  __asm jmp dword ptr   ptimeEndPeriod}

void NAKED mixerSetControlDetails(void) {  __asm jmp dword ptr   pmixerSetControlDetails}
void NAKED mixerOpen(void) {  __asm jmp dword ptr   pmixerOpen}
void NAKED mixerClose(void) {  __asm jmp dword ptr   pmixerClose}
void NAKED mixerGetLineInfoA(void) {  __asm jmp dword ptr   pmixerGetLineInfoA}
void NAKED mixerGetLineInfoW(void) {  __asm jmp dword ptr   pmixerGetLineInfoW}
void NAKED mixerGetLineControlsA(void) {  __asm jmp dword ptr   pmixerGetLineControlsA}
void NAKED mixerGetLineControlsW(void) {  __asm jmp dword ptr   pmixerGetLineControlsW}
void NAKED mixerGetControlDetailsA(void) {  __asm jmp dword ptr   pmixerGetControlDetailsA}
void NAKED mixerGetControlDetailsW(void) {  __asm jmp dword ptr   pmixerGetControlDetailsW}

void NAKED waveOutGetPosition(void) {  __asm jmp dword ptr   pwaveOutGetPosition}
void NAKED waveOutPrepareHeader(void) {  __asm jmp dword ptr   pwaveOutPrepareHeader}
void NAKED waveOutUnprepareHeader(void) {  __asm jmp dword ptr   pwaveOutUnprepareHeader}

void NAKED CloseDriver(void) { __asm jmp dword ptr pCloseDriver  }
void NAKED DefDriverProc(void) { __asm jmp dword ptr pDefDriverProc  }
void NAKED DriverCallback(void) { __asm jmp dword ptr pDriverCallback  }
void NAKED DrvGetModuleHandle(void) { __asm jmp dword ptr pDrvGetModuleHandle  }
void NAKED GetDriverModuleHandle(void) { __asm jmp dword ptr pGetDriverModuleHandle  }
void NAKED MigrateAllDrivers(void) { __asm jmp dword ptr pMigrateAllDrivers  }
//void NAKED MigrateMidiUser(void) { __asm jmp dword ptr pMigrateMidiUser  }
void NAKED MigrateSoundEvents(void) { __asm jmp dword ptr pMigrateSoundEvents  }
void NAKED NotifyCallbackData(void) { __asm jmp dword ptr pNotifyCallbackData  }
void NAKED OpenDriver(void) { __asm jmp dword ptr pOpenDriver  }
void NAKED PlaySound(void) { __asm jmp dword ptr pPlaySound  }
void NAKED PlaySoundA(void) { __asm jmp dword ptr pPlaySoundA  }
void NAKED PlaySoundW(void) { __asm jmp dword ptr pPlaySoundW  }
void NAKED SendDriverMessage(void) { __asm jmp dword ptr pSendDriverMessage  }
void NAKED WOW32DriverCallback(void) { __asm jmp dword ptr pWOW32DriverCallback  }
void NAKED WOW32ResolveMultiMediaHandle(void) { __asm jmp dword ptr pWOW32ResolveMultiMediaHandle  }
void NAKED WOWAppExit(void) { __asm jmp dword ptr pWOWAppExit  }
void NAKED WinmmLogoff(void) { __asm jmp dword ptr pWinmmLogoff  }
void NAKED WinmmLogon(void) { __asm jmp dword ptr pWinmmLogon  }
void NAKED aux32Message(void) { __asm jmp dword ptr paux32Message  }
void NAKED auxGetDevCapsA(void) { __asm jmp dword ptr pauxGetDevCapsA  }
void NAKED auxGetDevCapsW(void) { __asm jmp dword ptr pauxGetDevCapsW  }
void NAKED auxGetNumDevs(void) { __asm jmp dword ptr pauxGetNumDevs  }
void NAKED auxGetVolume(void) { __asm jmp dword ptr pauxGetVolume  }
void NAKED auxOutMessage(void) { __asm jmp dword ptr pauxOutMessage  }
void NAKED auxSetVolume(void) { __asm jmp dword ptr pauxSetVolume  }
void NAKED joy32Message(void) { __asm jmp dword ptr pjoy32Message  }
void NAKED timeGetTime(void) {  __asm jmp dword ptr   ptimeGetTime}
void NAKED joyGetDevCapsW(void) {  __asm jmp dword ptr   pjoyGetDevCapsW}
void NAKED joyGetPos(void) {  __asm jmp dword ptr   pjoyGetPos}



UINT NAKED waveInAddBuffer(HWAVEIN a, void* b, UINT c) {  __asm jmp dword ptr   pwaveInAddBuffer}
UINT NAKED waveInClose(HWAVEIN a) {  __asm jmp dword ptr   pwaveInClose}
UINT NAKED waveInOpen(void* a, UINT b, void* c, DWORD d, DWORD e, DWORD f) {  __asm jmp dword ptr   pwaveInOpen}
UINT NAKED waveInPrepareHeader(HWAVEIN a, void* b, UINT c) {  __asm jmp dword ptr   pwaveInPrepareHeader}
UINT NAKED waveInStart(HWAVEIN a ) {  __asm jmp dword ptr   pwaveInStart}
UINT NAKED waveInStop(HWAVEIN a) {  __asm jmp dword ptr   pwaveInStop}
UINT NAKED waveInUnprepareHeader(HWAVEIN a, void* b, UINT c) {  __asm jmp dword ptr   pwaveInUnprepareHeader}
UINT NAKED waveInReset(HWAVEIN a) {  __asm jmp dword ptr   pwaveInReset}
