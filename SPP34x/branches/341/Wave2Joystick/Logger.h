#pragma once

class CLogger
{
public:
	CLogger(void);
	bool	CreateWaveFile(TCHAR * FileName, WAVEFORMATEX  * pWaveFormat);
	bool	StartReadWaveFile(TCHAR * FileName, WAVEFORMATEX  * pWaveFormat, int MiliSeconds=25);
	LONG	PutWaveData(void * pData, LONG size);
	LONG	GetWaveData(void * pData);
	void	CloseWaveFile(void);
	virtual ~CLogger(void);

private:
	HMMIO		m_hWaveFile;
	MMCKINFO	m_DataChunkInfo, m_ChunkInfo;
	int			m_BufferSize;
};
