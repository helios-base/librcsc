// -*-c++-*-

/*!
  \file bhv_shoot2008.cpp
  \brief advanced shoot planning and behavior.
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

#include "bhv_shoot2008.h"

#include <rcsc/common/logger.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include "neck_turn_to_goalie_or_scan.h"

#include "body_smart_kick.h"

namespace rcsc {

ShootTable2008 Bhv_Shoot2008::S_shoot_table;

/*-------------------------------------------------------------------*/
/*!

*/
bool
Bhv_Shoot2008::execute( PlayerAgent * agent )
{
    if ( ! agent->world().self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__":  not kickable" );
        return false;
    }

    const ShootTable2008::ShotCont & shots = S_shoot_table.getShots( agent );

    // update
    if ( shots.empty() )
    {
        dlog.addText( Logger::SHOOT,
                      __FILE__": no shoot route" );
        return false;
    }

    ShootTable2008::ShotCont::const_iterator shot
        = std::min_element( shots.begin(),
                            shots.end(),
                            ShootTable2008::ScoreCmp() );

    if ( shot == shots.end() )
    {
        dlog.addText( Logger::SHOOT,
                      __FILE__": no best shot" );
        return false;
    }

    // it is necessary to evaluate shoot courses

    Vector2D target_point = shot->point_;
    double first_speed = shot->speed_;

    agent->debugClient().addMessage( "Shoot" );
    agent->debugClient().setTarget( target_point );

    Vector2D one_step_vel
        = KickTable::calc_max_velocity( ( target_point - agent->world().ball().pos() ).th(),
                                        agent->world().self().kickRate(),
                                        agent->world().ball().vel() );
    double one_step_speed = one_step_vel.r();

    dlog.addText( Logger::SHOOT,
                  __FILE__": shoot to (%.2f, %.2f) speed=%f one_kick_max_speed=%f",
                  target_point.x, target_point.y,
                  first_speed,
                  one_step_speed );

    if ( one_step_speed > first_speed * 0.99 )
    {
        if ( Body_SmartKick( target_point,
                             one_step_speed,
                             one_step_speed * 0.99 - 0.0001,
                             1 ).execute( agent ) )
        {
             agent->setNeckAction( new Neck_TurnToGoalieOrScan( -1 ) );
             agent->debugClient().addMessage( "Force1Step" );
             return true;
        }
    }

    if ( Body_SmartKick( target_point,
                         first_speed,
                         first_speed * 0.99,
                         3 ).execute( agent ) )
    {
        agent->setNeckAction( new Neck_TurnToGoalieOrScan( -1 ) );
        return true;
    }

    dlog.addText( Logger::SHOOT,
                  __FILE__": failed" );
    return false;
}

}
