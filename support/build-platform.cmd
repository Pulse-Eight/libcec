@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build

rem x86 platform lib
call "%VS120COMNTOOLS%..\..\VC\bin\vcvars32.bat"
call generate-cmake.cmd x86 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform" static
CD "%BUILDBASEDIR%\buildplatform"
nmake install

rem amd64 platform lib
CD "%BASEDIR%\support"
call "%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
call generate-cmake.cmd amd64 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform64" static
CD "%BUILDBASEDIR%\buildplatform64"
nmake install
