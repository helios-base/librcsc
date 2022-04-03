// -*-c++-*-

/*!
  \file server_param.h
  \brief server parametor Header File
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

#ifndef RCSC_PARAM_SERVER_PARAM_H
#define RCSC_PARAM_SERVER_PARAM_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/math_util.h>

#include <memory>

#include <string>

namespace rcsc {

namespace rcg {
struct server_params_t;
}
class ParamMap;

/*!
  \class ServerParam
  \brief server parameter set
 */
class ServerParam {
private:

    //! parameter map implementation
    std::unique_ptr< ParamMap > M_param_map;

public:
    //////////////////////////////////////////////////////
    // FIXED field parameters: defined in rcssserver/param.h
    static const int    DEFAULT_MAX_PLAYER;
    static const double DEFAULT_PITCH_LENGTH;
    static const double DEFAULT_PITCH_WIDTH;
    static const double DEFAULT_PITCH_MARGIN;
    static const double DEFAULT_CENTER_CIRCLE_R;
    static const double DEFAULT_PENALTY_AREA_LENGTH;
    static const double DEFAULT_PENALTY_AREA_WIDTH;
    static const double DEFAULT_PENALTY_SPOT_DIST;
    static const double DEFAULT_PENALTY_CIRCLE_R;
    static const double DEFAULT_GOAL_AREA_LENGTH;
    static const double DEFAULT_GOAL_AREA_WIDTH;
    static const double DEFAULT_GOAL_DEPTH;
    static const double DEFAULT_CORNER_ARC_R;
    //static const double DEFAULT_KICK_OFF_CLEAR_DISTANCE;
    static const double DEFAULT_GOAL_POST_RADIUS;

    static const double DEFAULT_WIND_WEIGHT; // = 10000.0

    //////////////////////////////////////////////////////
    // Parameters receivable from rcsserver-7+

    // defined in server_param_t
    static const double DEFAULT_GOAL_WIDTH;
    static const double DEFAULT_INERTIA_MOMENT; // defined as IMPARAM 5.0 in rcssserver/param.h

    static const double DEFAULT_PLAYER_SIZE;
    static const double DEFAULT_PLAYER_DECAY;
    static const double DEFAULT_PLAYER_RAND;
    static const double DEFAULT_PLAYER_WEIGHT;
    static const double DEFAULT_PLAYER_SPEED_MAX;
    static const double DEFAULT_PLAYER_ACCEL_MAX;

    static const double DEFAULT_STAMINA_MAX;
    static const double DEFAULT_STAMINA_INC_MAX;

    static const double DEFAULT_RECOVER_INIT;
    static const double DEFAULT_RECOVER_DEC_THR;
    static const double DEFAULT_RECOVER_MIN;
    static const double DEFAULT_RECOVER_DEC;

    static const double DEFAULT_EFFORT_INIT;
    static const double DEFAULT_EFFORT_DEC_THR;
    static const double DEFAULT_EFFORT_MIN;
    static const double DEFAULT_EFFORT_DEC;
    static const double DEFAULT_EFFORT_INC_THR;
    static const double DEFAULT_EFFORT_INC;

    static const double DEFAULT_KICK_RAND;
    static const bool   DEFAULT_TEAM_ACTUATOR_NOISE;
    static const double DEFAULT_PLAYER_RAND_FACTOR_L;
    static const double DEFAULT_PLAYER_RAND_FACTOR_R;
    static const double DEFAULT_KICK_RAND_FACTOR_L;
    static const double DEFAULT_KICK_RAND_FACTOR_R;

    static const double DEFAULT_BALL_SIZE;
    static const double DEFAULT_BALL_DECAY;
    static const double DEFAULT_BALL_RAND;
    static const double DEFAULT_BALL_WEIGHT;
    static const double DEFAULT_BALL_SPEED_MAX;
    static const double DEFAULT_BALL_ACCEL_MAX;

    static const double DEFAULT_DASH_POWER_RATE;
    static const double DEFAULT_KICK_POWER_RATE;
    static const double DEFAULT_KICKABLE_MARGIN;
    static const double DEFAULT_CONTROL_RADIUS;
    //static const double DEFAULT_CONTROL_RADIUS_WIDTH;

    static const double DEFAULT_MAX_POWER;
    static const double DEFAULT_MIN_POWER;
    static const double DEFAULT_MAX_MOMENT;
    static const double DEFAULT_MIN_MOMENT;
    static const double DEFAULT_MAX_NECK_MOMENT;
    static const double DEFAULT_MIN_NECK_MOMENT;
    static const double DEFAULT_MAX_NECK_ANGLE;
    static const double DEFAULT_MIN_NECK_ANGLE;

    static const double DEFAULT_VISIBLE_ANGLE;
    static const double DEFAULT_VISIBLE_DISTANCE;

    static const double DEFAULT_WIND_DIR;
    static const double DEFAULT_WIND_FORCE;
    static const double DEFAULT_WIND_ANGLE;
    static const double DEFAULT_WIND_RAND;

    //static const double DEFAULT_KICKABLE_AREA;

    static const double DEFAULT_CATCH_AREA_L;
    static const double DEFAULT_CATCH_AREA_W;
    static const double DEFAULT_CATCH_PROBABILITY;
    static const int    DEFAULT_GOALIE_MAX_MOVES;

    static const double DEFAULT_CORNER_KICK_MARGIN;
    static const double DEFAULT_OFFSIDE_ACTIVE_AREA_SIZE;

    static const bool   DEFAULT_WIND_NONE;
    static const bool   DEFAULT_USE_WIND_RANDOM;

    static const int    DEFAULT_COACH_SAY_COUNT_MAX; // defined as DEF_SAY_COACH_CNT_MAX in rcssserver/param.h
    static const int    DEFAULT_COACH_SAY_MSG_SIZE; // defined as DEF_SAY_COACH_MSG_SIZE in rcssserver/param.h

    static const int    DEFAULT_CLANG_WIN_SIZE;
    static const int    DEFAULT_CLANG_DEFINE_WIN;
    static const int    DEFAULT_CLANG_META_WIN;
    static const int    DEFAULT_CLANG_ADVICE_WIN;
    static const int    DEFAULT_CLANG_INFO_WIN;
    static const int    DEFAULT_CLANG_MESS_DELAY;
    static const int    DEFAULT_CLANG_MESS_PER_CYCLE;

