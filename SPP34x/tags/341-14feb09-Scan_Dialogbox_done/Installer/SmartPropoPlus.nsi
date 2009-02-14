/*****************************************************************************
        SmartPropoPlus Installation script - version 3.4.1 and up
  Logic: Single section that has two flavours:
         1. With FMS support (If FMS is installed and if the user select this option)
         2. Without FMS support (If FMS is NOT installed or if the user does not select this option)

  If a former version of SPP is already installed - this installed copies the current registry
  enries to the new registry location.
*****************************************************************************/
;-----------------------------------------------------------------------------
;           Written by Shaul Eizikovich - November 2008
;-----------------------------------------------------------------------------

/*** Include files ***/
!include "MUI2.nsh" 		; Modern User Interface
!include "LogicLib.nsh"
!include "nsDialogs.nsh"
!include "FileFunc.nsh"	; File functions
!insertmacro GetParent
!include "registry.nsh" ; Registry macros
!insertmacro COPY_REGISTRY_KEY
!include "Version.nsh" ; Version manipulation

/*** General definitions ***/
!define INST_VERSION     "341" ; Version Number
!define EXEFILE          "SppInstall${INST_VERSION}.exe" ; Name of output file
!define MUI_ICON         "..\SppConsole\res\SppConsole.ico" /* Icon */
!define MUI_UNICON       "UnInstaller.ico" /* Uninstall Icon */

/*** Definitions Registry entries ***/
!define KREG_UNINST_SPP    "Software\Microsoft\Windows\CurrentVersion\Uninstall\SmartPropoPlus"
!define KREG_UNINST_FMS    "Software\Microsoft\Windows\CurrentVersion\Uninstall\FMS"
!define KREG_UNINST_PPJ    "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Parallel Port Joystick"
!define KREG_FMS_BASE      "Software\Flying-Model-Simulator"
!define KREG_A2J_BASE      "Software\Audio2Joystick"
!define KREG_OLD_BASE      "Software\Flying-Model-Simulator\SmartPropoPlus"
!define KREG_NEW_BASE      "Software\Audio2Joystick\SmartPropoPlus"

/*** Definitions Files & Folders ***/
!define DIR_START_SPP    "$SMPROGRAMS\SmartPropoPlus" ; SPP on Start Menu
!define DIR_START_UTILS  "${DIR_START_SPP}\SmartPropoPlus Utilities"
!define DIR_SPP          "$PROGRAMFILES\SmartPropoPlus"
!define DIR_SRC_UTILS    "C:\Program Files\Osc"

/*** Interface text Settings ***/
LangString FMSPAGE_TITLE        ${LANG_ENGLISH} "FMS"
LangString FMSPAGE_SUBTITLE     ${LANG_ENGLISH} "If you want FMS to be controlled by SmartPropoPlus - Check this box"
LangString FMSPAGE_CB_TEXT      ${LANG_ENGLISH} "Use your transmitter as &FMS joystick"
LangString FMSPAGE_BROWSE_TEXT  ${LANG_ENGLISH} "Browse ..."
LangString FINISH_TEXT          ${LANG_ENGLISH} "Completing SmartPropoPlus"
LangString FINISH_SUBTEXT       ${LANG_ENGLISH} "SmartPropoPlus has been installed on your computer"
LangString START_FMS            ${LANG_ENGLISH} "Start Flying Model Simulator (FMS)"
LangString START_SPP            ${LANG_ENGLISH} "Start SmartPropoPlus"

/*** Global variables ***/
VAR  RunAppOnFinish       ; The application to run at the end of the installation
VAR  RunAppOnFinishText   ; The text next to the checkbox of the application to run
VAR  FmsExist		  ; Boolean: 1 if FMS installed
VAR  UtilsDir
VAR  SupportFMS           ; Boolean: 1 if SPP should support FMS
VAR  VERSION		  ; SPP DLL file version (3.3.3)
VAR  OLD_VERSION	  ; SPP DLL file version of the previously installed SPP(3.3.3)
VAR  PPJoyExist           ; Boolean: 1 if PPJoy installed
VAR  UNINSTALL            ; The uninstall command (Previous version)

