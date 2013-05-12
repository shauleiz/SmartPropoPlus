// User-defined messages
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;	// Message from the system tray
UINT const WMAPP_SM_INIT = WM_APP + 2;			// State machine initialized
UINT const WMAPP_SM_GUI  = WM_APP + 3;			// State machine Display GUI



UINT const WMSPP_AUDIO_CHNG =	WM_APP + 100;			// Default Device changed
UINT const WMSPP_AUDIO_ADD =		WM_APP + 101;			// Device added
UINT const WMSPP_AUDIO_REM =			WM_APP + 102;			// Device removed
UINT const WMAPP_DEV_CHANGED =		WM_APP + 103;			// Device changed
UINT const WMSPP_AUDIO_PROP =		WM_APP + 104;			// Device property changed
UINT const WMAPP_POS_READY =		WM_APP + 105;			// Joystick Position data is ready
UINT const WMAPP_DEV_INFO =			WM_APP + 106;			// Device information available

UINT const WMAPP_GUI_SHOW =			WM_APP + 200;			// Show GUI Window
UINT const WMAPP_GUI_HIDE =			WM_APP + 201;			// Hide GUI Window
UINT const WMAPP_GUI_AUDIO =		WM_APP + 202;			// Information destination: Audio GUI
UINT const WMSPP_PRCS_CHMNTR =			WM_APP + 203;			// Channel data is ready to be monitored. The payload: Wparam is channel index, Lparam is channel value

UINT const BASE_SUB			=			1000;					// Base value
UINT const POPULATE_JACKS	=			BASE_SUB+1;				// The payload is a pointer to struct jack_info
UINT const SET_DEF_JACKS	=			BASE_SUB+2;				// The payload is a pointer the id of the default jack
UINT const ADD_JACK			=			BASE_SUB+3;				// The payload is a pointer the id of the added jack
UINT const REM_JACK			=			BASE_SUB+4;				// The payload is a pointer the id of the removed jack
UINT const REM_ALL_JACK		=			BASE_SUB+5;				// remove all jacks from GUI
UINT const WMSPP_PRCS_GETID	=			BASE_SUB+6;				// Get pointer to ID string of selected ID
UINT const WMSPP_PRCS_SETMOD		=			BASE_SUB+7;				// The payload is a pointer to struct MOD_STRUCT
UINT const WMSPP_DLG_MOD		=			BASE_SUB+8;				// The payload is a pointer to the internal name of the selected modulation
UINT const WMSPP_DLG_MONITOR		=			BASE_SUB+9;				// The payload is boolean Start/Stop monitoring raw channel
UINT const FILTER_DLL		=			BASE_SUB+10;			// The payload is boolean: Filter DLL exists/absent
UINT const FILTER_VER		=			BASE_SUB+11;			// The payload is boolean + version: If false version is too low. 
UINT const FILTER_NUM		=			BASE_SUB+12;			// The payload is the number of filters in the filter DLL 
UINT const FILTER_ADDA		=			BASE_SUB+13;			// The payload is: Filter Index and Pointer to filter name (ASCII)
UINT const FILTER_ADDW		=			BASE_SUB+13;			// The payload is: Filter Index and Pointer to filter name (WCHAR)
UINT const WMSPP_DLG_FILTER	=			BASE_SUB+14;			// The payload is: Filter Index of the selected filter (if none then -1)
