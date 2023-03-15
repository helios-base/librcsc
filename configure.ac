#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)
LT_PREREQ(2.2)
AC_INIT([librcsc],[2023],[akky@users.sourceforge.jp])

#LT_PREREQ(2.2.4)

AC_CONFIG_SRCDIR([config.h.in])
#AM_CONFIG_HEADER([config.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])

#
#AM_INIT_AUTOMAKE([gnu check-news])
AM_INIT_AUTOMAKE([gnu])

# it is necessary to check c++ header files.
AC_LANG([C++])

##################################################
# libtool settings
##################################################

LT_INIT([shared disable-static])
LT_LANG([C++])
# AC_PROG_LIBTOOL
# AC_ENABLE_SHARED
# AC_DISABLE_STATIC
AC_SUBST(LIBTOOL_DEPS)

##################################################
# Checks for programs.
##################################################

AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

##################################################
# Checks for libraries.
##################################################

AC_CHECK_LIB([m], [cos],
             [LIBS="-lm $LIBS"],
             [AC_MSG_ERROR([*** -lm not found! ***])])
libz="yes"
AC_CHECK_LIB([z], [deflate],
             [AC_DEFINE([HAVE_LIBZ], [1],
                        [Define to 1 if you have the `z' library (-lz).])
              LIBS="-lz $LIBS"],
             [libz="no"])

##################################################
# Checks for header files.
##################################################

AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([fcntl.h],
                 break,
                 [AC_MSG_ERROR([*** fcntl.h not found ***])])
AC_CHECK_HEADERS([netinet/in.h],
                 break,
                 [AC_MSG_ERROR([*** netinet/in.h not found ***])])
AC_CHECK_HEADERS([netdb.h],
                 break,
                 [AC_MSG_ERROR([*** netdb.h not found ***])])
AC_CHECK_HEADERS([sys/socket.h],
                 break,
                 [AC_MSG_ERROR([*** sys/socket.h not found ***])])
AC_CHECK_HEADERS([sys/time.h],
                 break,
                 [AC_MSG_ERROR([*** sys/time.h not found ***])])
AC_CHECK_HEADERS([unistd.h],
                 break,
                 [AC_MSG_ERROR([*** unistd.h not found ***])])

##################################################
# Checks for types.
##################################################

AC_CHECK_TYPES(socklen_t,,
               AC_DEFINE(socklen_t, int, [Define missing socklen_t.]), [
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include <sys/socket.h>])

##################################################
# Checks for typedefs, structures, and compiler characteristics.
##################################################

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
#AC_TYPE_INT16_T
#AC_TYPE_INT32_T
#AC_TYPE_INT64_T
#AC_TYPE_UINT32_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

##################################################
# Checks for library functions.
##################################################
AC_HEADER_STDC
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor inet_addr getaddrinfo gethostbyname gettimeofday])
AC_CHECK_FUNCS([memset pow rint select socket sqrt strerror strtol])

##################################################
# check C++
##################################################

AX_CXX_COMPILE_STDCXX_17(noext)

##################################################
# check boost
##################################################

AX_BOOST_BASE([1.41.0])
AX_BOOST_SYSTEM
CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"

##################################################
# enable/disable unit test
##################################################

AC_ARG_ENABLE(unit-test,
              AS_HELP_STRING([--enable-unit-test],[build unit test code. (default=no)]))
if test "x$enable_unit_test" = "xyes"; then
  AC_MSG_NOTICE(enabled unit test)
#  AX_BOOST_UNIT_TEST_FRAMEWORK
  AM_PATH_CPPUNIT([1.12.0],
                  [],
                  [AC_MSG_ERROR([*** CppUnit not found! ***])])
  AM_CONDITIONAL(UNIT_TEST, [test "1" = "1"])
else
  AM_CONDITIONAL(UNIT_TEST, [test "1" = "0"])
fi

##################################################
# enable/disable debug mode
##################################################

AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug],[turn on debugging. DEBUG is defined in preprocessor. (default=no)]))
if test "x$enable_debug" = "xyes"; then
  AC_MSG_NOTICE(enabled debug)
#  AC_DEFINE([DEBUG], [1], [debugging flag])
  CFLAGS="-DDEBUG $CFLAGS"
  CXXFLAGS="-DDEBUG $CXXFLAGS"
fi


##################################################
# enable/disable example code
##################################################

#example code
AC_ARG_ENABLE(example,
              AS_HELP_STRING([--enable-example],[build example code (default=no)]))
if test "x$enable_example" = "xyes"; then
  AC_MSG_NOTICE(enabled example build)
  AM_CONDITIONAL(BUILD_EXAMPLE, [test "1" = "1"])
else
  AM_CONDITIONAL(BUILD_EXAMPLE, [test "1" = "0"])
fi

##################################################
# check external tools
##################################################

#AC_CHECK_PROG([DOXYGEN], [doxygen], [yes], [no])
#AM_CONDITIONAL(DOXYGEN, test x$DOXYGEN = xyes)
AC_SUBST(DOXYGEN)
AC_SUBST(HAVE_DOT)
AC_PATH_PROG([DOXYGEN], [doxygen], [AC_MSG_NOTICE(doxygen not found)])
AC_CHECK_PROG([HAVE_DOT], [dot], [YES], [NO])

##################################################
# generated files
##################################################

AC_CONFIG_FILES([Doxyfile
                 librcsc.pc
                 librcsc.spec
                 librcsc-config
                 librcscenv
                 Makefile
                 rcsc/Makefile
                 rcsc/ann/Makefile
                 rcsc/clang/Makefile
                 rcsc/color/Makefile
                 rcsc/geom/Makefile
                 rcsc/geom/triangle/Makefile
                 rcsc/gz/Makefile
                 rcsc/monitor/Makefile
                 rcsc/net/Makefile
                 rcsc/param/Makefile
                 rcsc/rcg/Makefile
                 rcsc/time/Makefile
                 rcsc/util/Makefile
                 rcsc/common/Makefile
                 rcsc/player/Makefile
                 rcsc/formation/Makefile
                 rcsc/coach/Makefile
                 rcsc/trainer/Makefile
                 example/Makefile
                 src/Makefile],
                 [test -f librcsc-config && chmod +x librcsc-config
                  test -f librcscenv && chmod +x librcscenv])

AC_OUTPUT
