@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build

rem generate visual studio project files
call build-platform.cmd amd64 Release
call generate-cmake.cmd amd64 vs "%BASEDIR%" "%BUILDBASEDIR%\build64" Release

call build-platform.cmd x86 Release
call generate-cmake.cmd x86 vs "%BASEDIR%" "%BUILDBASEDIR%\build" Release

rem cls
echo Visual Studio solutions can be found in:
echo 32 bits: %BASEDIR%\cmake-build\build\libcec.sln
echo 64 bits: %BASEDIR%\cmake-build\build64\libcec.sln
pause