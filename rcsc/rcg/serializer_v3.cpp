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
SerializerV3::serializeHeader( std::ostream & os )
{
    return serializeImpl( os, REC_VERSION_3 );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV3::serializeParam( std::ostream & os,
                              const std::string & msg )
{
    if ( ! msg.compare( 0, 14, "(server_param " ) )
    {
        server_params_t param;
        parseParam( msg, param );
        serializeImpl( os, param );
    }
    else if ( ! msg.compare( 0, 14, "(player_param " ) )
    {
        player_params_t pparam;
        parseParam( msg, pparam );
        serializeImpl( os, pparam );
    }
    else if ( ! msg.compare( 0, 13, "(player_type " ) )
    {
        player_type_t type;
        parseParam( msg, type );
        serializeImpl( os, type );
    }

    return os;
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
/*!

 */
bool
SerializerV3::parseParam( const std::string & msg,
                          std::map< std::string, std::string > & param_map )
{
    int n_read = 0;
    {

        char buf[32];
        if ( std::sscanf( msg.c_str(), " ( %31s %n ", buf, &n_read ) != 1 )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << ":error: failed to parse the message name."
                      << std::endl;
            return false;
        }
    }

    for ( std::string::size_type pos = msg.find_first_of( '(', n_read );
          pos != std::string::npos;
          pos = msg.find_first_of( '(', pos ) )
    {
        std::string::size_type end_pos = msg.find_first_of( ' ', pos );
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << ": error: failed to find parameter name."
                      << std::endl;
            return false;
        }

        pos += 1;
        const std::string name_str( msg, pos, end_pos - pos );

        pos = end_pos;
        // search end paren or double quatation
        end_pos = msg.find_first_of( ")\"", end_pos );
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << ": error: failed to parse parameter value for [" << name_str << "] "
                      << std::endl;
            return false;
        }

        // quated value is found
        if ( msg[end_pos] == '\"' )
        {
            pos = end_pos;
            end_pos = msg.find_first_of( '\"', end_pos + 1 );
            if ( end_pos == std::string::npos )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << ": error: failed to parse the quated value for ["
                          << name_str << "] "
                          << std::endl;
                return false;
            }
            end_pos += 1; // skip double quatation
        }
        else
        {
            pos += 1; // skip white space
        }

        const std::string value_str( msg, pos, end_pos - pos );

        param_map.insert( std::make_pair( name_str, value_str ) );

        pos = end_pos;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
