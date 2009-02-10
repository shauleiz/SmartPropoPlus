#include "StdAfx.h"
#include <math.h>
#include ".\wavein.h"

//---------------------------------------------------------------------------
/*
	
*/
static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR lpUser, WAVEHDR *buf, DWORD Reserved)
{
	DWORD ThreadId = (DWORD)lpUser;
	PostThreadMessage(ThreadId, uMsg, (WPARAM)hwi, (LPARAM)buf);
}
//---------------------------------------------------------------------------
/*
 * Thread to which the Windows Low Level WAVE API passes messages
 * regarding digital audio recording (such as MM_WIM_DATA, MM_WIM_OPEN, and
 * MM_WIM_CLOSE).
 *
 * Before opening the WAVE In Device, I use CreateThread() (from my main
 * thread) to start up waveInProc() as a background thread. waveInThreadProc()
 * simply loops around a call to GetMessage() to process those messages
 * that the audio driver sends me during recording.
 *
 * When I subsequently open the WAVE In device (using waveInOpen() in my
 * main thread), I pass CALLBACK_THREAD and the ID of this thread.

 * 
 * For every entry in the audio buffer, calles ProcessData() with the entry as a parameter
 */
DWORD WINAPI g_WaveInThreadProc(LPVOID arg)
{
	MSG		msg;
	WAVEHDR * buf/*, * hdr*/;
	BOOL bRet;
//	char DebugData[1000];
	HWAVEIN hWaveIn;

	CWaveIn * ThisWaveIn = (CWaveIn *)arg;

	/* Wait for a message sent to me by the audio driver */
	while (bRet = GetMessage(&msg, (HWND)-1, 0, 0) != 0)
	{
		if (bRet == -1)
		{// Error
			return -1;
		};

		switch (msg.message)
		{
		////	A buffer has been filled by the driver 
		case MM_WIM_DATA:
			{
				//	The msg.lParam contains a pointer to the WAVEHDR structure for the filled buffer	
				buf = (WAVEHDR *)msg.lParam;
				hWaveIn = (HWAVEIN)msg.wParam;
				ThisWaveIn->DebugPrintWaveHdr(buf, (GetMessageTime()&0xFFFF));
				ThisWaveIn->ProcessBuffer(buf, hWaveIn);
				ThisWaveIn->ResetBuffer(buf);
				break;

			} // case MM_WIM_DATA

		/* Our main thread is opening the WAVE device */
		case MM_WIM_OPEN:
			{
				break;
			} // case MM_WIM_OPEN

			// Our main thread is closing the WAVE device
		case MM_WIM_CLOSE:
			{
				waveInReset((HWAVEIN)msg.wParam);
				break ;
			} // case MM_WIM_CLOSE
		}

	}; // While

	return 0;
}



/*
	This thread starts from PlayWaveFileLog() (Start reading a WAV file)
	It gets as input parameter a pointer to the calling object
	* Resets the Stopped/Ended flags
	* Starts a waitable timer that tick 5 milliseconds. This timer has a Completion routine that does the actual work
	* Starts a loop of Sleep that is broken when one of the Stopped/Ended flags is set.
*/
DWORD WINAPI g_ReadWaveFileThreadProc(LPVOID arg)
{
	// Pointer to the calling object
	CWaveIn * ThisWaveIn = (CWaveIn *)arg;

	// Initialize flags
	ThisWaveIn->m_PlayStopped = false;
	ThisWaveIn->m_PlayEnded = false;

	// Starts a waitable timer that tick 5 milliseconds. This timer has a Completion routine that does the actual work
	HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, TEXT("SPP - PLAY WAVE TIMER"));
    LARGE_INTEGER liDueTime;
    liDueTime.QuadPart=-100;
	BOOL TimerSet = SetWaitableTimer(hTimer, &liDueTime, POLLING_PERIOD, TimerCompletionRoutine, (LPVOID)ThisWaveIn, TRUE); 

	// Starts a loop of Sleep that is broken when one of the Stopped/Ended flags is set.
	while (!ThisWaveIn->m_PlayStopped && !ThisWaveIn->m_PlayEnded)
		SleepEx(1000, TRUE);

	// Reading ended. Either stopped by user or reached file end - now kill timer, releas memory, restart WaveIn and exit thread
	CloseHandle(hTimer);
	ThisWaveIn->StopWaveFileLog();
	ThisWaveIn->Start();

	return 0;
}

/* 
	Timer Completion routine started in g_ReadWaveFileThreadProc()
	This routine is called peiodically, getting a buffer of data from input WAV file
	When the reading routine reads past the end of the file it returns ZERO - in this case the PlayEnded flag is set
	Otherwise it returns the size of the data buffer in bytes.
*/
VOID CALLBACK TimerCompletionRoutine(LPVOID lpArg,   DWORD dwTimerLowValue,   DWORD dwTimerHighValue )
{
	static char buffer[100000];
	static LONG actualsize;
	static int Size;

	// Pointer to the calling object
	CWaveIn * ThisWaveIn = (CWaveIn *)lpArg;

	// Get next data buffer. If size of buffer is 0 or error occured - set flag and return
	actualsize = ThisWaveIn->GetWaveDataFromLogger((void *)buffer, &Size);
	if (actualsize<=0)
	{
		ThisWaveIn->m_PlayEnded = true;
		return;
	};

#ifdef DEBUG
	static int 	_nSamples, _nBytes;
	static SYSTEMTIME _Start, _Current;
	GetSystemTime(&_Current);
	if (!_nSamples) GetSystemTime(&_Start);
	_nSamples++;
	_nBytes+=actualsize;
#endif

	// Normal operation - process data
	int Length = actualsize / Size;


	if (Size == 1) /* 8-bit per sample. Value range: 0-255 */
		for (int i = 0; i < Length; i++) 
			ThisWaveIn->ProcessData((unsigned char)buffer[i]);
	else if (Size == 2)  /* 16-bit per sample. Value range: -32k - +32k */
		for (int i = 0; i < Length; i++) 
			ThisWaveIn->ProcessData(((signed short*)(buffer))[i]);
}

