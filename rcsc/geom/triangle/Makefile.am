## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_geom_triangle.la

librcsc_geom_triangle_la_SOURCES = \
	triangle.c

noinst_HEADERS = \
	triangle.h

librcsc_geom_triangle_la_CPPFLAGS =
librcsc_geom_triangle_la_CFLAGS = -DTRILIBRARY -DREDUCED -DCDT_ONLY -DVOID=int -DREAL=double
librcsc_geom_triangle_la_CXXFLAGS = -DTRILIBRARY -DREDUCED -DCDT_ONLY -DVOID=int -DREAL=double

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~

EXTRA_DIST = \
	A.poly \
	README \
	makefile.org \
	showme.c \
	tricall.c
