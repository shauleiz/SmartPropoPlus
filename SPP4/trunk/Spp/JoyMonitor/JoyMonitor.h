#pragma once

#include "resource.h"

#ifdef X64
#ifdef _DEBUG
#pragma  comment(lib, "..\\x64\\Debug\\vJoyInterface.lib")
#pragma  comment(lib, "..\\x64\\Debug\\vJoyMonitor.lib")
#else
#pragma  comment(lib, "..\\x64\\Release\\vJoyInterface.lib")
#pragma  comment(lib, "..\\x64\\Release\\vJoyMonitor.lib")
#endif
#else
#ifdef _DEBUG
#pragma  comment(lib, "..\\Debug\\vJoyInterface.lib")
#pragma  comment(lib, "..\\Debug\\vJoyMonitor.lib")
#else
#pragma  comment(lib, "..\\Release\\vJoyInterface.lib")
#pragma  comment(lib, "..\\Release\\vJoyMonitor.lib")
#endif
#endif
