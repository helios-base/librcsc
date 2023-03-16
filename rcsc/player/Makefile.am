## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_player.la

librcsc_player_la_SOURCES = \
	abstract_player_object.cpp \
	action_effector.cpp \
	audio_sensor.cpp \
	ball_object.cpp \
	body_sensor.cpp \
	debug_client.cpp \
	fullstate_sensor.cpp \
	intercept.cpp \
	intercept_simulator_player.cpp \
	intercept_simulator_self_v17.cpp \
	intercept_table.cpp \
	localization_default.cpp \
	object_table.cpp \
	penalty_kick_state.cpp \
	player_command.cpp \
	player_agent.cpp \
	player_config.cpp \
	player_object.cpp \
	player_state.cpp \
	say_message_builder.cpp \
	see_state.cpp \
	self_object.cpp \
	soccer_action.cpp \
	view_grid_map.cpp \
	view_mode.cpp \
	visual_sensor.cpp \
	world_model.cpp

librcsc_playerincludedir = $(includedir)/rcsc/player

librcsc_playerinclude_HEADERS = \
	abstract_player_object.h \
	action_effector.h \
	audio_sensor.h \
	ball_object.h \
	body_sensor.h \
	debug_client.h \
	free_message.h \
	fullstate_sensor.h \
	intercept.h \
	intercept_simulator_player.h \
	intercept_simulator_self.h \
	intercept_simulator_self_v17.h \
	intercept_table.h \
	localization.h \
	localization_default.h \
	object_table.h \
	penalty_kick_state.h \
	player_command.h \
	player_agent.h \
	player_config.h \
	player_evaluator.h \
	player_object.h \
	player_predicate.h \
	player_state.h \
	say_message_builder.h \
	see_state.h \
	self_object.h \
	soccer_action.h \
	soccer_intention.h \
	view_area.h \
	view_grid_map.h \
	view_mode.h \
	visual_sensor.h \
	world_model.h

AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~
