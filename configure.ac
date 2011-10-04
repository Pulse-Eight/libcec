AC_INIT([libcec], 0:4:0)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_PROG_CXX
AC_PROG_LIBTOOL

AC_CHECK_LIB([rt], [main],, AC_MSG_ERROR("required library 'rt' is missing"))
AC_CHECK_LIB([pthread], [main],, AC_MSG_ERROR("required library 'pthread' is missing"))
AC_CHECK_LIB([udev], [main],, AC_MSG_ERROR("required library 'udev' is missing"))

CXXFLAGS="-fPIC -Wall -Wextra $CXXFLAGS"

AC_CONFIG_FILES([src/lib/libcec.pc])
AC_OUTPUT([Makefile src/lib/Makefile src/testclient/Makefile])
