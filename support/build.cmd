@echo off

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build

rem build amd64
call build-platform.cmd amd64 Release
call generate-cmake.cmd amd64 nmake "%BASEDIR%" "%BUILDBASEDIR%\build64" Release
call build-cmake.cmd amd64 "%BUILDBASEDIR%\build64"

rem build x86
call build-platform.cmd x86 Release
call generate-cmake.cmd x86 nmake "%BASEDIR%" "%BUILDBASEDIR%\build" Release
call build-cmake.cmd x86 "%BUILDBASEDIR%\build"
