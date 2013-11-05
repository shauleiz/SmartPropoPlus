// SPP4 top UI window
// This is the entry point for the communication between the UI and the CU
// It is implemented as a (hidden) Win32 window DLL
// Created once for SPP4 session - Creates the needed WPF/Win32 child windows


#include "stdafx.h"
using namespace System;
using namespace System::Xaml;
using namespace System::Windows;
using namespace System::Windows::Documents;
using namespace System::Threading;
using namespace System::Windows::Controls;
using namespace System::Windows::Media;

using namespace System::Windows::Interop;
using namespace System::Threading;
using namespace System::Runtime::InteropServices; // Marshal
using namespace WpfCtrlWin;
//using namespace DatabindingExample;

#include "uxtheme.h"
#include <stdio.h>
#include <string>
#using <mscorlib.dll>
#include <msclr/marshal.h>
#include <msclr\auto_gcroot.h>

#include "SppTopWin.h"


// Global Variables:
HINSTANCE hInst;														// current instance
TCHAR szTitleTest[MAX_LOADSTRING] = L"Top UI Title";					// The title bar text
TCHAR szWindowClassTest[MAX_LOADSTRING] = L"TopUiWindowClass";			// the main window class name
HWND hTopWnd = 0;
HWND hAppWin = 0;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitTopWinInstance(HINSTANCE, int);
LRESULT CALLBACK	TopWinWndProc(HWND, UINT, WPARAM, LPARAM);
void				StartTopWinThread(Object^ data);
static void			GetHwnd(Object^ data);

public ref class MainPage : Window
{
};

public ref class WPFPageHost
{
public:
	WPFPageHost() {};
	static WpfCtrlWin::CtrlWindow^ hostedPage;
	static WindowInteropHelper^ ctrlhelper;
};


class ControlWrapperPrivate
{
    public: msclr::auto_gcroot<WpfCtrlWin::CtrlWindow^> CtrlWindow;
};

ControlWrapperPrivate* privateCtrl;


// Get the handle to the Top UI Window
HWND SPPTOPWIN_API GetTopUiWnd(void)
{
	return hTopWnd;
}

// Entry point to the Top UI Window DLL
// It runs in the application thread
// Starts the Top UI Window thread and returns
int SPPTOPWIN_API MyEntryPoint(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow,
					 _In_opt_ HWND		hTopAppWnd)
{

	Thread^ thCtrlWin;

	hInst = hInstance; // Store instance handle in our global variable
	hAppWin = hTopAppWnd;


	// Starting independent thread for top UI window
	thCtrlWin = gcnew Thread(gcnew ParameterizedThreadStart(&StartTopWinThread));
	thCtrlWin->Name = "Top SPP UI Thread";
	thCtrlWin->SetApartmentState(System::Threading::ApartmentState::STA);
	thCtrlWin->Start();

	return  0;
}

// Starting point for the Top UI Window thread
// Initializes the Top UI Window (Hidden)
// Starts the child windows (WPF)
// Gets into the message loop for the Top UI Window
void StartTopWinThread(Object^ data)
{
	::MSG lmsg;
	int nCmdShow = SW_SHOW; // TODO: Change to SW_HIDE

	MyRegisterClass(hInst);

	// Perform application initialization:
	if (!InitTopWinInstance (hInst, nCmdShow))
		return;
		
	privateCtrl = new ControlWrapperPrivate();
	privateCtrl->CtrlWindow = gcnew WpfCtrlWin::CtrlWindow();

	// Top UI Window message loop:
	while (GetMessage(&lmsg, NULL, 0, 0))
	{
		TranslateMessage(&lmsg);
		DispatchMessage(&lmsg);
	}

	return;
}

// Create the Top UI Window
// Initialize the handle to this window
BOOL InitTopWinInstance(HINSTANCE hInstance, int nCmdShow)
{

   hTopWnd = CreateWindow(szWindowClassTest, szTitleTest, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hTopWnd)
   {
      return FALSE;
   }

   ShowWindow(hTopWnd, nCmdShow);
   UpdateWindow(hTopWnd);

   return TRUE;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= TopWinWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= 0;//LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTUI));
	wcex.hCursor		= 0;//LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;//MAKEINTRESOURCE(IDC_TESTUI);
	wcex.lpszClassName	= szWindowClassTest;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, 0/*MAKEINTRESOURCE(IDI_SMALL)*/);

	return RegisterClassEx(&wcex);
}

//
//  FUNCTION: TopWinWndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK TopWinWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
        case WM_CREATE: 
        // Initialize the window. 
		hTopWnd = hWnd;
		GetHwnd(nullptr);
        return 0; 

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_EXIT:
			WPFPageHost::hostedPage->Hide();
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		DestroyWindow(hWnd);
		break;
	case 12347:
		//_private->Clock->Set_YYY();
		//_bindingwin->MainPage->Set_YYY(gcnew System::String("12345"));
		//WPFPageHost::hostedPage->Set_YYY(gcnew System::String("12345"));
		//title = (const char *)Marshal::StringToHGlobalAnsi(WPFPageHost::hostedPage->title).ToPointer();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

static void GetHwnd(Object^ data)
{
	WpfCtrlWin::CtrlWindow^ ctrlpage =  gcnew WpfCtrlWin::CtrlWindow();
	WPFPageHost::hostedPage = ctrlpage;
	WindowInteropHelper^ ctrlhelper = gcnew WindowInteropHelper(ctrlpage);
	WPFPageHost::ctrlhelper = ctrlhelper;

	ctrlhelper->Owner = (IntPtr)hTopWnd;
	ctrlpage->Show();
}
