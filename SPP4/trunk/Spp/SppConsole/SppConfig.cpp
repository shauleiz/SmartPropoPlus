#include "stdafx.h"
#include "SppConfig.h"
#include "Knownfolders.h"
#include "Shlobj.h"


CSppConfig::CSppConfig(void)
{
	CSppConfig(DEF_CONF_FILE);
}

CSppConfig::CSppConfig(LPTSTR FileName) :
	m_doc(NULL)
{
	HRESULT hr = S_OK;
	PWSTR path;
	std::wstring w_full_path;
	std::string full_path;
	bool loaded, saved;
	const char *ErrorTxt;

	// Get the full path name of the config file and convert it to UTF8
	hr  = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &path);
	w_full_path = path;
	w_full_path = w_full_path + TEXT("\\") + DEF_CONF_DIR + TEXT("\\") + FileName;
	full_path = utf8_encode(w_full_path);

	// Create a document and try to load the config file
	TiXmlDocument * doc = new TiXmlDocument(full_path);
	loaded = doc->LoadFile();

	// If not loaded then create a default config file (And sub-folder)
	if (!loaded && doc->ErrorId()/* == TiXmlBase::TIXML_ERROR_OPENING_FILE*/)
	{
		doc->ClearError();
		doc = CreateDefaultConfig(doc);
		w_full_path = path;
		w_full_path = w_full_path + TEXT("\\") + DEF_CONF_DIR;
		CreateDirectory(w_full_path.c_str(), 0);
	};
	// TODO: Log operation

	saved = doc->SaveFile();
	if (saved)
		m_doc = doc;
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