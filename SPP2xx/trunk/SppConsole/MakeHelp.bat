@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by SPPCONSOLE.HPJ. >"hlp\SppConsole.hm"
echo. >>"hlp\SppConsole.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\SppConsole.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\SppConsole.hm"
echo. >>"hlp\SppConsole.hm"
echo // Prompts (IDP_*) >>"hlp\SppConsole.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\SppConsole.hm"
echo. >>"hlp\SppConsole.hm"
echo // Resources (IDR_*) >>"hlp\SppConsole.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\SppConsole.hm"
echo. >>"hlp\SppConsole.hm"
echo // Dialogs (IDD_*) >>"hlp\SppConsole.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\SppConsole.hm"
echo. >>"hlp\SppConsole.hm"
echo // Frame Controls (IDW_*) >>"hlp\SppConsole.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\SppConsole.hm"
REM -- Make help for Project SPPCONSOLE


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\SppConsole.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\SppConsole.hlp" goto :Error
if not exist "hlp\SppConsole.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\SppConsole.hlp" Debug
if exist Debug\nul copy "hlp\SppConsole.cnt" Debug
if exist Release\nul copy "hlp\SppConsole.hlp" Release
if exist Release\nul copy "hlp\SppConsole.cnt" Release
echo.
goto :done

:Error
echo hlp\SppConsole.hpj(1) : error: Problem encountered creating help file

:done
echo.
