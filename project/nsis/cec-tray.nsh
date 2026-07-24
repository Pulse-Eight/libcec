Section "libCEC Tray" SecTray
	SetShellVarContext current
	SectionIn 1

	; Copy binaries (net8.0; relies on the "libCEC for .Net" section for
	; LibCecSharp.dll + cec.dll in the same folder)
	SetOutPath "$INSTDIR\net8.0"
	File "${BINARY_SOURCE_DIR}\net8.0\cec-tray.exe"
	File "${BINARY_SOURCE_DIR}\net8.0\cec-tray.dll"
	File "${BINARY_SOURCE_DIR}\net8.0\cec-tray.deps.json"
	File "${BINARY_SOURCE_DIR}\net8.0\cec-tray.runtimeconfig.json"

	; Start menu item
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk" "$INSTDIR\net8.0\cec-tray.exe" \
		"" "$INSTDIR\net8.0\cec-tray.exe" 0 SW_SHOWNORMAL \
		"" "Start libCEC Tray."
	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd
