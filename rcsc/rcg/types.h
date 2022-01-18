// -*-c++-*-

/*!
  \file rcsc/rcg/types.h
  \brief rcg data format types.
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

#ifndef RCSC_RCG_TYPES_H
#define RCSC_RCG_TYPES_H

#include <rcsc/types.h>

#include <boost/cstdint.hpp>

#include <string>
#include <cmath>

namespace rcsc {
namespace rcg {


//! type of the 16bits integer value
typedef boost::int16_t Int16;
//! type of the unsigned 16bits integer value
typedef boost::uint16_t UInt16;
//! type of the 32bits integer value
typedef boost::int32_t Int32;
//! type of the unsigned 32bits integer value
typedef boost::int32_t UInt32;

/*!
  \brief max length of color name string.

  Actually, this variable is not used.
*/
enum {
    COLOR_NAME_MAX = 64
};

/*!
  \enum DispInfoMode
  \brief monitor data type enumeration.
 */
enum DispInfoMode {
    NO_INFO     = 0, //!< dummy type
    SHOW_MODE   = 1, //!< showinfo_t
    MSG_MODE    = 2, //!< msg info
    DRAW_MODE   = 3, //!< drawinfo_t
    BLANK_MODE  = 4, //!< dummy. until sserver-6.07
    PM_MODE     = 5, //!< playmode
    TEAM_MODE   = 6, //!< team_t (team name & score)
    PT_MODE     = 7, //!< player_type_t
    PARAM_MODE  = 8, //!< server_params_t
    PPARAM_MODE = 9  //!< player_params_t
};

/*!
  \enum MsgInfoMode
  \brief message data type id enumeration.
 */
enum MsgInfoMode {
    MSG_BOARD = 1,
    LOG_BOARD = 2
};

/*!
  \enum PlayerStatus
  \brief player status bit mask.
 */
enum PlayerStatus {
    DISABLE         = 0x00000000,
    STAND           = 0x00000001,
    KICK            = 0x00000002,
    KICK_FAULT      = 0x00000004,
    GOALIE          = 0x00000008,
    CATCH           = 0x00000010,
    CATCH_FAULT     = 0x00000020,
    BALL_TO_PLAYER  = 0x00000040,
    PLAYER_TO_BALL  = 0x00000080,
    DISCARD         = 0x00000100,
    LOST            = 0x00000200,
    BALL_COLLIDE    = 0x00000400,
    PLAYER_COLLIDE  = 0x00000800,
    TACKLE          = 0x00001000,
    TACKLE_FAULT    = 0x00002000,
    BACK_PASS       = 0x00004000,
    FREE_KICK_FAULT = 0x00008000,
    POST_COLLIDE    = 0x00010000, // player collided with goal posts
    FOUL_CHARGED     = 0x00020000, // player is frozen by intentional tackle foul
    YELLOW_CARD     = 0x00040000,
    RED_CARD        = 0x00080000,
    ILLEGAL_DEFENSE = 0x00100000,
};


/*-------------------------------------------------------------------*/
// monitor protocol version 1
// game log format versin 1 & version 2

const double SHOWINFO_SCALE = 16.0; //!< scaling factor
const float SHOWINFO_SCALEF = 16.0f; //!< scaling factor
const int REC_OLD_VERSION = 1; //!< version number of rcg v1
const int REC_VERSION_2 = 2; //!< recorded value of rcg v2

/*!
  \struct pos_t
  \brief object data
*/
struct pos_t {
    Int16 enable; //!< determines if this object is in field.
    Int16 side;   //!< side type of this object
    Int16 unum;   //!< uniform number
    Int16 angle;  //!< player's body angle. This is degree data.
    Int16 x;      //!< x-coordinate
    Int16 y;      //!< y-coordinate
};

/*!
  \struct team_t
  \brief team data
*/
struct team_t {
    char name[16]; //!< team name string
    Int16 score;   //!< score
};

/*!
  \struct showinfo_t
  \brief view data of rcssmonitor v1 protocol
*/
struct showinfo_t {
    char pmode;     //!< playmode id
    team_t team[2]; //!< team date
    pos_t pos[MAX_PLAYER * 2 + 1]; //!< all movable objects
    Int16 time; //!< game time
};

/*!
  \struct msginfo_t
  \brief message data
*/
struct msginfo_t {
    Int16 board;  //!< message type id
    char message[2048]; //!< message body
};

