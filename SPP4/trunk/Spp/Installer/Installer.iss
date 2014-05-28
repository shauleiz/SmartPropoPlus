; Installation Script for SmartPropoPlus 4

#define MyAppName "SmartPropoPlus 4 - Alpha Release"
#define MyShortAppName "SmartPropoPlus"
#define MyAppPublisher "Shaul Eizikovich"
#define MyAppURL "http://SmartPropoPlus.sourceforge.net"
#define AppGUID "{{2E84A5A4-351E-4B00-9926-F50DBD7481E9}"

; Folders & Files
#define Src86Folder "..\Release"
#define Src64Folder "..\x64\Release"
#define ExternFolder ".\Externals"
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
UninstallDisplayIcon=.\UnInstaller.ico
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
; Executable
Source: "{#Src86Folder}\{#SppExec}"; DestDir: "{app}"  ; Check: IsX86
Source: "{#Src64Folder}\{#SppExec}"; DestDir: "{app}"  ; Check: IsX64
; vJoy Installer
Source: "{#ExternFolder}\{#vJoyInstaller}"; DestDir: "{app}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\Uninstall SmartPropoPlus"; Filename: "{uninstallexe}"

;[UninstallRun]
;Filename: "{code:vJoyUnInstaller}";  Parameters: "/LOG /silent " ; StatusMsg: "Uninstalling vJoy device"; Flags: waituntilterminated

[Run]
Filename: "{app}\{#vJoyInstaller}";  Parameters: "/LOG /SILENT /NORESTART /SUPPRESSMSGBOXES"  ; WorkingDir: "{app}"; Flags: waituntilterminated RunHidden; StatusMsg: "Installing vJoy device"


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

(*  event functions *)

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