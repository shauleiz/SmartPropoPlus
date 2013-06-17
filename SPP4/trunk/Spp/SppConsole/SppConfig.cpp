#include "stdafx.h"
#include "SppConfig.h"
#include "Knownfolders.h"
#include "Shlobj.h"


//CSppConfig::CSppConfig(void)
//{
//	CSppConfig(DEF_CONF_FILE);
//}

CSppConfig::CSppConfig(LPTSTR FileName) 
{
	HRESULT hr = S_OK;
	PWSTR path;
	wstring w_full_path;
	string full_path;
	bool loaded, saved;
	//const char *ErrorTxt;

	// Get the full path name of the config file and convert it to UTF8
	hr  = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
	w_full_path = path;
	w_full_path = w_full_path + TEXT("\\") + DEF_CONF_DIR + TEXT("\\") + FileName;
	full_path = utf8_encode(w_full_path);

	// Create a document and try to load the config file
	//m_doc = new TiXmlDocument(full_path);
	loaded = m_doc.LoadFile(full_path);

	// If not loaded then create a default config file (And sub-folder)
	if (!loaded && m_doc.ErrorId()/* == TiXmlBase::TIXML_ERROR_OPENING_FILE*/)
	{
		m_doc.ClearError();
		CreateDefaultConfig(&m_doc);
		w_full_path = path;
		w_full_path = w_full_path + TEXT("\\") + DEF_CONF_DIR;
		CreateDirectory(w_full_path.c_str(), 0);
	};
	// TODO: Log operation


	saved = m_doc.SaveFile();
	if (!saved)
		m_doc = NULL;
}

CSppConfig::~CSppConfig(void)
{
}

// Given a pointer to an already created document this function creates
// a default config in this document
// If a NULL pointer is passed, this function creates a document
// The function returns a pointer to a valid default document
TiXmlDocument * CSppConfig::CreateDefaultConfig(TiXmlDocument *  doc)
{
	TiXmlDocument *  i_doc = doc;
	// if input is NULL - Create a new document
	if (!doc)
		i_doc  = new TiXmlDocument();

	// Clear document from previous content
	i_doc->Clear();

	// Create Declaration
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "yes" );
	i_doc->LinkEndChild(decl);

	// Create root element and comments
	TiXmlElement * root = new TiXmlElement(SPP_ROOT);
	root->SetAttribute("Version", SPP_VER);
	root->SetAttribute("SubVersion", SPP_SUBVER);
	doc->LinkEndChild(new TiXmlComment("This file holds the configuration of SmartPropoPlus - Tampering with it will not be wise"));
	doc->LinkEndChild( root );

	//// Add default settings
	// Default target is vJoy Device #1
	TiXmlElement * Targets = new TiXmlElement(SPP_TARGETS);
	root->LinkEndChild(Targets);
	TiXmlElement * vJoyDevice = new TiXmlElement(SPP_VJOYDEV);
	Targets->LinkEndChild(vJoyDevice);
	TiXmlElement * vJoyDevId = new TiXmlElement(SPP_VJOYID);
	vJoyDevId->LinkEndChild(new TiXmlText( "1" ));
	vJoyDevice->LinkEndChild(vJoyDevId);

	return i_doc;
}

// Target Section - vJoy device related info
// Only assumption is: Document (m_doc) exists
// If Section does not exist then create it
// If vJoy device does not exist then create it

