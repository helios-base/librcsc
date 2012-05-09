// -*-c++-*-

/*!
  \file body_go_to_point2007.cpp
  \brief run behavior which has target point.
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

#include "body_go_to_point2007.h"

#include "basic_actions.h"
#include "body_stop_dash.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_GoToPoint2007::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_GoToPoint2007" );

    if ( std::fabs( M_dash_power ) < EPS )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": dash_power=0  turn only" );
        agent->doTurn( 0.0 );
        return false;
    }

    const WorldModel & wm = agent->world();

    const Vector2D my_point = wm.self().inertiaPoint( M_cycle );
    Vector2D target_rel = M_target_point - my_point;

    // ------------------------------------------------------------
    // already there
    double target_dist = target_rel.r();
    if ( target_dist < M_dist_thr )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": already there. inertia_point_dist=%.3f < dist_thr=%.3f",
                      target_dist, M_dist_thr );
        agent->doTurn( 0.0 );
        return false;
    }

    AngleDeg accel_angle = wm.self().body();

    // ------------------------------------------------------------
    // turn
    if ( doTurn( agent, target_rel, &accel_angle ) )
    {
        return true;
    }

    // ------------------------------------------------------------
    // dash
    if ( doDash( agent, target_rel, accel_angle ) )
    {
        return true;
    }

    agent->doTurn( 0.0 ); // dummy action
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_GoToPoint2007::doTurn( PlayerAgent * agent,
                            const Vector2D & target_rel,
                            AngleDeg * accel_angle )
{
    const WorldModel & wm = agent->world();

    AngleDeg turn_angle = target_rel.th() - wm.self().body();
    AngleDeg turn_angle_reverse = turn_angle - 180.0;

    if ( M_back_mode )
    {
        std::swap( turn_angle, turn_angle_reverse );
        *accel_angle -= 180.0;
        dlog.addText( Logger::ACTION,
                      __FILE__": doTurn. back mode. accel_angle=%.1f  turn_angle=%.1f",
                      accel_angle->degree(), turn_angle.degree() );
    }

    const double target_dist = target_rel.r();

    // if target is very near && turn_angle is big && agent has enough stamina,
    // it is useful to reverse accel angle.
    if ( turn_angle.abs() > 90.0
         && target_dist < 1.0
         && ( wm.self().stamina()
              > ServerParam::i().recoverDecThrValue() + 500.0 )
         )
    {
        M_back_mode = ! M_back_mode;
        std::swap( turn_angle, turn_angle_reverse );
        *accel_angle -= 180.0;
        dlog.addText( Logger::ACTION,
                      __FILE__": doTurn. swap back mode. accel_angle=%.1f  turn_angle=%.1f",
                      accel_angle->degree(), turn_angle.degree() );
    }

    // calculate turn moment threshold
    double turn_thr = 180.0; // if already reached, turn is not required.
    if ( M_dist_thr < target_dist )
    {
        // not reached yet
        turn_thr = AngleDeg::asin_deg( M_dist_thr / target_dist );
    }
    // normalize by minimal turn thr
    turn_thr = std::max( M_dir_thr, turn_thr );

    // ----- turn -----
    if ( turn_angle.abs() > turn_thr )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": doTurn. turn to point. angle=%.1f",
                      turn_angle.degree() );
        return agent->doTurn( turn_angle );
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_GoToPoint2007::doDash( PlayerAgent * agent,
                            Vector2D target_rel,
                            const AngleDeg & accel_angle )
{
    const WorldModel & wm = agent->world();

    target_rel.rotate( -accel_angle ); // required_dash_dist == target_rel.x

    // consider inertia travel
    double first_speed
        = calc_first_term_geom_series
        ( target_rel.x,
          wm.self().playerType().playerDecay(),
          M_cycle );
    first_speed = min_max( - wm.self().playerType().playerSpeedMax(),
                                 first_speed,
                                 wm.self().playerType().playerSpeedMax() );
    Vector2D rel_vel = wm.self().vel();
    rel_vel.rotate( -accel_angle );

    double required_accel = first_speed - rel_vel.x;

    dlog.addText( Logger::ACTION,
                  __FILE__": doDash. target_rel=(%.2f %.2f) first_speed=%.3f  accel=%.3f",
                  target_rel.x, target_rel.y, first_speed, required_accel );

    if ( std::fabs( required_accel ) < 0.05 )
    {
        // ------- no action -------
        dlog.addText( Logger::ACTION,
                      __FILE__": doDash. Required acces %.3f is too small. Not needed to dash!",
                      required_accel );
        return false;
    }

    double dash_power = required_accel / wm.self().dashRate();
    dash_power = std::min( dash_power, M_dash_power );
    if ( M_back_mode )
    {
        dash_power = -dash_power;
    }
    dash_power = ServerParam::i().normalizeDashPower( dash_power );
    dlog.addText( Logger::ACTION,
                  __FILE__": doDash. required dash power = %.1f",
                  dash_power );
    if ( M_save_recovery )
    {
        dash_power = wm.self().getSafetyDashPower( dash_power );
    }

    dlog.addText( Logger::ACTION,
                  __FILE__": doDash. dash to point. power=%.1f",
                  dash_power );

    return agent->doDash( dash_power );
}

}
