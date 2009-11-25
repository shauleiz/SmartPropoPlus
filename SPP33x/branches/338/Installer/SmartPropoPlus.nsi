;SmartPropoPlus Installation script
;Written by Shaul Eizikovich

;--------------------------------

!include "MUI.nsh" 		; Modern User Interface
;!include "Sections.nsh"

!include "FileFunc.nsh"	; File functions
!insertmacro GetParent

!include "WinVer.nsh" ; Window Version Functions

;--------------------------------
; Macro definitions

; Create a Windows shortcut to a URL
!macro CreateInternetShortcut FILENAME URL ICONFILE ICONINDEX
	WriteINIStr "${FILENAME}.url" "InternetShortcut" "URL" "${URL}"
	WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconFile" "${ICONFILE}"
	WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconIndex" "${ICONINDEX}"
!macroend

; Get the version number of a local file (Version format: a.b.c.d)
!macro GetLocalFileVer FILENAME VERSTR
	GetDLLVersionLocal ${FILENAME} $R0 $R1
	IntOp $R2 $R0 / 0x00010000
	IntOp $R3 $R0 & 0x0000FFFF
	IntOp $R4 $R1 / 0x00010000
	IntOp $R5 $R1 & 0x0000FFFF
	StrCpy ${VERSTR} "$R2.$R3.$R4.$R5"
!macroend

;--------------------------------
;General

   VAR  PPJoyExist		; Boolean: 1 if PPJoy installed
   VAR  FmsExist		; Boolean: 1 if FMS installed
   VAR  VERSION			; SPP DLL file version (3.3.3)
   VAR	FolderSPP4FMS		; Destination folder for SPP for FMS
   VAR	FolderGenSpp		; Destination folder for Generic SPP
   VAR	FolderUtils		; Destination folder for Utilities
   VAR  FolderFMS               ; FMS folder. "" If does not exist
   VAR	SubText			; Subtext for finishing page
   VAR	FinishText		; Text for finishing page
   VAR	RadioButtonStart	; Default Selected Section
   VAR	SPP4FMSremoved		; Boolean: 1 if SPP for FMS component was removed
   VAR	GenSPPremoved		; Boolean: 1 if Generic SPP component was removed
   VAR	Utilsremoved		; Boolean: 1 if Utilities component was removed
   VAR	RunAppOnFinish		; The application to run at the end of the installation
   VAR	RunAppOnFinishText	; The text nexr to the checkbox of the application to run
  
 ;Name and file
  Name "SmartPropoPlus"
  OutFile "InstallSPP.exe"
  !define MUI_ICON  "..\SppConsole\res\SppConsole.ico" /* Icon */
  !define MUI_UNICON  "UnInstaller.ico" /* Uninstall Icon */

  ;Default installation folder
  InstallDir "$PROGRAMFILES\FMS"  

  ; Combination of Section flags (SF_)
  !define	SF_SELBOLD	0x9			; SELECTED | BOLD
  !define	SF_RO_INV	0xFFEF		; NOT Read Only
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  !define MUI_COMPONENTSPAGE_NODESC	; No component description area - makes the type field much wider
  
  ; Make the FINISH page run SPP
  !define MUI_INSTFILESPAGE_FINISHHEADER_TEXT $FinishText
  !define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT $SubText
  !define MUI_FINISHPAGE_RUN $RunAppOnFinish
  !define MUI_FINISHPAGE_RUN_TEXT $RunAppOnFinishText

;--------------------------------
;Pages

  !define 		MUI_PAGE_CUSTOMFUNCTION_LEAVE  ComponentLeaveFunction
  !insertmacro	MUI_PAGE_COMPONENTS
  !insertmacro	MUI_PAGE_DIRECTORY
  !insertmacro	MUI_PAGE_INSTFILES
  !define 		MUI_PAGE_CUSTOMFUNCTION_PRE FinishPreFunction
  !insertmacro	MUI_PAGE_FINISH
  
 !insertmacro	MUI_UNPAGE_COMPONENTS
 !insertmacro	MUI_UNPAGE_INSTFILES
  

