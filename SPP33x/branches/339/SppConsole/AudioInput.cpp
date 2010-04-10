// AudioInput.cpp: implementation of the CAudioInput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sppconsole.h"
#include "AudioInput.h"
#include <math.h>
#include "SmartPropoPlus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Generic functions
//////////////////////////////////////////////////////////////////////

bool isSpk(unsigned int type)
{
	switch (type)
	{
	case  MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
	case  MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
	case  MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER:
		return true;
	default:
		return false;
	};
}


MMRESULT GetLineControlByID(HMIXEROBJ hMixerDevice, unsigned long ControlID, MIXERCONTROL * Control)
{
	MMRESULT mmres;
	MIXERLINECONTROLS mixControls;				
				
	// Get the controls for the current destination mixer line
	mixControls.cbStruct = sizeof(MIXERLINECONTROLS);
	mixControls.dwControlID = ControlID;
	mixControls.cControls = 1;			// Number of controls
	mixControls.cbmxctrl = sizeof(MIXERCONTROL);
	mixControls.pamxctrl = Control;
	// Get ALL line controls for this  line 
	mmres = mixerGetLineControls((HMIXEROBJ)hMixerDevice,  &mixControls, MIXER_GETLINECONTROLSF_ONEBYID);
				
	return mmres;				
}



/*
	Set a list of mixer controls BOOLEAN details for a given control ID
*/
MMRESULT Mute(HMIXER hMixerDevice, unsigned int ControlID, bool value)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;
	MIXERCONTROLDETAILS_BOOLEAN list;
	MIXERCONTROL Control;

	/* Get control info by the given control ID */
	mmres = GetLineControlByID((HMIXEROBJ) hMixerDevice, ControlID, &Control);
	if(mmres != MMSYSERR_NOERROR) return false;
	if (Control.dwControlType != MIXERCONTROL_CONTROLTYPE_MUTE) return false;
	
	/* Set the mute value */
	list.fValue = value;
	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = ControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = 0;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mixDetails.paDetails = &list;
	mmres = mixerSetControlDetails((HMIXEROBJ)hMixerDevice, (LPMIXERCONTROLDETAILS) &mixDetails,	MIXER_SETCONTROLDETAILSF_VALUE);
				
	return mmres;
				
}


/*
	Get the value of MUTE for a given control by control ID
	The value is true or false
	Return true if value is valid
*/
bool GetMuteValue(HMIXER hMixerDevice, unsigned int ControlID, bool * value)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;
	MIXERCONTROLDETAILS_BOOLEAN list;
	MIXERCONTROL Control;

	/* Get control info by the given control ID */
	mmres = GetLineControlByID((HMIXEROBJ) hMixerDevice, ControlID, &Control);
	if(mmres != MMSYSERR_NOERROR) return false;
	if (Control.dwControlType != MIXERCONTROL_CONTROLTYPE_MUTE) return false;

	/* Get volume absolute value from control */
	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = ControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = NULL;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mixDetails.paDetails = &list;
	mmres = mixerGetControlDetails((HMIXEROBJ)hMixerDevice, (LPMIXERCONTROLDETAILS) &mixDetails,	MIXER_GETCONTROLDETAILSF_VALUE);
	if (list.fValue)
		*value = true;
	else
		*value = false;
	
	return true;
}


long GetSelSrcLineID(HMIXER hMixerDevice, unsigned int DstLineID)
{
	MMRESULT mmres;
	MIXERLINE mxl;
	MIXERCONTROL ControlList[50]  = {0};
	MIXERCONTROLDETAILS_LISTTEXT mixList[50];
	MIXERCONTROLDETAILS_BOOLEAN mixBoolean[50];
	long SelSrc = -1;
	
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwLineID = DstLineID;
	mmres = mixerGetLineInfo((HMIXEROBJ)hMixerDevice, &mxl, MIXER_GETLINEINFOF_LINEID);
	if (mmres != MMSYSERR_NOERROR)
		return -1;
	GetLineControlList((HMIXEROBJ)hMixerDevice, mxl.dwLineID, ControlList);
	/* Loop on all source-line controls */
	for  (unsigned int iCtrl=0; iCtrl<mxl.cControls; iCtrl++)
	{
		/* For a given control in line Put result in mixList - a list of sub-control names and parameters */
		if (!ControlList[iCtrl].cMultipleItems)
			continue;
		mmres = GetControlDetailList(hMixerDevice, &mixList[0], ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
		if(mmres != MMSYSERR_NOERROR)	continue ;
		
		/* Put result in mixBoolean - a list of boolean values (Selected) */
		mmres = GetControlDetailList(hMixerDevice, &mixBoolean[0],  ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
		if(mmres != MMSYSERR_NOERROR) continue ;
		
		/* Get the selected item */
		for (unsigned int i=0 ; i<ControlList[iCtrl].cMultipleItems ; i++)
		{
			if (mixBoolean[i].fValue)
				SelSrc = mixList[i].dwParam1;
			
		};
	};
	
	return SelSrc;
}




/*
	Set a list of mixer controls BOOLEAN details for a given control ID
*/
MMRESULT SetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_BOOLEAN list, DWORD cMultipleItems, unsigned int dwControlID)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;
	
	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = dwControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = (HWND)cMultipleItems;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mixDetails.paDetails = list;
	mmres = mixerSetControlDetails((HMIXEROBJ)hmxobj, (LPMIXERCONTROLDETAILS) &mixDetails,	MIXER_SETCONTROLDETAILSF_VALUE);
				
	return mmres;
				
}

/*
	Set a list of mixer controls UNSIGNED details for a given control ID
*/
MMRESULT SetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_UNSIGNED list, DWORD cMultipleItems, unsigned int dwControlID)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;
	
	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = dwControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = (HWND)cMultipleItems;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mixDetails.paDetails = list;
	mmres = mixerSetControlDetails((HMIXEROBJ)hmxobj, (LPMIXERCONTROLDETAILS) &mixDetails,	MIXER_SETCONTROLDETAILSF_VALUE);
				
	return mmres;
				
}


