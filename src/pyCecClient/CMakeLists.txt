project(pyCecClient)
cmake_minimum_required(VERSION 2.8.9)

# Python
include(FindPythonLibs)
find_package(PythonLibs)

if (PYTHONLIBS_FOUND)
  if (WIN32)
    install(FILES pyCecClient.py
            DESTINATION python/.)
  else()
    install(FILES pyCecClient.py
            DESTINATION bin/.)
  endif()
endif()