// CreatevJoyDevice - Create by ID if does not exist (Optionally set as selected)
// Opens if exists (Change 'select' value if needed)
// Returns handle to the vJoy_Device Element
TiXmlHandle  CSppConfig::CreatevJoyDevice(UINT id, bool selected)
{
	// Sanity check - document exists, id is valid
	if ( id<1 || id>16)
		return NULL;

	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return NULL;
	TiXmlHandle RootHandle( root );

	// If Section 'Targets' does not exist - create it
	TiXmlElement* Targets = RootHandle.FirstChild( SPP_TARGETS ).ToElement();
	if (!Targets)
	{
		Targets = new TiXmlElement(SPP_TARGETS);
		root->LinkEndChild(Targets);
	};
	
	// If selected==true - assign id to the attribute 'selected'
	if (selected)
			Targets->SetAttribute(SPP_SELECT, id);

	// if vJoy_Device(Id=id) does not exist - create it
	TiXmlElement* Device = RootHandle.FirstChild( SPP_TARGETS ).FirstChild(SPP_VJOYDEV).ToElement();

	// Now we know that there's at least one device - look for the one with the requested id
	for (Device; Device; Device = Device->NextSiblingElement())
	{
		TiXmlHandle DeviceHandle( Device );
		TiXmlElement* IdElement = DeviceHandle.FirstChild(SPP_VJOYID).ToElement();
		if (!IdElement)
			continue;
		const char * idStr = IdElement->GetText();
		if (!idStr)
			continue;
		if (id == atoi(idStr))
			break;
	};

	string idString = to_string(id);
	if (!Device)
	{
		// No devices exist - create one
		Device =  new TiXmlElement(SPP_VJOYDEV);
		Targets->LinkEndChild(Device);
		TiXmlElement * Id = new TiXmlElement(SPP_VJOYID);
		Device->LinkEndChild(Id);
		TiXmlText * idTxt = new TiXmlText(idString );
		Id->LinkEndChild(idTxt);
	};

	// Obtain handle to the opened/created vJoy_Device Element
	TiXmlHandle out( Device );
	return out;
}

// SelectvJoyDevice - Set a device to be marked as selected
// If does not exist then create it first
bool CSppConfig::SelectvJoyDevice(UINT id)
{
	TiXmlHandle h = CreatevJoyDevice(id, true);
	if (h.ToElement())
		return true;
	return false;
}

// SelectedvJoyDevice - Get the Id of the selected vJoy device
// Returns the Id of the selected device.
// If There's only one device - it returns it's Id
// If Unable to determine return 0
UINT CSppConfig::SelectedvJoyDevice(void)
{
	int id =0;
	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return id;
	TiXmlHandle RootHandle( root );

	// Get 'Targets' and get its 'selected' attribute
	TiXmlElement* Targets = RootHandle.FirstChild( SPP_TARGETS ).ToElement();
	if (!Targets)
		return id;
	const char * selected = Targets->Attribute(SPP_SELECT);
	if (selected)
		return atoi(selected);

	// If no 'selected' attribute then:
	/// If there are no children then return 0
	if (Targets->NoChildren())
		return id;

	// Get the ID of the first child
	TiXmlElement *  Device = Targets->FirstChildElement(SPP_VJOYDEV);
	TiXmlHandle DeviceHandle( Device );
	TiXmlElement* IdElement = DeviceHandle.FirstChild(SPP_VJOYID).ToElement();
	if (!IdElement)
		return 0;
	const char * idStr = IdElement->GetText();
	if (!idStr)
		return 0;
	id = atoi(idStr);

	// If only one return its ID - else return 0
	if (Device->NextSiblingElement(SPP_VJOYDEV))
		return 0;
	else
		return id;
}

