AC_PREREQ(2.59)
AC_INIT([libcec], [1:8:0], [http://libcec.pulse-eight.com/])
AC_CONFIG_HEADERS([config.h])
AH_TOP([#pragma once])

AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CANONICAL_HOST

cflags_reset="$CFLAGS"
AC_LANG(C++)
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_LIBTOOL_DLOPEN
CFLAGS="$cflags_reset"

msg_pkg_config_missing="'pkg-config' is missing - adapter detection will not be available"
msg_pthread_missing="required library 'pthread' is missing"
msg_dl_missing="required library 'dl' is missing"
msg_udev_missing="library 'udev' is missing - adapter detection will not be available"
msg_dirent_missing="dirent.h header is missing - adapter detection will not be available"
msg_lockdev_missing="required library 'liblockdev' is missing"
msg_required_header_missing="required header is missing"

## debugging symbols
AC_ARG_ENABLE([debug],
  [AS_HELP_STRING([--enable-debug],
  [include debug symbols (default is no)])],
  [use_debug=$enableval],
  [use_debug=no])

## optimisation
AC_ARG_ENABLE([optimisation],
  [AS_HELP_STRING([--enable-optimisation],
  [optimisation flag (default is yes)])],
  [use_optimisation=$enableval],
  [use_optimisation=yes])

## add the top dir and include to the include path, so we can include config.h and cec.h
CPPFLAGS="$CPPFLAGS -I\$(abs_top_srcdir)/src -I\$(abs_top_srcdir)/include"

## search for pkg-config
AC_CHECK_PROG(HAVE_PKG_CONFIG, pkg-config, yes)
if test "x$HAVE_PKG_CONFIG" != "xyes" ; then
  AC_MSG_WARN($msg_pkg_config_missing)
fi

## search for pthread, required by all targets
AC_SEARCH_LIBS([pthread_create],[pthread],,AC_MSG_ERROR($msg_pthread_missing))
AC_CHECK_FUNCS([pthread_mutexattr_init pthread_cond_init pthread_cond_destroy pthread_cond_signal pthread_cond_broadcast pthread_cond_wait pthread_cond_timedwait])

## search for dlopen, required by all targets
AC_SEARCH_LIBS([dlopen], [dl],
  [test "$ac_cv_search_dlopen" = "none required" || LIBS_DL=$ac_cv_search_dlopen],
  AC_MSG_ERROR($msg_dl_missing))
AC_CHECK_FUNCS([dlopen dlcose dlsym])

## platform specific libs, required by all targets
case "${host}" in
  *-*-linux*)
    # for timeutils
    LIBS="$LIBS -lrt"
    ;;
  *-apple-darwin*)
    LIBS="$LIBS -framework CoreVideo -framework IOKit"
    ;;
esac

## we found all the libs and headers that we need for the client applications
libs_client="$LIBS"

## search for udev, lockdev and the RPi API, only required by libCEC
use_udev="no"
use_adapter_detection="yes"
case "${host}" in
  *-*-linux*)
    ## search for udev if pkg-config was found
    if test "x$HAVE_PKG_CONFIG" = "xyes" ; then
      PKG_CHECK_MODULES([UDEV],[libudev],use_udev="yes",AC_MSG_WARN($msg_udev_missing))
    fi

    ## we need dirent.h on linux too
    if test "$use_udev" = "yes"; then
      AC_CHECK_HEADER(dirent.h,,[use_udev="no";AC_MSG_WARN($msg_dirent_missing)])
    fi

    if test "$use_udev" != "yes"; then
      use_adapter_detection="no"
    fi

    ## search for lockdev
    AC_CHECK_HEADER(lockdev.h,,AC_MSG_ERROR($msg_lockdev_missing))
    AC_CHECK_LIB(lockdev,dev_unlock,,AC_MSG_ERROR($msg_lockdev_missing))

    AC_CHECK_HEADER(time.h,,AC_MSG_ERROR($msg_required_header_missing))
    AC_CHECK_HEADER(sys/prctl.h,,AC_MSG_ERROR($msg_required_header_missing))
    ;;
  *-apple-darwin*)
    AC_CHECK_HEADER(mach/mach_time.h,,AC_MSG_ERROR($msg_required_header_missing))
    AC_CHECK_HEADER(CoreVideo/CVHostTime.h,,AC_MSG_ERROR($msg_required_header_missing))
    ;;
esac

## define the build info
LIB_INFO="host: ${host}, features:"

features="Configured features:\n  Pulse-Eight CEC Adapter :\t\tyes"
LIB_INFO="$LIB_INFO 'P8 USB'"
AC_DEFINE([HAVE_P8_USB],[1],[Define to 1 to include support for the Pulse-Eight USB-CEC Adapter])
AM_CONDITIONAL(USE_P8_USB, true)

