// JsChPostProc.cpp : Defines the initialization routines for the DLL.
//
// Debugging info - Start
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
// Debugging info - End

#include "stdafx.h"
#include "JsChPostProc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJsChPostProcApp

BEGIN_MESSAGE_MAP(CJsChPostProcApp, CWinApp)
	//{{AFX_MSG_MAP(CJsChPostProcApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJsChPostProcApp construction

 CJsChPostProcApp::CJsChPostProcApp()
 {
 	// TODO: add construction code here,
 	// Place all significant initialization in InitInstance
 }

/////////////////////////////////////////////////////////////////////////////
// The one and only CJsChPostProcApp object

CJsChPostProcApp theApp;

/** Globals **/

// List of filters - add a new filter here
FilterItem ListOfFilters[] = 
{
	/* FilterID,	function,			Filter name*/
	{108, (void *)ConvertEskyCCPM,		"CCPM: E-sky (6ch) Type 1"},			
	{109, (void *)ConvertEskyCCPMNew,	"CCPM: E-sky (6ch) Type 2"},	// Contributed by Joseph Congiundi
	{110, (void *)ConvertEskyCCPM3D,	"CCPM: E-sky (6ch) 3D"},		// Contributed by Matthew Morrison
	{102, (void *)ConvertVTail,			"V-Tail (Ch1+Ch4)"},
	{103, (void *)ConvertDeltaWing,		"Delta Wing (Ch1+Ch2)"},
	NULL
};

int SelectedFilter = -1;	/* By default - nothing selected */

//DEL BOOL CJsChPostProcApp::InitInstance() 
//DEL {
//DEL 	
//DEL 	return CWinApp::InitInstance();
//DEL }

LONG PASCAL GetDllVersion()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// Debugging memory leaks - Start
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF );
	// Debugging memory leaks - End
	return FILTER_VER;
}

/*
	This function calls a function place holder.
	This is not a very effient way.
*/
PJS_CHANNELS PASCAL ProcessChannels(PJS_CHANNELS data, int max, int min)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ConvertChannels(data, max, min);
}



/*
Contributed by Matthew Morrison

PJS_CHANNELS ConvertEskyCCPM3D(PJS_CHANNELS data, int max, int min)

Convert E-Sky CCPM 6 channel signal into 4 orthogonal channels and a 5th for blade pitch

CCPM operation:
Rudder: Independent channel.
Elevator: Obtained by Front servo going up (or down) while both rear servos going down (or up).
Roll: Obtained by one rear servo going up and one rear servo going down.
Power: Throttle stick throw obtained from Power channel.
Input 6 channels are - 
* Channel 1 (in_data[0]) - Rear servo 1
* Channel 2 (in_data[1]) - Front servo
* Channel 3 (in_data[2]) - Power 
* Channel 4 (in_data[3]) - Rudder
* Channel 5 (in_data[4]) - Not used
* Channel 6 (in_data[6]) - Rear servo 2

Output 5 channels are - 
* Channel 1 (out_data[0]) - Rudder
* Channel 2 (out_data[1]) - Throttle
* Channel 3 (out_data[2]) - Elevator 
* Channel 4 (out_data[3]) - Ailerons
* Channel 5 (out_data[4]) - Pitch

  Input:
data Pointer to structure containing CCPM channel data
data->ch Number of input channels (only the first 6 are relevant)
data->value[] Array of channel values
min Minimum possible CCPM channel value
max Maximum possible CCPM channel value

  Return:
Pointer to structure containing orthogonal channel data (5ch)

*/
PJS_CHANNELS ConvertEskyCCPM3D(PJS_CHANNELS data, int max, int min)
{
static far JS_CHANNELS output;
static far int in_data[6]={0,0,0,0,0,0}, out_data[8]={0,0,0,0,0,0,0,0};
int servo0 = max - in_data[0]; // Servo 0 needs to be inverted


// Copy input data to input buffer
for (int i=0; i<6; i++)
in_data[i] = data->value[i];

// Rudder
out_data[0] = in_data[3];


// Throttle = Power channel
out_data[1] = in_data[2];


// Pitch: average of all servos compared to the max
out_data[4] = (int)(servo0 + in_data[5] + in_data[1])/3;


// Roll: the difference between the rear servos compared to the midpoint
out_data[3] = (max/2 + in_data[5] - servo0)/2;


// Elevator: the difference between the front servo and the mean of the rear servos
out_data[2] = max/2 + (in_data[1] - (servo0 + in_data[5])/2)/2;

output.ch = 5;
output.value = out_data;
return &output;
}



