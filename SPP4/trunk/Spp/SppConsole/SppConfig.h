#pragma once
#define TIXML_USE_STL 1
#include "TinyXml.h"
#include <array>

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
#define SPP_GENERAL	"General"
#define SPP_MONCHNL	"Monitor_CH"
#define SPP_PLSSCP	"Pulse_SCP"
#define SPP_SHOWLOG	"Show_Log"
#define SPP_CHECKED	"Checked"
#define SPP_BTNPREF	"Button"
#define SPP_BACKUP	"Manual"
#define SPP_WIZARD	"Wizard_GUI"
#define SPP_STOPPED	"Stopped"



class CSppConfig
{
public:
	//CSppConfig(void);
	CSppConfig(HWND m_hPrntWnd, LPTSTR FileName = DEF_CONF_FILE);
	virtual 		~CSppConfig(void);

	// vJoy
	bool 			SelectvJoyDevice(UINT id);
	UINT 			SelectedvJoyDevice(void);
	DWORD 			MapAxis(UINT id);
	void			GetMapButtons(UINT id, BTNArr* ButtonMap);
	void			Map(UINT id, Mapping* GeneralMap);
	void			GetMap(UINT id, Mapping* GeneralMap);

	// Modulations
	bool			AddModulation(PVOID data);
	bool			SelectModulation(LPTSTR Type);
	wstring			GetSelectedModulation(void);
	string			GetSubTypeModulationSelected();
	wstring			GetNameModulationSelected();
	bool			SetAutoDecoder(bool);
	bool			IsDecoderAuto(void);

	// Audio
	bool			AddAudioDevice(LPTSTR Id, LPTSTR Name, bool select=false);
	bool			SetAudioDeviceBitRate(LPTSTR Id, UINT BitRate);
	bool			SetAudioDeviceChannel(LPTSTR Id, LPTSTR Channel);
	bool			SetDefaultBitRate(UINT BitRate);
	bool			SetDefaultChannel(LPTSTR Channel);
	bool			IsDefaultChannelRight();
	wstring			GetAudioDeviceName(LPTSTR Id);
	UINT			GetAudioDeviceBitRate(LPTSTR Id);
	wstring			GetAudioDeviceChannel(LPTSTR Id);
	bool			SetAutoChannel(bool, LPTSTR Channel=TEXT(""));
	bool			SetAutoBitRate(bool,UINT br=0);
	bool			IsDefaultChannelAuto(void);
	bool			IsDefaultBitRateAuto(void);

	// Filters
	bool			FilterFile(LPTSTR FilePath, LPTSTR Version);
	bool			AddFilter(UINT Id, LPTSTR Name, bool select=false);
	bool			AddFilter(UINT Id, const char * Name, bool select=false);

	UINT			GetSelectedFilter(void);
	wstring			GetSelectedFilterName(void);
	wstring			FilterFile(void);

	// General
	int				MonitorChannels();
	bool			MonitorChannels(bool Monitor);
	int				PulseScope();
	bool			PulseScope(bool Monitor);
	int				ShowLog();
	bool			ShowLog(bool Monitor);
	bool			Wizard();
	bool			Wizard(bool active);
	bool			Stopped();
	bool			Stopped(bool active);

	bool			Save(void);
	bool			Load(void);


	void Test(void);

private:
	TiXmlDocument * CreateDefaultConfig(TiXmlDocument *  doc = NULL);
	TiXmlHandle		CreatevJoyDevice(UINT id, bool selected = false);
	UINT			GetSingleAxisMap(TiXmlHandle DeviceHandle, const char * axis);
	UINT			GetSingleButtonMap(TiXmlHandle DeviceHandle, const UINT button);
	bool			AddModulation(wstring Type, wstring SubType, wstring Name, bool select=false);
	bool			AddModulation(LPTSTR Type, LPTSTR SubType, LPTSTR Name, bool select=false);
	string			GetSubTypeModulation(wstring Type);
	wstring			GetNameModulation(wstring Type);
	TiXmlHandle		GetModulationHandle(wstring Type);
	void			SetModulationAttrib(LPTSTR Attrib, LPTSTR Value);
	wstring			GetModulationAttrib(LPTSTR Attrib);
	void			RemoveModulationAttrib(LPTSTR Attrib);

	TiXmlHandle		GetAudioHandle(LPTSTR Id);
	bool			AddAudioDevice(LPTSTR Id, LPTSTR Name, UINT BitRate=0, LPTSTR Channel=NULL, bool select=false);
	bool			SetAudioAttrib(LPTSTR Id, LPTSTR Element, LPTSTR Attrib, LPTSTR Value);
	wstring			GetAudioAttrib(LPTSTR Id, LPTSTR Element, LPTSTR Attrib);
	void			RemoveAudioAttrib(LPTSTR Id, LPTSTR Element, LPTSTR Attrib);
	wstring			GetCurrentAudio(void);
	TiXmlHandle		GetFilterHandle(LPTSTR Id);
	wstring			GetFilterName(LPTSTR Id);
	TiXmlHandle		GetFilterFileHandle(void);
	void			MapAxis(UINT id, DWORD map);
	void			MapButtons(UINT id, BTNArr ButtonMap);

	int				GetGeneralElemetsBool(const char * Element);
	bool			SetGeneralElemetsBool(const char * Element, bool Val);
	void			LogMessage(int Severity, int Code, LPCTSTR Msg = NULL);



private:
	TiXmlDocument			m_doc;
	recursive_mutex			m_mx_General;
	FILE *					m_pFile;
	wstring					m_filename;
	HWND					m_hPrntWnd;
	HMODULE					m_hInstance;
};

string utf8_encode(const wstring &wstr);
wstring utf8_decode(const string &str);
LPCTSTR utf8_decode(const char * str);
TiXmlHandle UniqueTextLeaf(TiXmlHandle Parent,  string &LeafName, wstring &LeafText, bool Replace);
TiXmlHandle	GetByKey(TiXmlHandle hParent, string Child, string Key, wstring Value);