    static const int    DEFAULT_HALF_TIME;
    static const int    DEFAULT_SIMULATOR_STEP; // defined as SIMULATOR_STEP_INTERVAL_MSEC in rcssserver/param.h
    static const int    DEFAULT_SEND_STEP; // defined as UDP_SEND_STEP_INTERVAL_MSEC in rcssserver/param.h
    static const int    DEFAULT_RECV_STEP; // defined as UDP_RECV_STEP_INTERVAL_MSEC in rcssserver/param.h
    static const int    DEFAULT_SENSE_BODY_STEP; // defined as SENSE_BODY_INTERVAL_MSEC in rcssserver/param.h
    //static const int    DEFAULT_LCM_STEP

    static const int    DEFAULT_PLAYER_SAY_MSG_SIZE;
    static const int    DEFAULT_PLAYER_HEAR_MAX;
    static const int    DEFAULT_PLAYER_HEAR_INC;
    static const int    DEFAULT_PLAYER_HEAR_DECAY;

    static const int    DEFAULT_CATCH_BAN_CYCLE;

    static const int    DEFAULT_SLOW_DOWN_FACTOR;

    static const bool   DEFAULT_USE_OFFSIDE;
    static const bool   DEFAULT_KICKOFF_OFFSIDE;
    static const double DEFAULT_OFFSIDE_KICK_MARGIN;

    static const double DEFAULT_AUDIO_CUT_DIST;

    static const double DEFAULT_DIST_QUANTIZE_STEP; // defined as DIST_QSTEP in rcssserver/param.h
    static const double DEFAULT_LANDMARK_DIST_QUANTIZE_STEP; // defined as LAND_QSTEP in rcssserver/param.h
    static const double DEFAULT_DIR_QUANTIZE_STEP; // defined as DIR_QSTEP in rcssserver/param.h
    //static const double DEFAULT_DIST_QUANTIZE_STEP_L;
    //static const double DEFAULT_DIST_QUANTIZE_STEP_R;
    //static const double DEFAULT_LANDMARK_DIST_QUANTIZE_STEP_L;
    //static const double DEFAULT_LANDMARK_DIST_QUANTIZE_STEP_R;
    //static const double DEFAULT_DIR_QUANTIZE_STEP_L;
    //static const double DEFAULT_DIR_QUANTIZE_STEP_R;

    static const bool   DEFAULT_COACH_MODE;
    static const bool   DEFAULT_COACH_WITH_REFEREE_MODE;
    static const bool   DEFAULT_USE_OLD_COACH_HEAR;

    static const double DEFAULT_SLOWNESS_ON_TOP_FOR_LEFT_TEAM;
    static const double DEFAULT_SLOWNESS_ON_TOP_FOR_RIGHT_TEAM;


    static const int    DEFAULT_START_GOAL_L;
    static const int    DEFAULT_START_GOAL_R;

    static const bool   DEFAULT_FULLSTATE_L;
    static const bool   DEFAULT_FULLSTATE_R;

    static const int    DEFAULT_DROP_BALL_TIME; // defined as DROP_TIME in rcssserver/param.h

    static const bool   DEFAULT_SYNC_MODE;
    static const int    DEFAULT_SYNC_OFFSET;
    static const int    DEFAULT_SYNC_MICRO_SLEEP;

    static const int    DEFAULT_POINT_TO_BAN;
    static const int    DEFAULT_POINT_TO_DURATION;


    // not defined in server_param_t
    static const int    DEFAULT_PLAYER_PORT;
    static const int    DEFAULT_TRAINER_PORT;
    static const int    DEFAULT_ONLINE_COACH_PORT;

    static const bool   DEFAULT_VERBOSE_MODE;

    static const int    DEFAULT_COACH_SEND_VI_STEP; // defined as SEND_VISUALINFO_INTERVAL_MSEC in rcssserver/param.h

    static const std::string DEFAULT_REPLAY_FILE; // unused after rcsserver-9+
    static const std::string DEFAULT_LANDMARK_FILE;

    static const int    DEFAULT_SEND_COMMS;

    static const bool   DEFAULT_TEXT_LOGGING;
    static const bool   DEFAULT_GAME_LOGGING;
    static const int    DEFAULT_GAME_LOG_VERSION;
    static const std::string DEFAULT_TEXT_LOG_DIR;
    static const std::string DEFAULT_GAME_LOG_DIR;
    static const std::string DEFAULT_TEXT_LOG_FIXED_NAME;
    static const std::string DEFAULT_GAME_LOG_FIXED_NAME;
    static const bool   DEFAULT_USE_TEXT_LOG_FIXED;
    static const bool   DEFAULT_USE_GAME_LOG_FIXED;
    static const bool   DEFAULT_USE_TEXT_LOG_DATED;
    static const bool   DEFAULT_USE_GAME_LOG_DATED;
    static const std::string DEFAULT_LOG_DATE_FORMAT;
    static const bool   DEFAULT_LOG_TIMES;
    static const bool   DEFAULT_RECORD_MESSAGES;
    static const int    DEFAULT_TEXT_LOG_COMPRESSION;
    static const int    DEFAULT_GAME_LOG_COMPRESSION;

    static const bool   DEFAULT_USE_PROFILE;

    static const double DEFAULT_TACKLE_DIST;
    static const double DEFAULT_TACKLE_BACK_DIST;
    static const double DEFAULT_TACKLE_WIDTH;
    static const double DEFAULT_TACKLE_EXPONENT;
    static const int    DEFAULT_TACKLE_CYCLES;
    static const double DEFAULT_TACKLE_POWER_RATE;

    static const int    DEFAULT_FREEFORM_WAIT_PERIOD;
    static const int    DEFAULT_FREEFORM_SEND_PERIOD;

    static const bool   DEFAULT_FREE_KICK_FAULTS;
    static const bool   DEFAULT_BACK_PASSES;

    static const bool   DEFAULT_PROPER_GOAL_KICKS;
    static const double DEFAULT_STOPPED_BALL_VEL;
    static const int    DEFAULT_MAX_GOAL_KICKS;

