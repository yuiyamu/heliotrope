AC_INIT([heliotrope], [1.2.1], [yuiyamuki@gmail.com], [heliotrope-1.2.1], [https://github.com/yuiyamu/heliotrope])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST

AC_USE_SYSTEM_EXTENSIONS
CFLAGS="" # wipes out default -O2 -g
AC_PROG_CC
LT_INIT

CFLAGS="$CFLAGS -std=gnu99"
if test "x$enable_debug" = "xyes"; then
    CFLAGS="$CFLAGS -O0 -g -fsanitize=address"
else
    CFLAGS="$CFLAGS -Os -g"
fi

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [build with debug flags/symbols~ >w<!!]), [], [enable_debug=no])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

