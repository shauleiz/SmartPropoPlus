#pragma once
using namespace std;
#include <tinyxml.h>
#include <tinystr.h>

///////////////////////////////////////////////////////////////////////////
#define	STR_CTRL_XML		L"\\SmartPropoPlus\\Control.xml"
#define	STR_DEFLT_CTRL_XML	L".\\Control.xml"
#define	XML_ROOT_ELEM		L"SmartPropoPlus"
#define	XML_AUDIO_ELEM		"Audio"
#define	XML_CEP_ELEM		"CaptureEndPoint"
#define	XML_ID_ELEM			"id"
#define	XML_COLOR_ELEM		"color"
#define	XML_FNAME_ELEM		"friendlyName"
#define	XML_CSTAT_ELEM		"currentState"
#define	XML_ENBLD_ELEM		"Enabled"
#define	XML_CHNLS_ELEM		"Channels"
#define	XML_SELEP_ELEM		"SelectedEndPoint"
#define	XML_ORIG_ELEM		"Original"
#define	XML_CURR_ELEM		"Current"
#define	XML_DCDRS_ELEM		"Decoders"
#define	XML_DCDR_ELEM		"Decoder"
#define	XML_VER_ELEM		"ver"

typedef std::map<std::string,std::string> MessageMap;

struct StateCapEP
{
	char Enabled[4]; // Yes/No
	char Channels[4]; // 2,3,4,5.1,7.1
};

struct CaptureEndPoint
{	// Reflect element <CaptureEndPoint>
	char friendlyName[MAX_PATH]; // UTF-8
	char id[MAX_PATH]; // Ex: {0.0.1.00000000}{E3DCE116-D402-4b6a-842F-4FDD238FB870}
	char color[12];		// Ex: 0x00FF8080
	StateCapEP currentState;
	StateCapEP originalState;
};

struct Decoder
{	// Reflect element <Decoder>
	char friendlyName[MAX_PATH]; // UTF-8
	char id[40]; // GUID representing decoder's id
	char ver[10]; // Version number
};

///////////////////////////////////////////////////////////////////////////

class CConfig
{
public:
	CConfig(void);
	virtual ~CConfig(void);
	bool LoadConfigFile(LPCWSTR FileName);
	bool SaveConfigFile(LPCWSTR FileName);
	bool ExistCaptureEP(void);
	bool GetFirstCaptureEP(const CaptureEndPoint * data);
	bool GetSelectedCaptureEP(const CaptureEndPoint * data, bool Current=true);
	bool GetCaptureEPbyIndex(const CaptureEndPoint * data, int index);
	bool ExistDecoder(void);
	bool GetDecoderbyIndex(const Decoder * data, int index);
	bool GetSelectedDecoder(const Decoder * data);

private:
	bool IsIdentical(const char * utf8, LPCWSTR wStr);
	bool IsIdentical(LPCWSTR wStr, const char * utf8);
	bool GetCaptureEP(TiXmlElement * pElem, const CaptureEndPoint * data);
	bool GetCaptureEPbyId(const CaptureEndPoint * data, const char * id);
	bool GetDecoder(TiXmlElement * pElem, const Decoder * data);
	bool GetDecoderbyId(const Decoder * data, const char * id);


private:
	TiXmlHandle m_hRoot;
	TiXmlHandle m_hDoc;
	LPWSTR		m_XmlPath;
};

