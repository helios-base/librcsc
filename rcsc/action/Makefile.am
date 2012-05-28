## Process this file with automake to produce Makefile.in

noinst_LTLIBRARIES = librcsc_action.la
## librcsc_action_obsolete.la

librcsc_action_la_SOURCES = \
	basic_actions.cpp \
	bhv_before_kick_off.cpp \
	bhv_emergency.cpp \
	bhv_go_to_point_look_ball.cpp \
	bhv_scan_field.cpp \
	body_advance_ball2009.cpp \
	body_clear_ball2009.cpp \
	body_dribble2008.cpp \
	body_go_to_point.cpp \
	body_go_to_point_dodge.cpp \
	body_hold_ball2008.cpp \
	body_intercept2009.cpp \
	body_kick_one_step.cpp \
	body_kick_to_relative.cpp \
	body_pass.cpp \
	body_smart_kick.cpp \
	body_stop_ball.cpp \
	body_stop_dash.cpp \
	intention_dribble2008.cpp \
	intention_time_limit_action.cpp \
	neck_scan_field.cpp \
	neck_scan_players.cpp \
	neck_turn_to_ball_and_player.cpp \
	neck_turn_to_ball_or_scan.cpp \
	neck_turn_to_goalie_or_scan.cpp \
	neck_turn_to_player_or_scan.cpp \
	neck_turn_to_low_conf_teammate.cpp \
	view_synch.cpp \
	kick_table.cpp

## librcsc_action_obsolete_la_SOURCES = \
##	obsolete/bhv_shoot2008.cpp \
## 	obsolete/body_advance_ball2007.cpp \
## 	obsolete/body_clear_ball2007.cpp \
## 	obsolete/body_dribble2006.cpp \
## 	obsolete/body_dribble2007.cpp \
## 	obsolete/body_go_to_point2007.cpp \
## 	obsolete/body_go_to_point2009.cpp \
## 	obsolete/body_go_to_point2010.cpp \
## 	obsolete/body_hold_ball2007.cpp \
## 	obsolete/body_intercept2007.cpp \
## 	obsolete/body_intercept2008.cpp \
## 	obsolete/body_kick_multi_step.cpp \
## 	obsolete/body_kick_two_step.cpp \
## 	obsolete/intention_dribble2006.cpp \
## 	obsolete/intention_dribble2007.cpp \
## 	obsolete/intention_kick2007.cpp \
##	obsolete/shoot_table2008.cpp

librcsc_actionincludedir = $(includedir)/rcsc/action

## librcsc_action_obsoleteincludedir = $(includedir)/rcsc/action/obsolete

librcsc_actioninclude_HEADERS = \
	basic_actions.h \
	arm_off.h \
	arm_point_to_point.h \
	bhv_before_kick_off.h \
	bhv_body_neck_to_ball.h \
	bhv_body_neck_to_point.h \
	bhv_emergency.h \
	bhv_go_to_point_look_ball.h \
	bhv_neck_body_to_ball.h \
	bhv_neck_body_to_point.h \
	bhv_scan_field.h \
	body_advance_ball.h \
	body_advance_ball2009.h \
	body_clear_ball.h \
	body_clear_ball2009.h \
	body_dribble.h \
	body_dribble2008.h \
	body_go_to_point.h \
	body_go_to_point_dodge.h \
	body_hold_ball.h \
	body_hold_ball2008.h \
	body_intercept.h \
	body_intercept2009.h \
	body_kick_one_step.h \
	body_kick_to_relative.h \
	body_pass.h \
	body_smart_kick.h \
	body_stop_ball.h \
	body_stop_dash.h \
	body_tackle_to_point.h \
	body_turn_to_angle.h \
	body_turn_to_ball.h \
	body_turn_to_point.h \
	intention_dribble2008.h \
	intention_time_limit_action.h \
	neck_scan_field.h \
	neck_scan_players.h \
	neck_turn_to_ball.h \
	neck_turn_to_ball_and_player.h \
	neck_turn_to_ball_or_scan.h \
	neck_turn_to_goalie_or_scan.h \
	neck_turn_to_player_or_scan.h \
	neck_turn_to_low_conf_teammate.h \
	neck_turn_to_point.h \
	neck_turn_to_relative.h \
	view_change_width.h \
	view_normal.h \
	view_synch.h \
	view_wide.h \
	kick_table.h

## librcsc_action_obsoleteinclude_HEADERS = \
##	obsolete/bhv_shoot.h \
##	obsolete/bhv_shoot2008.h \
## 	obsolete/body_advance_ball2007.h \
## 	obsolete/body_clear_ball2007.h \
## 	obsolete/body_dribble2006.h \
## 	obsolete/body_dribble2007.h \
## 	obsolete/body_go_to_point2007.h \
## 	obsolete/body_go_to_point2009.h \
## 	obsolete/body_go_to_point2010.h \
## 	obsolete/body_hold_ball2007.h \
## 	obsolete/body_intercept2007.h \
## 	obsolete/body_intercept2008.h \
## 	obsolete/body_kick_multi_step.h \
## 	obsolete/body_kick_two_step.h \
## 	obsolete/intention_dribble2006.h \
## 	obsolete/intention_dribble2007.h \
## 	obsolete/intention_kick2007.h \
##	obsolete/shoot_table2008.h


AM_CPPFLAGS = -I$(top_srcdir)
AM_CFLAGS = -Wall -W
AM_CXXFLAGS = -Wall -W
AM_LDFLAGS =

CLEANFILES = *~