;  Variables related to the FMS custom page
VAR  hFmsDialog        ; Handle to the FMS custom dialog box
VAR  FmsDir            ; Where to install winmm.dll
VAR  hFmsDir           ; Handle to the control holding 'FmsDir'
VAR  hFmsBrowseBtn     ; Handle to browse button
VAR  hFmsCheckbox      ; Handle to 'support FMS' checkbox
VAR  FmsCheckboxState  ; State of the 'support FMS' checkbox


/*** General information: Output file, stype, output directory etc. ***/
Name "SmartPropoPlus"     ; Default caption for installer windows
OutFile "${EXEFILE}"      ; Name of output executable file
XPStyle on
InstallDir "${DIR_SPP}"   ;Default installation folder

/*** Finish page modifications ***/
; Make the FINISH page run SPP
!define MUI_INSTFILESPAGE_FINISHHEADER_TEXT "$(FINISH_TEXT)"        ; Text for the Finish page
!define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT "$(FINISH_SUBTEXT)"  ; Sub-text for the Finish page
!define MUI_FINISHPAGE_RUN $RunAppOnFinish                          ; Application to run at the end of the instalation
!define MUI_FINISHPAGE_RUN_TEXT $RunAppOnFinishText                 ; Text next to the checkbox in the finish page

;-------------------------------- Pages --------------------------------
;--- Install Pages
Page custom FmsPage FmsPageLeave ":  FMS Interface"
!define 	MUI_PAGE_CUSTOMFUNCTION_PRE DirectoryPreFunction
!insertmacro	MUI_PAGE_DIRECTORY	; Directory page - Where to install
!define 	MUI_PAGE_CUSTOMFUNCTION_PRE InstfilesPreFunction ; Pre fuction for InstFiles page
!insertmacro	MUI_PAGE_INSTFILES	; installation page - Where the sections are executed
!define 	MUI_PAGE_CUSTOMFUNCTION_PRE FinishPreFunction ; Pre fuction for finish page
!insertmacro	MUI_PAGE_FINISH

;--- Uninstall Page
!insertmacro	MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English" ; This must come AFTER the definitions of the pages

;-------------------------------- Macros --------------------------------

!macro GetLocalFileVer FILENAME VERSTR
	GetDLLVersionLocal ${FILENAME} $R0 $R1
	IntOp $R2 $R0 / 0x00010000
	IntOp $R3 $R0 & 0x0000FFFF
	IntOp $R4 $R1 / 0x00010000
	IntOp $R5 $R1 & 0x0000FFFF
	StrCpy ${VERSTR} "$R2.$R3.$R4.$R5"
!macroend

; Create a Windows shortcut to a URL
!macro CreateInternetShortcut FILENAME URL ICONFILE ICONINDEX
	WriteINIStr "${FILENAME}.url" "InternetShortcut" "URL" "${URL}"
	WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconFile" "${ICONFILE}"
	WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconIndex" "${ICONINDEX}"
!macroend


;--------------------------------------------------------------------------
; Installation section - Single, hiddle section
;--------------------------------------------------------------------------
Section  "-MainSection" MainSection
 call InstallSppFiles ; Install SPP
 call MigrateReg      ; Migrate SPP registry key from old location to new location
 call WriteUnInstallInfo ; Write all uninstall data into the registry
 SetAutoClose false
SectionEnd ;MainSection

Section "-un.install"
 DetailPrint "[I] Uninstalling SmartPropoPlus" ; Debug
 call Un.InstallSppFiles ; Install SPP

 ; Remove registry key
 DetailPrint "[I] Removing SmartPropoPlus entry from registry key ${KREG_UNINST_SPP}" ; Debug
 DeleteRegKey HKLM "${KREG_UNINST_SPP}"
