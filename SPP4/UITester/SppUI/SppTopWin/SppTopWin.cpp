// SPP4 top UI window
// This is the entry point for the communication between the UI and the CU
// It is implemented as a (hidden) Win32 window DLL
// Created once for SPP4 session - Creates the needed WPF/Win32 child windows


#include "stdafx.h"
using namespace System;
//using namespace System::Xaml;
using namespace System::Windows;
using namespace System::Windows::Documents;
using namespace System::Threading;
using namespace System::Windows::Controls;
using namespace System::Windows::Media;

using namespace System::Windows::Interop;
using namespace System::Threading;
using namespace System::Runtime::InteropServices; // Marshal
using namespace CtrlWindowNS;

#include "uxtheme.h"
#include <stdio.h>
#include <string>
#using <mscorlib.dll>
#include <msclr/marshal.h>
#include <msclr\auto_gcroot.h>

#include "../Tester/WinMessages.h" // TODO - Replace
#include "../Tester/Tester.h" // TODO - Replace
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
void				AddLine2AudioList(jack_info * jack);
void				AudioChannelParams(UINT Bitrate, WCHAR Channel);
void				AudioAutoParams(WORD Mask, WORD Flags);
void				DisplayAudioLevels(PVOID Id, UINT Left, UINT Right);
void				vJoyDevSetAvail(UINT id, controls * ctrl);
void				SetButtonValues(UINT id, BTNArr * BtnVals);
public ref class CtrlWindow : Window
{
//public:
//	delegate void WinMoving(double Left, double Top);
//	event WinMoving ^OnMove;
};

public ref class WPFPageHost
{
public:
	WPFPageHost() {};
	static CtrlWindowNS::CtrlWindow^ hostedPage;
	static WindowInteropHelper^ ctrlhelper;
};


class ControlWrapperPrivate
{
    public: msclr::auto_gcroot<CtrlWindowNS::CtrlWindow^> CtrlWindow;
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
	int nCmdShow = SW_HIDE; 

	MyRegisterClass(hInst);

	// Perform application initialization:
	if (!InitTopWinInstance (hInst, nCmdShow))
		return;
		
	privateCtrl = new ControlWrapperPrivate();
	privateCtrl->CtrlWindow = gcnew CtrlWindowNS::CtrlWindow();

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

	// Audio related messages
	case REM_ALL_JACK:
		WPFPageHost::hostedPage->CleanAudioList();
		break;

	case POPULATE_JACKS:
		AddLine2AudioList((jack_info *)(wParam));
		break;

	case SET_AUDIO_PARAMS:
		AudioChannelParams((UINT)wParam, (WCHAR)lParam);
		break;
		
	case SET_AUDIO_AUTO:
		AudioAutoParams((UINT)wParam, (WCHAR)lParam);
		break;

