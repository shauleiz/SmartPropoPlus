// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WINMM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WINMM_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINMM_EXPORTS
#define WINMM_API __declspec(dllexport)
#else
#define WINMM_API __declspec(dllimport)
#endif

#define NAKED __declspec(naked)

#define PW_FUTABA	6.623
#define PW_JR		7.340
#define PPM_MIN		30.0
#define PPM_MAX		80.0
#define PPM_TRIG	200
#define PPM_SEP		15.0

#define BASEPRODUCT	  "SmartPropoPlus"
#ifdef AIR_PCM1
#define PRODUCT	  BASEPRODUCT " - Airtronics1"
#elif defined AIR_PCM2
#define PRODUCT	  BASEPRODUCT " - Airtronics2"
#elif defined JR_PCM
#define PRODUCT	  BASEPRODUCT " - JR PCM"
#elif defined FUTABA_PCM
#define PRODUCT	  BASEPRODUCT " - Futaba PCM"
#elif defined PPM
#define PRODUCT	  BASEPRODUCT " - General PPM"
#else
#define PRODUCT	  BASEPRODUCT
#endif

typedef  void ( * PP)(int width, BOOL input);

extern WINMM_API int nWinmm;


extern WINMM_API void timeGetTime(void);
extern WINMM_API void joyGetThreshold(void);
extern WINMM_API void joyReleaseCapture(void);
extern WINMM_API void joySetCapture(void) ;
extern WINMM_API void joySetThreshold(void) ;
extern WINMM_API void mci32Message(void) ;
extern WINMM_API void mciDriverNotify(void) ;
extern WINMM_API void mciDriverYield(void) ;
extern WINMM_API void mciExecute(void) ;
extern WINMM_API void mciFreeCommandResource(void) ;
extern WINMM_API void mciGetCreatorTask(void) ;
extern WINMM_API void mciGetDeviceIDA(void) ;
extern WINMM_API void mciGetDeviceIDFromElementIDA(void) ;
extern WINMM_API void mciGetDeviceIDFromElementIDW(void) ;
extern WINMM_API void mciGetDeviceIDW(void) ;
extern WINMM_API void mciGetDriverData(void) ;
extern WINMM_API void mciGetErrorStringA(void) ;
extern WINMM_API void mciGetErrorStringW(void) ;
extern WINMM_API void mciGetYieldProc(void) ;
extern WINMM_API void mciLoadCommandResource(void) ;
extern WINMM_API void mciSendCommandA(void) ;
extern WINMM_API void mciSendCommandW(void) ;
extern WINMM_API void mciSendStringA(void) ;
extern WINMM_API void mciSendStringW(void) ;
extern WINMM_API void mciSetDriverData(void) ;
extern WINMM_API void mciSetYieldProc(void) ;
extern WINMM_API void mid32Message(void) ;
extern WINMM_API void midiConnect(void) ;
extern WINMM_API void midiDisconnect(void) ;
extern WINMM_API void midiInAddBuffer(void) ;
extern WINMM_API void midiInClose(void) ;
extern WINMM_API void midiInGetDevCapsA(void) ;
extern WINMM_API void midiInGetDevCapsW(void) ;
extern WINMM_API void midiInGetErrorTextA(void) ;
extern WINMM_API void midiInGetErrorTextW(void) ;
extern WINMM_API void midiInGetID(void) ;
extern WINMM_API void midiInGetNumDevs(void) ;
extern WINMM_API void midiInMessage(void) ;
extern WINMM_API void midiInOpen(void) ;
extern WINMM_API void midiInPrepareHeader(void) ;
extern WINMM_API void midiInReset(void) ;
extern WINMM_API void midiInStart(void) ;
extern WINMM_API void midiInStop(void) ;
extern WINMM_API void midiInUnprepareHeader(void) ;
extern WINMM_API void midiOutCacheDrumPatches(void) ;
extern WINMM_API void midiOutCachePatches(void) ;
extern WINMM_API void midiOutClose(void) ;
extern WINMM_API void midiOutGetDevCapsA(void) ;
extern WINMM_API void midiOutGetDevCapsW(void) ;
extern WINMM_API void midiOutGetErrorTextA(void) ;
extern WINMM_API void midiOutGetErrorTextW(void) ;
extern WINMM_API void midiOutGetID(void) ;
extern WINMM_API void midiOutGetNumDevs(void) ;
extern WINMM_API void midiOutGetVolume(void) ;
extern WINMM_API void midiOutLongMsg(void) ;
extern WINMM_API void midiOutMessage(void) ;
extern WINMM_API void midiOutOpen(void) ;
extern WINMM_API void midiOutPrepareHeader(void) ;
extern WINMM_API void midiOutReset(void) ;
extern WINMM_API void midiOutSetVolume(void) ;
extern WINMM_API void midiOutShortMsg(void) ;
extern WINMM_API void midiOutUnprepareHeader(void) ;
extern WINMM_API void midiStreamClose(void) ;
extern WINMM_API void midiStreamOpen(void) ;
extern WINMM_API void midiStreamOut(void) ;
extern WINMM_API void midiStreamPause(void) ;
extern WINMM_API void midiStreamPosition(void) ;
extern WINMM_API void midiStreamProperty(void) ;
extern WINMM_API void midiStreamRestart(void) ;
extern WINMM_API void midiStreamStop(void) ;
extern WINMM_API void mixerClose(void) ;
extern WINMM_API void mixerGetControlDetailsA(void) ;
extern WINMM_API void mixerGetControlDetailsW(void) ;
extern WINMM_API void mixerGetDevCapsA(void) ;
extern WINMM_API void mixerGetDevCapsW(void) ;
extern WINMM_API void mixerGetID(void) ;
extern WINMM_API void mixerGetLineControlsA(void) ;
extern WINMM_API void mixerGetLineControlsW(void) ;
extern WINMM_API void mixerGetLineInfoA(void) ;
extern WINMM_API void mixerGetLineInfoW(void) ;
extern WINMM_API void mixerGetNumDevs(void) ;
extern WINMM_API void mixerMessage(void) ;
extern WINMM_API void mixerOpen(void) ;
extern WINMM_API void mixerSetControlDetails(void) ;
extern WINMM_API void mmDrvInstall(void) ;
extern WINMM_API void mmGetCurrentTask(void) ;
extern WINMM_API void mmTaskBlock(void) ;
extern WINMM_API void mmTaskCreate(void) ;
extern WINMM_API void mmTaskSignal(void) ;
extern WINMM_API void mmTaskYield(void) ;
extern WINMM_API void mmioAdvance(void) ;
extern WINMM_API void mmioAscend(void) ;
extern WINMM_API void mmioClose(void) ;
extern WINMM_API void mmioCreateChunk(void) ;
extern WINMM_API void mmioDescend(void) ;
extern WINMM_API void mmioFlush(void) ;
extern WINMM_API void mmioGetInfo(void) ;
extern WINMM_API void mmioInstallIOProcA(void) ;
extern WINMM_API void mmioInstallIOProcW(void) ;
extern WINMM_API void mmioOpenA(void) ;
extern WINMM_API void mmioOpenW(void) ;
extern WINMM_API void mmioRead(void) ;
extern WINMM_API void mmioRenameA(void) ;
extern WINMM_API void mmioRenameW(void) ;
extern WINMM_API void mmioSeek(void) ;
extern WINMM_API void mmioSendMessage(void) ;
extern WINMM_API void mmioSetBuffer(void) ;
extern WINMM_API void mmioSetInfo(void) ;
extern WINMM_API void mmioStringToFOURCCA(void) ;
extern WINMM_API void mmioStringToFOURCCW(void) ;
extern WINMM_API void mmioWrite(void) ;
extern WINMM_API void mmsystemGetVersion(void) ;
extern WINMM_API void mod32Message(void) ;
extern WINMM_API void mxd32Message(void) ;
extern WINMM_API void sndPlaySoundA(void) ;
extern WINMM_API void sndPlaySoundW(void) ;
extern WINMM_API void tid32Message(void) ;
extern WINMM_API void timeBeginPeriod(void) ;
extern WINMM_API void timeEndPeriod(void) ;
extern WINMM_API void timeGetDevCaps(void) ;
extern WINMM_API void timeGetSystemTime(void) ;
extern WINMM_API void timeKillEvent(void) ;
extern WINMM_API void timeSetEvent(void) ;
extern WINMM_API void waveInGetDevCapsA(void) ;
extern WINMM_API void waveInGetDevCapsW(void) ;
extern WINMM_API void waveInGetErrorTextA(void) ;
extern WINMM_API void waveInGetErrorTextW(void) ;
extern WINMM_API void waveInGetID(void) ;
extern WINMM_API void waveInGetNumDevs(void) ;
extern WINMM_API void waveInGetPosition(void) ;
extern WINMM_API void waveInMessage(void) ;
extern WINMM_API void waveOutBreakLoop(void) ;
extern WINMM_API void waveOutClose(void) ;
extern WINMM_API void waveOutGetDevCapsA(void) ;
extern WINMM_API void waveOutGetDevCapsW(void) ;
extern WINMM_API void waveOutGetErrorTextA(void) ;
extern WINMM_API void waveOutGetErrorTextW(void) ;
extern WINMM_API void waveOutGetID(void) ;
extern WINMM_API void waveOutGetNumDevs(void) ;
extern WINMM_API void waveOutGetPitch(void) ;
extern WINMM_API void waveOutGetPlaybackRate(void) ;
extern WINMM_API void waveOutGetPosition(void) ;
extern WINMM_API void waveOutGetVolume(void) ;
extern WINMM_API void waveOutMessage(void) ;
extern WINMM_API void waveOutOpen(void) ;
extern WINMM_API void waveOutPause(void) ;
extern WINMM_API void waveOutPrepareHeader(void) ;
extern WINMM_API void waveOutReset(void) ;
extern WINMM_API void waveOutRestart(void) ;
extern WINMM_API void waveOutSetPitch(void) ;
extern WINMM_API void waveOutSetPlaybackRate(void) ;
extern WINMM_API void waveOutSetVolume(void) ;
extern WINMM_API void waveOutUnprepareHeader(void) ;
extern WINMM_API void waveOutWrite(void) ;
extern WINMM_API void wid32Message(void) ;
extern WINMM_API void winmmDbgOut(void) ;
extern WINMM_API void winmmSetDebugLevel(void) ;
extern WINMM_API void wod32Message(void) ;