/*!
  \struct pointinfo_t
  \brief point data in drawinfo_t
*/
struct pointinfo_t {
    Int16 x; //!< x pos
    Int16 y; //!< y pos
    char color[COLOR_NAME_MAX]; //!< color name
};

/*!
  \struct circleinfo_t
  \brief circle data in drawinfo_t
*/
struct circleinfo_t {
    Int16 x; //!< x pos
    Int16 y; //!< y pos
    Int16 r; //!< radius
    char color[COLOR_NAME_MAX]; //!< color name
};

/*!
  \struct lineinfo_t
  \brief line data in drawinfo_t
*/
struct lineinfo_t {
    Int16 x1; //!< x of point1
    Int16 y1; //!< y of point1
    Int16 x2; //!< x of point2
    Int16 y2; //!< y of point2
    char color[COLOR_NAME_MAX]; //!< color name
};

/*!
  \struct drawinfo_t
  \brief drawing data
*/
struct drawinfo_t {
    Int16 mode; //!< data type id
    union {
        pointinfo_t pinfo;
        circleinfo_t cinfo;
        lineinfo_t linfo;
    } object; //!< union variable
};

/*!
  \struct dispinfo_t
  \brief data block for rcssmonitor v1 protocol and rcg v1/v2
*/
struct dispinfo_t {
    Int16 mode; //!< data type id
    union {
        showinfo_t show; //!< view data
        msginfo_t msg; //!< message data
        drawinfo_t draw; //!< draw data
    } body; //!< union variable
};

/*-------------------------------------------------------------------*/
// monitor protocol version 2
// game log format version 3

//! scaling variable
const double SHOWINFO_SCALE2 = 65536.0;
//! scaling variable
const float SHOWINFO_SCALE2F = 65536.0f;
//! version number
const int REC_VERSION_3 = 3;

/*!
  \struct ball_t
  \brief ball data
*/
struct ball_t {
    Int32 x; //!< pos x scaled by SHOWINFO_SCALE2
    Int32 y; //!< pos y scaled by SHOWINFO_SCALE2
    Int32 deltax; //!< velocity x scaled by SHOWINFO_SCALE2
    Int32 deltay; //!< velocity y scaled by SHOWINFO_SCALE2
};

/*!
  \struct player_t
  \brief player data
*/
struct player_t {
    Int16 mode; //!< status flag
    Int16 type; //!< player type id
    Int32 x; //!< scaled pos x
    Int32 y; //!< scaled pos y
    Int32 deltax; //!< scaled velocity x
    Int32 deltay; //!< scaled velocity y
    Int32 body_angle; //!< scaled body angle. radian data
    Int32 head_angle; //!< scaled heading angle relative to body. radian data
    Int32 view_width; //!< scaled view width. radian data
    Int16 view_quality; //!< view quality id
    Int32 stamina; //!< scaled stamina value.
    Int32 effort; //!< scaled effort value
    Int32 recovery; //!< scaled recover value
    Int16 kick_count; //!< executed kick command count
    Int16 dash_count; //!< executed dash command count
    Int16 turn_count; //!< executed turn command count
    Int16 say_count; //!< executed say command count
    Int16 turn_neck_count; //!< executed turn_neck command count
    Int16 catch_count; //!< executed catch command count
    Int16 move_count; //!< executed move command count
    Int16 change_view_count; //!< executed change_view command count
};

/*!
  \struct showinfo_t2
  \brief view data of rcssmonitor v2 protocol
*/
struct showinfo_t2 {
    char pmode; //!< playmode id
    team_t team[2]; //!< team info
    ball_t ball; //!< ball info
    player_t pos[MAX_PLAYER * 2]; //!< all player info
    Int16 time; //!< game time
};

/*!
  \struct short_showinfo_t2
  \brief view data.

  rcg v3 files include only this as view data.
*/
struct short_showinfo_t2 {
    ball_t ball; //!< ball info
    player_t pos[MAX_PLAYER * 2]; //!< player info
    Int16 time; //!< game time
};

/*!
  \struct player_type_t
  \brief player type parameters
 */
