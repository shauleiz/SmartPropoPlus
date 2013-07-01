// User-defined messages
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;	// Message from the system tray
UINT const WMAPP_SM_INIT = WM_APP + 2;			// State machine initialized
UINT const WMAPP_SM_GUI  = WM_APP + 3;			// State machine Display GUI



UINT const WMSPP_AUDIO_CHNG =			WM_APP + 100;			// Default Device changed
UINT const WMSPP_AUDIO_ADD	=			WM_APP + 101;			// Device added
UINT const WMSPP_AUDIO_REM	=			WM_APP + 102;			// Device removed
UINT const WMAPP_DEV_CHANGED=			WM_APP + 103;			// Device changed
UINT const WMSPP_AUDIO_PROP =			WM_APP + 104;			// Device property changed
UINT const WMAPP_POS_READY	=			WM_APP + 105;			// Joystick Position data is ready
UINT const WMAPP_DEV_INFO	=			WM_APP + 106;			// Device information available
UINT const WMSPP_AUDIO_INSIG	=		WM_APP + 107;			// Input signal (raw audio) available
UINT const WMSPP_PROC_PULSE	=			WM_APP + 108;			// Pulse data available

UINT const WMAPP_GUI_SHOW	=			WM_APP + 200;			// Show GUI Window
UINT const WMAPP_GUI_HIDE	=			WM_APP + 201;			// Hide GUI Window
UINT const WMAPP_GUI_AUDIO	=			WM_APP + 202;			// Information destination: Audio GUI
UINT const WMSPP_PRCS_RCHMNT=			WM_APP + 203;			// Raw Channel data is ready to be monitored. The payload: Wparam is channel index, Lparam is channel value
UINT const WMSPP_PRCS_PCHMNT=			WM_APP + 204;			// Processed Channel data is ready to be monitored. The payload: Wparam is channel index, Lparam is channel value

UINT const BASE_SUB			=			1000;					// Base value
UINT const POPULATE_JACKS	=			BASE_SUB+1;				// The payload is a pointer to struct jack_info
UINT const SET_DEF_JACKS	=			BASE_SUB+2;				// The payload is a pointer the id of the default jack
UINT const ADD_JACK			=			BASE_SUB+3;				// The payload is a pointer the id of the added jack
UINT const REM_JACK			=			BASE_SUB+4;				// The payload is a pointer the id of the removed jack
UINT const REM_ALL_JACK		=			BASE_SUB+5;				// remove all jacks from GUI
UINT const WMSPP_PRCS_GETID	=			BASE_SUB+6;				// Get pointer to ID string of selected ID
UINT const WMSPP_PRCS_SETMOD=			BASE_SUB+7;				// The payload is a pointer to struct MOD and modulation type
UINT const WMSPP_DLG_MOD	=			BASE_SUB+8;				// The payload is a pointer to the internal name of the selected modulation
UINT const WMSPP_DLG_MONITOR=			BASE_SUB+9;				// The payload is boolean Start/Stop monitoring raw channel
UINT const FILTER_DLL		=			BASE_SUB+10;			// The payload is boolean: Filter DLL exists/absent
UINT const FILTER_VER		=			BASE_SUB+11;			// The payload is boolean + version: If false version is too low. 
UINT const FILTER_NUM		=			BASE_SUB+12;			// The payload is the number of filters in the filter DLL 
UINT const FILTER_ADDA		=			BASE_SUB+13;			// The payload is: Filter Index and Pointer to filter name (ASCII)
UINT const FILTER_ADDW		=			BASE_SUB+13;			// The payload is: Filter Index and Pointer to filter name (WCHAR)
UINT const WMSPP_DLG_FILTER	=			BASE_SUB+14;			// The payload is: Filter Index of the selected filter (if none then -1)
UINT const WMSPP_PRCS_GETSPR=			BASE_SUB+15;			// Get Number of Samples/Second for the default capture device
UINT const WMSPP_PRCS_GETBPS=			BASE_SUB+16;			// Get Number of Bits per Sample for the default capture device
UINT const WMSPP_PRCS_GETNCH=			BASE_SUB+17;			// Get Number of audio channels (Mono(1)/Stereo(2))
UINT const WMSPP_PRCS_GETLR	=			BASE_SUB+18;			// Get active audio channel (Left(0), Right(1))
UINT const WMSPP_DLG_LOG	=			BASE_SUB+19;			// The payload is boolean Show/Hide log window
UINT const WMSPP_DLG_INSIG	=			BASE_SUB+20;			// The payload is boolean Start recording input signal (Raw audio)
UINT const WMSPP_DLG_PULSE	=			BASE_SUB+21;			// The payload is boolean Start recording Pulse data (Including input sig)
UINT const WMSPP_JMON_AXIS	=			BASE_SUB+22;			// Inform of new value in an axis of a vJoy device
UINT const WMSPP_DLG_MAP	=			BASE_SUB+23;			// The payload is DWORD that describe mapping of channels to axes
UINT const WMSPP_MAP_UPDT	=			BASE_SUB+24;			// The payload is DWORD that describe mapping of channels to axes
UINT const WMSPP_DLG_CHNL	=			BASE_SUB+25;			// Change in audio channel params - The payload is wParam: Number of bits, lParam: channel L/R/M
UINT const WMSPP_PRCS_GETMOD=			BASE_SUB+26;			// Get the selected modulation
UINT const WMSPP_AUDIO_GETSU=			BASE_SUB+27;			// Get the selected audio setup: Payload is Audio Id. Bit rate and Channel. Return value: 8/16 + channel bit (0 if left, 1 if right) 
UINT const SET_AUDIO_PARAMS	=			BASE_SUB+28;			// The payload is Bitrate in WPARAMS, Channel ('L'/'R') in LPARAMS
UINT const MONITOR_CH		=			BASE_SUB+29;			// The payload is in WPARAMS, true: Start monitoring channels
UINT const VJOYDEV_REMALL	=			BASE_SUB+30;			// Remove all vJoy devices from GUI
UINT const VJOYDEV_ADD		=			BASE_SUB+31;			// Add vJoy device to GUI: The payload in WPARAMS = id, in LPARAMS = selected
UINT const VJOYDEV_SETSEL	=			BASE_SUB+32;			// Set vJoy device in GUI as selected

// Log Messages
UINT const WMSPP_LOG_BASE	=				WM_APP + 10000;					// Base value for log messages
UINT const WMSPP_LOG_CNTRL	=				WMSPP_LOG_BASE + 0;				// Base value for log messages comming from SppControl
UINT const WMSPP_LOG_AUDIO	=				WMSPP_LOG_BASE + 1000;			// Base value for log messages comming from SppAudio
UINT const WMSPP_LOG_PRSC	=				WMSPP_LOG_BASE + 2000;			// Base value for log messages comming from SppProcess


