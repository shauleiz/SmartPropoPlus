#include "stdafx.h"
#include "SmartPropoPlus.h"
#include "SppDbg.h"
#include <errno.h>
#include <assert.h>

SppDbg::SppDbg(void) :
	m_FileDbgInSig(NULL)
{
}


SppDbg::~SppDbg(void)
{
	StopDbgInputSignal();
	StopDbgPulse();
}

void SppDbg::StartDbgInputSignal(void)
{
	TCHAR lpTempPathBuffer[MAX_PATH];

	DWORD dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
                           lpTempPathBuffer); // buffer for path 

	    //  Generates a temporary file name. 
    UINT uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
                              TEXT("SIG"),     // temp file name prefix 
                              0,                // create unique name 
                              m_FileDbgInSigName);  // buffer for name 

	//memcpy_s(m_FileDbgInSigName, MAX_PATH,  L"SPPDBGINSIG.TXT", sizeof( L"SPPDBGINSIG.TXT"));
	if (!m_FileDbgInSig)
		_wfopen_s(&m_FileDbgInSig,m_FileDbgInSigName, L"w+"); 

		fprintf(m_FileDbgInSig,  "Start Logging raw input signal");

}

void SppDbg::StartDbgPulse(void)
{
	TCHAR lpTempPathBuffer[MAX_PATH];

	DWORD dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
                           lpTempPathBuffer); // buffer for path 

	    //  Generates a temporary file name. 
    UINT uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
                              TEXT("PULSE"),     // temp file name prefix 
                              0,                // create unique name 
                              m_FileDbgPulseName);  // buffer for name 

	//memcpy_s(m_FileDbgPulseName, MAX_PATH,  L"C:/SPPDBGPULSE.TXT", sizeof( L"C:/SPPDBGPULSE.TXT"));
	if (!m_FileDbgPulse)
		_wfopen_s(&m_FileDbgPulse,m_FileDbgPulseName, L"w+"); 
}

void SppDbg::StopDbgInputSignal(void)
{
	if (!m_FileDbgInSig)
		return;

	OPENFILENAME ofn;
	fclose(m_FileDbgInSig);
	m_FileDbgInSig = NULL;

	// Get destination file
	WCHAR szFileName[MAX_PATH] = L"";
	memcpy_s(szFileName, MAX_PATH, L"SPPDBGINSIG.TXT", sizeof(L"SPPDBGINSIG.TXT"));
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME); 
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = (LPCWSTR)L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
    ofn.lpstrFile = (LPWSTR)szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = (LPCWSTR)L"txt";
	GetSaveFileName(&ofn);

	// Move tempfile to selected destination
	BOOL moved = MoveFileEx(m_FileDbgInSigName, ofn.lpstrFile, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
}
void SppDbg::StopDbgPulse(void)
{
	if (!m_FileDbgPulse)
		return;

	OPENFILENAME ofn;
	int closed = fclose(m_FileDbgPulse);
	m_FileDbgPulse=NULL;

	// Get destination file
	WCHAR szFileName[MAX_PATH] = {L""};
	memcpy_s(szFileName, MAX_PATH, L"SPPDBGPULSE.TXT", sizeof(L"SPPDBGPULSE.TXT"));
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME); 
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = (LPCWSTR)L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
    ofn.lpstrFile = (LPWSTR)szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = (LPCWSTR)L"txt";
	assert(GetSaveFileName(&ofn));

	// Move tempfile to selected destination
	BOOL moved = MoveFileEx(m_FileDbgPulseName, ofn.lpstrFile, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
	DWORD err = GetLastError();
}

// Data packet arrives with raw input signal (Digital Audio)
// Get the data and write it into a file
void SppDbg::InputSignalReady(PBYTE buffer, PVOID info)
{
	char sep;

	struct InputSignalStruct 
	{
		BYTE sStruct; // Size of structure in bytes
		UINT bSize;  // Number of frames
		WORD nChannels;
		WORD wBitsPerSample;
		DWORD dwCount;
	} * sInputSig;

	if (!m_FileDbgInSig)
		return;

	sInputSig = (InputSignalStruct *)info;

	// Print packet header
	fprintf(m_FileDbgInSig,  "\n>>> Packet %d, nCh %d, Bits %d, Frames %d\n",  sInputSig->dwCount, sInputSig->nChannels, sInputSig->wBitsPerSample,sInputSig->bSize);

	// Print 32 frames per line.
	// If 8-bit samples then print two-digit Hex
	// If 16-bit samples then print 5 + sign digits signed drcimal
	// If stereo, print '/' between Left/Right

	// Channel seperator
	if (sInputSig->nChannels == 2)
		sep = '/';
	else
		sep = ',';

	int line=0;
	int row=0;

	if (sInputSig->wBitsPerSample == 8)
		// Eight Bits
			for (UINT32 i=0; i<sInputSig->bSize*sInputSig->nChannels;i+=sInputSig->nChannels)
			{
				if (!row)
					fprintf(m_FileDbgInSig,  ">> %3d,  ", line++);
				row++;
				fprintf(m_FileDbgInSig,  "%02x%c%02x",(((unsigned char *)buffer)[i]), sep, (((unsigned char *)buffer)[i+1]));
				if (row == 32)
				{
					fprintf(m_FileDbgInSig,"\n");
					row=0;
				}
				else
					fprintf(m_FileDbgInSig,", ");
			}
	else
		//16 Bits
		for (UINT32 i=0; i<sInputSig->bSize*sInputSig->nChannels;i+=sInputSig->nChannels)
		{
			if (!row)
				fprintf(m_FileDbgInSig,  ">> %3d,  ", line++);
			row++;
			fprintf(m_FileDbgInSig,  "%06d%c%06d",(((signed short *)buffer)[i]), sep, (((signed short *)buffer)[i+1]));
			if (row == 32)
			{
				//fprintf(m_FileDbgInSig,"\n");
				row=0;
			}
			else
				fprintf(m_FileDbgInSig,", ");
		}
}

// Data packet arrives with Pulse data (Raw & Normalized) along with an array of samples
// Get the data and write it into a file
void SppDbg::PulseReady(PVOID buffer, UINT ArraySize)
{
	if (!m_FileDbgPulse || !buffer)
		return;

	DbgPulseInfo * sPulseInfo = (DbgPulseInfo *)buffer;

	// Print Pulse data
	fprintf(m_FileDbgPulse,  "\n>>> Pulse length (Raw/Normalized): %d/%d, ", sPulseInfo->RawPulse,  sPulseInfo->NormPulse);
	if (sPulseInfo->negative)
		fprintf(m_FileDbgPulse,  "Low\n");
	else
		fprintf(m_FileDbgPulse,  "High\n");

	// Print samples
	int line=0;
	int row=0;
	USHORT  * Samples = (USHORT  *)sPulseInfo->Samples;

	for (UINT i=0; i<ArraySize; i++)
	{
		if (!row)
			fprintf(m_FileDbgPulse,  ">> %3d,  ", line++);
		row++;

		fprintf(m_FileDbgPulse,  "%06d", Samples[i]);
		if (row == 16)
		{
			fprintf(m_FileDbgPulse,"\n");
			row=0;
		}
		else
			fprintf(m_FileDbgPulse,", ");

	}



}