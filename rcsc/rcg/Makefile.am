## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = librcsc_rcg.la
#lib_LTLIBRARIES = librcsc_rcg.la

librcsc_rcg_la_SOURCES = \
	simdjson/simdjson.cpp \
	handler.cpp \
	line_parser.cpp \
	parser.cpp \
	parser_v1.cpp \
	parser_v2.cpp \
	parser_v3.cpp \
	parser_v4.cpp \
	parser_simdjson.cpp \
	serializer.cpp \
	serializer_v1.cpp \
	serializer_v2.cpp \
	serializer_v3.cpp \
	serializer_v4.cpp \
	serializer_v5.cpp \
	serializer_v6.cpp \
	serializer_json.cpp \
	util.cpp \
	types.cpp

librcsc_rcgincludedir = $(includedir)/rcsc/rcg

#pkginclude_HEADERS
librcsc_rcginclude_HEADERS = \
	handler.h \
	parser.h \
	parser_v1.h \
	parser_v2.h \
	parser_v3.h \
	parser_v4.h \
	parser_simdjson.h \
	serializer.h \
	serializer_v1.h \
	serializer_v2.h \
	serializer_v3.h \
	serializer_v4.h \
	serializer_v5.h \
	serializer_v6.h \
	serializer_json.h \
	types.h \
	util.h

noinst_HEADERS = \
	line_parser.h \
	simdjson/simdjson.h

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
# the parsers use std::thread (line_parser.cpp)
AM_CXXFLAGS = -Wall -W -pthread
AM_LDFLAGS = -pthread

EXTRA_DIST = \
	simdjson/LICENSE \
	test_data/head_json.rcg \
	test_data/head_v5.rcg \
	test_data/head_v6.rcg

CLEANFILES = *~

if UNIT_TEST
TESTS = \
	run_test_rcg_types \
	run_test_rcg_parser \
	run_test_rcg_line_parser
endif

check_PROGRAMS = $(TESTS)

run_test_rcg_types_SOURCES = test_types.cpp
run_test_rcg_types_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W -pthread
run_test_rcg_types_LDADD = librcsc_rcg.la ../gz/librcsc_gz.la $(LIBDEFLATE_LIBS) $(CPPUNIT_LIBS)

# the test data are the first records of real game logs (test_data/)
run_test_rcg_parser_SOURCES = test_parser.cpp
run_test_rcg_parser_CPPFLAGS = $(AM_CPPFLAGS) -DTEST_DATA_DIR=\"$(abs_srcdir)/test_data\"
run_test_rcg_parser_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W -pthread
run_test_rcg_parser_LDADD = librcsc_rcg.la ../gz/librcsc_gz.la $(LIBDEFLATE_LIBS) $(CPPUNIT_LIBS)

# the internal functions in line_parser.h
run_test_rcg_line_parser_SOURCES = test_line_parser.cpp
run_test_rcg_line_parser_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W -pthread
run_test_rcg_line_parser_LDADD = librcsc_rcg.la ../gz/librcsc_gz.la $(LIBDEFLATE_LIBS) $(CPPUNIT_LIBS)

#EXTRA_DIST =
