@echo off

echo Downloading boost
bin\wget.exe "http://mirrors.xbmc.org/build-deps/win32/boost-1_46_1-xbmc-win32.7z"

echo Extracting boost
bin\7za.exe x -y "boost-1_46_1-xbmc-win32.7z"

echo Copying boost
xcopy boost-1_46_1-xbmc-win32\include\* "..\include\" /E /Q /I /Y

echo Cleaning up
del boost-1_46_1-xbmc-win32.7z