/* Get list of controls for a given line (By mixer and line ID) */
MMRESULT GetLineControlList(HMIXEROBJ hMixerDevice, unsigned long LineID, MIXERCONTROL * ControlList, int * cControls)
{
	MMRESULT mmres;
	MIXERLINECONTROLS mixControls;
	MIXERLINE tmp_mxl;
	
	tmp_mxl.cbStruct = sizeof(MIXERLINE);
	tmp_mxl.dwLineID = LineID;
	mmres = mixerGetLineInfo((HMIXEROBJ)hMixerDevice, &tmp_mxl, MIXER_GETLINEINFOF_LINEID);
	if (mmres != MMSYSERR_NOERROR)
		return mmres;
				
				
	// Get the controls for the current destination mixer line
	mixControls.cbStruct = sizeof(MIXERLINECONTROLS);	
	mixControls.dwLineID = LineID;			// Line ID
	mixControls.cControls = tmp_mxl.cControls;			// Number of controls
	mixControls.cbmxctrl = sizeof(MIXERCONTROL);
	mixControls.pamxctrl = ControlList;
	// Get ALL line controls for this  line 
	mmres = mixerGetLineControls((HMIXEROBJ)hMixerDevice,  &mixControls, MIXER_GETLINECONTROLSF_ALL);

	if (cControls)
		*cControls = mixControls.cControls;
	return mmres;
				
}

/*
	Wrapper of mixerGetControlDetails() with parameter MIXER_GETCONTROLDETAILSF_LISTTEXT
	For a multiple-item control, this function fills data for every control in 'list' which is an array of MIXERCONTROLDETAILS_LISTTEXT
	Used to get information about the control item (Name, type etc.)
*/
MMRESULT GetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_LISTTEXT list, DWORD cMultipleItems, unsigned int dwControlID)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;

	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = dwControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = (HWND)cMultipleItems;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
	mixDetails.paDetails = list;
	mmres = mixerGetControlDetails((HMIXEROBJ)hmxobj,(LPMIXERCONTROLDETAILS) &mixDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT);
				
	return mmres;
}


/*
	Wrapper of mixerGetControlDetails() with parameter MIXER_GETCONTROLDETAILSF_VALUE
	For a multiple-item control, this function fills boolean data for every control in 'list'.
	Used for detection of MUTE 
*/
MMRESULT GetControlDetailList(HMIXER hmxobj, LPMIXERCONTROLDETAILS_BOOLEAN list, DWORD cMultipleItems, unsigned int dwControlID)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;

	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = dwControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = (HWND)cMultipleItems;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
	mixDetails.paDetails = list;
	mmres = mixerGetControlDetails((HMIXEROBJ)hmxobj,(LPMIXERCONTROLDETAILS) &mixDetails, MIXER_GETCONTROLDETAILSF_VALUE);
				
	return mmres;

}


/*
	Get the value of the volume for a given control by control ID
	The value is normalized for to a scale of 0-100
	Return true if value is valid
*/
bool GetVolumeValue(HMIXER hMixerDevice, unsigned int ControlID, long * value)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS mixDetails;
	MIXERCONTROLDETAILS_BOOLEAN list;
	MIXERCONTROL Control;
	double Volume;

	/* Get control info by the given control ID */
	mmres = GetLineControlByID((HMIXEROBJ) hMixerDevice, ControlID, &Control);
	if(mmres != MMSYSERR_NOERROR) return false;
	if (Control.dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME) return false;

	/* Get volume absolute value from control */
	mixDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mixDetails.dwControlID = ControlID;
	mixDetails.cChannels = 1;
	mixDetails.hwndOwner = NULL;
	mixDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
	mixDetails.paDetails = &list;
	mmres = mixerGetControlDetails((HMIXEROBJ)hMixerDevice, (LPMIXERCONTROLDETAILS) &mixDetails,	MIXER_GETCONTROLDETAILSF_VALUE);
	Volume = list.fValue;
	
	/* Normalize volume value into a 0-100 range*/
	Volume = ceil(100*(Volume-Control.Bounds.dwMinimum)/(Control.Bounds.dwMaximum-Control.Bounds.dwMinimum));
	*value = (long)Volume;
	return true;
}


int SetVolumeValue(HMIXER hMixerDevice, unsigned int ControlID, long  value)
{
	MMRESULT mmres;
	MIXERCONTROLDETAILS_UNSIGNED mixDetails;
	MIXERCONTROL Control;
	long OldValue;

	/* Get control info by the given control ID */
	mmres = GetLineControlByID((HMIXEROBJ) hMixerDevice, ControlID, &Control);
	if(mmres != MMSYSERR_NOERROR) return -1;
	if (Control.dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME) return -1;

	/* aligne to boundries */
	if (value>100)
		value=100;
	else
		if (value<0)
			value=0;

	/* Get the current value */
	if (!GetVolumeValue(hMixerDevice,  ControlID, &OldValue))
		OldValue = -1;

	/* Set value*/
	mmres = mixerGetControlDetails((HMIXEROBJ)hMixerDevice, (LPMIXERCONTROLDETAILS) &mixDetails,	MIXER_GETCONTROLDETAILSF_VALUE);
	mixDetails.dwValue = value*(Control.Bounds.dwMaximum-Control.Bounds.dwMinimum)/100 + Control.Bounds.dwMinimum;
	mmres = SetControlDetailList(hMixerDevice, &mixDetails, NULL, ControlID);
	
	return (OldValue);
}



/***************************************************************************************
	Class CAudioInput

	Object represents the audio system (single object)
	members consist of list of mixer devices & index of current mixer device
***************************************************************************************/

CAudioInput::CAudioInput()
{

	CMixerDevice * mixer;
	m_CurrentMixerDevice = -1;

	// Get the number of mixer devices
	unsigned int nMixDev = mixerGetNumDevs();
	if (!nMixDev)
		return;

	// Create the array of mixer devices
	for (unsigned int index=0; index<nMixDev ; index++)
	{
		mixer = new CMixerDevice(index);
		m_ArrayMixerDevice.Add(mixer);
	};

}

