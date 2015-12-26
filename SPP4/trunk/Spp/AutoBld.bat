@ECHO ON

SET VS=14.0
SET BUILDER=%ProgramFiles(x86)%\MSBuild\%VS%\Bin\MSBuild.exe

REM Setting the Path and Environment Variables for Command-Line Builds
CALL "%VS110COMNTOOLS%"..\..\vc\bin\vcvars32.bat


:build32
echo Building SmartPropoPlus (x86)
"%BUILDER%"  spp.sln  /t:rebuild /p:Platform=Win32;Configuration=Release
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail

:build64
echo Building SmartPropoPlus (x64)
"%BUILDER%"  spp.sln  /t:rebuild /p:Platform=x64;Configuration=Release
set BUILD_STATUS=%ERRORLEVEL%
if not %BUILD_STATUS%==0 goto fail

REM Create Installer
CALL "%ProgramFiles(x86)%\Inno Setup 5\ISCC" Installer\Installer.iss"
PAUSE

:fail
exit /b 1