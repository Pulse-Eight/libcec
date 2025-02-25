Section "libCEC Tray" SecTray
	SetShellVarContext current
	SectionIn 1

	; Copy binaries
	SetOutPath "$INSTDIR\netfx"
	File "${BINARY_SOURCE_DIR}\cec-tray.exe"

	; Start menu item
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	SetShellVarContext all
	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk" "$INSTDIR\netfx\cec-tray.exe" \
		"" "$INSTDIR\netfx\cec-tray.exe" 0 SW_SHOWNORMAL \
		"" "Start libCEC Tray."
	!insertmacro MUI_STARTMENU_WRITE_END
SectionEnd
