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
	char m_FileDbgInSigName[FILENAME_MAX];
};