CAudioInput::~CAudioInput()
{
	/* Destroy the list of Mixer devices */
	while (m_ArrayMixerDevice.GetSize())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(0);
		delete(md);
		m_ArrayMixerDevice.RemoveAt(0);
	};

	/* Distroy the container itself */
	m_ArrayMixerDevice.RemoveAll();
}

int CAudioInput::GetCountMixerDevice()
{
	return (int)m_ArrayMixerDevice.GetSize();
}




const char * CAudioInput::GetMixerDeviceName(int index)
{
	if (index>=0 && index<GetCountMixerDevice())
		return m_ArrayMixerDevice.GetAt(index)->GetName();
	else
		return "WAVE_MAPPER";
}




const char * CAudioInput::GetMixerDeviceInputLineName(int Mixer, int Line)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return NULL;
		return md->GetInputLineName(Line);
	}
	else
		return NULL;
}

bool CAudioInput::SetMixerDeviceSelectInputLine(int Mixer, int Line)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return false;
		return md->SetSelectedInputLine(Line);
	}
	else
		return false;
}

bool CAudioInput::GetMixerDeviceSelectInputLine(int Mixer, unsigned int * iLine)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return false;
		return md->GetSelectedInputLine(iLine);
	}
	else
		return false;
}

bool CAudioInput::GetMixerDeviceInputLineIndex(int Mixer, unsigned int SrcID, unsigned int * Index)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return false;
		return md->GetInputLineIndex(SrcID, Index);
	}
	else
		return false;
}

bool CAudioInput::GetMixerDeviceInputLineSrcID(int Mixer, unsigned int * SrcID, unsigned int Index)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return false;
		return md->GetInputLineSrcID(SrcID, Index);
	}
	else
		return false;
}

bool CAudioInput::MuteSelectedInputLine(int Mixer, unsigned int line, bool mute, bool temporary)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return false;
		return md->MuteSelectedInputLine(line, mute, temporary);
	}
	else
		return false;

}

int CAudioInput::SetSpeakers(int Mixer, bool restore, bool mute)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		if (!md)
			return false;
		return md->SetSpeakers(restore, mute);
	}
	else
		return false;

}

CAudioInput::CMixerDevice * CAudioInput::GetMixerDevice(int Mixer)
{
	if (Mixer>=0 && Mixer<GetCountMixerDevice())
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		return md;
	}
	else
		return NULL;
}




void CAudioInput::Restore()
{
	CMixerDevice * mixer;

	// Get the number of mixer devices
	int nMixDev = mixerGetNumDevs();
	if (!nMixDev)
		return;

	for (int index=0; index<nMixDev ; index++)
	{
		mixer = m_ArrayMixerDevice.GetAt(index);
		mixer->Restore();
	};
}

int CAudioInput::GetMixerDeviceIndex(char *mixer)
{
	const char * MixerName;

	int cMixer = GetCountMixerDevice();
	for  (int Mixer=0 ; Mixer<cMixer ; Mixer++)
	{
		CMixerDevice * md = m_ArrayMixerDevice.GetAt(Mixer);
		MixerName = md->GetName();
		if (!strcmp(MixerName,mixer))
			return Mixer;
	};

	return 0; // Default
}

bool CAudioInput::SetCurrentMixerDevice(int i)
{
	int cMixer = GetCountMixerDevice();
	if (i>=0 && i<cMixer)
	{
		m_CurrentMixerDevice = i;
		return true;
	}
	else
	{
		m_CurrentMixerDevice = -1;
		return false;
	};
}


int CAudioInput::GetCurrentMixerDevice()
{
	return m_CurrentMixerDevice;
}


/***************************************************************************************
	Class CMixerDevice
	Hierarchy: CAudioInput::CMixerDevice

	Object represents an audio mixer device
	members consist of handle to the mixer device, ID, name, 
	array of Input Lines, array of physical devices
***************************************************************************************/
CAudioInput::CMixerDevice::CMixerDevice()
{
	CMixerDevice(0);
}

CAudioInput::CMixerDevice::~CMixerDevice()
{
	/* Restore the original situation */
	Restore();
	
	/* Destroy the list of Input Lines */
	while (m_ArrayInputLine.GetSize())
	{
		CInputLine * il = m_ArrayInputLine.GetAt(0);
		delete(il);
		m_ArrayInputLine.RemoveAt(0);
	};
	m_ArrayInputLine.RemoveAll();

	/* Destroy the list of Physical Devices */
	while (m_ArrayPhysicalDev.GetSize())
	{
		CPhysicalDevice * phd = m_ArrayPhysicalDev.GetAt(0);
		delete(phd);
		m_ArrayPhysicalDev.RemoveAt(0);
	};
	m_ArrayPhysicalDev.RemoveAll();
}

CAudioInput::CMixerDevice::CMixerDevice(int index)
{
	unsigned int mmres;
	MIXERCAPS mixCaps;

	/* Initialize Mixer Device */
	m_ID = index;
	m_CurrentSrcID = -1;
	m_ArrayInputLine.RemoveAll( );

	mmres = mixerOpen((LPHMIXER) &m_hMixerDevice, m_ID, (DWORD) 0, (DWORD)NULL, MIXER_OBJECTF_MIXER);
	if(mmres != MMSYSERR_NOERROR)
		return;

	/* Get Mixer Device' capabilities */
	mmres = mixerGetDevCaps((UINT_PTR)m_hMixerDevice, (LPMIXERCAPS) &mixCaps,	sizeof(MIXERCAPS));
	if(mmres != MMSYSERR_NOERROR) return;

	/* Device name */
	m_name = strdup(mixCaps.szPname);

	/* Create an array of input lines for this mixer device */
	int nInputLines = CreateInputLineArray();

	/* Create an array of Physical devices (Speakers Headphones etc.) */
	int nPhysicalDevices = CreatePhysicalDevArray();
}

/* 
	Go over every InputLine - Set the given line to default volume value reset all the rest
*/