// MapAxis - Register the channel-to-axis mapping of a given vJoy device
// If device does not exist then create it first
// Device accessed by Id
//
// The number of axes currently supported is 1 to 8
// The number of channels currently supported is 1 to 15
// Each map nibble represents a channel to axis mapping. The nibble value is the channel index (1-based). The nibble location is the axis
// Nibbles are set from the upper nibble (X axis) to lower nibble (SL1 axis)
// Special case: Map nibble = 0, don't change mapping for this axis.
void CSppConfig::MapAxis(UINT id, DWORD map)
{
	// Create a vjoy device (if it does not exist)
	TiXmlHandle  DeviceHandle = CreatevJoyDevice(id);

	// Create a Device_Map/Axes element (if it does not exist) - there's only one
	TiXmlElement * Device_Map = DeviceHandle.FirstChildElement(SPP_DEVMAP).ToElement();
	if (!Device_Map)
	{
		Device_Map = new TiXmlElement(SPP_DEVMAP);
		DeviceHandle.ToElement()->LinkEndChild(Device_Map);
	};
	TiXmlHandle MapHandle(Device_Map);
	TiXmlElement * Axis_Map = MapHandle.FirstChildElement(SPP_MAPAX).ToElement();
	if (!Axis_Map)
	{
		Axis_Map = new TiXmlElement(SPP_MAPAX);
		MapHandle.ToElement()->LinkEndChild(Axis_Map);
	};
	TiXmlHandle AxisHandle(Axis_Map);

	// For every nibble in map (that is not 0) Create/Replace value
	if (map & 0xF)
		UniqueTextLeaf(AxisHandle, string("SL1"), to_string(map & 0xF), true);
	if ((map & 0xF0)>>4)
		UniqueTextLeaf(AxisHandle, string("SL0"), to_string ((map & 0xF0)>>4), true);
	if ((map & 0xF00)>>8)
		UniqueTextLeaf(AxisHandle, string("RZ"), to_string ((map & 0xF00)>>8), true);
	if ((map & 0xF000)>>12)
		UniqueTextLeaf(AxisHandle, string("RY"), to_string ((map & 0xF000)>>12), true);
	if ((map & 0xF0000)>>16)
		UniqueTextLeaf(AxisHandle, string("RX"), to_string ((map & 0xF0000)>>16), true);
	if ((map & 0xF00000)>>20)
		UniqueTextLeaf(AxisHandle, string("Z"), to_string ((map & 0xF00000)>>20), true);
	if ((map & 0xF000000)>>24)
		UniqueTextLeaf(AxisHandle, string("Y"), to_string ((map & 0xF000000)>>24), true);
	if ((map & 0xF0000000)>>28)
		UniqueTextLeaf(AxisHandle, string("X"), to_string ((map & 0xF0000000)>>28), true);
}

// MApAxis - Get the channel-to-axis mapping of a given vJoy device
// If device does not exist then return 0
// Device accessed by Id
//
// The number of axes currently supported is 1 to 8
// The number of channels currently supported is 1 to 15
// Each map nibble represents a channel to axis mapping. The nibble value is the channel index (1-based). The nibble location is the axis
// Nibbles are set from the upper nibble (X axis) to lower nibble (SL1 axis)
// Special case: Map nibble = 0 - no info for this axis
DWORD CSppConfig::MapAxis(UINT id)
{

	// Get handle to vJoy Device
	TiXmlHandle DeviceHandle = CreatevJoyDevice(id);
	 
	int x_val =  GetSingleAxisMap(DeviceHandle, "X");
	int y_val =  GetSingleAxisMap(DeviceHandle, "Y");
	int z_val =  GetSingleAxisMap(DeviceHandle, "Z");
	int rx_val =  GetSingleAxisMap(DeviceHandle, "RX");
	int ry_val =  GetSingleAxisMap(DeviceHandle, "RY");
	int rz_val =  GetSingleAxisMap(DeviceHandle, "RZ");
	int sl0_val =  GetSingleAxisMap(DeviceHandle, "SL0");
	int sl1_val =  GetSingleAxisMap(DeviceHandle, "SL1");

	DWORD out = (sl1_val<<0) | (sl0_val<<4) | (rz_val<<8) | (ry_val<<12) | (rx_val<<16) | (z_val<<20) | (y_val<<24) | (x_val<<28);

	return out;

}


UINT CSppConfig::GetSingleAxisMap(TiXmlHandle DeviceHandle, const char * axis)
{
	TiXmlText * Text = DeviceHandle.FirstChildElement(SPP_DEVMAP).FirstChildElement(SPP_MAPAX).FirstChildElement(axis).FirstChild().ToText();
	if (Text)
		return  stoi (Text->ValueStr());
	else
		return 0;
}

