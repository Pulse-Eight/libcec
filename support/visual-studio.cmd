@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
set BUILDMODE=Debug
SET VSVERSION=12

IF "%1" == "Release" (
  set BUILDMODE=Release
)

rem generate visual studio project files
call build-platform.cmd amd64 %BUILDMODE% %VSVERSION%
call generate-cmake.cmd amd64 vs "%BASEDIR%" "%BUILDBASEDIR%\build64" %BUILDMODE% %VSVERSION%

rem TODO
rem call build-platform.cmd arm %BUILDMODE% %VSVERSION%
rem call generate-cmake.cmd arm vs "%BASEDIR%" "%BUILDBASEDIR%\buildarm" %BUILDMODE% %VSVERSION%

call build-platform.cmd x86 %BUILDMODE% %VSVERSION%
call generate-cmake.cmd x86 vs "%BASEDIR%" "%BUILDBASEDIR%\build" %BUILDMODE% %VSVERSION%

echo Visual Studio solutions can be found in:
echo 32 bits: %BASEDIR%\cmake-build\build\libcec.sln
echo 64 bits: %BASEDIR%\cmake-build\build64\libcec.sln
echo.
echo These projects only compile in %BUILDMODE% mode
pause