/* Header file for PPJoy extension */
#ifndef __PPJOYEX_H
#define __PPJOYEX_H

#include "shlwapi.h"
#include <winioctl.h>

#define	CHANNELS	16


/* Define to use byte-size values for joystick axes, else dword size */
#undef UCHAR_AXES

#define	PPJOY_AXIS_MIN				1
#ifdef UCHAR_AXES
#define	PPJOY_AXIS_MAX				127
#define	PPJOY_AXIS_STEP				12
#else
#define	PPJOY_AXIS_MAX				32767
#define	PPJOY_AXIS_STEP				3000
#endif

#define FILE_DEVICE_PPORTJOY			FILE_DEVICE_UNKNOWN

#define PPORTJOY_IOCTL(_index_)	\
	CTL_CODE (FILE_DEVICE_PPORTJOY, _index_, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_PPORTJOY_SET_STATE		PPORTJOY_IOCTL (0x0)

#define	JOYSTICK_STATE_V1	0x53544143

typedef struct
{
 ULONG	Version;
 UCHAR	Data[1];
} JOYSTICK_SET_STATE, *PJOYSTICK_SET_STATE;

#define	NUM_ANALOG	8		/* Number of analog values which we will provide */
#define	NUM_DIGITAL	16		/* Number of digital values which we will provide */

#pragma pack(push,1)		/* All fields in structure must be byte aligned. */
struct JOYSTICK_STATE
{
	unsigned long	Signature;				/* Signature to identify packet to PPJoy IOCTL */
	char			NumAnalog;				/* Num of analog values we pass */
	long			Analog[CHANNELS];		/* Analog values */
	char			NumDigital;				/* Num of digital values we pass */
	char			Digital[NUM_DIGITAL];	/* Digital values */
} ;

#pragma pack(pop)


/*HANDLE				hJoy=NULL;
char				ch;
DWORD				RetSize;
DWORD				rc;
long				*Analog;
char				*Digital;
char				*DevName;
int					PpjActive=0;*/


#define PPJoyIOCTL \
	{\
		"\\\\.\\PPJoyIOCTL1",\
		"\\\\.\\PPJoyIOCTL1", "\\\\.\\PPJoyIOCTL2", "\\\\.\\PPJoyIOCTL3", "\\\\.\\PPJoyIOCTL4",\
		"\\\\.\\PPJoyIOCTL5", "\\\\.\\PPJoyIOCTL6", "\\\\.\\PPJoyIOCTL7", "\\\\.\\PPJoyIOCTL8",\
		"\\\\.\\PPJoyIOCTL9", "\\\\.\\PPJoyIOCTL10", "\\\\.\\PPJoyIOCTL11", "\\\\.\\PPJoyIOCTL12",\
		"\\\\.\\PPJoyIOCTL13", "\\\\.\\PPJoyIOCTL14", "\\\\.\\PPJoyIOCTL15", "\\\\.\\PPJoyIOCTL16",\
	};


void SendPPJoy(int nChannels, int *Channel);
int ConnectPPJoyDriver(int index);
int DisconnectPPJoyDriver(int index);
#endif //__PPJOYEX_H