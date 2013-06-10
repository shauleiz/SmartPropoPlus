#pragma once
#define TIXML_USE_STL 1
#include "../TinyXml/TinyXml.h"

#ifdef X64
#ifdef _DEBUG
#pragma  comment(lib, "..\\tinyxml\\x64\\Debug_STL\\tinyxmlSTL.lib")
#else
#pragma  comment(lib, "..\\tinyxml\\x64\\Release_STL\\tinyxmlSTL.lib")
#endif
#else
#ifdef _DEBUG
#pragma  comment(lib, "..\\tinyxml\\Debug_STL\\tinyxmlSTL.lib")
#else
#pragma  comment(lib, "..\\tinyxml\\Release_STL\\tinyxmlSTL.lib")
#endif
#endif

#define DEF_CONF_FILE TEXT("Config.xml")
#define DEF_CONF_DIR TEXT("SmartPropoPlus")
#define SPP_ROOT "SmartPropoPlus"
#define SPP_SUBVER 0001
#define SPP_VER 4
#define SPP_TARGETS "Targets"
#define SPP_VJOYDEV "vJoy_Device"
#define SPP_VJOYID  "Id"
class CSppConfig
{
public:
	CSppConfig(void);
	CSppConfig(LPTSTR FileName);
	virtual ~CSppConfig(void);

private:
	TiXmlDocument * CreateDefaultConfig(TiXmlDocument *  doc = NULL);

private:
	TiXmlDocument * m_doc;

};


std::string utf8_encode(const std::wstring &wstr);
std::wstring utf8_decode(const std::string &str);