//---------------------------------------------------------------------------
/*Calculate audio threshold
	____________________________________________________________________
	Based on RCAudio V 3.0 and original Smartpropo
	copyright (C) Philippe G.De Coninck 2007
	
	Copied from: http://www.rcuniverse.com/forum/m_3413991/tm.htm
	____________________________________________________________________
*/
_inline double CalcThreshold(int value, int * min, int * max)
{
	// RCAudio V 3.0 : (C) Philippe G.De Coninck 2007

	static double aud_max_val, aud_min_val;
	double average;
	static int h,l; /* Values representing HIGH/LOW pulse length */
	double delta_max = fabs(value - aud_max_val); /* Absolute delta between input and MAX */
	double delta_min = fabs(value - aud_min_val); /* Absolute delta between input and MIN */

	/**********  MIN/MAX values follow the drift in the HIGH/LOW values **********/
	/*
		If signal is closer to the minimum then the signal is at its LOW phase
		Adjust the minimum to be closer to the current value.
		Slowly, the minimum will follow the drift in the value of the low phase
		In addition, increse the length of the LOW pulse by one and set the value of the HIGH pulse to 0
	*/
	if (delta_max > delta_min)
	{
		aud_min_val = (4*aud_min_val + value)/5;
		h=0;
		l++;
	}

	/*
		If signal is closer to the maximum then the signal is at its HIGH phase
		Adjust the maximum to be closer to the current value.
		Slowly, the maximum will follow the drift in the value of the high phase
		In addition, increse the length of the HIGH pulse by one and set the value of the LOW pulse to 0
	*/
	else 
	{
		aud_max_val = (4*aud_max_val + value)/5;
		l=0;
		h++;
	};
	/******************************************************************************/

	/* If the HIGH or LOW pulse are rediculously long, reset everything */
	if (h>10000 || l>10000)
		aud_min_val=aud_max_val=h=l=0;

	/* If the Minimum & maximum are too close (or switched) then spread them appart */
	if (aud_max_val < aud_min_val + 2) 
	{
		aud_max_val = aud_min_val + 1;
		aud_min_val = aud_min_val - 1;
	}

	/* Threshold */
	average = (aud_max_val + aud_min_val)/2;

	/* Return the average of the MIN/MAX as threshold and the MIN/MAX for debug */
	*min = (int)aud_min_val;
	*max = (int)aud_max_val;
	return(average); 
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


CWaveIn::CWaveIn(void)
{
	m_id = -1;
	m_DeviceName = NULL;
	m_hWaveIn = NULL;
	m_waveFmt.cbSize = 0;
	m_waveBufSize = SIZE_WAVEIN_BUF;
	m_ProcessPulse = &CWaveIn::ProcessPulsePpm;
	m_JoystickTargetFunc = NULL;
	m_JoystickTargetObj = NULL;
	m_nPositions = MAX_JS_CH;
	m_ValidPositions = false;
	m_LogWr = NULL;
	m_LogRd = NULL;
	m_PlayStopped = false;
	m_PlayEnded = false;
	m_AntiJitterEnabled = true;
	for (int i=0; i<N_WAVEIN_BUF; i++)
		m_waveBuf[i] = NULL;
	m_RawPulseLogStat = m_AudioHeaderLogStat = m_PulseLogStat = Idle;
	m_pPulseLogFile = m_pRawPulseLogFile = m_pAudioHeaderLogFile = NULL;
}

bool CWaveIn::Init(int DeviceId)
{
	WAVEINCAPS caps;
	MMRESULT res;

	// Start the thread that runs the WaveIn callback procedure
	HANDLE hWaveInThread = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)g_WaveInThreadProc, (LPVOID *)this, CREATE_SUSPENDED, &m_waveInThreadId);
	if (!hWaveInThread)
		return false;

	/* Get the Thread ID so that the additional thread could post messages to it */
	m_ThreadId = GetCurrentThreadId();

	/* Elevate thread priority */
	::SetThreadPriority(hWaveInThread, THREAD_PRIORITY_HIGHEST);

	/* Get Mixer capabilities */
	res = waveInGetDevCaps(DeviceId, &caps, sizeof(WAVEINCAPS));

	// Device Name & ID
	m_id = DeviceId;
	m_DeviceName = strdup(caps.szPname);

	// Format
	if ((caps.dwFormats & WAVE_FORMAT_96S16) || (caps.dwFormats & WAVE_FORMAT_96M16))
	{
		m_waveFmt.nSamplesPerSec = 96000;
		m_waveFmt.wBitsPerSample = 16;
	}
	else if ((caps.dwFormats & WAVE_FORMAT_48S16) || (caps.dwFormats & WAVE_FORMAT_48M16))
	{
		m_waveFmt.nSamplesPerSec = 48000;
		m_waveFmt.wBitsPerSample = 16;
	}
	else if ((caps.dwFormats & WAVE_FORMAT_96S08) || (caps.dwFormats & WAVE_FORMAT_96M08))
	{
		m_waveFmt.nSamplesPerSec = 96000;
		m_waveFmt.wBitsPerSample = 8;
	}
	else if ((caps.dwFormats & WAVE_FORMAT_48S08) || (caps.dwFormats & WAVE_FORMAT_48M08))
	{
		m_waveFmt.nSamplesPerSec = 48000;
		m_waveFmt.wBitsPerSample = 8;
	}
	else 
	{
		m_waveFmt.nSamplesPerSec = 44100;
		m_waveFmt.wBitsPerSample = 16;
	};

	/* Wave format structure initialization */
    m_waveFmt.wFormatTag = WAVE_FORMAT_PCM;	/* Wave format = PCM	*/
    m_waveFmt.nChannels = 1;					/* Mono					*/
    m_waveFmt.nBlockAlign = m_waveFmt.wBitsPerSample / 8 *  m_waveFmt.nChannels;
    m_waveFmt.nAvgBytesPerSec =  m_waveFmt.nSamplesPerSec *  m_waveFmt.nBlockAlign;
    m_waveFmt.cbSize = 0;

	/*	
		Open the WaveIn device according to the device ID
		The handle will now reside in pWaveIn->waveIn and the device will call a callback thread (waveInThreadId) 
	*/ 
	m_hWaveIn = 0;
	/* Open audio stream, assigning 'waveInProc()' as the WAVE IN callback function*/
	res = waveInOpen(&(m_hWaveIn), DeviceId, &(m_waveFmt), (DWORD_PTR)(waveInProc), (DWORD_PTR)m_waveInThreadId, CALLBACK_FUNCTION);
	if (res || !m_hWaveIn)
		return false;

	/*	Creating the buffers for the device to fill with data */
    for (int i = 0; i < N_WAVEIN_BUF; i++) {
        m_waveBuf[i] = (WAVEHDR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEHDR));
        m_waveBuf[i]->lpData = (char*)HeapAlloc(GetProcessHeap(), 0, m_waveBufSize);
        m_waveBuf[i]->dwBufferLength = m_waveBufSize;
        m_waveBuf[i]->dwUser = i;
		m_waveBuf[i]->dwFlags = 0;
        res = waveInPrepareHeader(m_hWaveIn, m_waveBuf[i], sizeof(WAVEHDR));
		if (res)
			return false;
        res = waveInAddBuffer(m_hWaveIn, m_waveBuf[i], sizeof(WAVEHDR));
		if (res)
			return false;
    };

	::ResumeThread(hWaveInThread);
	return true;

}

bool CWaveIn::Start()
{
	MMRESULT res;

	/* Start running the device */
	if (!m_hWaveIn)
		return false;
    res = waveInStart(m_hWaveIn);
	if (res != MMSYSERR_NOERROR)
		return false;

	return true;
}

bool CWaveIn::Stop()
{
	MMRESULT res;

	/* Start running the device */
	if (!m_hWaveIn)
		return false;
    res = waveInStop(m_hWaveIn);
	if (res != MMSYSERR_NOERROR)
		return false;

	return true;
}

int CWaveIn::GetId()
{
	return m_id;
}


/* Walkera helper functions */

/* Convert pulse width to binary value */
unsigned char  CWaveIn::WalkeraConvert2Bin(int width)
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
unsigned char  CWaveIn::WalkeraConvert2Oct(int width)
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
int CWaveIn::WalkeraElevator(const unsigned char * cycle)
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
int CWaveIn::WalkeraAilerons(const unsigned char * cycle)
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
int CWaveIn::WalkeraThrottle(const unsigned char * cycle)
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
int CWaveIn::WalkeraRudder(const unsigned char * cycle)
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
int CWaveIn::WalkeraGear(const unsigned char * cycle)
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
int CWaveIn::WalkeraPitch(const unsigned char * cycle)
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


int CWaveIn::WalkeraGyro(const unsigned char * cycle)
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

