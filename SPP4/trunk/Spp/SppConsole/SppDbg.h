#pragma once
class SppDbg
{
public:
	SppDbg(void);
	virtual ~SppDbg(void);
	void StartDbgInputSignal(void);
	void StopDbgInputSignal(void);
	void InputSignalReady(PBYTE buffer, PVOID info);

private:
	FILE * m_FileDbgInSig;
	TCHAR m_FileDbgInSigName[MAX_PATH];
};

