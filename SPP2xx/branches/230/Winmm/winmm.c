// pcpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "math.h"
#include "mmsystem.h"
#include "jr.h"
#include "futaba.h"
#include "airtronics.h"
#include "stdio.h"
#include <stdlib.h> 
#include <malloc.h>
#include <time.h>
#include "SmartPropoPlus.h"
#include "GlobalMemory.h"
#include "winmm.h"

#define MESSAGE(msg,type) Message(msg, type,  __FILE__, __LINE__)
#define	INITWINMM	if (!hWinmm) {LoadWinmm( __LINE__); TlsSetValue(TlsIndex, hWinmm); StartPropo();};

/** Helper functions **/

/**  Get the Debug level from the registry  - Default is 0 **/
int _GetDebugLevel()
{
	LONG res;
	HKEY  hkSpp;
	unsigned long	ValueDataSize;
	unsigned int i=0, index=0;
	int iActive = -1;
	char Active[MAX_VAL_NAME] = "";
	int  level;

	

	/* Open SPP  key for data query */
	res = RegOpenKeyEx(HKEY_CURRENT_USER,REG_SPP, 0, KEY_QUERY_VALUE , &hkSpp);
	if (res != ERROR_SUCCESS)
		return 0;

	/* Get Shift data */	
	ValueDataSize = 4;
	res = RegQueryValueEx(hkSpp, DEBUG_LEVEL,  NULL, NULL, (unsigned char *)&level,  &ValueDataSize);
	if (res != ERROR_SUCCESS)
		return 0;

	return level;
}

void _DebugWelcomePopUp(struct Modulations *  Modulation)
{
	char msg[4000];
	char CtrlLogFileName[255];
	char DataLogFileName[255];
   char dbuffer [9];
   char tbuffer [9];

	if (!gDebugLevel)
		return;

	sprintf(msg, "SmartPropoPlus Version %x\nDebug level %d",VER_DLL,gDebugLevel);

	if (Modulation)
		sprintf(msg, "%s\nModulation: %s", msg, Modulation->ModulationList[Modulation->Active]->ModTypeInternal);

	if (Modulation)
	{
		if (Modulation->ShiftAutoDetect)
			sprintf(msg, "%s\tAuto-Detect", msg);
		else
		{
			if (Modulation->PositiveShift)
				sprintf(msg, "%s\tPositive Shift", msg);
			else
				sprintf(msg, "%s\tNegative Shift", msg);
		};
	}

	if (gDebugLevel>1 && getenv("TEMP"))
	{
		/* Create a Control-flow log file */
		sprintf(CtrlLogFileName,"%s\\%s", getenv("TEMP"), CTRL_LOG_FILE);
		gCtrlLogFile = fopen(CtrlLogFileName, "w");
		if (gCtrlLogFile)
		{
			fprintf(gCtrlLogFile, "%s - %s\n%s\n\n%s\n\n", _strdate( dbuffer ), _strtime( tbuffer ), msg, SEPARATOR);
			sprintf(msg, "%s\n\nControl-Flow logfile: %s", msg,CtrlLogFileName);
		}
		else
		{
			sprintf(msg, "%s\n\nCannot open logfile: %s", msg,CtrlLogFileName);
			gDebugLevel=1;
		}
	};

	if (gDebugLevel>2 && getenv("TEMP"))
	{
		/* Create a Data-flow log file */
		sprintf(DataLogFileName,"%s\\%s", getenv("TEMP"), DATA_LOG_FILE);
		gDataLogFile = fopen(DataLogFileName, "w");
		if (gDataLogFile)
		{
			fprintf(gDataLogFile, "%s - %s\n%s\n\n%s\n\n", _strdate( dbuffer ), _strtime( tbuffer ), msg, SEPARATOR);
			sprintf(msg, "%s\n\nData-Flow logfile: %s", msg,DataLogFileName);
		}		
		else
		{
			sprintf(msg, "%s\n\nCannot open logfile: %s", msg,CtrlLogFileName);
			gDebugLevel=1;
		}

	};

	MessageBox(NULL,msg, "SmartPropoPlus Debug Message" , MB_SYSTEMMODAL);
}