int CWaveIn::WalkeraChannel8(const unsigned char * cycle)
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
int * CWaveIn::WalkeraCheckSum(const unsigned char * cycle)
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
int CWaveIn::ProcessPulseWalPcm(int width, BOOL input)
{
	static int nPulse;
	static unsigned char cycle[50];
	int Elevator=0, Ailerons=0,Throttle=0,Rudder=0,Gear=0, Pitch=0, Gyro=0, Ch8=0;
	const int fixed_n_channel = 8;
	int vPulse;
	int * cs;


	/* Detect Sync pulse - if detected then reset pulse counter and return */
	if (width>56 && width<70)
	{
		DebugPrintRawPulses("ProcessPulseWalPcm", width, (unsigned char *)cycle, 50);
		if (nPulse==0 && width<300) m_ValidPositions = true;
		nPulse = 1;
		return 2;
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
			m_Position[0] = smooth(m_Position[0], Elevator);
			m_Position[1] = smooth(m_Position[1], Ailerons);
			m_Position[2] = smooth(m_Position[2], Throttle);
			m_Position[3] = smooth(m_Position[3], Rudder);
			//m_ValidPositions = true;
		}

		/* Copy data to joystick positions if checksum is valid (ch5-ch8) */
		else if ((cs[2] == cycle[47]) && (cs[3] == cycle[48]))
		{
			m_Position[4] = smooth(m_Position[4], Gear);
			m_Position[5] = smooth(m_Position[5], Pitch);
			m_Position[6] = smooth(m_Position[6], Gyro);
			m_Position[7] = smooth(m_Position[7], Ch8);
			//m_ValidPositions = true;
		} 
		else
			m_ValidPositions = false;
		nPulse = 0;
	};

	m_nPositions = 8;
	return 1;
}
/* Helper function - Airtronic/Sanwa PCM1 data convertor */
int  __fastcall CWaveIn::Convert15bits(unsigned int in)
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
int CWaveIn::ProcessPulseAirPcm1(int width, BOOL input)
{
	int pulse;
	static int sync;
	static int datacount = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
	static chunk = -1;
	unsigned int		shift;
    static unsigned int data[10];
	static int i = 0;
	const int fixed_n_channel = 8;
	static int Accumulate=0;
	static int StaleCount=11;

	DebugPrintRawPulses("ProcessPulseAirPcm1", width*(1-2*input), (const unsigned char  *)data, 10,sizeof(int));		

	// If data was stale for more than 10 consecutive times then the positions are declared invalid
	if (width>200 || StaleCount++>80)
		m_ValidPositions = false;
	else
		m_ValidPositions = true;

		pulse = width/8; // Width to bits
		Accumulate += pulse;
		if (pulse == 4)  // 4-bit pulse marks a bigining of a data chunk
		{
			if (!input)
			{
				// First data chunk - clear chunnel counter
				datacount = 0;
			}
			else
			{	// Second data chunk - get joystick position from channel data
				m_Position[0] = smooth(m_Position[0], Convert15bits(data[1]));
				m_Position[1] = smooth(m_Position[1], Convert15bits(data[2]));
				m_Position[2] = smooth(m_Position[2], Convert15bits(data[3]));
				m_Position[3] = smooth(m_Position[3], Convert15bits(data[4]));
				m_Position[4] = smooth(m_Position[4], Convert15bits(data[6]));
				m_Position[5] = smooth(m_Position[5], Convert15bits(data[7]));
				m_Position[6] = smooth(m_Position[6], Convert15bits(data[8]));
				m_Position[7] = smooth(m_Position[7], Convert15bits(data[9]));
			};
			sync = 1;		// Sync bit is set for the first 10 bits of the chunk (No channel data here)
			bitstream = 0;
			bitcount = -1;
			chunk = input;	// Mark chunk polarity - 0: Low channels, 1: High channels

			// Accumulate should be 70 or 71 here - every miss is counted as stale data
			if (Accumulate>68 && Accumulate<72)
				StaleCount=0;
			Accumulate=0;
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

		m_nPositions = 8;
		return 1;
}

//#elif defined AIR_PCM2
/* Helper function - Airtronic/Sanwa PCM2 data convertor */
int  __fastcall CWaveIn::Convert20bits(int in)
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
int CWaveIn::ProcessPulseAirPcm2(int width, BOOL input)
{
	int pulse;
	static int sync;
	static int datacount = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
	static chunk = -1;
	unsigned int		shift;
    static unsigned int data[10];
	static int i = 0;
	static int StaleCount=0;

	DebugPrintRawPulses("ProcessPulseAirPcm2", width*(1-2*input), (const unsigned char  *)data, 10,sizeof(int));

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
				m_Position[0] = smooth(m_Position[0], Convert20bits(data[2]));
				m_Position[1] = smooth(m_Position[1], Convert20bits(data[7]));
				m_Position[2] = smooth(m_Position[2], Convert20bits(data[6]));
				m_Position[3] = smooth(m_Position[3], Convert20bits(data[3]));
				m_Position[4] = smooth(m_Position[4], Convert20bits(data[1]));
				m_Position[5] = smooth(m_Position[5], Convert20bits(data[5]));
			};
			sync = 1;		// Sync bit is set for the first 10 bits of the chunk (No channel data here)
			bitstream = 0;
			bitcount = -1;
			chunk = input;	// Mark chunk polarity - 0: Low channels, 1: High channels
			StaleCount=0;
			m_ValidPositions = true;
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
			if (StaleCount++ > 100)
				m_ValidPositions = false;
		};

		m_nPositions = 6;
		return 1;
}



int CWaveIn::ProcessPulseJrPcm(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
    static int data[30];
    static int datacount = 0;
	static int i = 0;
	static int StaleCount=0;

	if (width>200 || StaleCount++>10)
		m_ValidPositions = false;
	else
		m_ValidPositions = true;

    if (sync == 0 && (int)floor(2.0 * width / PW_JR + 0.5) == 5) {
		DebugPrintRawPulses("ProcessPulseJrPcm", width, (const unsigned char  *)data, 30,sizeof(int));
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
        case 3:  m_Position[2] = 1023 - ((data[1] << 5) | data[2]); break;
        case 6:  m_Position[0] = 1023 - ((data[4] << 5) | data[5]);  break;
        case 11: m_Position[5] = 1023 - ((data[9] << 5) | data[10]); break;
        case 14: m_Position[7] = 1023 - ((data[12] << 5) | data[13]); break;
        case 18: m_Position[3] = 1023 - ((data[16] << 5) | data[17]); break;
        case 21: m_Position[1] = 1023 - ((data[19] << 5) | data[20]); break;
        case 26: m_Position[4] = 1023 - ((data[24] << 5) | data[25]); break;
        case 29: m_Position[6] = 1023 - ((data[27] << 5) | data[28]); break;
        case 30: sync = 0;
    };

	m_nPositions = 8;
	 return 1;
}


//---------------------------------------------------------------------------

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

   The first 8 channels (Analog) are position are the POS field as follows:
	Ch1: POS[0] (even frame) - if bit[5] of AUX[0] is '1'
	Ch2: POS[4] (odd  frame) - if bit[5] of AUX[4] is '1'
	Ch3: POS[1] (even frame) - if bit[5] of AUX[0] is '1'
	Ch4: POS[5] (odd  frame) - if bit[5] of AUX[4] is '1'
	Ch5: POS[2] (even frame) - if bit[5] of AUX[2] is '1'
	Ch6: POS[6] (odd  frame) - if bit[5] of AUX[6] is '1'
	Ch7: POS[3] (even frame) - if bit[5] of AUX[2] is '1'
	Ch8: POS[7] (odd  frame) - if bit[5] of AUX[6] is '1'

   The next channels (binary)  are mapped like this:
    CH9:  Bit 0 of AUX[3] lower bit - if bit[5] of AUX[2] is '1'
    CH10: Bit 0 of AUX[5] lower bit - if bit[5] of AUX[4] is '1'

	---------------------------------------------------------------------------------------------------------
	For historic reasons, the channel numbers in this code do not conform to the official Futaba channel numbers
	The channels in this code are mapped into array Position[0-9]:
	Position[0] = Ch4 (Packet 5)
	Position[1] = Ch2 (Packet 4)
	Position[2] = Ch1 (Packet 0)
	Position[3] = Ch3 (Packet 1)
	Position[4] = Ch5 (Packet 2)
	Position[5] = Ch6 (Packet 6)
	Position[6] = Ch7 (Packet 3)
	Position[7] = Ch8 (Packet 7)

