// Data types

#ifndef __GENWAVE_H
#define __GENWAVE_H
#include <mmsystem.h>

// Number of audio buffers
#define	N_WAVEIN_BUF	8	/* 3.3.5 Vista compatability */
#define	SIZE_WAVEIN_BUF 1024 

/************************ Constants ************************/
// Shift constants
#define I_AUT_SHIFT	1
#define I_POS_SHIFT	2
#define I_NEG_SHIFT	3

//// Modulation types
//#define MOD_TYPE_PPM	"PPM"
//#define MOD_TYPE_JR 	"JR"
//#define MOD_TYPE_FUT	"FUT"
//#define MOD_TYPE_AIR1	"AIR1"
//#define MOD_TYPE_AIR2	"AIR2"
//#define MOD_TYPE_WAL	"WAL"
//

// Interface parameters
#define MOD			1000	// Dodulation
#define SHIFT		1001	// Shift type
#define JITTER		1002	// Anti-jitter
#define DEVICE		1003	// Audio Device
#define INPUT		1004	// Audio Input
#define MIXER		1005	// Mixer selector
#define WAVSTART	1006	// Start Wave file logging
#define WAVSTOP		1007	// Stop Wave file logging
#define WAVPLAY		1008	// Play Wave file
#define WAVSTAT		1009	// Status of logger
#define AUDIOLEVEL	1010	// Monitoring: Audio Level

#define NSAMPLES	1011	// WaveIn overrides: Number of samples per second
#define NBITS		1012	// WaveIn overrides: Number of bits per samples
#define NBUFS		1013	// WaveIn overrides: Number of buffers
#define NBUFSIZES	1014	// WaveIn overrides: Size of buffers

#define AHSTART		1016	// Start Audio Header logging
#define AHSTOP		1017	// Stop Audio Header logging
#define AHSTAT		1019	// Status of Audio Header logger

#define PLSSTART	1020	// Start Pulse logging
#define PLSSTOP		1021	// Stop Pulse logging
#define PLSSTAT		1023	// Status of Pulse logger

#define RPLSSTART	1024	// Start Raw Pulse logging
#define RPLSSTOP	1025	// Stop Raw Pulse logging
#define RPLSSTAT	1026	// Status of Raw Pulse logger

#define NJOYPOS		1027	// Number of joystick positions
#define JOYPOS		1028	// Joystick positions (Pre-Filter)
#define POSTFLTR	1029	// Joystick positions (Post-Filter)

// Modulation-specific constants
#define PW_FUTABA	6.623
#define PW_JR		7.340
#define PPM_MIN		30.0
#define PPM_MAX		80.0
#define PPM_TRIG	200
#define PPM_SEP		15.0

// Signal processing constants
#define MAX_JS_CH	12

// Wave file Recorder constants 
#define PLAYING		101
#define STOPPED		102
// Window message
//#define	WAVEINTHREADPROC	"SPP - Joystick data from WaveIn thread"
//static	UINT NEAR			WM_WAVEINTHREADPROC;

/************************ ENUM Declarations ************************/ 
enum MODE {
	UnknownMode = -1,
	PpmAuto,
	PpmNeg,
	PpmPos,
	PcmJr,
	PcmFut,
	PcmAir1,
	PcmAir2,
	PcmWalkera
};


#endif // __GENWAVE_H

// Class declarations
class CWaveIn;
class CWave2Joystick;
class CPpjoyIf;
