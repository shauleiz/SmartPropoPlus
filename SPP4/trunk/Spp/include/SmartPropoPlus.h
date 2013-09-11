/* Generic SmartPropoPlus header file */
#if defined(__cplusplus)
extern "C"
{
#endif

#ifndef __SMARTPROPOPLUS
#define __SMARTPROPOPLUS
/** Definitions **/
#define REG_FMS _T("Software\\Flying-Model-Simulator")
#define REG_SPP _T("Software\\Flying-Model-Simulator\\SmartPropoPlus")
#define REG_MOD _T("Software\\Flying-Model-Simulator\\SmartPropoPlus\\Modulation-Types")
#define REG_AUD _T("Software\\Flying-Model-Simulator\\SmartPropoPlus\\Audio-Sources")

#define MOD_TYPE_PPM	_T("PPM")
#define MOD_TYPE_PPMP	_T("PPMPOS")
#define MOD_TYPE_PPMN	_T("PPMNEG")
#define MOD_TYPE_PPMW	_T("PPMW")
#define MOD_TYPE_JR 	_T("JR")
#define MOD_TYPE_FUT	_T("FUT")
#define MOD_TYPE_AIR1	_T("AIR1")
#define MOD_TYPE_AIR2	_T("AIR2")
#define MOD_TYPE_WAL	_T("WAL")

#define MOD_NAME_PPM	_T("PPM (Generic)")
#define MOD_NAME_PPMP	_T("PPM (Positive)")
#define MOD_NAME_PPMN	_T("PPM (Negative)")
#define MOD_NAME_PPMW	_T("PPM (Walkera)")
#define MOD_NAME_JR 	_T("JR (PCM)")
#define MOD_NAME_FUT	_T("Futaba (PCM)")
#define MOD_NAME_AIR1	_T("Sanwa/Air (PCM1)")
#define MOD_NAME_AIR2	_T("Sanwa/Air (PCM2)")
#define MOD_NAME_WAL	_T("Walkera (PCM)")

#define MOD_DEF_STR  {\
						MOD_TYPE_PPM, MOD_NAME_PPM,\
						MOD_TYPE_PPMP, MOD_NAME_PPMP,\
						MOD_TYPE_PPMN, MOD_NAME_PPMN,\
						MOD_TYPE_PPMW, MOD_NAME_PPMW,\
						MOD_TYPE_JR,  MOD_NAME_JR,\
						MOD_TYPE_FUT, MOD_NAME_FUT,\
						MOD_TYPE_AIR1,MOD_NAME_AIR1,\
						MOD_TYPE_AIR2,MOD_NAME_AIR2,\
						MOD_TYPE_WAL,MOD_NAME_WAL,\
						NULL}

#define	MNM_POS		"&Positive"
#define	MNM_NEG		"&Negative"
#define	MNM_POS_HID	"Positive"
#define	MNM_NEG_HID	"Negative"

#define MOD_ACTIVE	_T("Active")
#define SHIFT_POS	_T("Positive-Shift")
#define SHIFT_AUTO	_T("Shift-Auto-Detect")
#define DEBUG_LEVEL	_T("Debug-Level")
#define	MIXER_DEV	_T("Mixer-Device")
#define	ENDPOINT	_T("EndPoint-Device")
#define	AUDIO		_T("Audio")

#define	SEL_FLTR	_T("Selected-Filter")

#define	PPJ_EXT		_T("PPJoy_Extension")

#define	DEF_VOL_MIC	_T("Default Volume Level - Microphone")
#define	DEF_VOL_AUX	_T("Default Volume Level - Aux")
#define	DEF_VOL_LIN	_T("Default Volume Level - Line")
#define	DEF_VOL_ANL	_T("Default Volume Level - Analog")
#define	DEF_VOL_UNK	_T("Default Volume Level - Unknown")

#define MUTEX_LABEL	_T("SPP Global Shared memory Mutex")
#define BLOCK_LABEL	_T("SPP Global Shared memory Block")
#define EVENT_MIXER	_T("Mixer Switching Event")
#define	VER_DLL		0x0003030b /* DLL File Version */
#define	VER_GUI		0x0003030b /* GUI File Version */
#define	VJOY_MIN_VER	0x202	/* Minimum vJoy version supported */


#define TX_VOLUME_MIC		5
#define TX_VOLUME_LINE		5
#define TX_VOLUME_AUX		7
#define TX_VOLUME_ANALOG	10
#define TX_VOLUME_UNKNOWN	10


#define MAX_VAL_NAME 254

#define FILTERDLL_NAME	TEXT("JsChPostProc.dll")
#define PPJDLL_NAME		TEXT("PPJoyEx")
#define CTRL_LOG_FILE	"SPP_ctrl.log"
#define CHNL_LOG_FILE	"SPP_chnl.log"
#define DATA_LOG_FILE	"SPP_data.log"
#define SEPARATOR		"*************************************************************************"