;--------------------------------
;Installation Types
  ;InstType "SmartPropoPlus for FMS"
  ;InstType "Generic SmartPropoPlus"
  ;InstType "un.SmartPropoPlus for FMS"
  ;InstType "un.Generic SmartPropoPlus"
  ;InstType /NOCUSTOM	; No 'custom' type
  ;InstType /COMPONENTSONLYONCUSTOM ; Components shown only for 'custom' type. Since 'custom' type does not exist - No Components field

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
 ; !insertmacro MUI_LANGUAGE "French"

;--------------------------------
;Vista: Execute as administrator
RequestExecutionLevel admin

;--------------------------------
;Installer Sections

;Section /o "!SmartPropoPlus for FMS" SPP4FMS
Section /o "!" SPP4FMS

  ;SectionIn 1
  DetailPrint "[I] Installing SmartPropoPlus for FMS" ; Debug
  SetShellVarContext all
  SetOutPath "$INSTDIR"
  

  ;SPP files
  	file ..\ReleaseNotes.pdf  
	file ..\..\msvcr71.dll 
	file ..\..\MFC71.dll
	file ..\..\AudioStudy\Release\AudioStudy.exe
	file ..\..\filters\JsChPostProc\Release\JsChPostProc.dll
	file ..\SppConsole\Release\SppConsole.exe
	file ..\winmm\Release\winmm.dll
	!insertmacro GetLocalFileVer "..\winmm\Release\winmm.dll" "$VERSION" ; DLL Version manipulation


  ; Start Menu
	CreateDirectory "$SMPROGRAMS\SmartPropoPlus"
	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\FMS.lnk" "$INSTDIR\FMS.exe"
	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\Release Notes.lnk" "$INSTDIR\ReleaseNotes.pdf"
	!insertmacro CreateInternetShortcut "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Web Site"  "http://www.smartpropoplus.com" "" ""
    
  ; Add uninstall information to Add/Remove Programs
  ; Standard
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayName" "SmartPropoPlus"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "UninstallString" "$INSTDIR\UninstallSPP.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "NoRepair" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "NoModify" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "URLInfoAbout" "http://www.SmartPropoPlus.com"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "Publisher" "SmartPropoPlus"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayVersion" $VERSION
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayIcon" "$INSTDIR\SppConsole.exe"
  
  ; Custom
  ; Enter the installation location of this section to enable later the uninstaller to remove only it
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderSPP4FMS" "$INSTDIR"
  
  ; Vista only: Run in XP-SP2 compatibility mode
  ${If} ${AtLeastWinVista}
  WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$OUTDIR\SppConsole.exe" "WINXPSP2"
  ${Endif}
  
  SetAutoClose false
  StrCpy $FinishText "Completing SmartPropoPlus (FMS Version)"
  StrCpy $SubText "SmartPropoPlus (FMS Version) has been installed on your computer"
  StrCpy $RunAppOnFinish "$INSTDIR\FMS.exe"
  StrCpy $RunAppOnFinishText "Start Flying Model Simulator (FMS)"

SectionEnd ;SPP4FMS


