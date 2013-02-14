// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the STUB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// STUB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef STUB_EXPORTS
#define STUB_API __declspec(dllexport)
#else
#define STUB_API __declspec(dllimport)
#endif

#define	NDOTS 10000
#define	HI	 0.8f
#define	LO	 0.2f
#define DEFAULT_RATE 192000
#define PULSE_BUF_SIZE DEFAULT_RATE/10
#define PULSE_MAX_SIZE DEFAULT_RATE/10
#define WM_BUFF_READY WM_USER+112

struct PULSE_DATA {
	UINT	nPulses;
	float	*pXBuff;
	float	*pYBuff;
};

// Globals
UINT	g_rate = DEFAULT_RATE; // Default
float	g_ScopeDots[NDOTS] = {0.0f};
UINT	g_iDots = 0;
HWND	g_hScopeWnd = NULL;

// This class is exported from the Stub.dll
class STUB_API CStub {
public:
	CStub(void);
	// TODO: add your methods here.
};



extern STUB_API int nStub;
STUB_API int fnStub(void);

STUB_API void SetSampleRate(int rate);
STUB_API void Pulse2Scope(int length, bool low, LPVOID Param);
STUB_API void InitDecoder(int rate);
