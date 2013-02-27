/**************************************************************
	SmartPropoPlus Configuration class
	Interface of TinyXML
	
	An object from this class represents a configuration XML
**************************************************************/
#include "StdAfx.h"
#include "Config.h"

CConfig::CConfig(void) : 
						m_hRoot(0),
						m_hDoc(0)
{
	//m_hRoot=TiXmlHandle(NULL);
	//m_hDoc=TiXmlHandle(NULL);
}

CConfig::~CConfig(void)
{
	delete(m_XmlPath);
}

bool CConfig::LoadConfigFile(LPCWSTR FileName)
{
	/* 
		Load XML configuration file by name
		Updates class members:
		m_hDoc - Handle to document
		m_hRoot - Handle to root element <SmartPropoPlus>
		m_XmlPath - Path to XML config file

		Returns true if succeeds

	*/
	FILE * f;
	TiXmlElement* pElem;

	// Open XML file
	errno_t err = _wfopen_s(&f, FileName, L"r+b, ccs=UTF-8");
	if (err)
		return false;

	// Open & load XML file with TunyXML 
	TiXmlDocument * doc = new(TiXmlDocument);
	bool loaded = doc->LoadFile(f, TIXML_ENCODING_UTF8);
	if (!loaded)
	{
		const char * desc = doc->ErrorDesc();
		return false;
	};

	m_hDoc=TiXmlHandle(doc);

	pElem=m_hDoc.FirstChildElement().Element();
	// should always have a valid root but handle gracefully if it does
	if (!pElem) 
	{
		return false;
	};

	// Rootname is expected to be "SmartPropoPlus"
	const char * root_name = pElem->Value();
	if(!root_name)
	{
		return false;
	};

	if (!IsIdentical(XML_ROOT_ELEM, root_name))
	{
		return false;
	};


	// OK - Rootname is indeed "SmartPropoPlus" 
	m_hRoot=TiXmlHandle(pElem);
	m_XmlPath = _wcsdup(FileName);
	return true;
}

/* Capture EndPoint related methods:

	ExistCaptureEP(): Test is there's at least one <CaptureEndPoint> entry
	GetCaptureEPbyIndex(): Get one Capture EndPoint structure by index. Return true is success
	GetFirstCaptureEP(): Equivalent to GetCaptureEPbyIndex() with index=0. Return true is success
	GetSelectedCaptureEP(): Get the selected Capture EndPoint (Current or Original). Return true is success
*/
bool CConfig::ExistCaptureEP()
{
	// Test if exists at least one CaptureEndPoint element in the XML
	TiXmlElement * pElem=m_hRoot.FirstChild(XML_AUDIO_ELEM).FirstChild(XML_CEP_ELEM).Element();

	if (pElem)
		return true;
	else
		return false;
}

bool CConfig::GetFirstCaptureEP(const CaptureEndPoint * data)
{
	// GetFirstCaptureEP(): Equivalent to GetCaptureEPbyIndex() with index=0. Return true is success
	TiXmlElement * pElem=m_hRoot.FirstChild(XML_AUDIO_ELEM).FirstChild(XML_CEP_ELEM).Element();
	if (!pElem)
		return false;

	return GetCaptureEP(pElem, data);
}

bool CConfig::GetCaptureEPbyIndex(const CaptureEndPoint * data, int index)
{
	// GetCaptureEPbyIndex(): Get one Capture EndPoint structure by index. 
	// Index is 0 based
	// Return true is success
	TiXmlElement * pElem=m_hRoot.FirstChild(XML_AUDIO_ELEM).Child(XML_CEP_ELEM, index).Element();
	if (!pElem)
		return false;

	return GetCaptureEP(pElem, data);
}
bool CConfig::GetSelectedCaptureEP(const CaptureEndPoint * data, bool Current)
{
	// GetSelectedCaptureEP(): Get the selected Capture EndPoint
	// The Capture EndPoint is either original or current according to value of parameter Current
	// Return true is success
	const char * target = XML_ORIG_ELEM;
	if (Current)
		target = XML_CURR_ELEM;

	TiXmlElement * pElem=m_hRoot.FirstChild(XML_AUDIO_ELEM).FirstChild(XML_SELEP_ELEM).Element();
	if (!pElem)
		return false;

	// Original/Current
	if (pElem->FirstChildElement(target))
	{
		return GetCaptureEPbyId(data, pElem->FirstChildElement(target)->GetText());
	}
	else
		return false;

}

bool CConfig::GetCaptureEPbyId(const CaptureEndPoint * data, const char * id)
{
	int index=0;
	errno_t err;

	while(GetCaptureEPbyIndex(data, index++))
	{
		err = strcmp((char *)id, data->id);
		if (!err)
			return true;
	}
	return false;
}

