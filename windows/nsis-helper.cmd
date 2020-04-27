@ECHO OFF

rem NSIS helper script. Used by create-installer.cmd

SETLOCAL

SET MYDIR=%~dp0

IF [%2] == [] (
  ECHO.This script should not be called manually.
  PAUSE
  EXIT /b 99
)

rem Check for NSIS
IF EXIST "%ProgramFiles%\NSIS\makensis.exe" (
  SET NSIS="%ProgramFiles%\NSIS\makensis.exe"
) ELSE IF EXIST "%ProgramFiles(x86)%\NSIS\makensis.exe" (
  SET NSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
) ELSE (
  ECHO.*** NSIS could not be found ***
  ECHO.
  ECHO.See docs\README.windows.md
  EXIT /b 2
)

SET NSISPROJECT="%1"
SET RESULTMATCH=%2
SET RESULTMATCH=%RESULTMATCH:"=%
IF [%3] == [] (
  SET NSISOPTS=""
) ELSE (
  SET NSISOPTS=%3
)
SET NSISOPTS=%NSISOPTS:"=%

ECHO. * creating installer "%1"
CD "%MYDIR%..\project"
DEL /F /Q "%MYDIR%..\build\%RESULTMATCH%" >nul 2>&1
%NSIS% /V1 %NSISOPTS% %NSISPROJECT%

FOR /F "delims=" %%F IN ('dir /b /s "%MYDIR%..\build\%RESULTMATCH%" 2^>nul') DO SET INSTALLER=%%F
IF [%INSTALLER%] == [] (
  ECHO. *** the installer could not be created ***
  ECHO.
  ECHO. The most likely cause is that something went wrong while compiling.
  EXIT /B 3
)

rem Sign the installer if sign-binary.cmd exists
IF EXIST "..\support\private\sign-binary.cmd" (
  ECHO. * signing installer binary
  CALL ..\support\private\sign-binary.cmd "%INSTALLER%" >nul
)

ECHO.installer built: %INSTALLER%
