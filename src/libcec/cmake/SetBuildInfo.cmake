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
  find_program(HAVE_GIT_BIN git /bin /usr/bin /usr/local/bin)
  if(HAVE_GIT_BIN)
    exec_program(${CMAKE_CURRENT_SOURCE_DIR}/cmake/git-rev.sh HEAD OUTPUT_VARIABLE GIT_REVISION)
    message(STATUS "git found: ${GIT_REVISION}")
  endif()
  if (GIT_REVISION)
    set(LIB_INFO "git revision: ${GIT_REVISION},")
  endif()

  # add compilation date to compile info
  STRING(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S" UTC)
  set(LIB_INFO "${LIB_INFO} compiled on ${BUILD_DATE}")

  # add user who built this to compile info
  find_program(HAVE_WHOAMI_BIN whoami /bin /usr/bin /usr/local/bin)
  if(HAVE_WHOAMI_BIN)
    exec_program(whoami OUTPUT_VARIABLE BUILD_USER)
    set(LIB_INFO "${LIB_INFO} by ${BUILD_USER}")
  else()
    set(LIB_INFO "${LIB_INFO} by (unknown user)")
  endif()


  # add host on which this was built to compile info
  find_program(HAVE_HOSTNAME_BIN hostname /bin /usr/bin /usr/local/bin)
  if(HAVE_HOSTNAME_BIN)
    exec_program(hostname ARGS -f OUTPUT_VARIABLE BUILD_HOST RETURN_VALUE RETURN_HOST)
    if (RETURN_HOST)
      exec_program(hostname OUTPUT_VARIABLE BUILD_HOST)
    endif()
    set(LIB_INFO "${LIB_INFO}@${BUILD_HOST}")
  endif()

  # add host info on which this was built to compile info
  find_program(HAVE_UNAME_BIN uname /bin /usr/bin /usr/local/bin)
  if(HAVE_UNAME_BIN)
    exec_program(uname ARGS -s OUTPUT_VARIABLE BUILD_SYSNAME)
    exec_program(uname ARGS -r OUTPUT_VARIABLE BUILD_SYSVER)
    exec_program(uname ARGS -m OUTPUT_VARIABLE BUILD_SYSARCH)

    set(LIB_INFO "${LIB_INFO} on ${BUILD_SYSNAME} ${BUILD_SYSVER} (${BUILD_SYSARCH})")
  endif()

endif()

