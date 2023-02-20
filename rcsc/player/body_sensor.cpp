// -*-c++-*-

/*!
  \file body_sensor.cpp
  \brief sense_body sensor Source File
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

#include "body_sensor.h"

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
BodySensor::BodySensor()
    : M_time( 0, 0 )
    , M_stamina( 4000.0 )
    , M_effort( 1.0 )
    , M_stamina_capacity( -1.0 )
    , M_speed_mag( 0.0 )
    , M_speed_dir_relative( 0.0 )
    , M_neck_relative( 0.0 )
    , M_kick_count( 0 )
    , M_dash_count( 0 )
    , M_turn_count( 0 )
    , M_say_count( 0 )
    , M_turn_neck_count( 0 )
    , M_catch_count( 0 )
    , M_move_count( 0 )
    , M_change_view_count( 0 )
    , M_change_focus_count( 0 )
    , M_arm_movable( 0 )
    , M_arm_expires( 0 )
    , M_pointto_dist( 0.0 )
    , M_pointto_dir( 0.0 )
    , M_pointto_count( 0 )
    , M_attentionto_side( NEUTRAL )
    , M_attentionto_unum( 0 )
    , M_attentionto_count( 0 )
    , M_tackle_expires( 0 )
    , M_tackle_count( 0 )
    , M_none_collided( false )
    , M_ball_collided( false )
    , M_player_collided( false )
    , M_post_collided( false )
    , M_charged_expires( 0 )
    , M_card( NO_CARD )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
BodySensor::parse( const char * msg,
                   const double & version,
                   const GameTime & current )
{
    // ver. 4 & under
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0)
    //  (kick 0) (dash 0) (turn 0) (say 0))

    // ver. 5
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0) (head_angle 0)
    //  (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0))

    // ver. 6
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0 0) (head_angle 0)
    //  (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0))

    // ver. 7
    // (sense_body 0 (view_mode high normal) (stamina 4000 1) (speed 0 0) (head_angle 0)
    //  (kick 0) (dash 0) (turn 0) (say 0) (turn_neck 0) (catch 0) (move 1) (change_view 0))

    // ver. 8
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0)))

    // ver. 12
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0))
    //  (collision {none|[(ball)][player][post]}))

    // ver. 13
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1 124000) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0))
    //  (collision {none|[(ball)][player][post]}))

    // ver. 14
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1 124000) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0))
    //  (collision {none|[(ball)][player][post]}))
    //  (foul (charged 0) (card {none|yellow|red}))

    // ver. 18
    // (sense_body 66 (view_mode high normal) (stamina 3503.4 1 124000) (speed 0.06 -79)
    //  (head_angle 89) (kick 4) (dash 20) (turn 24) (say 0) (turn_neck 28) (catch 0)
    //  (move 1) (change_view 16) (arm (movable 0) (expires 0) (target 0 0) (count 0))
    //  (focus (target none) (count 0)) (tackle (expires 0) (count 0))
    //  (collision {none|[(ball)][player][post]})
    //  (foul (charged 0) (card {none|yellow|red})
    //  (focus_point 0 0))

    //char ss[8];

    M_time = current;

    ++msg; // skip first paren
    while ( *msg != '\0' && *msg != '(' ) ++msg; // skip "sense_body <time> "

    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(view_mode"
    ++msg; // skip space
    // parse view quality
    switch ( *msg ) {
    case 'h':  // high
        M_view_quality = ViewQuality::HIGH;
        break;
    case 'l':  // low
        M_view_quality = ViewQuality::LOW;
        break;
    default:
        std::cerr << "sense_body: Unknown View Quality" << std::endl;
        break;
    }

    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip view_quality string
    ++msg; // skip space

    // parse view width
    switch ( *( msg + 1 ) ) {
    case 'o':  // "normal"
        M_view_width = ViewWidth::NORMAL;
        break;
    case 'a':  // "narrow"
        M_view_width = ViewWidth::NARROW;
        break;
    case 'i':  // "wide"
        M_view_width = ViewWidth::WIDE;
        break;
    default:
        std::cerr << "sense_body: Unknown View Width" << std::endl;
        break;
    }

    char *next;

    // read stamina values
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(stamina"
    M_stamina = std::strtod( msg, &next ); msg = next;
    M_effort = std::strtod( msg, &next ); msg = next;
    if ( version >= 13.0 )
    {
        if ( *msg != ')' )
        {
            M_stamina_capacity = std::strtod( msg, &next ); msg = next;
        }
    }

    // read speed values
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(speed"
    M_speed_mag = std::strtod( msg, &next );
    msg = next; // this value is quantized by 0.01
    if ( version >= 6.0 )
    {
        // Sensed speed_dir is the velocity dir relative to player's face angle
        // global_vel_dir = (sensed_speed_dir + my_global_neck_angle)
        M_speed_dir_relative = std::strtod( msg, &next );
        msg = next;
    }

    if ( version >= 5.0 )
    {
        while ( *msg != '(' ) ++msg;
        while ( *msg != ' ' ) ++msg; // skip "(head_angle"
        M_neck_relative = std::strtod( msg, &next );
        msg = next;
    }

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(kick"
    M_kick_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(dash"
    M_dash_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(turn"
    M_turn_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(say"
    M_say_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version < 5.0 )
    {
        return;
    }

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(turn_neck"
    M_turn_neck_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version < 7.0 )
    {
        return;
    }

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(catch"
    M_catch_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(move"
    M_move_count  = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(chage_view"
    M_change_view_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
    msg = next;

    if ( version >= 18.0 )
    {
        while ( *msg != '\0' && *msg != '(' ) ++msg;
        while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(change_focus"
        M_change_focus_count = static_cast< int >( std::strtol( msg, &next, 10 ) );
        msg = next;
    }

    if ( version < 8.0 )
    {
        return;
    }

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    if ( ! parseArm( msg, &next ) )
    {
        return;
    }
    msg = next;

    // (focus (target <SIDE> [<UNUM>]) (count <COUNT>)
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    if ( ! parseAttentionto( msg, &next ) )
    {
        return;
    }
    msg = next;

    // (tackle (expires <EXPIRES>) (count <COUNT>))
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    if ( ! parseTackle( msg, &next ) )
    {
        return;
    }
    msg = next;

    if ( version < 12.0 )
    {
        return;
    }

    // (collision {none|[(ball)][(player)][(post)]})
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    parseCollision( msg, &next );
    msg = next;

    if ( version < 14.0 )
    {
        return;
    }

    //
    // (foul (charged 0) (card {none|yellow|red}))
    //
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    parseFoul( msg, &next );
    msg = next;

    if ( version < 18.0 )
    {
        return;
    }

    //
    //  (focus_point 0.0 0.0))
    //
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    parseFocusPoint( msg, &next );
}

#if 0
/*-------------------------------------------------------------------*/
/*!

*/
void
BodySensor::parse2( const char * msg,
                    const double & version,
                    const GameTime & current )
{
    long sense_time;
    char vqual[8], vwidth[8];
    char attentionto_target[8];

    M_time = current;

    int n_read = 0;

    if ( version >= 8.0
         && std::sscanf( msg,
                         " (sense_body %ld (view_mode %7[^ ] %7[^)])"
                         " (stamina %lf %lf)"
                         " (speed %lf %lf) (head_angle %lf)"
                         " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)"
                         " (catch %d) (move %d) (change_view %d)"
                         " (arm (movable %d) (expires %d)"
                         " (target %lf %lf) (count %d))"
                         " (focus (target %[^)]) (count %d))"
                         " (tackle (expires %d) (count %d)) %n ",
                         &sense_time, vqual, vwidth,
                         &M_stamina, &M_effort,
                         &M_speed_mag, &M_speed_dir_relative,
                         &M_neck_relative,
                         &M_kick_count, &M_dash_count,
                         &M_turn_count, &M_say_count,
                         &M_turn_neck_count, &M_catch_count,
                         &M_move_count, &M_change_view_count,
                         &M_arm_movable , &M_arm_expires,
                         &M_pointto_dist, &M_pointto_dir,
                         &M_pointto_count,
                         attentionto_target , &M_attentionto_count,
                         &M_tackle_expires, &M_tackle_count,
                         &n_read ) == 25 )
    {
        if ( attentionto_target[0] == 'n' )
        {
            M_attentionto_side = NEUTRAL;
            M_attentionto_unum = Unum_Unknown;
        }
        else if ( attentionto_target[0] == 'l' )
        {
            M_attentionto_side = LEFT;
            M_attentionto_unum
                = static_cast< int >( std::strtol( attentionto_target + 2, NULL, 10 ) );
        }
        else if ( attentionto_target[0] == 'r' )
        {
            M_attentionto_side = RIGHT;
            M_attentionto_unum
                = static_cast< int >( std::strtol( attentionto_target + 2, NULL, 10 ) );
        }
        else
        {
            std::cerr << "Received unknown attentionto target ["
                      << attentionto_target << "]" << std::endl;
            M_attentionto_side = NEUTRAL;
            M_attentionto_unum = Unum_Unknown;
        }

        char *next = nullptr;
        if ( version >= 12.0 )
        {
            parseCollision( msg + n_read, &next );
            msg = next;
        }

        if ( version >= 14.0 )
        {
            parseFoul( msg, &next );
            msg = next;
        }

        if ( version >= 18.0 )
        {
            parseFocusPoint( msg, &next );
            msg = next;
        }
    }
    else if ( version >= 7.0
              && std::sscanf( msg,
                              "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                              " (stamina %lf %lf)"
                              " (speed %lf %lf) (head_angle %lf)"
                              " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)"
                              " (catch %d) (move %d) (change_view %d))",
                              &sense_time, vqual, vwidth,
                              &M_stamina, &M_effort,
                              &M_speed_mag, &M_speed_dir_relative,
                              &M_neck_relative,
                              &M_kick_count, &M_dash_count,
                              &M_turn_count, &M_say_count,
                              &M_turn_neck_count, &M_catch_count,
                              &M_move_count, &M_change_view_count) == 16 )
    {

    }
    else if ( version >= 6.0
              && std::sscanf( msg,
                              "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                              " (stamina %lf %lf)"
                              " (speed %lf %lf) (head_angle %lf)"
                              " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)",
                              &sense_time, vqual, vwidth,
                              &M_stamina, &M_effort,
                              &M_speed_mag, &M_speed_dir_relative,
                              &M_neck_relative,
                              &M_kick_count, &M_dash_count,
                              &M_turn_count, &M_say_count,
                              &M_turn_neck_count) == 13 )
    {

    }
    else if ( version >= 5.0
              && std::sscanf( msg,
                              "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                              " (stamina %lf %lf)"
                              " (speed %lf) (head_angle %lf)"
                              " (kick %d) (dash %d) (turn %d) (say %d) (turn_neck %d)",
                              &sense_time, vqual, vwidth,
                              &M_stamina, &M_effort,
                              &M_speed_mag,
                              &M_neck_relative,
                              &M_kick_count, &M_dash_count,
                              &M_turn_count, &M_say_count,
                              &M_turn_neck_count) == 12 )
    {

    }
    else if ( std::sscanf( msg,
                           "(sense_body %ld (view_mode %7[^ ] %7[^)])"
                           " (stamina %lf %lf)"
                           " (speed %lf)"
                           " (kick %d) (dash %d) (turn %d) (say %d)",
                           &sense_time, vqual, vwidth,
                           &M_stamina, &M_effort,
                           &M_speed_mag,
                           &M_kick_count, &M_dash_count,
                           &M_turn_count, &M_say_count) == 10 )
    {

    }
    else
    {
        std::cerr << "parse2 error" << std::endl;
        return;
    }



    switch ( vqual[0] ) {
    case 'h':
        M_view_quality = ViewQuality::HIGH;
        break;
    case 'l':
        M_view_quality = ViewQuality::LOW;
        //std::cerr << "CAUTION!! sense_body: view quality is LOW" << std::endl;
        break;
    default:
        std::cerr << "sense_body: Unknown View Quality" << std::endl;
        break;
    }

    switch ( vwidth[1] ) {
    case 'o':  // "normal"
        M_view_width = ViewWidth::NORMAL;
        break;
    case 'a':  // "narrow"
        M_view_width = ViewWidth::NARROW;
        break;
    case 'i':  // "wide"
        M_view_width = ViewWidth::WIDE;
        break;
    default:
        std::cerr << "sense_body: Unknown View Width" << std::endl;
        break;
    }
}
#endif

