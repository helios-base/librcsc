// -*-c++-*-

/*!
  \file player_config.cpp
  \brief player configuration Source File
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

#include "player_config.h"

#include <rcsc/param/param_map.h>
#include <rcsc/param/param_parser.h>
#include <rcsc/types.h>

#include <iostream>
#include <cassert>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PlayerConfig::PlayerConfig()
    : M_param_map( new ParamMap( "Player options" ) )
{
    setDefaultParam();
    createParamMap();
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerConfig::~PlayerConfig()
{
    delete M_param_map;
    M_param_map = nullptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerConfig::setDefaultParam()
{
    // basic setting
    M_team_name = "HELIOS_base";
    M_version = 15;
    M_reconnect_number = 0;
    M_goalie = false;

    M_interval_msec = 10;
    M_server_wait_seconds = 5;

    M_wait_time_thr_synch_view = 30; //79;
    M_wait_time_thr_nosynch_view = 75;

    M_normal_view_time_thr = 15;

    M_rcssserver_host = "localhost";
    M_rcssserver_port = 6000;

    M_compression = -1;

    M_clang_min = 7;
    M_clang_max = 8;

    M_use_communication = true;
    M_hear_opponent_audio = false;
    M_audio_shift = 0;

    M_use_fullstate = true;
    M_debug_fullstate = false;

    M_synch_see = false;

    // accuracy threshold
    M_self_pos_count_thr = 20;
    M_self_vel_count_thr = 10;
    M_self_face_count_thr = 5;

    M_ball_pos_count_thr = 10;
    M_ball_rpos_count_thr = 5;
    M_ball_vel_count_thr = 10;

    M_player_pos_count_thr = 30;
    M_player_vel_count_thr = 5;
    M_player_face_count_thr = 2;

    // formation param
    M_player_number = 0;

    // configuration
    M_config_dir = "./";

    //
    // debug
    //

    M_debug = false;
    M_log_dir = "/tmp";

    //
    // debug server
    //
    M_debug_server_connect = false;
    M_debug_server_logging = false;
    M_debug_server_host = "localhost";
    M_debug_server_port = 6000 + 32;

    //
    // offline client
    //
    M_offline_logging = false;
    M_offline_log_ext = ".ocl";

    M_offline_client_number = Unum_Unknown;

    //
    // debug logging
    //
    M_debug_start_time = -1;
    M_debug_end_time = 99999999;

    M_debug_log_ext = ".log";

    M_debug_system = false;
    M_debug_sensor = false;
    M_debug_world = false;
    M_debug_action = false;
    M_debug_intercept = false;
    M_debug_kick = false;
    M_debug_hold = false;
    M_debug_dribble = false;
    M_debug_pass = false;
    M_debug_cross = false;
    M_debug_shoot = false;
    M_debug_clear = false;
    M_debug_block = false;
    M_debug_mark = false;
    M_debug_positioning = false;
    M_debug_role = false;
    M_debug_plan = false;
    M_debug_team = false;
    M_debug_communication = false;
    M_debug_analyzer = false;
    M_debug_action_chain = false;
    M_debug_training = false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerConfig::createParamMap()
{
    M_param_map->add()
        ( "team_name", "t", &M_team_name, "specifies team name string." )
        ( "version", "v", &M_version, "specifies the protocol version to communicate with the rcssserver." )
        ( "reconnect", "r", &M_reconnect_number,
          "recconect to the rcssserver. This option requres the player\'s uniform number for the reconnection." )

        ( "goalie", "g", BoolSwitch( &M_goalie ) )

        ( "interval_msec", "", &M_interval_msec )
        ( "server_wait_seconds", "", &M_server_wait_seconds )

        ( "wait_time_thr_synch_view", "", &M_wait_time_thr_synch_view )
        ( "wait_time_thr_nosynch_view","", &M_wait_time_thr_nosynch_view )

        ( "normal_view_time_thr", "", &M_normal_view_time_thr )

        ( "host", "h", &M_rcssserver_host )
        ( "port", "p", &M_rcssserver_port )

        ( "compression", "", &M_compression )

        ( "clang_min", "", &M_clang_min )
        ( "clang_max", "", &M_clang_max )

        ( "use_communication", "", &M_use_communication )
        ( "hear_opponent_audio", "", &M_hear_opponent_audio )
        ( "audio_shift", "", &M_audio_shift )

        ( "use_fullstate", "", &M_use_fullstate )
        ( "debug_fullstate", "", &M_debug_fullstate )
        ( "synch_see", "", &M_synch_see )

        ( "self_pos_count_thr", "", &M_self_pos_count_thr )
        ( "self_vel_count_thr", "", &M_self_vel_count_thr )
        ( "self_face_count_thr", "", &M_self_face_count_thr )
        ( "ball_pos_count_thr", "", &M_ball_pos_count_thr )
        ( "ball_rpos_count_thr", "", &M_ball_rpos_count_thr )
        ( "ball_vel_count_thr", "", &M_ball_vel_count_thr )
        ( "player_pos_count_thr", "", &M_player_pos_count_thr )
        ( "player_vel_count_thr", "", &M_player_vel_count_thr )
        ( "player_face_count_thr", "", &M_player_face_count_thr )

        ( "player_number", "n",  &M_player_number, "specifies the player's position number (not a uniform number)." )

        ( "config_dir", "", &M_config_dir )

        ( "debug", "", BoolSwitch( &M_debug ) )
        ( "log_dir", "", &M_log_dir )

        ( "debug_server_connect", "", BoolSwitch( &M_debug_server_connect ) )
        ( "debug_server_logging", "", BoolSwitch( &M_debug_server_logging ) )
        ( "debug_server_host", "", &M_debug_server_host )
        ( "debug_server_port", "", &M_debug_server_port )

        ( "offline_logging", "", BoolSwitch( &M_offline_logging ) )
        ( "offline_log_ext", "", &M_offline_log_ext )
        ( "offline_client_number", "", &M_offline_client_number )

        ( "debug_start_time", "", &M_debug_start_time )
        ( "debug_end_time", "", &M_debug_end_time )

        ( "debug_log_ext", "", &M_debug_log_ext )

        ( "debug_system", "", BoolSwitch( &M_debug_system ) )
        ( "debug_sensor", "", BoolSwitch( &M_debug_sensor ) )
        ( "debug_world", "", BoolSwitch( &M_debug_world ) )
        ( "debug_action", "", BoolSwitch( &M_debug_action ) )
        ( "debug_intercept", "", BoolSwitch( &M_debug_intercept ) )
        ( "debug_kick", "", BoolSwitch( &M_debug_kick ) )
        ( "debug_hold", "", BoolSwitch( &M_debug_hold ) )
        ( "debug_dribble", "", BoolSwitch( &M_debug_dribble ) )
        ( "debug_pass", "", BoolSwitch( &M_debug_pass ) )
        ( "debug_cross", "", BoolSwitch( &M_debug_cross ) )
        ( "debug_shoot", "", BoolSwitch( &M_debug_shoot ) )
        ( "debug_clear", "", BoolSwitch( &M_debug_clear ) )
        ( "debug_block", "", BoolSwitch( &M_debug_block ) )
        ( "debug_mark", "", BoolSwitch( &M_debug_mark ) )
        ( "debug_positioning", "", BoolSwitch( &M_debug_positioning ) )
        ( "debug_role", "", BoolSwitch( &M_debug_role ) )
        ( "debug_plan", "", BoolSwitch( &M_debug_plan ) )
        ( "debug_team", "", BoolSwitch( &M_debug_team ) )
        ( "debug_communication", "", BoolSwitch( &M_debug_communication ) )
        ( "debug_analyzer", "", BoolSwitch( &M_debug_analyzer ) )
        ( "debug_action_chain", "", BoolSwitch( &M_debug_action_chain ) )
        ( "debug_training", "", BoolSwitch( &M_debug_training ) )
        ;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerConfig::parse( ParamParser & parser )
{
    if ( M_param_map )
    {
        parser.parse( *M_param_map );
    }

    if ( M_offline_client_number < 1
         || 11 < M_offline_client_number )
    {
        M_offline_client_number = Unum_Unknown;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerConfig::printHelp( std::ostream & os ) const
{
    if ( M_param_map )
    {
        M_param_map->printHelp( os );
    }

    return os;
}

}
