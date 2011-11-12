AC_INIT([libcec], 1:2:0)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PROG_CXX
AC_PROG_LIBTOOL

case "${host}" in
  *-*-linux*)
    AC_SEARCH_LIBS([udev_new], [udev],, AC_MSG_ERROR("required library 'udev' is missing"))
    REQUIRES="udev"
    ;;
  *-apple-darwin*)
    LIBS+="-framework CoreVideo -framework IOKit"
    ;;
esac

AC_SEARCH_LIBS([pthread_create],[pthread],, AC_MSG_ERROR("required library 'pthread' is missing"))

libs_pre_dl=$LIBS
  AC_SEARCH_LIBS(dlopen, [dl],
    [test "$ac_cv_search_dlopen" = "none required" || LIBS_DL=$ac_cv_search_dlopen],
    AC_MSG_ERROR("required library 'dl' is missing"))
  AC_SUBST([LIBS_DL])
LIBS=$libs_pre_dl

CXXFLAGS="-fPIC -Wall -Wextra $CXXFLAGS"

AC_SUBST(REQUIRES)
AC_CONFIG_FILES([src/lib/libcec.pc])
AC_OUTPUT([Makefile src/lib/Makefile src/testclient/Makefile])