__inline void _DebugChangeModPopUp(struct SharedDataBlock * dBlock)
{
	char msg[4000];
	static int mod=-1, detect=-1, pos=-1;

	if (!gDebugLevel || !dBlock)
		return;


	if (mod==dBlock->ActiveModulation.iModType && detect==dBlock->ActiveModulation.AutoDetectModShift && pos==dBlock->ActiveModulation.ActiveModShift)
		return;

	sprintf(msg, "Former Modulation:\tMod=%d; Auto=%d; Pos=%d", mod, detect, pos);
	mod=dBlock->ActiveModulation.iModType;
	detect=dBlock->ActiveModulation.AutoDetectModShift;
	pos=dBlock->ActiveModulation.ActiveModShift;
	sprintf(msg, "%s\nNew Modulation:\tMod=%d; Auto=%d; Pos=%d", msg, mod, detect, pos);

	MessageBox(NULL,msg, "SmartPropoPlus Debug Message" , MB_SYSTEMMODAL);
}

__inline  void _DebugProcessData(int i, double min, double max, double threshold, int pulse, int polarity)
{
	char tbuffer [9];

	if (gDebugLevel>=3 && gDataLogFile)
	{
		if (polarity)
			fprintf(gDataLogFile,"\n%s - ProcessData(%7d) (H); Min/Max/Thresh=%d/%d/%d P-Width=%3d", 
			_strtime( tbuffer ), i,
			(int)min, (int)max, (int)threshold,
			pulse);
		else
			fprintf(gDataLogFile,"\n%s - ProcessData(%7d) (L); Min/Max/Thresh=%d/%d/%d P-Width=%3d", 
			_strtime( tbuffer ), i,
			(int)min, (int)max, (int)threshold,
			pulse);
	};

}

