# /Z7, not /Zi: /Zi routes debug info through mspdbsrv.exe, a per-user
# singleton shared by every cl.exe. With /MP and more than one build on an
# agent it falls over and compiles die with "fatal error C1090: PDB API call
# failed, error code '23'". /Z7 puts the debug info in the .obj instead; the
# linker still writes cec.pdb where cmake links with /DEBUG.
if(MSVC)
  set(CMAKE_CXX_FLAGS "/MP /DWIN32 /D_WINDOWS /W3 /GR /Z7 /EHsc /arch:SSE2")
  set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Ob0 /Od /RTC1 /D_HAS_ITERATOR_DEBUGGING=0 /D_SECURE_SCL=0")
  set(CMAKE_CXX_FLAGS_RELEASE "/MD /Ox /Ob2 /Oi /Ot /Oy /GL /DNDEBUG")
endif(MSVC)