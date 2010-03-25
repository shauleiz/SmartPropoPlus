/*==========================================================================
 *
 *  Private version of:
 *
 *  mmsystem.h -- Include file for Multimedia API's
 *
 *  Version 4.00
 *
 *  Copyright 1992 - 1998 Microsoft Corporation.  All Rights Reserved.
 *
 *==========================================================================*/
#ifndef _INC_MMSYSTEM
#define _INC_MMSYSTEM   /* #defined if mmsystem.h has been included */

#define MAXPNAMELEN      32     /* max product name length (including NULL) */
#define MAXERRORLENGTH   256    /* max error text length (including NULL) */
#define MAX_JOYSTICKOEMVXDNAME 260 /* max oem vxd name length (including NULL) */
#define JOYCAPS_HASZ 		0x0001
#define JOYCAPS_HASR 		0x0002
#define JOYCAPS_HASU 		0x0004
#define JOYCAPS_HASV		0x0008
#define JOYERR_NOERROR        (0)                  /* no error */
#define JOYERR_BASE            160
#define JOYERR_UNPLUGGED      (JOYERR_BASE+7)      /* joystick is unplugged */
#define WAVERR_BASE            32
#define WAVERR_STILLPLAYING   (WAVERR_BASE + 1)    /* still something playing */
#define WIM_DATA              0x3C0
#define WAVE_FORMAT_PCM     1
#define WAVE_MAPPER     ((UINT)-1)
#define CALLBACK_FUNCTION   0x00030000l    /* dwCallback is a FARPROC */
#define CALLBACK_NULL       0x00000000l    /* no callback */
#define MIXER_SHORT_NAME_CHARS   16
#define MIXER_LONG_NAME_CHARS    64

#define MIXER_GETLINEINFOF_DESTINATION      0x00000000L
#define MIXER_GETLINEINFOF_SOURCE           0x00000001L
#define MIXER_GETLINEINFOF_LINEID           0x00000002L
#define MIXER_GETLINEINFOF_COMPONENTTYPE    0x00000003L
#define MIXER_GETLINEINFOF_TARGETTYPE       0x00000004L
#define MIXER_OBJECTF_WAVEOUT   0x10000000L

/* flags for dwFlags field of WAVEHDR */
#define WHDR_DONE       0x00000001  /* done bit */
#define WHDR_PREPARED   0x00000002  /* set if this header has been prepared */
#define WHDR_BEGINLOOP  0x00000004  /* loop start block */
#define WHDR_ENDLOOP    0x00000008  /* loop end block */
#define WHDR_INQUEUE    0x00000010  /* reserved for driver */


/* */
/*  MIXERLINE.dwComponentType */
/* */
/*  component types for destinations and sources */
/* */
/* */
#define MIXERLINE_COMPONENTTYPE_DST_FIRST       0x00000000L
#define MIXERLINE_COMPONENTTYPE_DST_UNDEFINED   (MIXERLINE_COMPONENTTYPE_DST_FIRST + 0)
#define MIXERLINE_COMPONENTTYPE_DST_DIGITAL     (MIXERLINE_COMPONENTTYPE_DST_FIRST + 1)
#define MIXERLINE_COMPONENTTYPE_DST_LINE        (MIXERLINE_COMPONENTTYPE_DST_FIRST + 2)
#define MIXERLINE_COMPONENTTYPE_DST_MONITOR     (MIXERLINE_COMPONENTTYPE_DST_FIRST + 3)
#define MIXERLINE_COMPONENTTYPE_DST_SPEAKERS    (MIXERLINE_COMPONENTTYPE_DST_FIRST + 4)
#define MIXERLINE_COMPONENTTYPE_DST_HEADPHONES  (MIXERLINE_COMPONENTTYPE_DST_FIRST + 5)
#define MIXERLINE_COMPONENTTYPE_DST_TELEPHONE   (MIXERLINE_COMPONENTTYPE_DST_FIRST + 6)
#define MIXERLINE_COMPONENTTYPE_DST_WAVEIN      (MIXERLINE_COMPONENTTYPE_DST_FIRST + 7)
#define MIXERLINE_COMPONENTTYPE_DST_VOICEIN     (MIXERLINE_COMPONENTTYPE_DST_FIRST + 8)
#define MIXERLINE_COMPONENTTYPE_DST_LAST        (MIXERLINE_COMPONENTTYPE_DST_FIRST + 8)

