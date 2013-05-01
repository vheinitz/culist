@REM Makes a release build 
@REM Valentin Heinitz, 2013-05-01


set PATH=%PATH%;"C:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\";"C:\Program Files (x86)\Inno Setup 5"
set MSVS=devenv.com


call createvcprj.bat
@cd ..
@%MSVS% culist.sln /build Release
@cd scripts
@REM Requires Inno Setup
@iscc.exe "CULIST.iss"
pause

