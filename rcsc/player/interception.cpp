// -*-c++-*-

/*!
  \file interception.cpp
  \brief interception utility class Source File
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

#include "interception.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

#include <algorithm>
#include <iostream>
#include <cmath>

//#define TRACE_NEWTON

namespace rcsc {

const double Interception::NEVER_CYCLE = 1000.0;


namespace {

/*-------------------------------------------------------------------*/
/*!
  \brief get the value of log(ball_decay)
  accessible only from this file.
*/
inline
const
double & logBallDecay()
{
    static const double LOG_BALL_DECAY = std::log( ServerParam::i().ballDecay() );
    return LOG_BALL_DECAY;
}

};

/*-------------------------------------------------------------------*/
/*!

*/
Interception::Interception( const Vector2D & ball_pos,
                            const double & ball_first_speed,
                            const AngleDeg & ball_vel_angle )
    : M_ball_first_pos( ball_pos )
    , M_ball_first_speed( ball_first_speed )
    , M_ball_vel_angle( ball_vel_angle )
    , M_ball_x_constant( ball_first_speed
                         / (1.0 - ServerParam::i().ballDecay()) )
    , M_ball_x_d_constant( (-ball_first_speed * logBallDecay())
                           / (1.0 - ServerParam::i().ballDecay()) )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Interception::Interception( const Vector2D & ball_pos,
                            const Vector2D & ball_vel )
    : M_ball_first_pos( ball_pos )
    , M_ball_first_speed( ball_vel.r() )
    , M_ball_vel_angle( ball_vel.th() )
    , M_ball_x_constant( M_ball_first_speed
                         / (1.0 - ServerParam::i().ballDecay()) )
    , M_ball_x_d_constant( (-M_ball_first_speed * logBallDecay())
                           / (1.0 - ServerParam::i().ballDecay()) )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
double
Interception::getReachCycle( const Vector2D & player_pos,
                             const Vector2D * player_vel,
                             const AngleDeg * /*player_angle*/,
                             const int player_count,
                             const double & control_buf,
                             const double & player_max_speed ) const
{
    static const int MAX_LOOP = 20;
    static const double MIN_ERROR = 0.05;

    // player related param
    /*
      const double player_max_speed
      = ( player_type
      ? player_type->playerSpeedMax()
      : ServerParam::i().defaultPlayerSpeedMax() );
      const double control_buf
      = ( goalie
      ? ServerParam::i().catchAreaLength()
      : ( player_type
      ? ( ServerParam::i().ballSize()
      + player_type->playerSize()
      + player_type->kickableMargin() )
      : ServerParam::i().defaultKickableArea() ) );
    */

    // relative to ball pos and ball velocity angle
    Vector2D start_point = player_pos;
    start_point -= M_ball_first_pos;

    ///////////////////////////////////////////
    // check kickable state at current
    if ( start_point.r2() < square( control_buf ) )
    {
#ifdef TRACE_NEWTON
        cout << "  Newton: ZERO" << endl;
#endif
        return 0.0;
    }


    // rotation
    start_point.rotate( - M_ball_vel_angle );


    // consider player confidence
    if ( player_count == 0 )
    {
        // if player is almost all stopped,
        // it is necessary for him to accelarate his dash speed at first.
        if ( player_vel && player_vel->r() < 0.1 )
        {
            if ( start_point.absY() > control_buf )
            {
                start_point.y += player_max_speed * ( start_point.y > 0.0 ? 1.0 : -1.0 );
            }
            else
            {
                start_point.x += player_max_speed * ( start_point.x > 0.0 ? 1.0 : -1.0 );
            }
        }
    }
    else
    {
        double virtual_dash = player_max_speed * 0.8;
        virtual_dash *= std::min(5, player_count);
        if ( start_point.absY() > virtual_dash )
        {
            start_point.y -= virtual_dash * ( start_point.y > 0.0 ? 1.0 : -1.0 );
        }
        else
        {
            double x_dash = std::sqrt( square( virtual_dash )
                                       - square( start_point.y ) );
            start_point.y = 0.0;
            if ( start_point.absX() > x_dash )
            {
                start_point.x -= x_dash * ( start_point.x > 0.0 ? 1.0 : -1.0 );
            }
            else
            {
                start_point.x = 0.0;
            }
        }
    }



    /////////////////////////////////////////////////////////////////////////
    // Newton method

    const double start_y2 = square( start_point.y );

    const double bdecay = ServerParam::i().ballDecay();

    // t: cycles to reach the ball
    // f: distance(ball_pos, player_first_pos) - (player_total_dash)
    // f_d: differential of f

    double t = 0.0;
    double f, f_d;
    int counter = 0;
    do
    {
        counter++;
        //double ball_x
        //    = util::inertia_n_step_distance( M_ball_first_speed, t, bdecay );
        double ball_x = M_ball_x_constant * ( 1.0 - std::pow( bdecay, t) );
        //double ball_x_d
        //    = (-M_ball_first_speed * std::pow( bdecay, t ) * LOG_BALL_DECAY )
        //    / (1.0 - ServerParam::i().ballDecay() );
        double ball_x_d = M_ball_x_d_constant * std::pow( ServerParam::i().ballDecay(), t );
        // distance to ball at t from player first pos
        double dist_to_ball = std::sqrt( square( ball_x - start_point.x )
                                         + start_y2 );
        f = dist_to_ball - player_max_speed * t - control_buf;
        f_d = (ball_x - start_point.x) * ball_x_d / dist_to_ball - player_max_speed;
        if ( ball_x < start_point.x && f_d != 0.0 )
        {
            //t = t - f / f_d;
            t -= f / f_d;
#ifdef TRACE_NEWTON
            cout << "  Newton:" << counter << ": update_1 cyc=" << t
                 << "  f=" << f
                 << "  f_d=" << f_d
                 << endl;
#endif
        }
        else if ( ball_x > start_point.x && f_d < 0.0 )
        {
            t -= f / f_d;
#ifdef TRACE_NEWTON
            cout << "  Newton:" << counter << ":  update_2 cyc=" << t
                 << "  f=" << f
                 << "  f_d=" << f_d
                 << endl;
#endif
        }
        else
        {
            t += f / player_max_speed;
#ifdef TRACE_NEWTON
            cout << "  Newton:" << counter << ":  update_3 cyc=" << t
                 << "  f=" << f
                 << "  f_d=" << f_d;
#endif
            // if differential of f is positive value,
            // ball is moving far away from player faster than player's speed.
            // And if f_d is small, Newton method required many loop.
            // so, it is good idea to add the cycles enforcely.
            if ( f_d > 0.0
                 // || ( f > 0.01 && fabs(f_d) < 0.01 )
                 )
            {
                t += 10.0; // XXX Magic Number XXX
#ifdef TRACE_NEWTON
                cout << "  update enforcely. cyc=" << t;
#endif
            }
#ifdef TRACE_NEWTON
            cout << endl;
#endif
        }

        if ( fabs(f) < MIN_ERROR )
        {
            break;
        }
    }
    while ( counter < MAX_LOOP );


    if ( std::fabs(f) > MIN_ERROR )
    {
        return NEVER_CYCLE;
    }

    return t;
}