    static const int    DEFAULT_CLANG_DEL_WIN;
    static const int    DEFAULT_CLANG_RULE_WIN;

    static const bool   DEFAULT_AUTO_MODE;
    static const int    DEFAULT_KICK_OFF_WAIT;
    static const int    DEFAULT_CONNECT_WAIT;
    static const int    DEFAULT_GAME_OVER_WAIT;
    static const std::string DEFAULT_TEAM_L_START;
    static const std::string DEFAULT_TEAM_R_START;

    static const bool   DEFAULT_KEEPAWAY_MODE;
    static const double DEFAULT_KEEPAWAY_LENGTH;
    static const double DEFAULT_KEEPAWAY_WIDTH;

    static const bool   DEFAULT_KEEPAWAY_LOGGING;
    static const std::string DEFAULT_KEEPAWAY_LOG_DIR;
    static const std::string DEFAULT_KEEPAWAY_LOG_FIXED_NAME;
    static const bool   DEFAULT_KEEPAWAY_LOG_FIXED;
    static const bool   DEFAULT_KEEPAWAY_LOG_DATED;

    static const int    DEFAULT_KEEPAWAY_START;

    static const int    DEFAULT_NR_NORMAL_HALFS;
    static const int    DEFAULT_NR_EXTRA_HALFS;
    static const bool   DEFAULT_PENALTY_SHOOT_OUTS;

    static const int    DEFAULT_PEN_BEFORE_SETUP_WAIT;
    static const int    DEFAULT_PEN_SETUP_WAIT;
    static const int    DEFAULT_PEN_READY_WAIT;
    static const int    DEFAULT_PEN_TAKEN_WAIT;
    static const int    DEFAULT_PEN_NR_KICKS;
    static const int    DEFAULT_PEN_MAX_EXTRA_KICKS;
    static const double DEFAULT_PEN_DIST_X;
    static const bool   DEFAULT_PEN_RANDOM_WINNER;
    static const bool   DEFAULT_PEN_ALLOW_MULT_KICKS;
    static const double DEFAULT_PEN_MAX_GOALIE_DIST_X;
    static const bool   DEFAULT_PEN_COACH_MOVES_PLAYERS;

    static const std::string DEFAULT_MODULE_DIR;

    static const double DEFAULT_BALL_STUCK_AREA;
    // 12
    static const double DEFAULT_MAX_TACKLE_POWER;
    static const double DEFAULT_MAX_BACK_TACKLE_POWER;
    static const double DEFAULT_PLAYER_SPEED_MAX_MIN;
    static const double DEFAULT_EXTRA_STAMINA;
    static const int DEFAULT_SYNCH_SEE_OFFSET;
    // 12.1.3
    static const int EXTRA_HALF_TIME;
    // 13.0.0
    static const double	STAMINA_CAPACITY;
    static const double MAX_DASH_ANGLE;
    static const double MIN_DASH_ANGLE;
    static const double DASH_ANGLE_STEP;
    static const double SIDE_DASH_RATE;
    static const double BACK_DASH_RATE;
    static const double MAX_DASH_POWER;
    static const double MIN_DASH_POWER;
    // 14.0.0
    static const double TACKLE_RAND_FACTOR;
    static const double FOUL_DETECT_PROBABILITY;
    static const double FOUL_EXPONENT;
    static const int FOUL_CYCLES;
    // 15.0.0
    static const double RED_CARD_PROBABILITY;
    // 16.0.0
    static const int ILLEGAL_DEFENSE_DURATION;
    static const int ILLEGAL_DEFENSE_NUMBER;
    static const double ILLEGAL_DEFENSE_DIST_X;
    static const double ILLEGAL_DEFENSE_WIDTH;
    // 17.0
    static const double MAX_CATCH_ANGLE;
    static const double MIN_CATCH_ANGLE;
private:

    //////////////////////////////////////////////////////
    //////////////////////////////////////////////////////
    // configurable variables

    // defined in server_param_t
    double M_goal_width;
    double M_inertia_moment;

    double M_player_size;
    double M_player_decay;
    double M_player_rand;
    double M_player_weight;
    double M_player_speed_max;
    double M_player_accel_max;

    double M_stamina_max;
    double M_stamina_inc_max;

    double M_recover_init;
    double M_recover_dec_thr;
    double M_recover_min;
    double M_recover_dec;

    double M_effort_init;
    double M_effort_dec_thr;
    double M_effort_min;
    double M_effort_dec;
    double M_effort_inc_thr;
    double M_effort_inc;

    double M_kick_rand;
    bool M_team_actuator_noise;
    double M_player_rand_factor_l;
    double M_player_rand_factor_r;
    double M_kick_rand_factor_l;
    double M_kick_rand_factor_r;

    double M_ball_size;
    double M_ball_decay;
    double M_ball_rand;
    double M_ball_weight;
    double M_ball_speed_max;
    double M_ball_accel_max;

    double M_dash_power_rate;
    double M_kick_power_rate;
    double M_kickable_margin;
    double M_control_radius;
    double M_control_radius_width;

    double M_max_power;
    double M_min_power;
    double M_max_moment;
    double M_min_moment;
    double M_max_neck_moment;
    double M_min_neck_moment;
    double M_max_neck_angle;
    double M_min_neck_angle;

    double M_visible_angle;
    double M_visible_distance;

    double M_wind_dir;
    double M_wind_force;
    double M_wind_angle;
    double M_wind_rand;

    double M_kickable_area;

    double M_catch_area_l;
    double M_catch_area_w;
    double M_catch_probability;
    int M_goalie_max_moves;

    double M_corner_kick_margin;
    double M_offside_active_area_size;

    bool M_wind_none;
    bool M_use_wind_random;

    int M_coach_say_count_max;
    int M_coach_say_msg_size;

    int M_clang_win_size;
    int M_clang_define_win;
    int M_clang_meta_win;
    int M_clang_advice_win;
    int M_clang_info_win;
    int M_clang_mess_delay;
    int M_clang_mess_per_cycle;

    int M_half_time;
    int M_simulator_step;
    int M_send_step; // player's normal visual info step
    int M_recv_step;
    int M_sense_body_step;
    int M_lcm_step;

    int M_player_say_msg_size;
    int M_player_hear_max;
    int M_player_hear_inc;
    int M_player_hear_decay;

