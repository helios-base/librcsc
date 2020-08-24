## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_clang.la
#lib_LTLIBRARIES = librcsc_clang.la

librcsc_clang_la_SOURCES = \
	clang_action.cpp \
	clang_condition.cpp \
	clang_directive.cpp \
	clang_info_message.cpp \
	clang_parser.cpp \
	clang_token.cpp \
	clang_unum.cpp

librcsc_clangincludedir = $(includedir)/rcsc/clang

librcsc_clanginclude_HEADERS = \
	clang_action.h \
	clang_condition.h \
	clang_directive.h \
	clang_info_message.h \
	clang_message.h \
	clang_parser.h \
	clang_token.h \
	clang_unum.h \
	clang.h \
	types.h


librcsc_clang_la_LDFLAGS = -version-info 0:0:0
#libXXXX_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
#    1. Start with version information of `0:0:0' for each libtool library.
#
#    2. Update the version information only immediately before a public
#       release of your software.  More frequent updates are unnecessary,
#       and only guarantee that the current interface number gets larger
#       faster.
#
#    3. If the library source code has changed at all since the last
#       update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#    4. If any interfaces have been added, removed, or changed since the
#       last update, increment CURRENT, and set REVISION to 0.
#
#    5. If any interfaces have been added since the last public release,
#       then increment AGE.
#
#    6. If any interfaces have been removed since the last public release,
#       then set AGE to 0.

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~


if UNIT_TEST
TESTS = \
	test_clang_parser
endif

check_PROGRAMS = $(TESTS)

test_clang_parser_SOURCES = test_clang_parser.cpp
test_clang_parser_CXXFLAGS = $(CPPUNIT_CFLAGS) -W -Wall
test_clang_parser_LDFLAGS = -L$(top_builddir)/rcsc/clang -L$(top_builddir)/rcsc/time -L$(top_builddir)/rcsc/geom
test_clang_parser_LDADD = -lrcsc_clang -lrcsc_time -lrcsc_geom $(CPPUNIT_LIBS)