#define MIXERLINE_COMPONENTTYPE_SRC_FIRST       0x00001000L
#define MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 0)
#define MIXERLINE_COMPONENTTYPE_SRC_DIGITAL     (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 1)
#define MIXERLINE_COMPONENTTYPE_SRC_LINE        (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 2)
#define MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE  (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 3)
#define MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 4)
#define MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 5)
#define MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 6)
#define MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 7)
#define MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT     (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 8)
#define MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY   (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 9)
#define MIXERLINE_COMPONENTTYPE_SRC_ANALOG      (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 10)
#define MIXERLINE_COMPONENTTYPE_SRC_LAST        (MIXERLINE_COMPONENTTYPE_SRC_FIRST + 10)


#define MIXER_GETLINEINFOF_QUERYMASK        0x0000000FL

#define LOAD_IGNORE_CODE_AUTHZ_LEVEL		0x00000010


DECLARE_HANDLE(HWAVEIN);
DECLARE_HANDLE(HWAVEOUT);


#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
    WORD    wFormatTag;        /* format type */
    WORD    nChannels;         /* number of channels (i.e. mono, stereo...) */
    DWORD   nSamplesPerSec;    /* sample rate */
    DWORD   nAvgBytesPerSec;   /* for buffer estimation */
    WORD    nBlockAlign;       /* block size of data */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */
    WORD    cbSize;            /* The count in bytes of the size of
                                    extra information (after cbSize) */

} WAVEFORMATEX;
typedef WAVEFORMATEX       *PWAVEFORMATEX;
typedef WAVEFORMATEX NEAR *NPWAVEFORMATEX;
typedef WAVEFORMATEX FAR  *LPWAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

typedef struct wavehdr_tag {
    LPSTR       lpData;                 /* pointer to locked data buffer */
    DWORD       dwBufferLength;         /* length of data buffer */
    DWORD       dwBytesRecorded;        /* used for input only */
    DWORD       dwUser;                 /* for client's use */
    DWORD       dwFlags;                /* assorted flags (see defines) */
    DWORD       dwLoops;                /* loop control counter */
    struct wavehdr_tag FAR *lpNext;     /* reserved for driver */
    DWORD       reserved;               /* reserved for driver */
} WAVEHDR;

typedef struct { 
    WORD      wMid; 
    WORD      wPid; 
    UINT	 vDriverVersion; 
    TCHAR     szPname[MAXPNAMELEN]; 
    DWORD     dwFormats; 
    WORD      wChannels; 
    WORD      wReserved1; 
} WAVEINCAPS; 

typedef UINT        MMVERSION;  /* major (high byte), minor (low byte) */
typedef struct { 
    WORD      wMid; 
    WORD      wPid; 
    MMVERSION vDriverVersion; 
    TCHAR     szPname[MAXPNAMELEN]; 
    DWORD     dwFormats; 
    WORD      wChannels; 
    WORD      wReserved1; 
    DWORD     dwSupport; 
} WAVEOUTCAPS; 

typedef struct { 
    WORD    wMid; 
    WORD    wPid; 
    MMVERSION vDriverVersion; 
    CHAR    szPname[MAXPNAMELEN]; 
    DWORD   fdwSupport; 
    DWORD   cDestinations; 
} MIXERCAPS;

typedef struct { 
    DWORD cbStruct; 
    DWORD dwDestination; 
    DWORD dwSource; 
    DWORD dwLineID; 
    DWORD fdwLine; 
    DWORD dwUser; 
    DWORD dwComponentType; 
    DWORD cChannels; 
    DWORD cConnections; 
    DWORD cControls; 
    CHAR  szShortName[MIXER_SHORT_NAME_CHARS]; 
    CHAR  szName[MIXER_LONG_NAME_CHARS]; 
    struct { 
        DWORD     dwType; 
        DWORD     dwDeviceID; 
        WORD      wMid; 
        WORD      wPid; 
        MMVERSION vDriverVersion; 
        CHAR      szPname[MAXPNAMELEN]; 
    } Target; 
} MIXERLINE; 
 