namespace {
Int16
parse_bool( const std::string & value )
{
    if ( value == "true"
         || value == "on"
         || value == "1" )
    {
        return 1;
    }

    if ( value == "false"
         || value == "off"
         || value == "0" )
    {
        return 0;
    }

    return 0;
}

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SerializerV3::parseParam( const std::string & from,
                          server_params_t & to )
{
    std::map< std::string, std::string > param_map;
    if ( ! parseParam( from, param_map ) )
    {
        return false;
    }

    int i = 0;
    try
    {
        to.goal_width = hdtonl( std::stod( param_map["goal_width"] ) ); ++i;
        to.inertia_moment = hdtonl( std::stod( param_map["inertia_moment"] ) ); ++i;
        to.player_size = hdtonl( std::stod( param_map["player_size"] ) ); ++i;
        to.player_decay = hdtonl( std::stod( param_map["player_decay"] ) ); ++i;
        to.player_rand = hdtonl( std::stod( param_map["player_rand"] ) ); ++i;
        to.player_weight = hdtonl( std::stod( param_map["player_weight"] ) ); ++i;
        to.player_speed_max = hdtonl( std::stod( param_map["player_speed_max"] ) ); ++i;
        to.player_accel_max = hdtonl( std::stod( param_map["player_accel_max"] ) ); ++i;
        to.stamina_max = hdtonl( std::stod( param_map["stamina_max"] ) ); ++i;
        to.stamina_inc = hdtonl( std::stod( param_map["stamina_inc_max"] ) ); ++i;
        to.recover_init = hdtonl( std::stod( param_map["recover_init"] ) ); ++i;
        to.recover_dec_thr = hdtonl( std::stod( param_map["recover_dec_thr"] ) ); ++i;
        to.recover_min = hdtonl( std::stod( param_map["recover_min"] ) ); ++i;
        to.recover_dec = hdtonl( std::stod( param_map["recover_dec"] ) ); ++i;
        to.effort_init = hdtonl( std::stod( param_map["effort_init"] ) ); ++i;
        to.effort_dec_thr = hdtonl( std::stod( param_map["effort_dec_thr"] ) ); ++i;
        to.effort_min = hdtonl( std::stod( param_map["effort_min"] ) ); ++i;
        to.effort_dec = hdtonl( std::stod( param_map["effort_dec"] ) ); ++i;
        to.effort_inc_thr = hdtonl( std::stod( param_map["effort_inc_thr"] ) ); ++i;
        to.effort_inc = hdtonl( std::stod( param_map["effort_inc"] ) ); ++i;
        to.kick_rand = hdtonl( std::stod( param_map["kick_rand"] ) ); ++i;
        to.team_actuator_noise = hitons( parse_bool( param_map["team_actuator_noise"] ) ); ++i;
        to.player_rand_factor_l = hdtonl( std::stod( param_map["prand_factor_l"] ) ); ++i;
        to.player_rand_factor_r = hdtonl( std::stod( param_map["prand_factor_r"] ) ); ++i;
        to.kick_rand_factor_l = hdtonl( std::stod( param_map["kick_rand_factor_l"] ) ); ++i;
        to.kick_rand_factor_r = hdtonl( std::stod( param_map["kick_rand_factor_r"] ) ); ++i;
        to.ball_size = hdtonl( std::stod( param_map["ball_size"] ) ); ++i;
        to.ball_decay = hdtonl( std::stod( param_map["ball_decay"] ) ); ++i;
        to.ball_rand = hdtonl( std::stod( param_map["ball_rand"] ) ); ++i;
        to.ball_weight = hdtonl( std::stod( param_map["ball_weight"] ) ); ++i;
        to.ball_speed_max = hdtonl( std::stod( param_map["ball_speed_max"] ) ); ++i;
        to.ball_accel_max = hdtonl( std::stod( param_map["ball_accel_max"] ) ); ++i;
        to.dash_power_rate = hdtonl( std::stod( param_map["dash_power_rate"] ) ); ++i;
        to.kick_power_rate = hdtonl( std::stod( param_map["kick_power_rate"] ) ); ++i;
        to.kickable_margin = hdtonl( std::stod( param_map["kickable_margin"] ) ); ++i;
        to.control_radius = hdtonl( std::stod( param_map["control_radius"] ) ); ++i;
        //to.control_radius_width = hdtonl( std::stod( param_map["control_radius_width"] ) ); ++i;
        to.control_radius_width = hdtonl( 1.7 ); ++i;
        to.max_power = hdtonl( std::stod( param_map["maxpower"] ) ); ++i;
        to.min_power = hdtonl( std::stod( param_map["minpower"] ) ); ++i;
        to.max_moment = hdtonl( std::stod( param_map["maxmoment"] ) ); ++i;
        to.min_moment = hdtonl( std::stod( param_map["minmoment"] ) ); ++i;
        to.max_neck_moment = hdtonl( std::stod( param_map["maxneckmoment"] ) ); ++i;
        to.min_neck_moment = hdtonl( std::stod( param_map["minneckmoment"] ) ); ++i;
        to.max_neck_angle = hdtonl( std::stod( param_map["maxneckang"] ) ); ++i;
        to.min_neck_angle = hdtonl( std::stod( param_map["minneckang"] ) ); ++i;
        to.visible_angle = hdtonl( std::stod( param_map["visible_angle"] ) ); ++i;
        to.visible_distance = hdtonl( std::stod( param_map["visible_distance"] ) ); ++i;
        to.wind_dir = hdtonl( std::stod( param_map["wind_dir"] ) ); ++i;
        to.wind_force = hdtonl( std::stod( param_map["wind_force"] ) ); ++i;
        to.wind_ang = hdtonl( std::stod( param_map["wind_ang"] ) ); ++i;
        to.wind_rand = hdtonl( std::stod( param_map["wind_rand"] ) ); ++i;
        //to.kickable_area = hdtonl( std::stod( param_map["kickable_area"] ) ); ++i;
        to.kickable_area = hdtonl( 1.085 ); ++i;
        to.catch_area_l = hdtonl( std::stod( param_map["catchable_area_l"] ) ); ++i;
        to.catch_area_w = hdtonl( std::stod( param_map["catchable_area_w"] ) ); ++i;
        to.catch_probability = hdtonl( std::stod( param_map["catch_probability"] ) ); ++i;

        to.goalie_max_moves = hitons( std::stoi( param_map["goalie_max_moves"] ) ); ++i;

        to.corner_kick_margin = hdtonl( std::stod( param_map["ckick_margin"] ) ); ++i;
        to.offside_active_area = hdtonl( std::stod( param_map["offside_active_area_size"] ) ); ++i;

        to.wind_none = hitons( parse_bool( param_map["wind_none"] ) ); ++i;
        to.use_wind_random = hitons( parse_bool( param_map["wind_random"] ) ); ++i;
        to.coach_say_count_max = hitons( std::stoi( param_map["say_coach_cnt_max"] ) ); ++i;
        to.coach_say_msg_size = hitons( std::stoi( param_map["say_coach_msg_size"] ) ); ++i;
        to.clang_win_size = hitons( std::stoi( param_map["clang_win_size"] ) ); ++i;
        to.clang_define_win = hitons( std::stoi( param_map["clang_define_win"] ) ); ++i;
        to.clang_meta_win = hitons( std::stoi( param_map["clang_meta_win"] ) ); ++i;
        to.clang_advice_win = hitons( std::stoi( param_map["clang_advice_win"] ) ); ++i;
        to.clang_info_win = hitons( std::stoi( param_map["clang_info_win"] ) ); ++i;
        to.clang_mess_delay = hitons( std::stoi( param_map["clang_mess_delay"] ) ); ++i;
        to.clang_mess_per_cycle = hitons( std::stoi( param_map["clang_mess_per_cycle"] ) ); ++i;
        to.half_time = hitons( std::stoi( param_map["half_time"] ) ); ++i;
        to.simulator_step = hitons( std::stoi( param_map["simulator_step"] ) ); ++i;
        to.send_step = hitons( std::stoi( param_map["send_step"] ) ); ++i;
        to.recv_step = hitons( std::stoi( param_map["recv_step"] ) ); ++i;
        to.sense_body_step = hitons( std::stoi( param_map["sense_body_step"] ) ); ++i;
        //to.lcm_step = hitons( std::stoi( param_map["lcm_step"] ) ); ++i;
        to.lcm_step = hitons( 300 ); ++i;
        to.player_say_msg_size = hitons( std::stoi( param_map["say_msg_size"] ) ); ++i;
        to.player_hear_max = hitons( std::stoi( param_map["hear_max"] ) ); ++i;
        to.player_hear_inc = hitons( std::stoi( param_map["hear_inc"] ) ); ++i;
        to.player_hear_decay = hitons( std::stoi( param_map["hear_decay"] ) ); ++i;
        to.catch_ban_cycle = hitons( std::stoi( param_map["catch_ban_cycle"] ) ); ++i;
        to.slow_down_factor = hitons( std::stoi( param_map["slow_down_factor"] ) ); ++i;
        to.use_offside = hitons( parse_bool( param_map["use_offside"] ) ); ++i;
        to.kickoff_offside = hitons( parse_bool( param_map["forbid_kick_off_offside"] ) ); ++i;

        to.offside_kick_margin = hdtonl( std::stod( param_map["offside_kick_margin"] ) ); ++i;
        to.audio_cut_dist = hdtonl( std::stod( param_map["audio_cut_dist"] ) ); ++i;
        to.dist_quantize_step = hdtonl( std::stod( param_map["quantize_step"] ) ); ++i;
        to.landmark_dist_quantize_step = hdtonl( std::stod( param_map["quantize_step_l"] ) ); ++i;
        //to.dir_quantize_step = hdtonl( std::stod( param_map["quantize_step_dir"] ) ); ++i;
        to.dir_quantize_step = hdtonl( 0.0 ); ++i;
        //to.dist_quantize_step_l = hdtonl( std::stod( param_map["quantize_step_dist_team_l"] ) ); ++i;
        to.dist_quantize_step_l = hdtonl( 0.0 ); ++i;
        //to.dist_quantize_step_r = hdtonl( std::stod( param_map["quantize_step_dist_team_r"] ) ); ++i;
        to.dist_quantize_step_r = hdtonl( 0.0 ); ++i;
        //to.landmark_dist_quantize_step_l = hdtonl( std::stod( param_map["quantize_step_dist_l_team_l"] ) ); ++i;
        to.landmark_dist_quantize_step_l = hdtonl( 0.0 ); ++i;
        //to.landmark_dist_quantize_step_l = hdtonl( std::stod( param_map["quantize_step_dist_l_team_r"] ) ); ++i;
        to.landmark_dist_quantize_step_l = hdtonl( 0.0 ); ++i;
        //to.dir_quantize_step_l = hdtonl( std::stod( param_map["quantize_step_dir_team_l"] ) ); ++i;
        to.dir_quantize_step_l = hdtonl( 0.0 ); ++i;
        //to.dir_quantize_step_r = hdtonl( std::stod( param_map["quantize_step_dir_team_r"] ) ); ++i;
        to.dir_quantize_step_r = hdtonl( 0.0 ); ++i;

        to.coach_mode = hitons( parse_bool( param_map["coach"] ) ); ++i;
        to.coach_with_referee_mode = hitons( parse_bool( param_map["coach_w_referee"] ) ); ++i;
        to.use_old_coach_hear = hitons( parse_bool( param_map["old_coach_hear"] ) ); ++i;
        to.online_coach_look_step = hitons( std::stoi( param_map["send_vi_step"] ) ); ++i;

        to.slowness_on_top_for_left_team = hdtonl( std::stod( param_map["slowness_on_top_for_left_team"] ) ); ++i;
        to.slowness_on_top_for_right_team = hdtonl( std::stod( param_map["slowness_on_top_for_right_team"] ) ); ++i;
        to.ka_length = hdtonl( std::stod( param_map["keepaway_length"] ) ); ++i;
        to.ka_width = hdtonl( std::stod( param_map["keepaway_width"] ) ); ++i;
        to.ball_stuck_area = hdtonl( std::stod( param_map["ball_stuck_area"] ) ); ++i;
        to.max_tackle_power = hdtonl( std::stod( param_map["max_tackle_power"] ) ); ++i;
        to.max_back_tackle_power = hdtonl( std::stod( param_map["max_back_tackle_power"] ) ); ++i;
        to.tackle_dist = hdtonl( std::stod( param_map["tackle_dist"] ) ); ++i;
        to.tackle_back_dist = hdtonl( std::stod( param_map["tackle_back_dist"] ) ); ++i;
        to.tackle_width = hdtonl( std::stod( param_map["tackle_width"] ) ); ++i;

        to.start_goal_l = hitons( std::stoi( param_map["start_goal_l"] ) ); ++i;
        to.start_goal_r = hitons( std::stoi( param_map["start_goal_r"] ) ); ++i;
        to.fullstate_l = hitons( parse_bool( param_map["fullstate_l"] ) ); ++i;
        to.fullstate_r = hitons( parse_bool( param_map["fullstate_r"] ) ); ++i;
        to.drop_ball_time = hitons( std::stoi( param_map["drop_ball_time"] ) ); ++i;
        to.synch_mode = hitons( parse_bool( param_map["synch_mode"] ) ); ++i;
        to.synch_offset = hitons( std::stoi( param_map["synch_offset"] ) ); ++i;
        to.synch_micro_sleep = hitons( std::stoi( param_map["synch_micro_sleep"] ) ); ++i;
        to.point_to_ban = hitons( std::stoi( param_map["point_to_ban"] ) ); ++i;
        to.point_to_duration = hitons( std::stoi( param_map["point_to_duration"] ) ); ++i;

    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ':' << __LINE__ << ' ' << e.what() << '\n'
                  << " failed to convert server_param message \n"
                  << " parameter index = " << i << '\n'
                  << from
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SerializerV3::parseParam( const std::string & from,
                          player_params_t & to )
{
    std::map< std::string, std::string > param_map;
    if ( ! parseParam( from, param_map ) )
    {
        return false;
    }

    int i = 0;
    try
    {
        to.player_types = hitons( std::stoi( param_map["player_types"] ) ); ++i;
        to.substitute_max = hitons( std::stoi( param_map["subs_max"] ) ); ++i;
        to.pt_max = hitons( std::stoi( param_map["pt_max"] ) ); ++i;
        to.player_speed_max_delta_min = hdtonl( std::stod( param_map["player_speed_max_delta_min"] ) ); ++i;
        to.player_speed_max_delta_max = hdtonl( std::stod( param_map["player_speed_max_delta_max"] ) ); ++i;
        to.stamina_inc_max_delta_factor = hdtonl( std::stod( param_map["stamina_inc_max_delta_factor"] ) ); ++i;
        to.player_decay_delta_min = hdtonl( std::stod( param_map["player_decay_delta_min"] ) ); ++i;
        to.player_decay_delta_max = hdtonl( std::stod( param_map["player_decay_delta_max"] ) ); ++i;
        to.inertia_moment_delta_factor = hdtonl( std::stod( param_map["inertia_moment_delta_factor"] ) ); ++i;
        to.dash_power_rate_delta_min = hdtonl( std::stod( param_map["dash_power_rate_delta_min"] ) ); ++i;
        to.dash_power_rate_delta_max = hdtonl( std::stod( param_map["dash_power_rate_delta_max"] ) ); ++i;
        to.player_size_delta_factor = hdtonl( std::stod( param_map["player_size_delta_factor"] ) ); ++i;
        to.kickable_margin_delta_min = hdtonl( std::stod( param_map["kickable_margin_delta_min"] ) ); ++i;
        to.kickable_margin_delta_max = hdtonl( std::stod( param_map["kickable_margin_delta_max"] ) ); ++i;
        to.kick_rand_delta_factor = hdtonl( std::stod( param_map["kick_rand_delta_factor"] ) ); ++i;
        to.extra_stamina_delta_min = hdtonl( std::stod( param_map["extra_stamina_delta_min"] ) ); ++i;
        to.extra_stamina_delta_max = hdtonl( std::stod( param_map["extra_stamina_delta_max"] ) ); ++i;
        to.effort_max_delta_factor = hdtonl( std::stod( param_map["effort_max_delta_factor"] ) ); ++i;
        to.effort_min_delta_factor = hdtonl( std::stod( param_map["effort_min_delta_factor"] ) ); ++i;
        to.random_seed = htonl( std::stol( param_map["random_seed"] ) ); ++i;
        to.new_dash_power_rate_delta_min = hdtonl( std::stod( param_map["new_dash_power_rate_delta_min"] ) ); ++i;
        to.new_dash_power_rate_delta_max = hdtonl( std::stod( param_map["new_dash_power_rate_delta_max"] ) ); ++i;
        to.new_stamina_inc_max_delta_factor = hdtonl( std::stod( param_map["new_stamina_inc_max_delta_factor"] ) ); ++i;
        to.allow_mult_default_type = hitons( parse_bool( param_map["allow_mult_default_type"] ) ); ++i;
    }
    catch ( std::exception & e )
    {
        std::cerr << e.what() << '\n'
                  << __FILE__ << ':' << __LINE__ << ':'
                  << " failed to convert player_param message \n"
                  << " parameter index = " << i << '\n'
                  << from
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SerializerV3::parseParam( const std::string & from,
                          player_type_t & to )
{
    std::map< std::string, std::string > param_map;
    if ( ! parseParam( from, param_map ) )
    {
        return false;
    }

    int i = 0;
    try
    {
        to.id = hitons( std::stoi( param_map["id"] ) ); ++i;
        to.player_speed_max = hdtonl( std::stod( param_map["player_speed_max"] ) ); ++i;
        to.stamina_inc_max = hdtonl( std::stod( param_map["stamina_inc_max"] ) ); ++i;
        to.player_decay = hdtonl( std::stod( param_map["player_decay"] ) ); ++i;
        to.inertia_moment = hdtonl( std::stod( param_map["inertia_moment"] ) ); ++i;
        to.dash_power_rate = hdtonl( std::stod( param_map["dash_power_rate"] ) ); ++i;
        to.player_size = hdtonl( std::stod( param_map["player_size"] ) ); ++i;
        to.kickable_margin = hdtonl( std::stod( param_map["kickable_margin"] ) ); ++i;
        to.kick_rand = hdtonl( std::stod( param_map["kick_rand"] ) ); ++i;
        to.extra_stamina = hdtonl( std::stod( param_map["extra_stamina"] ) ); ++i;
        to.effort_max = hdtonl( std::stod( param_map["effort_max"] ) ); ++i;
        to.effort_min = hdtonl( std::stod( param_map["effort_min"] ) ); ++i;
    }
    catch ( std::exception & e )
    {
        std::cerr << e.what() << '\n'
                  << __FILE__ << ':' << __LINE__
                  << " failed to convert player_type message \n"
                  << " parameter index = " << i << '\n'
                  << from
                  << std::endl;
        return false;
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

*/
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