    int M_catch_ban_cycle;

    int M_slow_down_factor;

    bool M_use_offside;
    bool M_kickoff_offside;
    double M_offside_kick_margin;

    double M_audio_cut_dist;

    double M_dist_quantize_step;
    double M_landmark_dist_quantize_step;
    double M_dir_quantize_step;
    double M_dist_quantize_step_l;
    double M_dist_quantize_step_r;
    double M_landmark_dist_quantize_step_l;
    double M_landmark_dist_quantize_step_r;
    double M_dir_quantize_step_l;
    double M_dir_quantize_step_r;

    bool M_coach_mode;
    bool M_coach_with_referee_mode;
    bool M_use_old_coach_hear;

    double M_slowness_on_top_for_left_team;
    double M_slowness_on_top_for_right_team;

    int M_start_goal_l;
    int M_start_goal_r;

    bool M_fullstate_l;
    bool M_fullstate_r;

    int M_drop_ball_time;

    //--------------------------------------------------------
    // version 8

    bool M_synch_mode;
    int M_synch_offset;
    int M_synch_micro_sleep;

    int M_point_to_ban; // if cycle diff from pointto command done is within this step, player cannot point other point
    int M_point_to_duration; // pointto action has effect at least in this step


    // not defined in server_param_t
    int M_player_port;
    int M_trainer_port;
    int M_online_coach_port;

    bool M_verbose_mode;

    int M_coach_send_vi_step; // coach's visual info step

    std::string M_replay_file; // unused after rcsserver-9+
    std::string M_landmark_file;

    bool M_send_comms; // flag for whether the message info is sent to monitor

    bool M_text_logging;
    bool M_game_logging;
    int M_game_log_version;
    std::string M_text_log_dir;
    std::string M_game_log_dir;
    std::string M_text_log_fixed_name;
    std::string M_game_log_fixed_name;
    bool M_use_text_log_fixed;
    bool M_use_game_log_fixed;
    bool M_use_text_log_dated;
    bool M_use_game_log_dated;
    std::string M_log_date_format;
    bool M_log_times; // flag for whether the cycle time infomation is written to text log
    bool M_record_message; // flag for whether the MSG_MODE data is written to game log(only version 1 & 2)
    int M_text_log_compression; // zlib compression level
    int M_game_log_compression;

    bool M_use_profile; // flag for whether the profile infomation with string is written to text log

    double M_tackle_dist;
    double M_tackle_back_dist;
    double M_tackle_width;
    double M_tackle_exponent;
    int M_tackle_cycles; // player is freezed this cycle, after tackle done
    double M_tackle_power_rate;

    int M_freeform_wait_period; // coach can only send one freeform advice per this period
    int M_freeform_send_period; // coach can send the freeform advice during this period

    bool M_free_kick_faults;
    bool M_back_passes;

    bool M_proper_goal_kicks; // flag for whether referee check the goal kick fault
    double M_stopped_ball_vel; // referee's judgement criterion for ball stopping when goal kicks
    int M_max_goal_kicks; // limit of goal kick fault

    int M_clang_del_win;
    int M_clang_rule_win;

    //--------------------------------------------------------
    // version 9

    // rcssserver-9.0.2
    bool M_auto_mode;    // auto mode on/off
    int M_kick_off_wait; // when auto_mode, server wait this cycle before automatic kick off
    int M_connect_wait;  // when auto_mode, server wait this cycle for team connecting
    int M_game_over_wait; // when auto_mode, server wait this cycle after 2 half
    std::string M_team_l_start; // when auto_mode, server invoke this command line to start left team
    std::string M_team_r_start;

    // rcssserver-9.1.0
    // keepaway params
    bool M_keepaway_mode; // kaway ; // keepaway mode on/off
    double M_keepaway_length; // keepaway region length
    double M_keepaway_width;  // keepaway region width

    bool M_keepaway_logging; // keepaway logging on/off
    std::string M_keepaway_log_dir;
    std::string M_keepaway_log_fixed_name;
    bool M_keepaway_log_fixed;
    bool M_keepaway_log_dated;

    int M_keepaway_start; // seconds waited before new episode

    // rcssserver-9.3.0
    // penalty shootouts params
    int M_nr_normal_halfs;     // nr of normal halfs: default 2)
    int M_nr_extra_halfs;      // nr of extra halfs: -1 is infinite)
    bool M_penalty_shoot_outs; // penalty shoot outs after extra halfs

    int M_pen_before_setup_wait; // cycles waited before penalty setup
    int M_pen_setup_wait;        // cycles waited to setup penalty
    int M_pen_ready_wait;        // cycles waited to take penalty
    int M_pen_taken_wait;        // cycles waited to finish penalty
    int M_pen_nr_kicks;          // number of penalty kicks
    int M_pen_max_extra_kicks;   // max. nr of extra penalty kicks
    double M_pen_dist_x;         // distance from goal to place ball
    bool M_pen_random_winner;    // random winner when draw?
    bool M_pen_allow_mult_kicks; // can attacker kick mult. times
    double M_pen_max_goalie_dist_x; // max distance from goal for goalie

    // rcssserver-9.4.0
    bool M_pen_coach_moves_players; // coach moves players when positioned wrongly
    std::string M_module_dir;

    // rcssserver-11.0.0
    double M_ball_stuck_area;
    std::string M_coach_msg_file;

    // rcssserver-12.0.0
    double M_max_tackle_power;
    double M_max_back_tackle_power;
    double M_player_speed_max_min;
    double M_extra_stamina;
    int M_synch_see_offset;

    // 12.1.3
    int M_extra_half_time;

    // 13.0.0
    double M_stamina_capacity;
    double M_max_dash_angle;
    double M_min_dash_angle;
    double M_dash_angle_step;
    double M_side_dash_rate;
    double M_back_dash_rate;
    double M_max_dash_power;
    double M_min_dash_power;

    // 14.0.0
    double M_tackle_rand_factor;
    double M_foul_detect_probability;
    double M_foul_exponent;
    int M_foul_cycles;
    bool M_golden_goal;

    // 15.0.0
    double M_red_card_probability;

