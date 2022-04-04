// -*-c++-*-

/*!
  \file server_param.cpp
  \brief server parametor Source File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "server_param.h"

#include <rcsc/param/param_map.h>
#include <rcsc/param/rcss_param_parser.h>

#include <rcsc/rcg/types.h>
#include <rcsc/rcg/util.h>

#include <sstream>
#include <iostream>
#include <cmath>
#include <cassert>

namespace rcsc {

const int    ServerParam::DEFAULT_MAX_PLAYER = 11;
const double ServerParam::DEFAULT_PITCH_LENGTH = 105.0;
const double ServerParam::DEFAULT_PITCH_WIDTH = 68.0;
const double ServerParam::DEFAULT_PITCH_MARGIN = 5.0;
const double ServerParam::DEFAULT_CENTER_CIRCLE_R = 9.15;
const double ServerParam::DEFAULT_PENALTY_AREA_LENGTH = 16.5;
const double ServerParam::DEFAULT_PENALTY_AREA_WIDTH = 40.32;
const double ServerParam::DEFAULT_PENALTY_CIRCLE_R = 9.15;
const double ServerParam::DEFAULT_PENALTY_SPOT_DIST = 11.0;
const double ServerParam::DEFAULT_GOAL_AREA_LENGTH = 5.5;
const double ServerParam::DEFAULT_GOAL_AREA_WIDTH = 18.32;
const double ServerParam::DEFAULT_GOAL_DEPTH = 2.44;
const double ServerParam::DEFAULT_CORNER_ARC_R = 1.0;
//const double ServerParam::DEFAULT_KICK_OFF_CLEAR_DISTANCE = ServerParam::CENTER_CIRCLE_R;
const double ServerParam::DEFAULT_GOAL_POST_RADIUS = 0.06;

const double ServerParam::DEFAULT_WIND_WEIGHT = 10000.0;


const double ServerParam::DEFAULT_GOAL_WIDTH = 14.02;
const double ServerParam::DEFAULT_INERTIA_MOMENT = 5.0;

const double ServerParam::DEFAULT_PLAYER_SIZE = 0.3;
const double ServerParam::DEFAULT_PLAYER_DECAY = 0.4;
const double ServerParam::DEFAULT_PLAYER_RAND = 0.1;
const double ServerParam::DEFAULT_PLAYER_WEIGHT = 60.0;
const double ServerParam::DEFAULT_PLAYER_SPEED_MAX = 1.05; // [13.0.0] 1.2 -> 1.05
const double ServerParam::DEFAULT_PLAYER_ACCEL_MAX = 1.0;

const double ServerParam::DEFAULT_STAMINA_MAX = 8000.0; // [13.0.0] 4000.0 -> 8000.0
const double ServerParam::DEFAULT_STAMINA_INC_MAX = 45.0;

const double ServerParam::DEFAULT_RECOVER_INIT = 1.0;
const double ServerParam::DEFAULT_RECOVER_DEC_THR = 0.3;
const double ServerParam::DEFAULT_RECOVER_MIN = 0.5;
const double ServerParam::DEFAULT_RECOVER_DEC = 0.002;

const double ServerParam::DEFAULT_EFFORT_INIT = 1.0;
const double ServerParam::DEFAULT_EFFORT_DEC_THR = 0.3;
const double ServerParam::DEFAULT_EFFORT_MIN = 0.6;
const double ServerParam::DEFAULT_EFFORT_DEC = 0.005;
const double ServerParam::DEFAULT_EFFORT_INC_THR = 0.6;
const double ServerParam::DEFAULT_EFFORT_INC = 0.01;

const double ServerParam::DEFAULT_KICK_RAND = 0.1; // [12.0.0] 0.0 -> 0.1
const bool   ServerParam::DEFAULT_TEAM_ACTUATOR_NOISE = false;
const double ServerParam::DEFAULT_PLAYER_RAND_FACTOR_L = 1.0;
const double ServerParam::DEFAULT_PLAYER_RAND_FACTOR_R = 1.0;
const double ServerParam::DEFAULT_KICK_RAND_FACTOR_L = 1.0;
const double ServerParam::DEFAULT_KICK_RAND_FACTOR_R = 1.0;

const double ServerParam::DEFAULT_BALL_SIZE = 0.085;
const double ServerParam::DEFAULT_BALL_DECAY = 0.94;
const double ServerParam::DEFAULT_BALL_RAND = 0.05;
const double ServerParam::DEFAULT_BALL_WEIGHT = 0.2;
//const double ServerParam::DEFAULT_BALL_SPEED_MAX = 2.7;
const double ServerParam::DEFAULT_BALL_SPEED_MAX = 3.0;
const double ServerParam::DEFAULT_BALL_ACCEL_MAX = 2.7;

const double ServerParam::DEFAULT_DASH_POWER_RATE = 0.006;
const double ServerParam::DEFAULT_KICK_POWER_RATE = 0.027;
const double ServerParam::DEFAULT_KICKABLE_MARGIN = 0.7;
const double ServerParam::DEFAULT_CONTROL_RADIUS = 2.0;
//const double ServerParam::DEFAULT_CONTROL_RADIUS_WIDTH
// = DEFAULT_CONTROL_RADIUS - DEFAULT_PLAYER_SIZE;

const double ServerParam::DEFAULT_MAX_POWER = 100.0;
const double ServerParam::DEFAULT_MIN_POWER = -100.0;
const double ServerParam::DEFAULT_MAX_MOMENT = 180.0;
const double ServerParam::DEFAULT_MIN_MOMENT = -180.0;
const double ServerParam::DEFAULT_MAX_NECK_MOMENT = 180.0;
const double ServerParam::DEFAULT_MIN_NECK_MOMENT = -180.0;
const double ServerParam::DEFAULT_MAX_NECK_ANGLE = 90.0;
const double ServerParam::DEFAULT_MIN_NECK_ANGLE = -90.0;

const double ServerParam::DEFAULT_VISIBLE_ANGLE = 90.0;
const double ServerParam::DEFAULT_VISIBLE_DISTANCE = 3.0;

const double ServerParam::DEFAULT_WIND_DIR = 0.0;
const double ServerParam::DEFAULT_WIND_FORCE = 0.0;
const double ServerParam::DEFAULT_WIND_ANGLE = 0.0;
const double ServerParam::DEFAULT_WIND_RAND = 0.0;

//const double ServerParam::DEFAULT_KICKABLE_AREA
// = KICKABLE_MARGIN + PLAYER_SIZE + BALL_SIZE;

const double ServerParam::DEFAULT_CATCH_AREA_L = 1.2; // [12.0.0] 2.0 -> 1.2
const double ServerParam::DEFAULT_CATCH_AREA_W = 1.0;
const double ServerParam::DEFAULT_CATCH_PROBABILITY = 1.0;
const int    ServerParam::DEFAULT_GOALIE_MAX_MOVES = 2;

const double ServerParam::DEFAULT_CORNER_KICK_MARGIN = 1.0;
const double ServerParam::DEFAULT_OFFSIDE_ACTIVE_AREA_SIZE = 2.5;

const bool   ServerParam::DEFAULT_WIND_NONE = false;
const bool   ServerParam::DEFAULT_USE_WIND_RANDOM = false;

const int    ServerParam::DEFAULT_COACH_SAY_COUNT_MAX = 128;
// defined as DEF_SAY_COACH_CNT_MAX
const int    ServerParam::DEFAULT_COACH_SAY_MSG_SIZE = 128;

const int    ServerParam::DEFAULT_CLANG_WIN_SIZE = 300;
const int    ServerParam::DEFAULT_CLANG_DEFINE_WIN = 1;
const int    ServerParam::DEFAULT_CLANG_META_WIN = 1;
const int    ServerParam::DEFAULT_CLANG_ADVICE_WIN = 1;
const int    ServerParam::DEFAULT_CLANG_INFO_WIN = 1;
const int    ServerParam::DEFAULT_CLANG_MESS_DELAY = 50;
const int    ServerParam::DEFAULT_CLANG_MESS_PER_CYCLE = 1;

const int    ServerParam::DEFAULT_HALF_TIME = 300;
const int    ServerParam::DEFAULT_SIMULATOR_STEP = 100;
const int    ServerParam::DEFAULT_SEND_STEP = 150;
const int    ServerParam::DEFAULT_RECV_STEP = 10;
const int    ServerParam::DEFAULT_SENSE_BODY_STEP = 100;
//const int    ServerParam::DEFAULT_LCM_STEP
// = lcm(sim_st, lcm(send_st, lcm(recv_st, lcm(sb_step, sv_st))))) of sync_offset;

const int    ServerParam::DEFAULT_PLAYER_SAY_MSG_SIZE = 10;
const int    ServerParam::DEFAULT_PLAYER_HEAR_MAX = 1;
const int    ServerParam::DEFAULT_PLAYER_HEAR_INC = 1;
const int    ServerParam::DEFAULT_PLAYER_HEAR_DECAY = 1;

const int    ServerParam::DEFAULT_CATCH_BAN_CYCLE = 5;

const int    ServerParam::DEFAULT_SLOW_DOWN_FACTOR = 1;

const bool   ServerParam::DEFAULT_USE_OFFSIDE = true;
const bool   ServerParam::DEFAULT_KICKOFF_OFFSIDE = true;
const double ServerParam::DEFAULT_OFFSIDE_KICK_MARGIN = 9.15;

const double ServerParam::DEFAULT_AUDIO_CUT_DIST = 50.0;

const double ServerParam::DEFAULT_DIST_QUANTIZE_STEP = 0.1;
const double ServerParam::DEFAULT_LANDMARK_DIST_QUANTIZE_STEP = 0.01;
const double ServerParam::DEFAULT_DIR_QUANTIZE_STEP = 0.1;
//const double ServerParam::DEFAULT_DIST_QUANTIZE_STEP_L;
//const double ServerParam::DEFAULT_DIST_QUANTIZE_STEP_R;
//const double ServerParam::DEFAULT_LANDMARK_DIST_QUANTIZE_STEP_L;
//const double ServerParam::DEFAULT_LANDMARK_DIST_QUANTIZE_STEP_R;
//const double ServerParam::DEFAULT_DIR_QUANTIZE_STEP_L;
//const double ServerParam::DEFAULT_DIR_QUANTIZE_STEP_R;

const bool   ServerParam::DEFAULT_COACH_MODE = false;
const bool   ServerParam::DEFAULT_COACH_WITH_REFEREE_MODE = false;
const bool   ServerParam::DEFAULT_USE_OLD_COACH_HEAR = false;

const double ServerParam::DEFAULT_SLOWNESS_ON_TOP_FOR_LEFT_TEAM = 1.0;
const double ServerParam::DEFAULT_SLOWNESS_ON_TOP_FOR_RIGHT_TEAM = 1.0;


const int    ServerParam::DEFAULT_START_GOAL_L = 0;
const int    ServerParam::DEFAULT_START_GOAL_R = 0;

const bool   ServerParam::DEFAULT_FULLSTATE_L = false;
const bool   ServerParam::DEFAULT_FULLSTATE_R = false;

const int    ServerParam::DEFAULT_DROP_BALL_TIME = 200;

const bool   ServerParam::DEFAULT_SYNC_MODE = false;
const int    ServerParam::DEFAULT_SYNC_OFFSET = 60;
const int    ServerParam::DEFAULT_SYNC_MICRO_SLEEP = 1;

const int    ServerParam::DEFAULT_POINT_TO_BAN = 5;
const int    ServerParam::DEFAULT_POINT_TO_DURATION = 20;



const int    ServerParam::DEFAULT_PLAYER_PORT = 6000;
const int    ServerParam::DEFAULT_TRAINER_PORT = 6001;
const int    ServerParam::DEFAULT_ONLINE_COACH_PORT = 6002;

const bool   ServerParam::DEFAULT_VERBOSE_MODE = false;

const int    ServerParam::DEFAULT_COACH_SEND_VI_STEP = 100;

const std::string ServerParam::DEFAULT_REPLAY_FILE = ""; // unused after rcsserver-9+
const std::string ServerParam::DEFAULT_LANDMARK_FILE = "~/.rcssserver-landmark.xml";

const int    ServerParam::DEFAULT_SEND_COMMS = false;

const bool   ServerParam::DEFAULT_TEXT_LOGGING = true;
const bool   ServerParam::DEFAULT_GAME_LOGGING = true;
const int    ServerParam::DEFAULT_GAME_LOG_VERSION = 3;
const std::string ServerParam::DEFAULT_TEXT_LOG_DIR = "./";
const std::string ServerParam::DEFAULT_GAME_LOG_DIR = "./";
const std::string ServerParam::DEFAULT_TEXT_LOG_FIXED_NAME = "rcssserver";
const std::string ServerParam::DEFAULT_GAME_LOG_FIXED_NAME = "rcssserver";
const bool   ServerParam::DEFAULT_USE_TEXT_LOG_FIXED = false;
const bool   ServerParam::DEFAULT_USE_GAME_LOG_FIXED = false;
const bool   ServerParam::DEFAULT_USE_TEXT_LOG_DATED = true;
const bool   ServerParam::DEFAULT_USE_GAME_LOG_DATED = true;
const std::string ServerParam::DEFAULT_LOG_DATE_FORMAT = "%Y%m%d%H%M-";
const bool   ServerParam::DEFAULT_LOG_TIMES = false;
const bool   ServerParam::DEFAULT_RECORD_MESSAGES = false;
const int    ServerParam::DEFAULT_TEXT_LOG_COMPRESSION = 0;
const int    ServerParam::DEFAULT_GAME_LOG_COMPRESSION = 0;

const bool   ServerParam::DEFAULT_USE_PROFILE = false;

const double ServerParam::DEFAULT_TACKLE_DIST = 2.0;
const double ServerParam::DEFAULT_TACKLE_BACK_DIST = 0.0; // [12.0.0] 0.5 -> 0.0
const double ServerParam::DEFAULT_TACKLE_WIDTH = 1.25; // [12.0.0] 1.0 -> 1.25
const double ServerParam::DEFAULT_TACKLE_EXPONENT = 6.0;
const int    ServerParam::DEFAULT_TACKLE_CYCLES = 10;
const double ServerParam::DEFAULT_TACKLE_POWER_RATE = 0.027;

const int    ServerParam::DEFAULT_FREEFORM_WAIT_PERIOD = 600;
const int    ServerParam::DEFAULT_FREEFORM_SEND_PERIOD = 20;

const bool   ServerParam::DEFAULT_FREE_KICK_FAULTS = true;
const bool   ServerParam::DEFAULT_BACK_PASSES = true;

const bool   ServerParam::DEFAULT_PROPER_GOAL_KICKS = false;
const double ServerParam::DEFAULT_STOPPED_BALL_VEL = 0.01;
const int    ServerParam::DEFAULT_MAX_GOAL_KICKS = 3;

const int    ServerParam::DEFAULT_CLANG_DEL_WIN = 1;
const int    ServerParam::DEFAULT_CLANG_RULE_WIN = 1;

const bool   ServerParam::DEFAULT_AUTO_MODE = false;
const int    ServerParam::DEFAULT_KICK_OFF_WAIT = 100;
const int    ServerParam::DEFAULT_CONNECT_WAIT = 300;
const int    ServerParam::DEFAULT_GAME_OVER_WAIT = 100;
const std::string ServerParam::DEFAULT_TEAM_L_START = "";
const std::string ServerParam::DEFAULT_TEAM_R_START = "";


const bool   ServerParam::DEFAULT_KEEPAWAY_MODE = false;
// these value are defined in rcssserver/param.h
const double ServerParam::DEFAULT_KEEPAWAY_LENGTH = 20.0;
const double ServerParam::DEFAULT_KEEPAWAY_WIDTH = 20.0;

const bool   ServerParam::DEFAULT_KEEPAWAY_LOGGING = true;
const std::string ServerParam::DEFAULT_KEEPAWAY_LOG_DIR = "./";
const std::string ServerParam::DEFAULT_KEEPAWAY_LOG_FIXED_NAME = "rcssserver";
const bool   ServerParam::DEFAULT_KEEPAWAY_LOG_FIXED = false;
const bool   ServerParam::DEFAULT_KEEPAWAY_LOG_DATED = true;

const int    ServerParam::DEFAULT_KEEPAWAY_START = -1;

const int    ServerParam::DEFAULT_NR_NORMAL_HALFS = 2;
const int    ServerParam::DEFAULT_NR_EXTRA_HALFS = 2;
const bool   ServerParam::DEFAULT_PENALTY_SHOOT_OUTS = true;

const int    ServerParam::DEFAULT_PEN_BEFORE_SETUP_WAIT = 10; // [13.2.0] 30 -> 10
const int    ServerParam::DEFAULT_PEN_SETUP_WAIT = 70; // [13.2.0] 100 -> 70
const int    ServerParam::DEFAULT_PEN_READY_WAIT = 10; // [13.2.0] 50 -> 10
const int    ServerParam::DEFAULT_PEN_TAKEN_WAIT = 150; // [13.2.0] 200 -> 150
const int    ServerParam::DEFAULT_PEN_NR_KICKS = 5;
const int    ServerParam::DEFAULT_PEN_MAX_EXTRA_KICKS = 5; // [13.2.0] 10 -> 5
const double ServerParam::DEFAULT_PEN_DIST_X = 42.5;
const bool   ServerParam::DEFAULT_PEN_RANDOM_WINNER = false;
const bool   ServerParam::DEFAULT_PEN_ALLOW_MULT_KICKS = true;
const double ServerParam::DEFAULT_PEN_MAX_GOALIE_DIST_X = 14.0;
const bool   ServerParam::DEFAULT_PEN_COACH_MOVES_PLAYERS = true;

const std::string ServerParam::DEFAULT_MODULE_DIR = "";

// 11.0.0
const double ServerParam::DEFAULT_BALL_STUCK_AREA = 3.0;

// 12.0.0
const double ServerParam::DEFAULT_MAX_TACKLE_POWER = 100.0;
const double ServerParam::DEFAULT_MAX_BACK_TACKLE_POWER = 0.0; // [13.0.0] 50.0 -> 0.0
const double ServerParam::DEFAULT_PLAYER_SPEED_MAX_MIN = 0.75; // [13.0.0] 0.8 -> 0.75
const double ServerParam::DEFAULT_EXTRA_STAMINA = 50.0; // [13.0.0] 0.0 -> 50.0
const int ServerParam::DEFAULT_SYNCH_SEE_OFFSET = 0; // [13.2.0] 30 -> 0

// 12.1.3
const int ServerParam::EXTRA_HALF_TIME = 100; // [13.0.0] 300 -> 100

// 13.0.0
const double ServerParam::STAMINA_CAPACITY = 130600.0; // [14.0.0] 148600.0 -> 130600.0
const double ServerParam::MAX_DASH_ANGLE = +180.0;
const double ServerParam::MIN_DASH_ANGLE = -180.0;
const double ServerParam::DASH_ANGLE_STEP = 45.0; // [14.0.0] 90.0 -> 45.0
const double ServerParam::SIDE_DASH_RATE = 0.4; // [14.0.0] 0.25 -> 0.4
const double ServerParam::BACK_DASH_RATE = 0.7; // [14.0.0] 0.5 -> 0.6, [17.0.0] 0.6 -> 0.7
const double ServerParam::MAX_DASH_POWER = 100.0;
const double ServerParam::MIN_DASH_POWER = 0.0; // [17.0.0] -100.0 -> 0.0

// 14.0.0
const double ServerParam::TACKLE_RAND_FACTOR = 2.0;
const double ServerParam::FOUL_DETECT_PROBABILITY = 0.5;
const double ServerParam::FOUL_EXPONENT = 10.0;
const int ServerParam::FOUL_CYCLES = 5;

// 15.0
const double ServerParam::RED_CARD_PROBABILITY = 0.0;

// 16.0
const int ServerParam::ILLEGAL_DEFENSE_DURATION = 20;
const int ServerParam::ILLEGAL_DEFENSE_NUMBER = 0;
const double ServerParam::ILLEGAL_DEFENSE_DIST_X = 16.5;
const double ServerParam::ILLEGAL_DEFENSE_WIDTH = 40.32;

// 17.0
const double ServerParam::MAX_CATCH_ANGLE = +90.0;
const double ServerParam::MIN_CATCH_ANGLE = -90.0;

/*
  It is necessary to check out whether server is NEW_QSTEP mode
  bool defined_qstep_l = false;
  bool defined_qstep_r = false;
  bool defined_qstep_l_l = false;
  bool defined_qstep_l_r = false;
  bool defined_qstep_dir_l = false;
  bool defined_qstep_dir_r = false;
*/

