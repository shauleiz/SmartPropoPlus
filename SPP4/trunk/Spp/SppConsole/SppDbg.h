#pragma once
class SppDbg
{
public:
	SppDbg(void);
	virtual ~SppDbg(void);
	void StartDbgInputSignal(void);
	void StopDbgInputSignal(void);
	void InputSignalReady(PBYTE buffer, PVOID info);
	void StartDbgPulse(void);
	void StopDbgPulse(void);
	void PulseReady(PVOID buffer, UINT ArraySize);

private:
	FILE * m_FileDbgInSig;
	FILE * m_FileDbgPulse;
	TCHAR m_FileDbgInSigName[MAX_PATH];
	TCHAR m_FileDbgPulseName[MAX_PATH];
};

