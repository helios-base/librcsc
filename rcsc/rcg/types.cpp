// -*-c++-*-

/*!
  \file types.cpp
  \brief Type definitions for rcg data format.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

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

#include "types.h"

#include "util.h"

#include <map>
#include <iostream>
#include <iomanip>
#include <cstring>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace rcsc {
namespace rcg {

namespace {

/*-------------------------------------------------------------------*/
inline
float
quantize( const double val,
          const double prec )
{
    return rint( val / prec ) * prec;
}

/*-------------------------------------------------------------------*/
template < typename T >
void
to_sexp( std::ostream & os,
         const char * name,
         const T & value )
{
    os << '(' << name << ' ' << value << ')';
}

/*-------------------------------------------------------------------*/
// template <>
// void
// to_sexp< std::string >( std::ostream & os,
//                         const char * name,
//                         const std::string & value )
// {
//     os << '(' << name << ' ' << std::quoted( value ) << ')';
// }

/*-------------------------------------------------------------------*/
std::string
clean_string( std::string str )
{
    if( str.empty() )
    {
        return str;
    }

    if ( *str.begin() == '\'' )
    {
        if( *str.rbegin() == '\''  )
        {
            str = str.substr( 1, str.length() - 2 );
        }
        else
        {
            return str;
        }

        // replace escape character
        for ( std::string::size_type escape_pos = str.find( "\\'" );
              escape_pos != std::string::npos;
              escape_pos = str.find( "\\'" ) )
        {
            // replace "\'" with "'"
            str.replace( escape_pos, 2, "'" );
        }
    }
    else if ( *str.begin() == '"' )
    {
        if ( *str.rbegin() == '"'  )
        {
            str = str.substr( 1, str.length() - 2 );
        }
        else
        {
            return str;
        }

        // replace escape character
        for( std::string::size_type escape_pos = str.find( "\\\"" );
             escape_pos != std::string::npos;
             escape_pos = str.find( "\\\"" ) )
        {
            // replace "\"" with """
            str.replace( escape_pos, 2, "\"" );
        }
    }

    return str;
}

