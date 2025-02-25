!include LogicLib.nsh
!include FileFunc.nsh

Var oldVersionDir

!macro !defineifexist _VAR_NAME _FILE_NAME
	!tempfile _TEMPFILE
	!ifdef NSIS_WIN32_MAKENSIS
		; Windows - cmd.exe
		!system 'if exist "${_FILE_NAME}" echo !define ${_VAR_NAME} > "${_TEMPFILE}"'
	!else
		; Posix - sh
		!system 'if [ -e "${_FILE_NAME}" ]; then echo "!define ${_VAR_NAME}" > "${_TEMPFILE}"; fi'
	!endif
	!include '${_TEMPFILE}'
	!delfile '${_TEMPFILE}'
	!undef _TEMPFILE
!macroend
!define !defineifexist "!insertmacro !defineifexist"

Function DeleteOldFiles
	RMDir /r "$INSTDIR\EventGhost"
	RMDir /r "$INSTDIR\include"
	RMDir /r "$INSTDIR\python"
	RMDir /r "$INSTDIR\x64"
	RMDir /r "$INSTDIR\x86"
	Delete "$INSTDIR\AUTHORS"
	Delete "$INSTDIR\cec.dll"
	Delete "$INSTDIR\cec.pdb"
	Delete "$INSTDIR\cecc-client.exe"
	Delete "$INSTDIR\cec-client.exe"
	Delete "$INSTDIR\cec-firmware-latest.exe"
	Delete "$INSTDIR\ChangeLog"
	Delete "$INSTDIR\COPYING"
	Delete "$INSTDIR\libusb0.dll"
	Delete "$INSTDIR\LICENSE.md"
	Delete "$INSTDIR\README.*"
	Delete "$INSTDIR\tv_on.cmd"
	Delete "$INSTDIR\tv_off.cmd"
	Delete "$INSTDIR\Uninstall.exe"
	Delete "$INSTDIR\uninstall_libcec.exe"
FunctionEnd

Function UninstallOldVersion
	ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware" "UninstallString"
	${If} $0 != ""
		${If} ${Cmd} `MessageBox MB_YESNO|MB_ICONQUESTION "An old libCEC installation was found. This version must be uninstalled before installing libCEC v${LIBCEC_VERSION_STRING}. Continue?" /SD IDYES IDYES`
			${GetParent} "$0" $oldVersionDir
			ExecWait '"$0" _?=$oldVersionDir' $0
			${If} $0 <> 0
				MessageBox MB_OK "Failed to uninstall"
				Quit
			${Else}
				; old uninstaller was bugged and didn't always uninstall
				Call DeleteOldFiles
				DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Pulse-Eight USB-CEC Adapter sofware"
				DeleteRegKey HKLM "Software\Pulse-Eight\USB-CEC Adapter sofware"
				DeleteRegKey /ifempty HKLM "Software\Pulse-Eight"
				SetShellVarContext current
				RMDir /r "$SMPROGRAMS\Pulse-Eight libCEC v6.0.2"
				SetShellVarContext all
				RMDir /r "$SMPROGRAMS\Pulse-Eight libCEC v6.0.2"
			${EndIf}
		${Else}
			Quit
		${EndIf}
	${EndIf}
FunctionEnd
