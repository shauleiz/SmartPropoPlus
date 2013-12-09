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

