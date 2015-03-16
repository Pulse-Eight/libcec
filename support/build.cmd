@echo off

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET SCRIPTSDIR=%BASEDIR%\cmake

call build-platform.cmd
cls

rem build amd64
cd "%BASEDIR%\support"
call "%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
call generate-cmake.cmd amd64 nmake "%BASEDIR%" "%BUILDBASEDIR%\build64"
pause
cd "%BUILDBASEDIR%\build64"
nmake install
pause

rem build x86
cd "%BASEDIR%\support"
call "%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
call generate-cmake.cmd x86 nmake "%BASEDIR%" "%BUILDBASEDIR%\build"
cd "%BUILDBASEDIR%\build"
nmake install
