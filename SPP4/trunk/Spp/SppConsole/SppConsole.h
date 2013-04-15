#pragma once

#include "resource.h"

#ifdef _DEBUG
#pragma  comment(lib, ".\\Debug\\SppMain.lib")
#else
#pragma  comment(lib, ".\\Release\\SppMain.lib")
#endif

static UINT NEAR WM_INTERSPPCONSOLE;
static UINT NEAR WM_INTERSPPAPPS;

