#include "stdafx.h"
#include "SppDbg.h"


SppDbg::SppDbg(void) :
	m_FileDbgInSig(NULL)
{
}


SppDbg::~SppDbg(void)
{
}

void SppDbg::StartDbgInputSignal(void)
{

	memcpy_s(m_FileDbgInSigName, FILENAME_MAX,  "SPPDBGINSIG.TXT", FILENAME_MAX);// TODO: Make the file name configurable
	if (!m_FileDbgInSig)
		fopen_s(&m_FileDbgInSig,m_FileDbgInSigName, "w+"); 
}

void SppDbg::StopDbgInputSignal(void)
{
	fclose(m_FileDbgInSig);
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
	fprintf(m_FileDbgInSig,  ">>> Packet %d, nCh %d, Bits %d, Frames %d\n",  sInputSig->dwCount, sInputSig->nChannels, sInputSig->wBitsPerSample,sInputSig->bSize);

	// Print 32 frames per line.
	// If 8-bit samples then print two-digit Hex
	// If 16-bit samples then print 5 + sign digits signed drcimal
	// If stereo, print '/' between Left/Right

	// Channel seperator
	if (sInputSig->nChannels == 2)
		sep = '/';
	else
		sep = ' ';

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
			fprintf(m_FileDbgInSig,  "%04x%c%04x",(((signed short *)buffer)[i]), sep, (((signed short *)buffer)[i+1]));
			if (row == 32)
			{
				fprintf(m_FileDbgInSig,"\n");
				row=0;
			}
			else
				fprintf(m_FileDbgInSig,", ");
		}
}

