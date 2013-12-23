#pragma once

#include "resource.h"

#define AUTOCHANNEL	0x0001
#define AUTOBITRATE	0x0002

	const COLORREF red        = RGB(0xFF, 0x00, 0x00);
	const COLORREF green      = RGB(0x00, 0xFF, 0x00);
	const COLORREF blue       = RGB(0x00, 0x00, 0xFF);
	const COLORREF white      = RGB(0xFF, 0xFF, 0xFF);
	const COLORREF black      = RGB(0x00, 0x00, 0x00);
	const COLORREF yellow     = RGB(0xFF, 0xFF, 0x00);
	const COLORREF cyan       = RGB(0x00, 0xFF, 0xFF); //
	const COLORREF magenta    = RGB(0xFF, 0x00, 0xFF); //
	const COLORREF brown      = RGB(0xA5, 0x2A, 0x2A);
	const COLORREF gray       = RGB(0x80, 0x80, 0x80);
	const COLORREF orange     = RGB(0xFF, 0xA5, 0x00);
	const COLORREF tomato     = RGB(0xFF, 0x63, 0x47);
	const COLORREF silver     = RGB(0xC0, 0xC0, 0xC0);
	const COLORREF pink       = RGB(0xFF, 0xC0, 0xCB);
	const COLORREF gold       = RGB(0xFF, 0xD7, 0x00);
	const COLORREF skyblue    = RGB(0x87, 0xCE, 0xEB);	

// Declarations
struct jack_info
{
	/* Inter-unit information about jack/capture endpoint information */
	int	struct_size;
	WCHAR * id;
	COLORREF color;
	WCHAR * FriendlyName;
	//bool	Enabled;
	bool	Default;
	int		nChannels;
};

/// Channel to Button/Axis mapping
#define MAX_BUTTONS		128
typedef std::array<BYTE, MAX_BUTTONS> BTNArr;
struct Mapping {
	UINT nAxes;
	DWORD * pAxisMap;
	UINT nButtons;
	BTNArr * ButtonArray;
};

// Control (Buttons/Axes) mask (availability)
struct controls
{
	UINT nButtons;	// Number of buttons
	BOOL axis[8];	// Axis-mask: axis[0] indicates if X exists ... axis[7] refers to SL1
};

struct MOD {
	LPCTSTR Type;		// Unique identifier of the modulation. Examples are PPMW, AIR1 …
	LPCTSTR Subtype;	// PPM/PCM
	LPCTSTR Name;		// User friendly name of the modulation to be desplayd.
	LPVOID func;		// This is the function of type PP that implements the modulation. Called by ProcessWave().
};

#define MOD_TYPE_PPM	_T("PPM")
#define MOD_TYPE_PPMP	_T("PPMPOS")
#define MOD_TYPE_PPMN	_T("PPMNEG")
#define MOD_TYPE_PPMW	_T("PPMW")
#define MOD_TYPE_JR 	_T("JR")
#define MOD_TYPE_FUT	_T("FUT")
#define MOD_TYPE_AIR1	_T("AIR1")
#define MOD_TYPE_AIR2	_T("AIR2")
#define MOD_TYPE_WAL	_T("WAL")

#define MOD_NAME_PPM	_T("PPM (Generic)")
#define MOD_NAME_PPMP	_T("PPM (Positive)")
#define MOD_NAME_PPMN	_T("PPM (Negative)")
#define MOD_NAME_PPMW	_T("PPM (Walkera)")
#define MOD_NAME_JR 	_T("JR (PCM)")
#define MOD_NAME_FUT	_T("Futaba (PCM)")
#define MOD_NAME_AIR1	_T("Sanwa/Air (PCM1)")
#define MOD_NAME_AIR2	_T("Sanwa/Air (PCM2)")
#define MOD_NAME_WAL	_T("Walkera (PCM)")


#define MOD_DEF_STR  {\
						MOD_TYPE_PPM, MOD_NAME_PPM,\
						MOD_TYPE_PPMP, MOD_NAME_PPMP,\
						MOD_TYPE_PPMN, MOD_NAME_PPMN,\
						MOD_TYPE_PPMW, MOD_NAME_PPMW,\
						MOD_TYPE_JR,  MOD_NAME_JR,\
						MOD_TYPE_FUT, MOD_NAME_FUT,\
						MOD_TYPE_AIR1,MOD_NAME_AIR1,\
						MOD_TYPE_AIR2,MOD_NAME_AIR2,\
						MOD_TYPE_WAL,MOD_NAME_WAL,\
						NULL}
