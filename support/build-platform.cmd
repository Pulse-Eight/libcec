@ECHO OFF

SETLOCAL

rem set paths
SET BASEDIR=%CD%\..
SET BUILDBASEDIR=%BASEDIR%\cmake-build
SET BUILDARCH=%1
SET BUILDTYPE=%2
SET VSVERSION=%3

echo --------------------------------------
echo Building platform library:
echo Architecture = %BUILDARCH%
echo Build type = %BUILDTYPE%
echo Visual Studio version = %VSVERSION%
echo --------------------------------------

IF "%BUILDARCH%" == "amd64" (
  call generate-cmake.cmd amd64 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform64" %BUILDTYPE% %VSVERSION% static
  call build-cmake.cmd amd64 "%BUILDBASEDIR%\buildplatform64" %VSVERSION%
) ELSE (
  IF "%BUILDARCH%" == "arm" (
    call generate-cmake.cmd arm nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatformarm" %BUILDTYPE% %VSVERSION% static
    call build-cmake.cmd arm "%BUILDBASEDIR%\buildplatformarm" %VSVERSION%
  ) ELSE (
    call generate-cmake.cmd x86 nmake "%BASEDIR%\src\platform" "%BUILDBASEDIR%\buildplatform" %BUILDTYPE% %VSVERSION% static
    call build-cmake.cmd x86 "%BUILDBASEDIR%\buildplatform" %VSVERSION%
  )
)
