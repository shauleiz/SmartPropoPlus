#pragma once
#include "waverec.h"

class CLogAudioHdrs :
	public CWaveRec
{
public:
	CLogAudioHdrs(void);
	virtual ~CLogAudioHdrs(void);
protected:
	virtual void UpdateControls(void);
	virtual void PostMessageStop();
	virtual void PostMessageStart();
	virtual void PostMessageCancel();

	// Encasulation functions
	virtual BOOL SppLoggerStop(void);
	virtual BOOL SppLoggerStart(TCHAR * FileName);
	virtual BOOL SppLoggerStat(int * stat);
};
