!ifndef NSISINCLUDEPDB
!ifndef INNER

Var EventGhostLocation

!define EVENTGHOST_SECTIONNAME "EventGhost plugin"
Section "" SecEvGhostCec
	SetShellVarContext current
	SectionIn 1

	SetOutPath "$INSTDIR\EventGhost"
	File "..\build\EventGhost\pulse_eight.egplugin"

	${If} $EventGhostLocation != ""
		ExecWait '"$EventGhostLocation\eventghost.exe" "$INSTDIR\EventGhost\pulse_eight.egplugin"'
	${EndIf}
SectionEnd

Function EventGhost
	ReadRegDword $1 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\EventGhost_is1" "InstallLocation"
	${If} $1 != ""
		StrCpy $EventGhostLocation "$1"
		!insertMacro SelectSection ${SecEvGhostCec}
		SectionSetText ${SecEvGhostCec} "${EVENTGHOST_SECTIONNAME}"
	${Else}
		!insertMacro UnSelectSection ${SecEvGhostCec}
	${Endif}
FunctionEnd

!endif
!endif