struct player_type_t {
    Int16 id; //!< type id
    Int32 player_speed_max; //!< max speed
    Int32 stamina_inc_max; //!< max stamina increment value
    Int32 player_decay; //!< speed decay
    Int32 inertia_moment; //!< inertia moment
    Int32 dash_power_rate; //!< dash power rate
    Int32 player_size; //!< body radius
    Int32 kickable_margin; //!< kickable margin
    Int32 kick_rand; //!< random factor for kick
    Int32 extra_stamina; //!< extra stamina value when stamina is 0.
    Int32 effort_max; //!< max(initial) effort value
    Int32 effort_min; //!< min effort value

    Int32 kick_power_rate; //!< kick power rate
    Int32 foul_detect_probability; //!< foul detect probability
    Int32 catchable_area_l_stretch; //!< catch area length stretch factor

    //Int32 sparelong1;  replaced by kick_power_rate
    //Int32 sparelong2;  replaced by foul_detect_probability
    //Int32 sparelong3;  replaced by catchable_area_l_stretch
    Int32 sparelong4; //!< spare variable
    Int32 sparelong5; //!< spare variable
    Int32 sparelong6; //!< spare variable
    Int32 sparelong7; //!< spare variable
    Int32 sparelong8; //!< spare variable
    Int32 sparelong9; //!< spare variable
    Int32 sparelong10; //!< spare variable
};

/*!
  \struct server_params_t
  \brief server parametors
 */
struct server_params_t {
    Int32 goal_width; //!< goal width
    Int32 inertia_moment; //!< related to player's turn action
    Int32 player_size; //!< normal player size
    Int32 player_decay; //!< normal player speed decay
    Int32 player_rand; //!< noize added to normal player movement
    Int32 player_weight; //!< normal player weight
    Int32 player_speed_max; //!< normal player speed max
    Int32 player_accel_max; //!< normal player accel max
    Int32 stamina_max; //!< normal player stamina max
    Int32 stamina_inc; //!< normal player stamina inc
    Int32 recover_init; //!< normal player recovery init
    Int32 recover_dec_thr; //!< normal player recovery decriment threshold
    Int32 recover_min; //!< normal player recovery min
    Int32 recover_dec; //!< normal player recovery decriment
    Int32 effort_init; //!< normal player dash effort init
    Int32 effort_dec_thr; //!< normal player dash effort decriment threshold
    Int32 effort_min; //!< normal player dash effrot min
    Int32 effort_dec; //!< normal player dash effort decriment
    Int32 effort_inc_thr; //!< normal player dash effort incriment threshold
    Int32 effort_inc; //!< normal player dash effort incriment
    Int32 kick_rand;  //!< noise added directly to normal player's kick
    Int16 team_actuator_noise; //!< flag whether to use team specific actuator noise
    Int32 player_rand_factor_l; //!< factor to multiple prand for left team
    Int32 player_rand_factor_r; //!< factor to multiple prand for right team
    Int32 kick_rand_factor_l; //!< factor to multiple kick_rand for left team
    Int32 kick_rand_factor_r; //!< factor to multiple kick_rand for right team
    Int32 ball_size; //!< ball size
    Int32 ball_decay; //!< ball speed decay
    Int32 ball_rand; //!< noise added to ball movement
    Int32 ball_weight; //!< ball weight
    Int32 ball_speed_max; //!< ball speed max
    Int32 ball_accel_max; //!< ball acceleration max
    Int32 dash_power_rate; //!< normal player's dash power rate
    Int32 kick_power_rate; //!< normal player's kick power rate
    Int32 kickable_margin; //!< normal player's kickable margin
    Int32 control_radius; //!< control radius
    Int32 control_radius_width; //!< (control radius) - (plyaer size)
    Int32 max_power; //!< max power
    Int32 min_power; //!< min power
    Int32 max_moment; //!< max moment
    Int32 min_moment; //!< min moment
    Int32 max_neck_moment; //!< max neck moment
    Int32 min_neck_moment; //!< min neck moment
    Int32 max_neck_angle; //!< max neck angle
    Int32 min_neck_angle; //!< min neck angle
    Int32 visible_angle; //!< visible angle
    Int32 visible_distance; //!< visible distance
    Int32 wind_dir; //!< wind direction
    Int32 wind_force; //!< wind force
    Int32 wind_ang; //!< wind angle for rand
    Int32 wind_rand; //!< wind noise for force
    Int32 kickable_area; //!< kickable_area
    Int32 catch_area_l; //!< goalie catchable area length
    Int32 catch_area_w; //!< goalie catchable area width
    Int32 catch_probability; //!< goalie catchable possibility
    Int16 goalie_max_moves; //!< goalie max moves after a catch
    Int32 corner_kick_margin; //!< corner kick margin
    Int32 offside_active_area; //!< offside active area size
    Int16 wind_none; //!< wind factor is none
    Int16 use_wind_random; //!< wind factor is random
    Int16 coach_say_count_max; //!< max count of coach's freeform say
    Int16 coach_say_msg_size; //!< max length of coach's freeform say message
    Int16 clang_win_size; //!< window of coach language window
    Int16 clang_define_win; //!< window of coach language define message
    Int16 clang_meta_win; //!< window of coach lauguage meta message
    Int16 clang_advice_win; //!< window of coach language advice message
    Int16 clang_info_win; //!< window of coach language info message
    Int16 clang_mess_delay; //!< coach language advise message is delayed this cycle
    Int16 clang_mess_per_cycle; //!< online coach can send CLang advise per this cycle
    Int16 half_time; //!<  half time cycle
    Int16 simulator_step; //!< simulator step interval msec
    Int16 send_step; //!< udp send step interval msec
    Int16 recv_step; //!< udp recv step interval msec
    Int16 sense_body_step; //!< sense_body interval step msec
    Int16 lcm_step; //!< lcm of all the above steps msec
    Int16 player_say_msg_size; //!< string size of say message
    Int16 player_hear_max; //!< player hear_capacity_max
    Int16 player_hear_inc; //!< player hear_capacity_inc
    Int16 player_hear_decay; //!< player hear_capacity_decay
    Int16 catch_ban_cycle; //!< goalie catch ban cycle
    Int16 slow_down_factor; //!< factor to slow down simulator and send intervals
    Int16 use_offside; //!< flag for using off side rule
    Int16 kickoff_offside; //!< flag for permit kick off offside
    Int32 offside_kick_margin; //!< offside kick margin
    Int32 audio_cut_dist; //!< audio cut off distance
    Int32 dist_quantize_step; //!< quantize step of distance
    Int32 landmark_dist_quantize_step; //!< quantize step of distance for landmark
    Int32 dir_quantize_step; //!< quantize step of direction
    Int32 dist_quantize_step_l; //!< team right quantize step of distance
    Int32 dist_quantize_step_r; //!< team left quantize step of distance
    Int32 landmark_dist_quantize_step_l; //!< team right quantize step of distance for landmark
    Int32 landmark_dist_quantize_step_r; //!< team left quantize step of distance for landmark
    Int32 dir_quantize_step_l; //!< team left quantize step of direction
    Int32 dir_quantize_step_r; //!< team right quantize step of direction
    Int16 coach_mode; //!< coach mode
    Int16 coach_with_referee_mode; //!< coach with referee mode
    Int16 use_old_coach_hear; //!< old format for hear command (coach)
    Int16 online_coach_look_step; //!< online coach's look interval step
    Int32 slowness_on_top_for_left_team;  //!< flag for left team dash power rate reduction on field top area
    Int32 slowness_on_top_for_right_team; //!< flag for right team dash power rate reduction on field top area

