AC_INIT([libcec], 1:6:0)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PROG_CXX
AC_PROG_LIBTOOL

has_libudev="yes"
case "${host}" in
  *-*-linux*)
    PKG_CHECK_MODULES([UDEV],[libudev],,[has_libudev="no"]; AC_MSG_WARN("library 'udev' is missing - adapter detection will not be available"))
    LIBS+=" -lrt"
    ;;
  *-apple-darwin*)
    has_libudev="no";
    LIBS+="-framework CoreVideo -framework IOKit"
    ;;
  *-freebsd*)
    has_libudev="no"
    ;;
esac

if test "x$has_libudev" != "xno"; then
  INCLUDES="$INCLUDES $UDEV_CFLAGS";LIBS="$LIBS $UDEV_LIBS"
  AC_DEFINE([HAVE_LIBUDEV],[1],["Define to 1 if libudev is installed"])
  REQUIRES="udev"
fi

AC_SEARCH_LIBS([pthread_create],[pthread],, AC_MSG_ERROR("required library 'pthread' is missing"))

libs_pre_dl=$LIBS
  AC_SEARCH_LIBS(dlopen, [dl],
    [test "$ac_cv_search_dlopen" = "none required" || LIBS_DL=$ac_cv_search_dlopen],
    AC_MSG_ERROR("required library 'dl' is missing"))
  AC_SUBST([LIBS_DL])
LIBS=$libs_pre_dl

CXXFLAGS="-fPIC -Wall -Wextra -Wno-missing-field-initializers $CXXFLAGS"

AC_SUBST(REQUIRES)
AC_CONFIG_FILES([src/lib/libcec.pc])
AC_OUTPUT([Makefile src/lib/Makefile src/testclient/Makefile src/cec-config/Makefile])
