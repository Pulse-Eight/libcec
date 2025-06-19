# - Set information about the system on which this was built in LIB_INFO
#
# This module sets the following variables
#       LIB_INFO                  supported features and compilation information
#

if(WIN32)

  # Windows
  set(LIB_INFO "compiled using MSVC ${CMAKE_CXX_COMPILER_VERSION}")

else()
  # not Windows
  set(LIB_INFO "")

  # add git revision to compile info
  find_program(GIT_BIN git /bin /usr/bin /usr/local/bin)
  if(GIT_BIN)
    execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cmake/git-rev.sh OUTPUT_VARIABLE GIT_REVISION)
    string(STRIP ${GIT_REVISION} GIT_REVISION)
    message(STATUS "git found: ${GIT_REVISION}")
  endif()
  if (GIT_REVISION)
    set(LIB_INFO "git revision: ${GIT_REVISION},")
  endif()

  # add compilation date to compile info
  STRING(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)
  set(LIB_INFO "${LIB_INFO} compiled on ${BUILD_DATE}")

  # add user who built this to compile info
  find_program(WHOAMI_BIN whoami /bin /usr/bin /usr/local/bin)
  if (DEFINED ENV{SOURCE_DATE_EPOCH})
    set(BUILD_USER "(reproducible)")
  else()
    if(WHOAMI_BIN)
      execute_process(COMMAND "${WHOAMI_BIN}" OUTPUT_VARIABLE BUILD_USER)
      string(STRIP ${BUILD_USER} BUILD_USER)
      set(LIB_INFO "${LIB_INFO} by ${BUILD_USER}")
    else()
      set(LIB_INFO "${LIB_INFO} by (unknown user)")
    endif()
  endif()

  # add host on which this was built to compile info
  if (DEFINED ENV{SOURCE_DATE_EPOCH})
    set(BUILD_HOST "(reproducible)")
  else()
    find_program(HOSTNAME_BIN hostname /bin /usr/bin /usr/local/bin)
    if(HOSTNAME_BIN)
      execute_process(COMMAND ${HOSTNAME_BIN} -f OUTPUT_VARIABLE BUILD_HOST RESULT_VARIABLE RETURN_HOST)
      if (RETURN_HOST)
        execute_process(COMMAND ${HOSTNAME_BIN} OUTPUT_VARIABLE BUILD_HOST)
      endif()
      string(STRIP ${BUILD_HOST} BUILD_HOST)
      set(LIB_INFO "${LIB_INFO}@${BUILD_HOST}")
    endif()
  endif()

  # add host info on which this was built to compile info
  find_program(UNAME_BIN uname /bin /usr/bin /usr/local/bin)
  if(UNAME_BIN)
    execute_process(COMMAND "${UNAME_BIN}" -s OUTPUT_VARIABLE BUILD_SYSNAME)
    execute_process(COMMAND "${UNAME_BIN}" -r OUTPUT_VARIABLE BUILD_SYSVER)
    execute_process(COMMAND "${UNAME_BIN}" -m OUTPUT_VARIABLE BUILD_SYSARCH)
    string(STRIP ${BUILD_SYSNAME} BUILD_SYSNAME)
    string(STRIP ${BUILD_SYSVER} BUILD_SYSVER)
    string(STRIP ${BUILD_SYSARCH} BUILD_SYSARCH)

    set(LIB_INFO "${LIB_INFO} on ${BUILD_SYSNAME} ${BUILD_SYSVER} (${BUILD_SYSARCH})")
  endif()

endif()

