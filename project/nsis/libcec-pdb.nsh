Section "libCEC debug symbols" SecPDB
  SetShellVarContext current
  SectionIn 1

  ; Copy to the installation directory
  SetOutPath "$INSTDIR"
  File "..\build\x86\lib\cec.pdb"
  SetOutPath "$INSTDIR\netfx"
  File "..\build\x86\LibCecSharp.pdb"
  SetOutPath "$INSTDIR\netcore"
  File "..\build\x86\netcore\LibCecSharpCore.pdb"
  File "..\build\x86\netcore\CecSharpCoreTester.pdb"

  SetOutPath "$INSTDIR\x64"
  File "..\build\amd64\lib\cec.pdb"
  SetOutPath "$INSTDIR\x64\netfx"
  File "..\build\amd64\LibCecSharp.pdb"
  SetOutPath "$INSTDIR\x64\netcore"
  File "..\build\amd64\netcore\LibCecSharpCore.pdb"
  File "..\build\amd64\netcore\CecSharpCoreTester.pdb"
SectionEnd
