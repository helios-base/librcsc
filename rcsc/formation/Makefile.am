## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_formation.la

librcsc_formation_la_SOURCES = \
	formation.cpp \
	formation_data.cpp \
	formation_parser.cpp \
	formation_parser_csv.cpp \
	formation_parser_json.cpp \
	formation_parser_static.cpp \
	formation_parser_v1.cpp \
	formation_parser_v2.cpp \
	formation_parser_v3.cpp \
	formation_dt.cpp \
	formation_static.cpp \
	role_type.cpp

#	formation_bpn.cpp
#	formation_cdt.cpp
#	formation_knn.cpp
#	formation_ngnet.cpp
#	formation_rbf.cpp
#	formation_sbsp.cpp
#	formation_uva.cpp


librcsc_formationincludedir = $(includedir)/rcsc/formation

librcsc_formationinclude_HEADERS = \
	formation.h \
	formation_data.h \
	formation_parser.h \
	formation_parser_csv.h \
	formation_parser_json.h \
	formation_parser_static.h \
	formation_parser_v1.h \
	formation_parser_v2.h \
	formation_parser_v3.h \
	formation_dt.h \
	formation_static.h \
	role_type.h

#	formation_bpn.h
#	formation_cdt.h
#	formation_knn.h
#	formation_ngnet.h
#	formation_rbf.h
#	formation_sbsp.h
#	formation_uva.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~

EXTRA_DIST = \
	test_data/pairs_one_way.conf \
	test_data/pairs_reversed.conf \
	test_data/pairs_reversed_v3.conf

if UNIT_TEST
TESTS = \
	run_test_formation
endif

check_PROGRAMS = $(TESTS)

# the formation files for the tests are in test_data/
run_test_formation_SOURCES = test_formation.cpp
run_test_formation_CPPFLAGS = $(AM_CPPFLAGS) -DTEST_DATA_DIR=\"$(abs_srcdir)/test_data\"
run_test_formation_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_formation_LDADD = \
	librcsc_formation.la \
	$(top_builddir)/rcsc/geom/librcsc_geom.la \
	$(CPPUNIT_LIBS)
