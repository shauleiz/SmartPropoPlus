// JsChPostProc.cpp : Defines the initialization routines for the DLL.
//

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

#ifdef _DEBUG
FILE * gChnlLogFile;
char ChnlLogFileName[1000];
#endif
// List of filters - add a new filter here
FilterItem ListOfFilters[] = 
{
	{101, (void *)ConvertEskyCCPM,   "CCPM: E-Sky (6ch)"},
	NULL
};

int SelectedFilter = -1;

BOOL CJsChPostProcApp::InitInstance() 
{
#ifdef _DEBUG // Collect channel data - open output file
	sprintf(ChnlLogFileName,"%s\\%s", getenv("TEMP"), "JsChData.log");
	gChnlLogFile = fopen(ChnlLogFileName, "w");
	fprintf(gChnlLogFile, "CH1\tCH2\tCH3\tCH4\tCH5\tCH6\t\tRudder\tThrottle\tElevator\tAilerons");
#endif
	
	return CWinApp::InitInstance();
}

LONG PASCAL GetDllVersion()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return FILTER_VER;
}

PJS_CHANNELS PASCAL ProcessChannels(PJS_CHANNELS data, int max, int min)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return ConvertChannels(data, max, min);
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

PJS_CHANNELS ConvertEskyCCPM_2(PJS_CHANNELS data, int max, int min)
{
	static far JS_CHANNELS output;
	static far int in_data[6]={0,0,0,0,0,0}, out_data[8]={0,0,0,0,0,0,0,0};
	int Ch1,Ch2,Ch3,Ch4,Ch6; // Input channels
	int Rudder, Throttle, Elevator,Ailerons;


	// Copy input data to input buffer
	for (int i=0; i<6; i++)
		in_data[i] = data->value[i];

	// Rename input channels
	Ch1 = in_data[0]; Ch2 = in_data[1]; Ch3 = in_data[2]; Ch4 = in_data[3]; Ch6 = in_data[5];

	// Rudder
	Rudder = Ch4;

	// Throttle = Power channel
	Throttle = Ch3;
	

	// Elevator: Front servo (Pitch correction when power above mid-point)
	if (Ch3< 320)	Elevator = Ch2-134;
	else if (Ch3< 340)	Elevator = Ch2-124;
	else if (Ch3< 360)	Elevator = Ch2-119;
	else if (Ch3< 380)	Elevator = Ch2-114;
	else if (Ch3< 400)	Elevator = Ch2-104;
	else if (Ch3< 420)	Elevator = Ch2-94;
	else if (Ch3< 440)	Elevator = Ch2-86;
	else if (Ch3< 460)	Elevator = Ch2-82;
	else if (Ch3< 480)	Elevator = Ch2-74;
	else if (Ch3< 500)	Elevator = Ch2-64;
	else if (Ch3< 520)	Elevator = Ch2-58;
	else if (Ch3< 540)	Elevator = Ch2-54;
	else if (Ch3< 560)	Elevator = Ch2-50;
	else if (Ch3< 580)	Elevator = Ch2-44;
	else if (Ch3< 600)	Elevator = Ch2-42;
	else if (Ch3< 620)	Elevator = Ch2-41;
	else if (Ch3< 640)	Elevator = Ch2-40;
	else if (Ch3< 660)	Elevator = Ch2-37;
	else if (Ch3< 680)	Elevator = Ch2-34;
	else if (Ch3< 700)	Elevator = Ch2-31;
	else if (Ch3< 720)	Elevator = Ch2-29;
	else if (Ch3< 740)	Elevator = Ch2-26;
	else if (Ch3< 760)	Elevator = Ch2-26;
	else if (Ch3< 780)	Elevator = Ch2-25;
	else if (Ch3< 800)	Elevator = Ch2-23;
	else if (Ch3< 820)	Elevator = Ch2-15;
	else if (Ch3< 840)	Elevator = Ch2-12;
	else if (Ch3< 860)	Elevator = Ch2-10;
	else if (Ch3< 880)	Elevator = Ch2-8;
	else if (Ch3< 900)	Elevator = Ch2-6;
	else if (Ch3< 920)	Elevator = Ch2-2;
	else 	Elevator = Ch2;

	// Ailerons: Rear servo minus pitch effect (roll correction when power above mid-point)
	Ailerons = (int)(max/4+Ch1 - 0.5*Ch2 + 1.5*(Ch2-Elevator));

	output.ch = 4;
	out_data[0] = Rudder; out_data[1] =  Throttle; out_data[2] = Elevator; out_data[3] = Ailerons;
	output.value = out_data;
#ifdef _DEBUG // Collect channel data - open output file
	fprintf(gChnlLogFile, "\n%d\t%d\t%d\t%d\t%d\t%d\t\t%d\t%d\t%d\t%d" ,\
		in_data[0],in_data[1],in_data[2],in_data[3],in_data[4],in_data[5],out_data[0],out_data[1],out_data[2],out_data[3]);
#endif
	return &output;
}



/* NO-OP */
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
