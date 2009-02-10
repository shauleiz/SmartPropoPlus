/********************************************************************************************************

Wrapper to a logger 

WAVE file in RIFF format
1. Create a new output file with CreateWaveFile() - Parameters are file name and a pointer to a WAVEFORMATEX structure
2. Repeated calls of PutWaveData() to add raw PCM data to the file (It is assumed that CreateWaveFile has been successful)
3. Finally call CloseWaveFile() - (It is assumed that CreateWaveFile has been successful)
4. Optionally - call RenameWaveFile()

********************************************************************************************************/
#include "StdAfx.h"
#include "mmsystem.h"
#include ".\logger.h"

CLogger::CLogger(void)
{
	m_BufferSize = 0;
}

CLogger::~CLogger(void)
{
	CloseWaveFile();
}

bool CLogger::CreateWaveFile(TCHAR * FileName, WAVEFORMATEX  * pWaveFormat)
{
	MMCKINFO FormatChunkInfo;
	DWORD Res;

	/* Create Wave file */
	m_hWaveFile = mmioOpen(FileName, NULL, MMIO_CREATE | MMIO_WRITE | MMIO_READ | MMIO_ALLOCBUF);
	if (!m_hWaveFile)
		return false;

	// Create RIFF chunk. First zero out m_ChunkInfo structure.
	memset(&m_ChunkInfo, 0, sizeof(MMCKINFO));
	m_ChunkInfo.fccType = mmioStringToFOURCC("WAVE", 0);
	Res = mmioCreateChunk(m_hWaveFile, &m_ChunkInfo, MMIO_CREATERIFF);
	if (Res)
	{
		CloseWaveFile();
		return false;
	};

	// Create the format chunk.
	FormatChunkInfo.ckid = mmioStringToFOURCC("fmt ", 0);
	FormatChunkInfo.cksize = sizeof(WAVEFORMATEX);
	Res = mmioCreateChunk(m_hWaveFile, &FormatChunkInfo, 0);
	if (Res)
	{
		CloseWaveFile();
		return false;
	};
	// Write the wave format data.
	mmioWrite(m_hWaveFile, (char*)pWaveFormat, sizeof(WAVEFORMATEX));
	Res = mmioAscend(m_hWaveFile, &FormatChunkInfo, 0);
	if (Res)
	{
		CloseWaveFile();
		return false;
	};

	// Create the data chunk.
	m_DataChunkInfo.ckid = mmioStringToFOURCC("data", 0);
	m_DataChunkInfo.cksize = 0; // Data size is 0 for now
	Res = mmioCreateChunk(m_hWaveFile, &m_DataChunkInfo, 0);
	if (Res)
	{
		CloseWaveFile();
		return false;
	};

	return true;
}


LONG CLogger::PutWaveData(void * pData, LONG size)
{
	return mmioWrite(m_hWaveFile, (char *)pData , size);
}

bool CLogger::StartReadWaveFile(TCHAR * FileName, WAVEFORMATEX  *pWaveFormat, int MiliSeconds)
{
	MMRESULT Res;
	MMCKINFO FormatChunkInfo;
	LONG read;


	///// Open the input wave file for reading - verify validity
	m_hWaveFile = mmioOpen(FileName, NULL, MMIO_READ | MMIO_ALLOCBUF);
	if (!m_hWaveFile)
		return false;

	///// Calculate buffer size for each GET operartion. 
	// Get the top chunk
	m_ChunkInfo.fccType = mmioStringToFOURCC("WAVE", 0);
	Res = mmioDescend(m_hWaveFile, &m_ChunkInfo, NULL, MMIO_FINDRIFF);
	if (Res != MMSYSERR_NOERROR)
	{
		CloseWaveFile();
		return false;
	};

	// Go to the format chunk to read WAVEFORMATEX
	FormatChunkInfo.ckid = mmioStringToFOURCC("fmt ", 0);
	FormatChunkInfo.dwDataOffset = 0;
	Res = mmioDescend(m_hWaveFile, &FormatChunkInfo, &m_ChunkInfo, MMIO_FINDCHUNK);
	if (Res != MMSYSERR_NOERROR)
	{
		CloseWaveFile();
		return false;
	};
	unsigned long Size = FormatChunkInfo.cksize;
	read = mmioRead(m_hWaveFile, (char *)(pWaveFormat), Size);
	if (read != Size)
	{
		CloseWaveFile();
		return false;
	};

	// Calculate
	m_BufferSize = pWaveFormat->nAvgBytesPerSec*MiliSeconds/1000;

	// Reset file for reading
	m_DataChunkInfo.ckid = mmioStringToFOURCC("data", 0);
	m_DataChunkInfo.dwDataOffset = 0;
	Res = mmioDescend(m_hWaveFile, &m_DataChunkInfo, &m_ChunkInfo, MMIO_FINDCHUNK);
	if (Res != MMSYSERR_NOERROR)
	{
		CloseWaveFile();
		return false;
	};

	return true;
}

LONG CLogger::GetWaveData(void * pData)
{
	return mmioRead(m_hWaveFile, (HPSTR)pData , m_BufferSize);
}

void CLogger::CloseWaveFile(void)
{

	DWORD Res;

	if (!m_hWaveFile)
	{
		memset(&m_ChunkInfo, 0, sizeof(MMCKINFO));
		memset(&m_DataChunkInfo, 0, sizeof(MMCKINFO));
		return;
	}

	// Get out of the data chunk so that the data size will be updated
	Res = mmioAscend(m_hWaveFile, &m_DataChunkInfo, 0);

	// this will result in a file that is unreadable by Windows95 Sound Recorder.
	Res = mmioAscend(m_hWaveFile, &m_ChunkInfo, 0);
	mmioClose(m_hWaveFile, 0);

	// Reset values
	m_hWaveFile=NULL;
	memset(&m_ChunkInfo, 0, sizeof(MMCKINFO));
	memset(&m_DataChunkInfo, 0, sizeof(MMCKINFO));

}