    // 16.0.0
    int M_illegal_defense_duration;
    int M_illegal_defense_number;
    double M_illegal_defense_dist_x;
    double M_illegal_defense_width;
    std::string M_fixed_teamname_l;
    std::string M_fixed_teamname_r;

    // 17.0
    double M_max_catch_angle;
    double M_min_catch_angle;

    // xxx
    int M_random_seed;
    double M_long_kick_power_factor;
    int M_long_kick_delay;

    // 999
    int M_max_monitors;

    //--------------------------------------------------------
    // additional params
    double M_catchable_area; //!< real catchable length (diagonal line length)
    double M_real_speed_max; //!< default player's real max speed

private:
    /*!
      \brief constructor defined as private member for Singleton Pattern
    */
    ServerParam();

    /*!
      \brief create name-param map
     */
    void createMap();

    /*!
      \brief set defaut values
    */
    void setDefaultParam();

    /*!
      \brief calculate additional parameters
     */
    void setAdditionalParam();

public:

    /*!
      \brief singleton instance interface
      \return reference to local static instance
     */
    static
    ServerParam & instance();

    /*!
      \brief singleton instance interface
      \return const reference to local static instance
     */
    inline
    static const
    ServerParam & i()
      {
          return instance();
      }

    /*!
      \brief analyze server message
      \param msg raw message string from server
      \param version client version that defines message protocol
      \return result of parse status
     */
    bool parse( const char * msg,
                const double & version );

private:
    /*!
      \brief analyze version 7 protocol message
      \param msg raw message string from rcssserver
      \return result of parse status
     */
    bool parseV7( const char * msg );

public:

    /*
    void applySlowDownFactor()
      {
          M_simulator_step  *= M_slow_down_factor;
          M_sense_body_step *= M_slow_down_factor;
          M_coach_send_vi_step *= M_slow_down_factor;
          M_send_step *= M_slow_down_factor;
          M_synch_offset *= M_slow_down_factor;
      }
     */

    /*!
      \brief convert from the monitor protocol format
      \param from monitor protocol data structure
     */
    void convertFrom( const rcg::server_params_t & from );

    /*!
      \brief convert to the monitor protocol format
      \param to monitor protocol data structure
     */
    void convertTo( rcg::server_params_t & to ) const;

    /*!
      \brief convert to the rcss parameter message
      \return parameter message string
     */
    std::string toServerString() const;


    // static parameters

    int maxPlayer() const
      {
          return DEFAULT_MAX_PLAYER;
      }

    double pitchLength() const
      {
          return DEFAULT_PITCH_LENGTH;
      }

    double pitchWidth() const
      {
          return DEFAULT_PITCH_WIDTH;
      }

    double pitchMargin() const
      {
          return DEFAULT_PITCH_MARGIN;
      }

    double centerCircleR() const
      {
          return DEFAULT_CENTER_CIRCLE_R;
      }

    double penaltyAreaLength() const
      {
          return DEFAULT_PENALTY_AREA_LENGTH;
      }

    double penaltyAreaWidth() const
      {
          return DEFAULT_PENALTY_AREA_WIDTH;
      }

    double goalAreaLength() const
      {
          return DEFAULT_GOAL_AREA_LENGTH;
      }

    double goalAreaWidth() const
      {
          return DEFAULT_GOAL_AREA_WIDTH;
      }

    double goalDepth() const
      {
          return DEFAULT_GOAL_DEPTH;
      }

    double penaltyCircleR() const
      {
          return DEFAULT_PENALTY_CIRCLE_R;
      }

    double penaltySpotDist() const
      {
          return DEFAULT_PENALTY_SPOT_DIST;
      }

    double cornerArcR() const
      {
          return DEFAULT_CORNER_ARC_R;
      }

    double kickOffClearDistance() const
      {
          return DEFAULT_CENTER_CIRCLE_R;
      }

    double windWeight() const
      {
          return DEFAULT_WIND_WEIGHT;
      }

    double goalPostRadius() const
      {
          return DEFAULT_GOAL_POST_RADIUS;
      }


    // configurable parameters

    double goalWidth() const { return M_goal_width; }
    // hetero param
    double defaultInertiaMoment() const { return M_inertia_moment; }
    // hetero param
    double defaultPlayerSize() const { return M_player_size; }
    // hetero param
    double defaultPlayerDecay() const { return M_player_decay; }
    double playerRand() const { return M_player_rand; }
    double playerWeight() const { return M_player_weight; }
    // hetero param
    double defaultPlayerSpeedMax() const { return M_player_speed_max; }
    double playerAccelMax() const { return M_player_accel_max; }
    double staminaMax() const { return M_stamina_max; }
    // hetero param
    double defaultStaminaIncMax() const { return M_stamina_inc_max; }
    double recoverInit() const { return M_recover_init; }
    double recoverDecThr() const { return M_recover_dec_thr; }
    double recoverMin() const { return M_recover_min; }
    double recoverDec() const { return M_recover_dec; }
    double effortInit() const { return M_effort_init; }
    double effortDecThr() const { return M_effort_dec_thr; }
    // hetero param
    double defaultEffortMax() const { return M_effort_init; }
    // hetero param
    double defaultEffortMin() const { return M_effort_min; }
    double effortDec() const { return M_effort_dec; }
    double effortIncThr() const { return M_effort_inc_thr; }
    double effortInc() const { return M_effort_inc; }
    // hetero param
    double defaultKickRand() const { return M_kick_rand; }
    bool teamActuatorNoise() const { return M_team_actuator_noise; }
    double playerRandFactorLeft() const { return M_player_rand_factor_l; }
    double playerRandFactorRight() const { return M_player_rand_factor_r; }
    double kickRandFactorLeft() const { return M_kick_rand_factor_l; }
    double kickRandFactorRight() const { return M_kick_rand_factor_r; }
    double ballSize() const { return M_ball_size; }
    double ballDecay() const { return M_ball_decay; }
    double ballRand() const { return M_ball_rand; }
    double ballWeight() const { return M_ball_weight; }
    double ballSpeedMax() const { return M_ball_speed_max; }
    double ballAccelMax() const { return M_ball_accel_max; }
    // hetero param
    double defaultDashPowerRate() const { return M_dash_power_rate; }
    double kickPowerRate() const { return M_kick_power_rate; }
    // hetero param
    double defaultKickableMargin() const { return M_kickable_margin; }
    double controlRadius() const { return M_control_radius; }
    double controlRadiusWidth() const { return M_control_radius_width; }

