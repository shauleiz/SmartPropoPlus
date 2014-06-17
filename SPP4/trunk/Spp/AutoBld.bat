@ECHO ON
CALL "%VS110COMNTOOLS%"..\..\vc\bin\vcvars32.bat
CALL msbuild  /t:Rebuild /p:Configuration=Release;Platform=Win32
CALL msbuild  /t:Rebuild /p:Configuration=Release;Platform=x64 
CALL "%ProgramFiles(x86)%\Inno Setup 5\ISCC" Installer\Installer.iss"
PAUSE

