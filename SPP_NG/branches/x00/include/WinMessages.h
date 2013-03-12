// User-defined messages
UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;	// Message from the system tray
UINT const WMAPP_SM_INIT = WM_APP + 2;			// State machine initialized
UINT const WMAPP_SM_GUI  = WM_APP + 3;			// State machine Display GUI



UINT const WMAPP_DEFDEV_CHANGED =	WM_APP + 100;			// Default Device changed
UINT const WMAPP_DEV_ADDED =		WM_APP + 101;			// Device added
UINT const WMAPP_DEV_REM =			WM_APP + 102;			// Device removed
UINT const WMAPP_DEV_CHANGED =		WM_APP + 103;			// Device changed
UINT const WMAPP_DEV_PROPTY =		WM_APP + 104;			// Device property changed
UINT const WMAPP_POS_READY =		WM_APP + 105;			// Joystick Position data is ready
UINT const WMAPP_DEV_INFO =			WM_APP + 106;			// Device information available

UINT const WMAPP_GUI_SHOW =			WM_APP + 200;			// Show GUI Window
UINT const WMAPP_GUI_HIDE =			WM_APP + 201;			// Hide GUI Window
UINT const WMAPP_GUI_AUDIO =		WM_APP + 202;			// Information destination: Audio GUI


UINT const POPULATE_JACKS =			1000;					// The payload is a pointer to struct jack_info
