; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CAboutDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "sppconsole.h"
LastPage=0

ClassCount=4
Class1=CMainDlg
Class2=CSppConsoleApp
Class3=CSppConsoleDlg

ResourceCount=3
Resource1=IDD_ABOUT
Resource2=IDD_SPPCONSOLE_DIALOG
Class4=CAboutDlg
Resource3=IDR_MENU_TEST

[CLS:CMainDlg]
Type=0
BaseClass=CDialog
HeaderFile=MainDlg.h
ImplementationFile=MainDlg.cpp
LastObject=CMainDlg
Filter=D
VirtualFilter=dWC

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
LastObject=IDC_ABOUT

[DLG:IDD_SPPCONSOLE_DIALOG]
Type=1
Class=CMainDlg
ControlCount=11
Control1=IDOK,button,1342242816
Control2=IDC_MOD_TYPE,listbox,1352728833
Control3=IDC_SHIFT_POS,button,1342308361
Control4=IDC_SHIFT_NEG,button,1342177289
Control5=IDC_SHIFT_AUTO,button,1342242819
Control6=IDC_AUDIO_SRC,listbox,1486946561
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1476526080
Control10=IDC_STATIC,button,1342177287
Control11=IDC_HIDE,button,1342242816

[MNU:IDR_MENU_TEST]
Type=1
Class=CSppConsoleDlg
Command1=IDC_SHOW
Command2=IDC_HIDE
Command3=IDC_ABOUT
Command4=IDOK
CommandCount=4

[DLG:IDD_ABOUT]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,static,1342177283
Control3=IDC_WEB_LINK,static,1342308352
Control4=IDC_TTL_1,static,1342308865
Control5=IDC_TTL_2,static,1342312448

[CLS:CAboutDlg]
Type=0
HeaderFile=AboutDlg.h
ImplementationFile=AboutDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_CUSTOM1
VirtualFilter=dWC

