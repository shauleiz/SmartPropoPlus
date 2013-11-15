#pragma once

#include "resource.h"

#define AUTOCHANNEL	0x0001
#define AUTOBITRATE	0x0002

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
