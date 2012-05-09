// -*-c++-*-

/*!
  \file body_stop_dash.cpp
  \brief try to change the agent's velocity to 0.
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

#include "body_stop_dash.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_StopDash::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_StopDash" );

    const WorldModel & wm = agent->world();

    if ( ! wm.self().velValid() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": self vel is invalid" );
        agent->doTurn( 0.0 );
        return false;
    }

    const ServerParam & SP = ServerParam::i();

    const double dash_angle_step = std::max( 15.0, SP.dashAngleStep() );
    const double min_dash_angle = ( -180.0 < SP.minDashAngle() && SP.maxDashAngle() < 180.0
                                    ? SP.minDashAngle()
                                    : dash_angle_step * static_cast< int >( -180.0 / dash_angle_step ) );
    const double max_dash_angle = ( -180.0 < SP.minDashAngle() && SP.maxDashAngle() < 180.0
                                    ? SP.maxDashAngle() + dash_angle_step * 0.5
                                    : dash_angle_step * static_cast< int >( 180.0 / dash_angle_step ) - 1.0 );

    double best_dir = -360.0;
    double best_result_speed = 100.0;
    double best_dash_power = 0.0;

    for ( double dir = min_dash_angle;
          dir < max_dash_angle;
          dir += dash_angle_step )
    {
        const double dash_rate = wm.self().dashRate() * SP.dashDirRate( dir );
        const AngleDeg dash_angle = wm.self().body() + SP.discretizeDashAngle( dir );
        Vector2D rel_vel = wm.self().vel();
        rel_vel.rotate( - dash_angle );

        double dash_power = - rel_vel.x / dash_rate;
        dash_power = SP.normalizeDashPower( dash_power );

        if ( M_save_recovery )
        {
            dash_power = wm.self().getSafetyDashPower( dash_power );
        }

        Vector2D result_vel
            = wm.self().vel()
            + Vector2D::from_polar( dash_power * dash_rate, dash_angle );

        double result_speed = result_vel.r();

        dlog.addText( Logger::ACTION,
                      __FILE__": dir=%.1f (angle=%.1f) dash_power=%.2f result_vel=(%.2f %.2f) speed=%.2f",
                      dir, dash_angle.degree(),
                      dash_power,
                      result_vel.x, result_vel.y, result_speed );

        if ( result_speed < best_result_speed )
        {
            best_dir = dir;
            best_result_speed = result_speed;
            best_dash_power = dash_power;
        }
    }

    if ( best_dir == -360.0 )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": Could not find stop dash" );
        return false;
    }

    dlog.addText( Logger::ACTION,
                  __FILE__": BEST: dir=%.1f dash_power=%.2f result_speed=%.2f",
                  best_dir,
                  best_dash_power,
                  best_result_speed );

    if ( std::fabs( best_dash_power ) < 1.0 )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": Too small dash power" );
        return false;
    }

    return agent->doDash( best_dash_power, AngleDeg( best_dir ) );
}

}
