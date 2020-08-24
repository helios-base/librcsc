## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_color.la

librcsc_color_la_SOURCES = \
	rgb_color.cpp \
	gradation_color_provider.cpp \
	gray_scale_provider.cpp \
	thermo_color_provider.cpp

librcsc_colorincludedir = $(includedir)/rcsc/color

librcsc_colorinclude_HEADERS = \
	rgb_color.h \
	gradation_color_provider.h \
	gray_scale_provider.h \
	thermo_color_provider.h


AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~ thermo_color_test


if UNIT_TEST
noinst_PROGRAMS = thermo_color_test
endif

thermo_color_test_SOURCES = thermo_color_test.cpp $(librcsc_color_la_SOURCES)
thermo_color_test_CXXFLAGS = $(AM_CXXFLAGS)
thermo_color_test_LDFLAGS = -L$(top_builddir)/rcsc/color $(AM_LDFLAGS)
