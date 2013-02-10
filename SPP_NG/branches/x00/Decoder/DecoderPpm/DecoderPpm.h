#pragma once

// {69A138EA-C592-49cd-AE34-B70B5AA61A14}
DEFINE_GUID(GUID_DECODER_PPM, 0x69a138ea, 0xc592, 0x49cd, 0xae, 0x34, 0xb7, 0xb, 0x5a, 0xa6, 0x1a, 0x14);
/*
#define PW_FUTABA	6.623
#define PW_JR		7.340
#define PPMW_MIN	18.0
#define PPMW_MAX	70.0
#define PPMW_TRIG	200
#define PPMW_SEP	15.0
*/
#define PPM_SEP		15.0
#define PPM_TRIG	200
#define PPM_MIN		30.0
#define PPM_MAX		80.0

class CDecoderPpm : public CDecoderGen
{
public:
	SPPINTERFACE_API	CDecoderPpm(HWND hTarget = NULL, LPVOID Params = NULL);
	SPPINTERFACE_API	~CDecoderPpm(void);

	SPPINTERFACE_API	void ProcessPulse(int length, bool low);

private:
	double m_param_separate;
	double m_param_trigger;
	double m_param_min;
	double m_param_max;
};
