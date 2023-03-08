## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_rcg.la
#lib_LTLIBRARIES = librcsc_rcg.la

librcsc_rcg_la_SOURCES = \
	handler.cpp \
	parser.cpp \
	parser_v1.cpp \
	parser_v2.cpp \
	parser_v3.cpp \
	parser_v4.cpp \
	serializer.cpp \
	serializer_v1.cpp \
	serializer_v2.cpp \
	serializer_v3.cpp \
	serializer_v4.cpp \
	serializer_v5.cpp \
	serializer_v6.cpp \
	util.cpp

librcsc_rcgincludedir = $(includedir)/rcsc/rcg

#pkginclude_HEADERS
librcsc_rcginclude_HEADERS = \
	handler.h \
	parser.h \
	parser_v1.h \
	parser_v2.h \
	parser_v3.h \
	parser_v4.h \
	serializer.h \
	serializer_v1.h \
	serializer_v2.h \
	serializer_v3.h \
	serializer_v4.h \
	serializer_v5.h \
	serializer_v6.h \
	types.h \
	util.h

librcsc_rcg_la_LDFLAGS = -version-info 6:0:0
#libXXXX_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
#		 1. Start with version information of `0:0:0' for each libtool library.
#
#		 2. Update the version information only immediately before a public
#				release of your software.	 More frequent updates are unnecessary,
#				and only guarantee that the current interface number gets larger
#				faster.
#
#		 3. If the library source code has changed at all since the last
#				update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#		 4. If any interfaces have been added, removed, or changed since the
#				last update, increment CURRENT, and set REVISION to 0.
#
#		 5. If any interfaces have been added since the last public release,
#				then increment AGE.
#
#		 6. If any interfaces have been removed since the last public release,
#				then set AGE to 0.

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~

#EXTRA_DIST =
