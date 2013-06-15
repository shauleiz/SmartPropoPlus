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
#define SPP_SELECT  "Selected"
#define SPP_DEVMAP  "Device_Map"
#define SPP_MAPAX   "Axes"
#define SPP_MAPBTN  "Buttons"


class CSppConfig
{
public:
	//CSppConfig(void);
	CSppConfig(LPTSTR FileName = DEF_CONF_FILE);
	virtual ~CSppConfig(void);
	bool SelectvJoyDevice(UINT id);
	UINT SelectedvJoyDevice(void);
	void MapAxis(UINT id, DWORD map);
	DWORD CSppConfig::MapAxis(UINT id);

	void Test(void);

private:
	TiXmlDocument * CreateDefaultConfig(TiXmlDocument *  doc = NULL);
	TiXmlHandle		CreatevJoyDevice(UINT id, bool selected = false);
	UINT			GetSingleAxisMap(TiXmlHandle DeviceHandle, const char * axis);


private:
	TiXmlDocument m_doc;

};


std::string utf8_encode(const std::wstring &wstr);
std::wstring utf8_decode(const std::string &str);
TiXmlHandle UniqueTextLeaf(TiXmlHandle Parent,  std::string &LeafName, std::string &LeafText, bool Replace);