/*-------------------------------------------------------------------*/
bool
set_value( const std::string & name,
           const std::string & value,
           ParamMap & param_map )
{
    ParamMap::iterator it = param_map.find( name );
    if ( it != param_map.end() )
    {
        int ** int_ptr = std::get_if< int * >( &it->second );
        if ( int_ptr )
        {
            try
            {
                **int_ptr = std::stoi( value );
                return true;
            }
            catch ( std::exception & e )
            {
                std::cerr << e.what()
                          << " name=" << name << " value=" << value << std::endl;
                return false;
            }
        }

        double ** double_ptr = std::get_if< double * >( &it->second );
        if ( double_ptr )
        {
            try
            {
                **double_ptr = std::stod( value );
            }
            catch ( std::exception & e )
            {
                std::cerr << e.what()
                          << " name=" << name << " value=" << value << std::endl;
                return false;
            }
            return true;
        }

        bool ** bool_ptr = std::get_if< bool * >( &it->second );
        if ( bool_ptr )
        {
            if ( value == "0" || value == "false" || value == "off" )
            {
                **bool_ptr = false;
            }
            else if ( value == "1" || value == "true" || value == "on" )
            {
                **bool_ptr = true;
            }
            else
            {
                std::cerr << "Unknown bool value. name=" << name << " value=" << value << std::endl;
            }
            return true;
        }

        std::string ** string_ptr = std::get_if< std::string * >( &it->second );
        if ( string_ptr )
        {
            **string_ptr = clean_string( value );
            return true;
        }
    }

    std::cerr << "Unsupported parameter. name=" << name << " value=" << value << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
bool
set_integer( const std::string & name,
             const int value,
             ParamMap & param_map )
{
    ParamMap::iterator it = param_map.find( name );
    if ( it != param_map.end() )
    {
        int ** int_ptr = std::get_if< int * >( &it->second );
        if ( int_ptr )
        {
            **int_ptr = value;
            return true;
        }

        double ** double_ptr = std::get_if< double * >( &it->second );
        if ( double_ptr )
        {
            **double_ptr = static_cast< double >( value );
            return true;
        }

        bool ** bool_ptr = std::get_if< bool * >( &it->second );
        if ( bool_ptr )
        {
            **bool_ptr = ( value == 0 ? false : true );
            return true;
        }
    }

    std::cerr << "Unsupported parameter. name=" << name << " value=" << value << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
bool
set_double( const std::string & name,
            const double value,
            ParamMap & param_map )
{
    ParamMap::iterator it = param_map.find( name );
    if ( it != param_map.end() )
    {
        double ** double_ptr = std::get_if< double * >( &it->second );
        if ( double_ptr )
        {
            **double_ptr = value;
            return true;
        }
    }

    std::cerr << "Unsupported parameter. name=" << name << " value=" << value << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
bool
set_boolean( const std::string & name,
             const bool value,
             ParamMap & param_map )
{
    ParamMap::iterator it = param_map.find( name );
    if ( it != param_map.end() )
    {
        bool ** bool_ptr = std::get_if< bool * >( &it->second );
        if ( bool_ptr )
        {
            **bool_ptr = value;
            return true;
        }
    }

    std::cerr << "Unsupported bool parameter. name=" << name << " value=" << value << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
bool
set_string( const std::string & name,
            const std::string & value,
            ParamMap & param_map )
{
    ParamMap::iterator it = param_map.find( name );
    if ( it != param_map.end() )
    {
        std::string ** string_ptr = std::get_if< std::string * >( &it->second );
        if ( string_ptr )
        {
            **string_ptr = value;
            return true;
        }
    }

    std::cerr << "Unsupported string parameter. name=" << name << " value=" << value << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
bool
parse_server_message( const std::string & msg,
                      ParamMap & param_map )
{
    int n_read = 0;

    //
    // reach message type
    //
    char message_name[32];
    if ( std::sscanf( msg.c_str(), " ( %31s %n ", message_name, &n_read ) != 1 )
    {
        std::cerr << __FILE__ << ": (parse_server_message) ERROR Failed to parse the message type [" << msg << "]" << std::endl;
        return false;
    }

    for ( std::string::size_type pos = msg.find_first_of( '(', n_read );
          pos != std::string::npos;
          pos = msg.find_first_of( '(', pos ) )
    {
        std::string::size_type end_pos = msg.find_first_of( ' ', pos );
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ": (parse_server_message) ERROR Could not find the seprator space. at " << pos
                      << " [" << msg << "]" << std::endl;
            return false;
        }
        pos += 1;

        const std::string name_str( msg, pos, end_pos - pos );

        pos = end_pos; // pos indcates the position of the white space after the param name

        // search end paren or double quatation
        end_pos = msg.find_first_of( ")\"", end_pos ); //"
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << " (parse_server_message) ERROR Could not find the parameter value for [" << name_str << ']' << std::endl;
            return false;
        }

        // found quated value
        if ( msg[end_pos] == '\"' )
        {
            pos = end_pos;
            end_pos = msg.find_first_of( '\"', end_pos + 1 ); //"
            if ( end_pos == std::string::npos )
            {
                std::cerr << __FILE__ << " (parse_server_message) ERROR Could not parse the quated value for [" << name_str << ']'
                          << std::endl;
                return false;
            }
            end_pos += 1; // skip double quatation
        }
        else
        {
            pos += 1; // skip the white space after the param name
        }

        // pos indicates the first position of the value string
        // end_pos indicates the position of the end of paren

        const std::string value_str( msg, pos, end_pos - pos );
        pos = end_pos;

        // pos indicates the position of the end of paren

        // set the value to the parameter map
        set_value( name_str, value_str, param_map );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!
  \brief visitor function to print the parameter variables stored as std::variant
 */
struct ValuePrinter {
    std::ostream & os_;

    ValuePrinter( std::ostream & os )
        : os_( os )
      { }

    std::ostream & operator()( const int * v )
      {
          return os_ << *v;
      }
    std::ostream & operator()( const double * v )
      {
          return os_ << *v;
      }
    std::ostream & operator()( const bool * v )
      {
          return os_ << std::boolalpha << *v;
      }
    std::ostream & operator()( const std::string * v )
      {
          return os_ << std::quoted( *v );
      }
};

/*-------------------------------------------------------------------*/
std::ostream &
print_server_message( std::ostream & os,
                      const std::string & message_name,
                      const ParamMap & param_map )
{
    std::map< ParamMap::key_type, ParamMap::mapped_type > sorted_map;
    for ( const ParamMap::value_type & v : param_map )
    {
        sorted_map.insert( v );
    }

    os << '(' << message_name << ' ';

    ValuePrinter printer( os );
    for ( const decltype( sorted_map )::value_type & v : sorted_map )
    {
        os << '(' << v.first << ' ';
        std::visit( printer, v.second );
        os << ')';
    }

    os << ')';

    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
print_json( std::ostream & os,
            const std::string & message_name,
            const ParamMap & param_map )
{
    std::map< ParamMap::key_type, ParamMap::mapped_type > sorted_map;
    for ( const ParamMap::value_type & v : param_map )
    {
        sorted_map.insert( v );
    }

    os << '{' << std::quoted( message_name ) << ':' << '{';

    ValuePrinter printer( os );
    bool first = true;
    for ( const decltype( sorted_map )::value_type & v : sorted_map )
    {
        if ( first ) first = false; else os << ',';
        os << std::quoted( v.first ) << ':';
        std::visit( printer, v.second );
    }

    os << '}' << '}';
    return os;
}

}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
TeamT::TeamT( const team_t & from )
    : TeamT()
{
    char buf[18];
    std::memset( buf, 0, 18 );
    std::strncpy( buf, from.name, sizeof( buf ) - 1 );

    name_ = buf;
    score_ = ntohs( from.score );
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
ServerParamT::ServerParamT()
    : goal_width_( 14.02 ),
      inertia_moment_( 5.0 ),
      player_size_( 0.3 ),
      player_decay_( 0.4 ),
      player_rand_( 0.1 ),
      player_weight_( 60.0 ),
      player_speed_max_( 1.2 ),
      player_accel_max_( 1.0 ),
      stamina_max_( 4000.0 ),
      stamina_inc_max_( 45.0 ),
      recover_init_( 1.0 ),
      recover_dec_thr_( 0.3 ),
      recover_min_( 0.5 ),
      recover_dec_( 0.002 ),
      effort_init_( 1.0 ),
      effort_dec_thr_( 0.3 ),
      effort_min_( 0.6 ),
      effort_dec_( 0.005 ),
      effort_inc_thr_( 0.6 ),
      effort_inc_( 0.01 ),
      kick_rand_( 0.1 ),
      team_actuator_noise_( false ),
      player_rand_factor_l_( 1.0 ),
      player_rand_factor_r_( 1.0 ),
      kick_rand_factor_l_( 1.0 ),
      kick_rand_factor_r_( 1.0 ),
      ball_size_( 0.085 ),
      ball_decay_( 0.94 ),
      ball_rand_( 0.05 ),
      ball_weight_( 0.2 ),
      ball_speed_max_( 3.0 ),
      ball_accel_max_( 2.7 ),
      dash_power_rate_( 0.006 ),
      kick_power_rate_( 0.027 ),
      kickable_margin_( 0.7 ),
      control_radius_( 2.0 ),
      //control_radius_width_( 1.7 ),
      catch_probability_( 1.0 ),
      catchable_area_l_( 1.2 ),
      catchable_area_w_( 1.0 ),
      goalie_max_moves_( 2 ),
      max_power_( 100.0 ),
      min_power_( -100.0 ),
      max_moment_( 180.0 ),
      min_moment_( -180.0 ),
      max_neck_moment_( 180.0 ),
      min_neck_moment_( -180.0 ),
      max_neck_angle_( 90.0 ),
      min_neck_angle_( -90.0 ),
      visible_angle_( 90.0 ),
      visible_distance_( 3.0 ),
      audio_cut_dist_( 50.0 ),
      dist_quantize_step_( 0.1 ),
      landmark_dist_quantize_step_( 0.01 ),
      corner_kick_margin_( 1.0 ),
      wind_dir_( 0.0 ),
      wind_force_( 0.0 ),
      wind_angle_( 0.0 ),
      wind_rand_( 0.0 ),
      wind_none_( false ),
      use_wind_random_( false ),
      half_time_( 300 ),
      drop_ball_time_( 200 ),
      port_( 6000 ),
      coach_port_( 6001 ),
      online_coach_port_( 6002 ),
      coach_say_count_max_( 128 ),
      coach_say_msg_size_( 128 ),
      simulator_step_( 100 ),
      send_step_( 150 ),
      recv_step_( 10 ),
      sense_body_step_( 100 ),
      player_say_msg_size_( 10 ),
      clang_win_size_( 300 ),
      clang_define_win_( 1 ),
      clang_meta_win_( 1 ),
      clang_advice_win_( 1 ),
      clang_info_win_( 1 ),
      clang_del_win_( 1 ),
      clang_rule_win_( 1 ),
      clang_mess_delay_( 50 ),
      clang_mess_per_cycle_( 1 ),
      player_hear_max_( 1 ),
      player_hear_inc_( 1 ),
      player_hear_decay_( 1 ),
      catch_ban_cycle_( 5 ),
      coach_mode_( false ),
      coach_with_referee_mode_( false ),
      use_old_coach_hear_( false ),
      online_coach_look_step_( 100 ),
      use_offside_( true ),
      offside_active_area_size_( 2.5 ),
      kickoff_offside_( true ),
      verbose_( false ),
      offside_kick_margin_( 9.15 ),
      slow_down_factor_( 1 ),
      synch_mode_( false ),
      synch_offset_( 60 ),
      synch_micro_sleep_( 1 ),
      start_goal_l_( 0 ),
      start_goal_r_( 0 ),
      fullstate_l_( false ),
      fullstate_r_( false ),
      slowness_on_top_for_left_team_( 1.0 ),
      slowness_on_top_for_right_team_( 1.0 ),
      landmark_file_(),
      send_comms_( false ),
      text_logging_( true ),
      game_logging_( true ),
      game_log_version_( 4 ),
      text_log_dir_( "." ),
      game_log_dir_( "." ),
      text_log_fixed_name_( "rcssserver" ),
      game_log_fixed_name_( "rcssserver" ),
      text_log_fixed_( false ),
      game_log_fixed_( false ),
      text_log_dated_( true ),
      game_log_dated_( true ),
      log_date_format_( "%Y%m%d%H%M-" ),
      log_times_( false ),
      record_messages_( false ),
      text_log_compression_( 0 ),
      game_log_compression_( 0 ),
      profile_( false ),
      point_to_ban_( 5 ),
      point_to_duration_( 20 ),
      tackle_dist_( 2.0 ),
      tackle_back_dist_( 0.5 ),
      tackle_width_( 1.0 ),
      tackle_exponent_( 6.0 ),
      tackle_cycles_( 10 ),
      tackle_power_rate_( 0.027 ),
      freeform_wait_period_( 600 ),
      freeform_send_period_( 20 ),
      free_kick_faults_( true ),
      back_passes_( true ),
      proper_goal_kicks_( false ),
      stopped_ball_vel_( 0.01 ),
      max_goal_kicks_( 3 ),
      auto_mode_( false ),
      kick_off_wait_( 100 ),
      connect_wait_( 300 ),
      game_over_wait_( 100 ),
      team_l_start_(),
      team_r_start_(),
      keepaway_mode_( false ),
      keepaway_length_( 20.0 ),
      keepaway_width_( 20.0 ),
      keepaway_logging_( true ),
      keepaway_log_dir_( "." ),
      keepaway_log_fixed_name_( "rcssserver" ),
      keepaway_log_fixed_( false ),
      keepaway_log_dated_( true ),
      keepaway_start_( -1 ),
      nr_normal_halfs_( 2 ),
      nr_extra_halfs_( 2 ),
      penalty_shoot_outs_( true ),
      pen_before_setup_wait_( 30 ),
      pen_setup_wait_( 100 ),
      pen_ready_wait_( 50 ),
      pen_taken_wait_( 200 ),
      pen_nr_kicks_( 5 ),
      pen_max_extra_kicks_( 10 ),
      pen_dist_x_( 42.5 ),
      pen_random_winner_( false ),
      pen_max_goalie_dist_x_( 14.0 ),
      pen_allow_mult_kicks_( true ),
      pen_coach_moves_players_( true ),
      ball_stuck_area_( 3.0 ),
      coach_msg_file_(),
      max_tackle_power_( 100.0 ),
      max_back_tackle_power_( 50.0 ),
      player_speed_max_min_( 0.8 ),
      extra_stamina_( 0.0 ),
      synch_see_offset_( 30 ),
      max_monitors_( -1 ),
      extra_half_time_( 300 ),
      stamina_capacity_( -1.0 ),
      max_dash_angle_( 0.0 ),
      min_dash_angle_( 0.0 ),
      dash_angle_step_( 90.0 ),
      side_dash_rate_( 0.25 ),
      back_dash_rate_( 0.5 ),
      max_dash_power_( 100.0 ),
      min_dash_power_( -100.0 ),
      tackle_rand_factor_( 1.0 ),
      foul_detect_probability_( 0.5 ),
      foul_exponent_( 10.0 ),
      foul_cycles_( 5 ),
      golden_goal_( true ),
      red_card_probability_( 0.0 ),
      illegal_defense_duration_( 20 ),
      illegal_defense_number_( 0 ),
      illegal_defense_dist_x_( 16.5 ),
      illegal_defense_width_( 40.32 ),
      fixed_teamname_l_( "" ),
      fixed_teamname_r_( "" ),
      max_catch_angle_( 180.0 ),
      min_catch_angle_( -180.0 )
      // dist_noise_rate_( 0.0125 ),
      // focus_dist_noise_rate_( 0.0125 ),
      // land_dist_noise_rate_( 0.00125 ),
      // land_focus_dist_noise_rate_( 0.00125 )
{
    param_map_.insert( ParamMap::value_type( "goal_width", &goal_width_ ) );
    param_map_.insert( ParamMap::value_type( "inertia_moment", &inertia_moment_ ) );
    param_map_.insert( ParamMap::value_type( "player_size", &player_size_ ) );
    param_map_.insert( ParamMap::value_type( "player_decay", &player_decay_ ) );
    param_map_.insert( ParamMap::value_type( "player_rand", &player_rand_ ) );
    param_map_.insert( ParamMap::value_type( "player_weight", &player_weight_ ) );
    param_map_.insert( ParamMap::value_type( "player_speed_max", &player_speed_max_ ) );
    param_map_.insert( ParamMap::value_type( "player_accel_max", &player_accel_max_ ) );
    param_map_.insert( ParamMap::value_type( "stamina_max", &stamina_max_ ) );
    param_map_.insert( ParamMap::value_type( "stamina_inc_max", &stamina_inc_max_ ) );
    param_map_.insert( ParamMap::value_type( "recover_init", &recover_init_ ) ); // not necessary
    param_map_.insert( ParamMap::value_type( "recover_dec_thr", &recover_dec_thr_ ) );
    param_map_.insert( ParamMap::value_type( "recover_min", &recover_min_ ) );
    param_map_.insert( ParamMap::value_type( "recover_dec", &recover_dec_ ) );
    param_map_.insert( ParamMap::value_type( "effort_init", &effort_init_ ) );
    param_map_.insert( ParamMap::value_type( "effort_dec_thr", &effort_dec_thr_ ) );
    param_map_.insert( ParamMap::value_type( "effort_min", &effort_min_ ) );
    param_map_.insert( ParamMap::value_type( "effort_dec", &effort_dec_ ) );
    param_map_.insert( ParamMap::value_type( "effort_inc_thr", &effort_inc_thr_ ) );
    param_map_.insert( ParamMap::value_type( "effort_inc",  &effort_inc_ ) );
    param_map_.insert( ParamMap::value_type( "kick_rand", &kick_rand_ ) );
    param_map_.insert( ParamMap::value_type( "team_actuator_noise", &team_actuator_noise_ ) );
    param_map_.insert( ParamMap::value_type( "prand_factor_l", &player_rand_factor_l_ ) );
    param_map_.insert( ParamMap::value_type( "prand_factor_r", &player_rand_factor_r_ ) );
    param_map_.insert( ParamMap::value_type( "kick_rand_factor_l", &kick_rand_factor_l_ ) );
    param_map_.insert( ParamMap::value_type( "kick_rand_factor_r", &kick_rand_factor_r_ ) );
    param_map_.insert( ParamMap::value_type( "ball_size", &ball_size_ ) );
    param_map_.insert( ParamMap::value_type( "ball_decay", &ball_decay_ ) );
    param_map_.insert( ParamMap::value_type( "ball_rand", &ball_rand_ ) );
    param_map_.insert( ParamMap::value_type( "ball_weight", &ball_weight_ ) );
    param_map_.insert( ParamMap::value_type( "ball_speed_max", &ball_speed_max_ ) );
    param_map_.insert( ParamMap::value_type( "ball_accel_max", &ball_accel_max_ ) );
    param_map_.insert( ParamMap::value_type( "dash_power_rate", &dash_power_rate_ ) );
    param_map_.insert( ParamMap::value_type( "kick_power_rate", &kick_power_rate_ ) );
    param_map_.insert( ParamMap::value_type( "kickable_margin", &kickable_margin_ ) );
    param_map_.insert( ParamMap::value_type( "control_radius", &control_radius_ ) );
    //( "control_radius_width", &control_radius_width_ ) );
    //( "kickable_area", &kickable_area_ ) ); // not needed
    param_map_.insert( ParamMap::value_type( "catch_probability", &catch_probability_ ) );
    param_map_.insert( ParamMap::value_type( "catchable_area_l", &catchable_area_l_ ) );
    param_map_.insert( ParamMap::value_type( "catchable_area_w", &catchable_area_w_ ) );
    param_map_.insert( ParamMap::value_type( "goalie_max_moves", &goalie_max_moves_ ) );
    param_map_.insert( ParamMap::value_type( "maxpower", &max_power_ ) );
    param_map_.insert( ParamMap::value_type( "minpower", &min_power_ ) );
    param_map_.insert( ParamMap::value_type( "maxmoment", &max_moment_ ) );
    param_map_.insert( ParamMap::value_type( "minmoment", &min_moment_ ) );
    param_map_.insert( ParamMap::value_type( "maxneckmoment", &max_neck_moment_ ) );
    param_map_.insert( ParamMap::value_type( "minneckmoment", &min_neck_moment_ ) );
    param_map_.insert( ParamMap::value_type( "maxneckang", &max_neck_angle_ ) );
    param_map_.insert( ParamMap::value_type( "minneckang", &min_neck_angle_ ) );
    param_map_.insert( ParamMap::value_type( "visible_angle", &visible_angle_ ) );
    param_map_.insert( ParamMap::value_type( "visible_distance", &visible_distance_ ) );
    param_map_.insert( ParamMap::value_type( "audio_cut_dist", &audio_cut_dist_ ) );
    param_map_.insert( ParamMap::value_type( "quantize_step", &dist_quantize_step_ ) );
    param_map_.insert( ParamMap::value_type( "quantize_step_l", &landmark_dist_quantize_step_ ) );
    //( "quantize_step_dir", &dir_quantize_step_ ) );
    //( "quantize_step_dist_team_l", &dist_quantize_step_l_ ) );
    //( "quantize_step_dist_team_r", &dist_quantize_step_r_ ) );
    //( "quantize_step_dist_l_team_l", &landmark_dist_quantize_step_l_ ) );
    //( "quantize_step_dist_l_team_r", &landmark_dist_quantize_step_r_ ) );
    //( "quantize_step_dir_team_l", &dir_quantize_step_l_ ) );
    //( "quantize_step_dir_team_r", &dir_quantize_step_r_ ) );
    param_map_.insert( ParamMap::value_type( "ckick_margin", &corner_kick_margin_ ) );
    param_map_.insert( ParamMap::value_type( "wind_dir", &wind_dir_ ) );
    param_map_.insert( ParamMap::value_type( "wind_force", &wind_force_ ) );
    param_map_.insert( ParamMap::value_type( "wind_ang", &wind_angle_ ) );
    param_map_.insert( ParamMap::value_type( "wind_rand", &wind_rand_ ) );
    param_map_.insert( ParamMap::value_type( "wind_none", &wind_none_ ) );
    param_map_.insert( ParamMap::value_type( "wind_random", &use_wind_random_ ) );
    param_map_.insert( ParamMap::value_type( "half_time", &half_time_ ) );
    param_map_.insert( ParamMap::value_type( "drop_ball_time", &drop_ball_time_ ) );
    param_map_.insert( ParamMap::value_type( "port", &port_ ) );
    param_map_.insert( ParamMap::value_type( "coach_port", &coach_port_ ) );
    param_map_.insert( ParamMap::value_type( "olcoach_port", &online_coach_port_ ) );
    param_map_.insert( ParamMap::value_type( "say_coach_cnt_max", &coach_say_count_max_ ) );
    param_map_.insert( ParamMap::value_type( "say_coach_msg_size", &coach_say_msg_size_ ) );
    param_map_.insert( ParamMap::value_type( "simulator_step", &simulator_step_ ) );
    param_map_.insert( ParamMap::value_type( "send_step", &send_step_ ) );
    param_map_.insert( ParamMap::value_type( "recv_step", &recv_step_ ) );
    param_map_.insert( ParamMap::value_type( "sense_body_step", &sense_body_step_ ) );
    //( "lcm_step", &lcm_step_ ) ); // not needed
    param_map_.insert( ParamMap::value_type( "say_msg_size", &player_say_msg_size_ ) );
    param_map_.insert( ParamMap::value_type( "clang_win_size", &clang_win_size_ ) );
    param_map_.insert( ParamMap::value_type( "clang_define_win", &clang_define_win_ ) );
    param_map_.insert( ParamMap::value_type( "clang_meta_win", &clang_meta_win_ ) );
    param_map_.insert( ParamMap::value_type( "clang_advice_win", &clang_advice_win_ ) );
    param_map_.insert( ParamMap::value_type( "clang_info_win", &clang_info_win_ ) );
    param_map_.insert( ParamMap::value_type( "clang_del_win", &clang_del_win_ ) );
    param_map_.insert( ParamMap::value_type( "clang_rule_win", &clang_rule_win_ ) );
    param_map_.insert( ParamMap::value_type( "clang_mess_delay", &clang_mess_delay_ ) );
    param_map_.insert( ParamMap::value_type( "clang_mess_per_cycle", &clang_mess_per_cycle_ ) );
    param_map_.insert( ParamMap::value_type( "hear_max", &player_hear_max_ ) );
    param_map_.insert( ParamMap::value_type( "hear_inc", &player_hear_inc_ ) );
    param_map_.insert( ParamMap::value_type( "hear_decay", &player_hear_decay_ ) );
    param_map_.insert( ParamMap::value_type( "catch_ban_cycle", &catch_ban_cycle_ ) );
    param_map_.insert( ParamMap::value_type( "coach", &coach_mode_ ) );
    param_map_.insert( ParamMap::value_type( "coach_w_referee", &coach_with_referee_mode_ ) );
    param_map_.insert( ParamMap::value_type( "old_coach_hear", &use_old_coach_hear_ ) );
    param_map_.insert( ParamMap::value_type( "send_vi_step", &online_coach_look_step_ ) );
    param_map_.insert( ParamMap::value_type( "use_offside", &use_offside_ ) );
    param_map_.insert( ParamMap::value_type( "offside_kick_margin", &offside_kick_margin_ ) );
    param_map_.insert( ParamMap::value_type( "forbid_kick_off_offside", &kickoff_offside_ ) );
    param_map_.insert( ParamMap::value_type( "verbose", &verbose_ ) );
    param_map_.insert( ParamMap::value_type( "offside_active_area_size", &offside_active_area_size_ ) );
    param_map_.insert( ParamMap::value_type( "slow_down_factor", &slow_down_factor_ ) );
    param_map_.insert( ParamMap::value_type( "synch_mode", &synch_mode_ ) );
    param_map_.insert( ParamMap::value_type( "synch_offset", &synch_offset_ ) );
    param_map_.insert( ParamMap::value_type( "synch_micro_sleep", &synch_micro_sleep_ ) );
    param_map_.insert( ParamMap::value_type( "start_goal_l", &start_goal_l_ ) );
    param_map_.insert( ParamMap::value_type( "start_goal_r", &start_goal_r_ ) );
    param_map_.insert( ParamMap::value_type( "fullstate_l", &fullstate_l_ ) );
    param_map_.insert( ParamMap::value_type( "fullstate_r", &fullstate_r_ ) );
    param_map_.insert( ParamMap::value_type( "slowness_on_top_for_left_team", &slowness_on_top_for_left_team_ ) );
    param_map_.insert( ParamMap::value_type( "slowness_on_top_for_right_team", &slowness_on_top_for_right_team_ ) );
    param_map_.insert( ParamMap::value_type( "landmark_file", &landmark_file_ ) );
    param_map_.insert( ParamMap::value_type( "send_comms", &send_comms_ ) );
    param_map_.insert( ParamMap::value_type( "text_logging", &text_logging_ ) );
    param_map_.insert( ParamMap::value_type( "game_logging", &game_logging_ ) );
    param_map_.insert( ParamMap::value_type( "game_log_version", &game_log_version_ ) );
    param_map_.insert( ParamMap::value_type( "text_log_dir", &text_log_dir_ ) );
    param_map_.insert( ParamMap::value_type( "game_log_dir", &game_log_dir_ ) );
    param_map_.insert( ParamMap::value_type( "text_log_fixed_name", &text_log_fixed_name_ ) );
    param_map_.insert( ParamMap::value_type( "game_log_fixed_name", &game_log_fixed_name_ ) );
    param_map_.insert( ParamMap::value_type( "text_log_fixed", &text_log_fixed_ ) );
    param_map_.insert( ParamMap::value_type( "game_log_fixed", &game_log_fixed_ ) );
    param_map_.insert( ParamMap::value_type( "text_log_dated", &text_log_dated_ ) );
    param_map_.insert( ParamMap::value_type( "game_log_dated", &game_log_dated_ ) );
    param_map_.insert( ParamMap::value_type( "log_date_format", &log_date_format_ ) );
    param_map_.insert( ParamMap::value_type( "log_times", &log_times_ ) );
    param_map_.insert( ParamMap::value_type( "record_messages", &record_messages_ ) );
    param_map_.insert( ParamMap::value_type( "text_log_compression", &text_log_compression_ ) );
    param_map_.insert( ParamMap::value_type( "game_log_compression", &game_log_compression_ ) );
    param_map_.insert( ParamMap::value_type( "profile", &profile_ ) );
    param_map_.insert( ParamMap::value_type( "point_to_ban", &point_to_ban_ ) );
    param_map_.insert( ParamMap::value_type( "point_to_duration", &point_to_duration_ ) );
    param_map_.insert( ParamMap::value_type( "tackle_dist", &tackle_dist_ ) );
    param_map_.insert( ParamMap::value_type( "tackle_back_dist", &tackle_back_dist_ ) );
    param_map_.insert( ParamMap::value_type( "tackle_width", &tackle_width_ ) );
    param_map_.insert( ParamMap::value_type( "tackle_exponent", &tackle_exponent_ ) );
    param_map_.insert( ParamMap::value_type( "tackle_cycles", &tackle_cycles_ ) );
    param_map_.insert( ParamMap::value_type( "tackle_power_rate", &tackle_power_rate_ ) );
    param_map_.insert( ParamMap::value_type( "freeform_wait_period", &freeform_wait_period_ ) );
    param_map_.insert( ParamMap::value_type( "freeform_send_period", &freeform_send_period_ ) );
    param_map_.insert( ParamMap::value_type( "free_kick_faults", &free_kick_faults_ ) );
    param_map_.insert( ParamMap::value_type( "back_passes", &back_passes_ ) );
    param_map_.insert( ParamMap::value_type( "proper_goal_kicks", &proper_goal_kicks_ ) );
    param_map_.insert( ParamMap::value_type( "stopped_ball_vel", &stopped_ball_vel_ ) );
    param_map_.insert( ParamMap::value_type( "max_goal_kicks", &max_goal_kicks_ ) );
    param_map_.insert( ParamMap::value_type( "auto_mode", &auto_mode_ ) );
    param_map_.insert( ParamMap::value_type( "kick_off_wait", &kick_off_wait_ ) );
    param_map_.insert( ParamMap::value_type( "connect_wait", &connect_wait_ ) );
    param_map_.insert( ParamMap::value_type( "game_over_wait", &game_over_wait_ ) );
    param_map_.insert( ParamMap::value_type( "team_l_start", &team_l_start_ ) );
    param_map_.insert( ParamMap::value_type( "team_r_start", &team_r_start_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway", &keepaway_mode_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_length", &keepaway_length_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_width", &keepaway_width_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_logging", &keepaway_logging_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_log_dir", &keepaway_log_dir_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_log_fixed_name", &keepaway_log_fixed_name_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_log_fixed", &keepaway_log_fixed_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_log_dated", &keepaway_log_dated_ ) );
    param_map_.insert( ParamMap::value_type( "keepaway_start", &keepaway_start_ ) );
    param_map_.insert( ParamMap::value_type( "nr_normal_halfs", &nr_normal_halfs_ ) );
    param_map_.insert( ParamMap::value_type( "nr_extra_halfs", &nr_extra_halfs_ ) );
    param_map_.insert( ParamMap::value_type( "penalty_shoot_outs", &penalty_shoot_outs_ ) );
    param_map_.insert( ParamMap::value_type( "pen_before_setup_wait", &pen_before_setup_wait_ ) );
    param_map_.insert( ParamMap::value_type( "pen_setup_wait", &pen_setup_wait_ ) );
    param_map_.insert( ParamMap::value_type( "pen_ready_wait", &pen_ready_wait_ ) );
    param_map_.insert( ParamMap::value_type( "pen_taken_wait", &pen_taken_wait_ ) );
    param_map_.insert( ParamMap::value_type( "pen_nr_kicks", &pen_nr_kicks_ ) );
    param_map_.insert( ParamMap::value_type( "pen_max_extra_kicks", &pen_max_extra_kicks_ ) );
    param_map_.insert( ParamMap::value_type( "pen_dist_x", &pen_dist_x_ ) );
    param_map_.insert( ParamMap::value_type( "pen_random_winner", &pen_random_winner_ ) );
    param_map_.insert( ParamMap::value_type( "pen_max_goalie_dist_x", &pen_max_goalie_dist_x_ ) );
    param_map_.insert( ParamMap::value_type( "pen_allow_mult_kicks", &pen_allow_mult_kicks_ ) );
    param_map_.insert( ParamMap::value_type( "pen_coach_moves_players", &pen_coach_moves_players_ ) );
    // v11
    param_map_.insert( ParamMap::value_type( "ball_stuck_area", &ball_stuck_area_ ) );
    param_map_.insert( ParamMap::value_type( "coach_msg_file", &coach_msg_file_ ) );
    // v12
    param_map_.insert( ParamMap::value_type( "max_tackle_power", &max_tackle_power_ ) );
    param_map_.insert( ParamMap::value_type( "max_back_tackle_power", &max_back_tackle_power_ ) );
    param_map_.insert( ParamMap::value_type( "player_speed_max_min", &player_speed_max_min_ ) );
    param_map_.insert( ParamMap::value_type( "extra_stamina", &extra_stamina_ ) );
    param_map_.insert( ParamMap::value_type( "synch_see_offset", &synch_see_offset_ ) );
    param_map_.insert( ParamMap::value_type( "max_monitors", &max_monitors_ ) );
    // v12.1.3
    param_map_.insert( ParamMap::value_type( "extra_half_time", &extra_half_time_ ) );
    // v13
    param_map_.insert( ParamMap::value_type( "stamina_capacity", &stamina_capacity_ ) );
    param_map_.insert( ParamMap::value_type( "max_dash_angle", &max_dash_angle_ ) );
    param_map_.insert( ParamMap::value_type( "min_dash_angle", &min_dash_angle_ ) );
    param_map_.insert( ParamMap::value_type( "dash_angle_step", &dash_angle_step_ ) );
    param_map_.insert( ParamMap::value_type( "side_dash_rate", &side_dash_rate_ ) );
    param_map_.insert( ParamMap::value_type( "back_dash_rate", &back_dash_rate_ ) );
    param_map_.insert( ParamMap::value_type( "max_dash_power", &max_dash_power_ ) );
    param_map_.insert( ParamMap::value_type( "min_dash_power", &min_dash_power_ ) );
    // 14.0.0
    param_map_.insert( ParamMap::value_type( "tackle_rand_factor", &tackle_rand_factor_ ) );
    param_map_.insert( ParamMap::value_type( "foul_detect_probability", &foul_detect_probability_ ) );
    param_map_.insert( ParamMap::value_type( "foul_exponent", &foul_exponent_ ) );
    param_map_.insert( ParamMap::value_type( "foul_cycles", &foul_cycles_ ) );
    param_map_.insert( ParamMap::value_type( "golden_goal", &golden_goal_ ) );
    // 15.0
    param_map_.insert( ParamMap::value_type( "red_card_probability", &red_card_probability_ ) );
    // 16.0
    param_map_.insert( ParamMap::value_type( "illegal_defense_duration", &illegal_defense_duration_ ) );
    param_map_.insert( ParamMap::value_type( "illegal_defense_number", &illegal_defense_number_ ) );
    param_map_.insert( ParamMap::value_type( "illegal_defense_dist_x", &illegal_defense_dist_x_ ) );
    param_map_.insert( ParamMap::value_type( "illegal_defense_width", &illegal_defense_width_ ) );
    param_map_.insert( ParamMap::value_type( "fixed_teamname_l", &fixed_teamname_l_ ) );
    param_map_.insert( ParamMap::value_type( "fixed_teamname_r", &fixed_teamname_r_ ) );
    // 17.0
    param_map_.insert( ParamMap::value_type( "max_catch_angle", &max_catch_angle_ ) );
    param_map_.insert( ParamMap::value_type( "min_catch_angle", &min_catch_angle_ ) );
    // 19.0
    // param_map_.insert( ParamMap::value_type( "dist_noise_rate", &dist_noise_rate_ ) );
    // param_map_.insert( ParamMap::value_type( "focus_dist_noise_rate", &focus_dist_noise_rate_ ) );
    // param_map_.insert( ParamMap::value_type( "land_dist_noise_rate", &land_dist_noise_rate_ ) );
    // param_map_.insert( ParamMap::value_type( "land_focus_dist_noise_rate", &land_focus_dist_noise_rate_ ) );
}

/*-------------------------------------------------------------------*/
std::ostream &
ServerParamT::toServerString( std::ostream & os ) const
{
    return print_server_message( os, "server_param", param_map_ );
}

/*-------------------------------------------------------------------*/
std::ostream &
ServerParamT::toJSON( std::ostream & os ) const
{
    return print_json( os, "server_param", param_map_ );
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::fromServerString( const std::string & msg )
{
    return parse_server_message( msg, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::fromStruct( const server_params_t & from )
{
    goal_width_ = nltohd( from.goal_width );
    inertia_moment_ = nltohd( from.inertia_moment );

    player_size_ = nltohd( from.player_size );
    player_decay_ = nltohd( from.player_decay );
    player_rand_ = nltohd( from.player_rand );
    player_weight_ = nltohd( from.player_weight );
    player_speed_max_ = nltohd( from.player_speed_max );
    player_accel_max_ = nltohd( from.player_accel_max );

    stamina_max_ = nltohd( from.stamina_max );
    stamina_inc_max_ = nltohd( from.stamina_inc );

    recover_init_ = nltohd( from.recover_init );
    recover_dec_thr_ = nltohd( from.recover_dec_thr );
    recover_min_ = nltohd( from.recover_min );
    recover_dec_ = nltohd( from.recover_dec );

    effort_init_ = nltohd( from.effort_init );
    effort_dec_thr_ = nltohd( from.effort_dec_thr );
    effort_min_ = nltohd( from.effort_min );
    effort_dec_ = nltohd( from.effort_dec );
    effort_inc_thr_ = nltohd( from.effort_inc_thr );
    effort_inc_ = nltohd( from.effort_inc );

    kick_rand_ = nltohd( from.kick_rand );
    team_actuator_noise_ = nstohb( from.team_actuator_noise );
    player_rand_factor_l_ = nltohd( from.player_rand_factor_l );
    player_rand_factor_r_ = nltohd( from.player_rand_factor_r );
    kick_rand_factor_l_ = nltohd( from.kick_rand_factor_l );
    kick_rand_factor_r_ = nltohd( from.kick_rand_factor_r );

    ball_size_ = nltohd( from.ball_size );
    ball_decay_ = nltohd( from.ball_decay );
    ball_rand_ = nltohd( from.ball_rand );
    ball_weight_ = nltohd( from.ball_weight );
    ball_speed_max_ = nltohd( from.ball_speed_max );
    ball_accel_max_ = nltohd( from.ball_accel_max );

    dash_power_rate_ = nltohd( from.dash_power_rate );
    kick_power_rate_ = nltohd( from.kick_power_rate );
    kickable_margin_ = nltohd( from.kickable_margin );
    control_radius_ = nltohd( from.control_radius );
    //control_radius_width_ = nltohd( from.control_radius_width );

    max_power_ = nltohd( from.max_power );
    min_power_ = nltohd( from.min_power );
    max_moment_ = nltohd( from.max_moment );
    min_moment_ = nltohd( from.min_moment );
    max_neck_moment_ = nltohd( from.max_neck_moment );
    min_neck_moment_ = nltohd( from.min_neck_moment );
    max_neck_angle_ = nltohd( from.max_neck_angle );
    min_neck_angle_ = nltohd( from.min_neck_angle );

    visible_angle_ = nltohd( from.visible_angle );
    visible_distance_ = nltohd( from.visible_distance );

    wind_dir_ = nltohd( from.wind_dir );
    wind_force_ = nltohd( from.wind_force );
    wind_angle_ = nltohd( from.wind_ang );
    wind_rand_ = nltohd( from.wind_rand );

    //kickable_area_ = nltohd( from.kickable_area );

    catchable_area_l_ = nltohd( from.catch_area_l );
    catchable_area_w_ = nltohd( from.catch_area_w );
    catch_probability_ = nltohd( from.catch_probability );
    goalie_max_moves_ = nstohi( from.goalie_max_moves );

    corner_kick_margin_ = nltohd( from.corner_kick_margin );
    offside_active_area_size_ = nltohd( from.offside_active_area );

    wind_none_ = nstohb( from.wind_none );
    use_wind_random_ = nstohb( from.use_wind_random );

    coach_say_count_max_ = nstohi( from.coach_say_count_max );
    coach_say_msg_size_ = nstohi( from.coach_say_msg_size );

    clang_win_size_ = nstohi( from.clang_win_size );
    clang_define_win_ = nstohi( from.clang_define_win );
    clang_meta_win_ = nstohi( from.clang_meta_win );
    clang_advice_win_ = nstohi( from.clang_advice_win );
    clang_info_win_ = nstohi( from.clang_info_win );
    clang_mess_delay_ = nstohi( from.clang_mess_delay );
    clang_mess_per_cycle_ = nstohi( from.clang_mess_per_cycle );

    half_time_ = nstohi( from.half_time );
    simulator_step_ = nstohi( from.simulator_step );
    send_step_ = nstohi( from.send_step );
    recv_step_ = nstohi( from.recv_step );
    sense_body_step_ = nstohi( from.sense_body_step );
    //lcm_step_ = nstohi( from.lcm_step )

    player_say_msg_size_ = nstohi( from.player_say_msg_size );
    player_hear_max_ = nstohi( from.player_hear_max );
    player_hear_inc_ = nstohi( from.player_hear_inc );
    player_hear_decay_ = nstohi( from.player_hear_decay );

    catch_ban_cycle_ = nstohi( from.catch_ban_cycle );

    slow_down_factor_ = nstohi( from.slow_down_factor );

    use_offside_ = nstohb( from.use_offside);
    kickoff_offside_ = nstohb( from.kickoff_offside );
    offside_kick_margin_ = nltohd( from.offside_kick_margin );

    audio_cut_dist_ = nltohd(from.audio_cut_dist );

    dist_quantize_step_ = nltohd( from.dist_quantize_step );
    landmark_dist_quantize_step_ = rcg::nltohd( from.landmark_dist_quantize_step );
    //nltohd( from.dir_quantize_step )
    //nltohd( from.dist_quantize_step_l )
    //nltohd( from.dist_quantize_step_r )
    //nltohd( from.landmark_dist_quantize_step_l )
    //nltohd( from.landmark_dist_quantize_step_r )
    //nltohd( from.dir_quantize_step_l )
    //nltohd( from.dir_quantize_step_r )

    coach_mode_ = nstohb( from.coach_mode );
    coach_with_referee_mode_ = nstohb( from.coach_with_referee_mode );
    use_old_coach_hear_ = nstohb( from.use_old_coach_hear );

    online_coach_look_step_ = nstohi( from.online_coach_look_step );

    slowness_on_top_for_left_team_ = nltohd( from.slowness_on_top_for_left_team );
    slowness_on_top_for_right_team_ = nltohd( from.slowness_on_top_for_right_team );

    keepaway_length_ = nltohd( from.ka_length );
    keepaway_width_ = nltohd( from.ka_width );;

    double tmp = 0.0;
    // 11.0.0
    tmp = nltohd( from.ball_stuck_area );
    if ( std::fabs( tmp ) < 1000.0 )
    {
        ball_stuck_area_ = tmp;;
    }

    // 12.0.0
    tmp = nltohd( from.max_tackle_power );
    if ( 0.0 < tmp && std::fabs( tmp ) < 200.0 )
    {
        max_tackle_power_ = tmp;;
    }

    tmp = nltohd( from.max_back_tackle_power );
    if ( 0.0 < tmp && std::fabs( tmp ) < 200.0 )
    {
        max_back_tackle_power_ = tmp;;
    }

    tmp = nltohd( from.tackle_dist );
    if ( 0.0 <= tmp && std::fabs( tmp ) < 3.0 )
    {
        tackle_dist_ = tmp;;
    }

    tmp = nltohd( from.tackle_back_dist );
    if ( 0.0 <= tmp && std::fabs( tmp ) < 1.0 )
    {
        tackle_back_dist_ = tmp;;
    }

    tmp = nltohd( from.tackle_width );
    if ( 0.0 < tmp && std::fabs( tmp ) < 2.0 )
    {
        tackle_width_ = tmp;;
    }

    start_goal_l_ = nstohi( from.start_goal_l );
    start_goal_r_ = nstohi( from.start_goal_r );

    fullstate_l_ = nstohb( from.fullstate_l );
    fullstate_r_ = nstohb( from.fullstate_r );

    drop_ball_time_ = nstohi( from.drop_ball_time );

    synch_mode_ = nstohb( from.synch_mode );
    synch_offset_ = nstohi( from.synch_offset );
    synch_micro_sleep_ = nstohi( from.synch_micro_sleep );

    point_to_ban_ = nstohi( from.point_to_ban );
    point_to_duration_ = nstohi( from.point_to_duration );

    return true;
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::setValue( const std::string & name,
                        const std::string & value )
{
    return set_value( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::setInt( const std::string & name,
                      const int value )
{
    return set_integer( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::setDouble( const std::string & name,
                         const double value )
{
    return set_double( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::setBool( const std::string & name,
                       const bool value )
{
    return set_boolean( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
ServerParamT::setString( const std::string & name,
                         const std::string & value )
{
    return set_string( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
PlayerParamT::PlayerParamT()
    : player_types_( 18 ),
      substitute_max_( 3 ),
      pt_max_( 1 ),
      allow_mult_default_type_( false ),
      player_speed_max_delta_min_( 0.0 ),
      player_speed_max_delta_max_( 0.0 ),
      stamina_inc_max_delta_factor_( 0.0 ),
      player_decay_delta_min_( -0.05 ),
      player_decay_delta_max_( 0.1 ),
      inertia_moment_delta_factor_( 25.0 ),
      dash_power_rate_delta_min_( 0.0 ),
      dash_power_rate_delta_max_( 0.0 ),
      player_size_delta_factor_( -100.0 ),
      kickable_margin_delta_min_( -0.1 ),
      kickable_margin_delta_max_( 0.1 ),
      kick_rand_delta_factor_( 1.0 ),
      extra_stamina_delta_min_( 0.0 ),
      extra_stamina_delta_max_( 100.0 ),
      effort_max_delta_factor_( -0.002 ),
      effort_min_delta_factor_( -0.002 ),
      new_dash_power_rate_delta_min_( -0.0005 ),
      new_dash_power_rate_delta_max_( 0.0015 ),
      new_stamina_inc_max_delta_factor_( -6000.0 ),
      random_seed_( -1 ),
      kick_power_rate_delta_min_( 0.0 ),
      kick_power_rate_delta_max_( 0.0 ),
      foul_detect_probability_delta_factor_( 0.0 ),
      catchable_area_l_stretch_min_( 0.0 ),
      catchable_area_l_stretch_max_( 0.0 )
{
    param_map_.insert( ParamMap::value_type( "player_types", &player_types_ ) );
    param_map_.insert( ParamMap::value_type( "subs_max", &substitute_max_ ) );
    param_map_.insert( ParamMap::value_type( "pt_max", &pt_max_ ) );
    param_map_.insert( ParamMap::value_type( "allow_mult_default_type", &allow_mult_default_type_ ) );
    param_map_.insert( ParamMap::value_type( "player_speed_max_delta_min", &player_speed_max_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "player_speed_max_delta_max", &player_speed_max_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "stamina_inc_max_delta_factor", &stamina_inc_max_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "player_decay_delta_min", &player_decay_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "player_decay_delta_max", &player_decay_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "inertia_moment_delta_factor", &inertia_moment_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "dash_power_rate_delta_min", &dash_power_rate_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "dash_power_rate_delta_max", &dash_power_rate_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "player_size_delta_factor", &player_size_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "kickable_margin_delta_min", &kickable_margin_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "kickable_margin_delta_max", &kickable_margin_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "kick_rand_delta_factor", &kick_rand_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "extra_stamina_delta_min", &extra_stamina_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "extra_stamina_delta_max", &extra_stamina_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "effort_max_delta_factor", &effort_max_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "effort_min_delta_factor", &effort_min_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "random_seed", &random_seed_ ) );
    param_map_.insert( ParamMap::value_type( "new_dash_power_rate_delta_min", &new_dash_power_rate_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "new_dash_power_rate_delta_max", &new_dash_power_rate_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "new_stamina_inc_max_delta_factor", &new_stamina_inc_max_delta_factor_ ) );
    // 14.0.0
    param_map_.insert( ParamMap::value_type( "kick_power_rate_delta_min", &kick_power_rate_delta_min_ ) );
    param_map_.insert( ParamMap::value_type( "kick_power_rate_delta_max", &kick_power_rate_delta_max_ ) );
    param_map_.insert( ParamMap::value_type( "foul_detect_probability_delta_factor", &foul_detect_probability_delta_factor_ ) );
    param_map_.insert( ParamMap::value_type( "catchable_area_l_stretch_min", &catchable_area_l_stretch_min_ ) );
    param_map_.insert( ParamMap::value_type( "catchable_area_l_stretch_max", &catchable_area_l_stretch_max_ ) );
}

/*-------------------------------------------------------------------*/
std::ostream &
PlayerParamT::toServerString( std::ostream & os ) const
{
    return print_server_message( os, "player_param", param_map_ );
}

/*-------------------------------------------------------------------*/
std::ostream &
PlayerParamT::toJSON( std::ostream & os ) const
{
    return print_json( os, "player_param", param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerParamT::fromServerString( const std::string & msg )
{
    return parse_server_message( msg, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerParamT::fromStruct( const player_params_t & from )
{
    player_types_ = nstohi( from.player_types );
    substitute_max_ = nstohi( from.substitute_max );
    pt_max_ = nstohi( from.pt_max );

    player_speed_max_delta_min_ = nltohd( from.player_speed_max_delta_min );
    player_speed_max_delta_max_ = nltohd( from.player_speed_max_delta_max );
    stamina_inc_max_delta_factor_ = nltohd( from.stamina_inc_max_delta_factor );

    player_decay_delta_min_ = nltohd( from.player_decay_delta_min );
    player_decay_delta_max_ = nltohd( from.player_decay_delta_max );
    inertia_moment_delta_factor_ = nltohd( from.inertia_moment_delta_factor );

    dash_power_rate_delta_min_ = nltohd( from.dash_power_rate_delta_min );
    dash_power_rate_delta_max_ = nltohd( from.dash_power_rate_delta_max );
    player_size_delta_factor_ = nltohd( from.player_size_delta_factor );

    kickable_margin_delta_min_ = nltohd( from.kickable_margin_delta_min );
    kickable_margin_delta_max_ = nltohd( from.kickable_margin_delta_max );
    kick_rand_delta_factor_ = nltohd( from.kick_rand_delta_factor );

    extra_stamina_delta_min_ = nltohd( from.extra_stamina_delta_min );
    extra_stamina_delta_max_ = nltohd( from.extra_stamina_delta_max );
    effort_max_delta_factor_ = nltohd( from.effort_max_delta_factor );
    effort_min_delta_factor_ = nltohd( from.effort_min_delta_factor );

    random_seed_ = static_cast< int >( static_cast< rcg::Int32 >( ntohl( from.random_seed ) ) );

    new_dash_power_rate_delta_min_ = nltohd( from.new_dash_power_rate_delta_min );
    new_dash_power_rate_delta_max_ = nltohd( from.new_dash_power_rate_delta_max );
    new_stamina_inc_max_delta_factor_ = nltohd( from.new_stamina_inc_max_delta_factor );

    allow_mult_default_type_ = nstohb( from.allow_mult_default_type );

    kick_power_rate_delta_min_ = nltohd( from.kick_power_rate_delta_min );
    kick_power_rate_delta_max_ = nltohd( from.kick_power_rate_delta_max );
    foul_detect_probability_delta_factor_ = nltohd( from.foul_detect_probability_delta_factor );

    catchable_area_l_stretch_min_ = nltohd( from.catchable_area_l_stretch_min );
    catchable_area_l_stretch_max_ = nltohd( from.catchable_area_l_stretch_max );

    return true;
}

/*-------------------------------------------------------------------*/
bool
PlayerParamT::setValue( const std::string & name,
                        const std::string & value )
{
    return set_value( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerParamT::setInt( const std::string & name,
                      const int value )
{
    return set_integer( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerParamT::setDouble( const std::string & name,
                         const double value )
{
    return set_double( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerParamT::setBool( const std::string & name,
                       const bool value )
{
    return set_boolean( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
PlayerTypeT::PlayerTypeT()
    : id_( 0 ),
      player_speed_max_( 1.2 ),
      stamina_inc_max_( 45.0 ),
      player_decay_( 0.4 ),
      inertia_moment_( 5.0 ),
      dash_power_rate_( 0.06 ),
      player_size_( 0.3 ),
      kickable_margin_( 0.7 ),
      kick_rand_( 0.1 ),
      extra_stamina_( 0.0 ),
      effort_max_( 1.0 ),
      effort_min_( 0.6 ),
      kick_power_rate_( 0.027 ),
      foul_detect_probability_( 0.5 ),
      catchable_area_l_stretch_( 1.0 )
{
    param_map_.insert( ParamMap::value_type( "id", &id_ ) );

    param_map_.insert( ParamMap::value_type( "player_speed_max", &player_speed_max_ ) );
    param_map_.insert( ParamMap::value_type( "stamina_inc_max", &stamina_inc_max_ ) );
    param_map_.insert( ParamMap::value_type( "player_decay", &player_decay_ ) );
    param_map_.insert( ParamMap::value_type( "inertia_moment", &inertia_moment_ ) );
    param_map_.insert( ParamMap::value_type( "dash_power_rate", &dash_power_rate_ ) );
    param_map_.insert( ParamMap::value_type( "player_size", &player_size_ ) );
    param_map_.insert( ParamMap::value_type( "kickable_margin", &kickable_margin_ ) );
    param_map_.insert( ParamMap::value_type( "kick_rand", &kick_rand_ ) );
    param_map_.insert( ParamMap::value_type( "extra_stamina", &extra_stamina_ ) );
    param_map_.insert( ParamMap::value_type( "effort_max", &effort_max_ ) );
    param_map_.insert( ParamMap::value_type( "effort_min", &effort_min_ ) );
    // 14.0.0
    param_map_.insert( ParamMap::value_type( "kick_power_rate", &kick_power_rate_ ) );
    param_map_.insert( ParamMap::value_type( "foul_detect_probability", &foul_detect_probability_ ) );
    param_map_.insert( ParamMap::value_type( "catchable_area_l_stretch", &catchable_area_l_stretch_ ) );
    // 18.0
    param_map_.insert( ParamMap::value_type( "unum_far_length", &unum_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "unum_too_far_length", &unum_too_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "team_far_length", &team_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "team_too_far_length", &team_too_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "player_max_observation_length", &player_max_observation_length_ ) );
    param_map_.insert( ParamMap::value_type( "ball_vel_far_length", &ball_vel_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "ball_vel_too_far_length", &ball_vel_too_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "ball_max_observation_length", &ball_max_observation_length_ ) );
    param_map_.insert( ParamMap::value_type( "flag_chg_far_length", &flag_chg_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "flag_chg_too_far_length", &flag_chg_too_far_length_ ) );
    param_map_.insert( ParamMap::value_type( "flag_max_observation_length", &flag_max_observation_length_ ) );
    // 19.0
    // param_map_.insert( ParamMap::value_type( "dist_noise_rate", &dist_noise_rate_ ) );
    // param_map_.insert( ParamMap::value_type( "focus_dist_noise_rate", &focus_dist_noise_rate_ ) );
    // param_map_.insert( ParamMap::value_type( "land_dist_noise_rate", &land_dist_noise_rate_ ) );
    // param_map_.insert( ParamMap::value_type( "land_focus_dist_noise_rate", &land_focus_dist_noise_rate_ ) );
}

/*-------------------------------------------------------------------*/
std::ostream &
PlayerTypeT::toServerString( std::ostream & os ) const
{
    //return print_server_message( os, "player_type", param_map_ );

    os << "(player_type ";
    to_sexp( os, "id", id_ );

    to_sexp( os, "player_speed_max", quantize( player_speed_max_, 0.00001 ) );
    to_sexp( os, "stamina_inc_max", quantize( stamina_inc_max_, 0.00001 ) );
    to_sexp( os, "player_decay", quantize(player_decay_, 0.000001 ) );
    to_sexp( os, "inertia_moment", quantize( inertia_moment_, 0.00001 ) );
    to_sexp( os, "dash_power_rate", quantize( dash_power_rate_, 0.00000001 ) );
    to_sexp( os, "player_size", quantize( player_size_, 0.00001 ) );
    to_sexp( os, "kickable_margin", quantize( kickable_margin_, 0.000001 ) );
    to_sexp( os, "kick_rand", quantize( kick_rand_, 0.000001 ) );
    to_sexp( os, "extra_stamina", quantize( extra_stamina_, 0.00001 ) );
    to_sexp( os, "effort_max", quantize( effort_max_, 0.000001 ) );
    to_sexp( os, "effort_min", quantize( effort_min_, 0.000001 ) );
    // 14.0
    to_sexp( os, "kick_power_rate", quantize( kick_power_rate_, 0.000001 ) );
    to_sexp( os, "foul_detect_probability", quantize( foul_detect_probability_, 0.000001 ) );
    to_sexp( os, "catchable_area_l_stretch", quantize( catchable_area_l_stretch_, 0.000001 ) );
    // 18.0
    to_sexp( os, "unum_far_length", quantize( unum_far_length_, 0.000001 ) );
    to_sexp( os, "unum_too_far_length", quantize( unum_too_far_length_, 0.000001 ) );
    to_sexp( os, "team_far_length", quantize( team_far_length_, 0.000001 ) );
    to_sexp( os, "team_too_far_length", quantize( team_too_far_length_, 0.000001 ) );
    to_sexp( os, "player_max_observation_length", quantize( player_max_observation_length_, 0.000001 ) );
    to_sexp( os, "ball_vel_far_length", quantize( ball_vel_far_length_, 0.000001 ) );
    to_sexp( os, "ball_vel_too_far_length", quantize( ball_vel_too_far_length_, 0.000001 ) );
    to_sexp( os, "ball_max_observation_length", quantize( ball_max_observation_length_, 0.000001 ) );
    to_sexp( os, "flag_chg_far_length", quantize( flag_chg_far_length_, 0.000001 ) );
    to_sexp( os, "flag_chg_too_far_length", quantize( flag_chg_too_far_length_, 0.000001 ) );
    to_sexp( os, "flag_max_observation_length", quantize( flag_max_observation_length_, 0.000001 ) );
    // 19.0
    // to_sexp( os, "dist_noise_rate", dist_noise_rate_ );
    // to_sexp( os, "focus_dist_noise_rate", dist_noise_rate_ );
    // to_sexp( os, "land_dist_noise_rate", dist_noise_rate_ );
    // to_sexp( os, "land_focus_dist_noise_rate", dist_noise_rate_ );

    os << ')';

    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
PlayerTypeT::toJSON( std::ostream & os ) const
{
    os << '{' << std::quoted( "player_type" ) << ':' << '{';

    os << std::quoted( "id" ) << ':' << id_;

    os << ',' << std::quoted( "player_speed_max" ) << ':' << quantize( player_speed_max_, 0.00001 );

    os << ',' << std::quoted( "stamina_inc_max" ) << ':' << quantize( stamina_inc_max_, 0.00001 );
    os << ',' << std::quoted( "player_decay" ) << ':' << quantize(player_decay_, 0.000001 );
    os << ',' << std::quoted( "inertia_moment" ) << ':' << quantize( inertia_moment_, 0.00001 );
    os << ',' << std::quoted( "dash_power_rate" ) << ':' << quantize( dash_power_rate_, 0.00000001 );
    os << ',' << std::quoted( "player_size" ) << ':' << quantize( player_size_, 0.00001 );
    os << ',' << std::quoted( "kickable_margin" ) << ':' << quantize( kickable_margin_, 0.000001 );
    os << ',' << std::quoted( "kick_rand" ) << ':' << quantize( kick_rand_, 0.000001 );
    os << ',' << std::quoted( "extra_stamina" ) << ':' << quantize( extra_stamina_, 0.00001 );
    os << ',' << std::quoted( "effort_max" ) << ':' << quantize( effort_max_, 0.000001 );
    os << ',' << std::quoted( "effort_min" ) << ':' << quantize( effort_min_, 0.000001 );
    // 14.0
    os << ',' << std::quoted( "kick_power_rate" ) << ':' << quantize( kick_power_rate_, 0.000001 );
    os << ',' << std::quoted( "foul_detect_probability" ) << ':' << quantize( foul_detect_probability_, 0.000001 );
    os << ',' << std::quoted( "catchable_area_l_stretch" ) << ':' << quantize( catchable_area_l_stretch_, 0.000001 );
    // 18.0
    os << ',' << std::quoted( "unum_far_length" ) << ':' << quantize( unum_far_length_, 0.000001 );
    os << ',' << std::quoted( "unum_too_far_length" ) << ':' << quantize( unum_too_far_length_, 0.000001 );
    os << ',' << std::quoted( "team_far_length" ) << ':' << quantize( team_far_length_, 0.000001 );
    os << ',' << std::quoted( "team_too_far_length" ) << ':' << quantize( team_too_far_length_, 0.000001 );
    os << ',' << std::quoted( "player_max_observation_length" ) << ':' << quantize( player_max_observation_length_, 0.000001 );
    os << ',' << std::quoted( "ball_vel_far_length" ) << ':' << quantize( ball_vel_far_length_, 0.000001 );
    os << ',' << std::quoted( "ball_vel_too_far_length" ) << ':' << quantize( ball_vel_too_far_length_, 0.000001 );
    os << ',' << std::quoted( "ball_max_observation_length" ) << ':' << quantize( ball_max_observation_length_, 0.000001 );
    os << ',' << std::quoted( "flag_chg_far_length" ) << ':' << quantize( flag_chg_far_length_, 0.000001 );
    os << ',' << std::quoted( "flag_chg_too_far_length" ) << ':' << quantize( flag_chg_too_far_length_, 0.000001 );
    os << ',' << std::quoted( "flag_max_observation_length" ) << ':' << quantize( flag_max_observation_length_, 0.000001 );
    // 19.0
    // os << ',' << std::quoted( "dist_noise_rate" ) << ':' << quantize( dist_noise_rate_, 0.000001 );
    // os << ',' << std::quoted( "focus_dist_noise_rate" ) << ':' << quantize( dist_noise_rate_, 0.000001 );
    // os << ',' << std::quoted( "land_dist_noise_rate" ) << ':' << quantize( dist_noise_rate_, 0.000001 );
    // os << ',' << std::quoted( "land_focus_dist_noise_rate" ) << ':' << quantize( dist_noise_rate_, 0.000001 );

    os << '}' << '}';
    return os;
}

/*-------------------------------------------------------------------*/
bool
PlayerTypeT::fromServerString( const std::string & msg )
{
    return parse_server_message( msg, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerTypeT::fromStruct( const player_type_t & from )
{
    id_ = nstohi( from.id );

    player_speed_max_ = nltohd( from.player_speed_max );
    stamina_inc_max_ = nltohd( from.stamina_inc_max );
    player_decay_ = nltohd( from.player_decay );
    inertia_moment_ = nltohd( from.inertia_moment );
    dash_power_rate_ = nltohd( from.dash_power_rate );
    player_size_ = nltohd( from.player_size );
    kickable_margin_ = nltohd( from.kickable_margin );
    kick_rand_ = nltohd( from.kick_rand );
    extra_stamina_ = nltohd( from.extra_stamina );
    effort_max_ = nltohd( from.effort_max );
    effort_min_ = nltohd( from.effort_min );;

    if ( from.kick_power_rate != 0 )
    {
        kick_power_rate_ = nltohd( from.kick_power_rate );;
    }

    if ( from.foul_detect_probability != 0 )
    {
        foul_detect_probability_ = nltohd( from.foul_detect_probability );;
    }

    if ( from.catchable_area_l_stretch != 0 )
    {
        catchable_area_l_stretch_ = nltohd( from.catchable_area_l_stretch );;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
PlayerTypeT::setValue( const std::string & name,
                       const std::string & value )
{
    return set_value( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerTypeT::setInt( const std::string & name,
                     const int value )
{
    return set_integer( name, value, param_map_ );
}

/*-------------------------------------------------------------------*/
bool
PlayerTypeT::setDouble( const std::string & name,
                        const double value )
{
    return set_double( name, value, param_map_ );
}

}
}