SectionEnd ;-un.install

Section "-Create Uninstaller" CreateUninstSpp
;Create uninstaller
WriteUninstaller "$INSTDIR\UninstallSPP.exe"
CreateShortCut "${DIR_START_SPP}\Uninstall SmartPropoPlus.lnk" "$INSTDIR\UninstallSPP.exe"
SectionEnd

/*************************************************************************************/
/*                                      Functions                                    */
/*************************************************************************************/

;--------------------------------------------------------------------------------------
;  Callback Function that is called first
;
;  Gets the version of this installation and the version of the already installed one
;  It then copares the two versions. If the this is OLDER than the installed then the user
;  is alerted.
;
;--------------------------------------------------------------------------------------
Function .OnInit
!insertmacro GetLocalFileVer "..\winmm\Release\winmm.dll" "$VERSION" ; DLL Version manipulation
ReadRegStr $OLD_VERSION HKLM "${KREG_UNINST_SPP}" "DisplayVersion"   ; Version of the current installation
${CmpVer} "$VERSION" "$OLD_VERSION" "$R2"                            ; Compare versions
${If} $R2 == -1 ; A newer version is already installed
      MessageBox MB_YESNO "You are trying to install SmartPropoPlus version $VERSION $\n\
      while a newer version ($OLD_VERSION) is currently installed $\n$\n\
      Would you like to proceed?" IDYES Proceed
${Else}
       goto Proceed
${EndIf}
Quit
Proceed:
FunctionEnd

;--------------------------------------------------------------------------------------
;  Function that is called just before the InstallFiles page
;
;  Get the uninstall function of the already installed SPP from the registry.
;  Runs it in order to uninstall it.
;
;--------------------------------------------------------------------------------------
Function InstfilesPreFunction
ReadRegStr $UNINSTALL HKLM "${KREG_UNINST_SPP}" "UninstallString"   ; Uninstall.exe
ExecWait '"$UNINSTALL" /S _?=$INSTDIR'
FunctionEnd

;--------------------------------------------------------------------------------------
;  Function that is called just before the FINISH page
;  Test if PPJoy is installed. If not issue a warning
;  Initialize FINISH page variables
;--------------------------------------------------------------------------------------
Function FinishPreFunction

 ; If support for FMS was not requested and PPJoy not installed - Issue warning
 ${If} $SupportFMS == 0
       ;Remind to install PPJoy (if not installed)
       call isPPJoyInstalled
       ${If} $PPJoyExist == 0
             MessageBox MB_OK|MB_ICONINFORMATION \
  	     "PPJoy not installed$\n\
  	     Please install it"
  	     Abort
       ${EndIf} ;$PPJoyExist == 0
 ${EndIf} ; $SupportFMS == 0
         
 ; Offer user to start FMS (if support for FMS was requested)
 ${If} $SupportFMS == 1
       StrCpy $RunAppOnFinish "$FmsDir\FMS.exe"
       StrCpy $RunAppOnFinishText "$(START_FMS)"
       return
 ${EndIf} ; $SupportFMS == 1
 
 ; Offer user to start SPP (if support for FMS was not requested and PPJoy was installed)
 ${If} $PPJoyExist == 1
       StrCpy $RunAppOnFinish "$INSTDIR\SppConsole.exe"
       StrCpy $RunAppOnFinishText "$(START_SPP)"
       return
 ${EndIf} ; $PPJoyExist == 1
 
FunctionEnd