bool CConfig::GetCaptureEP(TiXmlElement * pElem, const CaptureEndPoint * data)
{
	const char *pName;


	// id
	if (pElem->FirstChildElement(XML_ID_ELEM)) 
		strcpy_s((char *)(data->id), sizeof(data->id), pElem->FirstChildElement(XML_ID_ELEM)->GetText());
	else
		return false;

	 // jack colour
	if (pElem->FirstChildElement(XML_COLOR_ELEM))
		strcpy_s((char *)(data->color), sizeof(data->color), pElem->FirstChildElement(XML_COLOR_ELEM)->GetText());
	else
		return false;

	// Friendly Name
	if (pElem->FirstChildElement(XML_FNAME_ELEM))
		strcpy_s((char *)(data->friendlyName), sizeof(data->friendlyName), pElem->FirstChildElement(XML_FNAME_ELEM)->GetText()); 
	else
		return false;

	// Current State
	if (pElem->FirstChildElement(XML_CSTAT_ELEM))
	{
		// Current State: Enable = Yes/No
		pName = pElem->FirstChildElement(XML_CSTAT_ELEM)->Attribute(XML_ENBLD_ELEM);
		if (pName)
			strcpy_s((char *)(data->currentState.Enabled), sizeof(data->currentState.Enabled),pName);
		else
			strcpy_s((char *)(data->currentState.Enabled), sizeof(data->currentState.Enabled),"");


		// Current State: Channels
		pName = pElem->FirstChildElement(XML_CSTAT_ELEM)->Attribute(XML_CHNLS_ELEM);
		if (pName)
			strcpy_s((char *)(data->currentState.Channels), sizeof(data->currentState.Channels),pName);
		else
			strcpy_s((char *)(data->currentState.Channels), sizeof(data->currentState.Channels),"");
	};

	return true;
}

/*	Decoders section in XML file

	ExistDecoder(): Test is there's at least one <Decoder> entry
	GetDecoderbyIndex(): Get one decoder structure by index. Return true is success
	GetSelectedDecoder(): Get the selected Decoder. Return true is success
*/
bool CConfig::GetDecoder(TiXmlElement * pElem, const Decoder * data)
{
	// id
	if (pElem->FirstChildElement(XML_ID_ELEM)) 
		strcpy_s((char *)(data->id), sizeof(data->id), pElem->FirstChildElement(XML_ID_ELEM)->GetText());
	else
		return false;

	// Friendly Name
	if (pElem->FirstChildElement(XML_FNAME_ELEM))
		strcpy_s((char *)(data->friendlyName), sizeof(data->friendlyName), pElem->FirstChildElement(XML_FNAME_ELEM)->GetText()); 
	else
		return false;

	// Version
	if (pElem->FirstChildElement(XML_VER_ELEM))
		strcpy_s((char *)(data->ver), sizeof(data->ver), pElem->FirstChildElement(XML_VER_ELEM)->GetText()); 
	else
		return false;

	return true;
}


bool CConfig::ExistDecoder()
{
	// Test if exists at least one CaptureEndPoint element in the XML
	TiXmlElement * pElem=m_hRoot.FirstChild(XML_DCDRS_ELEM).FirstChild(XML_DCDR_ELEM).Element();

	if (pElem)
		return true;
	else
		return false;
}

bool CConfig::GetDecoderbyIndex(const Decoder * data, int index)
{
	// GetDecoderbyIndex(): Get one decoder structure by index. 
	// Index is 0 based
	// Return true is success
	TiXmlElement * pElem=m_hRoot.FirstChild(XML_DCDRS_ELEM).Child(XML_DCDR_ELEM, index).Element();
	if (!pElem)
		return false;

	return GetDecoder(pElem, data);
}
bool CConfig::GetDecoderbyId(const Decoder * data, const char * id)
{
	int index=0;
	errno_t err;

	while(GetDecoderbyIndex(data, index++))
	{
		err = strcmp((char *)id, data->id);
		if (!err)
			return true;
	}
	return false;
}

bool CConfig::GetSelectedDecoder(const Decoder * data)
{
	// GetSelectedDecoder(): Get the selected Decoder
	// Return true is success

	TiXmlElement * pElem=m_hRoot.FirstChild(XML_DCDRS_ELEM).FirstChild("Selected").Element();
	if (!pElem)
		return false;

	if (pElem)
	{
		return GetDecoderbyId(data, pElem->GetText());
	}
	else
		return false;

}


/*
	Helper functions
*/
bool CConfig::IsIdentical(LPCWSTR wStr, const char * utf8)
{
	if (!wStr || !utf8)
		return false;

	WCHAR w_utf8[MAX_PATH];
	int mb = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w_utf8,0); 
	mb = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, w_utf8,mb); 
	DWORD lasterr = GetLastError();
	if (lasterr)
		return false;

	if (wcscmp(wStr, w_utf8))
		return false;
	else
		return true;
}
bool CConfig::IsIdentical(const char * utf8, LPCWSTR wStr)
{
	return IsIdentical(wStr, utf8);
}