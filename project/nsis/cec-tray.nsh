Section "libCEC Tray" SecTray
  SetShellVarContext current
  SectionIn 1

  ; Copy to the installation directory
  SetOutPath "$INSTDIR\x86\netfx"
  File "..\build\x86\cec-tray.exe"
  SetOutPath "$INSTDIR\x64\netfx"
  File /nonfatal "..\build\amd64\cec-tray.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath "$INSTDIR"

  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  ${If} ${RunningX64}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk" "$INSTDIR\x64\netfx\cec-tray.exe" \
      "" "$INSTDIR\x64\netfx\cec-tray.exe" 0 SW_SHOWNORMAL \
      "" "Start libCEC Tray (x64)."
  ${Else}
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\cec-tray.lnk" "$INSTDIR\x86\netfx\cec-tray.exe" \
      "" "$INSTDIR\netfx\cec-tray.exe" 0 SW_SHOWNORMAL \
      "" "Start libCEC Tray."
  ${EndIf}
  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd
