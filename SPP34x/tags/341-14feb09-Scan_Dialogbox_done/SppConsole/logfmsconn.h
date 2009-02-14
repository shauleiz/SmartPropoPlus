#pragma once
#include "waverec.h"

class CLogFmsConn :
	public CWaveRec
{
public:
	CLogFmsConn(void);
	virtual ~CLogFmsConn(void);
protected:
	virtual void UpdateControls(void);
	virtual void PostMessageStop();
	virtual void PostMessageStart();
	virtual void PostMessageCancel();

	// Encasulation functions
	virtual BOOL SppLoggerStop(void);
	virtual BOOL SppLoggerStart(TCHAR * FileName);
};
