;libCEC installer
;Copyright (C) 2012 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

Name "Pulse-Eight USB-CEC Adapter"
OutFile "..\build\libCEC-installer.exe"

XPStyle on
InstallDir "$PROGRAMFILES\Pulse-Eight\USB-CEC Adapter"
InstallDirRegKey HKLM "Software\Pulse-Eight\USB-CEC Adapter software" ""
RequestExecutionLevel admin
Var StartMenuFolder

!define MUI_FINISHPAGE_LINK "Visit http://www.pulse-eight.com/ for more information."
!define MUI_FINISHPAGE_LINK_LOCATION "http://www.pulse-eight.com/"
!define MUI_ABORTWARNING  

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Pulse-Eight\USB-CEC Adapter sofware" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder  

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

InstType "USB-CEC driver & libCEC"
InstType "USB-CEC driver only"
InstType "Full installation"

Section "USB-CEC driver" SecDriver
  SetShellVarContext current
  SectionIn RO
  SectionIn 1 2 3

  ; Uninstall the old unsigned software if it's found
  ReadRegStr $1 HKCU "Software\libCEC" ""
  ${If} $1 != ""
    MessageBox MB_OK \
	  "A previous libCEC and USB-CEC driver was found. This update requires the old version to be uninstalled. Press OK to uninstall the old version."
    ExecWait '"$1\Uninstall.exe" /S _?=$1'
	Delete "$1\Uninstall.exe"
	RMDir "$1"
  ${EndIf}

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\AUTHORS"
  File "..\COPYING"

  ; Copy the driver installer
  SetOutPath "$INSTDIR\driver"
  File "..\build\p8-usbcec-driver-installer.exe"

  ;Store installation folder
  WriteRegStr HKLM "Software\Pulse-Eight\USB-CEC Adapter software" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall Pulse-Eight USB-CEC Adapter software.lnk" "$INSTDIR\Uninstall.exe" \
    "" "$INSTDIR\Uninstall.exe" 0 SW_SHOWNORMAL \
    "" "Uninstall Pulse-Eight USB-CEC Adapter software."

  WriteINIStr "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url" "InternetShortcut" "URL" "http://www.pulse-eight.com/"
  !insertmacro MUI_STARTMENU_WRITE_END  
    
  ;add entry to add/remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "DisplayName" "Pulse-Eight USB-CEC Adapter software"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "NoRepair" 1
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "DisplayIcon" "$INSTDIR\cec-client.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "Publisher" "Pulse-Eight Limited"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "HelpLink" "http://www.pulse-eight.com/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" \
                 "URLInfoAbout" "http://www.pulse-eight.com"

  ;install driver
  ExecWait '"$INSTDIR\driver\p8-usbcec-driver-installer.exe" /S'
  Delete "$INSTDIR\driver\p8-usbcec-driver-installer.exe"
SectionEnd

Section "libCEC" SecLibCec
  SetShellVarContext current
  SectionIn 1 3

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\ChangeLog"
  File "..\README"
  File "..\build\*.dll"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\x64\*.dll"

  ; Copy to XBMC\system
  ReadRegStr $1 HKCU "Software\XBMC" ""
  ${If} $1 != ""
    SetOutPath "$1\system"
	File "..\build\libcec.dll"
  ${EndIf}

  ; Copy the headers
  SetOutPath "$INSTDIR\include"
  File /r /x *.so "..\include\cec*.*"

  ; Copy libcec.dll and libcec.x64.dll to the system directory
  SetOutPath "$SYSDIR"
  File "..\build\libcec.dll"
  ${If} ${RunningX64}
    File /nonfatal "..\build\x64\libcec.x64.dll"
  ${EndIf}
SectionEnd

Section "CEC debug client" SecCecClient
  SetShellVarContext current
  SectionIn 3

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File /x p8-usbcec-driver-installer.exe "..\build\*.exe"
  SetOutPath "$INSTDIR\x64"
  File /nonfatal "..\build\x64\*.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk" "$INSTDIR\cec-client.exe" \
    "" "$INSTDIR\cec-client.exe" 0 SW_SHOWNORMAL \
    "" "Start the CEC Test client."
  ${If} ${RunningX64}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\CEC Test client (x64).lnk" "$INSTDIR\x64\cec-client.x64.exe" \
      "" "$INSTDIR\cec-client.x64.exe" 0 SW_SHOWNORMAL \
      "" "Start the CEC Test client (x64)."
  ${EndIf}
  !insertmacro MUI_STARTMENU_WRITE_END  
    
SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetShellVarContext current

  Delete "$INSTDIR\AUTHORS"
  Delete "$INSTDIR\cec*.exe"
  Delete "$INSTDIR\ChangeLog"
  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\*.lib"
  Delete "$INSTDIR\x64\*.dll"
  Delete "$INSTDIR\x64\*.lib"
  Delete "$INSTDIR\README"

  ; Uninstall the driver
  ReadRegStr $1 HKLM "Software\Pulse-Eight\USB-CEC Adapter driver" ""
  ${If} $1 != ""
    ExecWait '"$1\Uninstall.exe" /S _?=$1'
  ${EndIf}

  RMDir /r "$INSTDIR\include"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"
  RMDir "$PROGRAMFILES\Pulse-Eight"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client.lnk"
  ${If} ${RunningX64}
    Delete "$SMPROGRAMS\$StartMenuFolder\CEC Test client (x64).lnk"
  ${EndIf}
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall Pulse-Eight USB-CEC Adapter software.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Visit Pulse-Eight.url"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter driver"
  DeleteRegKey /ifempty HKLM "Software\Pulse-Eight\USB-CEC Adapter software"
  DeleteRegKey /ifempty HKLM "Software\Pulse-Eight"
SectionEnd