typedef struct {
    WORD    wMid;                /* manufacturer ID */
    WORD    wPid;                /* product ID */
    CHAR    szPname[MAXPNAMELEN];/* product name (NULL terminated string) */
    UINT    wXmin;               /* minimum x position value */
    UINT    wXmax;               /* maximum x position value */
    UINT    wYmin;               /* minimum y position value */
    UINT    wYmax;               /* maximum y position value */
    UINT    wZmin;               /* minimum z position value */
    UINT    wZmax;               /* maximum z position value */
    UINT    wNumButtons;         /* number of buttons */
    UINT    wPeriodMin;          /* minimum message period when captured */
    UINT    wPeriodMax;          /* maximum message period when captured */
    UINT    wRmin;               /* minimum r position value */
    UINT    wRmax;               /* maximum r position value */
    UINT    wUmin;               /* minimum u (5th axis) position value */
    UINT    wUmax;               /* maximum u (5th axis) position value */
    UINT    wVmin;               /* minimum v (6th axis) position value */
    UINT    wVmax;               /* maximum v (6th axis) position value */
    UINT    wCaps;	 	 /* joystick capabilites */
    UINT    wMaxAxes;	 	 /* maximum number of axes supported */
    UINT    wNumAxes;	 	 /* number of axes in use */
    UINT    wMaxButtons;	 /* maximum number of buttons supported */
    CHAR    szRegKey[MAXPNAMELEN];/* registry key */
    CHAR    szOEMVxD[MAX_JOYSTICKOEMVXDNAME]; /* OEM VxD in use */
} *LPJOYCAPS;


typedef struct {
    UINT wXpos;                 /* x position */
    UINT wYpos;                 /* y position */
    UINT wZpos;                 /* z position */
    UINT wButtons;              /* button states */
} *LPJOYINFO;

typedef struct {
    DWORD dwSize;		 /* size of structure */
    DWORD dwFlags;		 /* flags to indicate what to return */
    DWORD dwXpos;                /* x position */
    DWORD dwYpos;                /* y position */
    DWORD dwZpos;                /* z position */
    DWORD dwRpos;		 /* rudder/4th axis position */
    DWORD dwUpos;		 /* 5th axis position */
    DWORD dwVpos;		 /* 6th axis position */
    DWORD dwButtons;             /* button states */
    DWORD dwButtonNumber;        /* current button number pressed */
    DWORD dwPOV;                 /* point of view state */
    DWORD dwReserved1;		 /* reserved for communication between winmm & driver */
    DWORD dwReserved2;		 /* reserved for future expansion */
} *FAR LPJOYINFOEX;

typedef HWAVEIN FAR *LPHWAVIN;
typedef HWAVEIN FAR *LPHWAVOUT;
typedef const WAVEFORMATEX FAR *LPCWAVEFORMATEX;