// AddModulation - Create/Replace a modulation entry (Optionally mark it as the selected modulation)
//
// Parameters:
//	Type: Short name (PPM, WAL, AIR1)
//	Subtype: (PPM or PCM)
//	Name: Friendly name such as �Sanwa/Air (PCM1)�
//	select: If true, mark this modulation as the selected
bool CSppConfig::AddModulation(string Type, string SubType, wstring Name, bool select)
{
	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return false;
	TiXmlHandle RootHandle( root );

	// If Section 'Modulations' does not exist - create it
	TiXmlElement* Modulations = RootHandle.FirstChild( SPP_MODS ).ToElement();
	if (!Modulations)
	{
		Modulations = new TiXmlElement(SPP_MODS);
		root->LinkEndChild(Modulations);
	};
	
	// If selected==true - assign id to the attribute 'selected'
	if (select)
			Modulations->SetAttribute(SPP_SELECT, Type);

	// Get/Create modulation entry of type 'Type'
	TiXmlElement* Mod = RootHandle.FirstChild( SPP_MODS ).FirstChild(SPP_MOD).ToElement();

	// Look for the Modulation element with the requested Type
	TiXmlElement* TypeElement;
	for (Mod; Mod; Mod = Mod->NextSiblingElement())
	{
		TiXmlHandle ModHandle( Mod );
		TypeElement = ModHandle.FirstChild(SPP_MODTYPE).ToElement();
		if (!TypeElement)
			continue;
		const char * TypeStr = TypeElement->GetText();
		if (!TypeStr)
			continue;
		if (!Type.compare(TypeStr))
			break;
	};

	// If Modulation element with the requested Type does not exist - create it
	if (!Mod)
	{
		// No devices exist - create one
		Mod =  new TiXmlElement(SPP_MOD);
		Modulations->LinkEndChild(Mod);
	};
	TiXmlHandle ModHandle( Mod );

	// Enter type
	UniqueTextLeaf(ModHandle, string(SPP_MODTYPE), Type, true);

	// Enter subtype
	UniqueTextLeaf(ModHandle, string(SPP_MODSUBT), SubType, true);

	// Convert & Enter name
	UniqueTextLeaf(ModHandle, string(SPP_MODNAME), utf8_encode(Name), true);

	return true;

}
bool CSppConfig::AddModulation(LPTSTR Type, LPTSTR SubType, LPTSTR Name, bool select)
{
	// Convert LPTSTR to wstring
	wstring wstr_Type = wstring(Type);
	wstring wstr_SubType = wstring(SubType);
	wstring wstr_Name = wstring(Name);

	// 

	return AddModulation(utf8_encode(wstr_Type), utf8_encode(wstr_SubType), wstr_Name, select);
}

// GetSelectedModulation - Get the selected modulation type
// Return the Type (PPMW, AIR1 ...) according to the 'selected' attribute
// If absent - return an empty string
string  CSppConfig::GetSelectedModulation(void)
{

	string selected =  string("");
	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return selected;

	TiXmlHandle RootHandle( root );

	// Get 'Modulations' and get its 'selected' attribute
	TiXmlElement* Modulations = RootHandle.FirstChild( SPP_MODS ).ToElement();
	if (!Modulations)
		return selected;
	const char * attr = Modulations->Attribute(SPP_SELECT);
	if (attr)
		selected = string(attr);
	return selected;
}

// GetModulationHandle - find a modulation element by 'Type'
// Return handle of the element
TiXmlHandle	 CSppConfig::GetModulationHandle(string Type)
{
	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return NULL;
	TiXmlHandle RootHandle( root );

	// Get Section 'Modulations'
	TiXmlElement* Modulations = RootHandle.FirstChild( SPP_MODS ).ToElement();
	if (!Modulations)
		return NULL;
	
	// Look for the Modulation element with the requested Type
	TiXmlElement* Mod = RootHandle.FirstChild( SPP_MODS ).FirstChild(SPP_MOD).ToElement();
	TiXmlElement* TypeElement;
	for (Mod; Mod; Mod = Mod->NextSiblingElement())
	{
		TiXmlHandle ModHandle( Mod );
		TypeElement = ModHandle.FirstChild(SPP_MODTYPE).ToElement();
		if (!TypeElement)
			continue;
		const char * TypeStr = TypeElement->GetText();
		if (!TypeStr)
			continue;
		if (!Type.compare(TypeStr))
			break;
	};
	if (!Mod)
		return NULL;

	// Found
	TiXmlHandle ModHandle( Mod );
	return ModHandle;
}


// GetSubTypeModulation - find a modulation element by 'Type'
// Return its subtype if exists - otherwise return empty string
string		CSppConfig::GetSubTypeModulation(string Type)
{
	string SubType;
	SubType.clear();

	TiXmlHandle	 MofHandle = GetModulationHandle(Type);
	TiXmlText * Text = MofHandle.FirstChildElement(SPP_MODSUBT).FirstChild().ToText();
	if (Text)
		SubType =  Text->ValueStr();
	return SubType;
}

