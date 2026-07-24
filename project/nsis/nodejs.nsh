Section "libCEC for Node.js" SecNodeJs
	SectionIn 1 2

	; Prebuilt native N-API addon (ABI-stable, so it works with any Node >= 16)
	; plus the JavaScript wrapper. cec.dll sits next to the addon: Windows loads a
	; .node with LOAD_WITH_ALTERED_SEARCH_PATH, so its dependencies resolve from
	; its own directory without the install dir being on PATH.
	SetOutPath "$INSTDIR\nodejs\build\Release"
	File "${BINARY_SOURCE_DIR}\nodejs\build\Release\cec_native.node"
	File "${BINARY_SOURCE_DIR}\cec.dll"

	SetOutPath "$INSTDIR\nodejs\lib"
	File "${BINARY_SOURCE_DIR}\nodejs\lib\*.js"

	SetOutPath "$INSTDIR\nodejs\client"
	File "${BINARY_SOURCE_DIR}\nodejs\client\*.js"

	SetOutPath "$INSTDIR\nodejs\example"
	File "${BINARY_SOURCE_DIR}\nodejs\example\*.js"

	SetOutPath "$INSTDIR\nodejs"
	File "${BINARY_SOURCE_DIR}\nodejs\package.json"
	File "${BINARY_SOURCE_DIR}\nodejs\README.md"
SectionEnd
