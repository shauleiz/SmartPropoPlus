// StateMachine.h
//
// Control unit State Machine declarations

///////////////////////////////////////////////////////////////////////////
// State machine states
enum CU_STATE
{
	UNDEF	= 0,	// Undefined state (Starting point)
	RUNNING,		// SPP is fully functional
	STOPPED,		// SPP was stopped
	STANDBY,		// SPP is waiting for incoming audio signal
	CONF,			// SPP is not running because it is being configured
	ERR,			// Error state
};
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
#define	STR_CTRL_XML		L"\\SmartPropoPlus\\Control.xml"
#define	STR_DEFLT_CTRL_XML	L".\\Control.xml"
#define	XML_ROOT_ELEM		L"SmartPropoPlus"

typedef std::map<std::string,std::string> MessageMap;

// Main class: CStateMachine
class CStateMachine
{
public:
	CStateMachine();
	virtual ~CStateMachine();
	virtual BOOL InitInstance(HWND const hWindow, HINSTANCE const hInstance);
	CU_STATE GetState(void);
	void UserInput(UINT message);
	void UnitIntup(UINT message, PVOID Opaque);

protected: // Class variables
	HWND hWnd;
	HINSTANCE hInst;
	CU_STATE state;

protected: // Class internal functions
	LRESULT NotifyParent(UINT message, WPARAM wParam=NULL, LPARAM lParam=NULL);
	HRESULT CStateMachine::LoadConfigFromLocalFile(void);
	HRESULT LoadConfigFromDefaultFile(void);
	HRESULT LoadConfigFromFile(LPCWSTR FileName);
	bool IsIdentical(LPCWSTR wStr, const char * utf8);
	bool IsIdentical(const char * utf8, LPCWSTR wStr);
};