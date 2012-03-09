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
#include <crtdbg.h>
#include "SmartPropoPlus.h"
#include "GlobalMemory.h"
#include "JsChPostProc.h"
#include "winmm.h"
#include <assert.h>
#ifdef PPJOY
#include <winioctl.h>
#include "ppjoyex.h"
#endif /* PPJOY */

#define MESSAGE(msg,type) Message(msg, type,  __FILE__, __LINE__)
#define	INITWINMM	if (!hWinmm) {LoadWinmm( __LINE__); TlsSetValue(TlsIndex, hWinmm); hJschpostproc = LoadJsChPostProc(); StartPropo();};

#define DEBUG_W2K "Version 5"
#undef DEBUG_W2K

/** Helper functions **/

/**  Get the Debug level from the registry  - Default is 0 **/
int _GetDebugLevel()
{
	unsigned int i=0, index=0;
	int iActive = -1;
	char Active[MAX_VAL_NAME] = "";
	int  level;
	char msg[4000];

	level =  GetDebugLevel();


	/* Get debug level */	
	if (level < 0)
	{	/* Version 3.3.1 */
		sprintf(msg, "Cannot open Registry value \"HKEY_CURRENT_USER\\%s\\%s\" ", REG_SPP, DEBUG_LEVEL);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
		return 0;
	}

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



/* Walkera helper functions */

/* Convert pulse width to binary value */
unsigned char  WalkeraConvert2Bin(int width)
{
	switch (width)
	{
		case 11:
		case 12:
		case 13:
		case 14: return 0;
		break;


		case 19:
		case 20:
		case 21: return 1;
		break;

		default:
			return 0;
	}
};

/* Convert pulse width to octal value */
unsigned char  WalkeraConvert2Oct(int width)
{
	switch (width)
	{
		case 11:
		case 12:
		case 13:
		case 14: return 0;
		break;

		case 17:
		case 18: return 1;
		break;

		case 19:
		case 20:
		case 21: return 2;
		break;

		case 24:
		case 25: return 3;
		break;

		case 27:
		case 28:
		case 29: return 4;
		break;

		case 30:
		case 31:
		case 32: return 5;
		break;

		case 34:
		case 35:
		case 36: return 6;
		break;

		case 38:
		case 39: return 7;
		break;

		default:
			return 8; /* Illegal value */
	};	

};

/*
	Data: cycle[1:5]
	cycle[1]:			0/1 for stick above/below middle point
	cycle[2]:			Octal - 0 at middle point (MSB)
	cycle[3]:			0/1
	cycle[4]:			Octal
	cycle[5]:			0/1
	MSBit of cycle[6]:	0/1
*/
int WalkeraElevator(const unsigned char * cycle)
{
	int value;

	value = cycle[2]*64+cycle[3]*32+cycle[4]*4+cycle[5]*2+(cycle[6]>>2);

	/* Mid-point is 511 */
	if(cycle[1])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};


/*
	Data: cycle[6:10]
	cycle[6]:	Bit[1]: 0/1 for stick Left/Right
	cycle[6]:	Bit[0]: (MSB)
	cycle[7]:	Binary - 0 at middle point
	cycle[8]:	Octal
	cycle[9]:	Binary
	cycle[10]:	Octal (LSB)
*/
int WalkeraAilerons(const unsigned char * cycle)
{
	int value, msb;

	msb = cycle[6]&1;

	/* Offset from mid-point */
	value = msb*256+cycle[7]*128+cycle[8]*16+cycle[9]*8+cycle[10];

	/* Mid-point is 511 */
	if(cycle[6]&2)
		value = 511-value; /* Left */
	else
		value = 511+value; /* Right */

	return value;
};

/*
	Data: cycle[11:15]
	cycle[11]:	0/1 for stick above/below middle point
	cycle[12]:	Octal - 0 at middle point (MSB)
	cycle[13]:	0/1
	cycle[14]:	Octal
	cycle[15]:	Binary (LSB)
*/
int WalkeraThrottle(const unsigned char * cycle)
{
	int value;

	value = cycle[12]*64+cycle[13]*32+cycle[14]*4+cycle[15]*2+((cycle[16]&4)>>2);

	/* Mid-point is 511 */
	if(cycle[11])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};


/*
	Data: cycle[16:21]
	cycle[16]:	Bit[1]: 0/1 for stick Left/Right
	cycle[16]:	Bit[0]: (MSB)
	cycle[17]:	Binary - 0 at middle point
	cycle[18]:	Octal
	cycle[19]:	Binary
	cycle[20]:	Octal (LSB)
*/
int WalkeraRudder(const unsigned char * cycle)
{
	int value,  msb;

	msb = cycle[16]&1;

	/* Offset from mid-point */
	value = msb*256+cycle[17]*128+cycle[18]*16+cycle[19]*8+cycle[20];

	/* Mid-point is 511 */
	if(cycle[16]&2)
		value = 511-value; /* Left */
	else
		value = 511+value; /* Right */

	return value;
};


/*
	Data: cycle[23]
*/
int WalkeraGear(const unsigned char * cycle)
{
	int value;

	value = cycle[24]*64+cycle[25]*32+cycle[26]*4+cycle[27]*2+((cycle[28]&4)>>2);

	/* Mid-point is 511 */
	if(cycle[23])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};



/*
	Data: cycle[29:33]
	cycle[28]:	Bit[1]: 0/1 for stick Left/Right
	cycle[28]:	Bit[0]: (MSB)
	cycle[29]:	Binary - 0 at middle point
	cycle[30]:	Octal
	cycle[31]:	Binary
	cycle[32]:	Octal (LSB)
*/
int WalkeraPitch(const unsigned char * cycle)
{
	int value,  msb;

	msb = cycle[28]&1;


	/* Offset from mid-point */
	value = msb*256+cycle[29]*128+cycle[30]*16+cycle[31]*8+cycle[32];

	/* Mid-point is 511 */
	if(cycle[28]&2)
		value = 511+value; /* Left */
	else
		value = 511-value; /* Right */

	return value;
};


int WalkeraGyro(const unsigned char * cycle)
{
	int value;

	value = cycle[34]*64+cycle[35]*32+cycle[36]*4+cycle[37]*2+((cycle[38]&4)>>2);

	/* Mid-point is 511 */
	if(cycle[33])
		value = 511-value; /* Below */
	else
		value = 511+value; /* Above */

	return value;
};

int WalkeraChannel8(const unsigned char * cycle)
{
	int value,  msb;

	msb = cycle[38]&1;


	/* Offset from mid-point */
	value = msb*256+cycle[39]*128+cycle[40]*16+cycle[41]*8+cycle[42];

	/* Mid-point is 511 */
	if(cycle[38]&2)
		value = 511+value; /* Left */
	else
		value = 511-value; /* Right */

	return value;
};
/*
	Calculates the four Checksum values:
	CS1,CS2: For channels 1-4
	CS3,CS4: For Channels 5-8
*/
int * WalkeraCheckSum(const unsigned char * cycle)
{
	int static cs[4]= {-1,-1,-1,-1};

	/* CS2 */
	cs[1] = cycle[2]+cycle[4]+cycle[6]+cycle[8]+cycle[10]+cycle[12]+cycle[14]+cycle[16]+cycle[18]+cycle[20];

	/* CS1 */
	cs[0] = cycle[1]+cycle[3]+cycle[5]+cycle[7]+cycle[9]+cycle[11]+cycle[13]+cycle[15]+cycle[17]+cycle[19]+(cs[1]>>3);

	/* CS4 */
	cs[3] = cycle[24]+cycle[26]+cycle[28]+cycle[30]+cycle[32]+cycle[34]+cycle[36]+cycle[38]+cycle[40]+cycle[42]+cycle[44]+cycle[46];

	/* CS3 */
	cs[2] = cycle[23]+cycle[25]+cycle[27]+cycle[29]+cycle[31]+cycle[33]+cycle[35]+cycle[37]+cycle[39]+cycle[41]+cycle[43]+cycle[45]+(cs[3]>>3);

	cs[0]&= 0x1;
	cs[1]&= 0x7;
	cs[2]&= 0x1;
	cs[3]&= 0x7;
	return cs;
}


/*
	Process Walkera PCM pulse (Tested with Walkera WK-0701)

	Major changes in version 3.3.1
	
	Pulse width may be from 11 to 58 samples.
	Pulse of over 56 samples is considered to be a sync pulse.
	A sync pulse is always followed by 49 data pulses.

	Data pulses are of two types:
	Binary pulses and Octal pulses.
	Binary pulses may be:
		0:	11-14 samples
		1:	19-21 samples
	Octal pulses may be:
		0:	11-14 samples
		1:	17-18 samples
		3:	19-21 samples
		4:	24-25 samples
		5:	27-28 samples
		6:	30-32 samples
		7:	34-35 samples
		8:	38-39 samples
*/
static PP ProcessPulseWalPcm(int width, BOOL input)
{
	static int nPulse;
	static unsigned char cycle[50];
	int Elevator=0, Ailerons=0,Throttle=0,Rudder=0,Gear=0, Pitch=0, Gyro=0, Ch8=0;
	const int fixed_n_channel = 8;
	int vPulse;
	int * cs;


	/* Detect Sync pulse - if detected then reset pulse counter and return */
	if (width>56)
	{
		nPulse = 1;
		return 0;
	};

	if (width < 11)
		return 0;

	/* Even pulses are binary, Odd pulses are Octal */
	if ((nPulse&1))
		vPulse = WalkeraConvert2Bin(width);
	else
		vPulse = WalkeraConvert2Oct(width);
	if (vPulse<8)
		cycle[nPulse] = vPulse;

	nPulse++;

	/* At the end of the 50-pulse cycle - calculate the channels */
	if (nPulse==50)
	{
		/* Channels */
		Elevator	= WalkeraElevator(cycle);	/* Ch1: Elevator */		
		Ailerons	= WalkeraAilerons(cycle);	/* Ch2: Ailerons */		
		Throttle	= WalkeraThrottle(cycle);	/* Ch3: Throttle */		
		Rudder		= WalkeraRudder(cycle);		/* Ch4: Rudder   */		
		Gear		= WalkeraGear(cycle);		/* Ch5: Gear     */
		Pitch		= WalkeraPitch(cycle);		/* Ch6: Pitch    */		
		Gyro		= WalkeraGyro(cycle);		/* Ch7: Gyro     */	/* version 3.3.1 */
		Ch8			= WalkeraChannel8(cycle);	/* Ch8: Not used */ /* version 3.3.1 */

		/* Checksum */
		cs = WalkeraCheckSum(cycle); /* version 3.3.1 */

		/* Copy data to joystick positions if checksum is valid (ch1-ch4) */
		if ((cs[0] == cycle[21]) && (cs[1] == cycle[22]))
		{
			Position[0] = smooth(Position[0], Elevator);
			Position[1] = smooth(Position[1], Ailerons);
			Position[2] = smooth(Position[2], Throttle);
			Position[3] = smooth(Position[3], Rudder);
		};

		/* Copy data to joystick positions if checksum is valid (ch5-ch8) */
		if ((cs[2] == cycle[47]) && (cs[3] == cycle[48]))
		{
			Position[4] = smooth(Position[4], Gear);
			Position[5] = smooth(Position[5], Pitch);
			Position[6] = smooth(Position[6], Gyro);
			Position[7] = smooth(Position[7], Ch8);
		};
		nPulse = 0;

#ifdef PPJOY
				SendPPJoy(fixed_n_channel-1, Position);
#endif
	};
	return 0;
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
	const int fixed_n_channel = 8;

	if (gDebugLevel>=2 && gCtrlLogFile && i++%10)
		fprintf(gCtrlLogFile,"\n%s - ProcessPulseAirPcm1(%d)", _strtime( tbuffer ), width);

		pulse = width/8; // Width to bits
		if (pulse == 4)  // 4-bit pulse marks a bigining of a data chunk
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
				Position[6] = smooth(Position[6], Convert15bits(data[8]));
				Position[7] = smooth(Position[7], Convert15bits(data[9]));
#ifdef PPJOY
				SendPPJoy(fixed_n_channel-1, Position);
#endif
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
			if (datacount>=fixed_n_channel+2)
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
#ifdef PPJOY
				SendPPJoy(6, Position);
#endif
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
        case 14: Position[7] = 1023 - ((data[12] << 5) | data[13]); break;
        case 18: Position[3] = 1023 - ((data[16] << 5) | data[17]); break;
        case 21: Position[1] = 1023 - ((data[19] << 5) | data[20]); break;
        case 26: Position[4] = 1023 - ((data[24] << 5) | data[25]); break;
        case 29: Position[6] = 1023 - ((data[27] << 5) | data[28]); break;
        case 30: sync = 0;
#ifdef PPJOY
				SendPPJoy(8, Position);
#endif
    };

	 return 0;
}