extern WINMM_API void CloseDriver(void)  ;
extern WINMM_API void DefDriverProc(void)  ;
extern WINMM_API void DriverCallback(void)  ;
extern WINMM_API void DrvGetModuleHandle(void)  ;
extern WINMM_API void GetDriverModuleHandle(void)  ;
extern WINMM_API void MigrateAllDrivers(void)  ;
//extern WINMM_API void MigrateMidiUser(void)  ;
extern WINMM_API void MigrateSoundEvents(void)  ;
extern WINMM_API void NotifyCallbackData(void)  ;
extern WINMM_API void OpenDriver(void)  ;
extern WINMM_API void PlaySound(void)  ;
extern WINMM_API void PlaySoundA(void)  ;
extern WINMM_API void PlaySoundW(void)  ;
extern WINMM_API void SendDriverMessage(void)  ;
extern WINMM_API void WOW32DriverCallback(void)  ;
extern WINMM_API void WOW32ResolveMultiMediaHandle(void)  ;
extern WINMM_API void WOWAppExit(void)  ;
extern WINMM_API void WinmmLogoff(void)  ;
extern WINMM_API void WinmmLogon(void)  ;
extern WINMM_API void aux32Message(void)  ;
extern WINMM_API void auxGetDevCapsA(void)  ;
extern WINMM_API void auxGetDevCapsW(void)  ;
extern WINMM_API void auxGetNumDevs(void)  ;
extern WINMM_API void auxGetVolume(void)  ;
extern WINMM_API void auxOutMessage(void)  ;
extern WINMM_API void auxSetVolume(void)  ;
extern WINMM_API void joy32Message(void)  ;
extern WINMM_API void joyConfigChanged(void)  ;