void CAudioInput::CMixerDevice::AdjustInputVolume(unsigned long SrcID)
{
	CInputLine * il;
	int i;
	int PrevVolume;

	INT_PTR nLines = m_ArrayInputLine.GetSize();

	/* Set the default value to the selected */
	for (i=0 ; i<nLines ; i++)
	{
		il = m_ArrayInputLine.GetAt(i);
		if (il->GetSrcID() == SrcID)
			il->SetDefaultVolume();
	};
	
	if (m_CurrentSrcID >= 0) /* Restore former value only if former ID is valid  */
	{
		/* Set the previous value to the un-selected */
		for (i=0 ; i<nLines ; i++)
		{
			il = m_ArrayInputLine.GetAt(i);
			if (il->GetSrcID() == (unsigned long)m_CurrentSrcID)
			{	// This was the previous selected control
				// Get the volume value and save it as default value
				PrevVolume = il->StoreDefaultVolume();
				il->RestoreVolume(); // Restore the original volume
			};
		};
	};
}


/*
	Create an array of InputLine
	Input line is an object that holds info about a selection control 
	and the relevant source lines.
*/
int CAudioInput::CMixerDevice::CreateInputLineArray()
{
	MIXERLINE mxl;
	HMIXER hMixerDevice;
	unsigned int mmres;
	MIXERCAPS mixCaps;
	int nDestLines;
	MIXERCONTROL ControlList[50]  = {0};
	MIXERCONTROLDETAILS_LISTTEXT mixList[50];
	CInputLine * pInputLine;
	MIXERCONTROLDETAILS_BOOLEAN mixBoolean[50];


	/* Initialize Mixer Device */
	mmres = mixerOpen((LPHMIXER) &hMixerDevice, m_ID, (DWORD) 0, (DWORD)NULL, MIXER_OBJECTF_MIXER);
	if(mmres != MMSYSERR_NOERROR)
		return 0;

	/* Get Mixer Device' capabilities */
	mmres = mixerGetDevCaps((INT_PTR)hMixerDevice, (LPMIXERCAPS) &mixCaps,	sizeof(MIXERCAPS));
	if(mmres != MMSYSERR_NOERROR) return 0;
	nDestLines = mixCaps.cDestinations;

	mxl.cbStruct = sizeof(MIXERLINE);
	
	/* Loop on all destinations of device DeviceId */
	for (int i=0 ; i<nDestLines ; i++)
	{
		/* Get Destination line info: ID, */
		mxl.dwDestination = i;
		mmres = mixerGetLineInfo((HMIXEROBJ)hMixerDevice, &mxl, MIXER_GETLINEINFOF_DESTINATION);
		if (mmres != MMSYSERR_NOERROR)
			break;

		/* Get list of destination line controls */
		GetLineControlList((HMIXEROBJ)hMixerDevice, mxl.dwLineID, ControlList);

		/* Loop on all destination line controls */
		for  (unsigned int iCtrl=0; iCtrl<mxl.cControls; iCtrl++)
		{
			/* For a given control in line Put result in mixList - a list of sub-control names and parameters */
			if (!ControlList[iCtrl].cMultipleItems)
				continue;

			/* only MUX and similar controls */
			if ((ControlList[iCtrl].dwControlType&MIXERCONTROL_CT_CLASS_MASK) != MIXERCONTROL_CT_CLASS_LIST)
				continue;

			/* Get list of sub-controls of the MUX control */
			mmres = GetControlDetailList(hMixerDevice, &mixList[0],   ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
			if(mmres != MMSYSERR_NOERROR)	continue ;

			/* Put result in mixBoolean - a list of boolean values (Selected) */
			mmres = GetControlDetailList(m_hMixerDevice, &mixBoolean[0],   ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
			if(mmres != MMSYSERR_NOERROR) continue  ;

			/* Create an entry for every sub-controls of the MUX control */
			for (unsigned iSubControl=0; iSubControl<ControlList[iCtrl].cMultipleItems; iSubControl++)
			{
				pInputLine = new CInputLine(hMixerDevice, mixList[iSubControl].dwParam1, mixList[iSubControl].szName, mixList[iSubControl].dwParam2, mixBoolean[iSubControl].fValue);
				m_ArrayInputLine.Add(pInputLine);
			};
		}
		
	};
	
	return (int)m_ArrayInputLine.GetSize();
}


/*
	Create an array of Physical Output Device objects
	Return the number of object created
*/
int CAudioInput::CMixerDevice::CreatePhysicalDevArray()
{
	MIXERLINE mxl;
	HMIXER hMixerDevice;
	unsigned int mmres;
	MIXERCAPS mixCaps;
	int nDestLines;
	MIXERCONTROL ControlList[50]  = {0};
	long volume;
	bool mute;
	bool found_vol, found_mute;
	unsigned long muteCtrlID, volumeCtrlID;


	/* Initialize Mixer Device */
	mmres = mixerOpen((LPHMIXER) &hMixerDevice, m_ID, (DWORD) 0, (DWORD)NULL, MIXER_OBJECTF_MIXER);
	if(mmres != MMSYSERR_NOERROR)
		return 0;

	/* Get Mixer Device' capabilities */
	mmres = mixerGetDevCaps((INT_PTR)hMixerDevice, (LPMIXERCAPS) &mixCaps,	sizeof(MIXERCAPS));
	if(mmres != MMSYSERR_NOERROR) return 0;
	nDestLines = mixCaps.cDestinations;

	mxl.cbStruct = sizeof(MIXERLINE);
	m_ArrayPhysicalDev.RemoveAll( );

	/* Loop on all destinations of device DeviceId */
	for (int i=0 ; i<nDestLines ; i++)
	{
		/* Get Destination line info: ID, */
		mxl.dwDestination = i;
		mmres = mixerGetLineInfo((HMIXEROBJ)hMixerDevice, &mxl, MIXER_GETLINEINFOF_DESTINATION);
		if (mmres != MMSYSERR_NOERROR)
			break;

		/* Continue only if this is a speaker, headphone etc. */
		if (!isSpk(mxl.dwComponentType))
			continue;

		/* Get list of destination line controls */
		GetLineControlList((HMIXEROBJ)hMixerDevice, mxl.dwLineID, ControlList);


		mute = found_vol = found_mute = false;
		/* Loop on all destination line controls */
		for  (unsigned int iCtrl=0; iCtrl<mxl.cControls; iCtrl++)
		{
			/* If volume */
			if ((ControlList[iCtrl].dwControlType==MIXERCONTROL_CONTROLTYPE_VOLUME))
			{
				found_vol = true;
				volumeCtrlID = ControlList[iCtrl].dwControlID;
				GetVolumeValue(hMixerDevice, ControlList[iCtrl].dwControlID , &volume);
			};

			/* If mute */
			if ((ControlList[iCtrl].dwControlType==MIXERCONTROL_CONTROLTYPE_MUTE))
			{
				found_mute = true;
				muteCtrlID = ControlList[iCtrl].dwControlID;
				GetMuteValue(hMixerDevice, ControlList[iCtrl].dwControlID , &mute);
			};

		};

		if (found_mute || found_vol)
		{
			CPhysicalDevice * phd = new CPhysicalDevice(m_hMixerDevice, mxl.dwLineID,  muteCtrlID, mute,  volumeCtrlID, volume);
			m_ArrayPhysicalDev.Add(phd);
		};
		
	};
	
	return (int)m_ArrayPhysicalDev.GetSize();
}

bool CAudioInput::CMixerDevice::GetInputLineIndex(unsigned int SrcID, unsigned int * Index)
{
	CInputLine * il;

	INT_PTR nLines = m_ArrayInputLine.GetSize();
	for (int i=0 ; i<nLines ; i++)
	{
		il = m_ArrayInputLine.GetAt(i);
		if (il->GetSrcID() == SrcID)
		{
			*Index = i;
			return true;
		};
	};
		return false;
}


const char * CAudioInput::CMixerDevice::GetInputLineName(int i)
{
	if (i >= m_ArrayInputLine.GetSize() || i<0)
		return NULL;

	return m_ArrayInputLine.GetAt(i)->GetName();
}


bool CAudioInput::CMixerDevice::GetInputLineSrcID(unsigned int *SrcID, unsigned int Index)
{
	CInputLine * il;
	UINT_PTR nLines = m_ArrayInputLine.GetSize();
	if (Index >= nLines)
		return false;

	il = m_ArrayInputLine.GetAt(Index);
	if (il)
	{
		*SrcID = il->GetSrcID();
		return true;
	}
	else
		return false;
}


const char * CAudioInput::CMixerDevice::GetName()
{
	return m_name;
}

bool CAudioInput::CMixerDevice::GetSelectedInputLine(unsigned int *iLine)
{
	MIXERLINE mxl;
	unsigned int mmres;
	MIXERCAPS mixCaps;
	int nDestLines;
	MIXERCONTROL ControlList[50]  = {0};
	MIXERCONTROLDETAILS_LISTTEXT mixList[50];
	MIXERCONTROLDETAILS_BOOLEAN mixBoolean[50];

	/* Default */
	*iLine = -1;

	/* Get Mixer Device' capabilities */
	mmres = mixerGetDevCaps((INT_PTR)m_hMixerDevice, (LPMIXERCAPS) &mixCaps,	sizeof(MIXERCAPS));
	if(mmres != MMSYSERR_NOERROR) return false;
	nDestLines = mixCaps.cDestinations;

	mxl.cbStruct = sizeof(MIXERLINE);
	
	/* Loop on all destinations of device DeviceId */
	for (int i=0 ; i<nDestLines ; i++)
	{
		/* Get Destination line info: ID, */
		mxl.dwDestination = i;
		mmres = mixerGetLineInfo((HMIXEROBJ)m_hMixerDevice, &mxl, MIXER_GETLINEINFOF_DESTINATION);
		if (mmres != MMSYSERR_NOERROR)
			break;

		/* Get list of destination line controls */
		GetLineControlList((HMIXEROBJ)m_hMixerDevice, mxl.dwLineID, ControlList);

		/* Loop on all destination line controls */
		for  (unsigned int iCtrl=0; iCtrl<mxl.cControls; iCtrl++)
		{
			/* For a given control in line Put result in mixList - a list of sub-control names and parameters */
			if (!ControlList[iCtrl].cMultipleItems)
				continue;

			/* only MUX and similar controls */
			if ((ControlList[iCtrl].dwControlType&MIXERCONTROL_CT_CLASS_MASK) != MIXERCONTROL_CT_CLASS_LIST)
				continue;

			/* Get list of sub-controls of the MUX control */
			mmres = GetControlDetailList(m_hMixerDevice, &mixList[0],   ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
			if(mmres != MMSYSERR_NOERROR)	continue ;

			/* Put result in mixBoolean - a list of boolean values (Selected) */
			mmres = GetControlDetailList(m_hMixerDevice, &mixBoolean[0],  ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
			if(mmres != MMSYSERR_NOERROR) return  false;

			/* Select */
			for (unsigned int i=0 ; i<ControlList[iCtrl].cMultipleItems ; i++)
			{
				if (mixBoolean[i].fValue)
				{
					*iLine = i;
					return true;
				};
			};
			//SetControlDetailList(m_hMixerDevice, &mixBoolean[0],  (HWND) ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
		}
		
	};
	return false;
}


/* 
	Go over every InputLine - go to the corresponding output line
	Mute/Unmute according to the line selection state
*/

bool CAudioInput::CMixerDevice::MuteOutputLine(unsigned long SrcID, bool mute, bool temporary) 
{
	CInputLine * il;
	bool out=false;
	int i;
	INT_PTR nLines = m_ArrayInputLine.GetSize();

	for (i=0 ; i<nLines ; i++)
	{
		il = m_ArrayInputLine.GetAt(i);
		if (il->GetSrcID() == SrcID)
			out = il->SetMute(mute, temporary);
	};

	if (m_CurrentSrcID >= 0 )
	{
		for (i=0 ; i<nLines ; i++)
		{
			il = m_ArrayInputLine.GetAt(i);
			if ((unsigned long)m_CurrentSrcID == SrcID)
				continue;
			if (il->GetSrcID() == (unsigned long)m_CurrentSrcID)
				il->RestoreMute();
		};
	};

	return out;
}


/*
	Set the specified Input Line to be (un)muted
	Return former value
*/
bool CAudioInput::CMixerDevice::MuteSelectedInputLine(unsigned int line, bool mute, bool temporary)
{
	/* Test scope */
	if (((int)line) >= m_ArrayInputLine.GetSize() || ((int)line)<0)
		return false;

	/* Get the source line ID */
	CInputLine * il = m_ArrayInputLine.GetAt(line);
	unsigned long SrcID = il->GetSrcID();

	return MuteOutputLine(SrcID, mute, temporary);
}

/*
	Restore the specified Input Line (Mute + Volume Values)
*/
void CAudioInput::CMixerDevice::RestoreSelectedInputLine(unsigned int line)
{
	/* Test scope */
	if (((int)line) >= m_ArrayInputLine.GetSize() || ((int)line)<0)
		return ;
	
	/* Get the source line ID */
	CInputLine * il = m_ArrayInputLine.GetAt(line);
	if (il)
	{
		il->RestoreMute();
		il->RestoreVolume();
	};
}

/*
	Restore all elements of mixer device to their former (NOT initial) state
	* For every input line: Restore mute and volume. 
	* Find the line that was initially selected and select it
*/
int CAudioInput::CMixerDevice::Restore()
{
	ULONG OrigSelSrcID=-1;
	CInputLine * il;
	INT_PTR nLines = m_ArrayInputLine.GetSize();
	for (int i=0 ; i<nLines ; i++)
	{
		il = m_ArrayInputLine.GetAt(i);
		il->RestoreMute();
		il->RestoreVolume();
		if (il->isInitSelected())
			OrigSelSrcID = il->GetSrcID();
	};

	SelectInputLine(OrigSelSrcID);

	return 0;
}



/*
	Select the input line given by unique line ID
*/
void CAudioInput::CMixerDevice::SelectInputLine(unsigned int SrcID)
{
	MIXERLINE mxl;
	unsigned int mmres;
	MIXERCAPS mixCaps;
	int nDestLines;
	MIXERCONTROL ControlList[50]  = {0};
	MIXERCONTROLDETAILS_LISTTEXT mixList[50];
	MIXERCONTROLDETAILS_BOOLEAN mixBoolean[50];


	/* Get Mixer Device' capabilities */
	mmres = mixerGetDevCaps((INT_PTR)m_hMixerDevice, (LPMIXERCAPS) &mixCaps,	sizeof(MIXERCAPS));
	if(mmres != MMSYSERR_NOERROR) return;
	nDestLines = mixCaps.cDestinations;

	mxl.cbStruct = sizeof(MIXERLINE);
	
	/* Loop on all destinations of device DeviceId */
	for (int i=0 ; i<nDestLines ; i++)
	{
		/* Get Destination line info: ID, */
		mxl.dwDestination = i;
		mmres = mixerGetLineInfo((HMIXEROBJ)m_hMixerDevice, &mxl, MIXER_GETLINEINFOF_DESTINATION);
		if (mmres != MMSYSERR_NOERROR)
			break;

		/* Get list of destination line controls */
		GetLineControlList((HMIXEROBJ)m_hMixerDevice, mxl.dwLineID, ControlList);

		/* Loop on all destination line controls */
		for  (unsigned int iCtrl=0; iCtrl<mxl.cControls; iCtrl++)
		{
			/* For a given control in line Put result in mixList - a list of sub-control names and parameters */
			if (!ControlList[iCtrl].cMultipleItems)
				continue;

			/* only MUX and similar controls */
			if ((ControlList[iCtrl].dwControlType&MIXERCONTROL_CT_CLASS_MASK) != MIXERCONTROL_CT_CLASS_LIST)
				continue;

			/* Get list of sub-controls of the MUX control */
			mmres = GetControlDetailList(m_hMixerDevice, &mixList[0],   ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
			if(mmres != MMSYSERR_NOERROR)	continue ;

			/* Put result in mixBoolean - a list of boolean values (Selected) */
			mmres = GetControlDetailList(m_hMixerDevice, &mixBoolean[0],  ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
			if(mmres != MMSYSERR_NOERROR) return  ;

			/* Select */
			for (unsigned int i=0 ; i<ControlList[iCtrl].cMultipleItems ; i++)
			{
				if (mixList[i].dwParam1 == SrcID)
					mixBoolean[i].fValue = TRUE;
				else
					mixBoolean[i].fValue = FALSE;
			};
			SetControlDetailList(m_hMixerDevice, &mixBoolean[0],  ControlList[iCtrl].cMultipleItems, ControlList[iCtrl].dwControlID);
		}
		
	};

}


/*
	Set the sepecified Input Line selected
	Return true unles 'line' is out of scope
*/
bool CAudioInput::CMixerDevice::SetSelectedInputLine(unsigned int line)
{
	/* Test scope */
	if (((int)line) >= m_ArrayInputLine.GetSize() || ((int)line)<0)
		return false;

	/* Get the source line ID */
	CInputLine * il = m_ArrayInputLine.GetAt(line);
	unsigned long SrcID = il->GetSrcID();

	SelectInputLine(SrcID);
	AdjustInputVolume(SrcID);
	MuteOutputLine(SrcID);

	/* Save the current Source ID */
	m_CurrentSrcID = SrcID;

	return true;
}




/*
	For every physical output device (Speakers etc.) either
		Restore previous mute value (restore=true)
		Mute (restore=false, mute=true)
		Un-Mute  (restore=false, mute=false)

	Return the number of destination lines affected
*/
int CAudioInput::CMixerDevice::SetSpeakers(bool restore, bool mute)
{
	/* Test */
	if (!m_ArrayPhysicalDev.GetSize())
		return 0;

	/* for every pysical output device */
	INT_PTR count = m_ArrayPhysicalDev.GetSize();
	for (int i=0; i<count; i++)
	{
		/* Get the Physical Device object */
		CPhysicalDevice * phd = m_ArrayPhysicalDev.GetAt(i);
		if (!phd)
			continue;

		if (restore)
			phd->restore();		/* Restore previous value */
		else
		{
			phd->SetMute(mute);		/* (Un)Mute */
			phd->SetVolume(25);		/* and volume to 25% */
		};
	};

	return (int)count;
}


/***************************************************************************************
	Class CInputLine
	Hierarchy: CAudioInput::CMixerDevice::CInputLine

	Object represents an input audio line
	members consist of handle to parent mixer device, ID, initial volume, name, type
***************************************************************************************/
CAudioInput::CMixerDevice::CInputLine::CInputLine()
{
	CInputLine(0, 0, NULL, 0, false);
}

CAudioInput::CMixerDevice::CInputLine::~CInputLine()
{
	m_ArrayMuteControl->RemoveAll();
	delete(m_ArrayMuteControl);
}

CAudioInput::CMixerDevice::CInputLine::CInputLine(HMIXER hMixerDevice, unsigned long id, const char * Name, unsigned long Type, long Selected)
{
	/**** Insert basic data ****/
	m_hMixerDevice = hMixerDevice;	// Handle to mixed device
	m_ID = id;						// Source Line ID
	m_Name = strdup(Name);			// Source line Name
	m_SrcType = Type;				// Source Line Type
	m_InitVolume=-1;				// Initial value of recording volume
	if (Selected)					// Initially selected (Y/N)
		m_InitSelected = true;
	else
		m_InitSelected = false;

	/**** Calculate Initial input volume of this Input line ****/
	MIXERCONTROL ControlList[50]  = {0};
	long Volume;
	int cControls;
	MMRESULT mmres;
	
	/* Get the control list asociated with this source line */
	mmres = GetLineControlList((HMIXEROBJ)hMixerDevice, m_ID, ControlList, &cControls);
	/* If the list is valid then loop on all controls. 
	For every control (that is a volume control) get the normalized value */
	if (mmres == MMSYSERR_NOERROR)
		for (int i=0 ; i<cControls ; i++)
		{
			bool Valid = ::GetVolumeValue(m_hMixerDevice, ControlList[i].dwControlID, &Volume);
			if (Valid)
				m_InitVolume = Volume;
		};
		
	/* Create a list of relevant mute controls */
	m_ArrayMuteControl = CreateArrayMuteCtrl(m_SrcType, m_Name);

}

CArray<  CAudioInput::CMixerDevice::CInputLine::sMuteLine , CAudioInput::CMixerDevice::CInputLine::sMuteLine> * CAudioInput::CMixerDevice::CInputLine::CreateArrayMuteCtrl(ULONG SrcType, const char * Name)
{
	CArray< sMuteLine ,sMuteLine> * MuteArray;

	/* Initialization */
	MuteArray = new (CArray< sMuteLine ,sMuteLine>);
	MuteArray->RemoveAll();

	/* Get number of destination lines for this mixer device */
	MIXERCAPS mixCaps;
	MMRESULT mmres = mixerGetDevCaps((INT_PTR)m_hMixerDevice, (LPMIXERCAPS) &mixCaps,	sizeof(MIXERCAPS));
	if(mmres != MMSYSERR_NOERROR) return MuteArray;
	int nDestLines = mixCaps.cDestinations;

	/**** Loop on all destination lines that might need muting ****/
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
	
	for (int i=0 ; i<nDestLines ; i++)
	{
		mxl.dwDestination = i;
		mmres = mixerGetLineInfo((HMIXEROBJ)m_hMixerDevice, &mxl, MIXER_GETLINEINFOF_DESTINATION);
		if (mmres != MMSYSERR_NOERROR)
			break;
		/* Continue only if this is a speaker, headphone etc. */
		if (!isSpk(mxl.dwComponentType))
			continue;

		/* Get number of source lines for this destination line */
		int nSrcLines = mxl.cConnections;
		int pass=0;
		int iSrc=0;
		int found=0;
		/**** Loop on source lines ****/
		while (iSrc<nSrcLines && pass<3)
		{
			/* Get source line for this destinatio line */
			mxl.dwSource = iSrc;
			mmres = mixerGetLineInfo((HMIXEROBJ)m_hMixerDevice, &mxl, MIXER_GETLINEINFOF_SOURCE);
			if (mmres != MMSYSERR_NOERROR)
				break;

			/* Is it of the correct type */
			if (mxl.dwComponentType != SrcType)
			{
				iSrc++;
				continue;
			};
			/* Does it have a MUTE control */
			MIXERCONTROL ControlList[50]  = {0};
			GetLineControlList((HMIXEROBJ)m_hMixerDevice, mxl.dwLineID, ControlList);
			/* Loop on all source-line controls */
			for  (unsigned int iCtrl=0; iCtrl<mxl.cControls; iCtrl++)
			{
				/* Is this control is MUTE? */
				if (ControlList[iCtrl].dwControlType != MIXERCONTROL_CONTROLTYPE_MUTE)
					continue;
				
					/*	OK - this control is mute and the source line is of the correct type 
				But is it the correct line? - now is the time for some hueristics */
				if 
					(
					(pass == 0 && !strcmp(Name,mxl.szName)) || // Exact match
					(pass == 1 && (strstr(Name,mxl.szName) || strstr(mxl.szName,Name))) || // similar names
					(pass == 2) // Same type
					)
				{
					sMuteLine * ml = new (sMuteLine);
					ml->CtrlID = ControlList[iCtrl].dwControlID;
					ml->LineID = mxl.dwLineID;
					GetMuteValue(m_hMixerDevice, ControlList[iCtrl].dwControlID, &(ml->mute));
					MuteArray->Add(*ml);
					found++;
					iSrc++; // next
					break;
					
				};
			}; // Controls
			iSrc++; // next
			// Re-start search for a higher pass level
			if (iSrc == nSrcLines)
			{
				if (found)
					break;
				iSrc = 0;
				found=0;
				pass++;
			};
		}; // Source
	}; // Destination


	return MuteArray;
}

const char * CAudioInput::CMixerDevice::CInputLine::GetName()
{
	return m_Name;
}

unsigned long CAudioInput::CMixerDevice::CInputLine::GetSrcID()
{
	return m_ID;
}

bool CAudioInput::CMixerDevice::CInputLine::isInitSelected()
{
	return m_InitSelected;
}

/*
	Reset Mute values of all corresponding lines
*/
void CAudioInput::CMixerDevice::CInputLine::RestoreMute()
{	sMuteLine sm;
	INT_PTR nMuteCtrls = m_ArrayMuteControl->GetSize();

	/* Go over all related mutes controls */
	for (int i=0 ; i<nMuteCtrls ; i++)
	{
		sm = m_ArrayMuteControl->GetAt(i);	// Get atruct that holds the Control ID & original mute value
		Mute(m_hMixerDevice, sm.CtrlID, sm.mute); // Restore original mute value
	};
}

int CAudioInput::CMixerDevice::CInputLine::RestoreVolume()
{
	MIXERCONTROL ControlList[50]  = {0};
	int cControls;
	MMRESULT mmres;
	int CurrentVolume=-1;
	
	/* Get the control list asociated with this source line */
	mmres = GetLineControlList((HMIXEROBJ)m_hMixerDevice, m_ID, ControlList, &cControls);
	/* If the list is valid then loop on all controls. 
	For every control (that is a volume control) get the normalized value */
	if (mmres != MMSYSERR_NOERROR)
		return -1;
	
	/* Loop on all comtrols - put the initial volume value in the volume control */
	for (int i=0 ; i<cControls ; i++)
		CurrentVolume = ::SetVolumeValue(m_hMixerDevice, ControlList[i].dwControlID, m_InitVolume);

	return CurrentVolume;
}

void CAudioInput::CMixerDevice::CInputLine::SetDefaultVolume()
{

	int value = ::GetDefaultVolumeValue(m_SrcType);


	MIXERCONTROL ControlList[50]  = {0};
	int cControls;
	MMRESULT mmres;
	int OldVolume;
	
	/* Get the control list asociated with this source line */
	mmres = GetLineControlList((HMIXEROBJ)m_hMixerDevice, m_ID, ControlList, &cControls);
	/* If the list is valid then loop on all controls. 
	For every control (that is a volume control) get the normalized value */
	if (mmres != MMSYSERR_NOERROR)
		return;
	
	/* Loop on all comtrols - put the initial volume value in the volume control */
	for (int i=0 ; i<cControls ; i++)
	{
		OldVolume = ::SetVolumeValue(m_hMixerDevice, ControlList[i].dwControlID, value);
		if (OldVolume>=0)
			m_InitVolume = OldVolume;
	}
}


/*
	Mute all corresponding lines
*/
bool CAudioInput::CMixerDevice::CInputLine::SetMute(bool mute, bool temporary)
{
	sMuteLine sm;
	bool out;
	INT_PTR nMuteCtrls = m_ArrayMuteControl->GetSize();
	
	/* Go over all related mutes controls */
	for (int i=0 ; i<nMuteCtrls ; i++)
	{
		sm = m_ArrayMuteControl->GetAt(i);	// Get atruct that holds the Control ID
		GetMuteValue(m_hMixerDevice, sm.CtrlID, &out);
		Mute(m_hMixerDevice, sm.CtrlID, mute);	// Mute
		
		if (!temporary)
		{
			// Save previous value
			sm.mute = out;
			m_ArrayMuteControl->SetAt(i,sm);
		};
	};

	return out;
}


void CAudioInput::CMixerDevice::CInputLine::SetName(const char *name)
{
	m_Name = strdup(name);
}

void CAudioInput::CMixerDevice::CInputLine::SetSrcType(unsigned long type)
{
	m_SrcType = type;
}


int CAudioInput::CMixerDevice::CInputLine::StoreDefaultVolume()
{

	MIXERCONTROL ControlList[50]  = {0};
	int cControls;
	MMRESULT mmres;
	long CurrentVolume=-1;
	
	/* Get the control list asociated with this source line */
	mmres = GetLineControlList((HMIXEROBJ)m_hMixerDevice, m_ID, ControlList, &cControls);
	/* If the list is valid then loop on all controls. 
	For every control (that is a volume control) get the normalized value */
	if (mmres != MMSYSERR_NOERROR)
		return -1;
	
	/* Loop on all comtrols - put the initial volume value in the volume control */
	for (int i=0 ; i<cControls ; i++)
	{
		 if (::GetVolumeValue(m_hMixerDevice, ControlList[i].dwControlID, &CurrentVolume))
			 ::SetDefaultVolumeValue(m_SrcType, CurrentVolume);
	}

	return CurrentVolume;
	
}


/***************************************************************************************
	Class CPhysicalDevice
	Hierarchy: CAudioInput::CMixerDevice::CPhysicalDevice

	Object represents an output device such as spks or earphones
	members consist of ID, volume (ID and value), Mute (ID and value)
***************************************************************************************/
CAudioInput::CMixerDevice::CPhysicalDevice::CPhysicalDevice()
{
	CPhysicalDevice(NULL,0xFFFF, 0xFFFF, false, 0xFFFF,-1);
}

CAudioInput::CMixerDevice::CPhysicalDevice::CPhysicalDevice(HMIXER hMixerDevice, unsigned long DstLineID, unsigned long muteCtrlID,bool mute, unsigned long volumeCtrlID, int volume)
{
	m_hMixerDevice = hMixerDevice;
	m_DstLineID = DstLineID;
	m_muteCtrlID = muteCtrlID;
	m_volumeCtrlID = volumeCtrlID;
	m_mute = mute;
	m_volume = volume;	
}


CAudioInput::CMixerDevice::CPhysicalDevice::~CPhysicalDevice()
{
}

/* Restore the original values of the Physical output (Mute & Volume) */
void CAudioInput::CMixerDevice::CPhysicalDevice::restore()
{
	bool mute = m_mute;
	int volume = m_volume;

	Mute(m_hMixerDevice, this->m_muteCtrlID, mute);	// Mute
	SetVolumeValue(m_hMixerDevice, this->m_volumeCtrlID, volume);

}

/* (Un)Mute this physical device */
void CAudioInput::CMixerDevice::CPhysicalDevice::SetMute(bool mute)
{
	Mute(m_hMixerDevice, this->m_muteCtrlID, mute);	// Mute
}

/* Set volume for this physical device */
void CAudioInput::CMixerDevice::CPhysicalDevice::SetVolume(int volume)
{
	SetVolumeValue(m_hMixerDevice, this->m_volumeCtrlID, volume);
}

