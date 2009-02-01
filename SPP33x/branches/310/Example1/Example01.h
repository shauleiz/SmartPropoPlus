#include <Windows.h>
#include <stdio.h>

/* Structures */
typedef struct _JS_CHANNELS	// Joystick channel data
{
	int ch;			// Number of channels
	int * value;		// Pointer to data buffer ; ch>n>=0 ; value[n] holds channel (n+1) 
} JS_CHANNELS, * PJS_CHANNELS;

/* Global parameters */
HINSTANCE g_hFilter;
int g_selected_filter;

/* Function prototypes */
HINSTANCE load_filter_dll();
int	is_inimal_api(HINSTANCE h);
int select_filter(HINSTANCE hFilter);
int set_filter(HINSTANCE hFilter, const int i_selected);
void run_filter(HINSTANCE hFilter);
void print_ver(HINSTANCE hFilter);


/* DLL API */
LONG (WINAPI *pGetDllVersion)();
PJS_CHANNELS (WINAPI *pProcessChannels)(PJS_CHANNELS, int max, int min);
int  (WINAPI *pGetNumberOfFilters)(void);
const char *    (WINAPI *pGetFilterNameByIndex)(const int iFilter);
const int   (WINAPI *pGetFilterIdByIndex)(const int iFilter);
const int  (WINAPI * pSelectFilterByIndex)(const int iFilter);
const int  (WINAPI *  pGetIndexOfSelectedFilter)(void);