// GetSubTypeModulation - find a modulation element by 'Type'
// Return its Name if exists - otherwise return empty string
string CSppConfig::GetNameModulation(string Type)
{
	string Name;
	Name.clear();

	TiXmlHandle	 MofHandle = GetModulationHandle(Type);
	TiXmlText * Text = MofHandle.FirstChildElement(SPP_MODNAME).FirstChild().ToText();
	if (Text)
		Name =  Text->ValueStr();
	return Name;

}

string CSppConfig::GetSubTypeModulationSelected()
{
	return GetSubTypeModulation(GetSelectedModulation());
}

string CSppConfig::GetNameModulationSelected()
{
	return GetNameModulation(GetSelectedModulation());
}

// AddAudioDevice - Add reference to an audio device
// Parameters:
//	Id: Unique identifier (e.g. {0.0.1.00000000}.{8512fa69-b703-45b9-b105-c35f74a51950})
//	Name: Friendly name (e.g. �Mic In at front panel (Pink) (Realtek High Definition Audio)�)
//	BitRate: 8/16
//	Channel: Left/Right
//
bool CSppConfig::AddAudioDevice(LPTSTR Id, LPTSTR Name, UINT BitRate, LPTSTR Channel)
{

	string str_Id = utf8_encode(wstring(Id));


	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return false;
	TiXmlHandle RootHandle( root );

	// If Section 'Audio' does not exist - create it
	TiXmlElement* Audio = RootHandle.FirstChild( SPP_AUDIO ).ToElement();
	if (!Audio)
	{
		Audio = new TiXmlElement(SPP_AUDIO);
		root->LinkEndChild(Audio);
	};
	
	// Get/Create Audio device element of type 'ID'
	TiXmlElement* Dev = RootHandle.FirstChild( SPP_AUDIO ).FirstChild(SPP_AUDDEV).ToElement();

	// Look for the audio device element with the requested ID
	TiXmlElement* IdElement;
	for (Dev; Dev; Dev = Dev->NextSiblingElement())
	{
		TiXmlHandle DevHandle( Dev );
		IdElement = DevHandle.FirstChild(SPP_AUDID).ToElement();
		if (!IdElement)
			continue;
		const char * IdStr = IdElement->GetText();
		if (!IdStr)
			continue;
		if (!str_Id.compare(IdStr))
			break;
	};

	// If Audio device element with the requested ID does not exist - create it
	if (!Dev)
	{
		// No devices exist - create one
		Dev =  new TiXmlElement(SPP_AUDDEV);
		Audio->LinkEndChild(Dev);
	};
	TiXmlHandle DevHandle( Dev );

	// Id: Convert & Enter
	UniqueTextLeaf(DevHandle, string(SPP_AUDID), utf8_encode(Id), true);

	// Name: Convert & Enter
	UniqueTextLeaf(DevHandle, string(SPP_AUDNAME), utf8_encode(Name), true);

	// Channel Convert & Enter
	UniqueTextLeaf(DevHandle, string(SPP_AUDCH), utf8_encode(Channel), true);

	// Bit Rate
	UniqueTextLeaf(DevHandle, string(SPP_AUDBR), to_string(BitRate), true);

	return true;

}

TiXmlHandle CSppConfig::GetAudioHandle(LPTSTR Id)
{
		string str_Id = utf8_encode(wstring(Id));

	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return NULL;
	TiXmlHandle RootHandle( root );

	// Get Section 'Audio'
	TiXmlElement* Audio = RootHandle.FirstChild( SPP_AUDIO ).ToElement();
	if (!Audio)
		return NULL;

	// Get Audio device element of type 'ID'
	TiXmlElement* Dev = RootHandle.FirstChild( SPP_AUDIO ).FirstChild(SPP_AUDDEV).ToElement();

	// Look for the audio device element with the requested ID
	TiXmlElement* IdElement;
	for (Dev; Dev; Dev = Dev->NextSiblingElement())
	{
		TiXmlHandle DevHandle( Dev );
		IdElement = DevHandle.FirstChild(SPP_AUDID).ToElement();
		if (!IdElement)
			continue;
		const char * IdStr = IdElement->GetText();
		if (!IdStr)
			continue;
		if (!str_Id.compare(IdStr))
			break;
	};
	if (!Dev)
		return NULL;

	// Found
	TiXmlHandle DevHandle( Dev );
	return DevHandle;

}

