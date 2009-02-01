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

BOOL CJsChPostProcApp::InitInstance() 
{
	// TODO: Add your specialized code here and/or call the base class
	
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
	return ConvertEskyCCPM(data, max, min);
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

/* Return the number of supported filters */
int  PASCAL  GetNumberOfFilters(void)
{
	return 1;
};


/* 
	Return a constant string representing the selected filter
	The filter is selected by its index (zero based);
	If the function is called with one-after-the-last filter index - it returns NULL.
	If the function is called with a filter index that is not implemented - it returns "".

*/
const char *  PASCAL  GetFilterNameByIndex(const int iFilter)
{
	if (iFilter == 0) 
		return "CCPM: E-Sky (6ch)";	
	if (iFilter > 0) 
		return NULL;
	else 
		return "";
	
};

/* 
	Return the filter ID (constant integer) representing the selected filter
	The filter is selected by its index (zero based);
	If the function is called with one-after-the-last filter index - it returns ID_FLTR_DLMTR.
	If the function is called with a filter index that is not implemented - it returns ID_FLTR_EMPTY.
*/
const int  PASCAL GetFilterIdByIndex(const int iFilter)
{
	if (iFilter == 0) 
		return 0;	
	if (iFilter > 0) 
		return -2;
	
	else 
		return -1;

}

/*
	Set active filter by Index (zero based);
	If the function succeeds - it returns 0
	Otherwise it return error code.
*/
const int  PASCAL SelectFilterByIndex(const int iFilter)
{
	return -1;
}

/*
	Returns the index of the currently selected filter
	Returns error code otherwise
*/
const int  PASCAL  GetIndexOfSelectedFilter(void)
{
	return -1;
}
