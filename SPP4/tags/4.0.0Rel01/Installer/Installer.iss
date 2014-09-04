; Installation Script for SmartPropoPlus 4

#define MyAppName "SmartPropoPlus"
#define MyShortAppName "SmartPropoPlus"
#define MyAppPublisher "Shaul Eizikovich"
#define MyAppURL "http://SmartPropoPlus.sourceforge.net"
#define AppGUID "{{2E84A5A4-351E-4B00-9926-F50DBD7481E9}"

; Folders & Files
#define Src86Folder "..\Release"
#define Src64Folder "..\x64\Release"
#define ExternFolder ".\Externals"
#define DllX86Folder ".\Externals\x86"
#define DllX64Folder ".\Externals\x64"
#define SppIconFile "..\SppUI\SppControl.ico"
#define SppExec "SppConsole.exe"
#define vJoyInstaller "vJoyInstall.exe"

; Get product version from the exectutable
#define SrcExe Src64Folder+"\"+SppExec
#define FileVerStr GetFileVersion(SrcExe)
#define StripBuild(str VerStr) Copy(VerStr, 1)
#define ExeVerStr StripBuild(FileVerStr)
#define vJoyID	"{8E31F76F-74C3-47F1-9550-E041EEDC5FBB}"

[Setup]
AppID={code:GetAppId}
AppName={#MyAppName}
AppVersion={#ExeVerStr}
VersionInfoVersion={#ExeVerStr}
AppVerName= {#MyAppName} {#ExeVerStr}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
CreateAppDir=true
OutputDir=.
OutputBaseFilename=Spp4Installer
SetupIconFile=.\Installer.ico
UninstallDisplayIcon={app}\UnInstaller.ico
Compression=lzma/Max
SolidCompression=true
DefaultDirName={pf}\{#MyShortAppName}
DefaultGroupName={#MyShortAppName}
VersionInfoCompany=Shaul Eizikovich
AppCopyright=Copyright (c) 2005-2014 by Shaul Eizikovich
MinVersion = 6.0.6000sp2
;SignTool=Sig sign /a /v /s My /t http://timestamp.digicert.com $f
;SignedUninstaller=yes
DisableDirPage=no
DisableProgramGroupPage=no
DisableReadyMemo=no
DisableFinishedPage=no
PrivilegesRequired=admin
ArchitecturesInstallIn64BitMode=x64
SetupLogging=true
UsePreviousLanguage=no

[Files]
;DLLs
Source: "{#Src86Folder}\*.dll"; DestDir: "{app}"; Flags: promptifolder; Check: IsX86
Source: "{#Src64Folder}\*.dll"; DestDir: "{app}"; Flags: promptifolder; Check: IsX64
Source: "{#DllX86Folder}\*.dll"; DestDir: "{app}"; Flags: promptifolder; Check: IsX86
Source: "{#DllX64Folder}\*.dll"; DestDir: "{app}"; Flags: promptifolder; Check: IsX64
; Executable
Source: "{#Src86Folder}\{#SppExec}"; DestDir: "{app}"  ; Check: IsX86
Source: "{#Src64Folder}\{#SppExec}"; DestDir: "{app}"  ; Check: IsX64
; vJoy Installer
Source: "{#ExternFolder}\{#vJoyInstaller}"; DestDir: "{app}"; Flags: deleteafterinstall

; Icon files
Source: "UnInstaller.ico"; DestDir: "{app}";

[Tasks]
Name: desktopicon;   Description: "Create a &desktop icon" ; Flags: unchecked; 

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#SppExec}" ; WorkingDir: "{app}" ;
Name: "{group}\Uninstall SmartPropoPlus"; Filename: {uninstallexe}; IconFilename: {app}\UnInstaller.ico; 
Name: "{group}\SmartPropoPlus Web Site"; Filename: "http://smartpropoplus.sourceforge.net/prod" ; WorkingDir: "{app}"
Name: "{userdesktop}\{#MyAppName}"; Filename: "{app}\{#SppExec}" ; WorkingDir: "{app}" ; Tasks: desktopicon

;[UninstallRun]
;Filename: "{code:vJoyUnInstaller}";  Parameters: "/LOG /silent " ; StatusMsg: "Uninstalling vJoy device"; Flags: waituntilterminated

[Run]
Filename: "{app}\{#vJoyInstaller}";  Parameters: "/LOG /SILENT /NORESTART /SUPPRESSMSGBOXES  /COMPONENTS=""Apps\vJoyFeeder,Apps\vJoyMon"" "  ; WorkingDir: "{app}"; Flags: waituntilterminated RunHidden; StatusMsg: "Installing vJoy device"
Filename: "{app}\{#SppExec}";  Flags: nowait postinstall

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\vjoy\Parameters\Device01" ; Flags: uninsdeletekeyifempty
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\vjoy\Parameters\Device01" ; Flags: createvalueifdoesntexist ; ValueType: dword; ValueName: "HidReportDesctiptorSize" ; ValueData: "91"
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\vjoy\Parameters\Device01" ; Flags: createvalueifdoesntexist ; ValueType: binary; ValueName: "HidReportDesctiptor" ;   ValueData:  "05 01 15 00 09 04 a1 01 05 01 85 01 09 01 15 00 26 ff 7f 75 20 95 01 a1 00 09 30 81 02 09 31 81 02 09 32 81 02 09 33 81 02 09 34 81 02 09 35 81 02 09 36 81 02 09 37 81 02 c0 75 20 95 04 81 01 05 09 15 00 25 01 55 00 65 00 19 01 29 20 75 01 95 20 81 02 75 60 95 01 81 01 c0"


[Code]
(* Constants related to installation under SPP *)
const
AppIdFlag	= 'ID';
AppIdParam	= ' /'+AppIdFlag;
UninstKey 	= 'Software\Microsoft\Windows\CurrentVersion\Uninstall\';

(* Forward Function declarations - Start *)
function  IsX64: Boolean; Forward;
function  IsX86: Boolean; Forward;
function  GetAppId(Param: String): String; Forward;
function  vJoyUnInstaller: String; Forward;
procedure vJoyUninstal; Forward;              
function InitializeSetup(): Boolean; Forward;
function IsUpgrade(): Boolean;  Forward;
function GetUninstallString(): String; Forward;
function UnInstallOldVersion(): Integer; Forward;

(*  event functions *)                       

(*
	InitializeSetup() is an event function that is 
	Called during Setup's initialization.
	Setup aborted if function returns False. 
	
	In this case:
  Always Returns True (never abort installation)
*)
function InitializeSetup(): Boolean;

begin

	Log('InitializeSetup()');
    if (IsUpgrade()) then
    begin
      UnInstallOldVersion();
   end;
 
  Result := True; 
end;

(* Called with every step of uninstaller *)
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  (* First thing - uninstall vJoy *)
  if CurUninstallStep = usUninstall then begin
  vJoyUninstal();
  end;
end;

(* Helper Functions *)
function IsX64: Boolean;
begin
  Result := ProcessorArchitecture = paX64;
end;

function IsX86: Boolean;
begin
  Result := ProcessorArchitecture = paX86;
end;

/////////////////////////////////////////////////////////////////////
function IsUpgrade(): Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

/////////////////////////////////////////////////////////////////////
function GetUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
end;


/////////////////////////////////////////////////////////////////////

function UnInstallOldVersion(): Integer;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin
// Return Values:
// 1 - uninstall string is empty
// 2 - error executing the UnInstallString
// 3 - successfully executed the UnInstallString

  // default return value
  Result := 0;

  // get the uninstall string of the old app
  sUnInstallString := GetUninstallString();
  if sUnInstallString <> '' then begin
    sUnInstallString := RemoveQuotes(sUnInstallString);
    if Exec(sUnInstallString, '/SILENT  /NORESTART /SUPPRESSMSGBOXES','', SW_HIDE, ewWaitUntilTerminated  , iResultCode) then
      Result := 3
    else
      Result := 2;
  end else
    Result := 1;
end;

(* Uninstalls vJoy *)
procedure vJoyUninstal;

var
	executable	: String;
  res         : Boolean;
  ResultCode  : Integer;

begin
  executable := vJoyUnInstaller();
  res := Exec(executable ,' /SILENT /LOG ', '', SW_SHOW, ewWaitUntilTerminated, ResultCode);
  if not res then Log('vJoyUninstal(): Failed: ' +  SysErrorMessage(ResultCode))
  else   Log('vJoyUninstal(): OK:');
end;

(* Return the Uninstaller of vJoy *)
function vJoyUnInstaller: String;

var
	UninstKeyVjoy, executable	: String;
	Res: Boolean;
  Len: Longint;

begin
  (* Get the vJoy uninstaller registry key *)
	UninstKeyVjoy := UninstKey + '{#vJoyID}' + '_is1';
	Log('vJoyUnInstaller(): Uninstall Key is ' + UninstKeyVjoy);

  (* Test if this registry key exists*)
	if not RegValueExists(HKEY_LOCAL_MACHINE, UninstKeyVjoy, 'UninstallString') then
	begin	// RegValueExists = False
		Log('Could not find registry value HKLM\'+UninstKeyVjoy+'\UninstallString');
		Result := '';
		exit;
	end;	// RegValueExists = False
         
  Res := RegQueryStringValue(HKEY_LOCAL_MACHINE, UninstKeyVjoy, 'UninstallString', executable);
 	if Res then Len := Length(executable) else Len := 0;
  if (Len > 0) then Result := RemoveQuotes(executable)  else    Result := '';
  Log('vJoyUnInstaller(): Result: '+ Result);

end;


(*
  Get AppID from command line parameter ID (/ID=ABC)
  If this parameter is missing use AppGUID constant
*)
function GetAppId(Param: String): String;	
begin
 Result := ExpandConstant('{param:'+AppIdFlag+'}');
  if Length(Result)=0 then
    Result :=  expandconstant('{#AppGUID}');;

end;

(*
	Return the Uninstaller of vJoy 
*)
function vJoyUnInstallerOld(Param: String): String;

var
	UninstKeyVjoy, executable	: String;
	Res: Boolean;
  Len: Longint;
	
begin
	UninstKeyVjoy := UninstKey + expandconstant('{#vJoyID}') + '_is1';
	Log('vJoyUnInstaller(): Uninstall Key is ' + UninstKeyVjoy);
	
	if not RegValueExists(HKEY_LOCAL_MACHINE, UninstKeyVjoy, 'UninstallString') then
	begin	// RegValueExists = False
		Log('Could not find registry value HKLM\'+UninstKeyVjoy+'\UninstallString');
		Result := '';
		exit;
	end;	// RegValueExists = False
	
	Res := RegQueryStringValue(HKEY_LOCAL_MACHINE, UninstKeyVjoy, 'UninstallString', executable);
 	if Res then Len := Length(executable) else Len := 0;
  if (Len > 0) then Result := RemoveQuotes(executable)  else    Result := '';
  Log('vJoyUnInstaller(): Result: '+ Result);
end;