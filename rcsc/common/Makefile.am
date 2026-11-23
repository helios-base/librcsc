## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_common.la

librcsc_common_la_SOURCES = \
	abstract_client.cpp \
	audio_codec.cpp \
	audio_memory.cpp \
	logger.cpp \
	offline_client.cpp \
	online_client.cpp \
	player_param.cpp \
	player_type.cpp \
	say_message_parser.cpp \
	server_param.cpp \
	soccer_agent.cpp \
	stamina_model.cpp \
	team_graphic.cpp

librcsc_commonincludedir = $(includedir)/rcsc/common

librcsc_commoninclude_HEADERS = \
	abstract_client.h \
	audio_codec.h \
	audio_memory.h \
	audio_message.h \
	free_message_parser.h \
	freeform_message.h \
	freeform_message_parser.h \
	logger.h \
	offline_client.h \
	online_client.h \
	player_param.h \
	player_type.h \
	say_message.h \
	say_message_parser.h \
	server_param.h \
	soccer_agent.h \
	stamina_model.h \
	team_graphic.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~

if UNIT_TEST
TESTS = \
	run_test_server_param
endif

check_PROGRAMS = $(TESTS)

# librcsc.la is built after this directory. Link the convenience libraries used by ServerParam.
run_test_server_param_SOURCES = test_server_param.cpp
run_test_server_param_CXXFLAGS = $(CPPUNIT_CFLAGS) -Wall -W
run_test_server_param_LDADD = \
	librcsc_common.la \
	$(top_builddir)/rcsc/rcg/librcsc_rcg.la \
	$(top_builddir)/rcsc/param/librcsc_param.la \
	$(top_builddir)/rcsc/geom/librcsc_geom.la \
	$(CPPUNIT_LIBS)