Section "!" GenSPP

  ;SectionIn 2
  DetailPrint "[I] Installing Generic SmartPropoPlus in $INSTDIR" ; Debug
  SetShellVarContext all
  SetOutPath "$INSTDIR"
 
  
  ;SPP files
  	file ..\ReleaseNotes.pdf  
  	file ..\..\msvcr71.dll 
  	file ..\..\MFC71.dll
  	file ..\..\AudioStudy\Release\AudioStudy.exe
  	file ..\..\filters\JsChPostProc\Release\JsChPostProc.dll
  	file ..\SppConsole\Release\SppConsole.exe
  	file ..\winmm\Release_PPJoy\PPJoyEx.dll
	!insertmacro GetLocalFileVer "..\winmm\Release_PPJoy\PPJoyEx.dll" "$VERSION" ; DLL Version manipulation


  ; Start Menu
	CreateDirectory "$SMPROGRAMS\SmartPropoPlus"
	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SppConsole.lnk" "$INSTDIR\SppConsole.exe"
	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\Release Notes.lnk" "$INSTDIR\ReleaseNotes.pdf"
	!insertmacro CreateInternetShortcut "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Web Site"  "http://www.smartpropoplus.com" "" ""
	
  ; Desktop 
  	CreateShortCut  "$DESKTOP\SppConsole.lnk" "$INSTDIR\SppConsole.exe"
  
  ; Add uninstall information to Add/Remove Programs
  ; Standard
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayName" "SmartPropoPlus"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "UninstallString" "$INSTDIR\UninstallSPP.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "NoRepair" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "NoModify" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "URLInfoAbout" "http://www.SmartPropoPlus.com"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "Publisher" "SmartPropoPlus"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayVersion" $VERSION
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayIcon" "$INSTDIR\SppConsole.exe"
  
  ; Custom
  ; Enter the installation location of this section to enable later the uninstaller to remove only it
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderGenSpp" "$INSTDIR"
  
  ; Vista only: Run in XP-SP2 compatibility mode
  ${If} ${AtLeastWinVista}
  WriteRegStr HKLM "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$OUTDIR\SppConsole.exe" "WINXPSP2"
  ${Endif}
  
  SetAutoClose false
  StrCpy $FinishText "Completing SmartPropoPlus (Generic Version)"
  StrCpy $SubText "SmartPropoPlus (Generic Version) has been installed on your computer"
  StrCpy $RunAppOnFinish "$INSTDIR\SppConsole.exe"
  StrCpy $RunAppOnFinishText "Start SmartPropoPlus Console"

SectionEnd ;GenSPP

Section /o "-Utilities" Utils_SPP4FMS
  ;SectionIn 1
  	; If utilities folder exist, use it
;	ReadRegStr $FolderUtils   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderUtils"
;  	StrLen $0 $FolderUtils
;  	IntCmp $0 0 AssFolder SkipAssFolder SkipAssFolder
;AssFolder:
	; Custom
	; Enter the installation location of this section to enable later the uninstaller to remove only it
	SetShellVarContext all
	StrCpy $FolderUtils "$INSTDIR\Utilities"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderUtils" "$FolderUtils"
	
;SkipAssFolder:
    ; Utilities
  	SetOutPath "$FolderUtils"
  	file "C:\Program Files\Osc\WinScope.*" 
   	file "C:\Program Files\Osc\RCAudio.exe" 
    file "C:\Program Files\Osc\AudPPMV.exe" 

    ; Start menu utility folder
	CreateDirectory "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities"
	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\Winscope.lnk" "$FolderUtils\Winscope.exe"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\RCAudio (PPM Thermometer).lnk" "$FolderUtils\RCAudio.exe"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\AudPPMV.lnk" "$FolderUtils\AudPPMV.exe"

 	
SectionEnd ; Utils_SPP4FMS

Section /o "-PPJoy Installation" PPJoy_Inst
  ;SectionIn 1
SectionEnd ; PPJoy_Inst


Section "-Create Uninstaller" CreateUninstSpp
  ;Create uninstaller
  SetShellVarContext all
  WriteUninstaller "$INSTDIR\UninstallSPP.exe"
  CreateShortCut "$SMPROGRAMS\SmartPropoPlus\Uninstall SmartPropoPlus.lnk" "$INSTDIR\UninstallSPP.exe"
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecSpp4Fms ${LANG_ENGLISH} "You intend to use SmartPropoPlus only with FMS"
  LangString DESC_SecGenSpp  ${LANG_ENGLISH} "You intend to use SmartPropoPlus with simulators other than FMS"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  	!insertmacro MUI_DESCRIPTION_TEXT ${SPP4FMS} $(DESC_SecSpp4Fms)
  	!insertmacro MUI_DESCRIPTION_TEXT ${GenSPP}  $(DESC_SecGenSpp)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
Section "un.SmartPropoPlus for FMS" UnSPP4FMS
;	Call un.GetInstallFolders
	SetShellVarContext all
	StrLen $0 $FolderSPP4FMS
	IntCmp $0 0 NoRemove Remove Remove

NoRemove:
	StrCpy $SPP4FMSremoved 1
	goto EndUninstSpp4Fms
