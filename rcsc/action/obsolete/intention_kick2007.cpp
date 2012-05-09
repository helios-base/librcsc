// -*-c++-*-

/*!
  \file intention_kick2007.cpp
  \brief queued kick behavior.
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

#include "intention_kick2007.h"

#include "basic_actions.h"
#include "body_kick_one_step.h"
#include "body_kick_two_step.h"
#include "body_kick_multi_step.h"
#include "body_hold_ball.h"
#include "neck_turn_to_low_conf_teammate.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
IntentionKick::IntentionKick( const Vector2D & target_point,
                              const double & first_speed,
                              const int kick_step,
                              const bool enforce_kick,
                              const GameTime & start_time )
    : M_target_point( target_point )
    , M_first_speed( std::min( first_speed, ServerParam::i().ballSpeedMax() ) )
    , M_kick_step( kick_step )
    , M_enforce_kick( enforce_kick )
    , M_last_execute_time( start_time )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
bool
IntentionKick::finished( const PlayerAgent * agent )
{
    if ( M_kick_step <= 0 )
    {
        dlog.addText( Logger::KICK,
                      __FILE__": kick intention finished" );
        return true;
    }

    if ( ! agent->world().self().isKickable() )
    {
        return true;
    }

    if ( M_last_execute_time.cycle() + 1 != agent->world().time().cycle() )
    {
        return true;
    }

    if ( agent->world().existKickableOpponent() )
    {
        return true;
    }

    // check near opponents
    const double kickable_plus = ServerParam::i().defaultKickableArea() + 0.1;

    const PlayerPtrCont::const_iterator o_end = agent->world().opponentsFromBall().end();
    for ( PlayerPtrCont::const_iterator it = agent->world().opponentsFromBall().begin();
          it != o_end;
          ++it )
    {
        if ( (*it)->distFromBall() > 5.0 )
        {
            break;
        }

        // check opponent kickable current ball pos
        if ( (*it)->distFromBall() < kickable_plus )
        {
            return true;
        }
        // check opponent goalie
        else if ( (*it)->goalie()
                  && (*it)->pos().x > ServerParam::i().theirPenaltyAreaLineX() + 1.5
                  && (*it)->pos().absY() < ServerParam::i().penaltyAreaHalfWidth() - 1.5
                  && (*it)->distFromBall() < ServerParam::i().catchAreaLength() )
        {
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
IntentionKick::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::KICK,
                  __FILE__": execute. target=(%.1f, %.1f) speed=%.3f",
                  M_target_point.x, M_target_point.y, M_first_speed );

    if ( M_kick_step <= 0 )
    {
        dlog.addText( Logger::KICK,
                      __FILE__": error. empty kick intention" );
        return false;
    }

    if ( M_enforce_kick )
    {
        if ( M_kick_step == 1 )
        {
            // if enforce mode. not consier opponent.
            dlog.addText( Logger::KICK,
                          __FILE__": execute. enforce one kick" );
            Body_KickOneStep( M_target_point, M_first_speed ).execute( agent );
        }
        else if ( M_kick_step == 2 )
        {
            dlog.addText( Logger::KICK,
                          __FILE__": execute. enforce two kick" );
            Body_KickTwoStep( M_target_point,
                              M_first_speed,
                              false
                              ).execute( agent );
        }
        else
        {
            dlog.addText( Logger::KICK,
                          __FILE__": execute. enforce, but multi kick" );
            Body_KickMultiStep( M_target_point,
                                M_first_speed,
                                false // not enforce
                                ).execute( agent );
        }
    }
    else
    {
        Vector2D next_vel;
        const Vector2D target_rpos = M_target_point - agent->world().self().pos();

        // SEE: body_kick_multi_step::execute()

        // try one step
        if ( Body_KickTwoStep::simulate_one_kick
             ( &next_vel,
               NULL,
               NULL,
               target_rpos,
               M_first_speed,
               Vector2D(0.0, 0.0), // my current pos
               agent->world().self().vel(),
               agent->world().self().body(),
               agent->world().ball().rpos(),
               agent->world().ball().vel(),
               agent,
               false ) // not enforce
             )
        {
            Vector2D required_acc = next_vel - agent->world().ball().vel();
            double kick_pow = required_acc.r() / agent->world().self().kickRate();
            AngleDeg kick_dir = required_acc.th() - agent->world().self().body();

            dlog.addText( Logger::KICK,
                          __FILE__": execute. can reach one step" );
            agent->doKick( kick_pow, kick_dir );
            this->clear(); // finish
        }
        // try two step
        else if ( M_kick_step >=2
                  && Body_KickTwoStep::simulate_two_kick
                  ( NULL,
                    &next_vel,
                    target_rpos,
                    M_first_speed,
                    Vector2D(0.0, 0.0), // my current pos
                    agent->world().self().vel(),
                    agent->world().self().body(),
                    agent->world().ball().rpos(),
                    agent->world().ball().vel(),
                    agent,
                    false ) // not enforced
                  )
        {
            Vector2D required_acc = next_vel - agent->world().ball().vel();
            double kick_pow = required_acc.r() / agent->world().self().kickRate();
            AngleDeg kick_dir = required_acc.th() - agent->world().self().body();

            dlog.addText( Logger::KICK,
                          __FILE__": execute. can reach two step" );
            agent->doKick( kick_pow, kick_dir );
        }
        else if ( M_kick_step >= 3
                  && Body_KickMultiStep::simulate_three_kick
                  ( NULL,
                    &next_vel,
                    target_rpos,
                    M_first_speed,
                    Vector2D( 0.0, 0.0 ), // my current pos
                    agent->world().self().vel(),
                    agent->world().self().body(),
                    agent->world().ball().rpos(),
                    agent->world().ball().vel(),
                    agent,
                    false ) // not enforced
                  )
        {
            Vector2D required_acc = next_vel - agent->world().ball().vel();
            double kick_pow = required_acc.r() / agent->world().self().kickRate();
            AngleDeg kick_dir = required_acc.th() - agent->world().self().body();

            dlog.addText( Logger::KICK,
                          __FILE__": execute. can reach three step" );
            agent->doKick( kick_pow, kick_dir );
        }
        else if ( M_kick_step >= 3 )
        {
            Body_HoldBall().execute( agent );
        }
        else
        {
            dlog.addText( Logger::KICK,
                          __FILE__": execute. never reach. failed" );
            this->clear();
            return false;
        }
    }

    --M_kick_step;

    // update
    M_last_execute_time = agent->world().time();

    const AngleDeg target_angle
        = ( M_target_point
            - ( agent->world().self().pos() + agent->world().self().vel() ) ).th();

    if ( agent->world().dirCount( target_angle ) == 0 )
    {
         agent->setNeckAction( new Neck_TurnToLowConfTeammate() );
    }
    else
    {
         agent->setNeckAction( new Neck_TurnToPoint( M_target_point ) );
    }

    return true;
}

}
