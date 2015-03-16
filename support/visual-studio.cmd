@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build

call build-platform.cmd

rem generate visual studio project files
CD "%BASEDIR%\support"
call "%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
call generate-cmake.cmd amd64 vs "%BASEDIR%" "%BUILDBASEDIR%\build64"
call "%VS120COMNTOOLS%..\..\VC\bin\vcvars32.bat"
call generate-cmake.cmd x86 vs "%BASEDIR%" "%BUILDBASEDIR%\build"

cls
echo Visual Studio solutions can be found in:
echo 32 bits: %BASEDIR%\cmake-build\build\libcec.sln
echo 64 bits: %BASEDIR%\cmake-build\build64\libcec.sln
pause