    Int32 ka_length; //!< keep away region length
    Int32 ka_width;  //!< keep away region width

    Int32 ball_stuck_area; //!< automatic drop ball threshold while play_on

    Int32 max_tackle_power; //!< allowed maximum tackle power
    Int32 max_back_tackle_power; //!< allowed minimum tackle power

    Int32 tackle_dist; //!< forward tackle area length
    Int32 tackle_back_dist; //!< back tackle area length
    Int32 tackle_width; //!< tackle area width

    Int16 start_goal_l; //!< initial left team score
    Int16 start_goal_r; //!< initial right team score
    Int16 fullstate_l; //!< flag for left team's fullstate info receive
    Int16 fullstate_r; //!< flag for right team's fullstate info receive
    Int16 drop_ball_time; //!< automatically drop_ball waiting cycle
    Int16 synch_mode; //!< flag for using synchronized mode
    Int16 synch_offset; //!< wheh sync_mode, server decide the message sent to clients before this number[ms]
    Int16 synch_micro_sleep; //!< the number of microseconds to sleep while waiting for players */
    Int16 point_to_ban; //!< player continue to point to the same point at least during this cycle
    Int16 point_to_duration; //!< point_to command has effectiveness during this cycle

    /*
      Int16 pen_before_setup_wait;
      Int16 pen_setup_wait;
      Int16 pen_ready_wait;
      Int16 pen_taken_wait;
      Int16 pen_nr_kicks;
      Int16 pen_max_extra_kicks;
      Int32 pen_dist_x;
      Int16 pen_random_winner;
      Int32 pen_max_goalie_dist_x;
      Int16 pen_allow_mult_kicks;
    */
};