Remove:
  DetailPrint "[I] Removing SmartPropoPlus for FMS files from $FolderSPP4FMS" ; Debug
; Remove files
  	Delete $FolderSPP4FMS\UninstallSPP.exe
    Delete $FolderSPP4FMS\ReleaseNotes.pdf  
	Delete $FolderSPP4FMS\msvcr71.dll 
	Delete $FolderSPP4FMS\MFC71.dll
	Delete $FolderSPP4FMS\AudioStudy.exe
	Delete $FolderSPP4FMS\JsChPostProc.dll
	Delete $FolderSPP4FMS\SppConsole.exe
	Delete $FolderSPP4FMS\winmm.dll
; Remove directory only if empty
  	RMDir $FolderSPP4FMS
  	
; Remove short cuts
 	Delete "$SMPROGRAMS\SmartPropoPlus\FMS.lnk"

; Remove registry value
	DeleteRegValue 	 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderSPP4FMS"
; 	DeleteRegValue	 HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$FolderSPP4FMS\SppConsole.exe"
 	DeleteRegValue	 HKLM "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$FolderSPP4FMS\SppConsole.exe"

	StrCpy $SPP4FMSremoved 1
	
StrCpy "$FolderUtils" "$FolderSPP4FMS\Utilities"
DetailPrint "[I] Removing Utility files from $FolderUtils" ; Debug
; Remove files
	Delete $FolderUtils\Winscope.*
	Delete $FolderUtils\RCAudio.exe
	Delete $FolderUtils\AudPPMV.exe
; Remove directory only if empty
  	RMDir  $FolderUtils
  	RMDir  $FolderSPP4FMS
 	  	
; Moove short cuts
	IfFileExists 	"$FolderGenSPP\Utilities" ReplaceUtils DeleteUtils
ReplaceUtils:
	DetailPrint "[I] Moving Utility file shortcuts to $FolderGenSPP\Utilities" ; Debug
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\Winscope.lnk" "$FolderGenSPP\Utilities\Winscope.exe"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\RCAudio (PPM Thermometer).lnk" "$FolderGenSPP\Utilities\RCAudio.exe"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\AudPPMV.lnk" "$FolderGenSPP\Utilities\AudPPMV.exe"
 	goto ReplaceCommon
DeleteUtils:
	DetailPrint "[I] Deleting Utilities shortcuts (SmartPropoPlus for FMS)" ; Debug
	Delete "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\*"
	RMDir "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities"
ReplaceCommon:
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\Release Notes.lnk" "$FolderGenSPP\ReleaseNotes.pdf"
 	CreateShortCut "$SMPROGRAMS\SmartPropoPlus\Uninstall SmartPropoPlus.lnk" "$FolderGenSPP\UninstallSPP.exe"
 	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "UninstallString" "$FolderGenSPP\UninstallSPP.exe"
  	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayIcon" "$FolderGenSPP\SppConsole.exe"


 	StrCpy $Utilsremoved 1
 	
EndUninstSpp4Fms:
SectionEnd ; UnSPP4FMS

Section "un.Generic SmartPropoPlus" UnGenSPP
;	Call un.GetInstallFolders
	SetShellVarContext all
	StrLen $0 $FolderGenSPP
	IntCmp $0 0 NoRemove Remove Remove

NoRemove:
	StrCpy $GenSPPremoved 1
	goto EndUninstGenSpp
Remove:
  DetailPrint "[I] Removing Generic SmartPropoPlus files from $FolderGenSpp" ; Debug
; Remove files
  	Delete $FolderGenSpp\UninstallSPP.exe
  	Delete $FolderGenSpp\ReleaseNotes.pdf  
  	Delete $FolderGenSpp\msvcr71.dll 
  	Delete $FolderGenSpp\MFC71.dll
  	Delete $FolderGenSpp\AudioStudy.exe
  	Delete $FolderGenSpp\JsChPostProc.dll
  	Delete $FolderGenSpp\SppConsole.exe
  	Delete $FolderGenSpp\PPJoyEx.dll 	
 	
; Remove short cuts
	Delete "$SMPROGRAMS\SmartPropoPlus\SppConsole.lnk"	; Remove SPPconsole from start menu
	Delete "$DESKTOP\SppConsole.lnk"					; Remove icon from desktop
	
