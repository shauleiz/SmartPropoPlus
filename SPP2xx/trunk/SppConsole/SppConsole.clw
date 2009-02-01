; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CSppConsoleDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "sppconsole.h"
LastPage=0

ClassCount=5
Class1=CAboutDlg
Class2=CSppConsoleApp
Class3=CSppConsoleDlg

ResourceCount=5
Resource1=IDD_ABOUT
Resource2=IDD_DEF_VOL
Class4=CTouchPad
Resource3=IDD_SPPCONSOLE_DIALOG
Resource4=IDR_MENU_TEST
Class5=CDefVolumeDlg
Resource5=IDR_MENU_MAIN

[CLS:CSppConsoleApp]
Type=0
BaseClass=CWinApp
HeaderFile=SppConsole.h
ImplementationFile=SppConsole.cpp
Filter=N
VirtualFilter=AC

[CLS:CSppConsoleDlg]
Type=0
BaseClass=CDialog
HeaderFile=SppConsoleDlg.h
ImplementationFile=SppConsoleDlg.cpp
Filter=C
VirtualFilter=dWC
LastObject=IDOK

[DLG:IDD_SPPCONSOLE_DIALOG]
Type=1
Class=CSppConsoleDlg
ControlCount=14
Control1=IDOK,button,1342242816
Control2=IDC_MOD_TYPE,listbox,1352728833
Control3=IDC_SHIFT_POS,button,1342308361
Control4=IDC_SHIFT_NEG,button,1342177289
Control5=IDC_SHIFT_AUTO,button,1342242819
Control6=IDC_AUDIO_SRC,listbox,1352728833
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,button,1342177287
Control10=IDC_HIDE,button,1342242816
Control11=IDC_STATIC,button,1342177287
Control12=IDC_MIXERDEVICE,combobox,1344340226
Control13=IDC_ENABLE_AUDIO,button,1342242819
Control14=IDC_SPK,static,1342177550

[MNU:IDR_MENU_TEST]
Type=1
Class=CSppConsoleDlg
Command1=IDC_SHOW
Command2=IDC_HIDE
Command3=IDC_ABOUT
Command4=IDOK
CommandCount=4

[CLS:CAboutDlg]
Type=0
HeaderFile=AboutDlg.h
ImplementationFile=AboutDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_WEB_LINK
VirtualFilter=dWC

[DLG:IDD_ABOUT]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342177283
Control3=IDC_WEB_LINK,static,1342308352
Control4=IDC_TTL_1,static,1342308865
Control5=IDC_TTL_2,static,1342312448

[CLS:CTouchPad]
Type=0
HeaderFile=TouchPad.h
ImplementationFile=TouchPad.cpp
BaseClass=CStatic
Filter=W
VirtualFilter=WC

[MNU:IDR_MENU_MAIN]
Type=1
Class=CSppConsoleDlg
Command1=IDC_HIDE
Command2=IDOK
Command3=IDC_SEL_SRC
Command4=IDC_AUDIO_DST
Command5=IDC_AUDIO_VOL
Command6=IDC_AUDIO_PRINT
CommandCount=6

[CLS:CDefVolumeDlg]
Type=0
HeaderFile=DefVolumeDlg.h
ImplementationFile=DefVolumeDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDOK
VirtualFilter=dWC

[DLG:IDD_DEF_VOL]
Type=1
Class=CDefVolumeDlg
ControlCount=14
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_SLIDER_MIC,msctls_trackbar32,1342242825
Control4=IDC_EDT_MIC,edit,1350639744
Control5=IDC_STATIC,button,1342177287
Control6=IDC_SLIDER_LIN,msctls_trackbar32,1342242825
Control7=IDC_EDT_LIN,edit,1350639744
Control8=IDC_STATIC,button,1342177287
Control9=IDC_SLIDER_AUX,msctls_trackbar32,1342242825
Control10=IDC_EDT_AUX,edit,1350639744
Control11=IDC_STATIC,button,1342177287
Control12=IDC_SLIDER_ANL,msctls_trackbar32,1342242825
Control13=IDC_EDT_ANL,edit,1350639744
Control14=IDC_STATIC,button,1342177287

