// -*-c++-*-

/*!
  \file serializer_json.cpp
  \brief json format rcg serializer Source File.
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

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "serializer_json.h"

#include "types.h"
#include "util.h"

#include <iomanip>
#include <cstring>
#include <cmath>

namespace {

constexpr double POS_PREC = 0.0001;
constexpr double DIR_PREC = 0.001;

inline
double
quantize( const double & val,
          const double & prec = 0.0001 )
{
    return rint( val / prec ) * prec;
}

}

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serializeBegin( std::ostream & os,
                                const std::string & server_version,
                                const std::string & timestamp )
{
    os << "[\n";

    // server version
    os << '{' << std::quoted( "version" ) << ':';
    if ( server_version.empty() )
    {
        os << std::quoted( "unknown" );
    }
    else
    {
        os << std::quoted( server_version ) << '}';
    }

    // time stamp
    if ( ! timestamp.empty() )
    {
        os << ",\n";
        os << '{' << std::quoted( "timestamp" ) << ':' << std::quoted( timestamp ) << '}';
    }

    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serializeEnd( std::ostream & os )
{
    return os << "\n]";
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const server_params_t & param )
{
    double tmp = 0.0;

    os << ",\n";
    os << '{' << std::quoted( "server_param" ) << ':'
       << '{';
    os << std::quoted( "goal_width" ) << ':' << quantize( nltohd( param.goal_width ) ) << ','
       << std::quoted( "inertia_moment" ) << ':' << quantize( nltohd( param.inertia_moment ) ) << ','
       << std::quoted( "player_size" ) << ':' << quantize( nltohd( param.player_size ) ) << ','
       << std::quoted( "player_decay" ) << ':' << quantize( nltohd( param.player_decay ) ) << ','
       << std::quoted( "player_rand" ) << ':' << quantize( nltohd( param.player_rand ), 0.0001 ) << ','
       << std::quoted( "player_weight" ) << ':' << quantize( nltohd( param.player_weight ) ) << ','
       << std::quoted( "player_speed_max" ) << ':' << quantize( nltohd( param.player_speed_max ) ) << ','
       << std::quoted( "player_accel_max" ) << ':' << quantize( nltohd( param.player_accel_max ) ) << ','
       << std::quoted( "stamina_max" ) << ':' << quantize( nltohd( param.stamina_max ) ) << ','
       << std::quoted( "stamina_inc_max" ) << ':' << quantize( nltohd( param.stamina_inc ) ) << ','
       << std::quoted( "recover_init" ) << ':' << quantize( nltohd( param.recover_init ) ) << ','
       << std::quoted( "recover_dec_thr" ) << ':' << quantize( nltohd( param.recover_dec_thr ) ) << ','
       << std::quoted( "recover_min" ) << ':' << quantize( nltohd( param.recover_min ) ) << ','
       << std::quoted( "recover_dec" ) << ':' << quantize( nltohd( param.recover_dec ) ) << ','
       << std::quoted( "effort_init" ) << ':' << quantize( nltohd( param.effort_init ) ) << ','
       << std::quoted( "effort_dec_thr" ) << ':' << quantize( nltohd( param.effort_dec_thr ) ) << ','
       << std::quoted( "effort_min" ) << ':' << quantize( nltohd( param.effort_min ), 0.00001 ) << ','
       << std::quoted( "effort_dec" ) << ':' << quantize( nltohd( param.effort_dec ), 0.00001 ) << ','
       << std::quoted( "effort_inc_thr" ) << ':' << quantize( nltohd( param.effort_inc_thr ) ) << ','
       << std::quoted( "effort_inc" ) << ':' << quantize( nltohd( param.effort_inc ), 0.0001 ) << ','
       << std::quoted( "kick_rand" ) << ':' << quantize( nltohd( param.kick_rand ), 0.00001 ) << ','
       << std::quoted( "team_actuator_noise" ) << ':' << nstohi( param.team_actuator_noise ) << ','
       << std::quoted( "prand_factor_l" ) << ':' << quantize( nltohd( param.player_rand_factor_l ) ) << ','
       << std::quoted( "prand_factor_r" ) << ':' << quantize( nltohd( param.player_rand_factor_r ) ) << ','
       << std::quoted( "kick_rand_factor_l" ) << ':' << quantize( nltohd( param.kick_rand_factor_l ) ) << ','
       << std::quoted( "kick_rand_factor_r" ) << ':' << quantize( nltohd( param.kick_rand_factor_r ) ) << ','
       << std::quoted( "ball_size" ) << ':' << quantize( nltohd( param.ball_size ) ) << ','
       << std::quoted( "ball_decay" ) << ':' << quantize( nltohd( param.ball_decay ) ) << ','
       << std::quoted( "ball_rand" ) << ':' << quantize( nltohd( param.ball_rand ) ) << ','
       << std::quoted( "ball_weight" ) << ':' << quantize( nltohd( param.ball_weight ) ) << ','
       << std::quoted( "ball_speed_max" ) << ':' << quantize( nltohd( param.ball_speed_max ) ) << ','
       << std::quoted( "ball_accel_max" ) << ':' << quantize( nltohd( param.ball_accel_max ) ) << ','
       << std::quoted( "dash_power_rate" ) << ':' << quantize( nltohd( param.dash_power_rate ), 0.0001 ) << ','
       << std::quoted( "kick_power_rate" ) << ':' << quantize( nltohd( param.kick_power_rate ), 0.0001 ) << ','
       << std::quoted( "kickable_margin" ) << ':' << quantize( nltohd( param.kickable_margin ) ) << ','
       << std::quoted( "control_radius" ) << ':' << quantize( nltohd( param.control_radius ) ) << ','
        //<< std::quoted( "control_radius_width" ) << ':' << quantize( nltohd( param.control_radius_width ) ) << ','
       << std::quoted( "maxpower" ) << ':' << quantize( nltohd( param.max_power ) ) << ','
       << std::quoted( "minpower" ) << ':' << quantize( nltohd( param.min_power ) ) << ','
       << std::quoted( "maxmoment" ) << ':' << quantize( nltohd( param.max_moment ) ) << ','
       << std::quoted( "minmoment" ) << ':' << quantize( nltohd( param.min_moment ) ) << ','
       << std::quoted( "maxneckmoment" ) << ':' << quantize( nltohd( param.max_neck_moment ) ) << ','
       << std::quoted( "minneckmoment" ) << ':' << quantize( nltohd( param.min_neck_moment ) ) << ','
       << std::quoted( "maxneckang" ) << ':' << quantize( nltohd( param.max_neck_angle ) ) << ','
       << std::quoted( "minneckang" ) << ':' << quantize( nltohd( param.min_neck_angle ) ) << ','
       << std::quoted( "visible_angle" ) << ':' << quantize( nltohd( param.visible_angle ) ) << ','
       << std::quoted( "visible_distance" ) << ':' << quantize( nltohd( param.visible_distance ) ) << ','
       << std::quoted( "wind_dir" ) << ':' << quantize( nltohd( param.wind_dir ) ) << ','
       << std::quoted( "wind_force" ) << ':' << quantize( nltohd( param.wind_force ) ) << ','
       << std::quoted( "wind_ang" ) << ':' << quantize( nltohd( param.wind_ang ) ) << ','
       << std::quoted( "wind_rand" ) << ':' << quantize( nltohd( param.wind_rand ) ) << ','
        //<< std::quoted( "kickable_area" ) << ':' << quantize( nltohd( param.kickable_area ) ) << ','
       << std::quoted( "catchable_area_l" ) << ':' << quantize( nltohd( param.catch_area_l ) ) << ','
       << std::quoted( "catchable_area_w" ) << ':' << quantize( nltohd( param.catch_area_w ) ) << ','
       << std::quoted( "catch_probability" ) << ':' << quantize( nltohd( param.catch_probability ) ) << ','
       << std::quoted( "goalie_max_moves" ) << ':' << nstohi( param.goalie_max_moves ) << ','
       << std::quoted( "ckick_margin" ) << ':' << quantize( nltohd( param.corner_kick_margin ) ) << ','
       << std::quoted( "offside_active_area_size" ) << ':' << quantize( nltohd( param.offside_active_area ) ) << ','
       << std::quoted( "wind_none" ) << ':' << nstohi( param.wind_none ) << ','
       << std::quoted( "wind_random" ) << ':' << nstohi( param.use_wind_random ) << ','
       << std::quoted( "say_coach_cnt_max" ) << ':' << nstohi( param.coach_say_count_max ) << ','
       << std::quoted( "say_coach_msg_size" ) << ':' << nstohi( param.coach_say_msg_size ) << ','
       << std::quoted( "clang_win_size" ) << ':' << nstohi( param.clang_win_size ) << ','
       << std::quoted( "clang_define_win" ) << ':' << nstohi( param.clang_define_win ) << ','
       << std::quoted( "clang_meta_win" ) << ':' << nstohi( param.clang_meta_win ) << ','
       << std::quoted( "clang_advice_win" ) << ':' << nstohi( param.clang_advice_win ) << ','
       << std::quoted( "clang_info_win" ) << ':' << nstohi( param.clang_info_win ) << ','
       << std::quoted( "clang_mess_delay" ) << ':' << nstohi( param.clang_mess_delay ) << ','
       << std::quoted( "clang_mess_per_cycle" ) << ':' << nstohi( param.clang_mess_per_cycle ) << ','
       << std::quoted( "half_time" ) << ':' << nstohi( param.half_time ) << ','
       << std::quoted( "simulator_step" ) << ':' << nstohi( param.simulator_step ) << ','
       << std::quoted( "send_step" ) << ':' << nstohi( param.send_step ) << ','
       << std::quoted( "recv_step" ) << ':' << nstohi( param.recv_step ) << ','
       << std::quoted( "sense_body_step" ) << ':' << nstohi( param.sense_body_step ) << ','
        //<< std::quoted( "lcm_step" ) << ':' << nstohi( param.lcm_step ) << ','
       << std::quoted( "say_msg_size" ) << ':' << nstohi( param.player_say_msg_size ) << ','
       << std::quoted( "hear_max" ) << ':' << nstohi( param.player_hear_max ) << ','
       << std::quoted( "hear_inc" ) << ':' << nstohi( param.player_hear_inc ) << ','
       << std::quoted( "hear_decay" ) << ':' << nstohi( param.player_hear_decay ) << ','
       << std::quoted( "catch_ban_cycle" ) << ':' << nstohi( param.catch_ban_cycle ) << ','
       << std::quoted( "slow_down_factor" ) << ':' << nstohi( param.slow_down_factor ) << ','
       << std::quoted( "use_offside" ) << ':' << nstohi( param.use_offside ) << ','
       << std::quoted( "forbid_kick_off_offside" ) << ':' << nstohi( param.kickoff_offside ) << ','
       << std::quoted( "offside_kick_margin" ) << ':' << quantize( nltohd( param.offside_kick_margin ) ) << ','
       << std::quoted( "audio_cut_dist" ) << ':' << quantize( nltohd( param.audio_cut_dist ) ) << ','
       << std::quoted( "quantize_step" ) << ':' << quantize( nltohd( param.dist_quantize_step ) ) << ','
       << std::quoted( "quantize_step_l" ) << ':' << quantize( nltohd( param.landmark_dist_quantize_step ), 0.0001 ) << ','
        //<< std::quoted( "quantize_step_dir" ) << ':' << quantize( nltohd( param.dir_quantize_step ) ) << ','
        //<< std::quoted( "quantize_step_dist_team_l" ) << ':' << quantize( nltohd( param.dist_quantize_step_l ) ) << ','
        //<< std::quoted( "quantize_step_dist_team_r" ) << ':' << quantize( nltohd( param.dist_quantize_step_r ) ) << ','
        //<< std::quoted( "quantize_step_dist_l_team_l" ) << ':' << quantize( nltohd( param.landmark_dist_quantize_step_l ) ) << ','
        //<< std::quoted( "quantize_step_dist_l_team_r" ) << ':' << quantize( nltohd( param.landmark_dist_quantize_step_r ) ) << ','
        //<< std::quoted( "quantize_step_dir_team_l" ) << ':' << quantize( nltohd( param.dir_quantize_step_l ) ) << ','
        //<< std::quoted( "quantize_step_dir_team_r" ) << ':' << quantize( nltohd( param.dir_quantize_step_r ) ) << ','
       << std::quoted( "coach" ) << ':' << nstohi( param.coach_mode ) << ','
       << std::quoted( "coach_w_referee" ) << ':' << nstohi( param.coach_with_referee_mode ) << ','
       << std::quoted( "old_coach_hear" ) << ':' << nstohi( param.use_old_coach_hear ) << ','
       << std::quoted( "send_vi_step" ) << ':' << nstohi( param.online_coach_look_step ) << ','
       << std::quoted( "slowness_on_top_for_left_team" ) << ':' << quantize( nltohd( param.slowness_on_top_for_left_team ) ) << ','
       << std::quoted( "slowness_on_top_for_right_team" ) << ':' << quantize( nltohd( param.slowness_on_top_for_right_team ) ) << ','
       << std::quoted( "keepaway_length" ) << ':' << quantize( nltohd( param.ka_length ) ) << ','
       << std::quoted( "keepaway_width" ) << ':' << quantize( nltohd( param.ka_width ) ) << ',';

    tmp = quantize( nltohd( param.ball_stuck_area ) );
    if ( std::fabs( tmp ) < 100.0 ) os << std::quoted( "ball_stuck_area" ) << ':' << tmp << ',';
    tmp = quantize( nltohd( param.max_tackle_power ) );
    if ( 0.0 <= tmp && tmp < 200.0 ) os << std::quoted( "max_tackle_power" ) << ':' << tmp << ',';
    tmp = quantize( nltohd( param.max_back_tackle_power ) );
    if ( 0.0 <= tmp && tmp < 200.0 ) os << std::quoted( "max_back_tackle_power" ) << ':' << tmp << ',';
    tmp = quantize( nltohd( param.tackle_dist ) );
    if ( 0.0 <= tmp && tmp < 100.0 ) os << std::quoted( "tackle_dist" ) << ':' << tmp << ',';
    tmp = quantize( nltohd( param.tackle_back_dist ) );
    if ( 0.0 <= tmp && tmp < 100.0 ) os << std::quoted( "tackle_back_dist" ) << ':' << tmp << ',';
    tmp = quantize( nltohd( param.tackle_width ) );
    if ( 0.0 <= tmp && tmp < 100.0 ) os << std::quoted( "tackle_width" ) << ':' << tmp << ',';

    os << std::quoted( "start_goal_l" ) << ':' << nstohi( param.start_goal_l ) << ','
       << std::quoted( "start_goal_r" ) << ':' << nstohi( param.start_goal_r ) << ','
       << std::quoted( "fullstate_l" ) << ':' << nstohi( param.fullstate_l ) << ','
       << std::quoted( "fullstate_r" ) << ':' << nstohi( param.fullstate_r ) << ','
       << std::quoted( "drop_ball_time" ) << ':' << nstohi( param.drop_ball_time ) << ','
       << std::quoted( "synch_mode" ) << ':' << nstohi( param.synch_mode ) << ','
       << std::quoted( "synch_offset" ) << ':' << nstohi( param.synch_offset ) << ','
       << std::quoted( "synch_micro_sleep" ) << ':' << nstohi( param.synch_micro_sleep ) << ','
       << std::quoted( "point_to_ban" ) << ':' << nstohi( param.point_to_ban ) << ','
       << std::quoted( "point_to_duration" ) << ':' << nstohi( param.point_to_duration );

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const player_params_t & pparam )
{
    os << ",\n";
    os << '{' << std::quoted( "player_param" ) << ':'
       << '{';

    os << std::quoted( "player_types" ) << ':' << nstohi( pparam.player_types ) << ','
       << std::quoted( "subs_max" ) << ':' << nstohi( pparam.substitute_max ) << ','
       << std::quoted( "pt_max" ) << ':' << nstohi( pparam.pt_max ) << ','
       << std::quoted( "player_speed_max_delta_min" ) << ':' << quantize( nltohd( pparam.player_speed_max_delta_min ) ) << ','
       << std::quoted( "player_speed_max_delta_max" ) << ':' << quantize( nltohd( pparam.player_speed_max_delta_max ) ) << ','
       << std::quoted( "stamina_inc_max_delta_factor" ) << ':' << quantize( nltohd( pparam.stamina_inc_max_delta_factor ) ) << ','
       << std::quoted( "player_decay_delta_min" ) << ':' << quantize( nltohd( pparam.player_decay_delta_min ) ) << ','
       << std::quoted( "player_decay_delta_max" ) << ':' << quantize( nltohd( pparam.player_decay_delta_max ) ) << ','
       << std::quoted( "inertia_moment_delta_factor" ) << ':' << quantize( nltohd( pparam.inertia_moment_delta_factor ) ) << ','
       << std::quoted( "dash_power_rate_delta_min" ) << ':' << quantize( nltohd( pparam.dash_power_rate_delta_min ) ) << ','
       << std::quoted( "dash_power_rate_delta_max" ) << ':' << quantize( nltohd( pparam.dash_power_rate_delta_max ) ) << ','
       << std::quoted( "player_size_delta_factor" ) << ':' << quantize( nltohd( pparam.player_size_delta_factor ) ) << ','
       << std::quoted( "kickable_margin_delta_min" ) << ':' << quantize( nltohd( pparam.kickable_margin_delta_min ) ) << ','
       << std::quoted( "kickable_margin_delta_max" ) << ':' << quantize( nltohd( pparam.kickable_margin_delta_max ) ) << ','
       << std::quoted( "kick_rand_delta_factor" ) << ':' << quantize( nltohd( pparam.kick_rand_delta_factor ) ) << ','
       << std::quoted( "extra_stamina_delta_min" ) << ':' << quantize( nltohd( pparam.extra_stamina_delta_min ) ) << ','
       << std::quoted( "extra_stamina_delta_max" ) << ':' << quantize( nltohd( pparam.extra_stamina_delta_max ) ) << ','
       << std::quoted( "effort_max_delta_factor" ) << ':' << quantize( nltohd( pparam.effort_max_delta_factor ) ) << ','
       << std::quoted( "effort_min_delta_factor" ) << ':' << quantize( nltohd( pparam.effort_min_delta_factor ) ) << ','
       << std::quoted( "random_seed" ) << ':' << static_cast< Int32 >( ntohl( pparam.random_seed ) ) << ','
       << std::quoted( "new_dash_power_rate_delta_min" ) << ':' << quantize( nltohd( pparam.new_dash_power_rate_delta_min ) ) << ','
       << std::quoted( "new_dash_power_rate_delta_max" ) << ':' << quantize( nltohd( pparam.new_dash_power_rate_delta_max ) ) << ','
       << std::quoted( "new_stamina_inc_max_delta_factor" ) << ':' << quantize( nltohd( pparam.new_stamina_inc_max_delta_factor ) ) << ','
       << std::quoted( "allow_mult_default_type" ) << ':' << static_cast< bool >( nstohi( pparam.allow_mult_default_type ) );

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const player_type_t & type )
{
    os << ",\n";
    os << '{' << std::quoted( "player_type" ) << ':'
       << '{';

    os << std::quoted( "id" ) << ':' << nstohi( type.id ) << ','
       << std::quoted( "player_speed_max" ) << ':' << quantize( nltohd( type.player_speed_max ) ) << ','
       << std::quoted( "stamina_inc_max" ) << ':' << quantize( nltohd( type.stamina_inc_max ) ) << ','
       << std::quoted( "player_decay" ) << ':' << quantize( nltohd( type.player_decay ) ) << ','
       << std::quoted( "inertia_moment" ) << ':' << quantize( nltohd( type.inertia_moment ) ) << ','
       << std::quoted( "dash_power_rate" ) << ':' << quantize( nltohd( type.dash_power_rate ) ) << ','
       << std::quoted( "player_size" ) << ':' << quantize( nltohd( type.player_size ) ) << ','
       << std::quoted( "kickable_margin" ) << ':' << quantize( nltohd( type.kickable_margin ) ) << ','
       << std::quoted( "kick_rand" ) << ':' << quantize( nltohd( type.kick_rand ) ) << ','
       << std::quoted( "extra_stamina" ) << ':' << quantize( nltohd( type.extra_stamina ) ) << ','
       << std::quoted( "effort_max" ) << ':' << quantize( nltohd( type.effort_max ) ) << ','
       << std::quoted( "effort_min" ) << ':' << quantize( nltohd( type.effort_min ) );

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
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
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const showinfo_t & show )
{
    M_time = static_cast< Int32 >( ntohs( show.time ) );

    if ( M_playmode != show.pmode )
    {
        M_playmode = show.pmode;

        serialize( os, show.pmode );
    }

    if ( M_teams[0].name_.length() != std::strlen( show.team[0].name )
         || M_teams[0].name_ != show.team[0].name
         || M_teams[0].score_ != ntohs( show.team[0].score )
         || M_teams[1].name_.length() != std::strlen( show.team[1].name )
         || M_teams[1].name_ != show.team[1].name
         || M_teams[1].score_ != ntohs( show.team[1].score ) )
    {
        convert( show.team[0], M_teams[0] );
        convert( show.team[1], M_teams[1] );

        serialize( os, show.team[0], show.team[1] );
    }

    ShowInfoT new_show;

    convert( show, new_show );

    return serialize( os, new_show );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const showinfo_t2 & show2 )
{
    M_time = static_cast< Int32 >( ntohs( show2.time ) );

    if ( M_playmode != show2.pmode )
    {
        M_playmode = show2.pmode;

        serialize( os, show2.pmode );
    }

    if ( M_teams[0].name_.length() != std::strlen( show2.team[0].name )
         || M_teams[0].name_ != show2.team[0].name
         || M_teams[0].score_ != ntohs( show2.team[0].score )
         || M_teams[1].name_.length() != std::strlen( show2.team[1].name )
         || M_teams[1].name_ != show2.team[1].name
         || M_teams[1].score_ != ntohs( show2.team[1].score ) )
    {
        convert( show2.team[0], M_teams[0] );
        convert( show2.team[1], M_teams[1] );

        serialize( os, show2.team[0], show2.team[1] );
    }

    ShowInfoT new_show;

    convert( show2, new_show );

    return serialize( os, new_show );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const short_showinfo_t2 & show2 )
{
    M_time = static_cast< Int32 >( ntohs( show2.time ) );

    ShowInfoT new_show;

    convert( show2, new_show );

    return serialize( os, new_show );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const msginfo_t & msg )
{
    os << ",\n";
    os << '{' << std::quoted( "msg" ) << ':'
       << '{';

    os << std::quoted( "time" ) << ':' << M_time << ',';
    if ( M_stime > 0 )
    {
        os << std::quoted( "stime" ) << ':' << M_stime << ',';
    }
    os << std::quoted( "board" ) << ':' << ntohs( msg.board ) << ',';
    os << std::quoted( "message") << ':' << std::quoted( msg.message );

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const Int16 board,
                           const std::string & msg )
{
    os << ",\n";
    os << '{' << std::quoted( "msg" ) << ':'
       << '{';

    os << std::quoted( "time" ) << ':' << M_time << ',';
    if ( M_stime > 0 )
    {
        os << std::quoted( "stime" ) << ':' << M_stime << ',';
    }
    os << std::quoted( "board" ) << ':' << ntohs( board ) << ',';
    os << std::quoted( "message") << ':' << std::quoted( msg );

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const drawinfo_t & /*draw*/ )
{
    // TODO
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const char playmode )
{
    static const char * playmode_strings[] = PLAYMODE_STRINGS;

    M_playmode = playmode;

    PlayMode pm = static_cast< PlayMode >( playmode );
    if ( pm < PM_Null || PM_MAX <= pm )
    {
        return os;
    }

    os << ",\n";
    os << '{' << std::quoted( "playmode" ) << ':'
       << '{';

    os << std::quoted( "time" ) << ':' << M_time << ',';
    if ( M_stime > 0 )
    {
        os << std::quoted( "stime" ) << ':' << M_stime << ',';
    }
    os << std::quoted( "mode" ) << ':' << std::quoted( playmode_strings[pm] );

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const team_t & team_l,
                           const team_t & team_r )
{
    convert( team_l, M_teams[0] );
    convert( team_r, M_teams[1] );

    return serialize( os, M_teams[0], M_teams[1] );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const TeamT & team_l,
                           const TeamT & team_r )
{
    M_teams[0] = team_l;
    M_teams[1] = team_r;


    os << ",\n";
    os << '{' << std::quoted( "team" ) << ':'
       << '{';

    os << std::quoted( "time" ) << ':' << M_time << ',';
    if ( M_stime > 0 )
    {
        os << std::quoted( "stime" ) << ':' << M_stime << ',';
    }

    // left
    os << std::quoted( "l" ) << ':' << '{';
    os << std::quoted( "name" ) << ':';
    if ( team_l.name_.empty() )
    {
        os << "null";
    }
    else
    {
        os << std::quoted( team_l.name_ );
    }
    os << ',' << std::quoted( "score" ) << ':' << team_l.score_;
    if ( team_l.penaltyTrial() > 0 || team_r.penaltyTrial() > 0 )
    {
        os << ',' << std::quoted( "pen_score" ) << ':' << team_l.pen_score_;
        os << ',' << std::quoted( "pen_miss" ) << ':' << team_l.pen_miss_;
    }
    os << '}';

    // right
    os << std::quoted( "r" ) << ':' << '{';
    os << std::quoted( "name" ) << ':';
    if ( team_r.name_.empty() )
    {
        os << "null";
    }
    else
    {
        os << std::quoted( team_r.name_ );
    }
    os << ',' << std::quoted( "score" ) << ':' << team_r.score_;
    if ( team_l.penaltyTrial() > 0 || team_r.penaltyTrial() > 0 )
    {
        os << ',' << std::quoted( "pen_score" ) << ':' << team_r.pen_score_;
        os << ',' << std::quoted( "pen_miss" ) << ':' << team_r.pen_miss_;
    }
    os << '}';

    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const ShowInfoT & show )
{
    os << ",\n";
    os << '{' << std::quoted( "show" ) << ':'
       << '{';

    os << std::quoted( "time" ) << ':' << show.time_;
    if ( show.stime_ > 0 )
    {
        os << ',' << std::quoted( "stime" ) << ':' << show.stime_;
    }

    // ball
    os << ',';
    os << std::quoted( "ball" ) << ':' << '{';
    os << std::quoted( "x" ) << ':' << show.ball_.x_;
    os << std::quoted( "y" ) << ':' << show.ball_.y_;
    if ( show.ball_.hasVelocity() )
    {
        os << std::quoted( "vx" ) << ':' << show.ball_.vx_;
        os << std::quoted( "vy" ) << ':' << show.ball_.vy_;
    }
    os << '}';

    // players
    os << ',';
    os << std::quoted( "players" ) << ':' << '[';
    for ( int i = 0; i < MAX_PLAYER*2; ++i )
    {
        const PlayerT & p = show.player_[i];

        if ( i > 0 ) os << ',';
        // begin
        os << '{' << std::quoted( "side" ) << ':' << '"' << p.side_ << '"'
           << ',' << std::quoted( "unum" ) << ':' << p.unum_
           << ',' << std::quoted( "type" ) << ':' << p.type_
           << ',' << std::quoted( "state" ) << ':' << p.state_;
        // pos
        os << ',' << std::quoted( "x" ) << ':' << quantize( p.x_, POS_PREC )
           << ',' << std::quoted( "y" ) << ':' << quantize( p.y_, POS_PREC )
           << ',' << std::quoted( "vx" ) << ':' << quantize( p.vx_, POS_PREC )
           << ',' << std::quoted( "vy" ) << ':' << quantize( p.vy_, POS_PREC )
           << ',' << std::quoted( "body" ) << ':' << quantize( p.body_, DIR_PREC )
           << ',' << std::quoted( "neck" ) << ':' << quantize( p.neck_, DIR_PREC );
        // arm
        if ( p.hasArm() )
        {
            os << ',' << std::quoted( "px" ) << ':' << quantize( p.point_x_, POS_PREC )
               << ',' << std::quoted( "py" ) << ':' << quantize( p.point_y_, POS_PREC );
        }
        // view mode
        os << ',' << std::quoted( "vq" ) << ':' << std::quoted( p.highQuality() ? "h" : "l" )
           << ',' << std::quoted( "vw" ) << ':' << quantize( p.view_width_, DIR_PREC );
        // focus point
        os << ',' << std::quoted( "fdist" ) << ':' << quantize( p.focus_dist_, POS_PREC )
           << ',' << std::quoted( "fdir" ) << ':' << quantize( p.focus_dir_, DIR_PREC );
        // stamina
        os << ',' << std::quoted( "stamina" ) << ':' << p.stamina_
           << ',' << std::quoted( "effort" ) << ':' << p.effort_
           << ',' << std::quoted( "recovery" ) << ':' << p.recovery_
           << ',' << std::quoted( "capacity" ) << ':' << p.stamina_capacity_;
        // focus
        if ( p.isFocusing() )
        {
            os << ',' << std::quoted( "fside" ) << ':' << '"' << p.focus_side_ << '"'
               << ',' << std::quoted( "fnum" ) << ':' << p.focus_unum_;
        }
        // count
        os << ',' << std::quoted( "kick" ) << ':' << p.kick_count_
           << ',' << std::quoted( "dash" ) << ':' << p.dash_count_
           << ',' << std::quoted( "turn" ) << ':' << p.turn_count_
           << ',' << std::quoted( "catch" ) << ':' << p.catch_count_
           << ',' << std::quoted( "move" ) << ':' << p.move_count_
           << ',' << std::quoted( "turn_neck" ) << ':' << p.turn_neck_count_
           << ',' << std::quoted( "change_view" ) << ':' << p.change_view_count_
           << ',' << std::quoted( "say" ) << ':' << p.say_count_
           << ',' << std::quoted( "tackle" ) << ':' << p.tackle_count_
           << ',' << std::quoted( "pointto" ) << ':' << p.pointto_count_
           << ',' << std::quoted( "attentionto" ) << ':' << p.attentionto_count_
           << ',' << std::quoted( "change_focus" ) << ':' << p.change_focus_count_;
        // end
        os << '}';
    }

    //
    os << '}';
    os << '}';
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const DispInfoT & disp )
{
    os << ",\n";

    char pm = static_cast< char >( disp.pmode_ );
    if ( pm != M_playmode )
    {
        serialize( os, pm );
    }

    if ( ! M_teams[0].equals( disp.team_[0] )
         || ! M_teams[1].equals( disp.team_[1] ) )
    {
        serialize( os, disp.team_[0], disp.team_[1] );
    }

    return serialize( os, disp.show_ );
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const ServerParamT & param )
{
    os << ",\n";
    return param.toJSON( os );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const PlayerParamT & param )
{
    os << ",\n";
    return param.toJSON( os );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const PlayerTypeT & param )
{
    os << ",\n";
    return param.toJSON( os );
}

/*-------------------------------------------------------------------*/
std::ostream &
SerializerJSON::serialize( std::ostream & os,
                           const SideID side,
                           const int x,
                           const int y,
                           const std::vector< std::string > & xpm )
{
    os << ",\n";
    os << '{' << std::quoted( "team_graphic" ) << ':';

    os << '{'; // begin body

    os << std::quoted( "side" ) << ':' << '"' << ( side == LEFT ? 'l' : side == RIGHT ? 'r' : 'n' ) << '"';

    os << ',' << std::quoted( "x" ) << ':' << x
       << ',' << std::quoted( "y" ) << ':' << y;

    os << ','; os << std::quoted( "xpm" ) << ':' << '['; // begin xpm array
    bool first = true;
    for ( const std::string & str : xpm )
    {
        if ( first ) first = false; else os << ',';
        os << std::quoted( str );
    }
    os << ']'; // end xpm array

    os << '}'; // end body
    os << '}'; // end team_graphic
    return os;
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
namespace {

Serializer::Ptr
create_json()
{
    Serializer::Ptr ptr( new SerializerJSON() );
    return ptr;
}

rcss::RegHolder vj = Serializer::creators().autoReg( &create_json, REC_VERSION_JSON );

}

}
}