/*!
  \struct player_params_t
  \brief heterogenious player trade-off parametors
*/
struct player_params_t {
    Int16 player_types; //!< the number of player types
    Int16 substitute_max; //!< max player substitution
    Int16 pt_max; //!< max each player type on field
    Int32 player_speed_max_delta_min; //!< minimal value wighin delta range
    Int32 player_speed_max_delta_max; //!< maximal value wighin delta range
    Int32 stamina_inc_max_delta_factor; //!< tradeoff parameter
    Int32 player_decay_delta_min; //!< minimal value wighin delta range
    Int32 player_decay_delta_max; //!< maximal value wighin delta range
    Int32 inertia_moment_delta_factor; //!< tradeoff parameter
    Int32 dash_power_rate_delta_min; //!< minimal value wighin delta range
    Int32 dash_power_rate_delta_max; //!< maximal value wighin delta range
    Int32 player_size_delta_factor; //!< tradeoff parameter
    Int32 kickable_margin_delta_min; //!< minimal value wighin delta range
    Int32 kickable_margin_delta_max; //!< maximal value wighin delta range
    Int32 kick_rand_delta_factor; //!< tradeoff parameter
    Int32 extra_stamina_delta_min; //!< minimal value wighin delta range
    Int32 extra_stamina_delta_max; //!< maximal value wighin delta range
    Int32 effort_max_delta_factor; //!< tradeoff parameter
    Int32 effort_min_delta_factor; //!< tradeoff parameter
    Int32 random_seed; //!< random seed to generate parameters
    Int32 new_dash_power_rate_delta_min; //!< minimal value wighin delta range
    Int32 new_dash_power_rate_delta_max; //!< maximal value wighin delta range
    Int32 new_stamina_inc_max_delta_factor; //!< tradeoff parameter

    Int32 kick_power_rate_delta_min;
    Int32 kick_power_rate_delta_max;
    Int32 foul_detect_probability_delta_factor;

    Int32 catchable_area_l_stretch_min;
    Int32 catchable_area_l_stretch_max;

    //Int32 sparelong5;  replaced by kick_power_rate_delta_min
    //Int32 sparelong6;  replaced by kick_power_rate_delta_max
    //Int32 sparelong7;  replaced by foul_detect_probability_delta_factor
    //Int32 sparelong8;  replaced by catchable_area_l_stretch_min
    //Int32 sparelong9;  replaced by catchable_area_l_stretch_max
    Int32 sparelong10; //!< spare variable

    Int16 allow_mult_default_type; //!< whether multiple default type is allowed

    Int16 spareshort2; //!< spare variable
    Int16 spareshort3; //!< spare variable
    Int16 spareshort4; //!< spare variable
    Int16 spareshort5; //!< spare variable
    Int16 spareshort6; //!< spare variable
    Int16 spareshort7; //!< spare variable
    Int16 spareshort8; //!< spare variable
    Int16 spareshort9; //!< spare variable
    Int16 spareshort10; //!< spare variable
};

/*!
  \struct dispinfo_t2
  \brief data block for rcssmonitor v2 protocol.

  This data format is used only for monitor protocol and not used in rcg.
*/
struct dispinfo_t2 {
    Int16 mode; //!< data type id
    union {
        showinfo_t2 show; //!< view data
        msginfo_t msg; //!< message data
        player_type_t ptinfo; //!< player type data
        server_params_t sparams; //!< server param data
        player_params_t pparams; //!< payer trade-off param data
    } body; //!< union variable
};

//
// Data structures for the text based monitor protocl
//

/*!
  \struct BallT
  \brief generic ball data for display information
 */
struct BallT {
    float x_; //!< ball position x
    float y_; //!< ball position y
    float vx_; //!< ball velocity x
    float vy_; //!< ball velocity y

    /*!
      \brief initialize all variables by 0
     */
    BallT()
        : x_( 0.0 ),
          y_( 0.0 ),
          vx_( SHOWINFO_SCALE2F ),
          vy_( SHOWINFO_SCALE2F )
      { }

