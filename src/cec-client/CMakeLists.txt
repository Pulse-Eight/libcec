project(cecclient)
cmake_minimum_required(VERSION 2.6)

set(cecclient_NAME cecclient)
set(cecclient_DESCRIPTION "libCEC test client")
set(cecclient_VERSION_MAJOR ${LIBCEC_VERSION_MAJOR})
set(cecclient_VERSION_MINOR ${LIBCEC_VERSION_MINOR})
set(cecclient_VERSION_PATCH ${LIBCEC_VERSION_PATCH})

enable_language(CXX)

find_package(platform REQUIRED)
find_package(Threads REQUIRED)

set(cecclient_SOURCES cec-client.cpp)

add_executable(cec-client ${cecclient_SOURCES})
target_link_libraries(cec-client ${platform_LIBRARIES})
target_link_libraries(cec-client ${CMAKE_THREAD_LIBS_INIT})

if (NOT WIN32)
  # check for dlopen
  check_library_exists(dl dlopen "" HAVE_DLOPEN)
  if (HAVE_DLOPEN)
    target_link_libraries(cec-client dl)
  endif()
else()
  add_definitions(-DTARGET_WINDOWS -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS -D_WINSOCKAPI_)
  if (NOT ${WIN64})
    add_definitions(-D_USE_32BIT_TIME_T)
  endif()
endif()

include_directories(${platform_INCLUDE_DIRS}
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