*/
/* 
	General fix in version 3.4.0 
	From this version the condition to read data changed to: 'if (data[0] & 0x20)'
	which is correct and prevents "hickups"
	Thanks to Niels Laukens of Belgium!
*/
int CWaveIn::ProcessPulseFutabaPcm(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bit = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;

    static int data[32];
    static int datacount = 0;
	static int i = 0;
	static int StaleCount=0;

    width = (int)floor(width / PW_FUTABA + 0.5);

	/* 
		Sync is determined as 18-bit wide pulse 
		If detected, move state machine to sync=1 (wait for parity marker) 
		and initialize all static params
	*/
    if (sync == 0 && width == 18) {
		DebugPrintRawPulses("ProcessPulseFutabaPcm", width, (const unsigned char  *)data, 32,sizeof(int));
        sync = 1;
        bit = 0;
        bitstream = 0;
        bitcount = 0;
        datacount = 0;	
		StaleCount = 0;
        return 0;
    }

	if (StaleCount++>10)
		m_ValidPositions = false;
	else
		m_ValidPositions = true;

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
			//Position[1] += futaba_delta[data[0] & 0x0F];         /* Ch2: Delta: packet[2:5] */
			if (data[0] & 0x20)									  /* More accurate condition: Fixed: version 3.4.0 */
				m_Position[2] = (data[1] << 4) | (data[2] >> 2);    /* Ch1: Position: packet[6:15] */
			break;

        case 7:  /* 4th sextet of second packet is ready */
			//m_Position[0] += futaba_delta[data[4] & 0x0F];         /* Ch0: Delta: packet[2:5] */
			if (data[0] & 0x20)
				m_Position[3] = (data[5] << 4) | (data[6] >> 2);    /* Ch3: Position: packet[6:15] */
			break;

        case 11:  /* 4th sextet of 3rd packet is ready */
			//m_Position[5] += futaba_delta[data[8] & 0x0F];          /* Ch6: Delta: packet[2:5] */
			if (data[8] & 0x20)
				m_Position[4] = (data[9] << 4) | (data[10] >> 2);     /* Ch5: Position: packet[6:15] */
			break;

        case 15:   /* 4th sextet of 4th packet is ready */
			//m_Position[7] += futaba_delta[data[12] & 0x0F];          /* Ch8: Delta: packet[2:5] */
			if (data[8] & 0x20)
			{
				m_Position[6] = (data[13] << 4) | (data[14] >> 2);	/* Ch7: Position: packet[6:15] */
				m_Position[8] = ((data[12]>>4)&1)*768;				/* Ch9: One of the auxilliary bits */ /* Fixed: version 3.4.0 */
			};
			sync = 0; /* End of even frame. Wait for sync */
			break;

		/* Odd frame */
        case 19:    /* 4th sextet of 5th packet is ready */
			//m_Position[2] += futaba_delta[data[16] & 0x0F];            /* Ch2: Delta: packet[2:5] */
			if (data[16] & 0x20)
				m_Position[1] = (data[17] << 4) | (data[18] >> 2);     /* Ch2: Position: packet[6:15] */
			break;
        case 23:    /* 4th sextet of 6th packet is ready */
			//m_Position[3] += futaba_delta[data[20] & 0x0F];            /* Ch4: Delta: packet[2:5] */
			if (data[16] & 0x20)
			{
				m_Position[0] = (data[21] << 4) | (data[22] >> 2);		/* Ch4: Position: packet[6:15] */
				m_Position[9] = ((data[29]>>4)&1)*768;					/* Ch10: One of the auxilliary bits *//* Fixed version 3.4.0 */
			}
			break;
        case 27:    /* 4th sextet of 7th packet is ready */
			//m_Position[4] += futaba_delta[data[24] & 0x0F];            /* Ch5: Delta: packet[2:5] */
			if (data[24] & 0x20)
				m_Position[5] = (data[25] << 4) | (data[26] >> 2);     /* Ch6: Position: packet[6:15] */
			break;
        case 31:    /* 4th sextet of 8th packet is ready */
			//m_Position[6] += futaba_delta[data[28] & 0x0F];            /* Ch7: Delta: packet[2:5] */
			if (data[24] & 0x20)
				m_Position[7] = (data[29] << 4) | (data[30] >> 2);     /* Ch8: Position: packet[6:15] */
			break;
        case 32: sync = 0;/* End of odd frame. Wait for sync */
			
    };

	m_nPositions = 10;
	return 1;
}
//---------------------------------------------------------------------------

/*
	Process PPM pulse
	A long (over 200 samples) leading high, then a short low, then up to six highs followed by short lows.
	The size of a high pulse may vary between 30 to 70 samples, mapped to joystick values of 1024 to 438
	where the mid-point of 50 samples is translated to joystick position of 731.
*/
int CWaveIn::ProcessPulsePpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */
	static int StaleCount=0;

	DebugPrintRawPulses("ProcessPulsePpm", width*(1-2*input), (const unsigned char  *)data, 14,sizeof(int));

	if (width < 5)
		return 0;

	if (StaleCount++>50)
		m_ValidPositions = false;
	else
		m_ValidPositions = true;

	/* If pulse is a separator then go to the next one */
	if (width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return 0;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
	if (width > PPM_TRIG) {
		sync = 1;
		m_nPositions = datacount;

		/* JR - Assign data to joystick channels */
		if (input)
			for (int iCh=0; iCh<m_nPositions; iCh++)
				m_Position[iCh] = data[iCh];

		/* Futaba - Assign data to joystick channels */
		else
			for (int iCh=0; iCh<m_nPositions; iCh++)
				switch (iCh)
			{
				case 0: 	m_Position[1]  = data[iCh];	break;/* Assign data to joystick channels */
				case 1: 	m_Position[2]  = data[iCh];	break;/* Assign data to joystick channels */
				case 2: 	m_Position[0]  = data[iCh];	break;/* Assign data to joystick channels */
				case 3: 	m_Position[3]  = data[iCh];	break;/* Assign data to joystick channels */
				case 4: 	m_Position[4]  = data[iCh];	break;/* Assign data to joystick channels */
				case 5: 	m_Position[5]  = data[iCh];	break;/* Assign data to joystick channels */
				case 6: 	m_Position[6]  = data[iCh];	break;/* Assign data to joystick channels */
				case 7: 	m_Position[7]  = data[iCh];	break;/* Assign data to joystick channels */
				case 8: 	m_Position[8]  = data[iCh];	break;/* Assign data to joystick channels */
				case 9: 	m_Position[9]  = data[iCh];	break;/* Assign data to joystick channels */
				case 10: 	m_Position[10] = data[iCh];	break;/* Assign data to joystick channels */
				case 11: 	m_Position[11] = data[iCh];	break;/* Assign data to joystick channels */
			};	// Switch

		StaleCount=0;
		datacount = 0;
		former_sync = 1;
		return 1;
    }

    if (!sync) 
		return 0; /* still waiting for sync */

	// Cancel jitter /* Version 3.3.3 */
	if (m_AntiJitterEnabled)
	{
	if (abs(PrevWidth[datacount] - width) < 2)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;
	};


	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input/* || JsChPostProc_selected!=-1 */)
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

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return 2;
}

int CWaveIn::ProcessPulseFutabaPpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */
	static int StaleCount=0;

	DebugPrintRawPulses("ProcessPulseFutabaPpm", width*(1-2*input), (const unsigned char  *)data, 14,sizeof(int));
	if (width < 5)
		return 0;

	if (StaleCount++>200)
		m_ValidPositions = false;
	else
		m_ValidPositions = true;

	//if (gDebugLevel>=2 /*&& gCtrlLogFile && !(i++%50)*/)
	//	fprintf(gCtrlLogFile,"\n%s - ProcessPulseFutabaPpm(width=%d, input=%d)", _strtime( tbuffer ), width, input);

	/* If pulse is a separator then go to the next one */
	if (!input || width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return 0;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (input &&  width > PPM_TRIG) {
        sync = 1;
		m_nPositions = datacount;

		/* JR - Assign data to joystick channels */
		if (input)
			for (int iCh=0; iCh<m_nPositions; iCh++)
				m_Position[iCh] = data[iCh];

		/* Futaba - Assign data to joystick channels */
		else
			for (int iCh=0; iCh<m_nPositions; iCh++)
				switch (iCh)
			{
				case 0: 	m_Position[1]  = data[iCh];	break;/* Assign data to joystick channels */
				case 1: 	m_Position[2]  = data[iCh];	break;/* Assign data to joystick channels */
				case 2: 	m_Position[0]  = data[iCh];	break;/* Assign data to joystick channels */
				case 3: 	m_Position[3]  = data[iCh];	break;/* Assign data to joystick channels */
				case 4: 	m_Position[4]  = data[iCh];	break;/* Assign data to joystick channels */
				case 5: 	m_Position[5]  = data[iCh];	break;/* Assign data to joystick channels */
				case 6: 	m_Position[6]  = data[iCh];	break;/* Assign data to joystick channels */
				case 7: 	m_Position[7]  = data[iCh];	break;/* Assign data to joystick channels */
				case 8: 	m_Position[8]  = data[iCh];	break;/* Assign data to joystick channels */
				case 9: 	m_Position[9]  = data[iCh];	break;/* Assign data to joystick channels */
				case 10: 	m_Position[10] = data[iCh];	break;/* Assign data to joystick channels */
				case 11: 	m_Position[11] = data[iCh];	break;/* Assign data to joystick channels */
			};	// Switch

		StaleCount=0;
        datacount = 0;
		former_sync = 1;
		return 1;
    }

    if (!sync) return 0; /* still waiting for sync */

	// Cancel jitter /* Version 3.4.0 */
	if (m_AntiJitterEnabled)
	{
	if (abs(PrevWidth[datacount] - width) < 2)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;
	};

	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input/* || JsChPostProc_selected!=-1 */)
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

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return 2;
}

int CWaveIn::ProcessPulseJrPpm(int width, BOOL input)
{
    static int sync = 0;

    int newdata;				/* Current width in joystick values */
    static int data[14];		/* Array of pulse widthes in joystick values */
    static int datacount = 0;	/* pulse index (corresponds to channel index) */
	static int former_sync = 0;
	static int i = 0;
	static int PrevWidth[14];	/* array of previous width values */
	static int StaleCount=0;

	DebugPrintRawPulses("ProcessPulseJrPpm", width*(1-2*input), (const unsigned char  *)data, 14,sizeof(int));
	if (width < 5)
		return 0;

	if (StaleCount++>200)
		m_ValidPositions = false;
	else
		m_ValidPositions = true;

	/* If pulse is a separator then go to the next one */
	if (input || width < PPM_SEP+7 || former_sync)
	{
		former_sync = 0;
		return 0;
	};

	/* sync is detected at the end of a very long pulse (over 200 samples = 4.5mSec) */
    if (!input && width > PPM_TRIG) {
        sync = 1;
		m_nPositions = datacount;

		/* JR - Assign data to joystick channels */
		if (input)
			for (int iCh=0; iCh<m_nPositions; iCh++)
				m_Position[iCh] = data[iCh];

		/* Futaba - Assign data to joystick channels */
		else
			for (int iCh=0; iCh<m_nPositions; iCh++)
				switch (iCh)
			{
				case 0: 	m_Position[1]  = data[iCh];	break;/* Assign data to joystick channels */
				case 1: 	m_Position[2]  = data[iCh];	break;/* Assign data to joystick channels */
				case 2: 	m_Position[0]  = data[iCh];	break;/* Assign data to joystick channels */
				case 3: 	m_Position[3]  = data[iCh];	break;/* Assign data to joystick channels */
				case 4: 	m_Position[4]  = data[iCh];	break;/* Assign data to joystick channels */
				case 5: 	m_Position[5]  = data[iCh];	break;/* Assign data to joystick channels */
				case 6: 	m_Position[6]  = data[iCh];	break;/* Assign data to joystick channels */
				case 7: 	m_Position[7]  = data[iCh];	break;/* Assign data to joystick channels */
				case 8: 	m_Position[8]  = data[iCh];	break;/* Assign data to joystick channels */
				case 9: 	m_Position[9]  = data[iCh];	break;/* Assign data to joystick channels */
				case 10: 	m_Position[10] = data[iCh];	break;/* Assign data to joystick channels */
				case 11: 	m_Position[11] = data[iCh];	break;/* Assign data to joystick channels */
			};	// Switch

		StaleCount=0;
        datacount = 0;
		former_sync = 1;
		return 1;
    }

    if (!sync) return 0; /* still waiting for sync */

	// Cancel jitter /* Version 3.4.0 */
	if (m_AntiJitterEnabled)
	{
	if (abs(PrevWidth[datacount] - width) < 2)
		width = PrevWidth[datacount];
	PrevWidth[datacount] = width;
	};

	/* convert pulse width in samples to joystick position values (newdata)
	joystick position of 0 correspond to width over 100 samples (2.25mSec)
	joystick position of 1023 correspond to width under 30 samples (0.68mSec)*/
	if (input /* || JsChPostProc_selected!=-1 */)
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

	if (datacount == 11)	sync = 0;			/* Reset sync after channel 12 */

    datacount++;
	return 2;
}


/*
	ProcessData - process a single audio sample of unknown type (8-16 bits mono)
	The audio sample may be 8-bit PCM, ranging 0-255, mid-point is 128
	The audio sample may be 16-bit PCM, ranging from -32768 to 32767, mid-point is 0
	The minimal step is 1
*/
void __fastcall CWaveIn::ProcessData(int i)
{
	static int min=0, max=0;
    static int high = 0;	/* Number of contingious above-threshold samples */
    static int low = 0;		/* Number of contingious below-threshold samples */
    double threshold;		/* calculated mid-point */
	static int FormerInput;
	static int volume=0;	/* Signal mean volume */
	int	PpRes;

	threshold		= CalcThreshold(i, &min, &max);								/* Version 3.4.0 - Calculate threashold and set min/max for debug */
	m_AudioLevel	= CalcAudioLevel(i, m_waveFmt.wBitsPerSample);				// Calculating signal strength 

	/* Update the width of the number of the low/high samples */
	/* If edge, then call ProcessPulse() to process the previous high/low level */
    if (i > threshold) 
	{
	high++;
        if (low) 
		{
			low=low*44100/m_waveFmt.nSamplesPerSec; // Normalize number of sumples
			PpRes = (*this.*m_ProcessPulse)(low, FALSE);
			SendJoystickData();
			DebugPrintPulses(0-low, threshold);
            low = 0;
        }
    } else 
	{
        low++;
        if (high) 
		{
			high=high*44100/m_waveFmt.nSamplesPerSec; // Normalize number of sumples
            PpRes = (*this.*m_ProcessPulse)(high, TRUE);
			SendJoystickData();
			DebugPrintPulses(high, threshold);
            high = 0;
        }
    }
}


