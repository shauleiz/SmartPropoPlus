#pragma once

class CFmsIf
{
public:
	CFmsIf(void);
	bool InitInstance();
	~CFmsIf(void);
protected:
	// Handle to the file mapping object
	HANDLE m_hFileMapping;
	Spp2FmsStruct * m_Spp2FmsBlock;
	HANDLE m_hMutex;
public:
	// Place joystick data - number of positions and position values - on the memory block shared with winmm.dll
	bool SetJoystickPos(int n, int * Pos);
	bool SetVersionStr(const char * VersionStr);
};