; Remove registry value
	DeleteRegValue 	 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderGenSpp"
; 	DeleteRegValue	 HKCU "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$FolderGenSpp\SppConsole.exe"
 	DeleteRegValue	 HKLM "Software\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers" "$FolderGenSpp\SppConsole.exe"
	StrCpy $GenSPPremoved 1

StrCpy "$FolderUtils" "$FolderGenSpp\Utilities"
DetailPrint "[I] Removing Utility files from $FolderUtils" ; Debug
; Remove files
	Delete $FolderUtils\Winscope.*
	Delete $FolderUtils\RCAudio.exe
	Delete $FolderUtils\AudPPMV.exe
; Remove directory only if empty
  	RMDir  $FolderUtils
  	RMDir  $FolderGenSpp
 	  	
; Move short cuts
	IfFileExists 	"$FolderSPP4FMS\Utilities" ReplaceUtils DeleteUtils
ReplaceUtils:
	DetailPrint "[I] Moving Utility file shortcuts to $FolderSPP4FMS\Utilities" ; Debug
	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\Winscope.lnk" "$FolderSPP4FMS\Utilities\Winscope.exe"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\RCAudio (PPM Thermometer).lnk" "$FolderSPP4FMS\Utilities\RCAudio.exe"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\AudPPMV.lnk" "$FolderSPP4FMS\Utilities\AudPPMV.exe"
 	goto ReplaceCommon
DeleteUtils:
	DetailPrint "[I] Deleting Utilities shortcuts (Generic SmartPropoPlus)" ; Debug
	Delete "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\*"
 	RMDir "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities"
ReplaceCommon:
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\Release Notes.lnk" "$FolderSPP4FMS\ReleaseNotes.pdf"
 	CreateShortCut  "$SMPROGRAMS\SmartPropoPlus\Uninstall SmartPropoPlus.lnk" "$FolderSPP4FMS\UninstallSPP.exe"
 	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "UninstallString" "$FolderSPP4FMS\UninstallSPP.exe"
  	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "DisplayIcon" "$FolderSPP4FMS\SppConsole.exe"

 	StrCpy $Utilsremoved 1
	
	
EndUninstGenSpp:
SectionEnd ; UnGenSPP
/*
Section "-un.Utilities" UnUtils
;	Call un.GetInstallFolders
	StrLen $0 $FolderUtils
	IntCmp $0 0 NoRemove Remove Remove

NoRemove:
	StrCpy $Utilsremoved 1
	goto EndUninstUtils
Remove:
  DetailPrint "[I] Removing Utility files from $FolderUtils" ; Debug
; Remove files
	Delete $FolderUtils\Winscope.*
	Delete $FolderUtils\RCAudio.exe
	Delete $FolderUtils\AudPPMV.exe
; Remove directory only if empty
  	RMDir  $FolderUtils
 	
; Remove short cuts
	Delete "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\*"
 	RMDir "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities"
	
; Remove registry value
	DeleteRegValue 	 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderUtils"
	StrCpy $Utilsremoved 1

EndUninstUtils:
SectionEnd ; UnUtils
*/

Section "-un.install"

	SetShellVarContext all
  	IntCmp $Utilsremoved 1 TestGenSPP  EndUninst  EndUninst
TestGenSPP:
  	IntCmp $GenSPPremoved 1 TestSPP4FMS  EndUninst  EndUninst
TestSPP4FMS:
  	IntCmp $SPP4FMSremoved 1 CleanAll  EndUninst  EndUninst
  	
CleanAll:
; Delete files - SPP for FMS
	DetailPrint "[I] Removing uninstall files" ; Debug
  	Delete "$FolderSPP4FMS\UninstallSPP.exe"
  	Delete "$FolderGenSpp\UninstallSPP.exe"

; Clean the start menu
	DetailPrint "[I] Removing common links from Start menu" ; Debug
; Remove Utilities short cuts
	Delete "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities\*"
 	RMDir "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Utilities"
