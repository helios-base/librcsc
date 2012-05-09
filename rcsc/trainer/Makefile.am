## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_trainer.la

librcsc_trainer_la_SOURCES = \
	trainer_agent.cpp \
	trainer_command.cpp \
	trainer_config.cpp

librcsc_trainerincludedir = $(includedir)/rcsc/trainer

librcsc_trainerinclude_HEADERS = \
	trainer_agent.h \
	trainer_command.h \
	trainer_config.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~
