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
#define MIXER_OBJECTF_HANDLE    0x80000000L
#define MIXER_OBJECTF_MIXER     0x00000000L
#define MIXER_OBJECTF_HMIXER    (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_MIXER)
#define MIXER_OBJECTF_WAVEOUT   0x10000000L
#define MIXER_OBJECTF_HWAVEOUT  (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_WAVEOUT)
#define MIXER_OBJECTF_WAVEIN    0x20000000L
#define MIXER_OBJECTF_HWAVEIN   (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_WAVEIN)
#define MIXER_OBJECTF_MIDIOUT   0x30000000L
#define MIXER_OBJECTF_HMIDIOUT  (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_MIDIOUT)
#define MIXER_OBJECTF_MIDIIN    0x40000000L
#define MIXER_OBJECTF_HMIDIIN   (MIXER_OBJECTF_HANDLE|MIXER_OBJECTF_MIDIIN)
#define MIXER_OBJECTF_AUX       0x50000000L

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

/****************************************************************************

                String resource number bases (internal use)

****************************************************************************/
typedef UINT        MMRESULT;

#define MMSYSERR_BASE          0
#define WAVERR_BASE            32
#define MIDIERR_BASE           64
#define TIMERR_BASE            96
#define JOYERR_BASE            160
#define MCIERR_BASE            256
#define MIXERR_BASE            1024

/****************************************************************************

                        General error return values

****************************************************************************/

/* general error return values */
#define MMSYSERR_NOERROR      0                    /* no error */
#define MMSYSERR_ERROR        (MMSYSERR_BASE + 1)  /* unspecified error */
#define MMSYSERR_BADDEVICEID  (MMSYSERR_BASE + 2)  /* device ID out of range */
#define MMSYSERR_NOTENABLED   (MMSYSERR_BASE + 3)  /* driver failed enable */
#define MMSYSERR_ALLOCATED    (MMSYSERR_BASE + 4)  /* device already allocated */
#define MMSYSERR_INVALHANDLE  (MMSYSERR_BASE + 5)  /* device handle is invalid */
#define MMSYSERR_NODRIVER     (MMSYSERR_BASE + 6)  /* no device driver present */
#define MMSYSERR_NOMEM        (MMSYSERR_BASE + 7)  /* memory allocation error */
#define MMSYSERR_NOTSUPPORTED (MMSYSERR_BASE + 8)  /* function isn't supported */
#define MMSYSERR_BADERRNUM    (MMSYSERR_BASE + 9)  /* error value out of range */
#define MMSYSERR_INVALFLAG    (MMSYSERR_BASE + 10) /* invalid flag passed */
#define MMSYSERR_INVALPARAM   (MMSYSERR_BASE + 11) /* invalid parameter passed */
#define MMSYSERR_HANDLEBUSY   (MMSYSERR_BASE + 12) /* handle being used */
                                                   /* simultaneously on another */
                                                   /* thread (eg callback) */
#define MMSYSERR_INVALIDALIAS (MMSYSERR_BASE + 13) /* specified alias not found */
#define MMSYSERR_BADDB        (MMSYSERR_BASE + 14) /* bad registry database */
#define MMSYSERR_KEYNOTFOUND  (MMSYSERR_BASE + 15) /* registry key not found */
#define MMSYSERR_READERROR    (MMSYSERR_BASE + 16) /* registry read error */
#define MMSYSERR_WRITEERROR   (MMSYSERR_BASE + 17) /* registry write error */
#define MMSYSERR_DELETEERROR  (MMSYSERR_BASE + 18) /* registry delete error */
#define MMSYSERR_VALNOTFOUND  (MMSYSERR_BASE + 19) /* registry value not found */
#define MMSYSERR_NODRIVERCB   (MMSYSERR_BASE + 20) /* driver does not call DriverCallback */
#define MMSYSERR_MOREDATA     (MMSYSERR_BASE + 21) /* more data to be returned */
#define MMSYSERR_LASTERROR    (MMSYSERR_BASE + 21) /* last error in range */
/****************************************************************************

                        Waveform audio support

****************************************************************************/

/* waveform audio error return values */
#define WAVERR_BADFORMAT      (WAVERR_BASE + 0)    /* unsupported wave format */
#define WAVERR_STILLPLAYING   (WAVERR_BASE + 1)    /* still something playing */
#define WAVERR_UNPREPARED     (WAVERR_BASE + 2)    /* header not prepared */
#define WAVERR_SYNC           (WAVERR_BASE + 3)    /* device is synchronous */
#define WAVERR_LASTERROR      (WAVERR_BASE + 3)    /* last error in range */

DECLARE_HANDLE(HMIXER);
DECLARE_HANDLE(HWAVEIN);
DECLARE_HANDLE(HWAVEOUT);


typedef struct {
    WORD        wFormatTag;         /* format type */
    WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
    DWORD       nSamplesPerSec;     /* sample rate */
    DWORD       nAvgBytesPerSec;    /* for buffer estimation */
    WORD        nBlockAlign;        /* block size of data */
    WORD        wBitsPerSample;     /* number of bits per sample of mono data */
    WORD        cbSize;             /* the count in bytes of the size of */
				    /* extra information (after cbSize) */
} WAVEFORMATEX;

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