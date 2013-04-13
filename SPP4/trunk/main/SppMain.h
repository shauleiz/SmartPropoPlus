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

using std::function;
using std::placeholders::_1;
using std::placeholders::_2;

typedef  std::vector<function<void (int, BOOL)>> vPP;


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


/* Globals */
int gDebugLevel = 0;
FILE * gCtrlLogFile = NULL;


class SPPMAIN_API CSppMain {
	public:
	CSppMain(void);
	~CSppMain();
	bool Start();

private:
	int LoadProcessPulseFunctions();
	void ProcessPulsePpm(int width, BOOL input);
	function<void (int, BOOL)> f;


private:
	bool	m_PropoStarted;
	LPVOID	m_pSharedBlock;
	LPWSTR	m_MixerName;
	int		m_JsChPostProc_selected;
	int		m_Position[MAX_JS_CH];
	vPP		m_ListProcessPulseFunc;
};