AC_PREREQ(2.59)
AC_INIT([libcec], [1:7:0], [http://libcec.pulse-eight.com/])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PROG_CXX
AC_PROG_LIBTOOL

# search for pthread, required by all targets
AC_SEARCH_LIBS([pthread_create],[pthread],,
  AC_MSG_ERROR("required library 'pthread' is missing"))

# search for dlopen, required by all targets
AC_SEARCH_LIBS([dlopen], [dl],
  [test "$ac_cv_search_dlopen" = "none required" || LIBS_DL=$ac_cv_search_dlopen],
  AC_MSG_ERROR("required library 'dl' is missing"))


# platform specific libs, required by all targets
case "${host}" in
  *-*-linux*)
    LIBS+=" -lrt"
    ;;
  *-apple-darwin*)
    LIBS+="-framework CoreVideo -framework IOKit"
    ;;
  *-freebsd*)
    ;;
esac

libs_client=$LIBS

# search for udev and lockdev, only required by libCEC
has_libudev="yes"
case "${host}" in
  *-*-linux*)
    PKG_CHECK_MODULES([UDEV],[libudev],,
      [has_libudev="no"]; AC_MSG_WARN("library 'udev' is missing - adapter detection will not be available"))

    AC_CHECK_HEADER(lockdev.h,,
      AC_MSG_ERROR("required library 'liblockdev' is missing"))

    LIBS+=" -llockdev"
    ;;
  *-apple-darwin*)
    has_libudev="no";
    ;;
  *-freebsd*)
    has_libudev="no"
    ;;
esac

# mark udev as available if it was found, so we can include adapter autodetection code
if test "x$has_libudev" != "xno"; then
  INCLUDES="$INCLUDES $UDEV_CFLAGS";LIBS="$LIBS $UDEV_LIBS"
  AC_DEFINE([HAVE_LIBUDEV],[1],["Define to 1 if libudev is installed"])
  REQUIRES="udev"
fi

LIBS_LIBCEC=$LIBS
LIBS=$libs_client

CXXFLAGS="-fPIC -Wall -Wextra -Wno-missing-field-initializers $CXXFLAGS"

AC_SUBST([REQUIRES])
AC_SUBST([LIBS])
AC_SUBST([LIBS_LIBCEC])
AC_CONFIG_FILES([src/lib/libcec.pc])
AC_OUTPUT([Makefile src/lib/Makefile src/testclient/Makefile src/cec-config/Makefile])
