@echo off

set NSIS="C:\Program Files (x86)\NSIS\makensis.exe"
set DDK=C:\WinDDK\7600.16385.1

IF "%VS100COMNTOOLS%"=="" (
  set COMPILER="%ProgramFiles%\Microsoft Visual Studio 10.0\Common7\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS100COMNTOOLS%\..\IDE\VCExpress.exe" (
  set COMPILER="%VS100COMNTOOLS%\..\IDE\VCExpress.exe"
) ELSE IF EXIST "%VS100COMNTOOLS%\..\IDE\devenv.exe" (
  set COMPILER="%VS100COMNTOOLS%\..\IDE\devenv.exe"
)

rem Compile libCEC
echo Cleaning libCEC
%COMPILER% libcec.sln /clean Release

echo Compiling libCEC
%COMPILER% libcec.sln /build Release /project libcec
echo Compiling cec-client
%COMPILER% libcec.sln /build Release /project testclient
echo Compiling LibCecSharp
%COMPILER% libcec.sln /build Release /project LibCecSharp

rem Copy driver installer
echo Copying driver installer
copy "%DDK%\redist\DIFx\dpinst\MultiLin\amd64\dpinst.exe" ..\dpinst-amd64.exe
copy "%DDK%\redist\DIFx\dpinst\MultiLin\x86\dpinst.exe" ..\dpinst-x86.exe

rem Run the NSIS installer
echo Creating the installer
%NSIS% /V1 /X"SetCompressor /FINAL lzma" "libCEC.nsi"

echo The installer can be found here: libCEC-installer.exe