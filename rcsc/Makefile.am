# Process this file with automake to produce Makefile.in

SUBDIRS = \
	util \
	geom \
	gz \
	param \
	rcg \
	net \
	time \
	ann \
	clang \
	formation \
	color \
	common \
	monitor \
	player \
	coach \
	trainer \
	.

lib_LTLIBRARIES = librcsc.la

librcsc_la_SOURCES =

librcsc_la_LIBADD = \
	geom/librcsc_geom.la \
	gz/librcsc_gz.la \
	net/librcsc_net.la \
	time/librcsc_time.la \
	util/librcsc_util.la \
	param/librcsc_param.la \
	ann/librcsc_ann.la \
	color/librcsc_color.la \
	clang/librcsc_clang.la \
	common/librcsc_common.la \
	rcg/librcsc_rcg.la \
	formation/librcsc_formation.la \
	monitor/librcsc_monitor.la \
	player/librcsc_player.la \
	coach/librcsc_coach.la \
	trainer/librcsc_trainer.la

#lib_LTLIBRARIES = librcsc_agent.la
#
#librcsc_agent_la_SOURCES =
#librcsc_agent_la_LIBADD = \
#	util/librcsc_util.la \
#	formation/librcsc_formation.la \
#	color/librcsc_color.la \
#	common/librcsc_common.la \
#	monitor/librcsc_monitor.la \
#	player/librcsc_player.la \
#	action/librcsc_action.la \
#	coach/librcsc_coach.la \
	trainer/librcsc_trainer.la

##	action/librcsc_action_obsolete.la

librcsc_la_LDFLAGS = -version-info 18:0:0
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

librcscincludedir = $(includedir)/rcsc
#librcsc_agenincludedir = $(includedir)/rcsc
#librcsc_agentinclude_HEADERS =

#rcscincludedir = $(includedir)/rcsc
#rcscinclude_HEADERS =
librcscinclude_HEADERS = \
	types.h \
	geom.h \
	gz.h \
	rcg.h \
	factory.h \
	game_mode.h \
	game_time.h \
	math_util.h \
	random.h \
	soccer_math.h \
	timer.h \
	version.h

CLEANFILES = *~
