// Header file for Post-Processing filters
// 
// If you are implementing your own filter look for notes starting with:
//  "/* [NEW]"

#ifdef FILTER_EXPORTS
#define FILTER_API /*__declspec(dllexport)*/
#else
#define FILTER_API __declspec(dllimport)
#endif

#define FILTER_VER	0x00040000

extern FILTER_API int nFilter;

FILTER_API int fnFilter(void);


typedef struct _JS_CHANNELS	// Joystick channel data
{
	int ch;			// Number of channels
	int * value;	// Pointer to data buffer ; ch>n>=0 ; value[n] holds channel (n+1) 
} JS_CHANNELS, * PJS_CHANNELS;

struct FilterItem {
	int	id;				// Filter ID
	void * Function;	// Filter Function
	char * DisplayName;	// Filter name as displayed in the menu
} ;


// Interface functions - don't change
extern "C"  FILTER_API LONG PASCAL GetDllVersion();
extern "C"  FILTER_API PJS_CHANNELS PASCAL ProcessChannels(PJS_CHANNELS, int max, int min);
extern "C"  FILTER_API int PASCAL GetNumberOfFilters();
extern "C"  FILTER_API const char * PASCAL GetFilterNameByIndex(const int iFilter);
extern "C"  FILTER_API const int PASCAL GetFilterIdByIndex(const int iFilter);
extern "C"  FILTER_API const int PASCAL SelectFilterByIndex(const int iFilter);
extern "C"  FILTER_API const int PASCAL GetIndexOfSelectedFilter(void);

typedef  PJS_CHANNELS ( * CC)(PJS_CHANNELS, int max, int min);
CC ConvertChannels;
PJS_CHANNELS ConvertDummy(PJS_CHANNELS data, int max, int min);

// Filter forward declarations
// Examples for filters
PJS_CHANNELS ConvertEskyCCPM(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertEskyCCPMNew(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertEskyCCPM3D(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertVTail(PJS_CHANNELS, int max, int min);
PJS_CHANNELS ConvertDeltaWing(PJS_CHANNELS, int max, int min);
/* [NEW]
Here you can add forward declaration for your own filter, following the following example:
PJS_CHANNELS MyNewFilter(PJS_CHANNELS, int max, int min);
*/

/** Globals **/
int SelectedFilter = -1;	/* By default - nothing selected */

// List of filters
FilterItem ListOfFilters[] = 
{
	/* FilterID,	function,			Filter name*/
	{108, (void *)ConvertEskyCCPM,		"CCPM: E-sky (6ch) Type 1"},			
	{109, (void *)ConvertEskyCCPMNew,	"CCPM: E-sky (6ch) Type 2"},	// Contributed by Joseph Congiundi
	{110, (void *)ConvertEskyCCPM3D,	"CCPM: E-sky (6ch) 3D"},		// Contributed by Matthew Morrison
	{102, (void *)ConvertVTail,			"V-Tail (Ch1+Ch4)"},
	{103, (void *)ConvertDeltaWing,		"Delta Wing (Ch1+Ch2)"},
	/* [NEW]
	Here you can add entry for your own filter, following the following example:
	{104, (void *)MyNewFilter,		"My New Filter - Example only"},*/
	NULL
};


