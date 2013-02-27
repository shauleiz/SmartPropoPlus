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
	class CConfig m_config;

protected: // Class internal functions
	LRESULT NotifyParent(UINT message, WPARAM wParam=NULL, LPARAM lParam=NULL);
	HRESULT CStateMachine::LoadConfigFromLocalFile(void);
	HRESULT LoadConfigFromDefaultFile(void);
	HRESULT LoadConfigFromFile(LPCWSTR FileName);
	bool IsIdentical(LPCWSTR wStr, const char * utf8);
	bool IsIdentical(const char * utf8, LPCWSTR wStr);
};