__inline  void _DebugJoyStickData()
{

	if (gDebugLevel>=3 && gDataLogFile)
	{
			fprintf(gDataLogFile,"\tJoyStick channels: Ch[1:6]: %4d %4d %4d %4d %4d %4d", Position[0], Position[1], Position[2], Position[3], Position[4], Position[5]);
	};

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

/***********************************************************************************************/
int WINAPI  DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{

    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			TlsIndex = TlsAlloc();
			DoStartPropo = TRUE;
			break;

        case DLL_THREAD_ATTACH:
			DoStartPropo = FALSE;
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
	Smooth values so that abrapt changes of value will be smoothed
	And illegal values will be ignored
 */
__inline  int  smooth(int orig, int newval)
{
	if (newval<0)
		return orig;

	if ((orig-newval > 100) || (newval-orig > 100))
		return (newval+orig)/2;

	return newval;

}


//#ifdef AIR_PCM1
/* Helper function - Airtronic/Sanwa PCM1 data convertor */
static int  __fastcall Convert15bits(unsigned int in)
{
	int quintet[3];

	/* Convert the upper 5-bits to value 0-3 */
	quintet[2] = air1_msb[((in>>10)&0x1F)];
	if (quintet[2]<0)
		return -1;

	/* Convert the mid 5-bits to value 0-15 */
	quintet[1] = air1_symbol[((in>>5)&0x1F)];
	if (quintet[1]<0)
		return -1;

	/* Convert the low 5-bits to value 0-15 */
	quintet[0] = air1_symbol[(in&0x1F)];
	if (quintet[0]<0)
		return -1;

	/* Return the calculated (inverted) channel value of 0-1023 */
	return 1023-(quintet[2]*256+quintet[1]*16+quintet[0]);

}

/*
	Process Airtronics/Sanwa PCM1 pulse (10-bit)
	
	Pulse width may last 1 to 4 bits. Every bit consists an avarege of 9 samples
	This is why the width is devided by 8.

	The data consists of 8 channels in two chunks:
	First Chunk begins with a 4-bit low pulse, second Chunk begins with a 4-bit high pulse
	Every channel data starts at the 10th bit of a chunk and consists of 15bit word. No delimiter between channel data

	Channel data is converted using function "Convert15bits()" (and inverted) into a joystick posision.
*/
static PP ProcessPulseAirPcm1(int width, BOOL input)
{
	int pulse;
	static int sync;
	static int datacount = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
	static chunk = -1;
	unsigned int		shift;
    static unsigned int data[10];
	char tbuffer [9];
	static int i = 0;

	if (gDebugLevel>=2 && gCtrlLogFile && i++%10)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseAirPcm1(%d)", _strtime( tbuffer ), width);

		pulse = width/8; // Width to bits
		if (pulse == 4)  // 4-bit pulse marks a biginind of a data chunk
		{
			if (!input)
			{
				// First data chunk - clear chunnel counter
				datacount = 0;
			}
			else
			{	// Second data chunk - get joystick position from channel data
				Position[0] = smooth(Position[0], Convert15bits(data[1]));
				Position[1] = smooth(Position[1], Convert15bits(data[2]));
				Position[2] = smooth(Position[2], Convert15bits(data[3]));
				Position[3] = smooth(Position[3], Convert15bits(data[4]));
				Position[4] = smooth(Position[4], Convert15bits(data[6]));
				Position[5] = smooth(Position[5], Convert15bits(data[7]));
			};
			sync = 1;		// Sync bit is set for the first 10 bits of the chunk (No channel data here)
			bitstream = 0;
			bitcount = -1;
			chunk = input;	// Mark chunk polarity - 0: Low channels, 1: High channels
			//return 0;
		};
		
		if (sync) 
		{
			shift = 9;	// Read the first 10 bits
		}
		else
		{
			shift =15; // Read a channel data
		};

		bitstream = ((bitstream << 1) + 1) << (pulse - 1);
		bitcount += pulse;
		
		if (bitcount >=shift) 
		{
			bitcount -= shift;
			data[datacount] = (bitstream >> bitcount) & 0x7FFF; // Put raw 15-bit channel data
			datacount++;
			sync = 0;
			if (datacount>=8)
				datacount = 0;
		};
		return 0;
}

//#elif defined AIR_PCM2
/* Helper function - Airtronic/Sanwa PCM2 data convertor */
static int  __fastcall Convert20bits(int in)
{
	int quartet[5];
	int value;

	quartet[4] = air2_symbol[((in>>16)&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[3] = air2_symbol[((in>>12)&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[2] = air2_symbol[((in>>8 )&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[1] = air2_symbol[((in>>4 )&0xF)];
	if (quartet[4]<0)
		return -1;

	quartet[0] = air2_symbol[((in>>0 )&0xC)];
	if (quartet[4]<0)
		return -1;

	value = quartet[4] + (quartet[3]<<2) + (quartet[2]<<4) + (quartet[1]<<6) + (quartet[0]<<8);
	return 1023-2*value;
}

/*
	Process Airtronics/Sanwa PCM2 pulse (9-bit)
	
	Pulse width may last 1 to 7 bits. Every bit consists an avarege of 13 samples
	This is why the width is devided by 12.

	The data consists of 6 channels in two chunks:
	First Chunk begins with a 7-bit high pulse, second Chunk begins with a 7-bit low pulse
	Every channel data starts at the 9th bit of a chunk and consists of 20bit word. No delimiter between channel data

	Channel data is converted using function "Convert20bits()" (and inverted) into a joystick posision. 
*/
static PP ProcessPulseAirPcm2(int width, BOOL input)
{
	int pulse;
	static int sync;
	static int datacount = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
	static chunk = -1;
	unsigned int		shift;
    static unsigned int data[10];
	char tbuffer [9];
	static int i = 0;

	if (gDebugLevel>=2 && gCtrlLogFile && !(i++%50))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseAirPcm2(Width=%d, input=%d)", _strtime( tbuffer ), width, input);

		pulse = width/12; // Width to bits
		if (pulse == 7)  // 4-bit pulse marks a biginind of a data chunk
		{
			if (!input)
			{
				// First data chunk - clear chunnel counter
				datacount = 0;
			}
			else
			{	// Second data chunk - get joystick position from channel data
				Position[0] = smooth(Position[0], Convert20bits(data[2]));
				Position[1] = smooth(Position[1], Convert20bits(data[7]));
				Position[2] = smooth(Position[2], Convert20bits(data[6]));
				Position[3] = smooth(Position[3], Convert20bits(data[3]));
				Position[4] = smooth(Position[4], Convert20bits(data[1]));
				Position[5] = smooth(Position[5], Convert20bits(data[5]));
			};
			sync = 1;		// Sync bit is set for the first 10 bits of the chunk (No channel data here)
			bitstream = 0;
			bitcount = -1;
			chunk = input;	// Mark chunk polarity - 0: Low channels, 1: High channels
			//return 0;
		};
		
		if (sync) 
		{
			shift = 7;	// Read the first 10 bits
		}
		else
		{
			shift =20; // Read a channel data
		};

		bitstream = ((bitstream << 1) + 1) << (pulse - 1);
		bitcount += pulse;
		
		if (bitcount >=shift) 
		{
			bitcount -= shift;
			data[datacount] = (bitstream >> bitcount) & 0xFFFFF; // Put raw 20-bit channel data
			datacount++;
			sync = 0;
			if (datacount>=8)
				datacount = 0;
		};

		return 0;
}



//#elif defined JR_PCM
static PP ProcessPulseJrPcm(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
    static int data[30];
    static int datacount = 0;
	char tbuffer [9];
	static int i = 0;

	if (gDebugLevel>=2 && gCtrlLogFile && !(i++%50))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseJrPcm(%d)", _strtime( tbuffer ), width);


    if (sync == 0 && (int)floor(2.0 * width / PW_JR + 0.5) == 5) {
        sync = 1;
        bitstream = 0;
        bitcount = -1;
        datacount = 0;
        return 0;
    }

    if (!sync) return 0;

    width = (int)floor((double)width / PW_JR + 0.5);
    bitstream = ((bitstream << 1) + 1) << (width - 1);
    bitcount += width;

    if (bitcount >= 8) {
        bitcount -= 8;
        if ((data[datacount++] = jr_symbol[(bitstream >> bitcount) & 0xFF]) < 0) {
            sync = 0;
            return 0;
        }
    }

    switch (datacount) {
        case 3:  Position[2] = 1023 - ((data[1] << 5) | data[2]); break;
        case 6:  Position[0] = 1023 - ((data[4] << 5) | data[5]); break;
        case 11: Position[5] = 1023 - ((data[9] << 5) | data[10]); break;
        case 14: break;
        case 18: Position[3] = 1023 - ((data[16] << 5) | data[17]); break;
        case 21: Position[1] = 1023 - ((data[19] << 5) | data[20]); break;
        case 26: Position[4] = 1023 - ((data[24] << 5) | data[25]); break;
        case 28: break;
        case 30: sync = 0;
    };

	 return 0;
}


//---------------------------------------------------------------------------

//#elif defined FUTABA_PCM
static PP ProcessPulseFutabaPcm(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bit = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;

    static int data[32];
    static int datacount = 0;
	char tbuffer [9];
	static int i = 0;

	if (gDebugLevel>=2 && gCtrlLogFile && !(i++%50))
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseFutabaPcm(%d)", _strtime( tbuffer ), width);

    width = (int)floor(width / PW_FUTABA + 0.5);

    if (sync == 0 && width == 18) {
        sync = 1;
        bit = 0;
        bitstream = 0;
        bitcount = 0;
        datacount = 0;
        return 0;
    }

    if (!sync) return 0;

    bitstream = (bitstream << width) | (bit >> (32 - width));
    bit ^= 0xFFFFFFFF;
    bitcount += width;

    if (sync == 1) {
        if (bitcount >= 6) {
            bitcount -= 6;
            if (((bitstream >> bitcount) & 0x3F) == 0x03) {
                sync = 2;
                datacount = 0;
            } else if (((bitstream >> bitcount) & 0x3F) == 0x00) {
                sync = 2;
                datacount = 16;
                bitcount -= 2;
            } else {
                sync = 0;
            }
        }
        return 0;
    }

    if (bitcount >= 10) {
        bitcount -= 10;
        if ((data[datacount++] = futaba_symbol[(bitstream >> bitcount) & 0x3FF]) < 0) {
            sync = 0;
            return 0;
        }
    }

    switch (datacount) {
        case 3:  if ((data[0] >> 4) != 0)  Position[2] = (data[1] << 4) | (data[2] >> 2); break;
        case 7:                            Position[3] = (data[5] << 4) | (data[6] >> 2); break;
        case 11: if ((data[0] >> 4) != 0)  Position[4] = (data[9] << 4) | (data[10] >> 2); break;
        case 15: sync = 0;
        case 19:                           Position[1] = (data[17] << 4) | (data[18] >> 2); break;
        case 23: if ((data[16] >> 4) != 1) Position[0] = (data[21] << 4) | (data[22] >> 2); break;
        case 27:                           Position[5] = (data[25] << 4) | (data[26] >> 2); break;
        case 31: break;
        case 32: sync = 0;
    };

	return 0;
}
//---------------------------------------------------------------------------


//#else PPM
/*
	Process PPM pulse
	A long (over 200 samples) leading high, then a short low, then up to six highs followed by short lows.
	The size of a high pulse may vary between 30 to 70 samples, mapped to joystick values of 1024 to 438
	where the mid-point of 50 samples is translated to joystick position of 731.
*/
static PP ProcessPulsePpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	char tbuffer [9];
	static int i = 0;

	if (width < 5)
		return 0;

	if (gDebugLevel>=2 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulsePpm(width=%d, input=%d)", _strtime( tbuffer ), width, input);

	/* If pulse is a separator then go to the next one */
	if (width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return 0;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (sync == 0 && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
		former_sync = 1;
		return 0;
    }

    if (!sync) 
		return 0; /* still waiting for sync */


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

	
	if (input)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1] = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2] = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0] = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3] = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4] = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5] = data[datacount];	break;/* Assign data to joystick channels */
	};

	if (datacount == 5)	sync = 0;			/* Reset sync after channel 6 */

    datacount++;
	return 0;
}
//#endif

static PP ProcessPulseFutabaPpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	char tbuffer [9];
	static int i = 0;


	if (width < 5)
		return 0;

	if (gDebugLevel>=2 /*&& gCtrlLogFile && !(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseFutabaPpm(width=%d, input=%d)", _strtime( tbuffer ), width, input);

	/* If pulse is a separator then go to the next one */
	if (!input || width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return 0;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (input && sync == 0 && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
		former_sync = 1;
		return 0;
    }

    if (!sync) return 0; /* still waiting for sync */


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

	
	if (input)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1] = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2] = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0] = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3] = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4] = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5] = data[datacount];	break;/* Assign data to joystick channels */
	};

	if (datacount == 5)	sync = 0;			/* Reset sync after channel 6 */

    datacount++;
	return 0;
}