//---------------------------------------------------------------------------

//#elif defined FUTABA_PCM
/*
	Futaba PCM1024Z format
	Based on the following documents:
		http://www.cg.its.tudelft.nl/~wouter/publications/pasman03k.pdf
		http://www.cg.its.tudelft.nl/~wouter/publications/pasman04b.pdf

	The data is incapsulated in alternating odd/even frames of 160 bits of RAW data
	Before every FRAME there is a 18-bit sync pulse that is followed by a PARITY MARKER.
	ODD parity marker is 00000011. EVEN parity marker is 000011.

	Every 10-bits of raw data are mapped into a 6-bit data SEXTET.
	Every 4 consequtive SEXTETS form a 24-bit PACKET.
	Every PACKET is built as follows:
	 packet[0:1]   - AUX:   Auxilary bits used to modify the meaning of the data
	 packet[2:5]   - DELTA: used to calculate position delta (not used here).
	 packet[6:15]  - POS:   Channel position data (10 bit data)
	 packet[16:23] - ECC:   Error correction code (not used here).

   The first 8 channels (Analog) are mapped like this:
	Ch1: POS[5] (Odd Frame)  Provided AUX[4] != 0x1
	Ch2: POS[4] (Odd Frame)  Provided AUX[4] != 0x1
	Ch3: POS[0] (Even Frame) Provided AUX[0] != 0x0
	Ch4: POS[1] (Even Frame) Provided AUX[0] != 0x0
	Ch5: POS[2] (Even Frame) Provided AUX[0] != 0x0
	Ch6: POS[6] (Odd Frame)  Provided AUX[4] != 0x1
	Ch7: POS[3] (Even Frame) Provided AUX[0] != 0x0
	Ch8: POS[7] (Odd Frame)  Provided AUX[4] != 0x1

   The next channels (binary)  are mapped like this:
    CH9:  
		AUX[3] lower bit = 0 Provided AUX[2] = 0x0
		AUX[11] lower bit = 0 Provided AUX[10] = 0x1
    CH10: 
		AUX[5] lower bit = 0 Provided AUX[4] = 0x0
		AUX[13] lower bit = 0 Provided AUX[12] = 0x1

*/
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


    width = (int)floor(width / PW_FUTABA + 0.5);

	/* 
		Sync is determined as 18-bit wide pulse 
		If detected, move state machine to sync=1 (wait for parity marker) 
		and initialize all static params
	*/
    if (sync == 0 && width == 18) {
        sync = 1;
        bit = 0;
        bitstream = 0;
        bitcount = 0;
        datacount = 0;		
			if (gDebugLevel>=2 && gCtrlLogFile /*&& !(i++%50)*/)
				fprintf(gCtrlLogFile,"\n%s - ProcessPulseFutabaPcm(%d) %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d - %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d ; %02d %02d %02d %02d", _strtime( tbuffer ), width,\
				data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],\
				data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15],\
				data[16], data[17], data[18], data[19], data[20], data[21], data[22], data[23],\
				data[24], data[25], data[26], data[27], data[28], data[29], data[30], data[31]\
				);
        return 0;
    }
	
    if (!sync) return 0;

    bitstream = (bitstream << width) | (bit >> (32 - width));
    bit ^= 0xFFFFFFFF;
    bitcount += width;

	/* 
		Parity marker must follow the sync pulse (sync==1)
		It might take one of two forms:
		- EVEN parity marker is 000011.
		- ODD parity marker is 00000011.
		If either is found mode state machine to sync=2 (read raw data)
	*/
    if (sync == 1) {
        if (bitcount >= 6) {
            bitcount -= 6;
            if (((bitstream >> bitcount) & 0x3F) == 0x03) { /* Even? */
                sync = 2;
                datacount = 0; /* Even: Reset sextet counter to read the first (even) 16 ones */
            } else if (((bitstream >> bitcount) & 0x3F) == 0x00) { /* Odd? */
                sync = 2;
                datacount = 16; /* Odd: Set sextet counter to 16  to read the last (odd) 16 ones */
                bitcount -= 2;
            } else {
                sync = 0;
            }
        }
        return 0;
    }

	/* 
		Read the next ten bits of raw data 
		Convert then into a sextet
		Increment sextet counter
		If data is illegal reset state machine to sync=0
	*/
    if (bitcount >= 10) {
        bitcount -= 10;
        if ((data[datacount++] = futaba_symbol[(bitstream >> bitcount) & 0x3FF]) < 0) {
            sync = 0;
            return 0;
        }
    }

	/* 
		Convert sextet data into channel (position) data
		Every channel is 10-bit, copied from POS in the corresponding packet.
		Every channel is calculated only after the corresponding packet is ready (forth sextet is ready).
	*/
    switch (datacount) {
		/* Even frame */
        case 3:  /* 4th sextet of first packet is ready */
			if ((data[0] >> 4) != 0)  
				Position[2] = (data[1] << 4) | (data[2] >> 2);    /* Ch3: Position: packet[6:15] */
			break;

        case 7:  /* 4th sextet of second packet is ready */                            
			if ((data[0] >> 4) != 0)  
				Position[3] = (data[5] << 4) | (data[6] >> 2);    /* Ch4: Position: packet[6:15] */
			Position[9] = ((data[20]>>4)&1)*512;  /* Ch 10: One of the auxilliary bits */
			break;

        case 11:  /* 4th sextet of 3rd packet is ready */ 
			if ((data[0] >> 4) != 0)  
				Position[4] = (data[9] << 4) | (data[10] >> 2);   /* Ch5: Position: packet[6:15] */
			break;

        case 15:   /* 4th sextet of 4th packet is ready */
			if ((data[0] >> 4) != 0)  
				Position[6] = (data[13] << 4) | (data[14] >> 2);   /* Ch7: Position: packet[6:15] */
			Position[8] = ((data[12]>>4)&1)*512; /* Ch 9: One of the auxilliary bits */
			sync = 0; /* End of even frame. Wait for sync */
			break;

		/* Odd frame */
        case 19:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				Position[1] = (data[17] << 4) | (data[18] >> 2);     /* Ch2: Position: packet[6:15] */
			break;
        case 23:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				Position[0] = (data[21] << 4) | (data[22] >> 2);     /* Ch1: Position: packet[6:15] */
			Position[9] = ((data[20]>>4)&1)*512; /* Ch 10: One of the auxilliary bits */
			break;
        case 27:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				Position[5] = (data[25] << 4) | (data[26] >> 2);     /* Ch6: Position: packet[6:15] */
			break;
        case 31:    /* 4th sextet of 4th packet is ready */
			if ((data[16] >> 4) != 1) 
				Position[7] = (data[29] << 4) | (data[30] >> 2);     /* Ch8: Position: packet[6:15] */
			Position[8] = ((data[28]>>4)&1)*512; /* Ch 9: One of the auxilliary bits */
			break;
        case 32: sync = 0;/* End of odd frame. Wait for sync */
			
#ifdef PPJOY
			SendPPJoy(10, Position);
#endif
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
    if (/*sync == 0 && */width > PPM_TRIG) {
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
	if (input|| JsChPostProc_selected!=-1)
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

	
	if (input|| JsChPostProc_selected!=-1)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	case 6: 	Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	case 7: 	Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	case 8: 	Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	case 9: 	Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	case 10: 	Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	case 11: 	Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	};
#ifdef PPJOY
				SendPPJoy(datacount, Position);
#endif

	if (gDebugLevel>=3 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile," data[%d]=%d", datacount, data[datacount]);

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

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
    if (input &&  width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
		former_sync = 1;
		return 0;
    }

    if (!sync) return 0; /* still waiting for sync */


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input|| JsChPostProc_selected!=-1)
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

	
	if (input|| JsChPostProc_selected!=-1)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	case 6: 	Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	case 7: 	Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	case 8: 	Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	case 9: 	Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	case 10: 	Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	case 11: 	Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	};