/*-------------------------------------------------------------------*/
bool
BodySensor::parseArm( const char * msg,
                      char ** next )
{
    int movable, expires, count;
    double dist, dir;
    int n_read = 0;
    if ( std::sscanf( msg,
                      " ( arm ( movable %d ) ( expires %d ) ( target %lf %lf ) ( count %d ) ) %n",
                      &movable, &expires, &dist, &dir, &count, &n_read ) != 5 )
    {
        std::cerr << M_time << " sense_body. illegal arm [" << msg << "]" << std::endl;
        return false;
    }

    M_arm_movable = movable;
    M_arm_expires = expires;
    M_pointto_dist = dist;
    M_pointto_dir = dir;
    M_pointto_count = count;

    *next = const_cast< char * >( msg ) + n_read;
    return true;
}

/*-------------------------------------------------------------------*/
bool
BodySensor::parseAttentionto( const char * msg,
                              char ** next )
{
    // `(focus (target <SIDE> [<UNUM>]) (count <COUNT>)'
    // <SIDE> := "none" | "l" | "r"

    char side[8];
    int unum = Unum_Unknown;
    int count = 0;
    int n_read = 0;

    if ( std::strncmp( "(focus ", msg, 7 ) )
    {
        std::cerr << "ERROR: " << M_time
                  << " (BodySensor::parseAttentionto)  [" << msg << "]" << std::endl;
        return false;
    }

    msg += 7;
    if ( std::sscanf( msg, " ( target %7[^)] %d ) %n", side, &unum, &n_read ) != 2
         && std::sscanf( msg, " ( target %7[^)] ) %n", side, &n_read ) != 1 )
    {
        std::cerr << "ERROR: " << M_time
                  << " (BodySensor::parseAttentionto)  [" << msg << "]" << std::endl;
        return false;
    }
    msg += n_read;

    if ( std::sscanf( msg, " ( count %d ) %n", &count, &n_read ) != 1 )
    {
        std::cerr << "ERROR: " << M_time
                  << " (BodySensor::parseAttentionto)  [" << msg << "]" << std::endl;
        return false;
    }
    msg += n_read;

    if ( side[0] == 'n' )
    {
        M_attentionto_side = NEUTRAL;
        M_attentionto_unum = Unum_Unknown;
    }
    else if ( side[0] == 'l' )
    {
        M_attentionto_side = LEFT;
        M_attentionto_unum = unum;
    }
    else if ( side[0] == 'r' )
    {
        M_attentionto_side = RIGHT;
        M_attentionto_unum = unum;
    }
    else
    {
        std::cerr << "ERROR: " << M_time
                  << " (BodySensor::parseAttentionto) Unknown side [" << side << "]"
                  << std::endl;
        return false;
    }

    M_attentionto_count = count;

    // skip to the next element
    while ( *msg != '\0' && *msg != '(' ) ++msg;
    *next = const_cast< char * >( msg );

    return true;
}

