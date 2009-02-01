#pragma once
#include "waverec.h"

class CLogPulse :
	public CWaveRec
{
public:
	CLogPulse(void);
	virtual ~CLogPulse(void);
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