#ifdef PPJOY
				SendPPJoy(datacount, Position);
#endif
	if (gDebugLevel>=3 && gCtrlLogFile /*&& !(i++%50)*/)
		fprintf(gCtrlLogFile," data[%d]=%d", datacount, data[datacount]);
	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

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
    if (!input && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
		former_sync = 1;
		return 0;
    }

    if (!sync) return 0; /* still waiting for sync */


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input || JsChPostProc_selected!=-1)
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

	
	if (input|| JsChPostProc_selected!=-1)
		Position[datacount] = data[datacount];	/* JR - Assign data to joystick channels */
	else
		switch (datacount)
	{ // Futaba
	case 0: 	Position[1]  = data[datacount];	break;/* Assign data to joystick channels */
	case 1: 	Position[2]  = data[datacount];	break;/* Assign data to joystick channels */
	case 2: 	Position[0]  = data[datacount];	break;/* Assign data to joystick channels */
	case 3: 	Position[3]  = data[datacount];	break;/* Assign data to joystick channels */
	case 4: 	Position[4]  = data[datacount];	break;/* Assign data to joystick channels */
	case 5: 	Position[5]  = data[datacount];	break;/* Assign data to joystick channels */
	case 6: 	Position[6]  = data[datacount];	break;/* Assign data to joystick channels */
	case 7: 	Position[7]  = data[datacount];	break;/* Assign data to joystick channels */
	case 8: 	Position[8]  = data[datacount];	break;/* Assign data to joystick channels */
	case 9: 	Position[9]  = data[datacount];	break;/* Assign data to joystick channels */
	case 10: 	Position[10] = data[datacount];	break;/* Assign data to joystick channels */
	case 11: 	Position[11] = data[datacount];	break;/* Assign data to joystick channels */
	};
#ifdef PPJOY
				SendPPJoy(datacount, Position);
#endif

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

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


	/* Version 3.3.2 */
	if (target && target->nModulations > 0)
		nMod = target->nModulations;
	else
	{	
		MessageBox(NULL,"Winmm.dll: Cannot get number of modulation modes - Assuming PPM", "SmartPropoPlus Message" , MB_SYSTEMMODAL);
		nMod = 1;
	};

	ListProcessPulseFunc = (far void **)calloc(3*nMod+1, sizeof(far void *));

	/* Loop on list of modulation types */
	while(index < nMod && index<MAX_MODS)
	{
		/* Get the internal name of the modulation */
		if (target && target->pInternalModName)/* Version 3.3.2 */
			strcpy(tmp, (char *)target->pInternalModName[index]);
		/* Version 3.3.2 */
		else
		{
			/* TODO: Take it from the registry */
			strcpy(tmp, "PPM");
		};


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
		else if (!strcmp(tmp, MOD_TYPE_WAL))
		{
			ListProcessPulseFunc[0*nMod+index] = (void **)ProcessPulseWalPcm;
			ListProcessPulseFunc[1*nMod+index] = (void **)ProcessPulseWalPcm;
			ListProcessPulseFunc[2*nMod+index] = (void **)ProcessPulseWalPcm;
		}
		index++;
	};


	ListProcessPulseFunc[2*nMod+index] = (void **)NULL;
	return index;
}

__inline void SetActiveProcessPulseFunction(struct SharedDataBlock * dBlock)
{
	int set;

	/* Version 3.3.2 */
	if (!dBlock)
	{
		/* Todo: take it from the registry*/
		ProcessPulse = (PP)ProcessPulsePpm;
		return;
	};

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

__inline void SetActiveJsChPostProcFunction(struct SharedDataBlock * dBlock)
{
	int sel;

	/* TODO: Take filter from registry if dBlok is invalid */

	/* No filter DLL evailable  - do nothing */
	if (!hJschpostproc || /* Added in Version 3.3.2 */!dBlock)
	{
		JsChPostProc_selected = -1;
		return;
	};

	/* Nothing changed - do nothing */
	sel = dBlock->i_sel_fltr;
	if (sel == JsChPostProc_selected)
		return;

	/* Filter changed */
	JsChPostProc_selected = sel;
	if (pSelectFilterByIndex)
		pSelectFilterByIndex(sel);
}


/* 
	Start the SppConsole GUI application if it exists
*/
int StartSppConsole()
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;
	int errorcode, res;
	static UINT NEAR WM_INTERSPPAPPS;	
	char msg[1000];
	
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
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "StartSppConsole(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};
	PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTARTING, 0);

	/* If a filter file is loaded - send filter info to the GUI */
	//WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);/* 3.3.1 */
	if (hJschpostproc)
	{
		res = GetJsChPostProcInfo(hJschpostproc);
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_JSCHPPEVAIL, (long)res);
	};

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


	/* TODO: move this to some other place - why call it every time? */
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

#ifdef PPJOY
extern __declspec(dllexport) UINT __stdcall   StartPPJoyInterface()
{
	if (!hWinmm)
	{
		LoadWinmm(0);
		StartPropo();
		ConnectPPJoyDriver();
		hJschpostproc = LoadJsChPostProc();
	};
	if (!hJoy)
		ConnectPPJoyDriver();
	return 0;
}

