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
#define SPP_MODS	"Modulations"
#define SPP_MOD		"Modulation"
#define SPP_MODTYPE	"Type"
#define SPP_MODSUBT	"SubType"
#define SPP_MODNAME	"Name"
#define SPP_AUDIO	"Audio"
#define SPP_AUDDEV	"Audio_Device"
#define SPP_AUDID	"Id"
#define SPP_AUDNAME	"Name"
#define SPP_AUDBR	"Bit_Rate"
#define SPP_AUDCH	"Channel"
#define SPP_FILTERS	"Filters"
#define SPP_DLLNAME	"DLL_Name"
#define SPP_DLLVER	"DLL_Version"
#define SPP_FILTER	"Filter"
#define SPP_FLTID	"Id"
#define SPP_FLTNAME	"Name"


class CSppConfig
{
public:
	//CSppConfig(void);
	CSppConfig(LPTSTR FileName = DEF_CONF_FILE);
	virtual 		~CSppConfig(void);

	// vJoy
	bool 			SelectvJoyDevice(UINT id);
	UINT 			SelectedvJoyDevice(void);
	void			MapAxis(UINT id, DWORD map);
	DWORD 			MapAxis(UINT id);

	// Modulations
	bool			AddModulation(LPTSTR Type, LPTSTR SubType, LPTSTR Name, bool select=false);
	string			GetSelectedModulation(void);
	string			GetSubTypeModulationSelected();
	string			GetNameModulationSelected();

	// Audio
	bool			AddAudioDevice(LPTSTR Id, LPTSTR Name, UINT BitRate=8, LPTSTR Channel=L"Left");
	wstring			GetAudioDeviceName(LPTSTR Id);
	UINT			GetAudioDeviceBitRate(LPTSTR Id);
	wstring			GetAudioDeviceChannel(LPTSTR Id);

	// Filters
	bool			FilterFile(LPTSTR FilePath, LPTSTR Version);
	bool			AddFilter(UINT Id, LPTSTR Name, bool select=false);
	UINT			GetSelectedFilter(void);
	wstring			GetSelectedFilterName(void);

	void Test(void);

private:
	TiXmlDocument * CreateDefaultConfig(TiXmlDocument *  doc = NULL);
	TiXmlHandle		CreatevJoyDevice(UINT id, bool selected = false);
	UINT			GetSingleAxisMap(TiXmlHandle DeviceHandle, const char * axis);
	bool			AddModulation(string Type, string SubType, wstring Name, bool select=false);
	string			GetSubTypeModulation(string Type);
	string			GetNameModulation(string Type);
	TiXmlHandle		GetModulationHandle(string Type);
	TiXmlHandle		GetAudioHandle(LPTSTR Id);
	TiXmlHandle		GetFilterHandle(LPTSTR Id);
	wstring			GetFilterName(LPTSTR Id);

private:
	TiXmlDocument m_doc;

};


string utf8_encode(const wstring &wstr);
wstring utf8_decode(const string &str);
LPCTSTR utf8_decode(const char * str);
TiXmlHandle UniqueTextLeaf(TiXmlHandle Parent,  string &LeafName, string &LeafText, bool Replace);
TiXmlHandle		GetByKey(TiXmlHandle hParent, string Child, string Key, string Value);