wstring CSppConfig::GetAudioDeviceName(LPTSTR Id)
{
	string Name = "";

	TiXmlHandle h = GetAudioHandle(Id);
	TiXmlText * Text = h.FirstChildElement(SPP_AUDNAME).FirstChild().ToText();
	if (Text)
		Name =  Text->ValueStr();

	return  utf8_decode(Name);
}

wstring CSppConfig::GetAudioDeviceChannel(LPTSTR Id)
{
	string Name = "";

	TiXmlHandle h = GetAudioHandle(Id);
	TiXmlText * Text = h.FirstChildElement(SPP_AUDCH).FirstChild().ToText();
	if (Text)
		Name =  Text->ValueStr();

	return  utf8_decode(Name);
}

UINT CSppConfig::GetAudioDeviceBitRate(LPTSTR Id)
{
	string Value = "0";
	TiXmlHandle h = GetAudioHandle(Id);
	TiXmlText * Text = h.FirstChildElement(SPP_AUDBR).FirstChild().ToText();
	if (Text)
		Value =  Text->ValueStr();

	return  stoi(Value);
}

// FilterFile - Create/Replace the 'Filters' subtree
// Parameters:
//	FilePath: Full path name of the filter DLL file (Creates element File_Name)
//	Version:  e.g. 3.3.8 (Creates element DLL_Version)
bool CSppConfig::FilterFile(LPTSTR FilePath, LPTSTR Version)
{
	string str_Name = utf8_encode(wstring(FilePath));
	string str_Ver = utf8_encode(wstring(Version));

	// Get handle of the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return false;
	TiXmlHandle RootHandle( root );

	// If Section 'Filters' does not exist - create it
	TiXmlElement* Filters = RootHandle.FirstChild( SPP_FILTERS ).ToElement();
	if (!Filters)
	{
		Filters = new TiXmlElement(SPP_FILTERS);
		root->LinkEndChild(Filters);
	};
	TiXmlHandle FiltersHandle(Filters);

	// Add Filter DLL file name
	UniqueTextLeaf(FiltersHandle, string(SPP_DLLNAME), utf8_encode(FilePath), true);

	// Add Filter DLL file version
	UniqueTextLeaf(FiltersHandle, string(SPP_DLLVER), utf8_encode(Version), true);

	return true;
}

bool CSppConfig::AddFilter(UINT Id, LPTSTR Name, bool select)
{
	// Get the root
	TiXmlElement* root = m_doc.FirstChildElement( SPP_ROOT);
	if (!root)
		return false;
	TiXmlHandle RootHandle( root );

	// Get the 'Filters' element
	TiXmlElement* Filters = RootHandle.FirstChild( SPP_FILTERS ).ToElement();
	if (!Filters)
		return false;

	// Set 'select' attribute (if needed)
	if (select)
			Filters->SetAttribute(SPP_SELECT, Id);

	// Create/Replace 'filter' element (By Id)
	TiXmlHandle	FilterHandle = GetByKey(Filters, SPP_FILTER, SPP_FLTID, to_string(Id));
	if (!FilterHandle.ToElement())
		return false;

	// Set filter's Name
	string str_Name = utf8_encode(wstring(Name));
	UniqueTextLeaf(FilterHandle, string(SPP_FLTNAME) , str_Name , true);
	return true;
}

