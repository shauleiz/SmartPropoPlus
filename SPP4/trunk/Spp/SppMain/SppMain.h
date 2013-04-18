// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the SPPMAIN_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PULSESCOPE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SPPMAIN_EXPORTS
#define SPPMAIN_API __declspec(dllexport)
#else
#define SPPMAIN_API __declspec(dllimport)
#endif


#include <vector>
#include <functional>
#include <thread>

using std::function;
using std::thread;
using std::placeholders::_1;
using std::placeholders::_2;

#ifdef _DEBUG
#pragma  comment(lib, "..\\Debug\\AudioLib.lib")
#else
#pragma  comment(lib, "..\\Release\\AudioLib.lib")
#endif


#define  PP function<void (int, BOOL)>

struct MOD_STRUCT {
	BOOL isPpm;
	LPCTSTR ModType;
	LPCTSTR ModName;
	BOOL ModSelect;
	PP func;   
};

typedef  std::vector<PP> vPP;
typedef  std::vector<MOD_STRUCT> vMOD;
typedef	 std::vector<MOD_STRUCT>::iterator iMOD;


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

#define MAX_JS_CH	12
#define MUTEX_STOP_START	_T("WaveIn Stopping and Starting are mutually exclusive")




class /*SPPMAIN_API*/ CSppMain {
	public:
	SPPMAIN_API CSppMain(void);
	SPPMAIN_API ~CSppMain();
	SPPMAIN_API bool Start(HWND hParentWnd);

private:
	int LoadProcessPulseFunctions();
	void ProcessPulsePpm(int width, BOOL input);
	void ProcessPulseWalPcm(int width, BOOL input);
	void ProcessPulseAirPcm1(int width, BOOL input);
	void ProcessPulseAirPcm2(int width, BOOL input);
	void ProcessPulseJrPcm(int width, BOOL input);
	void ProcessPulseFutabaPcm(int width, BOOL input);
	void ProcessPulseWK2401Ppm(int width, BOOL input);
	void ProcessPulseFutabaPpm(int width, BOOL input);
	void ProcessPulseJrPpm(int width, BOOL input);

	void SendPPJoy(int nChannels, int *Channel);
	__inline  int  smooth(int orig, int newval);
	int  __fastcall Convert15bits(unsigned int in);
	int  __fastcall  CSppMain::Convert20bits(int in);
	static DWORD WINAPI  ListenToGuiStatic(LPVOID obj);
	void ListenToGui(void);
	void SendModInfoToParent(HWND hParentWnd);


private:	// Walkera (PCM) helper functions
	unsigned char  WalkeraConvert2Bin(int width);
	unsigned char  WalkeraConvert2Oct(int width);
	int WalkeraElevator(const unsigned char * cycle);
	int WalkeraAilerons(const unsigned char * cycle);
	int WalkeraThrottle(const unsigned char * cycle);
	int WalkeraRudder(const unsigned char * cycle);
	int WalkeraGear(const unsigned char * cycle);
	int WalkeraPitch(const unsigned char * cycle);
	int WalkeraGyro(const unsigned char * cycle);
	int WalkeraChannel8(const unsigned char * cycle);
	int * WalkeraCheckSum(const unsigned char * cycle);

private:
	bool	m_PropoStarted;
	LPVOID	m_pSharedBlock;
	LPWSTR	m_MixerName;
	int		m_JsChPostProc_selected;
	int		m_Position[MAX_JS_CH];
	vMOD	m_ListProcessPulseFunc;
	HANDLE	m_hMutexStartStop;
	HANDLE	m_hCaptureAudioThread;
	volatile BOOL m_closeRequest;
	volatile BOOL m_waveRecording;
	struct Modulations *  m_Modulation;
};



static int futaba_symbol[1024] = {
    -1, -1, -1, -1, -1, -1, -1, 63, -1, -1, -1, -1, 62, -1, -1, 39,
    -1, -1, -1, -1, -1, -1, -1, -1, 60, -1, -1, -1, 61, -1, -1, 38,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    58, -1, -1, 43, -1, -1, -1, -1, 59, -1, -1, -1, 48, -1, -1, 10,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    56, -1, -1, 42, -1, -1, -1, 34, -1, -1, -1, -1, -1, -1, -1, -1,
    57, -1, -1, 33, -1, -1, -1, -1, 49, -1, -1, -1, 37, -1, -1,  9,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    52, -1, -1, 41, -1, -1, -1, 32, -1, -1, -1, -1, 40, -1, -1, 19,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    51, -1, -1, 35, -1, -1, -1, 18, -1, -1, -1, -1, -1, -1, -1, -1,
    50, -1, -1, 17, -1, -1, -1, -1, 36, -1, -1, -1, 16, -1, -1,  8,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    55, -1, -1, 47, -1, -1, -1, 27, -1, -1, -1, -1, 46, -1, -1, 13,
    -1, -1, -1, -1, -1, -1, -1, -1, 45, -1, -1, -1, 28, -1, -1, 12,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    44, -1, -1, 23, -1, -1, -1, -1, 31, -1, -1, -1, 22, -1, -1, 11,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    54, -1, -1, 26, -1, -1, -1, 14, -1, -1, -1, -1, 30, -1, -1,  6,
    -1, -1, -1, -1, -1, -1, -1, -1, 29, -1, -1, -1, 21, -1, -1,  7,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    53, -1, -1, 15, -1, -1, -1,  4, -1, -1, -1, -1, 20, -1, -1,  5,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    25, -1, -1,  2, -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1,
    24, -1, -1,  1, -1, -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1
};

static int jr_symbol[256] = {
    -1,  0, 16, -1, 17, 26, -1, -1, 21, 10,  8, -1, -1, -1, -1, -1,
    23, 14, 12, -1,  4, 27, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    19, 15, 13, -1,  5, 25, -1, -1,  7, 29, 31, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    18, 11,  9, -1,  1, 24, -1, -1,  3, 28, 30, -1, -1, -1, -1, -1,
     2, 20, 22, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static int air1_symbol[32] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 10,  8, -1, 6, 14, 12,
    -1, -1,  9,  0, -1,  5, 13,  4, -1, 3, 11,  1, -1, 7, 15, -1,
};




static int air1_msb[32] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0,  2, -1,  0,  0,  2,
    -1, -1,  3,  2, -1,  3,  3,  2, -1,  1,  1,  3, -1,  1,  1, -1,
};

static int air2_symbol[16] = {
	-1, -1, -1, -1,  0,  0, -1,  2, -1, -1, -1, -1,   1,  1, -1,  3
};
