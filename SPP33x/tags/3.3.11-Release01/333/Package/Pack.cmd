@echo off

REM Create packages for SPP distribution
REM Created on 2/12/07

"C:\Program Files\7-Zip\7z.exe" a  -tzip  SmartPropoPlus_3.x.y_package.zip @SmartPropoPlus_3.x.y_package.txt
"C:\Program Files\7-Zip\7z.exe" a  -tzip  SmartPropoPlus_3.x.y_PPJ_package.zip @SmartPropoPlus_3.x.y_PPJ_package.txt

Pause

