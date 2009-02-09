#include "StdAfx.h"
#include <TCHAR.H>
#include "JsChPostProc.h"
#include "SmartPropoPlus.h"
#include "JsChPostProc.h"
#include "SppRegistry.h"
#include "filterif.h"

CFilterIf::CFilterIf(void)
: m_nFilters(0), m_verDll(0), m_hDll(NULL), m_Active(false)
{


}

CFilterIf::~CFilterIf(void)
{
}

bool CFilterIf::Init(void)
{
	/* Load DLL */
	m_hDll = LoadLibraryEx(JSCHPOSTPROC_DLL, NULL, 0);
	if (!m_hDll)
		return false;

	// Load Get local GetDLLVersion for this DLL
	LONG (*pGetDllVersion)(void) = (long(*)())GetProcAddress(m_hDll, "GetDllVersion");
	if (!pGetDllVersion)
		return false;

	// Continue only if version is above 3.1.0
	m_verDll = pGetDllVersion();
	if (m_verDll < 0x30100)
		return false;

	// Get the number of filters
	int  (WINAPI *pGetNumberOfFilters)(void);
	pGetNumberOfFilters = (int  (WINAPI *)(void))GetProcAddress(m_hDll,"GetNumberOfFilters");
	if (!pGetNumberOfFilters)
		return false;
	m_nFilters =  pGetNumberOfFilters();
	return true;
}

int CFilterIf::GetNumberOfFilters(void)
{
	return m_nFilters;
}

int CFilterIf::GetIndexOfSelected(void)
{
	// Get the name of the selected filter from the registry
	TCHAR * name = GetSelectedFilterNameFromRegistry();
	if (!name || !strlen(name))
		return -1;

	// Get the index of the selected filter by looping on all filter names
	TCHAR * FilterName;
	TCHAR *  (WINAPI *pGetFilterNameByIndex)(const int);
	pGetFilterNameByIndex = (char *  (WINAPI *)(const int))GetProcAddress(m_hDll,"GetFilterNameByIndex");
	if (!pGetFilterNameByIndex)
		return -1;
	for (int i=0; i<m_nFilters; i++)
	{
		FilterName = pGetFilterNameByIndex(i);
		if (!_tcscmp(FilterName, name))
		{
			if (name) free(name);
			return i;
		};
	};
	if (name) free(name);
	return -1;
}

TCHAR * CFilterIf::GetFilterName(int index)
{
	TCHAR *  (WINAPI *pGetFilterNameByIndex)(const int);
	pGetFilterNameByIndex = (char *  (WINAPI *)(const int))GetProcAddress(m_hDll,"GetFilterNameByIndex");
	if (!pGetFilterNameByIndex)
		return '\0';

	return  pGetFilterNameByIndex(index);
}

int CFilterIf::SetSelected(const int index)
{
	const int  (WINAPI *pSelectFilterByIndex)(const int);
	pSelectFilterByIndex = (const int  (WINAPI *)(const int))GetProcAddress(m_hDll,"SelectFilterByIndex");
	if (!pSelectFilterByIndex)
		return -1;
	if (index < 0)
		m_Active = false;
	else
		m_Active = true;

	return pSelectFilterByIndex(index);
}

void * CFilterIf::Run(void * pIn, int max, int min)
{
	PJS_CHANNELS   (WINAPI *pProcessChannels)(PJS_CHANNELS, int max, int min);
	pProcessChannels = (PJS_CHANNELS   (WINAPI *)(PJS_CHANNELS, int max, int min))GetProcAddress(m_hDll,"ProcessChannels");
	if (!pProcessChannels)
		return NULL;

	return (void *)pProcessChannels((PJS_CHANNELS)pIn, max, min);
}

int * CFilterIf::Filter(int * Positions, int * nPositions, int max, int min)
{
	JS_CHANNELS in, *out;

	if (!m_Active)
		return Positions;

	// Run filter on input data
	in.value = Positions;
	in.ch = *nPositions;
	out = (JS_CHANNELS *)Run(&in, max,min);
	
	// Copy data
	*nPositions = out->ch;
	return out->value;
}