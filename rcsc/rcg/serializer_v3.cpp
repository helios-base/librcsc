// -*-c++-*-

/*!
  \file serializer_v3.cpp
  \brief v3 format rcg serializer Source File.
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

#include "serializer_v3.h"

#include "util.h"

#include <iostream>
#include <cstring>
#include <cstdio>

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serializeBegin( std::ostream & os,
                              const std::string &,
                              const std::string & )
{
    return serializeImpl( os, REC_VERSION_3 );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const server_params_t & param )
{
    return serializeImpl( os, param );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const player_params_t & pparam )
{
    return serializeImpl( os, pparam );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const player_type_t & type )
{
    return serializeImpl( os, type );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const dispinfo_t & disp )
{
    switch ( ntohs( disp.mode ) ) {
    case SHOW_MODE:
        serialize( os, disp.body.show );
        break;
    case MSG_MODE:
        serialize( os, disp.body.msg );
        break;
    case DRAW_MODE:
        serialize( os, disp.body.draw );
        break;
    default:
        break;
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const showinfo_t & show )
{
    showinfo_t2 show2;

    convert( show, show2 );

    return serializeImpl( os, show2 );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const showinfo_t2 & show2 )
{
    return serializeImpl( os, show2 );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const short_showinfo_t2 & show2 )
{
    return serializeImpl( os, show2 );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const msginfo_t & msg )
{
    return serializeImpl( os, msg );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const Int16 board,
                         const std::string & msg )
{
    msginfo_t info;

    info.board = board;
    std::memset( info.message, 0, sizeof( info.message ) );
    std::strncpy( info.message, msg.c_str(),
                  std::min( sizeof( info.message ) - 1, msg.length() ) );

    return serializeImpl( os, info );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const drawinfo_t & draw )
{
    return serializeImpl( os, draw );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const char playmode )
{
    M_playmode = playmode;

    return serializeImpl( os, playmode );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const team_t & team_l,
                         const team_t & team_r )
{
    convert( team_l, M_teams[0] );
    convert( team_r, M_teams[1] );

    return serializeImpl( os, team_l, team_r );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const TeamT & team_l,
                         const TeamT & team_r )
{
    M_teams[0] = team_l;
    M_teams[1] = team_r;

    team_t teams[2];
    convert( team_l, teams[0] );
    convert( team_r, teams[1] );

    return serializeImpl( os, teams[0], teams[1] );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const ShowInfoT & show )
{
    short_showinfo_t2 short_show2;

    convert( show, short_show2 );

    return serializeImpl( os, short_show2 );

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const DispInfoT & disp )
{
    char pm = static_cast< char >( disp.pmode_ );
    if ( pm != M_playmode )
    {
        serialize( os, pm );
    }

//     if ( M_teams[0].name_ != disp.team_[0].name_
//          || M_teams[0].score_ != disp.team_[0].score_
//          || M_teams[0].pen_score_ != disp.team_[0].pen_score_
//          || M_teams[0].pen_miss_ != disp.team_[0].pen_miss_
//          || M_teams[1].name_ != disp.team_[1].name_
//          || M_teams[1].score_ != disp.team_[1].score_
//          || M_teams[1].pen_score_ != disp.team_[1].pen_score_
//          || M_teams[1].pen_miss_ != disp.team_[1].pen_miss_ )
    if ( ! M_teams[0].equals( disp.team_[0] )
         || ! M_teams[1].equals( disp.team_[1] ) )
    {
        serialize( os, disp.team_[0], disp.team_[1] );
    }

    return serialize( os, disp.show_ );
}


/*-------------------------------------------------------------------*/
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const ServerParamT & param )
{
    server_params_t to;

    to.goal_width = hdtonl( param.goal_width_ );
    to.inertia_moment = hdtonl( param.inertia_moment_ );
    to.player_size = hdtonl( param.player_size_ );
    to.player_decay = hdtonl( param.player_decay_ );
    to.player_rand = hdtonl( param.player_rand_ );
    to.player_weight = hdtonl( param.player_weight_ );
    to.player_speed_max = hdtonl( param.player_speed_max_ );
    to.player_accel_max = hdtonl( param.player_accel_max_ );
    to.stamina_max = hdtonl( param.stamina_max_ );
    to.stamina_inc = hdtonl( param.stamina_inc_max_ );
    to.recover_init = hdtonl( param.recover_init_ );
    to.recover_dec_thr = hdtonl( param.recover_dec_thr_ );
    to.recover_min = hdtonl( param.recover_min_ );
    to.recover_dec = hdtonl( param.recover_dec_ );
    to.effort_init = hdtonl( param.effort_init_ );
    to.effort_dec_thr = hdtonl( param.effort_dec_thr_ );
    to.effort_min = hdtonl( param.effort_min_ );
    to.effort_dec = hdtonl( param.effort_dec_ );
    to.effort_inc_thr = hdtonl( param.effort_inc_thr_ );
    to.effort_inc = hdtonl( param.effort_inc_ );
    to.kick_rand = hdtonl( param.kick_rand_ );
    to.team_actuator_noise = hbtons( param.team_actuator_noise_ );
    to.player_rand_factor_l = hdtonl( param.player_rand_factor_l_ );
    to.player_rand_factor_r = hdtonl( param.player_rand_factor_r_ );
    to.kick_rand_factor_l = hdtonl( param.kick_rand_factor_l_ );
    to.kick_rand_factor_r = hdtonl( param.kick_rand_factor_r_ );
    to.ball_size = hdtonl( param.ball_size_ );
    to.ball_decay = hdtonl( param.ball_decay_ );
    to.ball_rand = hdtonl( param.ball_rand_ );
    to.ball_weight = hdtonl( param.ball_weight_ );
    to.ball_speed_max = hdtonl( param.ball_speed_max_ );
    to.ball_accel_max = hdtonl( param.ball_accel_max_ );
    to.dash_power_rate = hdtonl( param.dash_power_rate_ );
    to.kick_power_rate = hdtonl( param.kick_power_rate_ );
    to.kickable_margin = hdtonl( param.kickable_margin_ );
    to.control_radius = hdtonl( param.control_radius_ );
    to.control_radius_width = hdtonl( 1.7 );
    to.max_power = hdtonl( param.max_power_ );
    to.min_power = hdtonl( param.min_power_ );
    to.max_moment = hdtonl( param.max_moment_ );
    to.min_moment = hdtonl( param.min_moment_ );
    to.max_neck_moment = hdtonl( param.max_neck_moment_ );
    to.min_neck_moment = hdtonl( param.min_neck_moment_ );
    to.max_neck_angle = hdtonl( param.max_neck_angle_ );
    to.min_neck_angle = hdtonl( param.min_neck_angle_ );
    to.visible_angle = hdtonl( param.visible_angle_ );
    to.visible_distance = hdtonl( param.visible_distance_ );
    to.wind_dir = hdtonl( param.wind_dir_ );
    to.wind_force = hdtonl( param.wind_force_ );
    to.wind_ang = hdtonl( param.wind_angle_ );
    to.wind_rand = hdtonl( param.wind_rand_ );
    to.kickable_area = hdtonl( param.player_size_ + param.kickable_margin_ + param.ball_size_ );
    to.catch_area_l = hdtonl( param.catchable_area_l_ );
    to.catch_area_w = hdtonl( param.catchable_area_w_ );
    to.catch_probability = hdtonl( param.catch_probability_ );

    to.goalie_max_moves = hitons( param.goalie_max_moves_ );

    to.corner_kick_margin = hdtonl( param.corner_kick_margin_ );
    to.offside_active_area = hdtonl( param.offside_active_area_size_ );

    to.wind_none = hbtons( param.wind_none_ );
    to.use_wind_random = hbtons( param.use_wind_random_ );
    to.coach_say_count_max = hitons( param.coach_say_count_max_ );
    to.coach_say_msg_size = hitons( param.coach_say_msg_size_ );
    to.clang_win_size = hitons( param.clang_win_size_ );
    to.clang_define_win = hitons( param.clang_define_win_ );
    to.clang_meta_win = hitons( param.clang_meta_win_ );
    to.clang_advice_win = hitons( param.clang_advice_win_ );
    to.clang_info_win = hitons( param.clang_info_win_ );
    to.clang_mess_delay = hitons( param.clang_mess_delay_ );
    to.clang_mess_per_cycle = hitons( param.clang_mess_per_cycle_ );
    to.half_time = hitons( param.half_time_ );
    to.simulator_step = hitons( param.simulator_step_ );
    to.send_step = hitons( param.send_step_ );
    to.recv_step = hitons( param.recv_step_ );
    to.sense_body_step = hitons( param.sense_body_step_ );
    to.lcm_step = hitons( 300 );
    to.player_say_msg_size = hitons( param.player_say_msg_size_ );
    to.player_hear_max = hitons( param.player_hear_max_ );
    to.player_hear_inc = hitons( param.player_hear_inc_ );
    to.player_hear_decay = hitons( param.player_hear_decay_ );
    to.catch_ban_cycle = hitons( param.catch_ban_cycle_ );
    to.slow_down_factor = hitons( param.slow_down_factor_ );
    to.use_offside = hitons( param.use_offside_ );
    to.kickoff_offside = hbtons( param.kickoff_offside_ );

    to.offside_kick_margin = hdtonl( param.offside_kick_margin_ );
    to.audio_cut_dist = hdtonl( param.audio_cut_dist_ );
    to.dist_quantize_step = hdtonl( param.dist_quantize_step_ );
    to.landmark_dist_quantize_step = hdtonl( param.landmark_dist_quantize_step_ );

    to.dir_quantize_step = hdtonl( 0.0 );
    to.dist_quantize_step_l = hdtonl( 0.0 );
    to.dist_quantize_step_r = hdtonl( 0.0 );
    to.landmark_dist_quantize_step_l = hdtonl( 0.0 );
    to.landmark_dist_quantize_step_r = hdtonl( 0.0 );
    to.dir_quantize_step_l = hdtonl( 0.0 );
    to.dir_quantize_step_r = hdtonl( 0.0 );

    to.coach_mode = hbtons( param.coach_mode_ );
    to.coach_with_referee_mode = hbtons( param.coach_with_referee_mode_ );
    to.use_old_coach_hear = hbtons( param.use_old_coach_hear_ );
    to.online_coach_look_step = hitons( param.online_coach_look_step_ );

    to.slowness_on_top_for_left_team = hdtonl( param.slowness_on_top_for_left_team_ );
    to.slowness_on_top_for_right_team = hdtonl( param.slowness_on_top_for_right_team_ );
    to.ka_length = hdtonl( param.keepaway_length_ );
    to.ka_width = hdtonl( param.keepaway_width_ );
    to.ball_stuck_area = hdtonl( param.ball_stuck_area_ );
    to.max_tackle_power = hdtonl( param.max_tackle_power_ );
    to.max_back_tackle_power = hdtonl( param.max_back_tackle_power_ );
    to.tackle_dist = hdtonl( param.tackle_dist_ );
    to.tackle_back_dist = hdtonl( param.tackle_back_dist_ );
    to.tackle_width = hdtonl( param.tackle_width_ );

    to.start_goal_l = hitons( param.start_goal_l_ );
    to.start_goal_r = hitons( param.start_goal_r_ );
    to.fullstate_l = hbtons( param.fullstate_l_ );
    to.fullstate_r = hbtons( param.fullstate_r_ );
    to.drop_ball_time = hitons( param.drop_ball_time_ );
    to.synch_mode = hbtons( param.synch_mode_ );
    to.synch_offset = hitons( param.synch_offset_ );
    to.synch_micro_sleep = hitons( param.synch_micro_sleep_ );
    to.point_to_ban = hitons( param.point_to_ban_ );
    to.point_to_duration = hitons( param.point_to_duration_ );

    return serializeImpl( os, to );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const PlayerParamT & param )
{
    player_params_t to;

    to.player_types = hitons( param.player_types_ );
    to.substitute_max = hitons( param.substitute_max_ );
    to.pt_max = hitons( param.pt_max_ );
    to.player_speed_max_delta_min = hdtonl( param.player_speed_max_delta_min_ );
    to.player_speed_max_delta_max = hdtonl( param.player_speed_max_delta_max_ );
    to.stamina_inc_max_delta_factor = hdtonl( param.stamina_inc_max_delta_factor_ );
    to.player_decay_delta_min = hdtonl( param.player_decay_delta_min_ );
    to.player_decay_delta_max = hdtonl( param.player_decay_delta_max_ );
    to.inertia_moment_delta_factor = hdtonl( param.inertia_moment_delta_factor_ );
    to.dash_power_rate_delta_min = hdtonl( param.dash_power_rate_delta_min_ );
    to.dash_power_rate_delta_max = hdtonl( param.dash_power_rate_delta_max_ );
    to.player_size_delta_factor = hdtonl( param.player_size_delta_factor_ );
    to.kickable_margin_delta_min = hdtonl( param.kickable_margin_delta_min_ );
    to.kickable_margin_delta_max = hdtonl( param.kickable_margin_delta_max_ );
    to.kick_rand_delta_factor = hdtonl( param.kick_rand_delta_factor_ );
    to.extra_stamina_delta_min = hdtonl( param.extra_stamina_delta_min_ );
    to.extra_stamina_delta_max = hdtonl( param.extra_stamina_delta_max_ );
    to.effort_max_delta_factor = hdtonl( param.effort_max_delta_factor_ );
    to.effort_min_delta_factor = hdtonl( param.effort_min_delta_factor_ );
    to.random_seed = htonl( param.random_seed_ );
    to.new_dash_power_rate_delta_min = hdtonl( param.new_dash_power_rate_delta_min_ );
    to.new_dash_power_rate_delta_max = hdtonl( param.new_dash_power_rate_delta_max_ );
    to.new_stamina_inc_max_delta_factor = hdtonl( param.new_stamina_inc_max_delta_factor_ );
    to.allow_mult_default_type = hbtons( param.allow_mult_default_type_ );

    return serializeImpl( os, to );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerV3::serialize( std::ostream & os,
                         const PlayerTypeT & param )
{
    player_type_t to;

    to.id = hitons( param.id_ );
    to.player_speed_max = hdtonl( param.player_speed_max_ );
    to.stamina_inc_max = hdtonl( param.stamina_inc_max_ );
    to.player_decay = hdtonl( param.player_decay_ );
    to.inertia_moment = hdtonl( param.inertia_moment_ );
    to.dash_power_rate = hdtonl( param.dash_power_rate_ );
    to.player_size = hdtonl( param.player_size_ );
    to.kickable_margin = hdtonl( param.kickable_margin_ );
    to.kick_rand = hdtonl( param.kick_rand_ );
    to.extra_stamina = hdtonl( param.extra_stamina_ );
    to.effort_max = hdtonl( param.effort_max_ );
    to.effort_min = hdtonl( param.effort_min_ );

    return serializeImpl( os, to );
}

/*-------------------------------------------------------------------*/
namespace {

Serializer::Ptr
create_v3()
{
    Serializer::Ptr ptr( new SerializerV3() );
    return ptr;
}

rcss::RegHolder v3 = Serializer::creators().autoReg( &create_v3, REC_VERSION_3 );

}

} // end of namespace
} // end of namespace