/*-------------------------------------------------------------------*/
/*!
  testing purpose
*/
double
Interception::getReachCycleGreedly( const Vector2D & player_pos,
                                    const Vector2D * /*player_vel*/,
                                    const AngleDeg * /*player_angle*/,
                                    const int /*player_count*/,
                                    const double & control_buf,
                                    const double & player_max_speed ) const
{
    // player related param
    /*
      const double player_max_speed
      = ( player_type
      ? player_type->playerSpeedMax()
      : ServerParam::i().defaultPlayerSpeedMax() );
      const double control_buf
      = ( goalie
      ? ServerParam::i().catchAreaLength()
      : ( player_type
      ? ( ServerParam::i().ballSize()
      + player_type->playerSize()
      + player_type->kickableMargin() )
      : ServerParam::i().defaultKickableArea() ) ) - 0.05;
    */

    Vector2D ball_pos( M_ball_first_pos );
    Vector2D ball_vel
        = Vector2D::polar2vector( M_ball_first_speed, M_ball_vel_angle );

    Vector2D start_point = player_pos;
    start_point -= M_ball_first_pos;
    start_point.rotate( -M_ball_vel_angle );
    const double player_cyc_to_intersect
        = std::max(0.0,
                   std::ceil( ( start_point.absY() - control_buf * 0.5 )
                              / player_max_speed )
                   );
    double cycle = std::max( 0.0, player_cyc_to_intersect - 1.0 );
    double player_dash = player_max_speed * cycle;
    ball_pos = inertia_n_step_point( ball_pos,
                                     ball_vel,
                                     (int)cycle,
                                     ServerParam::i().ballDecay() );
    ball_vel *= std::pow( ServerParam::i().ballDecay(), cycle );
#ifdef TRACE_NEWTON
    cout << "  Greedy: start=" << cycle << endl;
#endif
    while ( cycle < 100.0 )
    {
        if ( player_pos.dist2( ball_pos ) < square( player_dash + control_buf ) )
        {
#ifdef TRACE_NEWTON
            cout << "  Greedy: " << cycle
                 << "  dist_to_ball: " << player_pos.dist(ball_pos)
                 << "  player_dash+ctrl: " << player_dash + control_buf
                 << endl;
#endif
            return cycle;
        }

        player_dash += player_max_speed;
        ball_pos += ball_vel;
        ball_vel *= ServerParam::i().ballDecay();
        cycle += 1.0;
    }


    return NEVER_CYCLE;
}

}
