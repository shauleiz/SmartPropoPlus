#include "StdAfx.h"
#include <sys/timeb.h>
#include <time.h>
#include "SmartPropoPlus.h"
#include "fmsif.h"
#include ".\fmsif.h"

static UINT NEAR WM_INTERSPPAPPS = RegisterWindowMessage(INTERSPPAPPS);

CFmsIf::CFmsIf(void)
{
	m_hFileMapping = NULL;	// Initialize the handle to the File Mapping object
	m_Spp2FmsBlock = NULL;	// Initialize the pointer to the SPP to FMS shared data structure
	m_hMutex		= NULL;	// Initialize the handle to the semaphor that gards the SPP to FMS shared data structure
}
bool CFmsIf::InitInstance()
{
	if (m_hFileMapping)
		return true;

	m_hMutex = CreateMutex(NULL, TRUE, SHARED_FILE_MUTEX);

	///// Memory mapped file from where winmm.dll will read joystick and other data
	m_hFileMapping = CreateFileMapping
		(
		INVALID_HANDLE_VALUE,// File handle
		NULL,                // Security attributes
		PAGE_READWRITE,      // Protection
		0,                   // Size - high 32 bits
		1<<16 ,               // Size - low 32 bits
		SHARED_FILE_NAME); // Name
	
	// File mapping should not fail - even if this file already exists
	if (!m_hFileMapping)
	{
		ReleaseMutex(m_hMutex);
		return false;
	};

	// Check if already exists - if exists, just return
    DWORD dwMapErr = GetLastError();
	if (dwMapErr == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(m_hMutex);
		return true;
	};

	// If did not exist, then map and initialize this memory file
	m_Spp2FmsBlock = (Spp2FmsStruct *)MapViewOfFile
		(
		m_hFileMapping,			// File mapping object
		FILE_MAP_ALL_ACCESS,	// Read/Write
		0,						// Offset - high 32 bits
		0,						// Offset - low 32 bits
		0);						// Map the whole thing
	if (!m_Spp2FmsBlock)
	{
		ReleaseMutex(m_hMutex);
		return false;
	}

	TCHAR ProdName[MAX_VERSTR];
	sprintf(ProdName,SPP_PROD_NAME,(VER_DLL&0xFF0000)>>16, (VER_DLL&0x00FF00)>>8, (VER_DLL&0x0000FF) );
	SetVersionStr(ProdName);
	m_Spp2FmsBlock->nChannels = 0;
	m_Spp2FmsBlock->Identifier = 0;
	for (int i=0; i<MAX_JS_CH; i++)
		m_Spp2FmsBlock->Channel[i] = i*10;

	ReleaseMutex(m_hMutex);
	return true;
}

CFmsIf::~CFmsIf(void)
{
	BOOL bFlag;

	bFlag = UnmapViewOfFile(m_Spp2FmsBlock);
	bFlag = CloseHandle(m_hFileMapping);
	bFlag = CloseHandle(m_hMutex);
}

// Place joystick data - number of positions and position values - on the memory block shared with winmm.dll
bool CFmsIf::SetJoystickPos(int n, int * Pos)
{

	// Sanity check
	if (n<0 || n>MAX_JS_CH || !Pos || !m_Spp2FmsBlock)
		return false;

	// Wait for this memory block to be released. If timeout (0.5 Sec) comes first then just exit
	DWORD Reason = WaitForSingleObject(m_hMutex, 50);
	if (Reason != WAIT_OBJECT_0)
		return false;

	// Set data
	m_Spp2FmsBlock->nChannels = n;
	for (int i=0; i<n; i++)
		m_Spp2FmsBlock->Channel[i] = Pos[i];
	for (int i=n; i<MAX_JS_CH; i++)
		m_Spp2FmsBlock->Channel[i] = 0;

	// Set timestamp for latency & responsiveness calculaions
	struct _timeb timebuffer;
	_ftime( &timebuffer );
	m_Spp2FmsBlock->Identifier = (UINT)((timebuffer.time)&0xFFF)*1000+timebuffer.millitm; 
	BOOL Posted = PostMessage(HWND_BROADCAST, WM_INTERSPPAPPS, MSG_TIMESTAMP2DLL, m_Spp2FmsBlock->Identifier);
	// Release mutex and exit
	ReleaseMutex(m_hMutex);

	return true;
}

bool CFmsIf::SetVersionStr(const char * VersionStr)
{
	// Sanity check
	if (!VersionStr || !m_Spp2FmsBlock)
		return false;

	// Wait for this memory block to be released. If timeout (0.5 Sec) comes first then just exit
	DWORD Reason = WaitForSingleObject(m_hMutex, 500);
	if (Reason != WAIT_OBJECT_0)
		return false;

	// Set data
	strncpy(m_Spp2FmsBlock->VersionStr, VersionStr, MAX_VERSTR-1);

	// Release mutex and exit
	ReleaseMutex(m_hMutex);
	return true;
}