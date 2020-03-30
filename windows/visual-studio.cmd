@ECHO OFF

rem Generate Visual Studio projects for libCEC
rem Usage: build-all.cmd [visual studio version]

SETLOCAL

rem optional parameter: visual studio version (2019)
IF "%1" == "" (
  SET VSVERSION=2019
) ELSE (
  SET VSVERSION=%1
)

SET MYDIR=%~dp0
SET BUILDTYPE=Debug
SET INSTALLPATH=%MYDIR%..\build

rem delete old build folder
RMDIR /s /q "%MYDIR%..\build" >nul 2>&1

rem build/generate vs project files
FOR %%T IN (amd64 x86) DO (
  CALL "%MYDIR%build-lib.cmd" %%T %BUILDTYPE% %VSVERSION% "%INSTALLPATH%" vs
  IF %errorlevel% neq 0 EXIT /b %errorlevel%
)

ECHO Visual Studio solutions can be found in:
ECHO 32 bits: "%MYDIR%..\build\cmake\x86\libcec.sln"
ECHO 64 bits: "%MYDIR%..\build\cmake\amd64\libcec.sln"
ECHO.
ECHO These projects only compile in %BUILDTYPE% mode and have been generated for Visual Studio %VSVERSION%.
