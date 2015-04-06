@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
set BUILDMODE=Debug

IF "%1" == "Release" (
  set BUILDMODE=Release
)

rem generate visual studio project files
call build-platform.cmd amd64 %BUILDMODE%
call generate-cmake.cmd amd64 vs "%BASEDIR%" "%BUILDBASEDIR%\build64" %BUILDMODE%

call build-platform.cmd x86 %BUILDMODE%
call generate-cmake.cmd x86 vs "%BASEDIR%" "%BUILDBASEDIR%\build" %BUILDMODE%

rem cls
echo Visual Studio solutions can be found in:
echo 32 bits: %BASEDIR%\cmake-build\build\libcec.sln
echo 64 bits: %BASEDIR%\cmake-build\build64\libcec.sln
echo.
echo These projects only compile in %BUILDMODE% mode
pause