extern WINMM_API void  gfxAddGfx(void);
extern WINMM_API void  gfxBatchChange(void);
extern WINMM_API void  gfxCreateGfxFactoriesList(void);
extern WINMM_API void  gfxCreateZoneFactoriesList(void) ;
extern WINMM_API void  gfxDestroyDeviceInterfaceList(void) ;
extern WINMM_API void  gfxEnumerateGfxs(void);
extern WINMM_API void  _gfxLogoff(void);
extern WINMM_API void  _gfxLogon(void) ;
extern WINMM_API void  gfxModifyGfx(void);
extern WINMM_API void  gfxOpenGfx(void) ;
extern WINMM_API void  gfxRemoveGfx(void) ;


extern WINMM_API UINT waveInAddBuffer(HWAVEIN, void*, UINT);
extern WINMM_API UINT waveInClose(HWAVEIN);
extern WINMM_API UINT waveInOpen(void*, UINT, void*, DWORD, DWORD, DWORD);
extern WINMM_API UINT waveInPrepareHeader(HWAVEIN, void*, UINT);
extern WINMM_API UINT waveInStart(HWAVEIN);
extern WINMM_API UINT waveInStop(HWAVEIN);
extern WINMM_API UINT waveInUnprepareHeader(HWAVEIN, void*, UINT);
extern WINMM_API UINT waveInReset(HWAVEIN) ;

//extern WINMM_API UINT joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji);
///extern WINMM_API void joyGetDevCapsA(void);
//extern WINMM_API UINT  joyGetDevCapsA(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc);
//extern WINMM_API UINT joyGetNumDevs(void);

BOOL LoadWinmm(int line);
extern void GetPointerToOriginalFunc(void);


//---------------------------------------------------------------------------
static HWAVEIN       waveIn;         // WAVE IN
static HWAVEOUT      waveOut;        // WAVE OUT
static WAVEFORMATEX  waveFmt;		 // WAVE FORMAT (IN)
static WAVEFORMATEX  waveFmtO;		 // WAVE FORMAT (OUT)
static WAVEHDR      *waveBuf[2];      // WAVEHDR�\���̂ւ̃|�C���^
static volatile BOOL waveRecording;
static const int     waveBufSize = 1024;
static int Position[6];
//HINSTANCE hWinmm;
FAR HMODULE hWinmm = 0;
DWORD TlsIndex;
BOOL DoStartPropo;
char path[1024];
static	struct SharedDataBlock * DataBlock;
PP ProcessPulse;
int gDebugLevel;
FILE * gCtrlLogFile, * gDataLogFile;
far void  ** ListProcessPulseFunc;
static int console_started;
//---------------------------------------------------------------------------

extern void StartPropo(void);
extern void StopPropo(void);