/*
	Contributed by by Joseph Congiundi

	PJS_CHANNELS ConvertEskyCCPMNew(PJS_CHANNELS data, int max, int min)

	Convert E-Sky CCPM 6 channel signal into 4 orthogonal channels

	CCPM operation:
	Rudder:	Independent channel.
	Pitch:	Obtained by Front servo going up (or down) while both rear servos going down (or up).
			Rear servos travel at 50% rate of the front servo.
			Elevator stick throw obtained from Front servo channel.
	Roll:	Obtained by one rear servo going up and one rear servo going down.
			Aileron throw obtained from a Rear servo channel with the reduction of the Pitch element.
	Power:	Throttle stick throw obtained from Power channel.
			When power channel in in the Mid to Max range, it also a Rear servo and the Front servo			
	
	Input 6 channels are - 
	* Channel 1 (in_data[0]) - Rear servo 1
	* Channel 2 (in_data[1]) - Front servo
	* Channel 3 (in_data[2]) - Power 
	* Channel 4 (in_data[3]) - Rudder
	* Channel 5 (in_data[4]) - Not used
	* Channel 6 (in_data[6]) - Rear servo 2

	Output 4 channes are - 
	* Channel 1 (out_data[0]) - Rudder
	* Channel 2 (out_data[1]) - Throttle
	* Channel 3 (out_data[2]) - Elevator 
	* Channel 4 (out_data[3]) - Ailerons

  Input:
	data			Pointer to structure containing CCPM channel data
	data->ch		Number of input channels (only the first 6 are relevant)
	data->value[]	Array of channel values
	min				Minimum possible CCPM channel value
	max				Maximum possible CCPM channel value

  Return:
	Pointer to structure containing orthogonal channel data (4ch)

*/
PJS_CHANNELS ConvertEskyCCPMNew(PJS_CHANNELS data, int max, int min)
{
	static far JS_CHANNELS output;
	static far int in_data[6]={0,0,0,0,0,0}, out_data[8]={0,0,0,0,0,0,0,0};


	// Copy input data to input buffer
	for (int i=0; i<6; i++)
		in_data[i] = data->value[i];

	// Rudder
	out_data[0] = in_data[3];

	// Throttle = Power channel
	out_data[1] = in_data[2];
	

	// Elevator: Front servo (Pitch correction when power above mid-point)
	out_data[2] = in_data[1]-(int)(.35*in_data[2]);


	// Ailerons: Rear servo minus pitch effect (roll correction when power above mid-point)
	out_data[3] = (int)(max/2+in_data[0] + .5*in_data[1]-.53*in_data[2]);


	output.ch = 4;
	output.value = out_data;
	return &output;
}

