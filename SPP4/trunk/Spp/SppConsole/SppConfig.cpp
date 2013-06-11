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
	std::wstring w_full_path;
	std::string full_path;
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

	std::string idString = std::to_string(id);
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
		UniqueTextLeaf(AxisHandle, std::string("SL1"), std::to_string(map & 0xF), true);
	if ((map & 0xF0)>>4)
		UniqueTextLeaf(AxisHandle, std::string("SL0"), std::to_string ((map & 0xF0)>>4), true);
	if ((map & 0xF00)>>8)
		UniqueTextLeaf(AxisHandle, std::string("RZ"), std::to_string ((map & 0xF00)>>8), true);
	if ((map & 0xF000)>>12)
		UniqueTextLeaf(AxisHandle, std::string("RY"), std::to_string ((map & 0xF000)>>12), true);
	if ((map & 0xF0000)>>16)
		UniqueTextLeaf(AxisHandle, std::string("RX"), std::to_string ((map & 0xF0000)>>16), true);
	if ((map & 0xF00000)>>20)
		UniqueTextLeaf(AxisHandle, std::string("Z"), std::to_string ((map & 0xF00000)>>20), true);
	if ((map & 0xF000000)>>24)
		UniqueTextLeaf(AxisHandle, std::string("Y"), std::to_string ((map & 0xF000000)>>24), true);
	if ((map & 0xF0000000)>>28)
		UniqueTextLeaf(AxisHandle, std::string("X"), std::to_string ((map & 0xF0000000)>>28), true);
}

void CSppConfig::Test(void)
{
	//CreatevJoyDevice(5);
	CreatevJoyDevice(0);
	MapAxis(1, 0x89A00000);
	 //SelectvJoyDevice(5);
	 SelectvJoyDevice(22);

	m_doc.SaveFile();
}

/////////// Helper functions
// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Create/Replace a UNIQUE leaf element with its text
// Parameters:
//	Parent:		Handle of the parent element
//	LeafName:	The name of the leaf to create (if does not exist)
//	LeafText:	The text to insert in the leaf element
//	Replace:	If the leaf element exist - should the text be replaced
// Returns: Handle to leaf element if successful - NULL otherwize
TiXmlHandle UniqueTextLeaf(TiXmlHandle Parent,  std::string &LeafName, std::string &LeafText, bool Replace)
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