// -*-c++-*-

/*!
  \file serializer_v4.cpp
  \brief v4 format rcg serializer Source File.
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

#include "serializer_v4.h"

#include "util.h"

#include <cstring>
#include <cmath>

namespace {
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
/*!

 */
std::ostream &
SerializerV4::serializeHeader( std::ostream & os )
{
    return os << "ULG4\n";
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serializeParam( std::ostream & os,
                              const std::string & msg )
{
    return os << msg << '\n';
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const server_params_t & param )
{
    double tmp = 0.0;
    os << "(server_param "
       << "(goal_width " << quantize( nltohd( param.goal_width ) ) << ')'
       << "(inertia_moment " << quantize( nltohd( param.inertia_moment ) ) << ')'
       << "(player_size " << quantize( nltohd( param.player_size ) ) << ')'
       << "(player_decay " << quantize( nltohd( param.player_decay ) ) << ')'
       << "(player_rand " << quantize( nltohd( param.player_rand ), 0.0001 ) << ')'
       << "(player_weight " << quantize( nltohd( param.player_weight ) ) << ')'
       << "(player_speed_max " << quantize( nltohd( param.player_speed_max ) ) << ')'
       << "(player_accel_max " << quantize( nltohd( param.player_accel_max ) ) << ')'
       << "(stamina_max " << quantize( nltohd( param.stamina_max ) ) << ')'
       << "(stamina_inc_max " << quantize( nltohd( param.stamina_inc ) ) << ')'
       << "(recover_init " << quantize( nltohd( param.recover_init ) ) << ')'
       << "(recover_dec_thr " << quantize( nltohd( param.recover_dec_thr ) ) << ')'
       << "(recover_min " << quantize( nltohd( param.recover_min ) ) << ')'
       << "(recover_dec " << quantize( nltohd( param.recover_dec ) ) << ')'
       << "(effort_init " << quantize( nltohd( param.effort_init ) ) << ')'
       << "(effort_dec_thr " << quantize( nltohd( param.effort_dec_thr ) ) << ')'
       << "(effort_min " << quantize( nltohd( param.effort_min ), 0.00001 ) << ')'
       << "(effort_dec " << quantize( nltohd( param.effort_dec ), 0.00001 ) << ')'
       << "(effort_inc_thr " << quantize( nltohd( param.effort_inc_thr ) ) << ')'
       << "(effort_inc " << quantize( nltohd( param.effort_inc ), 0.0001 ) << ')'
       << "(kick_rand " << quantize( nltohd( param.kick_rand ), 0.00001 ) << ')'
       << "(team_actuator_noise " << nstohi( param.team_actuator_noise ) << ')'
       << "(prand_factor_l " << quantize( nltohd( param.player_rand_factor_l ) ) << ')'
       << "(prand_factor_r " << quantize( nltohd( param.player_rand_factor_r ) ) << ')'
       << "(kick_rand_factor_l " << quantize( nltohd( param.kick_rand_factor_l ) ) << ')'
       << "(kick_rand_factor_r " << quantize( nltohd( param.kick_rand_factor_r ) ) << ')'
       << "(ball_size " << quantize( nltohd( param.ball_size ) ) << ')'
       << "(ball_decay " << quantize( nltohd( param.ball_decay ) ) << ')'
       << "(ball_rand " << quantize( nltohd( param.ball_rand ) ) << ')'
       << "(ball_weight " << quantize( nltohd( param.ball_weight ) ) << ')'
       << "(ball_speed_max " << quantize( nltohd( param.ball_speed_max ) ) << ')'
       << "(ball_accel_max " << quantize( nltohd( param.ball_accel_max ) ) << ')'
       << "(dash_power_rate " << quantize( nltohd( param.dash_power_rate ), 0.0001 ) << ')'
       << "(kick_power_rate " << quantize( nltohd( param.kick_power_rate ), 0.0001 ) << ')'
       << "(kickable_margin " << quantize( nltohd( param.kickable_margin ) ) << ')'
       << "(control_radius " << quantize( nltohd( param.control_radius ) ) << ')'
        //<< "(control_radius_width " << quantize( nltohd( param.control_radius_width ) ) << ')'
       << "(maxpower " << quantize( nltohd( param.max_power ) ) << ')'
       << "(minpower " << quantize( nltohd( param.min_power ) ) << ')'
       << "(maxmoment " << quantize( nltohd( param.max_moment ) ) << ')'
       << "(minmoment " << quantize( nltohd( param.min_moment ) ) << ')'
       << "(maxneckmoment " << quantize( nltohd( param.max_neck_moment ) ) << ')'
       << "(minneckmoment " << quantize( nltohd( param.min_neck_moment ) ) << ')'
       << "(maxneckang " << quantize( nltohd( param.max_neck_angle ) ) << ')'
       << "(minneckang " << quantize( nltohd( param.min_neck_angle ) ) << ')'
       << "(visible_angle " << quantize( nltohd( param.visible_angle ) ) << ')'
       << "(visible_distance " << quantize( nltohd( param.visible_distance ) ) << ')'
       << "(wind_dir " << quantize( nltohd( param.wind_dir ) ) << ')'
       << "(wind_force " << quantize( nltohd( param.wind_force ) ) << ')'
       << "(wind_ang " << quantize( nltohd( param.wind_ang ) ) << ')'
       << "(wind_rand " << quantize( nltohd( param.wind_rand ) ) << ')'
        //<< "(kickable_area " << quantize( nltohd( param.kickable_area ) ) << ')'
       << "(catchable_area_l " << quantize( nltohd( param.catch_area_l ) ) << ')'
       << "(catchable_area_w " << quantize( nltohd( param.catch_area_w ) ) << ')'
       << "(catch_probability " << quantize( nltohd( param.catch_probability ) ) << ')'
       << "(goalie_max_moves " << nstohi( param.goalie_max_moves ) << ')'
       << "(ckick_margin " << quantize( nltohd( param.corner_kick_margin ) ) << ')'
       << "(offside_active_area_size " << quantize( nltohd( param.offside_active_area ) ) << ')'
       << "(wind_none " << nstohi( param.wind_none ) << ')'
       << "(wind_random " << nstohi( param.use_wind_random ) << ')'
       << "(say_coach_cnt_max " << nstohi( param.coach_say_count_max ) << ')'
       << "(say_coach_msg_size " << nstohi( param.coach_say_msg_size ) << ')'
       << "(clang_win_size " << nstohi( param.clang_win_size ) << ')'
       << "(clang_define_win " << nstohi( param.clang_define_win ) << ')'
       << "(clang_meta_win " << nstohi( param.clang_meta_win ) << ')'
       << "(clang_advice_win " << nstohi( param.clang_advice_win ) << ')'
       << "(clang_info_win " << nstohi( param.clang_info_win ) << ')'
       << "(clang_mess_delay " << nstohi( param.clang_mess_delay ) << ')'
       << "(clang_mess_per_cycle " << nstohi( param.clang_mess_per_cycle ) << ')'
       << "(half_time " << nstohi( param.half_time ) << ')'
       << "(simulator_step " << nstohi( param.simulator_step ) << ')'
       << "(send_step " << nstohi( param.send_step ) << ')'
       << "(recv_step " << nstohi( param.recv_step ) << ')'
       << "(sense_body_step " << nstohi( param.sense_body_step ) << ')'
        //<< "(lcm_step " << nstohi( param.lcm_step ) << ')'
       << "(say_msg_size " << nstohi( param.player_say_msg_size ) << ')'
       << "(hear_max " << nstohi( param.player_hear_max ) << ')'
       << "(hear_inc " << nstohi( param.player_hear_inc ) << ')'
       << "(hear_decay " << nstohi( param.player_hear_decay ) << ')'
       << "(catch_ban_cycle " << nstohi( param.catch_ban_cycle ) << ')'
       << "(slow_down_factor " << nstohi( param.slow_down_factor ) << ')'
       << "(use_offside " << nstohi( param.use_offside ) << ')'
       << "(forbid_kick_off_offside " << nstohi( param.kickoff_offside ) << ')'
       << "(offside_kick_margin " << quantize( nltohd( param.offside_kick_margin ) ) << ')'
       << "(audio_cut_dist " << quantize( nltohd( param.audio_cut_dist ) ) << ')'
       << "(quantize_step " << quantize( nltohd( param.dist_quantize_step ) ) << ')'
       << "(quantize_step_l " << quantize( nltohd( param.landmark_dist_quantize_step ), 0.0001 ) << ')'
        //<< "(quantize_step_dir " << quantize( nltohd( param.dir_quantize_step ) ) << ')'
        //<< "(quantize_step_dist_team_l " << quantize( nltohd( param.dist_quantize_step_l ) ) << ')'
        //<< "(quantize_step_dist_team_r " << quantize( nltohd( param.dist_quantize_step_r ) ) << ')'
        //<< "(quantize_step_dist_l_team_l " << quantize( nltohd( param.landmark_dist_quantize_step_l ) ) << ')'
        //<< "(quantize_step_dist_l_team_r " << quantize( nltohd( param.landmark_dist_quantize_step_r ) ) << ')'
        //<< "(quantize_step_dir_team_l " << quantize( nltohd( param.dir_quantize_step_l ) ) << ')'
        //<< "(quantize_step_dir_team_r " << quantize( nltohd( param.dir_quantize_step_r ) ) << ')'
       << "(coach " << nstohi( param.coach_mode ) << ')'
       << "(coach_w_referee " << nstohi( param.coach_with_referee_mode ) << ')'
       << "(old_coach_hear " << nstohi( param.use_old_coach_hear ) << ')'
       << "(send_vi_step " << nstohi( param.online_coach_look_step ) << ')'
       << "(slowness_on_top_for_left_team " << quantize( nltohd( param.slowness_on_top_for_left_team ) ) << ')'
       << "(slowness_on_top_for_right_team " << quantize( nltohd( param.slowness_on_top_for_right_team ) ) << ')'
       << "(keepaway_length " << quantize( nltohd( param.ka_length ) ) << ')'
       << "(keepaway_width " << quantize( nltohd( param.ka_width ) ) << ')';

    tmp = quantize( nltohd( param.ball_stuck_area ) );
    if ( std::fabs( tmp ) < 100.0 ) os << "(ball_stuck_area " << tmp << ')';
    tmp = quantize( nltohd( param.max_tackle_power ) );
    if ( 0.0 <= tmp && tmp < 200.0 ) os << "(max_tackle_power " << tmp << ')';
    tmp = quantize( nltohd( param.max_back_tackle_power ) );
    if ( 0.0 <= tmp && tmp < 200.0 ) os << "(max_back_tackle_power " << tmp << ')';
    tmp = quantize( nltohd( param.tackle_dist ) );
    if ( 0.0 <= tmp && tmp < 100.0 ) os << "(tackle_dist " << tmp << ')';
    tmp = quantize( nltohd( param.tackle_back_dist ) );
    if ( 0.0 <= tmp && tmp < 100.0 ) os << "(tackle_back_dist " << tmp << ')';
    tmp = quantize( nltohd( param.tackle_width ) );
    if ( 0.0 <= tmp && tmp < 100.0 ) os << "(tackle_width " << tmp << ')';

    os << "(start_goal_l " << nstohi( param.start_goal_l ) << ')'
       << "(start_goal_r " << nstohi( param.start_goal_r ) << ')'
       << "(fullstate_l " << nstohi( param.fullstate_l ) << ')'
       << "(fullstate_r " << nstohi( param.fullstate_r ) << ')'
       << "(drop_ball_time " << nstohi( param.drop_ball_time ) << ')'
       << "(synch_mode " << nstohi( param.synch_mode ) << ')'
       << "(synch_offset " << nstohi( param.synch_offset ) << ')'
       << "(synch_micro_sleep " << nstohi( param.synch_micro_sleep ) << ')'
       << "(point_to_ban " << nstohi( param.point_to_ban ) << ')'
       << "(point_to_duration " << nstohi( param.point_to_duration ) << ')'
       << ")\n";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const player_params_t & pparam )
{
    os << "(player_param "
       << "(player_types " << nstohi( pparam.player_types ) << ')'
       << "(subs_max " << nstohi( pparam.substitute_max ) << ')'
       << "(pt_max " << nstohi( pparam.pt_max ) << ')'
       << "(player_speed_max_delta_min " << quantize( nltohd( pparam.player_speed_max_delta_min ) ) << ')'
       << "(player_speed_max_delta_max " << quantize( nltohd( pparam.player_speed_max_delta_max ) ) << ')'
       << "(stamina_inc_max_delta_factor " << quantize( nltohd( pparam.stamina_inc_max_delta_factor ) ) << ')'
       << "(player_decay_delta_min " << quantize( nltohd( pparam.player_decay_delta_min ) ) << ')'
       << "(player_decay_delta_max " << quantize( nltohd( pparam.player_decay_delta_max ) ) << ')'
       << "(inertia_moment_delta_factor " << quantize( nltohd( pparam.inertia_moment_delta_factor ) ) << ')'
       << "(dash_power_rate_delta_min " << quantize( nltohd( pparam.dash_power_rate_delta_min ) ) << ')'
       << "(dash_power_rate_delta_max " << quantize( nltohd( pparam.dash_power_rate_delta_max ) ) << ')'
       << "(player_size_delta_factor " << quantize( nltohd( pparam.player_size_delta_factor ) ) << ')'
       << "(kickable_margin_delta_min " << quantize( nltohd( pparam.kickable_margin_delta_min ) ) << ')'
       << "(kickable_margin_delta_max " << quantize( nltohd( pparam.kickable_margin_delta_max ) ) << ')'
       << "(kick_rand_delta_factor " << quantize( nltohd( pparam.kick_rand_delta_factor ) ) << ')'
       << "(extra_stamina_delta_min " << quantize( nltohd( pparam.extra_stamina_delta_min ) ) << ')'
       << "(extra_stamina_delta_max " << quantize( nltohd( pparam.extra_stamina_delta_max ) ) << ')'
       << "(effort_max_delta_factor " << quantize( nltohd( pparam.effort_max_delta_factor ) ) << ')'
       << "(effort_min_delta_factor " << quantize( nltohd( pparam.effort_min_delta_factor ) ) << ')'
       << "(random_seed " << static_cast< Int32 >( ntohl( pparam.random_seed ) ) << ')'
       << "(new_dash_power_rate_delta_min " << quantize( nltohd( pparam.new_dash_power_rate_delta_min ) ) << ')'
       << "(new_dash_power_rate_delta_max " << quantize( nltohd( pparam.new_dash_power_rate_delta_max ) ) << ')'
       << "(new_stamina_inc_max_delta_factor " << quantize( nltohd( pparam.new_stamina_inc_max_delta_factor ) ) << ')'
       << "(allow_mult_default_type " << static_cast< bool >( nstohi( pparam.allow_mult_default_type ) ) << ')'
       << ")\n";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const player_type_t & type )
{
    os << "(player_type "
       << "(id " << nstohi( type.id ) << ')'
       << "(player_speed_max " << quantize( nltohd( type.player_speed_max ) ) << ')'
       << "(stamina_inc_max " << quantize( nltohd( type.stamina_inc_max ) ) << ')'
       << "(player_decay " << quantize( nltohd( type.player_decay ) ) << ')'
       << "(inertia_moment " << quantize( nltohd( type.inertia_moment ) ) << ')'
       << "(dash_power_rate " << quantize( nltohd( type.dash_power_rate ) ) << ')'
       << "(player_size " << quantize( nltohd( type.player_size ) ) << ')'
       << "(kickable_margin " << quantize( nltohd( type.kickable_margin ) ) << ')'
       << "(kick_rand " << quantize( nltohd( type.kick_rand ) ) << ')'
       << "(extra_stamina " << quantize( nltohd( type.extra_stamina ) ) << ')'
       << "(effort_max " << quantize( nltohd( type.effort_max ) ) << ')'
       << "(effort_min " << quantize( nltohd( type.effort_min ) ) << ')'
       << ")\n";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
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
SerializerV4::serialize( std::ostream & os,
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
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
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
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const short_showinfo_t2 & show2 )
{
    M_time = static_cast< Int32 >( ntohs( show2.time ) );

    ShowInfoT new_show;

    convert( show2, new_show );

    return serialize( os, new_show );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const msginfo_t & msg )
{
    os << "(msg " << M_time << ' ' << ntohs( msg.board )
       << " \"" << msg.message << "\")\n";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const Int16 board,
                         const std::string & msg )
{
    os << "(msg " << M_time << ' ' << ntohs( board )
       << " \"" << msg << "\")\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const drawinfo_t & )
{
    // TODO
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const char playmode )
{
    static const char * playmode_strings[] = PLAYMODE_STRINGS;

    M_playmode = playmode;

    PlayMode pm = static_cast< PlayMode >( playmode );
    if ( pm < PM_Null || PM_MAX <= pm )
    {
        return os;
    }

    os << "(playmode " << M_time << ' ' << playmode_strings[pm] << ")\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const team_t & team_l,
                         const team_t & team_r )
{
    convert( team_l, M_teams[0] );
    convert( team_r, M_teams[1] );

    return serialize( os, M_teams[0], M_teams[1] );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const TeamT & team_l,
                         const TeamT & team_r )
{
    M_teams[0] = team_l;
    M_teams[1] = team_r;

    os << "(team " << M_time
       << ' ' << ( team_l.name_.empty() ? "null" : team_l.name_.c_str() )
       << ' ' << ( team_r.name_.empty() ? "null" : team_r.name_.c_str() )
       << ' ' << team_l.score_
       << ' ' << team_r.score_;
    if ( team_l.penaltyTrial() > 0 || team_r.penaltyTrial() > 0 )
    {
        os << ' ' << team_l.pen_score_ << ' ' << team_l.pen_miss_
           << ' ' << team_r.pen_score_ << ' ' << team_r.pen_miss_;
    }
    os << ")\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV4::serialize( std::ostream & os,
                         const ShowInfoT & show )
{
    M_time = show.time_;

    os << "(show " << show.time_;

    // ball

    os << " ((b)"
       << ' ' << show.ball_.x_ << ' ' << show.ball_.y_;
    if ( show.ball_.hasVelocity() )
    {
        os << ' ' << show.ball_.vx_ << ' ' << show.ball_.vy_;
    }
    else
    {
        os << " 0 0";
    }
    os << ')';

    // players

    for ( int i = 0; i < MAX_PLAYER*2; ++i )
    {
        const PlayerT & p = show.player_[i];

        os << " ((" << p.side_ << ' ' << p.unum_ << ')';
        os << ' ' << p.type_;
        os << ' ' << std::hex << std::showbase
           << p.state_
           << std::dec << std::noshowbase;

        os << ' ' << p.x_ << ' ' << p.y_;
        if ( p.hasVelocity() )
        {
            os << ' ' << p.vx_ << ' ' << p.vy_;
        }
        else
        {
            os << " 0 0";
        }
        os << ' ' << p.body_
           << ' ' << ( p.hasNeck() ? p.neck_ : 0.0f );

        if ( p.isPointing() )
        {
            os << ' ' << p.point_x_ << ' ' << p.point_y_;
        }

        if ( p.hasView() )
        {
            os << " (v " << p.view_quality_ << ' ' << p.view_width_ << ')';
        }
        else
        {
            os << " (v h 90)";
        }

        if ( p.hasStamina() )
        {
            os << " (s " << p.stamina_
               << ' ' << p.effort_
               << ' ' << p.recovery_
               << ')';
        }
        else
        {
            os << " (s 4000 1 1)";
        }

        if ( p.focus_side_ != 'n' )
        {
            os << " (f" << p.focus_side_ << ' ' << p.focus_unum_ << ')';
        }

        os << " (c"
           << ' ' << p.kick_count_
           << ' ' << p.dash_count_
           << ' ' << p.turn_count_
           << ' ' << p.catch_count_
           << ' ' << p.move_count_
           << ' ' << p.turn_neck_count_
           << ' ' << p.change_view_count_
           << ' ' << p.say_count_
           << ' ' << p.tackle_count_
           << ' ' << p.pointto_count_
           << ' ' << p.attentionto_count_
           << ')';
        os << ')';
    }

    os << ")\n";

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV4::serialize( std::ostream & os,
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
namespace {

Serializer::Ptr
create_v4()
{
    Serializer::Ptr ptr( new SerializerV4() );
    return ptr;
}

rcss::RegHolder v4 = Serializer::creators().autoReg( &create_v4, REC_VERSION_4 );

}

} // end of namespace
} // end of namespace