;--------------------------------------------------------------------------------------
;   Function that creats the FMS custom page
;   The page is displayed only if FMS is already installed
;   The page contains
;       Checkbox (Default = Checked)
;       Directory field (Default = FMS directory)
;       Browse button
;
;   The Directory field & the Browse button are enabled only when the checkbox is checked.
;   If the ceckbox is checked then $SupportFMS=1 and $FmsDir is set to the FMS folder
;   If the ceckbox is NOT checked then $SupportFMS=0
;--------------------------------------------------------------------------------------
Function FmsPage
  !insertmacro MUI_HEADER_TEXT "$(FMSPAGE_TITLE)" "$(FMSPAGE_SUBTITLE)"   # Display the page.

  # Display this page only if FMS is installed
  call isFmsInstalled
  ${if} $FmsExist == 0
        IntOp $SupportFMS 0 + 0 ; Do not support FMS
        Abort
  ${EndIf}

  # Get the user entered values.
  nsDialogs::Create /NOUNLOAD 1018
  Pop $hFmsDialog

  ${If} $hFmsDialog == error
      IntOp $SupportFMS 0 + 0 ; Do not support FMS
      Abort
  ${EndIf}

  # Get the FMS directory
  StrLen $0 $FmsDir
  ${If} $0 == 0
      call GetFmsFolder
      pop $FmsDir             ; This is where GetFmsFolder() pushed the folder name
      IntOp $FmsCheckboxState 0 + 1  ; Starting point is: Checkbox checked
  ${EndIf}

  # Create the FMS check box
  ${NSD_CreateCheckbox} 0 10u 100% 10u "$(FMSPAGE_CB_TEXT)"
  Pop $hFmsCheckbox
  ${NSD_SetState} $hFmsCheckbox $FmsCheckboxState

  # Create the FMS directory field
  ${NSD_CreateDirRequest} 0 30u 70% 15u $FmsDir
  Pop $hFmsDir

  # Create the Browse button - if clicked the function 'OnClickBrowse' is called
  ${NSD_CreateBrowseButton} 320 30u 20% 15u "$(FMSPAGE_BROWSE_TEXT)"
  Pop $hFmsBrowseBtn
  ${NSD_OnClick} $hFmsBrowseBtn OnClickBrowse

  # If checkbox state changes then the browse button and the directory field are enabled/disabled accordingly
  ${NSD_OnClick} $hFmsCheckbox OnClickFmsCB

  # Enable/Disable fields according to checkbox status
  call OnClickFmsCB

  nsDialogs::Show

FunctionEnd

;--------------------------------------------------------------------------------------
; Function called when FMS checkbox state changes and before FMS custom page is displayed ##
; Get checkbox status and enable/disable folder control & browse button accordingly
; Set $SupportFMS to 1/0 according the state (Checked/Un-checked) of the checkbox
;--------------------------------------------------------------------------------------
Function OnClickFmsCB
         ${NSD_GetState} $hFmsCheckbox $FmsCheckboxState
         ${If} $FmsCheckboxState == ${BST_CHECKED}
               EnableWindow $hFmsBrowseBtn 1
               EnableWindow $hFmsDir 1
               IntOp $SupportFMS 0 + 1 ; Support FMS
         ${Else}
               EnableWindow $hFmsBrowseBtn 0
               EnableWindow $hFmsDir 0
               IntOp $SupportFMS 0 + 0 ; Do not support FMS
         ${EndIf}
FunctionEnd

;--------------------------------------------------------------------------------------
; Function called when FMS folder browse button is clicked
; Sets the value of FmsDir to the FMS folder
;--------------------------------------------------------------------------------------
Function OnClickBrowse
         # Get the FMS directory
         ${NSD_GetText} $hFmsDir $FmsDir
         nsDialogs::SelectFolderDialog /NOUNLOAD "" $FmsDir
         Pop $FmsDir
 	 ${If} $FmsDir == error
	       goto End
	 ${EndIf}
         ${NSD_SetText} $hFmsDir $FmsDir
End:
FunctionEnd

Function FmsPageLeave
FunctionEnd


