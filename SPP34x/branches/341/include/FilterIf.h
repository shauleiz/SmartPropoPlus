#pragma once

#define JSCHPOSTPROC_DLL	".\\JsChPostProc.dll"
struct JS_STRUCT	// Joystick channel data
{
	int ch;			// Number of channels
	int * value;	// Pointer to data buffer ; ch>n>=0 ; value[n] holds channel (n+1) 
};

class CFilterIf
{
public:
	CFilterIf(void);
	~CFilterIf(void);
protected:
	int m_nFilters;		// Number of filters
	long m_verDll;		// Filter-DLL version
	HINSTANCE m_hDll;	// Handle to the loaded Filter-DLL file
	bool m_Active;		// True if a filter is seleted
public:
	bool Init(void);
	int GetNumberOfFilters(void);
	int GetIndexOfSelected(void);
	TCHAR * GetFilterName(int index);
	int SetSelected(const int index);
	int * Filter(int * Positions, int * nPositions, int min, int max);

protected:
	void * Run(void *, int max , int min);
};
