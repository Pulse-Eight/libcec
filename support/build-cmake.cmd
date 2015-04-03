@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDDIR=%2

rem set Visual C++ build environment
IF "%1" == "amd64" (
  echo Compiling for win64
  call "%VS120COMNTOOLS%..\..\VC\bin\amd64\vcvars64.bat"
)
IF NOT "%1" == "amd64" (
  echo Compiling for win32
  call "%VS120COMNTOOLS%..\..\VC\bin\vcvars32.bat"
)

rem go into the build directory
CD "%BUILDDIR%"
nmake install