#pragma once

#include "resource.h"

UINT const WMAPP_NOTIFYCALLBACK = WM_APP + 1;	// Message from the system tray
UINT const WMAPP_SM_INIT = WM_APP + 2;			// State machine initialized

void NotificationGraphics(HWND hWnd, DWORD_PTR state);