void FAR CWaveIn::ProcessBuffer(WAVEHDR * hdr, HWAVEIN hDevice)
{
	if (m_hWaveIn != hDevice)
		return;

	// Logging data into WAV file
	if (m_LogWr)
		m_LogWr->PutWaveData((void *)(hdr->lpData), hdr->dwBytesRecorded);

	int Size = m_waveFmt.nBlockAlign;
	int Length = hdr->dwBytesRecorded / Size;

	if (Size == 1) /* 8-bit per sample. Value range: 0-255 */
		for (int i = 0; i < Length; i++) 
			ProcessData((unsigned char)hdr->lpData[i]);
	else if (Size == 2)  /* 16-bit per sample. Value range: -32k - +32k */
		for (int i = 0; i < Length; i++) 
			ProcessData(((signed short*)(hdr->lpData))[i]);

}

void FAR CWaveIn::ResetBuffer(WAVEHDR * hdr)
{
	/* Requests the audio device to refill the current buffer */
	//hdr->dwFlags = 0;
	//waveInPrepareHeader(m_hWaveIn, hdr, sizeof(WAVEHDR));
	hdr->dwBufferLength = m_waveBufSize;
	waveInAddBuffer(m_hWaveIn, hdr , sizeof(WAVEHDR));
}

bool CWaveIn::SetModulation(const MODE Type)
{
	int (CWaveIn::*ProcessPulse)(int, BOOL) = NULL;
	m_ValidPositions = false;

	switch (Type)
	{
		case PpmAuto:		ProcessPulse = &CWaveIn::ProcessPulsePpm; break;
		case PpmNeg:		ProcessPulse = &CWaveIn::ProcessPulseFutabaPpm; break;
		case PpmPos:		ProcessPulse = &CWaveIn::ProcessPulseJrPpm; break;
		case PcmJr:			ProcessPulse = &CWaveIn::ProcessPulseJrPcm; break;
		case PcmFut:		ProcessPulse = &CWaveIn::ProcessPulseFutabaPcm; break;
		case PcmAir1:		ProcessPulse = &CWaveIn::ProcessPulseAirPcm1; break;
		case PcmAir2:		ProcessPulse = &CWaveIn::ProcessPulseAirPcm2; break;
		case PcmWalkera:	ProcessPulse = &CWaveIn::ProcessPulseWalPcm; break;
		default: return false;
	};

	// TODO: Sync this
	m_ProcessPulse = ProcessPulse;
	m_ValidPositions = false;
	return true;
}

// Set the callback function to call when joystick data has changed
void CWaveIn::SetJoystickEventCallback(void FAR * callbackfunc, void FAR * Obj)
{
	m_JoystickTargetObj =	(Wave2Joystick *)Obj;
	m_JoystickTargetFunc =	callbackfunc;
}