;--------------------------------------------------------------------------------------
;  Function called before the Directory Page is called
;  If FMS is not installed then the previous FMS custom page was not displayed.
;  If this case - the Directory Page becomes the first page hence button 'Next' should be disabled
;--------------------------------------------------------------------------------------
Function DirectoryPreFunction
## Disable Back button text if former page was not displayed
call isFmsInstalled
${if} $FmsExist == 0
      GetDlgItem     $R0 $HWNDPARENT 3
      EnableWindow $R0 0
${EndIf}
FunctionEnd

;--------------------------------------------------------------------------------------
;  Function tests in the registry for FMS.
;  If installed, $FmsExist=1
;  If NOT installed, $FmsExist=0
;--------------------------------------------------------------------------------------
Function "isFmsInstalled"
	DetailPrint "[I] Searching for FMS in the registry"
	; Get the Display name of Fms and test it
	ReadRegStr $0 HKLM "${KREG_UNINST_FMS}" "DisplayName"
	StrLen $1 $0
	; If found and length longer than 0 set $FmsExist to 1. Else set to 0
	IntCmp $1 0 NotInstalled
	IntOp $FmsExist 0 + 1 ; Installed
	return
NotInstalled:
	IntOp $FmsExist 0 + 0 ; FMS NOT  installed
FunctionEnd ; isFmsInstalled

; Get installation folders Spp4FMS
/*Function "un.GetInstallFolders"
   StrCpy $FmsDir ""
   StrCpy $SppDir ""
   StrCpy $UtilsDir ""
   ReadRegStr $FmsDir          HKLM "${KREG_UNINST_SPP}" "FolderSPP4FMS"
   ReadRegStr $SppDir          HKLM "${KREG_UNINST_SPP}" "FolderGenSpp"
   ReadRegStr $UtilsDir        HKLM "${KREG_UNINST_SPP}" "FolderUtils"
FunctionEnd*/

;--------------------------------------------------------------------------------------
;  Find FMS folder - Push it on the stack
;--------------------------------------------------------------------------------------
Function GetFmsFolder
	ReadRegStr $R1 HKLM "${KREG_UNINST_FMS}" "UninstallString"
	${GetParent} "$R1" $R0
	StrLen $R2 $R0
	IntCmp $R2 0 Nothing
;CopyString:
	Push $R0
Nothing:
FunctionEnd

;--------------------------------------------------------------------------------------
;  Install SPP Files
;  SPP files in the default output directory
;  Utility files
;  Shortcuts
;--------------------------------------------------------------------------------------
Function InstallSppFiles
 ;SPP files
 SetOutPath "$INSTDIR"
 DetailPrint "[I] Installing SmartPropoPlus files to $INSTDIR" ; Debug
 file ..\ReleaseNotes.pdf
 file ..\..\msvcr71.dll
 file ..\..\MFC71.dll
 file ..\..\AudioStudy\Release\AudioStudy.exe
 file ..\..\filters\JsChPostProc\Release\JsChPostProc.dll
 file ..\SppConsole\Release\SppConsole.exe
 file ..\Wave2Joystick\Release\Wave2Joystick.dll

 
 ; Utilities
 StrCpy $UtilsDir "$INSTDIR\Utilities"
 DetailPrint "[I] Installing utilities to $UtilsDir" ; Debug
 SetOutPath "$UtilsDir"
 file "${DIR_SRC_UTILS}\WinScope.*"
 file "${DIR_SRC_UTILS}\RCAudio.exe"
 file "${DIR_SRC_UTILS}\AudPPMV.exe"

 ; Start Menu
 DetailPrint "[I] Creating shortcuts in ${DIR_START_SPP}" ; Debug
 CreateDirectory "${DIR_START_SPP}"
 CreateShortCut  "${DIR_START_SPP}\SppConsole.lnk" "$INSTDIR\SppConsole.exe"
 CreateShortCut  "${DIR_START_SPP}\Release Notes.lnk" "$INSTDIR\ReleaseNotes.pdf"
 !insertmacro CreateInternetShortcut "${DIR_START_SPP}\SmartPropoPlus Web Site"  "http://www.smartpropoplus.com" "" ""

 ; Start menu utility folder
 CreateDirectory "${DIR_START_UTILS}"
 CreateShortCut  "${DIR_START_UTILS}\Winscope.lnk"                    "$UtilsDir\Winscope.exe"
 CreateShortCut  "${DIR_START_UTILS}\RCAudio (PPM Thermometer).lnk"   "$UtilsDir\RCAudio.exe"
 CreateShortCut  "${DIR_START_UTILS}\AudPPMV.lnk"                     "$UtilsDir\AudPPMV.exe"

 ; FMS Support (if requested)
 ${If} $SupportFMS == 1
       DetailPrint "[I] Installing FMS extenssion to $FmsDir" ; Debug
       file  /oname=$FmsDir\winmm.dll ..\winmm\Release\winmm.dll
       CreateShortCut  "${DIR_START_SPP}\FMS.lnk" "$FmsDir\FMS.exe"
 ${EndIf}
 
 ; Desktop Icon
 DetailPrint "[I] Creating desktop shortcut" ; Debug
 CreateShortCut  "$DESKTOP\SppConsole.lnk"                            "$INSTDIR\SppConsole.exe"