#define	SPP_ERROR_MSG		TEXT("SmartPropoPlus Error")
#define SPP_MSG				TEXT("SmartPropoPlus Message")

#define CONSOLE_TT			"SmartPropoPlus Console"
#define CONSOLE_TT_OFF		"SmartPropoPlus is OFF"
#define CONSOLE_TT_ON		"SmartPropoPlus is ON"
#define CONSOLE_BALOON_TTL	_T("SmartPropoPlus Console")
#define CONSOLE_BALOON_TXT	"Use this application to control:\n- Modulation Type\n- Modulation Shift\n\nChanges are effective Immediately!"
#define BALOON_DLL_STOPPED	"SmartPropoPlus stopped"
#define BALOON_DLL_STARTING	"SmartPropoPlus is starting"
#define BALOON_DLL_MODTYPE	"\nModulation type:\t%s"
#define BALOON_DLL_AUTOON	"\nPolarity Auto-Detect:\tOn"
#define BALOON_DLL_AUTOOFF	"\nPolarity Auto-Detect:\tOff"
#define BALOON_DLL_SHIFTPOS	"\nShift Polarity:\t\tPositive"
#define BALOON_DLL_SHIFTNEG	"\nShift Polarity:\t\tNegative"
#define BALOON_DLL_AUDIO	"\n\nAudio Source:\t%s"
#define BALOON_DLL_PPJOY	"\n\nPPJoy Status:\t%s"
#define BALOON_PPJ_CNCT		"Virtual Joystick Connected"
#define BALOON_PPJ_DISCNCT	"Virtual Joystick Disconnected"
#define BALOON_PPJ_NOTFND	"Virtual Joystick cannot be found"
#define BALOON_PPJ_INVHNDL	"Virtual Joystick device handle is invalid"
	

#define MAX_MSG_SIZE		1000
#define SPP_HOME_URL		"http://www.SmartPropoPlus.com"
#define SPP_HOME_TITLE		"SmartPropoPlus Home"
#define	SPP_MAIN_TEXT		"\
Version %d.%d.%d - Build %s\n\
Copyright (c) 2005-2012 by Shaul Eizikovich\n\n\
This software is based on SmartPropo by Tsutomu SEKI\n\
You may use and destribute this program freely.\n\n\
SmartPropoPlus enables you to connect your R/C transmitter to your computer through \
one of the sound-card inputs (e.g. Microphone, Line-in) so it can be used as a joystick.\n\
For further details please go to the SmartPropoPlus Home using the link below."

/* WINMM Messages */
#define	MM_STD_HDR		_T("SmartPropoPlus Message")
#define	MM_NO_NUM_MOD	_T("Winmm.dll: Cannot get number of modulation modes - Assuming PPM")
#define	MM_CANNOT_REG	_T("StartSppConsole(): WM_INTERSPPAPPS = %d - cannot register window message) INTERSPPAPPS")
#define	MM_VISTA_NOSP1	_T("Your operating system is Vista\r\nYou must install Service Pack 1 (SP1)")
#define	MM_CANNOT_REG2	_T("ExitPropo(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS")
#define	MM_CANNOT_REG3	_T("ConnectPPJoyDriver(): WM_INTERSPPAPPS = %d - cannot register window message) INTERSPPAPPS")
#define	MM_CANNOT_REG4	_T("DisconnectPPJoyDriver(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS")
#define	MM_CANNOT_REG5	_T("SendPPJoy(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS")
#define	MM_CANNOT_REG6	_T("StartJsChPostProc(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS")
#define	MM_WASAPI_REG	_T("WASAPI Class not registered - SmartPropoPlus will not function correctly")
#define	MM_WASAPI_ENUM	_T("WASAPI Enumerating endpoints failed - SmartPropoPlus will not function correctly")
#define	MM_WASAPI_CNT	_T("WASAPI Could not get the count of available endpoints - SmartPropoPlus will not function correctly")
#define	MM_WASAPI_BDEV	_T("Device cannot be used\r\nSelect another input device")
#define	MM_WASAPI_STRT	_T("WASAPI: Could not start audio capture\r\nStopping audio capture")
#define	MM_WASAPI_SIZE	_T("WASAPI: Could not get size of next data-packet\r\nRestart SmartPropoPlus")
#define	MM_WASAPI_INTR	_T("Audio input interrupded\r\nWould you like to retry?")
#define	MM_WASAPI_ABFR	_T("Audio buffer failure\r\nWould you like to retry?")
#define	MM_WASAPI_RLS	_T("WASAPI: Could not release data buffer\r\nStopping audio capture")
#define	MM_WASAPI_NOREQ	_T("Could not find requested device\r\nStarting default device")

