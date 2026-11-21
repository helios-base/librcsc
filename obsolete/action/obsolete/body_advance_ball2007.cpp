// -*-c++-*-

/*!
  \file body_advance_ball2007.cpp
  \brief kick the ball to forward direction to an avoid opponent
  player's interfare.
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

#include "body_advance_ball2007.h"

//#include "intention_kick.h"
#include "body_clear_ball2007.h"
#include "body_kick_one_step.h"
//#include "body_kick_two_step.h"
#include "body_smart_kick.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

GameTime Body_AdvanceBall2007::S_last_calc_time( 0, 0 );
AngleDeg Body_AdvanceBall2007::S_cached_best_angle = 0.0;

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_AdvanceBall2007::execute( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    dlog.addText( Logger::TEAM,
                  __FILE__": Body_AdvanceBall2007" );

    if ( ! wm.self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": Body_AdvanceBall2007. not kickable" );
        return false;
    }

    if ( S_last_calc_time != wm.time() )
    {
        dlog.addText( Logger::CLEAR,
                      __FILE__": update" );
        S_cached_best_angle = getBestAngle( agent );
        S_last_calc_time = wm.time();
    }


    const Vector2D target_point
        = wm.self().pos()
        + Vector2D::polar2vector( 30.0, S_cached_best_angle );

    dlog.addText( Logger::CLEAR,
                  __FILE__": target_angle=%.1f",
                  S_cached_best_angle.degree() );
    agent->debugClient().setTarget( target_point );
    agent->debugClient().addLine( wm.ball().pos(), target_point );

    if ( wm.gameMode().type() != GameMode::PlayOn )
    {
        agent->debugClient().addMessage( "Advance1K" );
        Body_KickOneStep( target_point,
                          ServerParam::i().ballSpeedMax()
                          ).execute( agent );
        return true;
    }

    Vector2D one_step_vel
        = Body_KickOneStep::get_max_possible_vel
        ( ( target_point - wm.ball().pos() ).th(),
          wm.self().kickRate(),
          wm.ball().vel() );

    if ( one_step_vel.r() > 2.1 )
    {
        Body_KickOneStep( target_point,
                          ServerParam::i().ballSpeedMax()
                          ).execute( agent );
        agent->debugClient().addMessage( "Advance1K" );
        return true;
    }

    agent->debugClient().addMessage( "Advance" );

    Body_SmartKick( target_point,
                    ServerParam::i().ballSpeedMax(),
                    ServerParam::i().ballSpeedMax() * 0.9,
                    3 ).execute( agent );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
AngleDeg
Body_AdvanceBall2007::getBestAngle( const PlayerAgent* agent )
{
    const BallObject & ball = agent->world().ball();

    const double max_dist = 30.0;

    Vector2D left_limit( 0.0,
                         -ServerParam::i().pitchHalfWidth() + 3.0 );
    double ydiff = std::fabs( left_limit.y - ball.pos().y );
    if ( ydiff < max_dist )
    {
        left_limit.x = ball.pos().x + std::sqrt( max_dist * max_dist
                                                 - ydiff * ydiff );
    }
    if ( left_limit.x > 50.0 )
    {
        left_limit.x = 50.0;
    }

    Vector2D right_limit( 0.0,
                          ServerParam::i().pitchHalfWidth() - 5.0 );
    ydiff = std::fabs( right_limit.y - ball.pos().y );
    if ( ydiff < max_dist )
    {
        right_limit.x = ball.pos().x + std::sqrt( max_dist * max_dist
                                                  - ydiff * ydiff );
    }
    if ( right_limit.x > 50.0 )
    {
        right_limit.x = 50.0;
    }

    double lower_angle = ( left_limit - ball.pos() ).th().degree();
    if ( lower_angle < -45.0 ) lower_angle = -45.0;
    double upper_angle = ( right_limit - ball.pos() ).th().degree();
    if ( upper_angle > 45.0 ) upper_angle = 45.0;

    if ( lower_angle > upper_angle )
    {
        dlog.addText( Logger::CLEAR,
                      __FILE__": getBestAngle. angle_error. lower=%.1f, uppser=%.1f",
                      lower_angle, upper_angle );
        return AngleDeg( 0.0 );
    }

    dlog.addText( Logger::CLEAR,
                  __FILE__": getBestAngle. left(%.1f %.1f)lower_angle=%.1f right(%.1f %.1f)upper_angle=%.1f",
                  left_limit.x, left_limit.y, lower_angle,
                  right_limit.x, right_limit.y, upper_angle );

    return Body_ClearBall2007::get_best_angle( agent,
                                               lower_angle, upper_angle,
                                               false );
}

}