; Remove the rest
	Delete "$SMPROGRAMS\SmartPropoPlus\Uninstall SmartPropoPlus.lnk"
 	Delete "$SMPROGRAMS\SmartPropoPlus\Release Notes.lnk"
	Delete "$SMPROGRAMS\SmartPropoPlus\SmartPropoPlus Web Site.url"
	RMDir  "$SMPROGRAMS\SmartPropoPlus"

; Remove registry key
	DetailPrint "[I] Removing SmartPropoPlus entry from registry" ; Debug
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus"
  
EndUninst:
SectionEnd

;--------------------------------
;Callback functions

  Function .onInit


  	call isFmsInstalled
  	call GetFmsFolder
  	Call  isPPJoyInstalled
  	
  	${If} $FmsExist <> 0     ; FMS Exist?
          ${If} $FolderFMS >= "" ; Valid FMS Folder?
                   ; FMS is installed: Set installation directory, select FMS radio button set SPP4FMS
                   DetailPrint "[I] FMS is installed, FolderFMS is $FolderFMS"
                   StrCpy $INSTDIR $FolderFMS                                 ; Set installation directory
                   StrCpy $RadioButtonStart ${SPP4FMS}                        ; Select radio button default
                   SectionSetFlags ${SPP4FMS} ${SF_SELBOLD}		      ; Install SPP for FMS
  	           SectionSetFlags ${Utils_SPP4FMS} ${SF_SELECTED}	      ; Install Utilities
  	           SectionSetText ${SPP4FMS} "SmartPropoPlus for FMS"         ; Set the text next to the FMS check-box
  	           ${If} $PPJoyExists <> 0   ; PPJoy also exist?
  	                 ; PPJoy is installed in addition to FMS
  	                 SectionSetText ${GenSPP} "Generic SmartPropoPlus"    ; Set the text next to the Generic check-box
  	                 SectionSetFlags ${GenSPP} ${SF_BOLD}		      ; Display Generic un-selected
                   ${Else}
  	                 ; PPJoy is NOT installed in addition to FMS
  	                 SectionSetText ${GenSPP} "Generic SmartPropoPlus $\n(PPJoy Must be installed first)"          ; Set the text next to the Generic check-box
  	                 SectionSetFlags ${GenSPP} ${SF_RO}		      ; Display Generic un-selected
  	           ${EndIf}   ; PPJoy also exist?
  	           goto EndOfFunconInit
          ${EndIf} ; Valid FMS Folder?
  	${EndIf}               ; FMS Exist?

        ; FMS is not installed - Test for PPJoy installation
        ${If} $PPJoyExists <> 0   ; PPJoy  exist?
              ; PPJoy  is installed: Set installation directory, select Generic radio button set GENSPP
    	      StrCpy $INSTDIR "$PROGRAMFILES\SmartPropoPlus"            ; Set installation directory
              StrCpy $RadioButtonStart ${GENSPP}			; Select radio button default
  	      SectionSetFlags ${GENSPP} ${SF_SELBOLD}			; Install Generic SPP
  	      SectionSetFlags ${Utils_SPP4FMS} ${SF_SELECTED}	        ; Install Utilities
	      SectionSetText ${GenSPP} "Generic SmartPropoPlus"         ; Set the text next to the Generic check-box
	      ; FMS is not installed
	      SectionSetText ${SPP4FMS} "SmartPropoPlus for FMS $\n(FMS Must be installed first)"         ; Set the text next to the FMS check-box
	      SectionSetFlags ${SPP4FMS} ${SF_RO}		      ; Display SPP4FMS un-selected
        ${Else}   ; PPJoy  exist?
              MessageBox MB_ICONEXCLAMATION  "SmartPropoPlus cannot be installed$\r$\nBefore installing SmartPropoPlus you must install either PPJoy or FMS"
              abort
        ${EndIf}   ; PPJoy  exist?

        EndOfFunconInit:
        
  FunctionEnd
  
  
  Function .onSelChange
  
    VAR /Global Changed
    
    ; Detect type change
  	IntOp $Changed 0 + 0
 	SectionGetFlags ${SPP4FMS} $0
  	SectionGetFlags ${GenSPP} $2
  	IntOp $0 $0 & ${SF_SELECTED} ; Spp for FMS
  	IntOp $2 $2 & ${SF_SELECTED} ; Generic SPP
  	IntCmp $0 0 RadioButtons
  	IntCmp $2 0 RadioButtons
  	IntOp $Changed 0 + 1
  	
  	; Make two types of SPP mutually exclusive
  	RadioButtons:
	!insertmacro StartRadioButtons $RadioButtonStart
   		!insertmacro RadioButton ${SPP4FMS}
		!insertmacro RadioButton ${GenSPP}
	!insertmacro EndRadioButtons
 
  	; Get the type selected 	
 	SectionGetFlags ${SPP4FMS} $0
  	SectionGetFlags ${GenSPP} $2
  	IntOp $0 $0 & ${SF_SELECTED} ; Spp for FMS
  	IntOp $2 $2 & ${SF_SELECTED} ; Generic SPP 	
 	
  	; Determine the output folder
  	IntCmp $0 1 isFMS
  	IntCmp $2 1 isGen
  	;Nothing:
  		MessageBox MB_OK "Nothing selected - Aborting" ; Debug
  		DetailPrint "[MB] Nothing selected - Aborting"
  		StrCpy $INSTDIR  ""
  		goto Next1
  	isFMS:
  		StrCpy $INSTDIR "$PROGRAMFILES\FMS"
  		SectionSetFlags ${PPJoy_Inst}  ${SF_RO}
  		Goto Next1
  	isGen:
  		StrCpy $INSTDIR "$PROGRAMFILES\SmartPropoPlus"
  		SectionGetFlags ${PPJoy_Inst} $7
  		IntOp $7 $7 & ${SF_RO_INV}			
  		SectionSetFlags ${PPJoy_Inst} $7	; Remove Read-Only flag from the PPJoy component
  		
    	; Check if PPJoy already installed. If NOT installed check the PPJoy component    	
  		IntCmp $Changed 0 Next1					; Activate this code only if type changed
  		Call  isPPJoyInstalled
  		IntCmp $PPJoyExist 1 Next1
  		IntOp $7 $7 | ${SF_SELECTED}
    	SectionSetFlags ${PPJoy_Inst} $7		; Check the PPJoy component
  		Goto Next1
  			
	Next1:	
	 
  FunctionEnd
  
  Function un.onInit
  
	StrCpy $SPP4FMSremoved 0
	StrCpy $GenSPPremoved  0
	StrCpy $Utilsremoved   0

   	Call un.GetInstallFolders
	SectionSetText ${UnGenSPP}  ""
	SectionSetText ${UnSPP4FMS} ""
