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
#define PPMW_MIN	18.0
#define PPMW_MAX	70.0
#define PPMW_TRIG	200
#define PPMW_SEP	15.0

#define	N_WAVEIN_BUF	64

#define MAX_JS_CH	12
#ifdef _DEBUG
#define BASEPRODUCT	  "SPP3.3.7"
#else
#define BASEPRODUCT	  "SmartPropoPlus"
#endif
#define PRODUCT BASEPRODUCT

#define MFTIMES_PER_MILLISEC  10000
#define EXIT_ON_ERROR(hres) if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->lpVtbl->Release(punk); (punk) = NULL; }

#define MUTEX_STOP_START	"WaveIn Stopping and Starting are mutually exclusive"

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

struct WAVEINSTRUCT 
{
	//BOOL			active;
	UINT			id;
	HWAVEIN			hWaveInDev;
	WAVEFORMATEX	waveFmt;
	WAVEHDR			*waveBuf[N_WAVEIN_BUF];
	volatile BOOL	waveRecording;
};

struct WAVEINSTRUCT *	WaveInInfo;
struct WAVEINSTRUCT *	CurrentWaveInInfo = NULL;
int						iCurrentWaveInInfo = -1;


//static HWAVEIN       waveIn=0;       // WAVE IN
HWAVEIN * hWaveInDev;

static HWAVEOUT      waveOut;        // WAVE OUT
//static WAVEFORMATEX  waveFmt;		 // WAVE FORMAT (IN)
static WAVEFORMATEX  waveFmtO;		 // WAVE FORMAT (OUT)
static WAVEHDR      *waveBuf[N_WAVEIN_BUF];      // WAVEHDR�\���̂ւ̃|�C���^
static volatile BOOL waveRecording;
static const int     waveBufSize = 1024;
static int Position[MAX_JS_CH];
//HINSTANCE hWinmm;
FAR HMODULE hWinmm = 0;
DWORD TlsIndex;
//BOOL DoStartPropo;
char path[1024];
static	struct SharedDataBlock * DataBlock;
PP ProcessPulse;
int gDebugLevel;
FILE * gCtrlLogFile, * gDataLogFile, * gChnlLogFile;
far void  ** ListProcessPulseFunc;
static int console_started;
int VistaOS;
static volatile BOOL closeRequest;
DWORD dwThreadId;
HANDLE hThread;
HANDLE hMutexStartStop;
//static UINT NEAR WM_INTERDLL;


//---------------------------------------------------------------------------

extern void StartPropo(void);
extern void StopPropo(void);
extern void ExitPropo(void);

DWORD WINAPI ProcThread(void *param);
DWORD WINAPI  ChangeStreaming(const char * DevName);
DWORD WINAPI  StartStreaming(const char * DevName);
DWORD WINAPI StopStreaming(void * pDummy);
void ReportChange(void);

int		OpenAllStreams();
HWAVEIN	OpenStream(struct WAVEINSTRUCT * wi);

BOOL PropoStarted(void);

//-------------- JsChPostProc.dll interface ---------------------------------

FAR HMODULE hJschpostproc = 0;
HINSTANCE LoadJsChPostProc();
int GetPointerToJsChPostProcOriginalFunc(HINSTANCE);
JS_CHANNELS * js_data = NULL;
int RunJsFilter(int * ch, int nChannels);
int GetJsChPostProcInfo(FAR HMODULE  hJschpostproc);
int JsChPostProc_selected = -1;
__inline void SetActiveJsChPostProcFunction(struct SharedDataBlock * dBlock);

//------------------------ WASAPI -----------------------------------------------
	//BCDE0395-E52F-467C-8E3D-C4579291692E
	static GUID const CLSID_MMDeviceEnumerator = {0xBCDE0395, 0xE52F, 0x467C, {0x8E,0x3D,0xC4,0x57,0x92,0x91,0x69,0x2E} };

	 //{A95664D2-9614-4F35-A746-DE8DB63617E6}
	static GUID const CLSID_IMMDeviceEnumerator = {0xA95664D2, 0x9614, 0x4F35, {0xA7,0x46,0xDE,0x8D,0xB6,0x36,0x17,0xE6} };

	 //{1CB9AD4C-DBFA-4C32-B178-C2F568A703B2}
	static GUID const IID_IAudioClient = {0x1CB9AD4C, 0xDBFA, 0x4C32, {0xB1,0x78,0xC2,0xF5,0x68,0xA7,0x03,0xB2} };

	//{C8ADBD64-E71E-48A0-A4DE-185C395CD317}
	static GUID const IID_IAudioCaptureClient = {0xC8ADBD64, 0xE71E, 0x48A0, {0xA4,0xDE,0x18,0x5C,0x39,0x5C,0xD3,0x17} };

int OpenAllStreamsW7(void);
HRESULT  StartStreamingW7(const char * DevName);
DWORD ChangeStreamingW7(void);
int GetIndexOfDevice(const char * DevName);
HRESULT InitAllEndPoints();
const char * GetFriendlyName(IMMDevice * pDev);
HRESULT GetWaveFormat(IAudioClient * pClient, WAVEFORMATEX ** pFmt);
DWORD WINAPI CaptureAudioW7(void *);
void StartListening(void);
DWORD WINAPI ListenToGui(void *);

struct WAVEINSTRUCT_W7
{
	BOOL				Usable;
	IMMDevice			*pDeviceIn;
	LPWSTR				DevId;
	IAudioClient		*pClientIn;
	char const			*DevFriendlyName;
	WAVEFORMATEX		*WaveFmt;
	IAudioCaptureClient *pCaptureClient;
};

IMMDeviceEnumerator *pEnumerator = NULL;
IMMDeviceCollection *pDeviceCollect = NULL;
struct WAVEINSTRUCT_W7 *	WaveInInfoW7;
struct WAVEINSTRUCT_W7 *	CurrentWaveInInfoW7 = NULL;
int							iCurrentWaveInInfoW7 = -1;
int							count = 0;
HANDLE						hBufferReady = 0;
HANDLE						hThreadListen = 0;
HANDLE						hCaptureAudioThread = 0;