FunctionEnd

;--------------------------------------------------------------------------------------
; Migrate SPP registry key from old location to new location
;--------------------------------------------------------------------------------------
Function MigrateReg
  ; Test if new location exists. If exists go out
  ; Loop on Audio2Joystick entry in search of SPP key
  ; $0 is enum counter
  ; $1 is the FMS sub-key
  StrCpy $0 0
  loop_new:
       EnumRegKey $1 HKCU ${KREG_A2J_BASE} $0
       StrCmp $1 "" no_new
       StrCmp $1 "SmartPropoPlus" done_new
       IntOp $0 $0 + 1
       goto loop_new
  done_new:
       DetailPrint "[I] New registry location already exists - Migration aborted"
       return
  no_new:
  
  ; Test if old location exists. If does not exist go out
  ; Loop on FMS entry in search of SPP key
  ; $0 is enum counter
  ; $1 is the FMS sub-key
  StrCpy $0 0
  loop_old:
       EnumRegKey $1 HKCU ${KREG_FMS_BASE} $0
       StrCmp $1 "" done_old
       StrCmp $1 "SmartPropoPlus" CopyRegKey
       IntOp $0 $0 + 1
       goto loop_old
  done_old:
       DetailPrint "[I]  Old registry location does not exist - Migration aborted"
       return
  
  ; Copy registry tree
  CopyRegKey:
        DetailPrint "[I] Migrating older registry entries to new location" ; Debug
        ${COPY_REGISTRY_KEY} HKCU ${KREG_OLD_BASE} HKCU ${KREG_NEW_BASE}

FunctionEnd ; MigrateReg

;--------------------------------------------------------------------------------------
;  Write SPP uninstall info to the registry
;--------------------------------------------------------------------------------------
Function WriteUnInstallInfo
  ; Add uninstall information to Add/Remove Programs
  ; Standard
  DetailPrint "[I] Writing uninstall info to registry key ${KREG_UNINST_SPP}" ; Debug
  WriteRegStr   HKLM   ${KREG_UNINST_SPP} "DisplayName" "SmartPropoPlus"
  WriteRegStr   HKLM   ${KREG_UNINST_SPP} "UninstallString" "$INSTDIR\UninstallSPP.exe"
  WriteRegDWORD HKLM   ${KREG_UNINST_SPP} "NoRepair" 1
  WriteRegDWORD HKLM   ${KREG_UNINST_SPP} "NoModify" 1
  WriteRegStr   HKLM   ${KREG_UNINST_SPP} "URLInfoAbout" "http://www.SmartPropoPlus.com"
  WriteRegStr   HKLM   ${KREG_UNINST_SPP} "Publisher" "SmartPropoPlus"
  WriteRegStr   HKLM   ${KREG_UNINST_SPP} "DisplayVersion" $VERSION
  WriteRegStr   HKLM   ${KREG_UNINST_SPP} "DisplayIcon" "$INSTDIR\SppConsole.exe"

 ; FMS Support (if requested)
 ${If} $SupportFMS == 1
       WriteRegStr HKLM ${KREG_UNINST_SPP} "FolderSPP4FMS" "$FmsDir"
 ${EndIf}