/*-------------------------------------------------------------------*/
bool
BodySensor::parseTackle( const char * msg,
                         char ** next )
{
    // `(tackle (expires <EXPIRES>) (count <COUNT>))'
    int expires, count;
    int n_read = 0;
    if ( std::sscanf( msg,
                      " ( tackle ( expires %d ) ( count %d ) ) %n",
                      &expires, &count, &n_read ) != 2 )
    {
        std::cerr << "ERROR: " << M_time
                  << " (BodySensor::parseTackle) [" << msg << "]" << std::endl;
        return false;
    }

    M_tackle_expires = expires;
    M_tackle_count = count;

    *next = const_cast< char * >( msg ) + n_read;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
BodySensor::parseCollision( const char * msg,
                            char ** next )
{
    // (collision {none|[(ball)][(player)][(post)]})

    if ( std::strncmp( "(collision ", msg, 11 ) )
    {
        std::cerr << M_time << " sense_body. illegal collision tag ["
                  << msg << "]" << std::endl;
        return false;
    }

    M_none_collided = false;
    M_ball_collided = false;
    M_player_collided = false;
    M_post_collided = false;

    msg += 11;
    //while ( *msg != '\0' && *msg != '(' ) ++msg;
    //while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(collision"
    while ( *msg == ' ' ) ++msg; // skip space

    if ( ! std::strncmp( msg, "none", 4 ) )
    {
        M_none_collided = true;

        if ( next )
        {
            while ( *msg != '\0' && *msg != '(' ) ++msg;
            *next = const_cast< char * >( msg );
        }
        return true;
    }

    int n_read = 0;
    char name[16];
    while ( *msg != '\0' && *msg != ')' )
    {
        if ( std::sscanf( msg, " ( %15[^()] ) %n ",
                          name, &n_read ) != 1 )
        {
            break;
        }
        msg += n_read;

        if ( ! std::strcmp( "ball", name ) )
        {
            M_ball_collided = true;
        }
        else if ( ! std::strcmp( "player", name ) )
        {
            M_player_collided = true;
        }
        else if ( ! std::strcmp( "post", name ) )
        {
            M_post_collided = true;
        }
        else
        {
            std::cerr << M_time << " sense_body. Unknown collision type ["
                      << name << "]" << std::endl;
        }
    }

    if ( next )
    {
        while ( *msg != '\0' && *msg != '(' ) ++msg;
        *next = const_cast< char * >( msg );
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
BodySensor::parseFoul( const char * msg,
                       char ** next )
{
    // (foul (charged 0) (card {none|yellow|red}))

    int n_read = 0;

    while ( *msg != '\0' && *msg != '(' ) ++msg;
    while ( *msg != '\0' && *msg != ' ' ) ++msg; // skip "(foul"

    int cycles = 0;

    if ( std::sscanf( msg, " ( charged %d ) %n ",
                      &cycles, &n_read ) != 1
         || n_read == 0 )
    {
        std::cerr << M_time << " sense_body. illegal foul charge expires ["
                  << msg << "]" << std::endl;
        return false;
    }
    msg += n_read;
    n_read = 0;

    M_charged_expires = cycles;

    char card[8];

    if ( std::sscanf( msg, " ( card %7[^)] ) %n",
                      card, &n_read ) != 1
         || n_read == 0 )
    {
        std::cerr << M_time << " sense_body. illegal card information ["
                  << msg << "]" << std::endl;
        return false;
    }
    msg += n_read;

    if ( ! std::strcmp( card, "none" ) )
    {
        M_card = NO_CARD;
    }
    else if ( ! std::strcmp( card, "yellow" ) )
    {
        M_card = YELLOW;
    }
    else if ( ! std::strcmp( card, "red" ) )
    {
        M_card = RED;
    }
    else
    {
        std::cerr << M_time << " sense_body. unknown card type ["
                  << card << "]" << std::endl;
        M_card = NO_CARD;
    }

    while ( *msg == ' ' ) ++msg;
    if ( *msg == ')' ) ++msg;

    if ( next )
    {
        *next = const_cast< char * >( msg );
    }

    return true;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
BodySensor::parseFocusPoint( const char * msg,
                             char ** next )
{
    // (focus_point <REAL> <REAL>)

    double focus_dist = 0.0;
    double focus_dir = 0.0;
    int n_read = 0;

    if ( std::sscanf( msg, " (focus_point %lf %lf) %n", &focus_dist, &focus_dir, &n_read ) != 2 )
    {
        std::cerr << M_time << " ERROR: Illegal focus_point in sense_body [" << msg << "]" << std::endl;
        return false;
    }

    M_focus_dist = focus_dist;
    M_focus_dir = focus_dir;

    if ( next )
    {
        *next = const_cast< char * >( msg ) + n_read;
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
BodySensor::print( std::ostream & os ) const
{
    std::string side_str;
    switch ( M_attentionto_side ) {
    case LEFT:
        side_str = "left";
        break;
    case RIGHT:
        side_str = "right";
        break;
    default:
        side_str = "neutral";
        break;
    }

    os <<  "sense_body" << M_time
       << "\n view_quality: " << M_view_quality.str()
       << "\n view_width: " << M_view_width.str()
       << "\n stamina: " << M_stamina
       << "\n effort: " << M_effort
       << "\n speed-mag: " << M_speed_mag
       << "\n speed-dir: " << M_speed_dir_relative
       << "\n neck_angle: " << M_neck_relative
       << "\n"
       << "\n kick:  " << M_kick_count
       << "\n dash:  " << M_dash_count
       << "\n turn:  " << M_turn_count
       << "\n say:   " << M_say_count
       << "\n turn_neck: " << M_turn_neck_count
       << "\n catch: " << M_catch_count
       << "\n move:  " << M_move_count
       << "\n change_view: " << M_change_view_count
       << "\n attentionto: " << M_attentionto_count
       << "\n pointto: " << M_pointto_count
       << "\n tackle: " << M_tackle_count
       << "\n"
       << "\n arm-movable: " << M_arm_movable
       << "\n arm-expire:  " << M_arm_expires
       << "\n pointto-dist: " << M_pointto_dist
       << "\n pointto-dir:  " << M_pointto_dir
       << "\n"
       << "\n attentionto-side: " << side_str
       << "\n attentionto-num: " << M_attentionto_unum
       << "\n tackle-expires: " << M_tackle_expires

       << std::endl;

    return os;
}

}
