#pragma once


class CPpjoyIf
{
public:
	CPpjoyIf(void);
	bool Start(int iDevice, DWORD *error);
	bool Stop(DWORD *error);
	bool Test(DWORD *error);
	bool Send(int * Positions, int nPositions, DWORD *error);
	virtual ~CPpjoyIf(void);
	int		GetDevId(void);

private:
	HANDLE	m_hJoy;
	int		m_iVirtualJoystick;
	JOYSTICK_STATE		m_JoyState;
	bool	m_Started;
};