;	SectionSetText ${UnUtils} ""
   	
  	StrLen $0 $FolderSPP4FMS
  	IntCmp $0 0 Generic 
  	
  	;Fms:
	SectionSetText ${UnSPP4FMS} "SmartPropoPlus for FMS"
  		
  	Generic:
  	StrLen $0 $FolderGenSpp
  	IntCmp $0 0 Utils 
	SectionSetText ${UnGenSPP} "Generic SmartPropoPlus"
	
  	Utils:
  	StrLen $0 $FolderUtils
  	IntCmp $0 0 Common 
;	SectionSetText ${UnUtils} "Utilities"
	return
	
  	Common:

  FunctionEnd
  
;--------------------------------
; User Functions
Function "FinishPreFunction"	  	
	; Get the type selected 
  	SectionGetFlags ${SPP4FMS} $0
  	SectionGetFlags ${GenSPP} $1
  	IntOp $0 $0 & ${SF_SELECTED} ; Spp for FMS
  	IntOp $1 $1 & ${SF_SELECTED} ; Generic SPP
 	
  	; Determine the output folder
  	IntCmp $0 1 isFMS 
  	IntCmp $1 1 isGen
  	
isFMS:
  	;Remind to install FMS (if not installed)
  	call isFmsInstalled
  	IntCmp $FmsExist 1 NoNeedToRemind
  	MessageBox MB_OK|MB_ICONINFORMATION  \
  	"FMS not installed$\n\
  	Please install it in folder $INSTDIR"
  	DetailPrint "[MB] Please install FMS in folder $INSTDIR"
