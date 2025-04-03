;libCEC EventGhost Plugin installer
;Copyright (C) 2025 Pulse-Eight Ltd.
;http://www.pulse-eight.com/

XPStyle on
RequestExecutionLevel user
SetCompressor /SOLID lzma

!include "LogicLib.nsh"
!include "MUI2.nsh"
!include "nsDialogs.nsh"
!include "nsis\libcec-version.nsh"

Name "Pulse-Eight libCEC v${LIBCEC_VERSION_STRING} EventGhost Plugin"
OutFile "..\build\libcec-eventghost-plugin-${LIBCEC_VERSION_STRING}.exe"
InstType "Full installation"

Var EventGhostLocation

Section "EventGhost plugin" SecEvGhostCec
	SetShellVarContext current
	SectionIn 1
	SectionIn RO

	SetOutPath "$%TEMP%\"
	File "..\build\EventGhost\pulse_eight.egplugin"

	ExecWait '"$EventGhostLocation\eventghost.exe" "$%TEMP%\pulse_eight.egplugin"'
	Delete "$%TEMP%\pulse_eight.egplugin"
SectionEnd

Function EventGhost
	ReadRegDword $1 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EventGhost_is1" "InstallLocation"
	${If} $1 != ""
		StrCpy $EventGhostLocation "$1"
	${Else}
		MessageBox MB_OK "EventGhost is not installed. Exiting."
		Quit
	${Endif}
FunctionEnd

!define MUI_FINISHPAGE_LINK "Visit https://libcec.pulse-eight.com/ for more information."
!define MUI_FINISHPAGE_LINK_LOCATION "https://libcec.pulse-eight.com/"
!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE.md"
!insertmacro MUI_PAGE_COMPONENTS

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Function .onInit
	; check for EventGhost
	Call EventGhost
FunctionEnd
