; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "SppConsole.h"

ClassCount=3
Class1=CSppConsoleApp
Class2=CSppConsoleDlg

ResourceCount=2
Resource1=IDR_MAINFRAME
Class3=CMainDlg
Resource2=IDD_SPPCONSOLE_DIALOG

[CLS:CSppConsoleApp]
Type=0
HeaderFile=SppConsole.h
ImplementationFile=SppConsole.cpp
Filter=N
LastObject=CSppConsoleApp

[CLS:CSppConsoleDlg]
Type=0
HeaderFile=SppConsoleDlg.h
ImplementationFile=SppConsoleDlg.cpp
Filter=D
LastObject=IDC_MOD_TYPE
BaseClass=CDialog
VirtualFilter=dWC



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
Control7=IDC_HIDE,button,1476460547
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1476526080
Control11=IDC_STATIC,button,1342177287

[CLS:CMainDlg]
Type=0
HeaderFile=MainDlg.h
ImplementationFile=MainDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_MOD_TYPE

