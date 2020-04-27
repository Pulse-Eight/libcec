project(pyCecClient)
cmake_minimum_required(VERSION 3.12.0)

# Python
if(WIN32 AND "${MSVC_C_ARCHITECTURE_ID}" STREQUAL "X86")
  set(PYTHON_USE_VERSION 2)
endif()

if(PYTHON_USE_VERSION EQUAL 2)
  include(FindPython2)
  find_package(Python2)
  set(PYTHONLIBS_FOUND "${Python2_FOUND}")
else()
  include(FindPythonLibs)
  find_package(PythonLibs)
endif()

if(PYTHONLIBS_FOUND)
  if (WIN32)
    install(PROGRAMS pyCecClient.py
            DESTINATION python/.)
  else()
    install(PROGRAMS pyCecClient.py
            DESTINATION bin/.
            RENAME pyCecClient)
  endif()
endif()