    double maxPower() const { return M_max_power; }
    double minPower() const { return M_min_power; }
    double maxMoment() const { return M_max_moment; }
    double minMoment() const { return M_min_moment; }
    double maxNeckMoment() const { return M_max_neck_moment; }
    double minNeckMoment() const { return M_min_neck_moment; }
    double maxNeckAngle() const { return M_max_neck_angle; }
    double minNeckAngle() const { return M_min_neck_angle; }

    double visibleAngle() const { return M_visible_angle; }
    double visibleDistance() const { return M_visible_distance; }

    double windDir() const { return M_wind_dir; }
    double windForce() const { return M_wind_force; }
    double windAngle() const { return M_wind_angle; }
    double windRand() const { return M_wind_rand; }

    double defaultKickableArea() const { return M_kickable_area; }
    double catchAreaLength() const { return M_catch_area_l; }
    double catchAreaWidth() const { return M_catch_area_w; }
    double catchProbability() const { return M_catch_probability; }
    int goalieMaxMoves() const { return M_goalie_max_moves; }

    double cornerKickMargin() const { return M_corner_kick_margin; }
    double offsideActiveAreaSize() const { return M_offside_active_area_size; }

    bool windNone() const { return M_wind_none; }
    bool useWindRandom() const { return M_use_wind_random; }

    int coachSayCountMax() const { return M_coach_say_count_max; }
    int coachSayMsgSize() const { return M_coach_say_msg_size; }

    int clangWinSize() const { return M_clang_win_size; }
    int clangDefineWin() const { return M_clang_define_win; }
    int clangMetaWin() const { return M_clang_meta_win; }
    int clangAdviceWin() const { return M_clang_advice_win; }
    int clangInfoWin() const { return M_clang_info_win; }
    int clangMessDelay() const { return M_clang_mess_delay; }
    int clangMessPerCycle() const { return M_clang_mess_per_cycle; }

    int halfTime() const { return M_half_time; }
    int simulatorStep() const { return M_simulator_step; }
    int sendStep() const { return M_send_step; }
    int recvStep() const { return M_recv_step; }
    int senseBodyStep() const { return M_sense_body_step; }
    int lcmStep() const { return M_lcm_step; }

    int playerSayMsgSize() const { return M_player_say_msg_size; }
    int playerHearMax() const { return M_player_hear_max; }
    int playerHearInc() const { return M_player_hear_inc; }
    int playerHearDecay() const { return M_player_hear_decay; }

    int catchBanCycle() const { return M_catch_ban_cycle; }

    int slowDownFactor() const { return M_slow_down_factor; }

    bool useOffside() const { return M_use_offside;}
    bool kickoffOffside() const { return M_kickoff_offside; }
    double offsideKickMargin() const { return M_offside_kick_margin; }

    double audioCutDist() const { return M_audio_cut_dist; }

    double distQuantizeStep() const { return M_dist_quantize_step; }
    double landmarkDistQuantizeStep() const { return M_landmark_dist_quantize_step; }
    double dirQuantizeStep() const { return M_dir_quantize_step; }
    double distQuantizeSteqLeft() const { return M_dist_quantize_step_l; }
    double distQuantizeStepRight() const { return M_dist_quantize_step_r; }
    double landmarkDistQuantizeStepLeft() const { return M_landmark_dist_quantize_step_l; }
    double landmarkDistQuantizeStepRight() const { return M_landmark_dist_quantize_step_r; }
    double dirQuantizeStepLeft() const { return M_dir_quantize_step_l; }
    double dirQuantizeStepRight() const { return M_dir_quantize_step_r; }

    bool coachMode() const { return M_coach_mode; }
    bool coachWithRefereeMode() const { return M_coach_with_referee_mode; }
    bool useOldCoachHear() const { return M_use_old_coach_hear; }

    double slownessOnTopForLeft() const { return M_slowness_on_top_for_left_team; }
    double slownessOnTopForRight() const { return M_slowness_on_top_for_right_team; }

    int startGoalLeft() const { return M_start_goal_l; }
    int stargGoalRight() const { return M_start_goal_r; }

    bool fullstateLeft() const { return M_fullstate_l; }
    bool fullstateRight() const { return M_fullstate_r; }

    int dropBallTime() const { return M_drop_ball_time; }

    bool synchMode() const { return M_synch_mode; }
    int synchOffset() const { return M_synch_offset; }
    int synchMicroSleep() const { return M_synch_micro_sleep; }

    int pointToBan() const { return M_point_to_ban; }
    int pointToDuration() const { return M_point_to_duration; }


    // not defined in server_param_t

    int playerPort() const { return M_player_port; }
    int trainerPort() const { return M_trainer_port; }
    int onlineCoachPort() const { return M_online_coach_port; }

    bool verboseMode() const { return M_verbose_mode; }

    int coachSendVIStep() const { return M_coach_send_vi_step; }

    const std::string & replayFile() const { return M_replay_file; }
    const std::string & landmarkFile() const { return M_landmark_file; }

    bool sendComms() const { return M_send_comms; }

    bool textLogging() const { return  M_text_logging; }
    bool gameLogging() const { return M_game_logging; }
    int gameLogVersion() const { return M_game_log_version; }
    const std::string & textLogDir() const { return M_text_log_dir; }
    const std::string & gameLogDir() const { return M_game_log_dir; }
    const std::string & textLogFixedName() const { return M_text_log_fixed_name; }
    const std::string & gameLogFixedName() const { return M_game_log_fixed_name; }
    bool textLogFixed() const { return M_use_text_log_fixed; }
    bool gameLogFixed() const { return M_use_game_log_fixed; }
    bool textLogDated() const { return M_use_text_log_dated; }
    bool gameLogDated() const { return M_use_game_log_dated; }
    const std::string & logDateFormat() const { return M_log_date_format; }
    bool logTimes() const { return M_log_times; }
    bool recordMessage() const { return M_record_message; }
    int textLogCompression() const { return M_text_log_compression; }
    int gameLogCompression() const { return M_game_log_compression; }