    /*!
      \brief check if this object has velocity info
      \return true if this object has velocity info
     */
    bool hasVelocity() const
      {
          return vx_ != SHOWINFO_SCALE2F;
          //&& vy_ != SHOWINFO_SCALE2F;
      }

    double x() const { return x_; }
    double y() const { return y_; }
    double deltaX() const { return vx_; }
    double deltaY() const { return vy_; }
};

/*!
  \struct PlayerT
  \brief generic player data for display information
 */
struct PlayerT {
    char side_; //!< player's side. 'l', 'r' or 'n'
    Int16 unum_; //!< player's uniform number. 0 means disabled player.
    Int16 type_; //!< heterogeneous player type id

    char view_quality_; //!< view quality indicator, 'l' or 'h'

    char focus_side_; //!< focus target side. 'l', 'r' or 'n'. 'n' means no focus target.
    Int16 focus_unum_; //!< focus target uniform number. 0 means no focus target.

    Int32 state_; //!< state bit flags

    float x_; //!< position x
    float y_; //!< position y
    float vx_; //!< velocity x
    float vy_; //!< velocity y
    float body_; //!< body direction (degree)
    float neck_; //!< head direction relative to body (degree)
    float point_x_; //!< arm pointing x
    float point_y_; //!< arm pointing y

    float view_width_; //!< view width (degree). high: value>0, low: value<0

    float stamina_; //!< satamina value
    float effort_; //!< effort value
    float recovery_; //!< recovery value
    float stamina_capacity_; //!< stamina capacity value

    UInt16 kick_count_; //!< kick command count
    UInt16 dash_count_; //!< dash command count
    UInt16 turn_count_; //!< turn command count
    UInt16 catch_count_; //!< catch command count
    UInt16 move_count_; //!< move command count
    UInt16 turn_neck_count_; //!< turn_neck command count
    UInt16 change_view_count_; //!< change_view command count
    UInt16 say_count_; //!< say command count
    UInt16 tackle_count_; //!< tackle command count
    UInt16 pointto_count_; //!< pointto command count
    UInt16 attentionto_count_; //!< attentionto command count

    /*!
      \brief initialize all variables
     */
    PlayerT()
        : side_( 'n' ),
          unum_( 0 ),
          type_( -1 ),
          view_quality_( 'h' ),
          focus_side_( 'n' ),
          focus_unum_( 0 ),
          state_( 0 ),
          x_( 0.0f ),
          y_( 0.0f ),
          vx_( SHOWINFO_SCALE2F ),
          vy_( SHOWINFO_SCALE2F ),
          body_( 0.0f ),
          neck_( SHOWINFO_SCALE2F ),
          point_x_( SHOWINFO_SCALE2F ),
          point_y_( SHOWINFO_SCALE2F ),
          view_width_( SHOWINFO_SCALE2F ),
          stamina_( SHOWINFO_SCALE2F ),
          effort_( SHOWINFO_SCALE2F ),
          recovery_( SHOWINFO_SCALE2F ),
          stamina_capacity_( -1.0f ),
          kick_count_( 0xFFFF ),
          dash_count_( 0xFFFF ),
          turn_count_( 0xFFFF ),
          catch_count_( 0xFFFF ),
          move_count_( 0xFFFF ),
          turn_neck_count_( 0xFFFF ),
          change_view_count_( 0xFFFF ),
          say_count_( 0xFFFF ),
          tackle_count_( 0xFFFF ),
          pointto_count_( 0xFFFF ),
          attentionto_count_( 0xFFFF )
      { }

    /*!
      \brief get side id
      \return side id
     */
    SideID side() const
      {
          return ( side_ == 'l' ? LEFT
                   : side_ == 'r' ? RIGHT
                   : NEUTRAL );
      }

    /*!
      \brief get uniform number
      \return uniform number
     */
    int unum() const
      {
          return unum_;
      }

    /*!
      \brief get heterogeneous player type id
      \return id number
     */
    int type() const
      {
          return type_;
      }

    //
    // information level
    //

    /*!
      \brief check if this object has player type info
      \return checked result
     */
    bool hasType() const
      {
          return type_ >= 0;
      }

    /*!
      \brief check if this object has velocity info
      \return checked result
     */
    bool hasVelocity() const
      {
          return vx_ != SHOWINFO_SCALE2F;
          //&& vy_ != SHOWINFO_SCALE2F;
      }

