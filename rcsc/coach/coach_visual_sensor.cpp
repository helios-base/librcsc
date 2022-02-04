// -*-c++-*-

/*!
  \file coach_visual_sensor.cpp
  \brief coach visual sensor data holder class Source File
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

#include "coach_visual_sensor.h"

#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
CoachVisualSensor::CoachVisualSensor()
    : M_time( -1, 0 )
{
    M_players.reserve( 22 );
}

/*-------------------------------------------------------------------*/
/*!

 */
CoachVisualSensor::~CoachVisualSensor()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachVisualSensor::parse( const char * msg,
                           const double & version,
                           const GameTime & current )
{
    /*
      if version >= 7.0 then identifier = "(see_global" else "(see"
      if version >= 7.0 then object name is short type.

      if coach use a "look" command, "see_global" is replaced by "ok look"
    */

    M_time = current;

    M_players.clear();

    if ( version >= 7.0 )
    {
        parseV7( msg );
    }
    else
    {
        parseV6( msg );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachVisualSensor::parseV7( const char * msg )
{
    //(see_global 0 ((g l) -52.5 0) ((g r) 52.5 0) ((b) 0 0 0 0) ((p "RoboSoc" 1) -10 -10 0 0 24 0))
    /*
    sample version >= 7.0
    (see_global TIME ((g l) -52.5 0) ((g r) 52.5 0) ((b) <x> <y> <vx> <vy>)
       ((p "TEAM" UNUM[ goalie]) <x> <y> <vx> <vy> <body> <neck>[ <arm>][ {t|k|f}][ {y|r}])
       ....)
        <-- arm is global
        <-- 't' means tackle
        <-- 'k' means kick
        <-- 'f' means foul charged
        <-- 'y' means yellow card
        <-- 'r' means red card
    (ok look TIME ((g l) -52.5 0) ((g r) 52.5 0) ((b) <x> <y> <vx> <vy>)
       ((p "TEAM" UNUM[ goalie]) <x> <y> <vx> <vy> <body> <neck>) <-- no arm & tackle
       ....)
   */

    // temporal variables
    double x, y, vx, vy, body, neck;

    if ( std::strncmp( msg, "(see_global ", 12 ) )
    {
        msg += 12;
    }
    else if ( std::strncmp( msg, "(ok look ", 9 ) )
    {
        msg += 9;
    }
    else
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << "***ERROR*** invalide message(1) " << msg << std::endl;
        return;
    }

    while ( *msg != ' ' ) ++msg;
    ++msg; // skip TIME

    // skip goals
    while ( *msg != ')' ) ++msg;
    ++msg;
    while ( *msg != ')' ) ++msg;
    ++msg; // skip ((g l) -52.5 0)
    while ( *msg != ')' ) ++msg;
    ++msg;
    while ( *msg != ')' ) ++msg;
    ++msg; // ((g r) 52.5 0)

    int n_read = 0;

    // ((b) <x> <y> <vx> <vy>)
    if ( std::sscanf( msg,
                      " ((b) %lf %lf %lf %lf) %n ",
                      &x, &y, &vx, &vy,
                      &n_read ) != 4 )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << "***ERROR*** invalide message(2) " << msg << ']'
                  << std::endl;
        return;
    }

    M_ball.setValue( x, y, vx, vy );

    msg += n_read;

    std::string team_name_left;
    std::string team_name_right;

    // ((p "TEAM" UNUM[ goalie]) <x> <y> <vx> <vy> <body> <neck>[ <arm>][ {k|t}][ {y|r}])
    while ( *msg != '\0' )
    {
        while ( *msg != '\0' && *msg != '(' ) ++msg;
        if ( *msg == '\0' )
        {
            break;
        }

        CoachPlayerObject player;

        char teamname[32];
        int unum = Unum_Unknown;
        bool goalie = false;

        if ( std::sscanf( msg,
                          " ((p %31[^ ] %d %n ",
                          teamname, &unum,
                          &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << "***ERROR*** invalide message(3) [" << msg << ']'
                      << std::endl;
            break;
        }

        SideID side = NEUTRAL;
        if ( team_name_left == teamname )
        {
            side = LEFT;
        }
        else if ( team_name_right == teamname )
        {
            side = RIGHT;
        }
        else if ( team_name_left.empty() )
        {
            team_name_left = teamname;
            side = LEFT;
        }
        else if ( team_name_right.empty() )
        {
            team_name_right = teamname;
            side = RIGHT;
        }
        else
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << "***ERROR*** invalide message(4) [" << msg << ']'
                      << std::endl;
            break;
        }

        msg += n_read;

        if ( *msg == 'g' )
        {
            goalie = true;
        }

        // skip to last of the player name
        while ( *msg != '\0' && *msg != ')' ) ++msg;
        ++msg; // skip ')'

        if ( std::sscanf( msg,
                          " %lf %lf %lf %lf %lf %lf %n ",
                          &x, &y, &vx, &vy, &body, &neck, &n_read ) != 6 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << "***ERROR*** invalide message(5) [" << msg << ']'
                      << std::endl;
            break;
        }

        player.setTeam( side, unum, goalie );
        player.setPos( x, y );
        player.setVel( vx, vy );
        player.setAngle( body, neck );

        msg += n_read;

        while ( *msg != '\0' && *msg != ')' )
        {
            while ( *msg == ' ' ) ++msg;
            if ( *msg == ')' ) break;

            if ( *msg == 'k' ) // kick
            {
                player.setKicking( true );
            }
            else if ( *msg == 't' ) // tackle
            {
                player.setTackle();
            }
            else if ( *msg == 'f' ) // foul charged
            {
                player.setCharged();
            }
            else if ( *msg == 'y' ) // yellow card
            {
                player.setCard( YELLOW );
            }
            else if ( *msg == 'r' ) // red card
            {
                player.setCard( RED );
            }
            else // point_dir
            {
                char *next;
                double point_dir = std::strtod( msg, &next );
                if ( point_dir == HUGE_VAL
                     || point_dir == -HUGE_VAL
                     || msg == next )
                {
                    break;
                }
                player.setArm( point_dir );
                msg = next;
            }

            while ( *msg != '\0' && *msg != ')' && *msg != ' ' ) ++msg;
        }

        M_players.push_back( player );

        // skip to the last paren of the player info
        while ( *msg != '\0' && *msg != ')' ) ++msg;
        while ( *msg == ')' ) ++msg;
    }

    if ( M_team_name_left.empty()
         && team_name_left.length() > 2 )
    {
        M_team_name_left = team_name_left.substr( 1, team_name_left.length() - 2 );
    }

    if ( M_team_name_right.empty()
         && team_name_right.length() > 2 )
    {
        M_team_name_right = team_name_right.substr( 1, team_name_right.length() - 2 );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachVisualSensor::parseV6( const char * msg )
{
    /*
      older version
      (see TIME ((goal l) -52.5 0) ((goal r) 52.5 0) ((ball) <x> <y> <vx> <vy>)
      ((player "TEAM" UNUM[ goalie]) <x> <y> <body> <neck> <vx> <vy>)
      ....)
      (ok look TIME ((g l) -52.5 0) ((g r) 52.5 0) ((b) <x> <y> <vx> <vy>)
      ((player "TEAM" UNUM[ goalie]) <x> <y> <body> <neck> <vx> <vy>)
      ....)
    */

    double x, y, vx, vy, body, neck;

    if ( std::strncmp( msg, "(see ", 5 ) )
    {
        msg += 5;
    }
    else if ( std::strncmp( msg, "(ok look ", 9 ) )
    {
        msg += 9;
    }
    else
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << "***ERROR*** invalide message(1) " << msg << ']'
                  << std::endl;
        return;
    }

    while ( *msg != ' ' ) ++msg;
    ++msg; // skip TIME

    // skip goals
    while ( *msg != ')' ) ++msg;
    ++msg;
    while ( *msg != ')' ) ++msg;
    ++msg; // skip ((g l) -52.5 0)
    while ( *msg != ')' ) ++msg;
    ++msg;
    while ( *msg != ')' ) ++msg;
    ++msg; // ((g r) 52.5 0)

    int n_read = 0;

    // ((b) <x> <y> <vx> <vy>)
    if ( std::sscanf( msg,
                      " ((ball) %lf %lf %lf %lf) %n ",
                      &x, &y, &vx, &vy,
                      &n_read ) != 4 )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << "***ERROR*** invalide message(2) " << msg << ']'
                  << std::endl;
        return;
    }

    M_ball.setValue( x, y, vx, vy );

    msg += n_read;

    std::string team_name_left;
    std::string team_name_right;

    // ((player "TEAM" UNUM[ goalie]) <x> <y> <body> <neck> <vx> <vy>)
    while ( *msg != '\0' )
    {
        while ( *msg != '\0' && *msg != '(' ) ++msg;
        if ( *msg == '\0' )
        {
            break;
        }

        CoachPlayerObject player;

        char teamname[32];
        int unum = 0;
        bool goalie = false;

        if ( std::sscanf( msg,
                          " ((player %31[^ ] %d %n ",
                          teamname, &unum ,
                          &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << "***ERROR*** invalide message(3) [" << msg << ']'
                      << std::endl;
            break;
        }

        SideID side = NEUTRAL;
        if ( team_name_left == teamname )
        {
            side = LEFT;
        }
        else if ( team_name_right == teamname )
        {
            side = RIGHT;
        }
        else if ( team_name_left.empty() )
        {
            team_name_left = teamname;
            side = LEFT;
        }
        else if ( team_name_right.empty() )
        {
            team_name_right = teamname;
            side = RIGHT;
        }
        else
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << "***ERROR*** invalide message(4) [" << msg << ']'
                      << std::endl;
            break;
        }

        msg += n_read;

        if ( *msg == 'g' )
        {
            goalie = true;
        }

        while ( *msg != ')' ) ++msg; // skip to last of the player name
        ++msg; // skip ')'

        if ( std::sscanf( msg,
                          " %lf %lf %lf %lf %lf %lf %n ",
                          &x, &y, &vx, &vy, &body, &neck, &n_read ) != 6 )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << "***ERROR*** invalide message(5) [" << msg << ']'
                      << std::endl;
            break;
        }
        msg += n_read;

        player.setTeam( side, unum, goalie );
        player.setPos( x, y );
        player.setVel( vx, vy );
        player.setAngle( body, neck );

        M_players.push_back( player );


        // skip to the last paren of the player info
        while ( *msg != '\0' && *msg != ')' ) ++msg;
        while ( *msg == ')' ) ++msg;
    }

    if ( ! team_name_left.empty() )
    {
        M_team_name_left = team_name_left;
    }

    if ( ! team_name_right.empty() )
    {
        M_team_name_right = team_name_right;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CoachVisualSensor::print( std::ostream & os ) const
{
    os << "Coach Visual " << M_time
       << " ball: " << M_ball.pos() << M_ball.vel() << std::endl;

    for ( const CoachPlayerObject & p : M_players )
    {
        p.print( os ) << '\n';
    }

    return os << std::flush;
}

}
