## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_monitor.la

librcsc_monitor_la_SOURCES = \
	monitor_command.cpp

librcsc_monitorincludedir = $(includedir)/rcsc/monitor

##pkginclude_HEADERS
librcsc_monitorinclude_HEADERS = \
	monitor_command.h

#librcsc_monitor_la_LDFLAGS = -version-info 0:0:0
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

#EXTRA_DIST =