    /*!
      \brief check if this object has neck info
      \return checked result
     */
    bool hasNeck() const
      {
          return neck_ != SHOWINFO_SCALE2F;
      }

    /*!
      \brief check if this object has view width info
      \return checked result
     */
    bool hasView() const
      {
          return view_width_ != SHOWINFO_SCALE2F;
      }

    /*!
      \brief check if this object has stamina info
      \return true if this object has stamina info
     */
    bool hasStamina() const
      {
          return stamina_ != SHOWINFO_SCALE2F;
      }

    /*!
      \brief check if this object has stamina capacity info
      \return checked result
     */
    bool hasStaminaCapacity() const
      {
          return stamina_capacity_ >= 0.0f;
      }

    /*!
      \brief check if this object has command count info
      \return checked result
     */
    bool hasCommandCount() const
      {
          return kick_count_ != 0xFFFF;
      }

    /*!
      \brief check if this object has arm information
      \return checked result
     */
    bool hasArm() const
      {
          return point_x_ != SHOWINFO_SCALE2F;
      }

    //
    // player state
    //

    /*!
      \brief check if this object is enabled.
      \return checked result
     */
    bool isAlive() const
      {
          return state_ != 0;
      }

    /*!
      \brief check if this object is kicking.
      \return checked result
     */
    bool isKicking() const
      {
          return state_ & KICK
              && !( state_ & KICK_FAULT );
      }

    /*!
      \brief check if this object is kicking fault.
      \return checked result
     */
    bool isKickingFault() const
      {
          return state_ & KICK_FAULT;
      }

    /*!
      \brief check if this object is goalie.
      \return checked result
     */
    bool isGoalie() const
      {
          return state_ & GOALIE;
      }

    /*!
      \brief check if this object is catching.
      \return checked result
     */
    bool isCatching() const
      {
          return state_ & CATCH
              && !( state_ & CATCH_FAULT );
      }

    /*!
      \brief check if this object is catching fault.
      \return checked result
     */
    bool isCatchingFault() const
      {
          return state_ & CATCH_FAULT;
      }

    /*!
      \brief check if this object is collided with ball.
      \return checked result
     */
    bool isCollidedBall() const
      {
          return state_ & BALL_COLLIDE
              || state_ & BALL_TO_PLAYER
              || state_ & PLAYER_TO_BALL;
      }

    /*!
      \brief check if this object is collided with player.
      \return checked result
     */
    bool isCollidedPlayer() const
      {
          return state_ & PLAYER_COLLIDE;
      }

    /*!
      \brief check if this object is tackling
      \return checked result
     */
    bool isTackling() const
      {
          return state_ & TACKLE
              && !( state_ & TACKLE_FAULT );
      }

    /*!
      \brief check if this object is tackling fault.
      \return checked result
     */
    bool isTacklingFault() const
      {
          return state_ & TACKLE_FAULT;
      }

    /*!
      \brief check if this object is pointing to somewhere.
      \return checked result
     */
    bool isPointing() const
      {
          return point_x_ != SHOWINFO_SCALE2F
              && point_y_ != SHOWINFO_SCALE2F;
      }

    /*!
      \brief check if this object is focusing to someone.
      \return checked result
     */
    bool isFocusing() const
      {
          return focus_side_ != 'n';
      }

    /*!
      \brief check if this player is chaged by an opponent player.
      \return checked result
     */
    bool isFoulCharged() const
      {
          return state_ & FOUL_CHARGED;
      }

    /*!
      \brief check if this player has yellow card or not.
      \return checked result.
     */
    bool hasYellowCard() const
      {
          return state_ & YELLOW_CARD;
      }

    /*!
      \brief check if this player has red card or not.
      \return checked result.
     */
    bool hasRedCard() const
      {
          return state_ & RED_CARD;
      }

    /*!
      \brief check if this player is marked as an illegal defense state
      \return checked result.
     */
    bool isIllegalDefenseState() const
      {
          return state_ & ILLEGAL_DEFENSE;
      }

    /*!
      \brief check if view quality is high.
      \return true if view quality is high.
     */
    bool highQuality() const
      {
          return ( view_quality_ == 'h' );
      }

    /*!
      \brief get focused player's side id
      \return side id
     */
    SideID focusSide() const
      {
          return ( focus_side_ == 'l' ? LEFT
                   : focus_side_ == 'r' ? RIGHT
                   : NEUTRAL );
      }