/* Original functions Functions */
    FARPROC pCloseDriver;
    FARPROC pDefDriverProc ;
    FARPROC pDriverCallback ;
    FARPROC pDrvGetModuleHandle ;
    FARPROC pGetDriverModuleHandle ;
    FARPROC pMigrateAllDrivers ;
    FARPROC pMigrateMidiUser ;
    FARPROC pMigrateSoundEvents ;
    FARPROC pNotifyCallbackData ;
    FARPROC pOpenDriver ;
    FARPROC pPlaySound ;
    FARPROC pPlaySoundA ;
    FARPROC pPlaySoundW ;
    FARPROC pSendDriverMessage ;
    FARPROC pWOW32DriverCallback ;
    FARPROC pWOW32ResolveMultiMediaHandle ;
    FARPROC pWOWAppExit ;
    FARPROC pWinmmLogoff ;
    FARPROC pWinmmLogon ;
    FARPROC paux32Message ;
    FARPROC pauxGetDevCapsA ;
    FARPROC pauxGetDevCapsW ;
    FARPROC pauxGetNumDevs ;
    FARPROC pauxGetVolume ;
    FARPROC pauxOutMessage ;
    FARPROC pauxSetVolume ;
    FARPROC pjoy32Message ;
    FARPROC pjoyConfigChanged ;
    FARPROC pjoyGetDevCapsA ;
    FARPROC pjoyGetDevCapsW ;
    FARPROC pjoyGetNumDevs ;
    FARPROC pjoyGetPos ;
    FARPROC pjoyGetThreshold ;
    FARPROC pjoyReleaseCapture ;
    FARPROC pjoySetCapture ;
    FARPROC pjoySetThreshold ;
    FARPROC pmci32Message ;
    FARPROC pmciDriverNotify ;
    FARPROC pmciDriverYield ;
    FARPROC pmciExecute ;
    FARPROC pmciFreeCommandResource ;
    FARPROC pmciGetCreatorTask ;
    FARPROC pmciGetDeviceIDA ;
    FARPROC pmciGetDeviceIDFromElementIDA ;
    FARPROC pmciGetDeviceIDFromElementIDW ;
    FARPROC pmciGetDeviceIDW ;
    FARPROC pmciGetDriverData ;
    FARPROC pmciGetErrorStringA ;
    FARPROC pmciGetErrorStringW ;
    FARPROC pmciGetYieldProc ;
    FARPROC pmciLoadCommandResource ;
    FARPROC pmciSendCommandA ;
    FARPROC pmciSendCommandW ;
    FARPROC pmciSendStringA ;
    FARPROC pmciSendStringW ;
    FARPROC pmciSetDriverData ;
    FARPROC pmciSetYieldProc ;
    FARPROC pmid32Message ;
    FARPROC pmidiConnect ;
    FARPROC pmidiDisconnect ;
    FARPROC pmidiInAddBuffer ;
    FARPROC pmidiInClose ;
    FARPROC pmidiInGetDevCapsA ;
    FARPROC pmidiInGetDevCapsW ;
    FARPROC pmidiInGetErrorTextA ;
    FARPROC pmidiInGetErrorTextW ;
    FARPROC pmidiInGetID ;
    FARPROC pmidiInGetNumDevs ;
    FARPROC pmidiInMessage ;
    FARPROC pmidiInOpen ;
    FARPROC pmidiInPrepareHeader ;
    FARPROC pmidiInReset ;
    FARPROC pmidiInStart ;
    FARPROC pmidiInStop ;
    FARPROC pmidiInUnprepareHeader ;
    FARPROC pmidiOutCacheDrumPatches ;
    FARPROC pmidiOutCachePatches ;
    FARPROC pmidiOutClose ;
    FARPROC pmidiOutGetDevCapsA ;
    FARPROC pmidiOutGetDevCapsW ;
    FARPROC pmidiOutGetErrorTextA ;
    FARPROC pmidiOutGetErrorTextW ;
    FARPROC pmidiOutGetID ;
    FARPROC pmidiOutGetNumDevs ;
    FARPROC pmidiOutGetVolume ;
    FARPROC pmidiOutLongMsg ;
    FARPROC pmidiOutMessage ;
    FARPROC pmidiOutOpen ;
    FARPROC pmidiOutPrepareHeader ;
    FARPROC pmidiOutReset ;
    FARPROC pmidiOutSetVolume ;
    FARPROC pmidiOutShortMsg ;
    FARPROC pmidiOutUnprepareHeader ;
    FARPROC pmidiStreamClose ;
    FARPROC pmidiStreamOpen ;
    FARPROC pmidiStreamOut ;
    FARPROC pmidiStreamPause ;
    FARPROC pmidiStreamPosition ;
    FARPROC pmidiStreamProperty ;
    FARPROC pmidiStreamRestart ;
    FARPROC pmidiStreamStop ;
    FARPROC pmixerClose ;
	FARPROC pmixerGetNumDevs ;
    FARPROC pmixerGetControlDetailsA ;
    FARPROC pmixerGetControlDetailsW ;
    FARPROC pmixerGetDevCapsA ;
    FARPROC pmixerGetDevCapsW ;
    FARPROC pmixerGetID ;
    FARPROC pmixerGetLineControlsA ;
    FARPROC pmixerGetLineControlsW ;
    FARPROC pmixerGetLineInfoA ;
    FARPROC pmixerGetLineInfoW ;
    FARPROC pmixerMessage ;
    FARPROC pmixerOpen ;
    FARPROC pmixerSetControlDetails ;
    FARPROC pmmDrvInstall ;
    FARPROC pmmGetCurrentTask ;
    FARPROC pmmTaskBlock ;
    FARPROC pmmTaskCreate ;
    FARPROC pmmTaskSignal ;
    FARPROC pmmTaskYield ;
    FARPROC pmmioAdvance ;
    FARPROC pmmioAscend ;
    FARPROC pmmioClose ;
    FARPROC pmmioCreateChunk ;
    FARPROC pmmioDescend ;
    FARPROC pmmioFlush ;
    FARPROC pmmioGetInfo ;
    FARPROC pmmioInstallIOProcA ;
    FARPROC pmmioInstallIOProcW ;
    FARPROC pmmioOpenA ;
    FARPROC pmmioOpenW ;
    FARPROC pmmioRead ;
    FARPROC pmmioRenameA ;
    FARPROC pmmioRenameW ;
    FARPROC pmmioSeek ;
    FARPROC pmmioSendMessage ;
    FARPROC pmmioSetBuffer ;
    FARPROC pmmioSetInfo ;
    FARPROC pmmioStringToFOURCCA ;
    FARPROC pmmioStringToFOURCCW ;
    FARPROC pmmioWrite ;
    FARPROC pmmsystemGetVersion ;
    FARPROC pmod32Message ;
    FARPROC pmxd32Message ;
    FARPROC psndPlaySoundA ;
    FARPROC psndPlaySoundW ;
    FARPROC ptid32Message ;
    FARPROC ptimeBeginPeriod ;
    FARPROC ptimeEndPeriod ;
    FARPROC ptimeGetDevCaps ;
    FARPROC ptimeGetSystemTime ;
    FARPROC ptimeGetTime ;
    FARPROC ptimeKillEvent ;
    FARPROC ptimeSetEvent ;
    FARPROC pwaveInGetDevCapsA ;
    FARPROC pwaveInGetDevCapsW ;
    FARPROC pwaveInGetErrorTextA ;
    FARPROC pwaveInGetErrorTextW ;
    FARPROC pwaveInGetID ;
    FARPROC pwaveInGetNumDevs ;
    FARPROC pwaveInGetPosition ;
    FARPROC pwaveInMessage ;

    FARPROC pwaveOutBreakLoop ;
    FARPROC pwaveOutClose ;
    FARPROC pwaveOutGetDevCapsA ;
    FARPROC pwaveOutGetDevCapsW ;
    FARPROC pwaveOutGetErrorTextA ;
    FARPROC pwaveOutGetErrorTextW ;
    FARPROC pwaveOutGetID ;
    FARPROC pwaveOutGetNumDevs ;
    FARPROC pwaveOutGetPitch ;
    FARPROC pwaveOutGetPlaybackRate ;
    FARPROC pwaveOutGetPosition ;
    FARPROC pwaveOutGetVolume ;
    FARPROC pwaveOutMessage ;
    FARPROC pwaveOutOpen ;
    FARPROC pwaveOutPause ;
    FARPROC pwaveOutPrepareHeader ;
    FARPROC pwaveOutReset ;
    FARPROC pwaveOutRestart ;
    FARPROC pwaveOutSetPitch ;
    FARPROC pwaveOutSetPlaybackRate ;
    FARPROC pwaveOutSetVolume ;
    FARPROC pwaveOutUnprepareHeader ;
    FARPROC pwaveOutWrite ;
    FARPROC pwid32Message ;
    FARPROC pwinmmDbgOut ;
    FARPROC pwinmmSetDebugLevel ;
    FARPROC pwod32Message ;
    FARPROC pwaveInReset ;

    FARPROC ppogoff ;
    FARPROC ppogon ;

    FARPROC pgfxAddGfx;
    FARPROC pgfxBatchChange;
    FARPROC pgfxCreateGfxFactoriesList;
    FARPROC pgfxCreateZoneFactoriesList;
    FARPROC pgfxDestroyDeviceInterfaceList;
    FARPROC pgfxEnumerateGfxs;
    FARPROC p_gfxLogoff;
    FARPROC p_gfxLogon;
    FARPROC pgfxModifyGfx;
    FARPROC pgfxOpenGfx;
    FARPROC pgfxRemoveGfx;

//	FARPROC pFunctionNumberTwo;


    UINT ( WINAPI *pwaveInAddBuffer)(HWAVEIN, void*, UINT);
    UINT ( WINAPI *pwaveInClose)(HWAVEIN);
    UINT ( WINAPI *pwaveInOpen)(LPHWAVIN, UINT, LPCWAVEFORMATEX, DWORD, DWORD, DWORD);
    UINT ( WINAPI *pwaveInPrepareHeader)(HWAVEIN, void*, UINT);
    UINT ( WINAPI *pwaveInStart)(HWAVEIN);
    UINT ( WINAPI *pwaveInStop)(HWAVEIN) ;
    UINT ( WINAPI *pwaveInUnprepareHeader)(HWAVEIN, void*, UINT) ;

	UINT ( WINAPI *pjoyGetPosEx)(UINT, LPJOYINFOEX) ;
//	UINT ( WINAPI *pjoyGetPos)(UINT, LPJOYINFO) ;


#endif