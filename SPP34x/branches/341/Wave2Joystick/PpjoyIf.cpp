#include "StdAfx.h"
#include "PpjoyEx.h"
#include ".\ppjoyif.h"

// Virtual Joystick data
char * Joystick_Ioctl[17] = PPJoyIOCTL;


CPpjoyIf::CPpjoyIf(void)
{
	m_iVirtualJoystick = -1;
	m_hJoy = INVALID_HANDLE_VALUE;
	m_Started = false;
}


bool CPpjoyIf::Start(int iDevice, DWORD *error)
{
	int i;
	
	m_iVirtualJoystick = iDevice;

	/* Create a file name for the selected virtual joystick */
	if (iDevice<1 || iDevice>16)
		iDevice = 1; /* Default joystick number is 1 */
	char * DevName= Joystick_Ioctl[iDevice];

	/* Open a handle to the control device for the virtual joystick. */
	/* Virtual joystick devices are names PPJoyIOCTL1 to PPJoyIOCTL16. */
	m_hJoy= CreateFile(DevName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
	
	if (m_hJoy==INVALID_HANDLE_VALUE)
	{
		*error = GetLastError();
		return false;
	}
	
	/* Initialise the IOCTL data structure */
	m_JoyState.Signature= JOYSTICK_STATE_V1;
	m_JoyState.NumAnalog= CHANNELS;	/* Number of analog values */
	m_JoyState.NumDigital= CHANNELS;	/* Number of digital values */ /* Version 3.4.0 - add buttons */
	for (i=0 ; i<m_JoyState.NumAnalog; i++)
		m_JoyState.Analog[i] = (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;
	for (i=0 ; i<m_JoyState.NumDigital; i++)/* Version 3.4.0 - add buttons */
		m_JoyState.Digital[i] = 1;

	m_Started = true;
	return true;
}

bool CPpjoyIf::Stop(DWORD *error)
{
	m_Started = false;
	if (m_hJoy && CloseHandle(m_hJoy))
	{
		m_hJoy = NULL;
		return true;
	}
	else
	{
		*error = GetLastError();
		return false;
	};
}

bool CPpjoyIf::Send(int * Positions, int nPositions, DWORD *error)
{
	DWORD				RetSize;


	if (!m_Started)
		return true;

	if (m_hJoy == INVALID_HANDLE_VALUE)
	{
		bool started = Start(m_iVirtualJoystick, error);
		if (!started)
		return false;
	};

	/* Test value of number of channels */
	if (nPositions>CHANNELS)
		nPositions=CHANNELS;
	if (nPositions<0)
		return false;

	/* Fill-in the PPJ structure */
	m_JoyState.NumAnalog= CHANNELS;	/* Number of analog values */
	m_JoyState.NumDigital= CHANNELS;	/* Number of analog values */
	for (int i=0; i<nPositions; i++)
	{
		m_JoyState.Analog[i] = PPJOY_AXIS_MIN+(Positions[i]<<5);
		m_JoyState.Digital[i] = m_JoyState.Analog[i] < ((PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)*0.75); /* Version 3.4.0 - add buttons */
	};


	/* Send data to PPJoy */
	if (!DeviceIoControl(m_hJoy,IOCTL_PPORTJOY_SET_STATE,&m_JoyState,sizeof(m_JoyState),NULL,0,&RetSize,NULL))
	{
		*error = GetLastError();
		return false;
	}

	return true;
}

int		CPpjoyIf::GetDevId(void)
{
	return m_iVirtualJoystick;
}

CPpjoyIf::~CPpjoyIf(void)
{
	CloseHandle(m_hJoy);
}

