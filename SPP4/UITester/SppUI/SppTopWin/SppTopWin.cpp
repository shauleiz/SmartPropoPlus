// This is the main DLL file.

#include "stdafx.h"
using namespace System;
//using namespace System::Xml;
using namespace System::Xaml;
using namespace System::Windows;
using namespace System::Windows::Documents;
using namespace System::Threading;
using namespace System::Windows::Controls;
using namespace System::Windows::Media;

using namespace System::Windows::Interop;
using namespace System::Threading;
using namespace System::Runtime::InteropServices; // Marshal
//using namespace WpfClockNS;
//using namespace DatabindingExample;

#include "uxtheme.h"
#include <stdio.h>
#include <string>
#using <mscorlib.dll>
#include <msclr/marshal.h>
#include <msclr\auto_gcroot.h>

#include "SppTopWin.h"

HWND SPPTOPWIN_API GetTestHiddenWnd(void)
{return NULL;} // TODO: Implement

int SPPTOPWIN_API MyEntryPoint(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow,
					 _In_opt_ HWND		hTopAppWnd)
{return 0;} // TODO: Implement