	case VJOYDEV_CH_LEVEL:
		DisplayAudioLevels((PVOID)wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case VJOYDEV_REMALL:
		WPFPageHost::hostedPage->vJoyRemoveAll();
		break;

	case VJOYDEV_ADD:
		WPFPageHost::hostedPage->vJoyDevAdd((UINT)wParam);
		if (lParam)
			WPFPageHost::hostedPage->vJoyDevSelect((UINT)wParam);
		break;

	case VJOYDEV_SETAVAIL:
		vJoyDevSetAvail((UINT)wParam, (controls * )lParam);
		break;

	case WMSPP_JMON_AXIS:
		WPFPageHost::hostedPage->SetJoystickAxisData((UCHAR)(wParam&0xFF), (UINT)(wParam>>16), (UINT32)lParam);
		break;

	case WMSPP_JMON_BTN:
		SetButtonValues((UINT)wParam, (BTNArr *)lParam);
		break;

	//case 12347:
		//_private->Clock->Set_YYY();
		//_bindingwin->MainPage->Set_YYY(gcnew System::String("12345"));
		//WPFPageHost::hostedPage->Set_YYY(gcnew System::String("12345"));
		//title = (const char *)Marshal::StringToHGlobalAnsi(WPFPageHost::hostedPage->title).ToPointer();
		//break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void CtrlWindowMoving(double Left, double Top);
void CtrlAudioChanging(int bitrate, WCHAR channel);
void CtrlAudioAutoChanging(bool AutoBitrateChecked, bool AutoChannelChecked);
void CtrlvJoyDeviceChanging(UINT id);

static void GetHwnd(Object^ data)
{
	CtrlWindowNS::CtrlWindow^ ctrlpage =  gcnew CtrlWindowNS::CtrlWindow();
	WPFPageHost::hostedPage = ctrlpage;
	WindowInteropHelper^ ctrlhelper = gcnew WindowInteropHelper(ctrlpage);
	WPFPageHost::ctrlhelper = ctrlhelper;

	ctrlhelper->Owner = (IntPtr)hTopWnd;
	ctrlpage->Show();

	// Assign event handlers
	ctrlpage->OnMove += gcnew CtrlWindowNS::CtrlWindow::WinMoving(CtrlWindowMoving);
	ctrlpage->OnAudioChanged += gcnew CtrlWindowNS::CtrlWindow::AudioChanging(CtrlAudioChanging);
	ctrlpage->OnAudioAutoChanged += gcnew CtrlWindowNS::CtrlWindow::AudioAutoChanging(CtrlAudioAutoChanging);
	ctrlpage->OnvJoyDeviceChanged += gcnew CtrlWindowNS::CtrlWindow::vJoyDeviceChanging(CtrlvJoyDeviceChanging);
}

// Add line to the list of audio devices
// Mark the selected device
void AddLine2AudioList(jack_info * jack)
{
	WPFPageHost::hostedPage->Insert_Jack(gcnew System::String(jack->id), gcnew System::String(jack->FriendlyName), 0, 0, jack->Default, jack->color);
}

// Set the parameters of the audio (8/16 bits Left/Right/Mono)
// If Bitrate = 0 then don't change
// If Channel="" or Channel=NULL then don't change
void AudioChannelParams(UINT Bitrate, WCHAR Channel)
{
	WPFPageHost::hostedPage->_event->AudioBitrate = Bitrate;
	WPFPageHost::hostedPage->_event->AudioChannel = Channel;  //>Set_Channel(Channel);
}

// Display the audio levels of channels (Left/Right)
// Levels are in the range 0-100
void DisplayAudioLevels(PVOID Id, UINT Left, UINT Right)
{
	WPFPageHost::hostedPage->SetAudioLevels_Jack(gcnew System::String((LPCTSTR)Id), Left, Right);
}

void vJoyDevSetAvail(UINT id, controls * ctrl)
{
	const UINT nAxes = 8;
	Mcontrols^ Mcrtl = gcnew Mcontrols();
	array<int>^ Maxis = gcnew array<int>(nAxes);
	Mcrtl->nButtons = ctrl->nButtons;
	for (int i=0; i<nAxes; i++)
		Mcrtl->axis[i] = ctrl->axis[i];
	
	WPFPageHost::hostedPage->EnableControls(id, Mcrtl);
}

void SetButtonValues(UINT id, BTNArr * BtnVals)
{
	const UINT  nButons = 32;
	array<BYTE>^ MBtnVals = gcnew array<BYTE>(nButons);
	BYTE * _array_of_vals = BtnVals->data();

	WPFPageHost::hostedPage->SetButtonValues(id,  (IntPtr)_array_of_vals);
}

// TODO: Use this function to coordinate moving of windows
void CtrlWindowMoving(double Left, double Top)
{
}

void CtrlAudioChanging(int bitrate, WCHAR channel)
{
	// Convert default values
	if (!bitrate)
		bitrate = 8;
	if (channel == TEXT('U'))
		channel = TEXT('L');

	// Update CU
	SendMessage(hAppWin, WMSPP_DLG_CHNL, bitrate,  channel);
}

void AudioAutoParams(WORD Mask, WORD Flags)
{
	if (Mask&AUTOCHANNEL)
	{
		if (Flags&AUTOCHANNEL)
			WPFPageHost::hostedPage->_event->IsNotAutoChannel = false;
		else
			WPFPageHost::hostedPage->_event->IsNotAutoChannel = true;
	};

	if (Mask&AUTOBITRATE)
	{
		if (Flags&AUTOBITRATE)
			WPFPageHost::hostedPage->_event->IsNotAutoBitrate = false;
		else
			WPFPageHost::hostedPage->_event->IsNotAutoBitrate = true;
	};
}

void CtrlAudioAutoChanging(bool AutoBitrateChecked, bool AutoChannelChecked)
{
	WORD val=0;

	if (AutoBitrateChecked)
		val |= AUTOBITRATE;
	if (AutoChannelChecked)
		val |= AUTOCHANNEL;

	SendMessage(hAppWin, WMSPP_DLG_AUTO, AUTOBITRATE|AUTOCHANNEL, val);
}

void CtrlvJoyDeviceChanging(UINT id)
{
	SendMessage(hAppWin, WMSPP_DLG_VJOYSEL, (WPARAM)id, 0);
}
