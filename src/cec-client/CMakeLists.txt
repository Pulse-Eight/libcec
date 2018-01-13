project(cecclient)
cmake_minimum_required(VERSION 2.8.9)

set(cecclient_NAME cecclient)
set(cecclient_DESCRIPTION "libCEC test client")
set(cecclient_VERSION_MAJOR ${LIBCEC_VERSION_MAJOR})
set(cecclient_VERSION_MINOR ${LIBCEC_VERSION_MINOR})
set(cecclient_VERSION_PATCH ${LIBCEC_VERSION_PATCH})

enable_language(CXX)
include(CheckCXXSourceCompiles)
include(CheckLibraryExists)
include(CheckIncludeFiles)
include(CheckCXXCompilerFlag)

check_cxx_compiler_flag("-std=c++11" SUPPORTS_CXX11)
if (SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

find_package(p8-platform REQUIRED)
find_package(Threads REQUIRED)

set(cecclient_SOURCES cec-client.cpp)

# curses
check_library_exists(curses initscr "" HAVE_CURSES_API)
if (HAVE_CURSES_API)
  list(APPEND cecclient_SOURCES curses/CursesControl.cpp)

  # tinfo
  find_library(HAVE_CURSES_TINFO tinfo)
endif()


add_executable(cec-client ${cecclient_SOURCES})
set_target_properties(cec-client PROPERTIES VERSION ${LIBCEC_VERSION_MAJOR}.${LIBCEC_VERSION_MINOR}.${LIBCEC_VERSION_PATCH})
target_link_libraries(cec-client ${p8-platform_LIBRARIES})
target_link_libraries(cec-client ${CMAKE_THREAD_LIBS_INIT})

if (NOT WIN32)
  # check for dlopen
  check_library_exists(dl dlopen "" HAVE_DLOPEN)
  if (HAVE_DLOPEN)
    target_link_libraries(cec-client dl)
  endif()

  # curses
  if (HAVE_CURSES_API)
    target_link_libraries(cec-client curses)
    if (HAVE_CURSES_TINFO)
      target_link_libraries(cec-client tinfo)
    endif()
  endif()

  # rt
  check_library_exists(rt clock_gettime "" HAVE_RT)
  if (HAVE_RT)
    target_link_libraries(cec-client rt)
  endif()

  # CoreVideo
  if (APPLE)
    target_link_libraries(cec-client "-framework CoreVideo")
  endif()
else()
  add_definitions(-DTARGET_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINSOCKAPI_)
  if (NOT ${WIN64})
    add_definitions(-D_USE_32BIT_TIME_T)
  endif()
endif()

include_directories(${p8-platform_INCLUDE_DIRS}
                    ${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/../../include)

# write env.h
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/env.h.in ${CMAKE_CURRENT_SOURCE_DIR}/env.h)

if (WIN32)
  install(TARGETS     cec-client
          DESTINATION .)
else()
  install(TARGETS     cec-client
          DESTINATION bin/.)
endif()