static PP ProcessPulseJrPpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	char tbuffer [9];
	static int i = 0;


	if (width < 5)
		return 0;

	if (gDebugLevel>=2 /*&& gCtrlLogFile && !(i++%50)*/)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseJrPpm(width=%d, input=%d)", _strtime( tbuffer ), width, input);

	/* If pulse is a separator then go to the next one */
	if (input || width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return 0;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (!input && sync == 0 && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
		former_sync = 1;
		return 0;
    }

    if (!sync) return 0; /* still waiting for sync */


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

	
	if (input)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1] = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2] = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0] = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3] = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4] = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5] = data[datacount];	break;/* Assign data to joystick channels */
	};

	if (datacount == 5)	sync = 0;			/* Reset sync after channel 6 */

    datacount++;
	return 0;
}

/*
	Create a list of pointers to functions (ProcessPulseXXX)
	The order is acording to the order of the modulation types in the Global Memory.
	The number of entries is 3 times the number of modulations + 1
	The first set will consist of the default (autodetect) version of the functions
	The second set will consist of the negative-shift version of the functions
	The third set will consist of the positive-shift version of the functions
	The addional input is a final NULL entry
*/
int LoadProcessPulseFunctions(struct SharedDataBlock * target)
{
	int index=0;
	char tmp[MAX_VAL_NAME];
	int nMod;

	nMod = target->nModulations;
	ListProcessPulseFunc = (far void **)calloc(nMod+1, sizeof(far void *));

	/* Loop on list of modulation types */
	while(index < nMod && index<MAX_MODS)
	{
		/* Get the internal name of the modulation */
		strcpy(tmp, (char *)target->pInternalModName[index]);

		if (!strcmp(tmp, MOD_TYPE_PPM))
		{	
			ListProcessPulseFunc[0*nMod+index] = (void *)ProcessPulsePpm;		/* Auto  Detect*/
			ListProcessPulseFunc[1*nMod+index] = (void *)ProcessPulseFutabaPpm;	/* Negative  Detect*/
			ListProcessPulseFunc[2*nMod+index] = (void *)ProcessPulseJrPpm;		/* Positive  Detect*/
		}
		else if (!strcmp(tmp, MOD_TYPE_JR))
		{
			ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseJrPcm;
			ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseJrPcm;
			ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseJrPcm;
		}
		else if (!strcmp(tmp, MOD_TYPE_FUT))
		{
			ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseFutabaPcm;
			ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseFutabaPcm;
			ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseFutabaPcm;
		}
		else if (!strcmp(tmp, MOD_TYPE_AIR1))
		{
			ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseAirPcm1;
			ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseAirPcm1;
			ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseAirPcm1;
		}
		else if (!strcmp(tmp, MOD_TYPE_AIR2))
		{
			ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseAirPcm2;
			ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseAirPcm2;
			ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseAirPcm2;
		}
		index++;
	};


	ListProcessPulseFunc[2*nMod+index] = (void **)NULL;
	return index;
}

