/****** JsChPostProc.Dll - Joystick channels posrprocessor *********************/

/* Structures */
typedef struct _JS_CHANNELS	// Joystick channel data
{
	int ch;			// Number of channels
	int * value;		// Pointer to data buffer ; ch>n>=0 ; value[n] holds channel (n+1) 
} JS_CHANNELS, * PJS_CHANNELS;

/* JsChPostProc.Dll interface functions */
LONG (WINAPI *GetDllVersion)();
PJS_CHANNELS (WINAPI *ProcessChannels)(PJS_CHANNELS, int max, int min);
int  (WINAPI *pGetNumberOfFilters)(void);
const char *    (WINAPI *pGetFilterNameByIndex)(const int iFilter);
const int   (WINAPI *pGetFilterIdByIndex)(const int iFilter);
const int  (WINAPI * pSelectFilterByIndex)(const int iFilter);
const int  (WINAPI *  pGetIndexOfSelectedFilter)(void);