void CSppConfig::Test(void)
{
	//CreatevJoyDevice(5);
	CreatevJoyDevice(0);
	 MapAxis(4);
	MapAxis(1, 0x89A00000);
	 //SelectvJoyDevice(5);
	 SelectvJoyDevice(22);

	 AddModulation("PPM", "PPM", L"Generic PPM", true);
	 AddModulation("PPMW", "PPM", L"Walkera PPM");
	 string sel = GetSelectedModulation();
	 string st = GetSubTypeModulation(string("PPMW"));
	 string name = GetNameModulation(string("PPMW"));
	 st = GetSubTypeModulationSelected();
	 name = GetNameModulationSelected();

	 bool ok = AddAudioDevice(L"{0.0.1.00000000}.{8512fa69-b703-45b9-b105-c35f74a51950}", L"Mic In at front panel (Pink) (Realtek High Definition Audio)", 16, L"Left");
	 wstring Name = GetAudioDeviceName(L"{0.0.1.00000000}.{8512fa69-b703-45b9-b105-c35f74a51950}");
	 int br = GetAudioDeviceBitRate(L"{0.0.1.00000000}.{8512fa69-b703-45b9-b105-c35f74a51950}");
	 wstring ch = GetAudioDeviceChannel(L"{0.0.1.00000000}.{8512fa69-b703-45b9-b105-c35f74a51950}");

	FilterFile(L"C:\\TEMP\\My Filter.dll", L"3.3.9");
	AddFilter(308, L"Filter 0308");
	AddFilter(309, L"Filter 0309",true);

	m_doc.SaveFile();
}

/////////// Helper functions
// Convert a wide Unicode string to an UTF8 string
string utf8_encode(const wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
wstring utf8_decode(const string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
LPCTSTR utf8_decode(const char * str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)strlen(str), NULL, 0);
	if (!size_needed)
		return NULL;

    LPTSTR wstrTo = (LPTSTR)calloc(size_needed + 1, sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)strlen(str), wstrTo, size_needed);
    return wstrTo;
}

// Create/Replace a UNIQUE leaf element with its text
// Parameters:
//	Parent:		Handle of the parent element
//	LeafName:	The name of the leaf to create (if does not exist)
//	LeafText:	The text to insert in the leaf element
//	Replace:	If the leaf element exist - should the text be replaced
// Returns: Handle to leaf element if successful - NULL otherwize
TiXmlHandle UniqueTextLeaf(TiXmlHandle Parent,  string &LeafName, string &LeafText, bool Replace)
{
	// Does the leaf exist and unique
	TiXmlElement * Leaf[2];
	Leaf[0] = Parent.ChildElement(LeafName, 0).ToElement();
	Leaf[1] = Parent.ChildElement(LeafName, 1).ToElement();
	if (Leaf[0] && Leaf[1]) // Leaf not unique
		return 0;

	if (!Replace && Leaf[0]) // Leaf already exists and not to be replaced
		return 0;

	if (!Leaf[0]) // Leaf does not exist - Create it
	{
		Leaf[0] = new TiXmlElement(LeafName);
		Parent.ToElement()->LinkEndChild(Leaf[0]);
	}

	// Replace
	Leaf[0]->Clear();
	Leaf[0]->LinkEndChild(new  TiXmlText(LeafText));
	TiXmlHandle LeafHandle(Leaf[0]);
	return LeafHandle;
}

// Find a leaf element by a key - create if absent
// Parameters:
//	hParent:	Handle of the parent element
//	Child:		The elemnt to be searched (Child of Parent)
//	Key:		The sub-elemnt of child by which to search (Grand child of Parent)
//	Value:		Key text value to be matched
TiXmlHandle	GetByKey(TiXmlHandle hParent, string Child, string Key, string Value)
{
	// Test handle
	if (!hParent.ToElement())
		return NULL;

	// Get pointerto child entry
	TiXmlElement* ChildElement = hParent.FirstChild( Child ).ToElement();
	TiXmlElement* KeyElement;

	// Look for a Child whos Key value matches
	for (ChildElement; ChildElement; ChildElement = ChildElement->NextSiblingElement())
	{
		TiXmlHandle ChildHandle( ChildElement );
		KeyElement = ChildHandle.FirstChild(Key).ToElement();
		if (!KeyElement)
			continue;
		const char * KeyStr = KeyElement->GetText();
		if (!KeyStr)
			continue;
		if (!Value.compare(KeyStr))
			break;
	};

	// If Modulation element with the requested Type does not exist - create it
	if (!ChildElement)
	{
		// No devices exist - create one
		ChildElement =  new TiXmlElement(Child);
		hParent.ToElement()->LinkEndChild(ChildElement);
	};

	// Get handle to the found/Created child
	TiXmlHandle ModHandle( ChildElement);

	// Enter Key
	UniqueTextLeaf(ModHandle, string(Key), Value, true);

	return ModHandle;
}

