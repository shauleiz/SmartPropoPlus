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

#include <functional>
using std::tr1::function;

//typedef  void ( * PP)(int width, BOOL input);
typedef  std::function<void (int, BOOL)> PP;


class SPPMAIN_API CSppMain {
	public:
	CSppMain(void);
	~CSppMain();
	bool Start();

private:
	int LoadProcessPulseFunctions();
	void ProcessPulsePpm(int width, BOOL input);


private:
	bool	m_PropoStarted;
	LPVOID	m_pSharedBlock;
	LPWSTR	m_MixerName;
	//LPVOID	*m_ListProcessPulseFunc;
	PP * m_ListProcessPulseFunc;
};