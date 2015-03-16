@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build

IF "%1" == "amd64" (
  call generate-cmake.cmd amd64 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform64" Release static
  call build-cmake.cmd amd64 "%BUILDBASEDIR%\buildplatform64"
)
IF NOT "%1" == "amd64" (
  call generate-cmake.cmd x86 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform" Release static
  call build-cmake.cmd x86 "%BUILDBASEDIR%\buildplatform"
)