__inline void SetActiveProcessPulseFunction(struct SharedDataBlock * dBlock)
{
	int set;

	if (dBlock->ActiveModulation.AutoDetectModShift)
		set = 0;
	else if  (!dBlock->ActiveModulation.ActiveModShift)
		set = 1;
	else
		set = 2;

	ProcessPulse = ListProcessPulseFunc[dBlock->nModulations*set + dBlock->ActiveModulation.iModType];
	if (!ProcessPulse)
		ProcessPulse = (PP)ProcessPulsePpm;

	_DebugChangeModPopUp(dBlock);
}

/* 
	Start the SppConsole GUI application if it exists
*/
int StartSppConsole()
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	int errorcode;
	static UINT NEAR WM_INTERSPPAPPS;
	
	
	/* Mark this DLL as running */
	if (!OpenMutex(MUTEX_ALL_ACCESS, TRUE, MUTXWINMM))
		CreateMutex(NULL, FALSE, MUTXWINMM);
	
	/* Start the executable */
	ZeroMemory(&StartupInfo, sizeof(STARTUPINFO));
	StartupInfo.cb = sizeof(STARTUPINFO);
	if (!CreateProcess(NULL,"SppConsole.exe -i", NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		errorcode = GetLastError();
		return 0;
	}
	else
	{
		DWORD rc = WaitForSingleObject(ProcessInformation.hProcess, 1000); // 1 second time out
		//out =  ProcessInformation.hProcess;
		
	};
	
	
	/* Send starting message to the GUI */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTARTING, 0);
	return 1;
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
	static int FormerInput;

	/* Initialization of the min/max vaues */
    max -= 0.1;
    min += 0.1;
    if (max < min) max = min + 1;

    if (i> max) max = i;			/* Update max value */
    else if (i < min) min = i;		/* Update min value */
    threshold = (min + max) / 2;	/* Update mid-point value */


	SetActiveProcessPulseFunction(DataBlock);


	/* Update the width of the number of the low/high samples */
	/* If edge, then call ProcessPulse() to process the previous high/low level */
    if (i > threshold) 
	{
	high++;
        if (low) 
		{
			_DebugProcessData(i,min, max, threshold, low,0);
            ProcessPulse(low, FALSE);
			 _DebugJoyStickData();
            low = 0;
        }
    } else 
	{
        low++;
        if (high) 
		{
			_DebugProcessData(i,min, max, threshold, high,1);
            ProcessPulse(high, TRUE);
			 _DebugJoyStickData();
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
	struct Modulations *  Modulation;
	int nActiveModulations;

	if (!DoStartPropo)
		return;

	/* Download configuration from the registry (if exists) */
	Modulation= GetModulation(0);

	/* Create the Global memory area (a.k.a. Shared Data) and upload configuration to it */
	DataBlock =  CreateDataBlock(Modulation);
	nActiveModulations = LoadProcessPulseFunctions(DataBlock);
	/* SetActiveProcessPulseFunction(DataBlock); */

	/* Get Debug level from the registry (if exists) and start debugging */
	gDebugLevel = _GetDebugLevel();
	_DebugWelcomePopUp(Modulation);


    waveRecording = TRUE; /* Start recording */

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
	char tbuffer [9];
	static UINT NEAR WM_INTERSPPAPPS;
	
    waveRecording = FALSE;
	if (waveIn)
	{
		pwaveInStop(waveIn);
		for (i = 0; i < 2 ;i++) {
			pwaveInUnprepareHeader(waveIn, waveBuf[i], sizeof(WAVEHDR));
			HeapFree(GetProcessHeap(), 0, waveBuf[i]->lpData);
			HeapFree(GetProcessHeap(), 0, waveBuf[i]);
		}
		pwaveInClose(waveIn);
	};
	
	if (gDebugLevel>=2 && gCtrlLogFile)
	{
		fprintf(gCtrlLogFile,"\n%s - Closing WINMM.DLL\n\n", _strtime( tbuffer ));
		fclose(gCtrlLogFile);
	};

	/* TODO - Release global memory by UnMapping File View */

	/* Release Mutex */
	ReleaseMutex(MUTXWINMM);

	/* Send closing message to the GUI */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	if (console_started)
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTOPPING, 0);
}

//---------------------------------------------------------------------------
int GetPosition(int ch)
{
    return Position[ch];
}
//---------------------------------------------------------------------------


// Exported function.
extern __declspec(dllexport) UINT __stdcall   joyGetDevCapsA(UINT uJoyID, LPJOYCAPS pjc, UINT cbjc) 
{

	/* 
		If the GUI console has not been started yet by the simulator then start it
		Assumption - FMS (and any other simulator) will use this function but the GUI will not
		Not assumed - the GUI console is not active yet.
	*/
	if (!console_started)
		console_started = StartSppConsole();

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

/*

*/
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

    pgfxAddGfx = GetProcAddress(hWinmm, "gfxAddGfx");
    pgfxBatchChange = GetProcAddress(hWinmm, "gfxBatchChange");
    pgfxCreateGfxFactoriesList = GetProcAddress(hWinmm, "gfxCreateGfxFactoriesLis");
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
void NAKED joyGetPos(void) {   INITWINMM __asm jmp dword ptr   pjoyGetPos}

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