/*-------------------------------------------------------------------*/
/*!

*/
ServerParam &
ServerParam::instance()
{
    static ServerParam S_instance;
    return S_instance;
}

/*-------------------------------------------------------------------*/
/*!

*/
ServerParam::ServerParam()
    : M_param_map( new ParamMap( "server_param" ) )
{
    assert( M_param_map );

    setDefaultParam();
    createMap();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ServerParam::setDefaultParam()
{
    M_goal_width = DEFAULT_GOAL_WIDTH;
    M_inertia_moment = DEFAULT_INERTIA_MOMENT;

    M_player_size = DEFAULT_PLAYER_SIZE;
    M_player_decay = DEFAULT_PLAYER_DECAY;
    M_player_rand = DEFAULT_PLAYER_RAND;
    M_player_weight = DEFAULT_PLAYER_WEIGHT;
    M_player_speed_max = DEFAULT_PLAYER_SPEED_MAX;
    M_player_accel_max = DEFAULT_PLAYER_ACCEL_MAX;

    M_stamina_max = DEFAULT_STAMINA_MAX;
    M_stamina_inc_max = DEFAULT_STAMINA_INC_MAX;

    M_recover_init = DEFAULT_RECOVER_INIT;
    M_recover_dec_thr = DEFAULT_RECOVER_DEC_THR;
    M_recover_min = DEFAULT_RECOVER_MIN;
    M_recover_dec = DEFAULT_RECOVER_DEC;

    M_effort_init = DEFAULT_EFFORT_INIT;
    M_effort_dec_thr = DEFAULT_EFFORT_DEC_THR;
    M_effort_min = DEFAULT_EFFORT_MIN;
    M_effort_dec = DEFAULT_EFFORT_DEC;
    M_effort_inc_thr = DEFAULT_EFFORT_INC_THR;
    M_effort_inc = DEFAULT_EFFORT_INC;

    M_kick_rand = DEFAULT_KICK_RAND;
    M_team_actuator_noise = DEFAULT_TEAM_ACTUATOR_NOISE;
    M_player_rand_factor_l = DEFAULT_PLAYER_RAND_FACTOR_L;
    M_player_rand_factor_r = DEFAULT_PLAYER_RAND_FACTOR_R;
    M_kick_rand_factor_l = DEFAULT_KICK_RAND_FACTOR_L;
    M_kick_rand_factor_r = DEFAULT_KICK_RAND_FACTOR_R;

    M_ball_size = DEFAULT_BALL_SIZE;
    M_ball_decay = DEFAULT_BALL_DECAY;
    M_ball_rand = DEFAULT_BALL_RAND;
    M_ball_weight = DEFAULT_BALL_WEIGHT;
    M_ball_speed_max = DEFAULT_BALL_SPEED_MAX;
    M_ball_accel_max = DEFAULT_BALL_ACCEL_MAX;

    M_dash_power_rate = DEFAULT_DASH_POWER_RATE;
    M_kick_power_rate = DEFAULT_KICK_POWER_RATE;
    M_kickable_margin = DEFAULT_KICKABLE_MARGIN;
    M_control_radius = DEFAULT_CONTROL_RADIUS;
    M_control_radius_width = DEFAULT_CONTROL_RADIUS - DEFAULT_PLAYER_SIZE;

    M_max_power = DEFAULT_MAX_POWER;
    M_min_power = DEFAULT_MIN_POWER;
    M_max_moment = DEFAULT_MAX_MOMENT;
    M_min_moment = DEFAULT_MIN_MOMENT;
    M_max_neck_moment = DEFAULT_MAX_NECK_MOMENT;
    M_min_neck_moment = DEFAULT_MIN_NECK_MOMENT;
    M_max_neck_angle = DEFAULT_MAX_NECK_ANGLE;
    M_min_neck_angle = DEFAULT_MIN_NECK_ANGLE;

    M_visible_angle = DEFAULT_VISIBLE_ANGLE;
    M_visible_distance = DEFAULT_VISIBLE_DISTANCE;

    M_wind_dir = DEFAULT_WIND_DIR;
    M_wind_force = DEFAULT_WIND_FORCE;
    M_wind_angle = DEFAULT_WIND_ANGLE;
    M_wind_rand = DEFAULT_WIND_RAND;

    M_kickable_area = DEFAULT_PLAYER_SIZE + DEFAULT_KICKABLE_MARGIN + DEFAULT_BALL_SIZE;

    M_catch_area_l = DEFAULT_CATCH_AREA_L;
    M_catch_area_w = DEFAULT_CATCH_AREA_W;
    M_catch_probability = DEFAULT_CATCH_PROBABILITY;
    M_goalie_max_moves = DEFAULT_GOALIE_MAX_MOVES;

    M_corner_kick_margin = DEFAULT_CORNER_KICK_MARGIN;
    M_offside_active_area_size = DEFAULT_OFFSIDE_ACTIVE_AREA_SIZE;

    M_wind_none = DEFAULT_WIND_NONE;
    M_use_wind_random = DEFAULT_USE_WIND_RANDOM;

    M_coach_say_count_max = DEFAULT_COACH_SAY_COUNT_MAX;
    M_coach_say_msg_size = DEFAULT_COACH_SAY_MSG_SIZE;

    M_clang_win_size = DEFAULT_CLANG_WIN_SIZE;
    M_clang_define_win = DEFAULT_CLANG_DEFINE_WIN;
    M_clang_meta_win = DEFAULT_CLANG_META_WIN;
    M_clang_advice_win = DEFAULT_CLANG_ADVICE_WIN;
    M_clang_info_win = DEFAULT_CLANG_INFO_WIN;
    M_clang_mess_delay = DEFAULT_CLANG_MESS_DELAY;
    M_clang_mess_per_cycle = DEFAULT_CLANG_MESS_PER_CYCLE;

    M_half_time = DEFAULT_HALF_TIME;
    M_simulator_step = DEFAULT_SIMULATOR_STEP;
    M_send_step = DEFAULT_SEND_STEP;
    M_recv_step = DEFAULT_RECV_STEP;
    M_sense_body_step = DEFAULT_SENSE_BODY_STEP;
    M_lcm_step = 300; //lcm(simulator_step, send_step, recv_step, sense_body_step, send_vi_step);

    M_player_say_msg_size = DEFAULT_PLAYER_SAY_MSG_SIZE;
    M_player_hear_max = DEFAULT_PLAYER_HEAR_MAX;
    M_player_hear_inc = DEFAULT_PLAYER_HEAR_INC;
    M_player_hear_decay = DEFAULT_PLAYER_HEAR_DECAY;

    M_catch_ban_cycle = DEFAULT_CATCH_BAN_CYCLE;

    M_slow_down_factor = DEFAULT_SLOW_DOWN_FACTOR;

    M_use_offside = DEFAULT_USE_OFFSIDE;
    M_kickoff_offside = DEFAULT_KICKOFF_OFFSIDE;
    M_offside_kick_margin = DEFAULT_OFFSIDE_KICK_MARGIN;

    M_audio_cut_dist = DEFAULT_AUDIO_CUT_DIST;

    M_dist_quantize_step = DEFAULT_DIST_QUANTIZE_STEP;
    M_landmark_dist_quantize_step = DEFAULT_LANDMARK_DIST_QUANTIZE_STEP;
    M_dir_quantize_step = DEFAULT_DIR_QUANTIZE_STEP;
    M_dist_quantize_step_l = DEFAULT_DIST_QUANTIZE_STEP;
    M_dist_quantize_step_r = DEFAULT_DIST_QUANTIZE_STEP;
    M_landmark_dist_quantize_step_l = DEFAULT_LANDMARK_DIST_QUANTIZE_STEP;
    M_landmark_dist_quantize_step_r = DEFAULT_LANDMARK_DIST_QUANTIZE_STEP;
    M_dir_quantize_step_l = DEFAULT_DIR_QUANTIZE_STEP;
    M_dir_quantize_step_r = DEFAULT_DIR_QUANTIZE_STEP;

    M_coach_mode = DEFAULT_COACH_MODE;
    M_coach_with_referee_mode = DEFAULT_COACH_WITH_REFEREE_MODE;
    M_use_old_coach_hear = DEFAULT_USE_OLD_COACH_HEAR;

    M_slowness_on_top_for_left_team = DEFAULT_SLOWNESS_ON_TOP_FOR_LEFT_TEAM;
    M_slowness_on_top_for_right_team = DEFAULT_SLOWNESS_ON_TOP_FOR_RIGHT_TEAM;

    M_start_goal_l = DEFAULT_START_GOAL_L;
    M_start_goal_r = DEFAULT_START_GOAL_R;

    M_fullstate_l = DEFAULT_FULLSTATE_L;
    M_fullstate_r = DEFAULT_FULLSTATE_R;

    M_drop_ball_time = DEFAULT_DROP_BALL_TIME;

    M_synch_mode = DEFAULT_SYNC_MODE;
    M_synch_offset = DEFAULT_SYNC_OFFSET;
    M_synch_micro_sleep = DEFAULT_SYNC_MICRO_SLEEP;

    M_point_to_ban = DEFAULT_POINT_TO_BAN;
    M_point_to_duration = DEFAULT_POINT_TO_DURATION;

    // not defined in server_param_t
    M_player_port = DEFAULT_PLAYER_PORT;
    M_trainer_port = DEFAULT_TRAINER_PORT;
    M_online_coach_port = DEFAULT_ONLINE_COACH_PORT;

    M_verbose_mode = DEFAULT_VERBOSE_MODE;

    M_coach_send_vi_step = DEFAULT_COACH_SEND_VI_STEP;

    M_replay_file = DEFAULT_REPLAY_FILE;
    M_landmark_file = DEFAULT_LANDMARK_FILE;

    M_send_comms = DEFAULT_SEND_COMMS;

    M_text_logging = DEFAULT_TEXT_LOGGING;
    M_game_logging = DEFAULT_GAME_LOGGING;
    M_game_log_version = DEFAULT_GAME_LOG_VERSION;
    M_text_log_dir = DEFAULT_TEXT_LOG_DIR;
    M_game_log_dir = DEFAULT_GAME_LOG_DIR;
    M_text_log_fixed_name = DEFAULT_TEXT_LOG_FIXED_NAME;
    M_game_log_fixed_name = DEFAULT_GAME_LOG_FIXED_NAME;
    M_use_text_log_fixed = DEFAULT_USE_TEXT_LOG_FIXED;
    M_use_game_log_fixed = DEFAULT_USE_GAME_LOG_FIXED;
    M_use_text_log_dated = DEFAULT_USE_TEXT_LOG_DATED;
    M_use_game_log_dated = DEFAULT_USE_GAME_LOG_DATED;
    M_log_date_format = DEFAULT_LOG_DATE_FORMAT;
    M_log_times = DEFAULT_LOG_TIMES;
    M_record_message = DEFAULT_RECORD_MESSAGES;
    M_text_log_compression = DEFAULT_TEXT_LOG_COMPRESSION;
    M_game_log_compression = DEFAULT_GAME_LOG_COMPRESSION;

    M_use_profile = DEFAULT_USE_PROFILE;

    M_tackle_dist = DEFAULT_TACKLE_DIST;
    M_tackle_back_dist = DEFAULT_TACKLE_BACK_DIST;
    M_tackle_width = DEFAULT_TACKLE_WIDTH;
    M_tackle_exponent = DEFAULT_TACKLE_EXPONENT;
    M_tackle_cycles = DEFAULT_TACKLE_CYCLES;
    M_tackle_power_rate = DEFAULT_TACKLE_POWER_RATE;

    M_freeform_wait_period = DEFAULT_FREEFORM_WAIT_PERIOD;
    M_freeform_send_period = DEFAULT_FREEFORM_SEND_PERIOD;

    M_free_kick_faults = DEFAULT_FREE_KICK_FAULTS;
    M_back_passes = DEFAULT_BACK_PASSES;

    M_proper_goal_kicks = DEFAULT_PROPER_GOAL_KICKS;
    M_stopped_ball_vel = DEFAULT_STOPPED_BALL_VEL;
    M_max_goal_kicks = DEFAULT_MAX_GOAL_KICKS;

    M_clang_del_win  = DEFAULT_CLANG_DEL_WIN;
    M_clang_rule_win = DEFAULT_CLANG_RULE_WIN;

    M_auto_mode = DEFAULT_AUTO_MODE;
    M_kick_off_wait = DEFAULT_KICK_OFF_WAIT;
    M_connect_wait = DEFAULT_CONNECT_WAIT;
    M_game_over_wait = DEFAULT_GAME_OVER_WAIT;
    M_team_l_start = DEFAULT_TEAM_L_START;
    M_team_r_start = DEFAULT_TEAM_R_START;

    M_keepaway_mode = DEFAULT_KEEPAWAY_MODE;
    M_keepaway_length = DEFAULT_KEEPAWAY_LENGTH;
    M_keepaway_width = DEFAULT_KEEPAWAY_WIDTH;

    M_keepaway_logging = DEFAULT_KEEPAWAY_LOGGING;
    M_keepaway_log_dir = DEFAULT_KEEPAWAY_LOG_DIR;
    M_keepaway_log_fixed_name = DEFAULT_KEEPAWAY_LOG_FIXED_NAME;
    M_keepaway_log_fixed = DEFAULT_KEEPAWAY_LOG_FIXED;
    M_keepaway_log_dated = DEFAULT_KEEPAWAY_LOG_DATED;

    M_keepaway_start = DEFAULT_KEEPAWAY_START;

    M_nr_normal_halfs = DEFAULT_NR_NORMAL_HALFS;
    M_nr_extra_halfs = DEFAULT_NR_EXTRA_HALFS;
    M_penalty_shoot_outs = DEFAULT_PENALTY_SHOOT_OUTS;

    M_pen_before_setup_wait = DEFAULT_PEN_BEFORE_SETUP_WAIT;
    M_pen_setup_wait = DEFAULT_PEN_SETUP_WAIT;
    M_pen_ready_wait = DEFAULT_PEN_READY_WAIT;
    M_pen_taken_wait = DEFAULT_PEN_TAKEN_WAIT;
    M_pen_nr_kicks = DEFAULT_PEN_NR_KICKS;
    M_pen_max_extra_kicks = DEFAULT_PEN_MAX_EXTRA_KICKS;
    M_pen_dist_x = DEFAULT_PEN_DIST_X;
    M_pen_random_winner = DEFAULT_PEN_RANDOM_WINNER;
    M_pen_allow_mult_kicks = DEFAULT_PEN_ALLOW_MULT_KICKS;
    M_pen_max_goalie_dist_x = DEFAULT_PEN_MAX_GOALIE_DIST_X;
    M_pen_coach_moves_players = DEFAULT_PEN_COACH_MOVES_PLAYERS;

    M_module_dir = DEFAULT_MODULE_DIR;

    // 11.0.0
    M_ball_stuck_area = DEFAULT_BALL_STUCK_AREA;
    // M_coach_msg_file = "";

    // 12.0.0
    M_max_tackle_power = DEFAULT_MAX_TACKLE_POWER;
    M_max_back_tackle_power = DEFAULT_MAX_BACK_TACKLE_POWER;
    M_player_speed_max_min = DEFAULT_PLAYER_SPEED_MAX_MIN;
    M_extra_stamina = DEFAULT_EXTRA_STAMINA;
    M_synch_see_offset = DEFAULT_SYNCH_SEE_OFFSET;

    M_max_monitors = -1;

    // 12.1.3
    M_extra_half_time = EXTRA_HALF_TIME;

    // 13.0.0
    M_stamina_capacity = STAMINA_CAPACITY;
    M_max_dash_angle = MAX_DASH_ANGLE;
    M_min_dash_angle = MIN_DASH_ANGLE;
    M_dash_angle_step = DASH_ANGLE_STEP;
    M_side_dash_rate = SIDE_DASH_RATE;
    M_back_dash_rate = BACK_DASH_RATE;
    M_max_dash_power = MAX_DASH_POWER;
    M_min_dash_power = MIN_DASH_POWER;

    // 14.0.0
    M_tackle_rand_factor = TACKLE_RAND_FACTOR;
    M_foul_detect_probability = FOUL_DETECT_PROBABILITY;
    M_foul_exponent = FOUL_EXPONENT;
    M_foul_cycles = FOUL_CYCLES;
    M_golden_goal = false; // [15.0.0] true -> false

    // 15.0.0
    M_red_card_probability = RED_CARD_PROBABILITY;

    // 16.0
    M_illegal_defense_duration = ILLEGAL_DEFENSE_DURATION;
    M_illegal_defense_number = ILLEGAL_DEFENSE_NUMBER;
    M_illegal_defense_dist_x = ILLEGAL_DEFENSE_DIST_X;
    M_illegal_defense_width = ILLEGAL_DEFENSE_WIDTH;
    M_fixed_teamname_l = "";
    M_fixed_teamname_r = "";

    // 17.0
    M_max_catch_angle = MAX_CATCH_ANGLE;
    M_min_catch_angle = MIN_CATCH_ANGLE;

    // XXX
    M_random_seed = -1;
    M_long_kick_power_factor = 2.0;
    M_long_kick_delay = 2;

    setAdditionalParam();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ServerParam::setAdditionalParam()
{
    M_kickable_area = M_kickable_margin + M_ball_size + M_player_size;
    M_catchable_area = std::sqrt( std::pow( catchAreaWidth() * 0.5, 2 )
                                  + std::pow( catchAreaLength(), 2 ) );
    M_control_radius_width = M_control_radius - M_player_size;

    // real speed max
    double accel_max = maxDashPower() * defaultDashPowerRate() * defaultEffortMax();
    M_real_speed_max = accel_max / ( 1.0 - defaultPlayerDecay() );  // sum inf geom series
    if ( M_real_speed_max > defaultPlayerSpeedMax() )
    {
        M_real_speed_max = defaultPlayerSpeedMax();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ServerParam::createMap()
{
    M_param_map->add()
        ( "goal_width", "", &M_goal_width )
        ( "inertia_moment", "", &M_inertia_moment )

        ( "player_size", "", &M_player_size )
        ( "player_decay", "", &M_player_decay )
        ( "player_rand", "", &M_player_rand )
        ( "player_weight", "", &M_player_weight )
        ( "player_speed_max", "", &M_player_speed_max )
        ( "player_accel_max", "", &M_player_accel_max )

        ( "stamina_max", "", &M_stamina_max )
        ( "stamina_inc_max", "", &M_stamina_inc_max )

        ( "recover_init", "", &M_recover_init ) // not necessary
        ( "recover_dec_thr", "", &M_recover_dec_thr )
        ( "recover_min", "", &M_recover_min )
        ( "recover_dec", "", &M_recover_dec )

        ( "effort_init", "", &M_effort_init )
        ( "effort_dec_thr", "", &M_effort_dec_thr )
        ( "effort_min", "", &M_effort_min )
        ( "effort_dec", "", &M_effort_dec )
        ( "effort_inc_thr", "", &M_effort_inc_thr )
        ( "effort_inc", "",  &M_effort_inc )

        ( "kick_rand", "", &M_kick_rand )
        ( "team_actuator_noise", "", &M_team_actuator_noise )
        ( "prand_factor_l", "", &M_player_rand_factor_l )
        ( "prand_factor_r", "", &M_player_rand_factor_r )
        ( "kick_rand_factor_l", "", &M_kick_rand_factor_l )
        ( "kick_rand_factor_r", "", &M_kick_rand_factor_r )

        ( "ball_size", "", &M_ball_size )
        ( "ball_decay", "", &M_ball_decay )
        ( "ball_rand", "", &M_ball_rand )
        ( "ball_weight", "", &M_ball_weight )
        ( "ball_speed_max", "", &M_ball_speed_max )
        ( "ball_accel_max", "", &M_ball_accel_max )

        ( "dash_power_rate", "", &M_dash_power_rate )
        ( "kick_power_rate", "", &M_kick_power_rate )
        ( "kickable_margin", "", &M_kickable_margin )
        ( "control_radius", "", &M_control_radius )
        //( "control_radius_width", "", &M_control_radius_width ) // not needed

        ( "maxpower", "", &M_max_power )
        ( "minpower", "", &M_min_power )
        ( "maxmoment", "", &M_max_moment )
        ( "minmoment", "", &M_min_moment )
        ( "maxneckmoment", "", &M_max_neck_moment )
        ( "minneckmoment", "", &M_min_neck_moment )
        ( "maxneckang", "", &M_max_neck_angle )
        ( "minneckang", "", &M_min_neck_angle )

        ( "visible_angle", "", &M_visible_angle )
        ( "visible_distance", "", &M_visible_distance )

        ( "wind_dir", "", &M_wind_dir )
        ( "wind_force", "", &M_wind_force )
        ( "wind_ang", "", &M_wind_angle )
        ( "wind_rand", "", &M_wind_rand )

        //( "kickable_area", "", &M_kickable_area ) // not needed

        ( "catchable_area_l", "", &M_catch_area_l )
        ( "catchable_area_w", "", &M_catch_area_w )
        ( "catch_probability", "", &M_catch_probability )
        ( "goalie_max_moves", "", &M_goalie_max_moves )

        ( "ckick_margin", "", &M_corner_kick_margin )
        ( "offside_active_area_size", "", &M_offside_active_area_size )

        ( "wind_none", "", BoolSwitch( &M_wind_none ) )
        ( "wind_random", "", BoolSwitch( &M_use_wind_random ) )

        ( "say_coach_cnt_max", "", &M_coach_say_count_max )
        ( "say_coach_msg_size", "", &M_coach_say_msg_size )

        ( "clang_win_size", "", &M_clang_win_size )
        ( "clang_define_win", "", &M_clang_define_win )
        ( "clang_meta_win", "", &M_clang_meta_win )
        ( "clang_advice_win", "", &M_clang_advice_win )
        ( "clang_info_win", "", &M_clang_info_win )
        ( "clang_mess_delay", "", &M_clang_mess_delay )
        ( "clang_mess_per_cycle", "", &M_clang_mess_per_cycle )

        ( "half_time", "", &M_half_time )
        ( "simulator_step", "", &M_simulator_step )
        ( "send_step", "", &M_send_step )
        ( "recv_step", "", &M_recv_step )
        ( "sense_body_step", "", &M_sense_body_step )
        //( "lcm_step", "", &M_lcm_step ) // not needed

        ( "say_msg_size", "", &M_player_say_msg_size )
        ( "hear_max", "", &M_player_hear_max )
        ( "hear_inc", "", &M_player_hear_inc )
        ( "hear_decay", "", &M_player_hear_decay )

        ( "catch_ban_cycle", "", &M_catch_ban_cycle )

        ( "slow_down_factor", "", &M_slow_down_factor )

        ( "use_offside", "", &M_use_offside )
        ( "forbid_kick_off_offside", "", &M_kickoff_offside )
        ( "offside_kick_margin", "", &M_offside_kick_margin )

        ( "audio_cut_dist", "", &M_audio_cut_dist )

        ( "quantize_step", "", &M_dist_quantize_step )
        ( "quantize_step_l", "", &M_landmark_dist_quantize_step )
        //( "quantize_step_dir", "", &M_dir_quantize_step )
        //( "quantize_step_dist_team_l", "", &M_dist_quantize_step_l )
        //( "quantize_step_dist_team_r", "", &M_dist_quantize_step_r )
        //( "quantize_step_dist_l_team_l", "", &M_landmark_dist_quantize_step_l )
        //( "quantize_step_dist_l_team_r", "", &M_landmark_dist_quantize_step_r )
        //( "quantize_step_dir_team_l", "", &M_dir_quantize_step_l )
        //( "quantize_step_dir_team_r", "", &M_dir_quantize_step_r )

        ( "coach", "", BoolSwitch( &M_coach_mode ) )
        ( "coach_w_referee", "", BoolSwitch( &M_coach_with_referee_mode ) )
        ( "old_coach_hear", "", BoolSwitch( &M_use_old_coach_hear ) )

        ( "slowness_on_top_for_left_team", "", &M_slowness_on_top_for_left_team )
        ( "slowness_on_top_for_right_team", "", &M_slowness_on_top_for_right_team )

        ( "start_goal_l", "", &M_start_goal_l )
        ( "start_goal_r", "", &M_start_goal_r )

        ( "fullstate_l", "", &M_fullstate_l )
        ( "fullstate_r", "", &M_fullstate_r )

        ( "drop_ball_time", "", &M_drop_ball_time )

        ( "synch_mode", "", &M_synch_mode )
        ( "synch_offset", "", &M_synch_offset )
        ( "synch_micro_sleep", "", &M_synch_micro_sleep )

        ( "point_to_ban", "", &M_point_to_ban )
        ( "point_to_duration", "", &M_point_to_duration )

        // not defined in server_params_t
        ( "port", "", &M_player_port )
        ( "coach_port", "", &M_trainer_port )
        ( "olcoach_port", "", &M_online_coach_port )

        ( "verbose", "", &M_verbose_mode )

        ( "send_vi_step", "", &M_coach_send_vi_step )

        ( "replay", "", &M_replay_file ) // unused after rcsserver-9+
        ( "landmark_file", "", &M_landmark_file )

        ( "send_comms", "", &M_send_comms )

        // logging params are not used in normal client
        ( "text_logging", "", &M_text_logging )
        ( "game_logging", "", &M_game_logging )
        ( "game_log_version", "", &M_game_log_version )
        ( "text_log_dir", "", &M_text_log_dir )
        ( "game_log_dir", "", &M_game_log_dir )
        ( "text_log_fixed_name", "", &M_text_log_fixed_name )
        ( "game_log_fixed_name", "", &M_game_log_fixed_name )
        ( "text_log_fixed", "", &M_use_text_log_fixed )
        ( "game_log_fixed", "", &M_use_game_log_fixed )
        ( "text_log_dated", "", &M_use_text_log_dated )
        ( "game_log_dated", "", &M_use_game_log_dated )
        ( "log_date_format", "", &M_log_date_format )
        ( "log_times", "", &M_log_times )
        ( "record_messages", "", &M_record_message )
        ( "text_log_compression", "", &M_text_log_compression )
        ( "game_log_compression", "", &M_game_log_compression )
        ( "profile", "", &M_use_profile )


        ( "tackle_dist", "", &M_tackle_dist )
        ( "tackle_back_dist", "", &M_tackle_back_dist )
        ( "tackle_width", "", &M_tackle_width )
        ( "tackle_exponent", "", &M_tackle_exponent )
        ( "tackle_cycles", "", &M_tackle_cycles )
        ( "tackle_power_rate", "", &M_tackle_power_rate )

        ( "freeform_wait_period", "", &M_freeform_wait_period )
        ( "freeform_send_period", "", &M_freeform_send_period )

        ( "free_kick_faults", "", &M_free_kick_faults )
        ( "back_passes", "", &M_back_passes )

        ( "proper_goal_kicks", "", &M_proper_goal_kicks )
        ( "stopped_ball_vel", "", &M_stopped_ball_vel )
        ( "max_goal_kicks", "", &M_max_goal_kicks )

        ( "clang_del_win", "", &M_clang_del_win )
        ( "clang_rule_win", "", &M_clang_rule_win )

        ( "auto_mode", "", &M_auto_mode )
        ( "kick_off_wait", "", &M_kick_off_wait )
        ( "connect_wait", "", &M_connect_wait )
        ( "game_over_wait", "", &M_game_over_wait )

        // not necessary
        ( "team_l_start", "", &M_team_l_start )
        ( "team_r_start", "", &M_team_r_start )


        ( "keepaway", "", &M_keepaway_mode )
        ( "keepaway_length", "", &M_keepaway_length )
        ( "keepaway_width", "", &M_keepaway_width )

        // logging params are not used in normal client
        ( "keepaway_logging", "", &M_keepaway_logging )
        ( "keepaway_log_dir", "", &M_keepaway_log_dir )
        ( "keepaway_log_fixed_name", "", &M_keepaway_log_fixed_name )
        ( "keepaway_log_fixed", "", &M_keepaway_log_fixed )
        ( "keepaway_log_dated", "", &M_keepaway_log_dated )

        ( "keepaway_start", "", &M_keepaway_start )

        ( "nr_normal_halfs", "", &M_nr_normal_halfs )
        ( "nr_extra_halfs", "", &M_nr_extra_halfs )
        ( "penalty_shoot_outs", "", &M_penalty_shoot_outs )

        ( "pen_before_setup_wait", "", &M_pen_before_setup_wait )
        ( "pen_setup_wait", "", &M_pen_setup_wait )
        ( "pen_ready_wait", "", &M_pen_ready_wait )
        ( "pen_taken_wait", "", &M_pen_taken_wait )
        ( "pen_nr_kicks", "", &M_pen_nr_kicks )
        ( "pen_max_extra_kicks", "", &M_pen_max_extra_kicks )
        ( "pen_dist_x", "", &M_pen_dist_x )
        ( "pen_random_winner", "", &M_pen_random_winner )
        ( "pen_max_goalie_dist_x", "", &M_pen_max_goalie_dist_x )
        ( "pen_allow_mult_kicks", "", &M_pen_allow_mult_kicks )
        ( "pen_coach_moves_players", "", &M_pen_coach_moves_players )

        ( "module_dir", "", &M_module_dir )

        ( "ball_stuck_area", "", &M_ball_stuck_area )
        ( "coach_msg_file", "", &M_coach_msg_file )
        // 12
        ( "max_tackle_power", "", &M_max_tackle_power )
        ( "max_back_tackle_power", "", &M_max_back_tackle_power )
        ( "player_speed_max_min", "", &M_player_speed_max_min )
        ( "extra_stamina", "", &M_extra_stamina )
        ( "synch_see_offset", "", &M_synch_see_offset )
        ( "max_monitors", "", &M_max_monitors )
        // 12.1.3
        ( "extra_half_time", "",  &M_extra_half_time )
        // 13.0.0
        ( "stamina_capacity", "", &M_stamina_capacity )
        ( "max_dash_angle", "", &M_max_dash_angle )
        ( "min_dash_angle", "", &M_min_dash_angle )
        ( "dash_angle_step", "", &M_dash_angle_step )
        ( "side_dash_rate", "",  &M_side_dash_rate )
        ( "back_dash_rate", "", &M_back_dash_rate )
        ( "max_dash_power", "", &M_max_dash_power )
        ( "min_dash_power", "", &M_min_dash_power )
        // 14.0.0
        ( "tackle_rand_factor", "", &M_tackle_rand_factor )
        ( "foul_detect_probability", "", &M_foul_detect_probability )
        ( "foul_exponent", "", &M_foul_exponent )
        ( "foul_cycles", "", &M_foul_cycles )
        ( "golden_goal", "", &M_golden_goal )
        // 15.0.0
        ( "red_card_probability", "", &M_red_card_probability )
        // 16.0.0
        ( "illegal_defense_duration", "", &M_illegal_defense_duration )
        ( "illegal_defense_number", "", &M_illegal_defense_number )
        ( "illegal_defense_dist_x", "", &M_illegal_defense_dist_x )
        ( "illegal_defense_width", "", &M_illegal_defense_width )
        ( "fixed_teamname_l", "", &M_fixed_teamname_l )
        ( "fixed_teamname_r", "", &M_fixed_teamname_r )
        // 17.0
        ( "max_catch_angle", "", &M_max_catch_angle )
        ( "min_catch_angle", "", &M_min_catch_angle )
        //( "random_seed", "", &M_random_seed )
        ;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ServerParam::parse( const char * msg,
                    const double & version )
{
    bool result = true;
    //std::cerr << "parse server_param\n";
    if ( version >= 8.0 )
    {
        // read v8 protocol
        RCSSParamParser parser( msg );
        result = parser.parse( *M_param_map );
    }
    else
    {
        // read v7 protocol
        // read only param value, no param name
        result = parseV7( msg );
    }

    setAdditionalParam();

    return result;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ServerParam::parseV7( const char * msg )
{
    /*************
     // used in sserver ver.7.xx
     (server_param <gwidth> <inertia_moment> <psize> <pdecay> <prand> <pweight>
      <pspeed_max> <paccel_max> <stamina_max> <stamina_inc> <recover_init>
      <recover_dthr> <recover_min> <recover_dec> <effort_init> <effort_dthr>
      <effort_min> <effort_dec> <effort_ithr> <effort_inc> <kick_rand>
      <team_actuator_noise> <prand_factor_l> <prand_factor_r><kick_rand_factor_l>
      <kick_rand_factor_r> <bsize> <bdecay> <brand> <bweight> <bspeed_max>
      <baccel_max> <dprate> <kprate> <kmargin> <ctlradius> <ctlradius_width>
      <maxp> <minp> <maxm> <minm> <maxnm> <minnm> <maxn> <minn> <visangle>
      <visdist> <windir> <winforce> <winang> <winrand> <kickable_area>
      <catch_area_l> <catch_area_w> <catch_prob> <goalie_max_moves> <ckmargin>
      <offside_area> <win_no> <win_random> <say_cnt_max> <SayCoachMsgSize>
      <clang_win_size> <clang_define_win> <clang_meta_win>
      <clang_advice_win> <clang_info_win> <clang_mess_delay> <clang_mess_per_cycle>
      <half_time> <sim_st> <send_st> <recv_st> <sb_step> <lcm_st> <SayMsgSize>
      <hear_max> <hear_inc> <hear_decay> <cban_cycle> <slow_down_factor>
      <useoffside> <kickoffoffside> <offside_kick_margin> <audio_dist> <dist_qstep>
      <land_qstep> <dir_qstep> <dist_qstep_l> <dist_qstep_r> <land_qstep_l>
      <land_qstep_r> <dir_qstep_l> <dir_qstep_r> <CoachMode> <CwRMode> <old_hear>
      <sv_st>)
    *************/

    std::istringstream msg_strm( msg );
    std::string tmp;

    msg_strm >> tmp // "(server_param"
             >> M_goal_width
             >> M_inertia_moment
             >> M_player_size
             >> M_player_decay
             >> M_player_rand
             >> M_player_weight
             >> M_player_speed_max
             >> M_player_accel_max
             >> M_stamina_max
             >> M_stamina_inc_max
             >> M_recover_init
             >> M_recover_dec_thr
             >> M_recover_min
             >> M_recover_dec
             >> M_effort_init
             >> M_effort_dec_thr
             >> M_effort_min
             >> M_effort_dec
             >> M_effort_inc_thr
             >> M_effort_inc
             >> M_kick_rand
             >> M_team_actuator_noise
             >> M_player_rand_factor_l
             >> M_player_rand_factor_r
             >> M_kick_rand_factor_l
             >> M_kick_rand_factor_r
             >> M_ball_size
             >> M_ball_decay
             >> M_ball_rand
             >> M_ball_weight
             >> M_ball_speed_max
             >> M_ball_accel_max
             >> M_dash_power_rate
             >> M_kick_power_rate
             >> M_kickable_margin
             >> M_control_radius
             >> M_control_radius_width
             >> M_max_power
             >> M_min_power
             >> M_max_moment
             >> M_min_moment
             >> M_max_neck_moment
             >> M_min_neck_moment
             >> M_max_neck_angle
             >> M_min_neck_angle
             >> M_visible_angle
             >> M_visible_distance
             >> M_wind_dir
             >> M_wind_force
             >> M_wind_angle
             >> M_wind_rand
             >> M_kickable_area
             >> M_catch_area_l
             >> M_catch_area_w
             >> M_catch_probability
             >> M_goalie_max_moves
             >> M_corner_kick_margin
             >> M_offside_active_area_size
             >> M_wind_none
             >> M_use_wind_random
             >> M_coach_say_count_max
             >> M_coach_say_msg_size
             >> M_clang_win_size
             >> M_clang_define_win
             >> M_clang_meta_win
             >> M_clang_advice_win
             >> M_clang_info_win
             >> M_clang_mess_delay
             >> M_clang_mess_per_cycle
             >> M_half_time
             >> M_simulator_step
             >> M_send_step
             >> M_recv_step
             >> M_sense_body_step
             >> M_lcm_step
             >> M_player_say_msg_size
             >> M_player_hear_max
             >> M_player_hear_inc
             >> M_player_hear_decay
             >> M_catch_ban_cycle
             >> M_slow_down_factor
             >> M_use_offside
             >> M_kickoff_offside
             >> M_offside_kick_margin
             >> M_audio_cut_dist
             >> M_dist_quantize_step
             >> M_landmark_dist_quantize_step
             >> M_dir_quantize_step
             >> M_dist_quantize_step_l
             >> M_dist_quantize_step_r
             >> M_landmark_dist_quantize_step_l
             >> M_landmark_dist_quantize_step_r
             >> M_dir_quantize_step_l
             >> M_dir_quantize_step_r
             >> M_coach_mode
             >> M_coach_with_referee_mode
             >> M_use_old_coach_hear
             >> M_coach_send_vi_step
             >> M_start_goal_l
             >> M_start_goal_r
             >> M_fullstate_l
             >> M_fullstate_r
             >> M_drop_ball_time;

    return msg_strm.good();
}

/*
  version 8 protocol
  int -> string -> bool -> onoff -> double
  )( <-

  EXAMPLE:
  (server_param
  (catch_ban_cycle 5)(clang_advice_win 1)(clang_define_win 1)
  (clang_info_win 1)(clang_mess_delay 50)(clang_mess_per_cycle 1)
  (clang_meta_win 1)(clang_win_size 300)(coach_port 6001)
  (drop_ball_time 200)(freeform_send_period 20)(freeform_wait_period 600)
  (game_log_compression 0)(game_log_version 3)(goalie_max_moves 2)
  (half_time 3000)(hear_decay 1)(hear_inc 1)(hear_max 1)(max_goal_kicks 3)
  (olcoach_port 6002)(point_to_ban 5)(point_to_duration 20)(port 6000)
  (recv_step 10)(say_coach_cnt_max 128)(say_coach_msg_size 128)
  (say_msg_size 10)(send_step 150)(send_vi_step 100)
  (sense_body_step 100)(simulator_step 100)(slow_down_factor 1)
  (start_goal_l 0)(start_goal_r 0)(synch_micro_sleep 1)(synch_offset 60)
  (tackle_cycles 10)(text_log_compression 0)

  (game_log_dir "/home/akiyama/work/log")(game_log_fixed_name "rcssserver")
  (landmark_file "~/.rcssserver-landmark.xml")(log_date_format "%Y%m%d%H%M-")
  (replay "")(text_log_dir "/home/akiyama/work/log")
  (text_log_fixed_name "rcssserver")

  (coach 0)(coach_w_referee 0)(old_coach_hear 0)(wind_none 0)
  (wind_random 0)(back_passes 1)(forbid_kick_off_offside 1)
  (free_kick_faults 1)(fullstate_l 0)(fullstate_r 0)
  (game_log_dated 0)(game_log_fixed 0)(game_logging 1)(log_times 0)
  (profile 0)(proper_goal_kicks 1)(record_messages 0)(send_comms 0)
  (synch_mode 0)(team_actuator_noise 0)(text_log_dated 0)
  (text_log_fixed 0)(text_logging 0)(use_offside 1)(verbose 0)

  (audio_cut_dist 50)(ball_accel_max 2.7)(ball_decay 0.94)(ball_rand 0.05)
  (ball_size 0.085)(ball_speed_max 2.7)(ball_weight 0.2)
  (catch_probability 1)(catchable_area_l 2)(catchable_area_w 1)
  (ckick_margin 1)(control_radius 2)(dash_power_rate 0.006)(effort_dec 0.005)
  (effort_dec_thr 0.3)(effort_inc 0.01)(effort_inc_thr 0.6)(effort_init 0)
  (effort_min 0.6)(goal_width 14.02)(inertia_moment 5)(kick_power_rate 0.027)
  (kick_rand 0)(kick_rand_factor_l 1)(kick_rand_factor_r 1)
  (kickable_margin 0.7)(maxmoment 180)(maxneckang 90)(maxneckmoment 180)
  (maxpower 100)(minmoment -180)(minneckang -90)(minneckmoment -180)
  (minpower -100)(offside_active_area_size 5)(offside_kick_margin 9.15)
  (player_accel_max 1)(player_decay 0.4)(player_rand 0.1)(player_size 0.3)
  (player_speed_max 1.2)(player_weight 60)(prand_factor_l 1)
  (prand_factor_r 1)(quantize_step 0.1)(quantize_step_l 0.01)
  (recover_dec 0.002)(recover_dec_thr 0.3)(recover_min 0.5)
  (slowness_on_top_for_left_team 1)(slowness_on_top_for_right_team 1)
  (stamina_inc_max 45)(stamina_max 4000)(stopped_ball_vel 0.01)
  (tackle_back_dist 0.5)(tackle_dist 2.5)(tackle_exponent 6)
  (tackle_power_rate 0.027)(tackle_width 1.25)(visible_angle 90)
  (visible_distance 3)(wind_ang 0)(wind_dir 0)(wind_force 0)(wind_rand 0))
*/

/*-------------------------------------------------------------------*/
/*!

*/
void
ServerParam::convertFrom( const rcg::server_params_t & from )
{
    double tmp = 0.0;

    M_goal_width = rcg::nltohd( from.goal_width );
    M_inertia_moment = rcg::nltohd( from.inertia_moment );

    M_player_size = rcg::nltohd( from.player_size );
    M_player_decay = rcg::nltohd( from.player_decay );
    M_player_rand = rcg::nltohd( from.player_rand );
    M_player_weight = rcg::nltohd( from.player_weight );
    M_player_speed_max = rcg::nltohd( from.player_speed_max );
    M_player_accel_max = rcg::nltohd( from.player_accel_max );

    M_stamina_max = rcg::nltohd( from.stamina_max );
    M_stamina_inc_max = rcg::nltohd( from.stamina_inc );

    M_recover_init = rcg::nltohd( from.recover_init );
    M_recover_dec_thr = rcg::nltohd( from.recover_dec_thr );
    M_recover_min = rcg::nltohd( from.recover_min );
    M_recover_dec = rcg::nltohd( from.recover_dec );

    M_effort_init = rcg::nltohd( from.effort_init );
    M_effort_dec_thr = rcg::nltohd( from.effort_dec_thr );
    M_effort_min = rcg::nltohd( from.effort_min );
    M_effort_dec = rcg::nltohd( from.effort_dec );
    M_effort_inc_thr = rcg::nltohd( from.effort_inc_thr );
    M_effort_inc = rcg::nltohd( from.effort_inc );

    M_kick_rand = rcg::nltohd( from.kick_rand );
    M_team_actuator_noise = rcg::nstohb( from.team_actuator_noise );
    M_player_rand_factor_l = rcg::nltohd( from.player_rand_factor_l );
    M_player_rand_factor_r = rcg::nltohd( from.player_rand_factor_r );
    M_kick_rand_factor_l = rcg::nltohd( from.kick_rand_factor_l );
    M_kick_rand_factor_r = rcg::nltohd( from.kick_rand_factor_r );

    M_ball_size = rcg::nltohd( from.ball_size );
    M_ball_decay = rcg::nltohd( from.ball_decay );
    M_ball_rand = rcg::nltohd( from.ball_rand );
    M_ball_weight = rcg::nltohd( from.ball_weight );
    M_ball_speed_max = rcg::nltohd( from.ball_speed_max );
    M_ball_accel_max = rcg::nltohd( from.ball_accel_max );

    M_dash_power_rate = rcg::nltohd( from.dash_power_rate );
    M_kick_power_rate = rcg::nltohd( from.kick_power_rate );
    M_kickable_margin = rcg::nltohd( from.kickable_margin );
    M_control_radius = rcg::nltohd( from.control_radius );
    M_control_radius_width = rcg::nltohd( from.control_radius_width );

    M_max_power = rcg::nltohd( from.max_power );
    M_min_power = rcg::nltohd( from.min_power );
    M_max_moment = rcg::nltohd( from.max_moment );
    M_min_moment = rcg::nltohd( from.min_moment );
    M_max_neck_moment = rcg::nltohd( from.max_neck_moment );
    M_min_neck_moment = rcg::nltohd( from.min_neck_moment );
    M_max_neck_angle = rcg::nltohd( from.max_neck_angle );
    M_min_neck_angle = rcg::nltohd( from.min_neck_angle );

    M_visible_angle = rcg::nltohd( from.visible_angle );
    M_visible_distance = rcg::nltohd( from.visible_distance );

    M_wind_dir = rcg::nltohd( from.wind_dir );
    M_wind_force = rcg::nltohd( from.wind_force );
    M_wind_angle = rcg::nltohd( from.wind_ang );
    M_wind_rand = rcg::nltohd( from.wind_rand );

    M_kickable_area = rcg::nltohd( from.kickable_area );

    M_catch_area_l = rcg::nltohd( from.catch_area_l );
    M_catch_area_w = rcg::nltohd( from.catch_area_w );
    M_catch_probability = rcg::nltohd( from.catch_probability );
    M_goalie_max_moves = rcg::nstohi( from.goalie_max_moves );

    M_corner_kick_margin = rcg::nltohd( from.corner_kick_margin );
    M_offside_active_area_size = rcg::nltohd( from.offside_active_area );

    M_wind_none = rcg::nstohb( from.wind_none );
    M_use_wind_random = rcg::nstohb( from.use_wind_random );

    M_coach_say_count_max = rcg::nstohi( from.coach_say_count_max );
    M_coach_say_msg_size = rcg::nstohi( from.coach_say_msg_size );

    M_clang_win_size = rcg::nstohi( from.clang_win_size );
    M_clang_define_win = rcg::nstohi( from.clang_define_win );
    M_clang_meta_win = rcg::nstohi( from.clang_meta_win );
    M_clang_advice_win = rcg::nstohi( from.clang_advice_win );
    M_clang_info_win = rcg::nstohi( from.clang_info_win );
    M_clang_mess_delay = rcg::nstohi( from.clang_mess_delay );
    M_clang_mess_per_cycle = rcg::nstohi( from.clang_mess_per_cycle );

    M_half_time = rcg::nstohi( from.half_time );
    M_simulator_step = rcg::nstohi( from.simulator_step );
    M_send_step = rcg::nstohi( from.send_step );
    M_recv_step = rcg::nstohi( from.recv_step );
    M_sense_body_step = rcg::nstohi( from.sense_body_step );
    M_lcm_step = rcg::nstohi( from.lcm_step );

    M_player_say_msg_size = rcg::nstohi( from.player_say_msg_size );
    M_player_hear_max = rcg::nstohi( from.player_hear_max );
    M_player_hear_inc = rcg::nstohi( from.player_hear_inc );
    M_player_hear_decay = rcg::nstohi( from.player_hear_decay );

    M_catch_ban_cycle = rcg::nstohi( from.catch_ban_cycle );

    M_slow_down_factor = rcg::nstohi( from.slow_down_factor );

    M_use_offside = rcg::nstohb( from.use_offside);
    M_kickoff_offside = rcg::nstohb( from.kickoff_offside );
    M_offside_kick_margin = rcg::nltohd( from.offside_kick_margin );

    M_audio_cut_dist = rcg::nltohd(from.audio_cut_dist );

    M_dist_quantize_step = rcg::nltohd( from.dist_quantize_step );
    M_landmark_dist_quantize_step = rcg::nltohd( from.landmark_dist_quantize_step );
    M_dir_quantize_step = rcg::nltohd( from.dir_quantize_step );
    M_dist_quantize_step_l = rcg::nltohd( from.dist_quantize_step_l );
    M_dist_quantize_step_r = rcg::nltohd( from.dist_quantize_step_r );
    M_landmark_dist_quantize_step_l = rcg::nltohd( from.landmark_dist_quantize_step_l );
    M_landmark_dist_quantize_step_r = rcg::nltohd( from.landmark_dist_quantize_step_r );
    M_dir_quantize_step_l = rcg::nltohd( from.dir_quantize_step_l );
    M_dir_quantize_step_r = rcg::nltohd( from.dir_quantize_step_r );

    M_coach_mode = rcg::nstohb( from.coach_mode );
    M_coach_with_referee_mode = rcg::nstohb( from.coach_with_referee_mode );
    M_use_old_coach_hear = rcg::nstohb( from.use_old_coach_hear );

    M_coach_send_vi_step = rcg::nstohi( from.online_coach_look_step );

    M_slowness_on_top_for_left_team = rcg::nltohd( from.slowness_on_top_for_left_team );
    M_slowness_on_top_for_right_team = rcg::nltohd( from.slowness_on_top_for_right_team );

    M_keepaway_length = rcg::nltohd( from.ka_length );
    M_keepaway_width = rcg::nltohd( from.ka_width );

    // 11.0.0
    tmp = rcg::nltohd( from.ball_stuck_area );
    if ( std::fabs( tmp ) < 1000.0 ) M_ball_stuck_area = tmp;

    // 12.0.0
    tmp = rcg::nltohd( from.max_tackle_power );
    if ( 0.0 < tmp && std::fabs( tmp ) < 200.0 ) M_max_tackle_power = tmp;
    tmp = rcg::nltohd( from.max_back_tackle_power );
    if ( 0.0 < tmp && std::fabs( tmp ) < 200.0 ) M_max_back_tackle_power = tmp;

    tmp = rcg::nltohd( from.tackle_dist );
    if ( 0.0 <= tmp && std::fabs( tmp ) < 3.0 ) M_tackle_dist = tmp;
    tmp = rcg::nltohd( from.tackle_back_dist );
    if ( 0.0 <= tmp && std::fabs( tmp ) < 1.0 ) M_tackle_back_dist = tmp;
    tmp = rcg::nltohd( from.tackle_width );
    if ( 0.0 < tmp && std::fabs( tmp ) < 2.0 ) M_tackle_width = tmp;

    //
    M_start_goal_l = rcg::nstohi( from.start_goal_l );
    M_start_goal_r = rcg::nstohi( from.start_goal_r );

    M_fullstate_l = rcg::nstohb( from.fullstate_l );
    M_fullstate_r = rcg::nstohb( from.fullstate_r );

    M_drop_ball_time = rcg::nstohi( from.drop_ball_time );

    M_synch_mode = rcg::nstohb( from.synch_mode );
    M_synch_offset = rcg::nstohi( from.synch_offset );
    M_synch_micro_sleep = rcg::nstohi( from.synch_micro_sleep );

    M_point_to_ban = rcg::nstohi( from.point_to_ban );
    M_point_to_duration = rcg::nstohi( from.point_to_duration );

    setAdditionalParam();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ServerParam::convertTo( rcg::server_params_t & to ) const
{
    to.goal_width = rcg::hdtonl( M_goal_width );
    to.inertia_moment = rcg::hdtonl( M_inertia_moment );

    to.player_size = rcg::hdtonl( M_player_size );
    to.player_decay = rcg::hdtonl( M_player_decay );
    to.player_rand = rcg::hdtonl( M_player_rand );
    to.player_weight = rcg::hdtonl( M_player_weight );
    to.player_speed_max = rcg::hdtonl( M_player_speed_max );
    to.player_accel_max = rcg::hdtonl( M_player_accel_max );

    to.stamina_max = rcg::hdtonl( M_stamina_max );
    to.stamina_inc = rcg::hdtonl( M_stamina_inc_max );

    to.recover_init = rcg::hdtonl( M_recover_init );
    to.recover_dec_thr = rcg::hdtonl( M_recover_dec_thr );
    to.recover_min = rcg::hdtonl( M_recover_min );
    to.recover_dec = rcg::hdtonl( M_recover_dec );

    to.effort_init = rcg::hdtonl( M_effort_init );
    to.effort_dec_thr = rcg::hdtonl( M_effort_dec_thr );
    to.effort_min = rcg::hdtonl( M_effort_min );
    to.effort_dec = rcg::hdtonl( M_effort_dec );
    to.effort_inc_thr = rcg::hdtonl( M_effort_inc_thr );
    to.effort_inc = rcg::hdtonl( M_effort_inc );

    to.kick_rand = rcg::hdtonl( M_kick_rand );
    to.team_actuator_noise = rcg::hbtons( M_team_actuator_noise );
    to.player_rand_factor_l = rcg::hdtonl( M_player_rand_factor_l );
    to.player_rand_factor_r = rcg::hdtonl( M_player_rand_factor_r );
    to.kick_rand_factor_l = rcg::hdtonl( M_kick_rand_factor_l );
    to.kick_rand_factor_r = rcg::hdtonl( M_kick_rand_factor_r );

    to.ball_size = rcg::hdtonl( M_ball_size );
    to.ball_decay = rcg::hdtonl( M_ball_decay );
    to.ball_rand = rcg::hdtonl( M_ball_rand );
    to.ball_weight = rcg::hdtonl( M_ball_weight );
    to.ball_speed_max = rcg::hdtonl( M_ball_speed_max );
    to.ball_accel_max = rcg::hdtonl( M_ball_accel_max );

    to.dash_power_rate = rcg::hdtonl( M_dash_power_rate );
    to.kick_power_rate = rcg::hdtonl( M_kick_power_rate );
    to.kickable_margin = rcg::hdtonl( M_kickable_margin );
    to.control_radius = rcg::hdtonl( M_control_radius );
    to.control_radius_width = rcg::hdtonl( M_control_radius_width );

    to.max_power = rcg::hdtonl( M_max_power );
    to.min_power = rcg::hdtonl( M_min_power );
    to.max_moment = rcg::hdtonl( M_max_moment );
    to.min_moment = rcg::hdtonl( M_min_moment );
    to.max_neck_moment = rcg::hdtonl( M_max_neck_moment );
    to.min_neck_moment = rcg::hdtonl( M_min_neck_moment );
    to.max_neck_angle = rcg::hdtonl( M_max_neck_angle );
    to.min_neck_angle = rcg::hdtonl( M_min_neck_angle );

    to.visible_angle = rcg::hdtonl( M_visible_angle );
    to.visible_distance = rcg::hdtonl( M_visible_distance );

    to.wind_dir = rcg::hdtonl( M_wind_dir );
    to.wind_force = rcg::hdtonl( M_wind_force );
    to.wind_ang = rcg::hdtonl( M_wind_angle );
    to.wind_rand = rcg::hdtonl( M_wind_rand );

    to.kickable_area = rcg::hdtonl( M_kickable_area );

    to.catch_area_l = rcg::hdtonl( M_catch_area_l );
    to.catch_area_w = rcg::hdtonl( M_catch_area_w );
    to.catch_probability = rcg::hdtonl( M_catch_probability );

    to.goalie_max_moves = rcg::hitons( M_goalie_max_moves );

    to.corner_kick_margin = rcg::hdtonl( M_corner_kick_margin );
    to.offside_active_area = rcg::hdtonl( M_offside_active_area_size );

    to.wind_none = rcg::hbtons( M_wind_none );
    to.use_wind_random = rcg::hbtons( M_use_wind_random );

    to.coach_say_count_max = rcg::hitons( M_coach_say_count_max );
    to.coach_say_msg_size = rcg::hitons( M_coach_say_msg_size );

    to.clang_win_size = rcg::hitons( M_clang_win_size );
    to.clang_define_win = rcg::hitons( M_clang_define_win );
    to.clang_meta_win = rcg::hitons( M_clang_meta_win );
    to.clang_advice_win = rcg::hitons( M_clang_advice_win );
    to.clang_info_win = rcg::hitons( M_clang_info_win );
    to.clang_mess_delay = rcg::hitons( M_clang_mess_delay );
    to.clang_mess_per_cycle = rcg::hitons( M_clang_mess_per_cycle );

    to.half_time = rcg::hitons( M_half_time );
    to.simulator_step = rcg::hitons( M_simulator_step );
    to.send_step = rcg::hitons( M_send_step );
    to.recv_step = rcg::hitons( M_recv_step );
    to.sense_body_step = rcg::hitons( M_sense_body_step );
    to.lcm_step = rcg::hitons( M_lcm_step );

    to.player_say_msg_size = rcg::hitons( M_player_say_msg_size );
    to.player_hear_max = rcg::hitons( M_player_hear_max );
    to.player_hear_inc = rcg::hitons( M_player_hear_inc );
    to.player_hear_decay = rcg::hitons( M_player_hear_decay );

    to.catch_ban_cycle = rcg::hitons( M_catch_ban_cycle );

    to.slow_down_factor = rcg::hitons( M_slow_down_factor );

    to.use_offside = rcg::hbtons( M_use_offside );
    to.kickoff_offside = rcg::hbtons( M_kickoff_offside );
    to.offside_kick_margin = rcg::hdtonl( M_offside_kick_margin );

    to.audio_cut_dist = rcg::hdtonl( M_audio_cut_dist );

    to.dist_quantize_step = rcg::hdtonl( M_dist_quantize_step );
    to.landmark_dist_quantize_step = rcg::hdtonl( M_landmark_dist_quantize_step );
    to.dir_quantize_step = rcg::hdtonl( M_dir_quantize_step );
    to.dist_quantize_step_l = rcg::hdtonl( M_dist_quantize_step_l );
    to.dist_quantize_step_r = rcg::hdtonl( M_dist_quantize_step_r );
    to.landmark_dist_quantize_step_l = rcg::hdtonl( M_landmark_dist_quantize_step_l );
    to.landmark_dist_quantize_step_r = rcg::hdtonl( M_landmark_dist_quantize_step_r );
    to.dir_quantize_step_l = rcg::hdtonl( M_dir_quantize_step_l );
    to.dir_quantize_step_r = rcg::hdtonl( M_dir_quantize_step_r );

    to.coach_mode = rcg::hbtons( M_coach_mode );
    to.coach_with_referee_mode = rcg::hbtons( M_coach_with_referee_mode );
    to.use_old_coach_hear = rcg::hbtons( M_use_old_coach_hear );

    to.online_coach_look_step = rcg::hitons( M_coach_send_vi_step );

    to.slowness_on_top_for_left_team = rcg::hdtonl( M_slowness_on_top_for_left_team );
    to.slowness_on_top_for_right_team = rcg::hdtonl( M_slowness_on_top_for_right_team );

    to.ka_length = rcg::hdtonl( M_keepaway_length );
    to.ka_width = rcg::hdtonl( M_keepaway_width );

    // 11.0.0
    to.ball_stuck_area = rcg::hdtonl( M_ball_stuck_area );
    // 12.0.0
    to.max_tackle_power = rcg::hdtonl( M_max_tackle_power );
    to.max_back_tackle_power = rcg::hdtonl( M_max_back_tackle_power );

    to.tackle_dist = rcg::hdtonl( M_tackle_dist );
    to.tackle_back_dist = rcg::hdtonl( M_tackle_back_dist );
    to.tackle_width = rcg::hdtonl( M_tackle_width );

    to.start_goal_l = rcg::hitons( M_start_goal_l );
    to.start_goal_r = rcg::hitons( M_start_goal_r );

    to.fullstate_l = rcg::hbtons( M_fullstate_l );
    to.fullstate_r = rcg::hbtons( M_fullstate_r );

    to.drop_ball_time = rcg::hitons( M_drop_ball_time );

    to.synch_mode = rcg::hbtons( M_synch_mode );
    to.synch_offset = rcg::hitons( M_synch_offset );
    to.synch_micro_sleep = rcg::hitons( M_synch_micro_sleep );

    to.point_to_ban = rcg::hitons( M_point_to_ban );
    to.point_to_duration = rcg::hitons( M_point_to_duration );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
ServerParam::toServerString() const
{
    std::ostringstream os;

    os << "(server_param ";

    for ( std::map< std::string, ParamEntity::Ptr >::const_iterator it = M_param_map->longNameMap().begin(), end = M_param_map->longNameMap().end();
          it != end;
          ++it )
    {
        os << '(' << it->second->longName() << ' ';
        it->second->printValue( os );
        os << ')';
    }

    os << ')';

    return os.str();
}

/*-------------------------------------------------------------------*/
/*!

*/
double
ServerParam::dashDirRate( const double & dir ) const
{
    double d = discretizeDashAngle( dir );
    double r = std::fabs( d ) > 90.0
        ? backDashRate() - ( ( backDashRate() - sideDashRate() )
                             * ( 1.0 - ( std::fabs( d ) - 90.0 ) / 90.0 )
                             )
        : sideDashRate() + ( ( 1.0 - sideDashRate() )
                             * ( 1.0 - std::fabs( d ) / 90.0 ) )
        ;
    return std::min( std::max( 1.0e-5, r ), 1.0 );
}

}