FunctionEnd

;--------------------------------------------------------------------------------------
;  Check if PPJoy is installed
;  Result in boolean variable $PPJoyExist
;--------------------------------------------------------------------------------------
Function "isPPJoyInstalled"

	;DetailPrint "[I] Searching for PPJoy in the registry"
	; Get the Display name of PPJoy and test it
	ReadRegStr $0 HKLM "${KREG_UNINST_PPJ}" "DisplayName"
	${If} $0 == ""
              IntOp $PPJoyExist 0 + 0 ; PPJoy NOT  installed
        ${Else}
	       IntOp $PPJoyExist 0 + 1 ; Installed
        ${EndIf}
FunctionEnd ; isPPJoyInstalled


;--------------------------------------------------------------------------------------
;  Un-Install SPP Files
;  SPP files from the SPP directory
;  Utility files
;  Shortcuts
;--------------------------------------------------------------------------------------
Function  Un.InstallSppFiles
 ; Delete files 
 DetailPrint "[I] Removing Generic SmartPropoPlus files from $INSTDIR" ; Debug
 Delete "$INSTDIR\UninstallSPP.exe"
 Delete "$INSTDIR\ReleaseNotes.pdf"
 Delete "$INSTDIR\msvcr71.dll"
 Delete "$INSTDIR\MFC71.dll"
 Delete "$INSTDIR\AudioStudy.exe"
 Delete "$INSTDIR\JsChPostProc.dll"
 Delete "$INSTDIR\SppConsole.exe"
 Delete "$INSTDIR\Wave2Joystick.dll"

; Remove utilities files
 StrCpy $UtilsDir "$INSTDIR\Utilities"
 DetailPrint "[I] Removing Utility files from $UtilsDir" ; Debug
 Delete "$UtilsDir\Winscope.*"
 Delete "$UtilsDir\RCAudio.exe"
 Delete "$UtilsDir\AudPPMV.exe"
 
 ; Remove directory only if empty
 RMDir  "$UtilsDir"
 RMDir  $INSTDIR

 ; Clean the start menu
 DetailPrint "[I] Removing common links from Start menu" ; Debug
 ; Remove Utilities short cuts
 Delete "${DIR_START_SPP}\SmartPropoPlus Utilities\*"
 RMDir "${DIR_START_SPP}\SmartPropoPlus Utilities"
 ; Remove the rest
 Delete "${DIR_START_SPP}\Uninstall SmartPropoPlus.lnk"
 Delete "${DIR_START_SPP}\Release Notes.lnk"
 Delete "${DIR_START_SPP}\SmartPropoPlus Web Site.url"
 Delete "${DIR_START_SPP}\SppConsole.lnk"
 Delete "${DIR_START_SPP}\Fms.lnk"
 RMDir  "${DIR_START_SPP}"
 
 ; Remove FMS extenssion
 StrCpy $FmsDir ""
 ReadRegStr $FmsDir          HKLM "${KREG_UNINST_SPP}" "FolderSPP4FMS"
 ${If} $FmsDir != ""
       DetailPrint "[I] Removing FMS extenssion from $FmsDir" ; Debug
       Delete "$FmsDir\winmm.dll"
 ${EndIf}
 
 DetailPrint "[I] Removing SppConsole icon from desktop" ; Debug
 Delete "$DESKTOP\SppConsole.lnk"			; Remove icon from desktop
FunctionEnd