;NoNeedToRemind1:
	Abort
 
isGen:
  	;Remind to install PPJoy (if not installed)
  	call isPPJoyInstalled
  	IntCmp $PPJoyExist 1 NoNeedToRemind
  	MessageBox MB_OK|MB_ICONINFORMATION \
  	"PPJoy not installed$\n\
  	Please install it"
  	DetailPrint "[MB] Please install PPJoy"
  	Abort
  	
NoNeedToRemind:  
		return

FunctionEnd

Function "isPPJoyInstalled"
	
	;DetailPrint "[I] Searching for PPJoy in the registry"
	; Get the Display name of PPJoy and test it
	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Parallel Port Joystick" "DisplayName"
	StrLen $1 $0
	; If found and length longer than 0 set $PPJoyExist to 1. Else set to 0
	IntCmp $1 0 NotInstalled
	IntOp $PPJoyExist 0 + 1 ; Installed
	goto end
		
NotInstalled:
	IntOp $PPJoyExist 0 + 0 ; PPJoy NOT  installed 
	
end:
FunctionEnd ; isPPJoyInstalled

Function "isFmsInstalled"
	
	;DetailPrint "[I] Searching for FMS in the registry"
	; Get the Display name of Fms and test it
	ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\FMS" "DisplayName"
	StrLen $1 $0
	; If found and length longer than 0 set $FmsExist to 1. Else set to 0
	IntCmp $1 0 NotInstalled
	IntOp $FmsExist 0 + 1 ; Installed
	goto end
		
NotInstalled:
	IntOp $FmsExist 0 + 0 ; FMS NOT  installed 
	
end:
FunctionEnd ; isFmsInstalled

; Get installation folders Spp4FMS
Function "un.GetInstallFolders"
   StrCpy $FolderSPP4FMS ""
   StrCpy $FolderGenSpp ""
   StrCpy $FolderUtils ""
   ReadRegStr $FolderSPP4FMS HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderSPP4FMS"
   ReadRegStr $FolderGenSpp  HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderGenSpp"
   ReadRegStr $FolderUtils   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderUtils"
FunctionEnd

; Find FMS folder
Function GetFmsFolder
	ReadRegStr $R1 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\FMS" "UninstallString"
	${GetParent} "$R1" $FolderFMS
FunctionEnd

; Leave function for the component page
Function ComponentLeaveFunction
	Push $R0
	Push $R1
	Push $R2
	Push $R3
	Push $R5

	; Get selection
  	SectionGetFlags ${SPP4FMS} $R5
  	IntOp $R5 $R5 & ${SF_SELECTED} ; Spp for FMS
  	IntCmp $R5 1 isFMS 
  	
  	SectionGetFlags ${GenSPP} $R5
  	IntOp $R5 $R5 & ${SF_SELECTED} ; Generic SPP
  	IntCmp $R5 1 isGen PopAll PopAll
 	
	
isGen:
	; Test is SPP for FMS is installed
	ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderSPP4FMS"
	StrLen $R2 $R0 
  	IntCmp $R2 0 PopAll
	DetailPrint "[MB] Installing Generic SmartPropoPlus. SmartPropoPlus for FMS is already installed"
	MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2   "\
	You are trying to install Generic SmartPropoPlus$\n\
	SmartPropoPlus for FMS is already installed$\n\
	Are you sure you want to continue?" IDYES yes IDNO no
	goto PopAll
	
isFMS:
	; Test if Generic SPP is installed
	ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus" "FolderGenSpp"
	StrLen $R2 $R0 
  	IntCmp $R2 0 PopAll
	DetailPrint "[MB] Installing SmartPropoPlus for FMS. Generic SmartPropoPlus is already installed"
	MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2   "\
	You are trying to install SmartPropoPlus for FMS$\n\
	Generic SmartPropoPlus is already installed$\n\
	Are you sure you want to continue?" IDYES yes IDNO no
	
yes:
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3
	Pop $R5
	return
no:
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3
	Pop $R5
	abort
	
PopAll:
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3
	Pop $R5
FunctionEnd
;--------------------------------
