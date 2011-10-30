@echo off

echo Downloading boost
bin\wget.exe "http://mirrors.xbmc.org/build-deps/win32/boost-1_46_1-xbmc-win32.7z"

echo Extracting boost
bin\7za.exe "boost-1_46_1-xbmc-win32.7z"

echo Copying boost
xcopy boost-1_46_1-xbmc-win32\include\* "..\include\" /E /Q /I /Y
xcopy boost-1_46_1-xbmc-win32\lib\* "%CUR_PATH%\" /E /Q /I /Y