    //
    // accessor methods
    //

    int focusUnum() const { return focus_unum_; }

    double x() const { return x_; }
    double y() const { return y_; }

    double deltaX() const { return vx_; }
    double deltaY() const { return vy_; }

    double body() const { return body_; }
    double head() const { return body_ + neck_; }

    double pointX() const { return point_x_; }
    double pointY() const { return point_y_; }

    double viewWidth() const { return view_width_; }

    double stamina() const { return stamina_; }
    double effort() const { return effort_; }
    double recovery() const { return recovery_; }
    double staminaCapacity() const { return stamina_capacity_; }

    int kickCount() const { return kick_count_; }
    int dashCount() const { return dash_count_; }
    int turnCount() const { return turn_count_; }
    int catchCount() const { return catch_count_; }
    int moveCount() const { return move_count_; }
    int turnNeckCount() const { return turn_neck_count_; }
    int changeViewCount() const { return change_view_count_; }
    int sayCount() const { return say_count_; }
    int tackleCount() const { return tackle_count_; }
    int pointtoCount() const { return pointto_count_; }
    int attentiontoCount() const { return attentionto_count_; }

    bool hasFullEffort( const double max_effort ) const
      {
          return std::fabs( max_effort - effort_ ) < 1.0e-5;
      }

    bool hasFullRecovery() const
      {
          return std::fabs( 1.0 - recovery_ ) < 1.0e-5;
      }
};

/*!
  \struct TeamT
  \brief team information
 */
struct TeamT {
    std::string name_; //!< team name string
    UInt16 score_; //!< total scores in normal game
    UInt16 pen_score_; //!< count of penalty score
    UInt16 pen_miss_; //!< count of penalty miss

    /*!
      \brief initialize all variables by 0
     */
    TeamT()
        : score_( 0 ),
          pen_score_( 0 ),
          pen_miss_( 0 )
      { }

    /*!
      \brief initialize all variables by specified values
      \param name team name
      \param score total score
      \param pen_score count of penalty score
      \param pen_miss count of penalty miss
     */
    TeamT( const char * name,
           const UInt16 score,
           const UInt16 pen_score,
           const UInt16 pen_miss )
        : name_( name ),
          score_( score ),
          pen_score_( pen_score ),
          pen_miss_( pen_miss )
      { }

    /*!
      \brief clear all variables
     */
    void clear()
      {
          name_.erase();
          score_ = 0;
          pen_score_ = 0;
          pen_miss_ = 0;
      }

    /*!
      \brief get the team name.
      \return team name string.
     */
    const std::string & name() const
      {
          return name_;
      }

    /*!
      \brief get the score of normal and overtime games.
      \return score value.
     */
    int score() const
      {
          return score_;
      }

    /*!
      \brief get the score of penalty shootouts.
      \return score of penalty shootouts.
     */
    int penaltyScore() const
      {
          return pen_score_;
      }

    /*!
      \brief get the miss count of penalty shootouts.
      \return miss count of penalty shootouts.
     */
    int penaltyMiss() const { return pen_miss_; }

    /*!
      \brief get the total penalty trial count
      \return the total penalty trial count
     */
    int penaltyTrial() const
      {
          return pen_score_ + pen_miss_;
      }

    /*!
      \brief check if artument variable has same values.
      \param other compared variable.
      \return compared result.
     */
    bool equals( const TeamT & other ) const
      {
          return ( name_ == other.name_
                   && score_ == other.score_
                   && pen_score_ == other.pen_score_
                   && pen_miss_ == other.pen_miss_ );
      }

};

/*!
  \struct ShowInfoT
  \brief show information
 */
struct ShowInfoT {
    UInt32 time_; //!< game time
    BallT ball_; //!< ball data
    PlayerT player_[MAX_PLAYER * 2]; //!< player data
};

/*!
  \struct DispInfoT
  \brief display information
 */
struct DispInfoT {
    PlayMode pmode_; //!< playmode id
    TeamT team_[2]; //!< team data
    ShowInfoT show_; //!< positional data
};


//! recorded value of rcg v4
const int REC_VERSION_4 = 4;

//! recorded value of rcg v5
const int REC_VERSION_5 = 5;

//! default rcg version
const int DEFAULT_LOG_VERSION = REC_VERSION_5;

} // end namespace
} // end namespace

#endif