extern __declspec(dllexport) UINT __stdcall   StopPPJoyInterface()
{
	if (hJoy)
		DisconnectPPJoyDriver();
	return 0;
}
#endif

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
	char ChnlLogFileName[2000] = {""};
	WAVEINCAPS Capabilities;
	int res=0;

#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: inside StartPropo",4); fflush(gCtrlLogFile);
#endif
	if (!DoStartPropo)
		return;

#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: inside StartPropo",5); fflush(gCtrlLogFile);
#endif
	/* Download configuration from the registry (if exists) */
	Modulation= GetModulation(0);


	/* Create the Global memory area (a.k.a. Shared Data) and upload configuration to it */
	DataBlock =  CreateDataBlock(Modulation);

	/* SetActiveProcessPulseFunction(DataBlock); */
	nActiveModulations = LoadProcessPulseFunctions(DataBlock);

#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: inside StartPropo",6); fflush(gCtrlLogFile);
#endif
	/* Get Debug level from the registry (if exists) and start debugging */
	gDebugLevel = GetDebugLevel();
	_DebugWelcomePopUp(Modulation);


    waveRecording = TRUE; /* Start recording */

	/* Get sample rate Version 3.3.2 
	res = pwaveInGetDevCapsA(waveIn, &Capabilities, sizeof(WAVEINCAPS));*/

	/* Wave format structure initialization */
    waveFmt.wFormatTag = WAVE_FORMAT_PCM;
    waveFmt.nChannels = 1;
    waveFmt.nSamplesPerSec = 44100;
    waveFmt.wBitsPerSample =16;
    waveFmt.nBlockAlign = waveFmt.wBitsPerSample / 8 * waveFmt.nChannels;
    waveFmt.nAvgBytesPerSec = waveFmt.nSamplesPerSec * waveFmt.nBlockAlign;
    waveFmt.cbSize = 0;

#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: inside StartPropo",7); fflush(gCtrlLogFile);
#endif
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
	
#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: End of StartPropo",8); fflush(gCtrlLogFile);
#endif
	
}
//---------------------------------------------------------------------------