## mark adapter detection as available if the required deps were found
if test "x$use_adapter_detection" = "xyes"; then
  ## mark udev as available if it was found
  if test "x$use_udev" = "xyes"; then
    INCLUDES="$INCLUDES $UDEV_CFLAGS"
    LIBS="$LIBS $UDEV_LIBS"
    AC_DEFINE([HAVE_LIBUDEV],[1],[Define to 1 if libudev is installed])
    REQUIRES="$REQUIRES udev"
  fi

  AC_DEFINE([HAVE_P8_USB_DETECT],[1],[Define to 1 to include autodetection support for the Pulse-Eight USB-CEC Adapter])
  AM_CONDITIONAL(USE_P8_USB_DETECT, true)

  features="$features\n  Pulse-Eight CEC Adapter detection :\tyes"
  LIB_INFO="$LIB_INFO 'P8 USB detect'"
else
  AM_CONDITIONAL(USE_P8_USB_DETECT, false)
  features="$features\n  Pulse-Eight CEC Adapter detection :\tno"
fi

## check if our build system is complete
AC_CHECK_HEADER(algorithm,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(ctype.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(dlfcn.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(errno.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(fcntl.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(functional,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(locale,,AC_DEFINE([SS_NO_LOCALE],[1],[Define to 1 to exclude locale support]))
AC_CHECK_HEADER(map,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(netdb.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(poll.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(pthread.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(queue,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(semaphore.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(set,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(stdarg.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(stdint.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(stdio.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(stdlib.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(string,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(string.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(termios.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(unistd.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(vector,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(wchar.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(wctype.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(arpa/inet.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(netinet/in.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(netinet/tcp.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(sys/socket.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(sys/time.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_HEADER(sys/types.h,,AC_MSG_ERROR($msg_required_header_missing))
AC_CHECK_FUNCS([close fcntl select write read shutdown send recv memset sprintf getaddrinfo getsockopt setsockopt connect poll sched_yield open strerror tcsetattr tcgetattr cfsetispeed cfsetospeed bind freeaddrinfo listen accept socket])

## add the build date to LIB_INFO
AC_CHECK_PROG(HAVE_GIT, git, yes)
if test "x$HAVE_GIT" = "xyes"; then
  revision=$(git --no-pager log --abbrev=7 -n 1 --pretty=format:"%h" HEAD)
fi
if test "x$revision" != "x"; then
  LIB_INFO="$LIB_INFO, git revision: ${revision}"
fi

AC_CHECK_PROG(HAVE_DATE, date, yes)
if test "x$HAVE_DATE" = "xyes"; then
  LIB_INFO="$LIB_INFO, compiled on: `date -u`"
else
  LIB_INFO="$LIB_INFO, compiled on: (unknown date)"
fi

## add the name of the user who built libCEC to LIB_INFO
AC_CHECK_PROG(HAVE_WHOAMI, whoami, yes)
if test "x$HAVE_WHOAMI" = "xyes" ; then
  LIB_INFO="$LIB_INFO by `whoami`"
else
  LIB_INFO="$LIB_INFO by (unknown user)"
fi

## add the hostname of the build host of libCEC to LIB_INFO
AC_CHECK_PROG(HAVE_HOSTNAME, hostname, yes)
if test "x$HAVE_HOSTNAME" = "xyes"; then
  LIB_INFO="$LIB_INFO@`hostname -f`"
fi

## add the system info of the build host of libCEC to LIB_INFO
AC_CHECK_PROG(HAVE_UNAME, uname, yes)
if test "x$HAVE_UNAME" = "xyes"; then
  LIB_INFO="$LIB_INFO on `uname -s` `uname -r` (`uname -m`)"
fi

## redefine the LIBS, so cec-client and cec-config aren't linked against things they don't need
LIBS_LIBCEC="$LIBS"
LIBS="$libs_client"

CXXFLAGS="$CXXFLAGS -fPIC -Wall -Wextra -Wno-missing-field-initializers"

if test "x$use_debug" = "xyes"; then
  CXXFLAGS="$CXXFLAGS -g"
fi

if test "x$optimisation" = "xyes"; then
  CXXFLAGS="$CXXFLAGS -O2"
fi

AC_DEFINE_UNQUOTED(LIB_INFO,"$LIB_INFO", "information about how libCEC was compiled")

AC_SUBST([REQUIRES])
AC_SUBST([LIBS])
AC_SUBST([LIBS_LIBCEC])
AC_SUBST([LIB_INFO])
AC_SUBST([USE_P8_USB])
AC_SUBST([USE_P8_USB_DETECT])
AC_SUBST([USE_RPI_API])
AC_CONFIG_FILES([src/lib/libcec.pc])
AC_OUTPUT([Makefile src/lib/Makefile src/testclient/Makefile src/cec-config/Makefile])

cat <<EOB

##############################################################################

libCEC version $VERSION configured

Compilation flags:
  CXXFLAGS :       $CXXFLAGS
  libCEC LDFLAGS : $LIBS_LIBCEC
  client LDFLAGS : $LIBS

EOB

echo -e "$features"

cat <<EOB

You can now build libCEC by running:
  make

##############################################################################

EOB
