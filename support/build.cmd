@echo off

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET VSVERSION=12

rem build amd64
call build-platform.cmd amd64 Release %VSVERSION%
call generate-cmake.cmd amd64 nmake "%BASEDIR%" "%BUILDBASEDIR%\build64" Release %VSVERSION%
rem because of some bug in cmake, we touch this file or the build will fail
cmake -E touch "%BUILDBASEDIR%\build64\src\libcec\libcec.py"
call build-cmake.cmd amd64 "%BUILDBASEDIR%\build64" %VSVERSION%

rem build x86
call build-platform.cmd x86 Release %VSVERSION%
call generate-cmake.cmd x86 nmake "%BASEDIR%" "%BUILDBASEDIR%\build" Release %VSVERSION%
rem because of some bug in cmake, we touch this file or the build will fail
cmake -E touch "%BUILDBASEDIR%\build\src\libcec\libcec.py"
call build-cmake.cmd x86 "%BUILDBASEDIR%\build" %VSVERSION%
