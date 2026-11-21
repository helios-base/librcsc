// -*-c++-*-

/*!
  \file body_clear_ball2007.cpp
  \brief kick the ball to escape from a dangerous situation
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

#include "body_clear_ball2007.h"

//#include "intention_kick.h"
#include "body_kick_one_step.h"
//#include "body_kick_two_step.h"
//#include "body_kick_multi_step.h"
#include "body_smart_kick.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/line_2d.h>

namespace rcsc {

const double Body_ClearBall2007::S_SEARCH_ANGLE = 8.0;

GameTime Body_ClearBall2007::S_last_calc_time( 0, 0 );
AngleDeg Body_ClearBall2007::S_cached_best_angle = 0.0;

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_ClearBall2007::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  "%s:%d: Body_ClearBall2007"
                  ,__FILE__, __LINE__ );

    if ( ! agent->world().self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      "%s:%d:  not kickable"
                      ,__FILE__, __LINE__ );
        return false;
    }

    if ( S_last_calc_time != agent->world().time() )
    {
        dlog.addText( Logger::CLEAR,
                      "%s:%d: update clear angle"
                      ,__FILE__, __LINE__ );

        double lower_angle, upper_angle;

        if ( agent->world().self().pos().y > ServerParam::i().goalHalfWidth() - 1.0 )
        {
            lower_angle = 0.0;
            upper_angle = 90.0;
        }
        else if ( agent->world().self().pos().y < -ServerParam::i().goalHalfWidth() + 1.0 )
        {
            lower_angle = -90.0;
            upper_angle = 0.0;
        }
        else
        {
            lower_angle = -60.0;
            upper_angle = 60.0;
        }

        S_cached_best_angle = get_best_angle( agent,
                                              lower_angle,
                                              upper_angle,
                                              ! agent->world().self().goalie() );
        S_last_calc_time = agent->world().time();
    }


    const Vector2D target_point
        = agent->world().self().pos()
        + Vector2D::polar2vector(30.0, S_cached_best_angle);

    dlog.addText( Logger::TEAM,
                  "%s:%d: clear angle = %f"
                  ,__FILE__, __LINE__,
                  S_cached_best_angle.degree() );

    agent->debugClient().setTarget( target_point );

    if ( agent->world().self().goalie()
         || agent->world().gameMode().type() == GameMode::GoalKick_ )
    {
        Body_KickOneStep( target_point,
                          ServerParam::i().ballSpeedMax()
                          ).execute( agent );
        agent->debugClient().addMessage( "Clear" );
        dlog.addText( Logger::TEAM,
                      "%s:%d: goalie or goal_kick register clear kick. one step."
                      ,__FILE__, __LINE__ );
    }
    else
    {
        Vector2D one_step_vel
            = Body_KickOneStep::get_max_possible_vel
            ( ( target_point - agent->world().ball().pos() ).th(),
              agent->world().self().kickRate(),
              agent->world().ball().vel() );

        if ( one_step_vel.r() > 2.0 )
        {
            Body_KickOneStep( target_point,
                              ServerParam::i().ballSpeedMax()
                              ).execute( agent );
            agent->debugClient().addMessage( "Clear1K" );
            return true;
        }

        agent->debugClient().addMessage( "ClearS" );
        Body_SmartKick( target_point,
                        ServerParam::i().ballSpeedMax(),
                        ServerParam::i().ballSpeedMax() * 0.85,
                        2 ).execute( agent );
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  lower < uppeer
*/
AngleDeg
Body_ClearBall2007::get_best_angle( const PlayerAgent * agent,
                                    const double & lower_angle,
                                    const double & upper_angle,
                                    const bool clear_mode )
{
    double scan_range = upper_angle - lower_angle;
    double scan_dec = 8.0;
    double scan_count = rint( scan_range / scan_dec );
    scan_dec = scan_range / scan_count;

    AngleDeg best_angle = 0.0;
    double best_score = 0.0;
    do
    {
        AngleDeg tmp_angle = upper_angle - scan_range;

        double tmp_score = calc_score( agent, tmp_angle );
        if ( clear_mode )
        {
            tmp_score *= ( 0.5
                           * ( AngleDeg::sin_deg(1.5 * tmp_angle.abs() + 45.0 )
                               + 1.0) );
        }
        int dir_count = agent->world().dirCount( tmp_angle ) - 3;
        if ( dir_count < 0 ) dir_count = 0;
        tmp_score *= std::pow( 0.95, dir_count );

        if ( tmp_score > best_score )
        {
            best_angle = tmp_angle;
            best_score = tmp_score;
        }
        scan_range -= scan_dec;

        dlog.addText( Logger::CLEAR,
                      "Body_ClearBall2007.get_best_angle. search_angle=%f, score=%f",
                      tmp_angle.degree(), tmp_score );

    }
    while ( scan_range >= 0.0 );

    return best_angle;
}

/*-------------------------------------------------------------------*/
/*!

 */
double
Body_ClearBall2007::calc_score( const PlayerAgent * agent,
                                const AngleDeg & target_angle )
{
    double score = 1.0;

    const Line2D angle_line( agent->world().self().pos(),
                             target_angle );

    const AngleDeg target_left_angle = target_angle - 30.0;
    const AngleDeg target_right_angle = target_angle + 30.0;

    const PlayerPtrCont::const_iterator end
        = agent->world().opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator
              it = agent->world().opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->angleFromSelf().isWithin( target_left_angle,
                                              target_right_angle ) )
        {
            Vector2D project_point = angle_line.projection( (*it)->pos() );
            double width = (*it)->pos().dist( project_point );
            double dist = agent->world().self().pos().dist( project_point );
            score *= width / dist;
        }
    }

    return score;
}

}
