project(ceccclient)
cmake_minimum_required(VERSION 2.8.9)

set(ceccclient_NAME ceccclient)
set(ceccclient_DESCRIPTION "libCEC test client")
set(ceccclient_VERSION_MAJOR ${LIBCEC_VERSION_MAJOR})
set(ceccclient_VERSION_MINOR ${LIBCEC_VERSION_MINOR})
set(ceccclient_VERSION_PATCH ${LIBCEC_VERSION_PATCH})

enable_language(C)
include(CheckCSourceCompiles)
include(CheckLibraryExists)
include(CheckIncludeFiles)

find_package(p8-platform REQUIRED)
find_package(Threads REQUIRED)

set(ceccclient_SOURCES cecc-client.c)

add_executable(cecc-client ${ceccclient_SOURCES})
set_target_properties(cecc-client PROPERTIES VERSION ${LIBCEC_VERSION_MAJOR}.${LIBCEC_VERSION_MINOR}.${LIBCEC_VERSION_PATCH})
target_link_libraries(cecc-client ${p8-platform_LIBRARIES})
target_link_libraries(cecc-client ${CMAKE_THREAD_LIBS_INIT})

if (NOT WIN32)
  # check for dlopen
  check_library_exists(dl dlopen "" HAVE_DLOPEN)
  if (HAVE_DLOPEN)
    target_link_libraries(cecc-client dl)
  endif()

  # CoreVideo
  if (APPLE)
    target_link_libraries(cecc-client "-framework CoreVideo")
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
  install(TARGETS     cecc-client
          DESTINATION .)
else()
  install(TARGETS     cecc-client
          DESTINATION bin/.)
endif()