/*
	Debug function
	Creates file 'SppDeb1.txt' in current directory
	First column: Number of channels ('*' marks a change in data)
	Next columns: Channel data

	Notes: 
	1. Not a large file. Feel free to use
	2. New file overrides previous one
*/
void CWaveIn::DebugPrintChannels(bool changed)
{
	static HANDLE hFileDebug;
	char buffer[10000];
	DWORD bWritten;
	char Simbol = ' ';
	// Init+header
	if (!hFileDebug)
	{
		hFileDebug = CreateFile("C:\\Documents and Settings\\Shaul\\Local Settings\\TEMP\\SppDeb1.txt", FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
		sprintf(buffer, "nCH\tCh1\tCh2\tCh3\tCh4\tCh5\tCh6");
		WriteFile(hFileDebug, buffer, (DWORD)strlen(buffer), (LPDWORD)&bWritten, (LPOVERLAPPED)NULL);
	}
	if (changed)
		Simbol = '*';
	sprintf(buffer, "\n%c%02d\t%05d\t%05d\t%05d\t%05d\t%05d\t%05d", Simbol, m_nPositions,m_Position[0] ,m_Position[1],m_Position[2],m_Position[3],m_Position[4] ,m_Position[5]);
	WriteFile(hFileDebug, buffer, (DWORD)strlen(buffer), (LPDWORD)&bWritten, (LPOVERLAPPED)NULL);
}
// Print Pulses
void inline CWaveIn::DebugPrintPulses(int Length, double Threshold)
{
	if ((m_PulseLogStat != Printing) && (m_PulseLogStat != Started)) return;
	if (m_PulseLogStat == Started) m_PulseLogStat = Printing;

	char Simbol=' ';
	static int PrevNPos;
	if (PrevNPos && PrevNPos != m_nPositions)
		Simbol='*';
	else 
		Simbol=' ';
	PrevNPos = m_nPositions;
	fprintf(m_pPulseLogFile, "%c[%2d]\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\t%04d\t\t%05d\t%05d\n", Simbol, m_nPositions,m_Position[0] ,m_Position[1],m_Position[2],m_Position[3],m_Position[4] ,m_Position[5],m_Position[6] ,m_Position[7], Length, (int)Threshold);
}

// Print WaveIn header data
inline void CWaveIn::DebugPrintWaveHdr(WAVEHDR * hdr, LONG MsgTime)
{
	if ((m_AudioHeaderLogStat != Printing) && (m_AudioHeaderLogStat != Started)) return;
	if (m_AudioHeaderLogStat == Started) m_AudioHeaderLogStat = Printing;

	static DWORD PrevUser=N_WAVEIN_BUF-1;
	static LONG PrevMsgTime;


	/* Print error message if buffer is missing (skipped) */
	if ((hdr->dwUser-PrevUser) !=1 && (hdr->dwUser-PrevUser) !=1-N_WAVEIN_BUF)
	{
		if (PrevUser==N_WAVEIN_BUF-1)
			fprintf(m_pAudioHeaderLogFile,"Error: Missing buffer %d\n", 0);
		else
			fprintf(m_pAudioHeaderLogFile,"Error: Missing buffer %d\n", PrevUser+1);
	};

	fprintf(m_pAudioHeaderLogFile,"%d\t\t%02d\t\t%d\t\t%03d\t\t--\t%2x\t\t%2x\t\t%2x\t\t%2x\t\t%2x\t\t%2x\t\t%2x\t\t%2x\n",\
					GetId(),hdr->dwUser, hdr->dwFlags, MsgTime-PrevMsgTime,\
					FLG(0), FLG(1), FLG(2), FLG(3), FLG(4), FLG(5), FLG(6), FLG(7));

	/* Update static values */
	PrevMsgTime = MsgTime;
	PrevUser = (DWORD)hdr->dwUser;
}

inline void CWaveIn::DebugPrintRawPulses(const char * FunctionName, const int width, const unsigned char * data, const int nData, int size)
{
	if ((m_RawPulseLogStat != Printing) && (m_RawPulseLogStat != Started)) return;
	if (m_RawPulseLogStat == Started) m_RawPulseLogStat = Printing;

	char tbuffer [9];
	fprintf(m_pRawPulseLogFile,"\n%s - %s(%4d) [P=%d] ",  _strtime( tbuffer ), FunctionName, width, nData);
	if (size == sizeof(unsigned char))
		for (int i=0; i<nData; i++) fprintf(m_pRawPulseLogFile," %02d", data[i]);
	else if (size == sizeof(int)) 
		for (int i=0; i<nData; i++) fprintf(m_pRawPulseLogFile," %02d", ((int *)data)[i]);
	fprintf(m_pRawPulseLogFile," :: [JP=%d] ",m_nPositions);
	for (int i=0; i<m_nPositions; i++) fprintf(m_pRawPulseLogFile," %02d", m_Position[i]);
}

LONG CWaveIn::GetWaveDataFromLogger(void * buffer, int * size)
{
	if (!m_LogRd)
		return 0;

	*size = m_waveFmtRd.nBlockAlign;
	return m_LogRd->GetWaveData(buffer);
}

// Send Joystick data (if channged). Returns true if sent or false if not sent
bool CWaveIn::SendJoystickData(bool force)
{
	static int	PrevPosition[MAX_JS_CH];
	bool		changed=false;
	int	nChannels = m_nPositions;
	static int counter;

	// Test if changed - return false if no change, copy new data if changed
	for (int i=0; i<m_nPositions; i++)
	{
		if (PrevPosition[i] == m_Position[i])
			continue;

		PrevPosition[i] = m_Position[i];
		changed=true;
	};

	// Clean values in unused channels
	for (int i=m_nPositions; i<MAX_JS_CH; i++)
		m_Position[i] = 1000;//i*500;

	if (++counter == 100)
	{
		counter = 0;
		force = true;
	};


	if (!changed && !force)
		return false;

	// Call callback function
	((void (*)(void *, int *, int *))m_JoystickTargetFunc)(m_JoystickTargetObj, m_Position, &nChannels);

	// Print first 6 channels
	//DebugPrintChannels(changed);
	return true;
}


// Create a WAV file for logging purpose
// If Successful: 
//		1. Return file name
//		2. m_LogWr member points to a CLogger object
// If Unsuccessful: 
//		Return file NULL

TCHAR * CWaveIn::StartWaveFileLog(void)
{
	TCHAR  WaveFileName[MAX_PATH], CurrentDir[MAX_PATH];

	// Create file name from sound card name and current directory 
	GetCurrentDirectory(MAX_PATH, CurrentDir);
	sprintf(WaveFileName,"%s\\%s.Wav", CurrentDir, m_DeviceName);
	m_LogWr = new CLogger();

	if (!m_LogWr->CreateWaveFile(WaveFileName, &m_waveFmt))
	{
		delete m_LogWr;
		m_LogWr=NULL;
		return NULL;
	};

	return strdup(WaveFileName);
}

bool CWaveIn::StopWaveFileLog(void)
{
	// Stop logging
	if (m_LogWr)
	{
		m_LogWr->CloseWaveFile();
		delete m_LogWr;
		m_LogWr = NULL;
		return true;
	};

	// Stop playback
	if (m_LogRd)
	{
		m_PlayStopped = true;
		m_LogRd->CloseWaveFile();
		delete m_LogRd;
		m_LogRd = NULL;
		return true;
	};

	return false;
}

bool CWaveIn::PlayWaveFileLog(TCHAR * WaveFileName)
{
	DWORD ThreadId;

	// Reset 
	m_PlayStopped = m_PlayEnded = false;

	// Stop reading the WaveIn input
	Stop();

	// Creat a new CLogger object then pass it as a parameter to a new 'g_ReadWaveFileThreadProc' thread
	m_LogRd = new CLogger();
	bool started = m_LogRd->StartReadWaveFile(WaveFileName, &m_waveFmtRd, POLLING_PERIOD);
	if (!started)
		return false;

	HANDLE hWaveInThread = ::CreateThread(0, 0, (LPTHREAD_START_ROUTINE)g_ReadWaveFileThreadProc, (LPVOID *)this, 0, &ThreadId);
	if (!hWaveInThread)
		return false;

	return true;
}

bool CWaveIn::StatWaveFileLog(int * stat)
{
	if (!m_PlayStopped && !m_PlayEnded)
		*stat = PLAYING;
	else if (m_PlayStopped || m_PlayEnded)
		*stat = STOPPED;
	else
		return false;

	return true;
}


TCHAR * CWaveIn::StartAudioHeaderLog(void)
{
	TCHAR  WaveFileName[MAX_PATH], CurrentDir[MAX_PATH];

	// Is it OK to start logging?
	if (m_AudioHeaderLogStat != Idle)
		return NULL;

	// Create file name from sound card name and current directory 
	GetCurrentDirectory(MAX_PATH, CurrentDir);
	sprintf(WaveFileName,"%s\\%s_Hdr.Log", CurrentDir, m_DeviceName);

	// Open file for writing
	m_pAudioHeaderLogFile = fopen(WaveFileName, "w");
	if (!m_pAudioHeaderLogFile)
		return NULL;

	// Print header
	time_t aclock;
	time( &aclock );  
	fprintf(m_pAudioHeaderLogFile,"+======================================================================================================\n");
	fprintf(m_pAudioHeaderLogFile,"+ Audio header information - Started at %s", asctime(localtime( &aclock )));
	fprintf(m_pAudioHeaderLogFile,"+\n");
	fprintf(m_pAudioHeaderLogFile,"+	Dev#:	Audio Device number (-1 is the default device)\n");
	fprintf(m_pAudioHeaderLogFile,"+	Buf#:	Buffer serial number\n");
	fprintf(m_pAudioHeaderLogFile,"+	Flg:	Buffer flags in hex: 0x01=Done, 0x10=Queued\n");
	fprintf(m_pAudioHeaderLogFile,"+	T(ms):	Time in miliseconds from the previous call\n");
	fprintf(m_pAudioHeaderLogFile,"+\n");
	fprintf(m_pAudioHeaderLogFile,"Dev#\tBuf#\tFlg\t\tT(ms)\t--\tFlg0\tFlg1\tFlg2\tFlg3\tFlg4\tFlg5\tFlg6\tFlg7\n");
	fprintf(m_pAudioHeaderLogFile,"+======================================================================================================\n");
	fflush(m_pAudioHeaderLogFile);

	// Change status and return name of temporary file
	m_AudioHeaderLogStat = Started;
	return strdup(WaveFileName);
}
bool CWaveIn::StopAudioHeaderLog(void)
{
	// Is it OK to stop logging?
	if (m_AudioHeaderLogStat == Idle || m_AudioHeaderLogStat == Finish || m_AudioHeaderLogStat == Stopping)
		return false;

	// If the other thread is printing then change status and wait 100 milisecs
	if (m_AudioHeaderLogStat == Printing)
	{
		m_AudioHeaderLogStat = Stopping;
		Sleep(100);
	};

	// It is assumed that by now the printing thread stopped printing
	// If for some reason the no printing was done then the status now is 'Started'
	m_AudioHeaderLogStat = Finish;

	// Print Footer
	time_t aclock;
	time( &aclock ); 
	fflush(m_pAudioHeaderLogFile);
	fprintf(m_pAudioHeaderLogFile,"+======================================================================================================\n");
	fprintf(m_pAudioHeaderLogFile,"+ Audio header information - Stopped at %s", asctime(localtime( &aclock )));
	fprintf(m_pAudioHeaderLogFile,"+======================================================================================================\n");
	fclose(m_pAudioHeaderLogFile);

	// Footer was printed and file closed - time to move status to 'Idle'
	m_AudioHeaderLogStat = Idle;
	return true;
}

bool CWaveIn::StatAudioHeaderLog(int * stat)
{
	*stat = (int)m_AudioHeaderLogStat;
	return false;
}

TCHAR * CWaveIn::StartPulseLog(void)
{
	TCHAR  WaveFileName[MAX_PATH], CurrentDir[MAX_PATH];

	// Is it OK to start logging?
	if (m_PulseLogStat != Idle)
		return NULL;

	// Create file name from sound card name and current directory 
	GetCurrentDirectory(MAX_PATH, CurrentDir);
	sprintf(WaveFileName,"%s\\%s_Pls.Log", CurrentDir, m_DeviceName);

	// Open file for writing
	m_pPulseLogFile = fopen(WaveFileName, "w");
	if (!m_pPulseLogFile)
		return NULL;

	// Print header
	time_t aclock;
	time( &aclock );  
	fprintf(m_pPulseLogFile,"+======================================================================================================\n");
	fprintf(m_pPulseLogFile,"+ Pulse & Joystick information - Started at %s", asctime(localtime( &aclock )));
	fprintf(m_pPulseLogFile,"+\n");
	fprintf(m_pPulseLogFile,"+	*:			Mrak with \'*\' if number of positions change\n");
	fprintf(m_pPulseLogFile,"+	N:			Number of raw joystick positions\n");
	fprintf(m_pPulseLogFile,"+	Ch1-Ch8:	Raw joystick position value (First 8 positions)\n");
	fprintf(m_pPulseLogFile,"+	Pulse:		Pulse width in number of samples (Negative values indicate LOW pulse)\n");
	fprintf(m_pPulseLogFile,"+	Thrs:		Threshold value\n");
	fprintf(m_pPulseLogFile,"+\n");
	fprintf(m_pPulseLogFile,"*[ N]\tCh1\t\tCh2\t\tCh3\t\tCh4\t\tCh5\t\tCh6\t\tCh7\t\tCh8\t\t\tPulse\tThrs\n");
	fprintf(m_pPulseLogFile,"+======================================================================================================\n");
	fflush(m_pPulseLogFile);

	// Change status and return name of temporary file
	m_PulseLogStat = Started;
	return strdup(WaveFileName);
}
bool CWaveIn::StopPulseLog(void)
{
	// Is it OK to stop logging?
	if (m_PulseLogStat == Idle || m_PulseLogStat == Finish || m_PulseLogStat == Stopping)
		return false;

	// If the other thread is printing then change status and wait 100 milisecs
	if (m_PulseLogStat == Printing)
	{
		m_PulseLogStat = Stopping;
		Sleep(100);
	};

	// It is assumed that by now the printing thread stopped printing
	// If for some reason the no printing was done then the status now is 'Started'
	m_PulseLogStat = Finish;

	// Print Footer
	time_t aclock;
	time( &aclock ); 
	fflush(m_pPulseLogFile);
	fprintf(m_pPulseLogFile,"+======================================================================================================\n");
	fprintf(m_pPulseLogFile,"+ Pulse & Joystick information - Stopped at %s", asctime(localtime( &aclock )));
	fprintf(m_pPulseLogFile,"+======================================================================================================\n");
	fclose(m_pPulseLogFile);

	// Footer was printed and file closed - time to move status to 'Idle'
	m_PulseLogStat = Idle;
	return true;
}

bool CWaveIn::StatPulseLog(int * stat)
{
	*stat = (int)m_PulseLogStat;
	return false;
}

TCHAR * CWaveIn::StartRawPulseLog(void)
{
	TCHAR  WaveFileName[MAX_PATH], CurrentDir[MAX_PATH];

	// Is it OK to start logging?
	if (m_RawPulseLogStat != Idle)
		return NULL;

	// Create file name from sound card name and current directory 
	GetCurrentDirectory(MAX_PATH, CurrentDir);
	sprintf(WaveFileName,"%s\\%s_RPls.Log", CurrentDir, m_DeviceName);

	// Open file for writing
	m_pRawPulseLogFile = fopen(WaveFileName, "w");
	if (!m_pRawPulseLogFile)
		return NULL;

	// Print header
	time_t aclock;
	time( &aclock );  
	fprintf(m_pRawPulseLogFile,"+======================================================================================================\n");
	fprintf(m_pRawPulseLogFile,"+ Raw Pulse information from within a ProcessPulse function - Started at %s", _tasctime(localtime( &aclock )));
	fprintf(m_pRawPulseLogFile,"+\n");
	fprintf(m_pRawPulseLogFile,"+======================================================================================================\n");
	fflush(m_pRawPulseLogFile);

	// Change status and return name of temporary file
	m_RawPulseLogStat = Started;
	return strdup(WaveFileName);
}
bool CWaveIn::StopRawPulseLog(void)
{
	// Is it OK to stop logging?
	if (m_RawPulseLogStat == Idle || m_RawPulseLogStat == Finish || m_RawPulseLogStat == Stopping)
		return false;

	// If the other thread is printing then change status and wait 100 milisecs
	if (m_RawPulseLogStat == Printing)
	{
		m_RawPulseLogStat = Stopping;
		Sleep(100);
	};

	// It is assumed that by now the printing thread stopped printing
	// If for some reason the no printing was done then the status now is 'Started'
	m_RawPulseLogStat = Finish;

	// Print Footer
	time_t aclock;
	time( &aclock ); 
	fflush(m_pRawPulseLogFile);
	fprintf(m_pRawPulseLogFile,"\n+======================================================================================================\n");
	fprintf(m_pRawPulseLogFile,"+ Raw Pulse information from within a ProcessPulse function - Stopped at %s", asctime(localtime( &aclock )));
	fprintf(m_pRawPulseLogFile,"+======================================================================================================\n");
	fclose(m_pRawPulseLogFile);

	// Footer was printed and file closed - time to move status to 'Idle'
	m_RawPulseLogStat = Idle;
	return true;
}

bool CWaveIn::StatRawPulseLog(int * stat)
{
	*stat = (int)m_RawPulseLogStat;
	return false;
}

// Calculate the audio level (volume) of the audio signal
// Input:
//	Value - A PCM data sample
//	threshold - reference level
//	Size - Number of bytes in a sample
// Return:
//	Sliding average signal level calculated as absolute offset from threshold level
//	Output is normalized to a scale of 0-255
int CWaveIn::CalcAudioLevel(int Value,  int Size)
{
	static double Min, Max;

	// Very slowly move extramums towards the center
	Max--;
	Min++;

	// If extramums meet then spread them appart
	if (Max<Min)	Max=Min+5;

	// If input value is higher than maximum - slowly raise max value
	if (Value<Min)		Min = (127*Min + Value)/128;
	// If input value is lower than minimum - slowly reduce min value
	else if (Value>Max)	Max = (127*Max + Value)/128;

	// The audio level is the amplitude = the difference between the extremums
	return (int)(Max-Min);
}

// Interface function - Get audio level of this device (Volume)
int	CWaveIn::GetAudioLevel(void)
{
	return m_AudioLevel;
}

// Interface function - Get number of joystick positions
int CWaveIn::GetNumJoyPos(void)
{
	return m_nPositions;
}

// Interface function - Get pointer to array of joystick positions
// Return number of valid positions
int CWaveIn::GetJoyPos(int ** pos)
{
	if (!(m_nPositions && pos))
		return 0;

	*pos = (int *)(&m_Position);
	return m_nPositions;
}

// Interface function - Check if joystick data valid
bool CWaveIn::isJoyPosValid(void)
{
	return m_ValidPositions;
}

// Enable/Disable anti jitter mechanism
bool CWaveIn::SetAntiJitter(const bool Enable)
{
	m_AntiJitterEnabled = Enable;
	return true;
}

CWaveIn::~CWaveIn(void)
{
	if (m_LogWr) delete m_LogWr;
	if (m_LogRd) delete m_LogRd;
	if (m_DeviceName) delete m_DeviceName;
}