    bool useProfile() const { return M_use_profile; }

    double tackleDist() const { return M_tackle_dist; }
    double tackleBackDist() const { return M_tackle_back_dist; }
    double tackleWidth() const { return M_tackle_width; }
    double tackleExponent() const { return M_tackle_exponent; }
    int tackleCycles() const { return M_tackle_cycles; }
    double tacklePowerRate() const { return M_tackle_power_rate; }

    int freeformWaitPeriod() const { return M_freeform_wait_period; }
    int freeformSendPeriod() const { return M_freeform_send_period; }

    bool freeKickFaults() const { return M_free_kick_faults; }
    bool backPasses() const { return M_back_passes; }

    bool properGoalKicks() const { return M_proper_goal_kicks; }
    double stoppedBallVel() const { return M_stopped_ball_vel; }
    int maxGoalKicks() const { return M_max_goal_kicks; }

    int clangDelWin() const { return M_clang_del_win; }
    int clangRuleWin() const { return M_clang_rule_win; }

    bool autoMode() const { return M_auto_mode; }
    int kickOffWait() const { return M_kick_off_wait; }
    int connectWait() const { return M_connect_wait; }
    int gameOverWait() const { return M_game_over_wait; }
    const std::string & teamLeftStartCommand() const { return M_team_l_start; }
    const std::string & teamRightStartCommand() const { return M_team_r_start; }

    bool keepawayMode() const { return M_keepaway_mode; }
    double keepawayLength() const { return M_keepaway_length; }
    double keepawayWidth() const { return M_keepaway_width; }

    bool keepawayLogging() const { return M_keepaway_logging; }
    const std::string & keepawayLogDir() const { return M_keepaway_log_dir; }
    const std::string & keepawayLogFixedName() const { return M_keepaway_log_fixed_name; }
    bool keepawayLogFixed() const { return M_keepaway_log_fixed; }
    bool keepawayLogDated() const { return M_keepaway_log_dated; }

    int keepawayStart() const { return M_keepaway_start; }

    int nrNormalHalfs() const { return M_nr_normal_halfs; }
    int nrExtraHalfs() const { return M_nr_extra_halfs; }
    bool penaltyShootOuts() const { return  M_penalty_shoot_outs; }

    int penBeforeSetupWait() const { return M_pen_before_setup_wait; }
    int penSetupWait() const { return M_pen_setup_wait; }
    int penReadyWait() const { return M_pen_ready_wait; }
    int penTakenWait() const { return M_pen_taken_wait; }
    int penNrKicks() const { return M_pen_nr_kicks; }
    int penMaxExtraKicks() const { return M_pen_max_extra_kicks; }
    double penDistX() const { return M_pen_dist_x; }
    bool penRandomWinner() const { return M_pen_random_winner; }
    bool penAllowMultKicks() const { return M_pen_allow_mult_kicks; }
    double penMaxGoalieDistX() const { return M_pen_max_goalie_dist_x; }
    bool penCoachMovesPlayers() const { return M_pen_coach_moves_players; }

    const std::string & moduleDir() const { return M_module_dir; }

    // 11.0.0
    double ballStuckArea() const { return M_ball_stuck_area; }
    const std::string & coachMsgFile() const { return M_coach_msg_file; }

    // 12.0.0
    double maxTacklePower() const { return M_max_tackle_power; }
    double maxBackTacklePower() const { return M_max_back_tackle_power; }
    double playerSpeedMaxMin() const { return M_player_speed_max_min; }
    double defaultExtraStamina() const { return M_extra_stamina; }

    int synchSeeOffset() const { return M_synch_see_offset; }
    int maxMonitors() const { return M_max_monitors; }

    // v12.1.3
    int extraHalfTime() const { return M_extra_half_time; }

    // v13
    double staminaCapacity() const { return M_stamina_capacity; }
    double maxDashAngle() const { return M_max_dash_angle; }
    double minDashAngle() const { return M_min_dash_angle; }
    double dashAngleStep() const { return M_dash_angle_step; }
    double sideDashRate() const { return M_side_dash_rate; }
    double backDashRate() const { return M_back_dash_rate; }
    double maxDashPower() const { return M_max_dash_power; }
    double minDashPower() const { return M_min_dash_power; }

    // v14
    double tackleRandFactor() const { return M_tackle_rand_factor; }
    double foulDetectProbability() const { return M_foul_detect_probability; }
    double foulExponent() const { return M_foul_exponent; }
    int foulCycles() const { return M_foul_cycles; }
    bool goldenGoal() const { return M_golden_goal; }

    // v15
    double redCardProbability() const { return M_red_card_probability; }

    // v16
    bool useIllegalDefense() const { return M_illegal_defense_number != 0; }
    int illegalDefenseDuration() const { return M_illegal_defense_duration; }
    int illegalDefenseNumber() const { return M_illegal_defense_number; }
    double illegalDefenseDistX() const { return M_illegal_defense_dist_x; }
    double illegalDefenseWidth() const { return M_illegal_defense_width; }
    const std::string & fixedTeamNameLeft() const { return M_fixed_teamname_l; }
    const std::string & fixedTeamNameRight() const { return M_fixed_teamname_r; }

    // v17
    double maxCatchAngle() const { return M_max_catch_angle; }
    double minCatchAngle() const { return M_min_catch_angle; }

    // XXX
    int randomSeed() const { return M_random_seed; }
    double longKickPowerFactor() const { return M_long_kick_power_factor; }
    int longKickDelay() const { return M_long_kick_delay; }


    // automatically defined values

    // double halfTimeScaler() const
    //   {
    //       double value = 1000.0 / ( M_simulator_step / M_slow_down_factor );
    //       return ( value != 0.0
    //                ? value
    //                : 1.0e-10 );
    //   }

    int actualHalfTime() const
      {
          return M_half_time * 10;
      }

    int actualExtraHalfTime() const
      {
          return M_extra_half_time * 10;
      }

    double pitchHalfLength() const
      {
          return pitchLength() * 0.5;
      }

    double pitchHalfWidth() const
      {
          return pitchWidth() * 0.5;
      }

    double penaltyAreaHalfWidth() const
      {
          return penaltyAreaWidth() * 0.5;
      }