/*
	PJS_CHANNELS ConvertEskyCCPM(PJS_CHANNELS data, int max, int min)

	Convert E-Sky CCPM 6 channel signal into 4 orthogonal channels

	CCPM operation:
	Rudder:	Independent channel.
	Pitch:	Obtained by Front servo going up (or down) while both rear servos going down (or up).
			Rear servos travel at 50% rate of the front servo.
			Elevator stick throw obtained from Front servo channel.
	Roll:	Obtained by one rear servo going up and one rear servo going down.
			Aileron throw obtained from a Rear servo channel with the reduction of the Pitch element.
	Power:	Throttle stick throw obtained from Power channel.
			When power channel in in the Mid to Max range, it also a Rear servo and the Front servo			
	
	Input 6 channels are - 
	* Channel 1 (in_data[0]) - Rear servo 1
	* Channel 2 (in_data[1]) - Front servo
	* Channel 3 (in_data[2]) - Power 
	* Channel 4 (in_data[3]) - Rudder
	* Channel 5 (in_data[4]) - Not used
	* Channel 6 (in_data[6]) - Rear servo 2

	Output 4 channes are - 
	* Channel 1 (out_data[0]) - Rudder
	* Channel 2 (out_data[1]) - Throttle
	* Channel 3 (out_data[2]) - Elevator 
	* Channel 4 (out_data[3]) - Ailerons

  Input:
	data			Pointer to structure containing CCPM channel data
	data->ch		Number of input channels (only the first 6 are relevant)
	data->value[]	Array of channel values
	min				Minimum possible CCPM channel value
	max				Maximum possible CCPM channel value

  Return:
	Pointer to structure containing orthogonal channel data (4ch)

*/
PJS_CHANNELS ConvertEskyCCPM(PJS_CHANNELS data, int max, int min)
{
	static far JS_CHANNELS output;
	static far int in_data[6]={0,0,0,0,0,0}, out_data[8]={0,0,0,0,0,0,0,0};


	// Copy input data to input buffer
	for (int i=0; i<6; i++)
		in_data[i] = data->value[i];

	// Rudder
	out_data[0] = in_data[3];

	// Throttle = Power channel
	out_data[1] = in_data[2];
	

	// Elevator: Front servo (Pitch correction when power above mid-point)
	if (in_data[2]>max/2)
		out_data[2] = in_data[1]+(int)(0.15*in_data[2]);
	else
		out_data[2] = in_data[1];


	// Ailerons: Rear servo minus pitch effect (roll correction when power above mid-point)
	if (in_data[2]>max/2)
		out_data[3] = (int)(max/2+in_data[0] - 0.5*in_data[1]-0.15*in_data[2]);
	else
		out_data[3] = (int)(max/2+in_data[0] - 0.5*in_data[1]);


	output.ch = 4;
	output.value = out_data;
	return &output;
}
/*
	V-Tail un-mixing
	Take the data going to both tail servos and extract the orthogonal axes.
	It is assumed that the mixed channels are ch1 and ch4
	All other channels do not change
*/
PJS_CHANNELS ConvertVTail(PJS_CHANNELS data, int max, int min)
{
	static far JS_CHANNELS output;
	static far int in_data[12]={0,0,0,0,0,0,0,0,0,0,0,0}, out_data[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	int rudder, elevator;

	// Copy input data to input buffer
	for (int i=0; i<6; i++)
		in_data[i] = data->value[i];

	if (data->ch>=4)
	{
		rudder = (max-min)/2+(in_data[0]-in_data[3])/4;
		elevator = (in_data[0]+in_data[3])/2;

		out_data[0] = elevator;
		out_data[1] = in_data[1];
		out_data[2] = in_data[2];
		out_data[3] = rudder;
		out_data[4] = in_data[4];
		out_data[5] = in_data[5];
		out_data[6] = in_data[6];
		out_data[7] = in_data[7];
		out_data[8] = in_data[8];
		out_data[9] = in_data[9];
		out_data[10] = in_data[10];
		out_data[11] = in_data[11];
	};

	output.ch = data->ch;
	output.value = out_data;

	return &output;
}

/*
	Delta wing un-mixing
	Take the data going to both wing servos and extract the orthogonal axes.
	No rudder.
	It is assumed that the mixed channels are ch1 and ch2
	All other channels do not change
*/
PJS_CHANNELS ConvertDeltaWing(PJS_CHANNELS data, int max, int min)
{
	static far JS_CHANNELS output;
	static far int in_data[12]={0,0,0,0,0,0,0,0,0,0,0,0}, out_data[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	int ailerons, elevator;

	// Copy input data to input buffer
	for (int i=0; i<6; i++)
		in_data[i] = data->value[i];

	if (data->ch>=2)
	{
		ailerons = (max-min)/2+(in_data[0]-in_data[1])/4;
		elevator = (in_data[0]+in_data[1])/2;

		out_data[0] = elevator;
		out_data[1] = ailerons;
		out_data[2] = in_data[2];
		out_data[3] = in_data[3];
		out_data[4] = in_data[4];
		out_data[5] = in_data[5];
		out_data[6] = in_data[6];
		out_data[7] = in_data[7];
		out_data[8] = in_data[8];
		out_data[9] = in_data[9];
		out_data[10] = in_data[10];
		out_data[11] = in_data[11];
	};

	output.ch = data->ch;
	output.value = out_data;

	return &output;
}


/* 
	NO-OP
	Used when no filter is selected
*/
PJS_CHANNELS ConvertDummy(PJS_CHANNELS data, int max, int min)
{
	return data;
}

/* Return the number of supported filters */
int  PASCAL  GetNumberOfFilters(void)
{
	FilterItem * item;

	for (int i=0; i < 127 ; i++)
	{
		item = &(ListOfFilters[i]);
		if (!item || !(item->Function))
			return i;
	};
	return 0;
};


/* 
	Return a constant string representing the selected filter
	The filter is selected by its index (zero based);
	If the function is called with one-after-the-last filter index - it returns NULL.
	If the function is called with a filter index that is not implemented - it returns "".

*/
const char *  PASCAL  GetFilterNameByIndex(const int iFilter)
{
	FilterItem * item;

	int n = GetNumberOfFilters();
	if (n == iFilter)
		return NULL;

	if (iFilter > n)
		return "";

	item = &(ListOfFilters[iFilter]);		
	return item->DisplayName;
	
};

/* 
	Return the filter ID (constant integer) representing the selected filter
	The filter is selected by its index (zero based);
	If the function is called with one-after-the-last filter index - it returns ID_FLTR_DLMTR.
	If the function is called with a filter index that is not implemented - it returns ID_FLTR_EMPTY.
*/
const int  PASCAL GetFilterIdByIndex(const int iFilter)
{
	FilterItem * item;

	int n = GetNumberOfFilters();
	if (n == iFilter)
		return -1;

	if (iFilter > n)
		return -2;

	item = &(ListOfFilters[iFilter]);		
	return item->id;

}

/*
	Set active filter by Index (zero based);
	If the function succeeds - it returns 0
	Otherwise it return error code.
*/
const int  PASCAL SelectFilterByIndex(const int iFilter)
{


	int n = GetNumberOfFilters();
	SelectedFilter = iFilter;

	if (iFilter <0 || iFilter>=n)
	{
		ConvertChannels = (CC)ConvertDummy;
		return -1;
	};

	ConvertChannels = (CC)ListOfFilters[SelectedFilter].Function;
	return 0;
}

/*
	Returns the index of the currently selected filter
	Returns error code otherwise
*/
const int  PASCAL  GetIndexOfSelectedFilter(void)
{
	return SelectedFilter;
}
