// JsChPostProc.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "JsChPostProc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

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

	int adapted_ch1 = (int)100 * (in_data[0]-((max+min)/2))/((max+min)/2)-min); // Rear servo 1
	int adapted_ch2 = (int)100 * (in_data[1]-((max+min)/2))/((max+min)/2)-min); // Front servo 2
	int adapted_ch3 = (int)100 * (in_data[2]-((max+min)/2))/((max+min)/2)-min); // Throttle
	

	// Elevator: Front servo (Pitch correction for all power throttle stick movement)

	out_data[2] = (int)-(0.1875 * adapted_ch3 + adapted_ch2) / 0.8125; //Elevator value


	// Ailerons: Rear servo minus pitch effect (roll correction for all power throttle stick movement)

	out_data[3] = (int)(0.1875 * adapted_ch3 - adapted_ch1 - 0.8125*0.3333 * out_data[2]) / (0.8125 * 0.6666); // Aileron value

 
	output.ch = 4;
	output.value = out_data;
	return &output;
}