#define CONSOLE_TITLE		"SmartPropoPlus Console"
#define	MUTXCONSOLE			TEXT("SPPcontrol Exists Mutex 1")
#define	MUTXWINMM			"WINMM Dll is running"
#define	MUTXPROPOSTARTED	"WINMM Dll - Propo Started"
#define	INTERSPPCONSOLE		TEXT("Inter SPPcontrol Message 1")
#define	INTERSPPAPPS		TEXT("Inter SPP Application Message 2")
#define DEFLT_WAVE			L"WAVE_MAPPER"

/* SPP Console messages */
#define CN_NO_INTERSPPCONSOLE	TEXT("wWinMain(): WM_INTERSPPCONSOLE = %u - cannot register window message INTERSPPCONSOLE")
#define CN_NO_INTERSPPAPPS		TEXT("wWinMain(): WM_INTERSPPAPPS = %d - cannot register window message INTERSPPAPPS")

/* User Defined Window-Messages */
#define WM_DBLLEFT	WM_APP+1
#define WM_LEFT		WM_APP+2
#define WM_RIGHT	WM_APP+3
#define WM_GENERAL	WM_APP+4
#define WM_SHOWGUI	WM_APP+5
#define WM_HIDEGUI	WM_APP+6

/* User Defined Window-Message parameters */
#define MSG_DLLSTOPPING		1280	// WPARAM: Inform GUI that DLL is stopping
#define MSG_DLLSTARTING		1281	// WPARAM: Inform GUI that DLL is starting
#define MSG_DLLPPJSTAT		1282	// WPARAM: Inform GUI of PPJoy/vJoy connection status
#define MSG_JSCHPPEVAIL		1283	// WPARAM: Inform GUI that Joystick channel postprocessor is evalable/unevailable
#define MSG_ISPPJOY			1284	// WPARAM: Inform GUI that Virtual Joystick type is PPJoy
#define MSG_ISVJOY			1285	// WPARAM: Inform GUI that Virtual Joystick type is vJoy


#define MSG_DPPJSTAT_DISCN	1984	// LPARAM: Intentional disconnection

#define AUTOCHANNEL	0x0001
#define AUTOBITRATE	0x0002

#define  SCP function<void (int index, int length, bool low, LPVOID timestamp, LPVOID Param)>
#define  PP function<void (int, BOOL)>
#define  Map2Nibble(Map,i) 	((Map & (0xF<<(4*(7-i))))>>(4*(7-i)))&0xF
#define Sleep_For(_X) std::this_thread::sleep_for( std::chrono::milliseconds(_X));

extern void SetThreadName(char* threadName);
#ifdef _DEBUG
#define THREAD_NAME(name) SetThreadName(name)
#else
#define THREAD_NAME(name)
#endif

/** Data types and structures ***/
struct Modulation
{
	LPTSTR ModTypeInternal;
	LPTSTR ModTypeDisplay;
	int index;
};

struct Modulations
{
	struct Modulation ** ModulationList;
	int Active;
	int PositiveShift;
	int	ShiftAutoDetect;
};

struct JsChPostProcInfo
{
	int n_fltr;				// Number of supported filters (1 or more)
	int	i_sel;				// Index of selected filter (0 based, -1 means nothing)
	far char ** name_fltr;	// list of display name of filters
} ;

struct DbgPulseInfo
{
	UINT size;
	LPVOID Samples;
	UINT RawPulse;
	UINT NormPulse;
	bool negative;
};

/// Channel to Button/Axis mapping
#define MAX_BUTTONS		128
typedef std::array<BYTE, MAX_BUTTONS> BTNArr;
struct Mapping {
	UINT nAxes;
	DWORD * pAxisMap;
	UINT nButtons;
	BTNArr * ButtonArray;
};

// Control (Buttons/Axes) mask (availability)
struct controls
{
	UINT nButtons;	// Number of buttons
	BOOL axis[8];	// Axis-mask: axis[0] indicates if X exists ... axis[7] refers to SL1
};

struct jack_info
{
	/* Inter-unit information about jack/capture endpoint information */
	int	struct_size;
	WCHAR * id;
	COLORREF color;
	WCHAR * FriendlyName;
	//bool	Enabled;
	bool	Default;
	int		nChannels;
};

struct filter_info
{
	/* Inter-unit information about filters */
	int	struct_size;		// Size of struct in bytes
	LPTSTR FilterFileName;	// Filter File Name 
	int	iSelFilter;			// Intex of selected filter (Zero-based, -1 means none selected)
	UINT nFilters;			// Number of filters in the following list
	LPTSTR * FilterName;	// Array of nFilters filter names
};


struct MOD {
	LPCTSTR Type;		// Unique identifier of the modulation. Examples are PPMW, AIR1 …
	LPCTSTR Subtype;	// PPM/PCM
	LPCTSTR Name;		// User friendly name of the modulation to be desplayd.
	PP func;			// This is the function of type PP that implements the modulation. Called by ProcessWave().
};

#endif // __SMARTPROPOPLUS

#if defined(__cplusplus)
}
#endif