    double goalAreaHalfWidth() const
      {
          return goalAreaWidth() * 0.5;
      }

    double goalHalfWidth() const
      {
          return goalWidth() * 0.5;
      }

    double ourTeamGoalLineX() const
      {
          return - pitchHalfLength();
      }

    double theirTeamGoalLineX() const
      {
          return + pitchHalfLength();
      }

    Vector2D ourTeamGoalPos() const
      {
          return Vector2D( - pitchHalfLength(), 0.0 );
      }

    Vector2D theirTeamGoalPos() const
      {
          return Vector2D( + pitchHalfLength(), 0.0 );
      }

    double ourPenaltyAreaLineX() const
      {
          return -pitchHalfLength() + penaltyAreaLength();
      }

    double theirPenaltyAreaLineX() const
      {
          return pitchHalfLength() - penaltyAreaLength();
      }


    Rect2D ourPenaltyArea() const
      {
          return Rect2D::from_corners( -pitchHalfLength(),
                                       -penaltyAreaHalfWidth(),
                                       -pitchHalfLength() + penaltyAreaLength(),
                                       +penaltyAreaHalfWidth() );
      }

    Rect2D theirPenaltyArea() const
      {
          return Rect2D::from_corners( pitchHalfLength() - penaltyAreaLength(),
                                       -penaltyAreaHalfWidth(),
                                       +pitchHalfLength(),
                                       +penaltyAreaHalfWidth() );
      }

    Rect2D ourGoalArea() const
      {
          return Rect2D::from_corners( -pitchHalfLength(),
                                       -goalAreaHalfWidth(),
                                       -pitchHalfLength() + goalAreaLength(),
                                       +goalAreaHalfWidth() );
      }

    Rect2D theirGoalArea() const
      {
          return Rect2D::from_corners( +pitchHalfLength() - goalAreaLength(),
                                       -goalAreaHalfWidth(),
                                       +pitchHalfLength(),
                                       +goalAreaHalfWidth() );
      }

    double defaultRealSpeedMax() const
      {
          return M_real_speed_max;
      }

    double recoverDecThrValue() const
      {
          return recoverDecThr() * staminaMax();
      }

    double effortDecThrValue() const
      {
          return effortDecThr() * staminaMax();
      }

    double effortIncThrValue() const
      {
          return effortIncThr() * staminaMax();
      }

    // additional params
    double catchableArea() const
      {
          return M_catchable_area;
      }

    // utility

    /*!
      \brief normalize power range
      \param power command argument power
      \return normalized power
     */
    double normalizePower( const double & power ) const
      {
          if ( power < minPower() ) return  minPower();
          else if ( power > maxPower() ) return maxPower();
          return power;
      }

    /*!
      \brief normalize dash power
      \param power dash power value
      \return normalized dash power value
     */
    double normalizeDashPower( const double & power ) const
      {
          if ( power < minDashPower() ) return  minDashPower();
          else if ( power > maxDashPower() ) return maxDashPower();
          return power;
      }

    /*!
      \brief normalize dash direction
      \param dir dash direction
      \return normalized dash direction
     */
    double normalizeDashAngle( const double & dir ) const
      {
          if ( dir < minDashAngle() ) return  minDashAngle();
          else if ( dir > maxDashAngle() ) return maxDashAngle();
          return dir;
      }

    /*!
      \brief discretize dash direction
      \param dir dash direction
      \return discretized dash direction
     */
    double discretizeDashAngle( const double & dir ) const
      {
          double d = normalizeDashAngle( dir );
          return ( dashAngleStep() < 1.0e-10 // SERVER_EPS
                   ? d
                   : dashAngleStep() * rint( d / dashAngleStep() ) );
      }

    /*!
      \brief calculate effectiveness for the dash direction.
      \param dir dash direction
      \return dash power effectiveness for dir
     */
    double dashDirRate( const double & dir ) const;

    /*!
      \brief normalize turn moment range
      \param moment command argument moment
      \return normalized moment
     */
    double normalizeMoment( const double & moment ) const
      {
          if ( moment < minMoment() ) return minMoment();
          else if ( moment > maxMoment() ) return maxMoment();
          return moment;
      }

    /*!
      \brief normalize the catch direction within [min_catch_angle, max_catch_angle]
      \param dir input direction value (degree)
      \return normalized direction value (degree)
     */
    double normalizeCatchAngle( const double dir ) const
    {
        return ( dir < minCatchAngle() ? minCatchAngle()
                 : maxCatchAngle() < dir ? maxCatchAngle()
                 : dir );
    }

    /*!
      \brief normalize neck moment
      \param moment command argument moment
      \return normalized moment
     */
    double normalizeNeckMoment( const double & moment ) const
      {
          if ( moment < minNeckMoment() ) return minNeckMoment();
          else if ( moment > maxNeckMoment() ) return maxNeckMoment();
          return moment;
      }
    /*!
      \brief normalize neck angle range
      \param neck_angle neck angle
      \return normalized neck angle
     */
    double normalizeNeckAngle( const double & neck_angle ) const
      {
          if ( neck_angle < minNeckAngle() ) return minNeckAngle();
          else if ( neck_angle > maxNeckAngle() ) return maxNeckAngle();
          return neck_angle;
      }


    /*!
      \brief calculate ball moves step for the input first speed & move distance
      \param first_ball_speed ball first speed. have to be a positive value.
      \param ball_move_dist total bal move distance. have to be a positive value.
      \return numerically calculated result(by ceiled integer)
     */
    int ballMoveStep( const double & first_ball_speed,
                      const double & ball_move_dist ) const
      {
          return static_cast< int >( std::ceil( calc_length_geom_series( first_ball_speed,
                                                                         ball_move_dist,
                                                                         ballDecay() ) )
                                     + 1.0e-10 );
      }

    /*!
      \brief calculate the first ball speed for the input move distance & steps
      \param ball_move_dist total ball move distance, have to be a positive value.
      \param total_step desired ball move steps. have to be greater than 1.
      \return numerically calculated result. the value may be greater than ballSpeedMax()
     */
    double firstBallSpeed( const double & ball_move_dist,
                           const int total_step ) const
      {
          return calc_first_term_geom_series( ball_move_dist,
                                              ballDecay(),
                                              total_step );
      }
};

}

#endif