void StopPropo(void)
{
    int i;
	char tbuffer [9], msg[1000];
	static UINT NEAR WM_INTERSPPAPPS;
	HANDLE hProcessHeap;

#ifdef DEBUG_W2K
	if (gCtrlLogFile) fclose(gCtrlLogFile);
#endif

    waveRecording = FALSE;
	hProcessHeap = GetProcessHeap();
	
	if (waveIn && hProcessHeap)
	{
		pwaveInStop(waveIn);
		for (i = 0; i < 2 ;i++) {
			pwaveInUnprepareHeader(waveIn, waveBuf[i], sizeof(WAVEHDR));
			HeapFree(hProcessHeap, 0, waveBuf[i]->lpData);
			HeapFree(hProcessHeap, 0, waveBuf[i]);
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
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "StopPropo(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};

	if (console_started)
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLSTOPPING, 0);

#ifdef _DEBUG // Collect channel data - close output file
	if (gChnlLogFile)
		fclose(gChnlLogFile);
#endif

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
		Not assumed - the GUI console is not active yet.*/
	static char  jc_name[50];
	if (!console_started)
		console_started = StartSppConsole();

#ifdef PPJOY
	/* If a filter file is loaded - send filter info to the GUI */
	return pjoyGetDevCapsA( uJoyID,  pjc,  cbjc);
#else /* PPJOY */

	/* Create joystick name string */
	if (!gDebugLevel)
		sprintf(jc_name,"%s %d.%d.%d", BASEPRODUCT, (VER_DLL&0xFF0000)>>16, (VER_DLL&0x00FF00)>>8, (VER_DLL&0x0000FF));
	else
		sprintf(jc_name,"%s %d.%d.%d DL%d", BASEPRODUCT, (VER_DLL&0xFF0000)>>16, (VER_DLL&0x00FF00)>>8, (VER_DLL&0x0000FF), gDebugLevel);

	if (uJoyID == 0) {
        pjc->wMid = 0;
        pjc->wPid = 0;
        strcpy((char*)pjc->szPname, jc_name);
        pjc->wXmin = pjc->wYmin = pjc->wZmin = pjc->wRmin = pjc->wUmin = pjc->wVmin = 0;
        pjc->wXmax = pjc->wYmax = pjc->wZmax = pjc->wRmax = pjc->wUmax = pjc->wVmax = 1023; /* Bug fix by Bruce Page */
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
#endif /* PPJOY */
}

/*

*/
extern __declspec(dllexport) UINT __stdcall        joyGetNumDevs(void)
{
#ifdef PPJOY
	UINT res;
	res = pjoyGetNumDevs( );
	return res;
#else /* PPJOY */
    return 1;
#endif /* PPJOY */
}


extern __declspec(dllexport) UINT __stdcall     joyGetPosEx(UINT uJoyID, LPJOYINFOEX pji)
{
#ifdef PPJOY
	UINT res;

	res = pjoyGetPosEx( uJoyID,  pji);
	return res;
#else /* PPJOY */
	int ch[MAX_JS_CH];
	if (uJoyID == 0) {
		
		/* Duplicate channel data */
		memcpy(ch, Position, MAX_JS_CH*sizeof(int));
		
		/* channel external filter */
		RunJsFilter(ch, MAX_JS_CH);

				
        pji->dwXpos = ch[3];
        pji->dwYpos = ch[2];
        pji->dwZpos = ch[1];
        pji->dwRpos = ch[0];
        pji->dwUpos = ch[4];
        pji->dwVpos = ch[5];
        return JOYERR_NOERROR;
	};
	
    return JOYERR_UNPLUGGED;
#endif /* PPJOY */
}

extern __declspec(dllexport) UINT __stdcall     joyGetPos(UINT uJoyID, LPJOYINFO pji)
{
#ifdef PPJOY
	UINT res;

	res = pjoyGetPos( uJoyID,  pji);
	return res;
#else /* PPJOY */
	int ch[MAX_JS_CH];
	if (uJoyID == 0) {
		
		/* Duplicate channel data */
		memcpy(ch, Position, MAX_JS_CH*sizeof(int));
		
		/* channel external filter */
		RunJsFilter(ch, MAX_JS_CH);

				
        pji->wXpos = ch[0];
        pji->wYpos = ch[1];
        pji->wZpos = ch[2];
        return JOYERR_NOERROR;
	};
	
    return JOYERR_UNPLUGGED;
#endif /* PPJOY */
}


BOOL LoadWinmm(int line)
{

	char msg[4000];

#ifdef DEBUG_W2K
	/* Create a Control-flow log file */
	char CtrlLogFileName[255];
	sprintf(CtrlLogFileName,"%s\\%s", getenv("TEMP"), CTRL_LOG_FILE);
	gCtrlLogFile = fopen(CtrlLogFileName, "w");
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: Starting LoadWinmm (%s)",1, DEBUG_W2K); fflush(gCtrlLogFile);
#endif

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

#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: inside LoadWinmm(%d), calling GetPointerToOriginalFunc",2,line); fflush(gCtrlLogFile);
#endif
	GetPointerToOriginalFunc();
#ifdef DEBUG_W2K
	if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: inside LoadWinmm(%d), After GetPointerToOriginalFunc",3,line); fflush(gCtrlLogFile);
#endif

	return TRUE;
}

/* Load the Joystick filter DLL */
HINSTANCE LoadJsChPostProc()
{

	HINSTANCE h;
	long filter_ver;

	/* Load DLL */
	h = LoadLibraryEx("JsChPostProc.dll", NULL, 0);
	if (!h)
		return h;

	/* Load the interface functions */
	if (!GetPointerToJsChPostProcOriginalFunc(h))
		return NULL;

	/* Allocate in/out joystick data buffer */
	js_data = (JS_CHANNELS *)calloc(1, sizeof(JS_CHANNELS));

	/* Initialize filter */
	filter_ver = GetDllVersion();
	if (filter_ver < 0x30100)
	{
		MessageBox(NULL,"Joystick filter too old - cannot load filter file", "SmartPropoPlus Debug Message" , MB_SYSTEMMODAL);
		return NULL;
	};

	return h;
}


/* Get pointer to functions in the JsChPostProc DLL */
int GetPointerToJsChPostProcOriginalFunc(HINSTANCE h)
{
	if (!h)
		return FALSE;

	GetDllVersion = GetProcAddress(h,"GetDllVersion");
	if (!GetDllVersion)
		return FALSE;

	ProcessChannels = (PJS_CHANNELS  (WINAPI *)(PJS_CHANNELS, int max, int min))GetProcAddress(h,"ProcessChannels");
	if (!ProcessChannels)
		return FALSE;

	pGetNumberOfFilters = (int  (WINAPI *)(void))GetProcAddress(h,"GetNumberOfFilters");
	pGetFilterNameByIndex = (const char *    (WINAPI *)(const int i))GetProcAddress(h,"GetFilterNameByIndex");
	pGetFilterIdByIndex = (const int   (WINAPI *)(const int iFilter))GetProcAddress(h,"GetFilterIdByIndex");
	pSelectFilterByIndex = (const int  (WINAPI *)(const int iFilter))GetProcAddress(h,"SelectFilterByIndex");
	pGetIndexOfSelectedFilter = (const int  (WINAPI *)(void))GetProcAddress(h,"GetIndexOfSelectedFilter");

	return TRUE;
}

/* Return Joystick post processor file information */
int GetJsChPostProcInfo(FAR HMODULE  hJschpostproc)
{
	int n, sel, i;

	char * names[127];
	if (!hJschpostproc)
		return 0;


	/* Number of supported filters */
	if (pGetNumberOfFilters)
		n = pGetNumberOfFilters();
	else
		n=1;
	SetNumberOfFilters(n);

	/* Selected filter */
	sel = -1;
	if (pGetIndexOfSelectedFilter)
		sel = pGetIndexOfSelectedFilter();
	if (sel >= 0)
		SetSelectedFilterIndex(sel);
	
	/* Filter names */
	if (pGetFilterNameByIndex)
	{
		for (i=0; i<n ; i++)
			names[i] = strdup(pGetFilterNameByIndex(i));
		names[i] = NULL;
	}
	else
	{
		names[0] = "(Default)";
		names[1] = NULL;
	};
	
	SetFilterNames(names);

	return 1;
}


/* Run Joystick post processor filter */
int RunJsFilter(int * ch, int nChannels)
{
	int n_out_ch=0;
	JS_CHANNELS  * js_filter_out;
	int i;

#if 0 // Collect channel data
	fprintf(gChnlLogFile, "\n%d\t;%d\t;%d\t;%d\t;%d\t;%d\t;%d\t;%d\t;%d\t;%d\t;%d\t;%d",\
		ch[0],ch[1],ch[2],ch[3],ch[4], ch[5],ch[6],ch[7],ch[8],ch[9],ch[10],ch[11]);
#endif

	SetActiveJsChPostProcFunction(DataBlock);

	if (hJschpostproc && JsChPostProc_selected!=-1)
	{
		js_data->ch = nChannels;
		js_data->value = ch;
		js_filter_out = ProcessChannels(js_data, 1023, 0);
		if (js_filter_out && js_filter_out->ch>0 && js_filter_out->ch<=MAX_JS_CH)
		{
			for (i=0;i<js_filter_out->ch;i++)
			{
				ch[i] = js_filter_out->value[i];
				js_filter_out->value[i] = 0;
			};
			n_out_ch = js_filter_out->ch;
			js_filter_out->ch=0;
		}
	};
	return n_out_ch;
}

#ifdef DEBUG_W2K
FARPROC GetProcAddressWithLog(HMODULE hModule, LPCSTR lpProcName)
{
	FARPROC proc;

	if (!gCtrlLogFile)
		return GetProcAddress( hModule,  lpProcName);

	fprintf(gCtrlLogFile, "\n\tGetProcAddress(%s)", lpProcName);  fflush(gCtrlLogFile);
	proc = GetProcAddress( hModule,  lpProcName);
	if (proc)
	{
		fprintf(gCtrlLogFile, " = %x", (long *)proc);  
		fflush(gCtrlLogFile);
	}
	else
	{
		fprintf(gCtrlLogFile, ">>>>>Error=%d", GetLastError());  
		fflush(gCtrlLogFile);
	};

	return proc;
}

#else
#define GetProcAddressWithLog GetProcAddress
#endif

/* Get pointer to functions in the original DLL */
void GetPointerToOriginalFunc(void)
{
    pCloseDriver = GetProcAddressWithLog(hWinmm, "CloseDriver");
    pDefDriverProc = GetProcAddressWithLog(hWinmm, "DefDriverProc");
    pDriverCallback = GetProcAddressWithLog(hWinmm, "DriverCallback");
    pDrvGetModuleHandle = GetProcAddressWithLog(hWinmm, "DrvGetModuleHandle");
    pGetDriverModuleHandle = GetProcAddressWithLog(hWinmm, "GetDriverModuleHandle");
    pMigrateAllDrivers = GetProcAddressWithLog(hWinmm, "MigrateAllDrivers");
//    pMigrateMidiUser = GetProcAddressWithLog(hWinmm, "MigrateMidiUser");
    pMigrateSoundEvents = GetProcAddressWithLog(hWinmm, "MigrateSoundEvents");
    pNotifyCallbackData = GetProcAddressWithLog(hWinmm, "NotifyCallbackData");
    pOpenDriver = GetProcAddressWithLog(hWinmm, "OpenDriver");
    pPlaySound = GetProcAddressWithLog(hWinmm, "PlaySound");
    pPlaySoundA = GetProcAddressWithLog(hWinmm, "PlaySoundA");
    pPlaySoundW = GetProcAddressWithLog(hWinmm, "PlaySoundW");
    pSendDriverMessage = GetProcAddressWithLog(hWinmm, "SendDriverMessage");
    pWOW32DriverCallback = GetProcAddressWithLog(hWinmm, "WOW32DriverCallback");
    pWOW32ResolveMultiMediaHandle = GetProcAddressWithLog(hWinmm, "WOW32ResolveMultiMediaHandle");
    pWOWAppExit = GetProcAddressWithLog(hWinmm, "WOWAppExit");
    pWinmmLogoff = GetProcAddressWithLog(hWinmm, "WinmmLogoff");
    pWinmmLogon = GetProcAddressWithLog(hWinmm, "WinmmLogon");
    paux32Message = GetProcAddressWithLog(hWinmm, "aux32Message");
    pauxGetDevCapsA = GetProcAddressWithLog(hWinmm, "auxGetDevCapsA");
    pauxGetDevCapsW = GetProcAddressWithLog(hWinmm, "auxGetDevCapsW");
    pauxGetNumDevs = GetProcAddressWithLog(hWinmm, "auxGetNumDevs");
    pauxGetVolume = GetProcAddressWithLog(hWinmm, "auxGetVolume");
    pauxOutMessage = GetProcAddressWithLog(hWinmm, "auxOutMessage");
    pauxSetVolume = GetProcAddressWithLog(hWinmm, "auxSetVolume");
    pjoy32Message = GetProcAddressWithLog(hWinmm, "joy32Message");
    pjoyConfigChanged = GetProcAddressWithLog(hWinmm, "joyConfigChanged");
    pjoyGetDevCapsA = GetProcAddressWithLog(hWinmm, "joyGetDevCapsA");
    pjoyGetDevCapsW = GetProcAddressWithLog(hWinmm, "joyGetDevCapsW");
    pjoyGetNumDevs = GetProcAddressWithLog(hWinmm, "joyGetNumDevs");
//    pjoyGetPos = GetProcAddressWithLog(hWinmm, "joyGetPos");
    pjoyGetThreshold = GetProcAddressWithLog(hWinmm, "joyGetThreshold");
    pjoyReleaseCapture = GetProcAddressWithLog(hWinmm, "joyReleaseCapture");
    pjoySetCapture = GetProcAddressWithLog(hWinmm, "joySetCapture");
    pjoySetThreshold = GetProcAddressWithLog(hWinmm, "joySetThreshold");
    pmci32Message = GetProcAddressWithLog(hWinmm, "mci32Message");
    pmciDriverNotify = GetProcAddressWithLog(hWinmm, "mciDriverNotify");
    pmciDriverYield = GetProcAddressWithLog(hWinmm, "mciDriverYield");
    pmciExecute = GetProcAddressWithLog(hWinmm, "mciExecute");
    pmciFreeCommandResource = GetProcAddressWithLog(hWinmm, "mciFreeCommandResource");
    pmciGetCreatorTask = GetProcAddressWithLog(hWinmm, "mciGetCreatorTask");
    pmciGetDeviceIDA = GetProcAddressWithLog(hWinmm, "mciGetDeviceIDA");
    pmciGetDeviceIDFromElementIDA = GetProcAddressWithLog(hWinmm, "mciGetDeviceIDFromElementIDA");
    pmciGetDeviceIDFromElementIDW = GetProcAddressWithLog(hWinmm, "mciGetDeviceIDFromElementIDW");
    pmciGetDeviceIDW = GetProcAddressWithLog(hWinmm, "mciGetDeviceIDW");
    pmciGetDriverData = GetProcAddressWithLog(hWinmm, "mciGetDriverData");
    pmciGetErrorStringA = GetProcAddressWithLog(hWinmm, "mciGetErrorStringA");
    pmciGetErrorStringW = GetProcAddressWithLog(hWinmm, "mciGetErrorStringW");
    pmciGetYieldProc = GetProcAddressWithLog(hWinmm, "mciGetYieldProc");
    pmciLoadCommandResource = GetProcAddressWithLog(hWinmm, "mciLoadCommandResource");
    pmciSendCommandA = GetProcAddressWithLog(hWinmm, "mciSendCommandA");
    pmciSendCommandW = GetProcAddressWithLog(hWinmm, "mciSendCommandW");
    pmciSendStringA = GetProcAddressWithLog(hWinmm, "mciSendStringA");
    pmciSendStringW = GetProcAddressWithLog(hWinmm, "mciSendStringW");
    pmciSetDriverData = GetProcAddressWithLog(hWinmm, "mciSetDriverData");
    pmciSetYieldProc = GetProcAddressWithLog(hWinmm, "mciSetYieldProc");
    pmid32Message = GetProcAddressWithLog(hWinmm, "mid32Message");
    pmidiConnect = GetProcAddressWithLog(hWinmm, "midiConnect");
    pmidiDisconnect = GetProcAddressWithLog(hWinmm, "midiDisconnect");
    pmidiInAddBuffer = GetProcAddressWithLog(hWinmm, "midiInAddBuffer");
    pmidiInClose = GetProcAddressWithLog(hWinmm, "midiInClose");
    pmidiInGetDevCapsA = GetProcAddressWithLog(hWinmm, "midiInGetDevCapsA");
    pmidiInGetDevCapsW = GetProcAddressWithLog(hWinmm, "midiInGetDevCapsW");
    pmidiInGetErrorTextA = GetProcAddressWithLog(hWinmm, "midiInGetErrorTextA");
    pmidiInGetErrorTextW = GetProcAddressWithLog(hWinmm, "midiInGetErrorTextW");
    pmidiInGetID = GetProcAddressWithLog(hWinmm, "midiInGetID");
    pmidiInGetNumDevs = GetProcAddressWithLog(hWinmm, "midiInGetNumDevs");
    pmidiInMessage = GetProcAddressWithLog(hWinmm, "midiInMessage");
    pmidiInOpen = GetProcAddressWithLog(hWinmm, "midiInOpen");
    pmidiInPrepareHeader = GetProcAddressWithLog(hWinmm, "midiInPrepareHeader");
    pmidiInReset = GetProcAddressWithLog(hWinmm, "midiInReset");
    pmidiInStart = GetProcAddressWithLog(hWinmm, "midiInStart");
    pmidiInStop = GetProcAddressWithLog(hWinmm, "midiInStop");
    pmidiInUnprepareHeader = GetProcAddressWithLog(hWinmm, "midiInUnprepareHeader");
    pmidiOutCacheDrumPatches = GetProcAddressWithLog(hWinmm, "midiOutCacheDrumPatches");
    pmidiOutCachePatches = GetProcAddressWithLog(hWinmm, "midiOutCachePatches");
    pmidiOutClose = GetProcAddressWithLog(hWinmm, "midiOutClose");
    pmidiOutGetDevCapsA = GetProcAddressWithLog(hWinmm, "midiOutGetDevCapsA");
    pmidiOutGetDevCapsW = GetProcAddressWithLog(hWinmm, "midiOutGetDevCapsW");
    pmidiOutGetErrorTextA = GetProcAddressWithLog(hWinmm, "midiOutGetErrorTextA");
    pmidiOutGetErrorTextW = GetProcAddressWithLog(hWinmm, "midiOutGetErrorTextW");
    pmidiOutGetID = GetProcAddressWithLog(hWinmm, "midiOutGetID");
    pmidiOutGetNumDevs = GetProcAddressWithLog(hWinmm, "midiOutGetNumDevs");
    pmidiOutGetVolume = GetProcAddressWithLog(hWinmm, "midiOutGetVolume");
    pmidiOutLongMsg = GetProcAddressWithLog(hWinmm, "midiOutLongMsg");
    pmidiOutMessage = GetProcAddressWithLog(hWinmm, "midiOutMessage");
    pmidiOutOpen = GetProcAddressWithLog(hWinmm, "midiOutOpen");
    pmidiOutPrepareHeader = GetProcAddressWithLog(hWinmm, "midiOutPrepareHeader");
    pmidiOutReset = GetProcAddressWithLog(hWinmm, "midiOutReset");
    pmidiOutSetVolume = GetProcAddressWithLog(hWinmm, "midiOutSetVolume");
    pmidiOutShortMsg = GetProcAddressWithLog(hWinmm, "midiOutShortMsg");
    pmidiOutUnprepareHeader = GetProcAddressWithLog(hWinmm, "midiOutUnprepareHeader");
    pmidiStreamClose = GetProcAddressWithLog(hWinmm, "midiStreamClose");
    pmidiStreamOpen = GetProcAddressWithLog(hWinmm, "midiStreamOpen");
    pmidiStreamOut = GetProcAddressWithLog(hWinmm, "midiStreamOut");
    pmidiStreamPause = GetProcAddressWithLog(hWinmm, "midiStreamPause");
    pmidiStreamPosition = GetProcAddressWithLog(hWinmm, "midiStreamPosition");
    pmidiStreamProperty = GetProcAddressWithLog(hWinmm, "midiStreamProperty");
    pmidiStreamRestart = GetProcAddressWithLog(hWinmm, "midiStreamRestart");
    pmidiStreamStop = GetProcAddressWithLog(hWinmm, "midiStreamStop");
    pmixerClose = GetProcAddressWithLog(hWinmm, "mixerClose");
    pmixerGetControlDetailsA = GetProcAddressWithLog(hWinmm, "mixerGetControlDetailsA");
    pmixerGetControlDetailsW = GetProcAddressWithLog(hWinmm, "mixerGetControlDetailsW");
    pmixerGetDevCapsA = GetProcAddressWithLog(hWinmm, "mixerGetDevCapsA");
    pmixerGetDevCapsW = GetProcAddressWithLog(hWinmm, "mixerGetDevCapsW");
    pmixerGetID = GetProcAddressWithLog(hWinmm, "mixerGetID");
    pmixerGetLineControlsA = GetProcAddressWithLog(hWinmm, "mixerGetLineControlsA");
    pmixerGetLineControlsW = GetProcAddressWithLog(hWinmm, "mixerGetLineControlsW");
    pmixerGetLineInfoA = GetProcAddressWithLog(hWinmm, "mixerGetLineInfoA");
    pmixerGetLineInfoW = GetProcAddressWithLog(hWinmm, "mixerGetLineInfoW");
    pmixerGetNumDevs = GetProcAddressWithLog(hWinmm, "mixerGetNumDevs");
    pmixerMessage = GetProcAddressWithLog(hWinmm, "mixerMessage");
    pmixerOpen = GetProcAddressWithLog(hWinmm, "mixerOpen");
    pmixerSetControlDetails = GetProcAddressWithLog(hWinmm, "mixerSetControlDetails");
    pmmDrvInstall = GetProcAddressWithLog(hWinmm, "mmDrvInstall");
    pmmGetCurrentTask = GetProcAddressWithLog(hWinmm, "mmGetCurrentTask");
    pmmTaskBlock = GetProcAddressWithLog(hWinmm, "mmTaskBlock");
    pmmTaskCreate = GetProcAddressWithLog(hWinmm, "mmTaskCreate");
    pmmTaskSignal = GetProcAddressWithLog(hWinmm, "mmTaskSignal");
    pmmTaskYield = GetProcAddressWithLog(hWinmm, "mmTaskYield");
    pmmioAdvance = GetProcAddressWithLog(hWinmm, "mmioAdvance");
    pmmioAscend = GetProcAddressWithLog(hWinmm, "mmioAscend");
    pmmioClose = GetProcAddressWithLog(hWinmm, "mmioClose");
    pmmioCreateChunk = GetProcAddressWithLog(hWinmm, "mmioCreateChunk");
    pmmioDescend = GetProcAddressWithLog(hWinmm, "mmioDescend");
    pmmioFlush = GetProcAddressWithLog(hWinmm, "mmioFlush");
    pmmioGetInfo = GetProcAddressWithLog(hWinmm, "mmioGetInfo");
    pmmioInstallIOProcA = GetProcAddressWithLog(hWinmm, "mmioInstallIOProcA");
    pmmioInstallIOProcW = GetProcAddressWithLog(hWinmm, "mmioInstallIOProcW");
    pmmioOpenA = GetProcAddressWithLog(hWinmm, "mmioOpenA");
    pmmioOpenW = GetProcAddressWithLog(hWinmm, "mmioOpenW");
    pmmioRead = GetProcAddressWithLog(hWinmm, "mmioRead");
    pmmioRenameA = GetProcAddressWithLog(hWinmm, "mmioRenameA");
    pmmioRenameW = GetProcAddressWithLog(hWinmm, "mmioRenameW");
    pmmioSeek = GetProcAddressWithLog(hWinmm, "mmioSeek");
    pmmioSendMessage = GetProcAddressWithLog(hWinmm, "mmioSendMessage");
    pmmioSetBuffer = GetProcAddressWithLog(hWinmm, "mmioSetBuffer");
    pmmioSetInfo = GetProcAddressWithLog(hWinmm, "mmioSetInfo");
    pmmioStringToFOURCCA = GetProcAddressWithLog(hWinmm, "mmioStringToFOURCCA");
    pmmioStringToFOURCCW = GetProcAddressWithLog(hWinmm, "mmioStringToFOURCCW");
    pmmioWrite = GetProcAddressWithLog(hWinmm, "mmioWrite");
    pmmsystemGetVersion = GetProcAddressWithLog(hWinmm, "mmsystemGetVersion");
    pmod32Message = GetProcAddressWithLog(hWinmm, "mod32Message");
    pmxd32Message = GetProcAddressWithLog(hWinmm, "mxd32Message");
    psndPlaySoundA = GetProcAddressWithLog(hWinmm, "sndPlaySoundA");
    psndPlaySoundW = GetProcAddressWithLog(hWinmm, "sndPlaySoundW");
    ptid32Message = GetProcAddressWithLog(hWinmm, "tid32Message");
    ptimeBeginPeriod = GetProcAddressWithLog(hWinmm, "timeBeginPeriod");
    ptimeEndPeriod = GetProcAddressWithLog(hWinmm, "timeEndPeriod");
    ptimeGetDevCaps = GetProcAddressWithLog(hWinmm, "timeGetDevCaps");
    ptimeGetSystemTime = GetProcAddressWithLog(hWinmm, "timeGetSystemTime");
    ptimeGetTime = GetProcAddressWithLog(hWinmm, "timeGetTime");
    ptimeKillEvent = GetProcAddressWithLog(hWinmm, "timeKillEvent");
    ptimeSetEvent = GetProcAddressWithLog(hWinmm, "timeSetEvent");
    pwaveInGetDevCapsA = GetProcAddressWithLog(hWinmm, "waveInGetDevCapsA");
    pwaveInGetDevCapsW = GetProcAddressWithLog(hWinmm, "waveInGetDevCapsW");
    pwaveInGetErrorTextA = GetProcAddressWithLog(hWinmm, "waveInGetErrorTextA");
    pwaveInGetErrorTextW = GetProcAddressWithLog(hWinmm, "waveInGetErrorTextW");
    pwaveInGetID = GetProcAddressWithLog(hWinmm, "waveInGetID");
    pwaveInGetNumDevs = GetProcAddressWithLog(hWinmm, "waveInGetNumDevs");
    pwaveInGetPosition = GetProcAddressWithLog(hWinmm, "waveInGetPosition");
    pwaveInMessage = GetProcAddressWithLog(hWinmm, "waveInMessage");
    pwaveInReset = GetProcAddressWithLog(hWinmm, "waveInReset");
    pwaveOutBreakLoop = GetProcAddressWithLog(hWinmm, "waveOutBreakLoop");
    pwaveOutClose = GetProcAddressWithLog(hWinmm, "waveOutClose");
    pwaveOutGetDevCapsA = GetProcAddressWithLog(hWinmm, "waveOutGetDevCapsA");
    pwaveOutGetDevCapsW = GetProcAddressWithLog(hWinmm, "waveOutGetDevCapsW");
    pwaveOutGetErrorTextA = GetProcAddressWithLog(hWinmm, "waveOutGetErrorTextA");
    pwaveOutGetErrorTextW = GetProcAddressWithLog(hWinmm, "waveOutGetErrorTextW");
    pwaveOutGetID = GetProcAddressWithLog(hWinmm, "waveOutGetID");
    pwaveOutGetNumDevs = GetProcAddressWithLog(hWinmm, "waveOutGetNumDevs");
    pwaveOutGetPitch = GetProcAddressWithLog(hWinmm, "waveOutGetPitch");
    pwaveOutGetPlaybackRate = GetProcAddressWithLog(hWinmm, "waveOutGetPlaybackRate");
    pwaveOutGetPosition = GetProcAddressWithLog(hWinmm, "waveOutGetPosition");
    pwaveOutGetVolume = GetProcAddressWithLog(hWinmm, "waveOutGetVolume");
    pwaveOutMessage = GetProcAddressWithLog(hWinmm, "waveOutMessage");
    pwaveOutOpen = GetProcAddressWithLog(hWinmm, "waveOutOpen");
    pwaveOutPause = GetProcAddressWithLog(hWinmm, "waveOutPause");
    pwaveOutPrepareHeader = GetProcAddressWithLog(hWinmm, "waveOutPrepareHeader");
    pwaveOutReset = GetProcAddressWithLog(hWinmm, "waveOutReset");
    pwaveOutRestart = GetProcAddressWithLog(hWinmm, "waveOutRestart");
    pwaveOutSetPitch = GetProcAddressWithLog(hWinmm, "waveOutSetPitch");
    pwaveOutSetPlaybackRate = GetProcAddressWithLog(hWinmm, "waveOutSetPlaybackRate");
    pwaveOutSetVolume = GetProcAddressWithLog(hWinmm, "waveOutSetVolume");
    pwaveOutUnprepareHeader = GetProcAddressWithLog(hWinmm, "waveOutUnprepareHeader");
    pwaveOutWrite = GetProcAddressWithLog(hWinmm, "waveOutWrite");
    pwid32Message = GetProcAddressWithLog(hWinmm, "wid32Message");
    pwinmmDbgOut = GetProcAddressWithLog(hWinmm, "winmmDbgOut");
    pwinmmSetDebugLevel = GetProcAddressWithLog(hWinmm, "winmmSetDebugLevel");
    pwod32Message = GetProcAddressWithLog(hWinmm, "wod32Message");

    pgfxAddGfx = GetProcAddressWithLog(hWinmm, "gfxAddGfx");
    pgfxBatchChange = GetProcAddressWithLog(hWinmm, "gfxBatchChange");
    pgfxCreateGfxFactoriesList = GetProcAddressWithLog(hWinmm, "gfxCreateGfxFactoriesList");
    pgfxCreateZoneFactoriesList = GetProcAddressWithLog(hWinmm, "gfxCreateZoneFactoriesList");
    pgfxDestroyDeviceInterfaceList = GetProcAddressWithLog(hWinmm, "gfxDestroyDeviceInterfaceList");
    pgfxEnumerateGfxs = GetProcAddressWithLog(hWinmm, "gfxEnumerateGfxs");
    p_gfxLogoff = GetProcAddressWithLog(hWinmm, "_gfxLogoff@0");
    p_gfxLogon = GetProcAddressWithLog(hWinmm, "_gfxLogon@4");
    pgfxModifyGfx = GetProcAddressWithLog(hWinmm, "gfxModifyGfx");
    pgfxOpenGfx = GetProcAddressWithLog(hWinmm, "gfxOpenGfx");
    pgfxRemoveGfx = GetProcAddressWithLog(hWinmm, "gfxRemoveGfx");

//	pFunctionNumberTwo = GetProcAddressWithLog(hWinmm, (const char *)2);
#ifdef DEBUG_W2K
//	fprintf(gCtrlLogFile, "\n%d: Calculate pgfxRemoveGfx - %x",9, *(long *)pgfxRemoveGfx);
//	fprintf(gCtrlLogFile, "\n%d: Calculate pFunctionNumberTwo - %x",10, *(long *)pFunctionNumberTwo);
#endif

    pwaveInAddBuffer =  (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddressWithLog(hWinmm, "waveInAddBuffer");
    pwaveInClose = (UINT ( WINAPI *)(HWAVEIN))GetProcAddressWithLog(hWinmm, "waveInClose");
    pwaveInOpen = (UINT (WINAPI*)(LPHWAVIN, UINT, LPCWAVEFORMATEX, DWORD, DWORD, DWORD))GetProcAddressWithLog(hWinmm, "waveInOpen");
    pwaveInPrepareHeader = (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddressWithLog(hWinmm, "waveInPrepareHeader");
    pwaveInStart = (UINT ( WINAPI *)(HWAVEIN))GetProcAddressWithLog(hWinmm, "waveInStart");
    pwaveInStop = (UINT ( WINAPI *)(HWAVEIN))GetProcAddressWithLog(hWinmm, "waveInStop");
    pwaveInUnprepareHeader = (UINT ( WINAPI *)(HWAVEIN, void*, UINT))GetProcAddressWithLog(hWinmm, "waveInUnprepareHeader");
	pjoyGetPosEx = (UINT ( WINAPI *)(UINT uJoyID, LPJOYINFOEX pji))GetProcAddressWithLog(hWinmm, "joyGetPosEx");
	pjoyGetPos = (UINT ( WINAPI *)(UINT uJoyID, LPJOYINFO pji))GetProcAddressWithLog(hWinmm, "joyGetPos");
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

#if 0
#ifdef DEBUG_W2K
void  FunctionNumberTwo(void) {
if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: Starting FunctionNumberTwo",9);  fflush(gCtrlLogFile);
INITWINMM __asm jmp dword ptr   pFunctionNumberTwo
if (gCtrlLogFile) fprintf(gCtrlLogFile, "\n%d: End of FunctionNumberTwo",10);  fflush(gCtrlLogFile);
}
#else
void NAKED FunctionNumberTwo(void) {   INITWINMM __asm jmp dword ptr   pFunctionNumberTwo}
#endif
#endif

UINT NAKED waveInAddBuffer(HWAVEIN a, void* b, UINT c) {  INITWINMM __asm jmp dword ptr   pwaveInAddBuffer}
UINT NAKED waveInClose(HWAVEIN a) { INITWINMM  __asm jmp dword ptr   pwaveInClose}
UINT NAKED waveInOpen(void* a, UINT b, void* c, DWORD d, DWORD e, DWORD f) { INITWINMM  __asm jmp dword ptr   pwaveInOpen}
UINT NAKED waveInPrepareHeader(HWAVEIN a, void* b, UINT c) { INITWINMM  __asm jmp dword ptr   pwaveInPrepareHeader}
UINT NAKED waveInStart(HWAVEIN a ) { INITWINMM  __asm jmp dword ptr   pwaveInStart}
UINT NAKED waveInStop(HWAVEIN a) { INITWINMM  __asm jmp dword ptr   pwaveInStop}
UINT NAKED waveInUnprepareHeader(HWAVEIN a, void* b, UINT c) { INITWINMM  __asm jmp dword ptr   pwaveInUnprepareHeader}
UINT NAKED waveInReset(HWAVEIN a) { INITWINMM  __asm jmp dword ptr   pwaveInReset}


#ifdef PPJOY
/*
	Connect to PPJoy driver and save handle to it
	- Create file & get handler
	- Post success/fail message to the GUI
	- Initialize IOCTL data structure: All channeles to mid-range
	- Set PPJoy Active flag (PpjActive)
*/
int ConnectPPJoyDriver()
{
	char *DevName= JOYSTICK_IOCTL1;
	int i;
	static UINT NEAR WM_INTERSPPAPPS;
	char msg[1000];
	
	/* Make sure we could open the device! */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "ConnectPPJoyDriver(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};

	/* Open a handle to the control device for the first virtual joystick. */
	/* Virtual joystick devices are names PPJoyIOCTL1 to PPJoyIOCTL16. */
	hJoy= CreateFile(JOYSTICK_IOCTL1,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	
	if (hJoy==INVALID_HANDLE_VALUE)
	{
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, GetLastError());
		return 0;
	}
	else
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, 0);

	
	/* Initialise the IOCTL data structure */
	JoyState.Signature= JOYSTICK_STATE_V1;
	JoyState.NumAnalog= CHANNELS;	/* Number of analog values */
	JoyState.NumDigital= 0;	/* Number of digital values */
	for (i=0 ; i<JoyState.NumAnalog; i++)
		JoyState.Analog[i] = (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;

	PpjActive = 1;
	return 1;
	
}
/*
	Disconnect from PPJoy driver and reset handle and PPJoy Active flag (PpjActive)
	Post 'disconnected' message to the GUI
*/
int DisconnectPPJoyDriver()
{
	static UINT NEAR WM_INTERSPPAPPS;
	char msg[1000];

	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "DisconnectPPJoyDriver(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};


	PpjActive = 0;
	if (hJoy && CloseHandle(hJoy))
	{
		hJoy = NULL;
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, MSG_DPPJSTAT_DISCN);
		return 1;
	}
	else
		return 0;
}

/*
	This function is called repeatedly to update the PPJoy driver of joystick position
	Inputs:
	- nChannels: Number of valid channels (minus one)
	- Channel: Array of integers - each one represents a joystich channel
	Operation:
	- Test number of channels
	- Populate the IOCTL data structure with channel data
	- If valid handle to PPJoy then send the data
	- If invalid handle to PPJoy then try to re-connect
*/
void SendPPJoy(int nChannels, int *Channel)
{
	DWORD	RetSize;
	int i, n_ch;
	static UINT NEAR WM_INTERSPPAPPS;
	char *DevName= JOYSTICK_IOCTL1;
	static int rc = 0, prev_rc=0;
	static time_t t0=0, t1;
	int ch[MAX_JS_CH];
	char msg[1000];

	/* Perform task only if PPJoy Active flag (PpjActive) is set*/
	if (!PpjActive)
		return;

	/* Test value of number of channels */
	nChannels++;
	if (nChannels>CHANNELS)
		nChannels=CHANNELS;
	if (nChannels<1)
		return;
	
	/* Duplicate channel data */
	memcpy(ch, Channel, MAX_JS_CH*sizeof(int));

	/* channel external filter */
	n_ch = RunJsFilter(ch, nChannels);
	if (n_ch)
		nChannels = n_ch;

	/* Fill-in the PPJ structure */
	JoyState.NumAnalog= CHANNELS;	/* Number of analog values */
	for (i=0; i<nChannels; i++)
	{
		JoyState.Analog[i] = PPJOY_AXIS_MIN+(ch[i]<<5);
	};
	if (hJoy != INVALID_HANDLE_VALUE)
	{
		/* Send data to PPJoy */
		if (!DeviceIoControl(hJoy,IOCTL_PPORTJOY_SET_STATE,&JoyState,sizeof(JoyState),NULL,0,&RetSize,NULL))
		{	/* If action faild the disconnect, reset values and set timeout start-value */
			CloseHandle(hJoy);
			hJoy = INVALID_HANDLE_VALUE;
			time(&t0);
		}	else
			t0=0;
	}

	else
	{	/* Cannot send data to PPJoy */
		/* Try to reconnect - if timeout and change if return status post status to GUI */
		hJoy= CreateFile(JOYSTICK_IOCTL1,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
		rc= GetLastError();
		time(&t1);
		if (t1-t0>1 && rc != prev_rc)
		{
			WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
			{	/* 3.3.1 */
				sprintf(msg, "SendPPJoy(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
				MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
			};
			if (hJoy == INVALID_HANDLE_VALUE)
				PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, rc);
			else
				PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_DLLPPJSTAT, 0);
			prev_rc = rc;
		};
	};
	
}

extern __declspec(dllexport) UINT __stdcall    DllGetVersion(DLLVERSIONINFO *pdvi)
{
	long GuiVer;

	GuiVer = VER_DLL;

	if (pdvi->cbSize != sizeof(DLLVERSIONINFO))
		return E_INVALIDARG;

	pdvi->dwBuildNumber  = GuiVer&0xFF;
	pdvi->dwMinorVersion = (GuiVer&0xFF00)>>8;
	pdvi->dwMajorVersion = (GuiVer&0xFF0000)>>16;
	pdvi->dwPlatformID = DLLVER_PLATFORM_WINDOWS;
	return NOERROR;

}

extern __declspec(dllexport) UINT __stdcall    StartJsChPostProc(void)
{
	static UINT NEAR WM_INTERSPPAPPS;	
	int res;
	char msg[1000];

	/* If a filter file is loaded - send filter info to the GUI */
	WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);
	if (!WM_INTERSPPAPPS)
	{	/* 3.3.1 */
		sprintf(msg, "StartJsChPostProc(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS", WM_INTERSPPAPPS);
		MessageBox(NULL,msg, "SmartPropoPlus Message" , MB_SYSTEMMODAL);
	};
	if (hJschpostproc)
	{
		res = GetJsChPostProcInfo(hJschpostproc);
		PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_JSCHPPEVAIL, (long)res);
	};

	return res;
}
#endif /* PPJOY */