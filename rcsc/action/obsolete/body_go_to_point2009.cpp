// -*-c++-*-

/*!
  \file body_go_to_point2009.cpp
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

#include "body_go_to_point2009.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_stop_dash.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

// #define USE_OMNI_DASH
// #define DEBUG_PRINT

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_GoToPoint2009::execute( PlayerAgent * agent )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_GoToPoint2009 target=(%.2f %.2f) max_power=%.3f speed=%.3f",
                  M_target_point.x, M_target_point.y,
                  M_max_dash_power, M_dash_speed );
#endif

    if ( std::fabs( M_max_dash_power ) < 0.1
         || std::fabs( M_dash_speed ) < 0.001 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": max_dash_power=%f dash_speed=%f, turn only",
                      M_max_dash_power, M_dash_speed );
#endif
        agent->doTurn( 0.0 );
        return false;
    }

    const WorldModel & wm = agent->world();

    const Vector2D inertia_point = wm.self().inertiaPoint( M_cycle );
    Vector2D target_rel = M_target_point - inertia_point;

    // ------------------------------------------------------------
    // already there
    double target_dist = target_rel.r();
    if ( target_dist < M_dist_thr )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": already there. inertia_point_dist=%.3f < dist_thr=%.3f",
                      target_dist, M_dist_thr );
#endif
        agent->doTurn( 0.0 ); // dumy action
        return false;
    }

    // ------------------------------------------------------------
    // omni dash
#ifdef USE_OMNI_DASH
    if ( doOmniDash( agent ) )
    {
        return true;
    }
#endif

    // ------------------------------------------------------------
    // turn
    if ( doTurn( agent ) )
    {
        return true;
    }

    // ------------------------------------------------------------
    // dash
    if ( doDash( agent ) )
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
Body_GoToPoint2009::doOmniDash( PlayerAgent * agent )
{
    const ServerParam & SP = ServerParam::i();
    const WorldModel & wm = agent->world();

    const Vector2D inertia_point = wm.self().inertiaPoint( M_cycle );
    Vector2D target_rel = M_target_point - inertia_point;
    target_rel.rotate( - wm.self().body() );

    if ( target_rel.absY() < M_dist_thr )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": (doOmniDash) target_y_diff=%.3f, omni dash is not required.",
                      target_rel.y );
#endif
        return false;
    }

    const AngleDeg target_angle = target_rel.th();

    if ( target_angle.abs() < M_dir_thr )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": (doOmniDash) target_angle=%.3f dir_thr=%.3f omni dash is not required.",
                      target_angle.degree(), M_dir_thr );
#endif
        return false;
    }

    if ( target_rel.absY() > 3.0 ) // XXX magic number XXX
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": (doOmniDash) big y diff. target_y_diff=%.3f",
                      target_rel.y );
#endif
        return false;
    }

    const AngleDeg target_rel_angle = target_rel.th();

    const Vector2D rel_vel = wm.self().vel().rotatedVector( - wm.self().body() );

    const double dash_angle_step = std::max( 15.0, SP.dashAngleStep() );
    const double min_dash_angle = ( -180.0 < SP.minDashAngle() && SP.maxDashAngle() < 180.0
                                    ? SP.minDashAngle()
                                    : dash_angle_step * static_cast< int >( -180.0 / dash_angle_step ) );
    const double max_dash_angle = ( -180.0 < SP.minDashAngle() && SP.maxDashAngle() < 180.0
                                    ? SP.maxDashAngle() + dash_angle_step * 0.5
                                    : dash_angle_step * static_cast< int >( 180.0 / dash_angle_step ) - 1.0 );

    double best_dir = -360.0;
    int best_cycle = 1000;
    double best_dash_power = 0.0;
    double best_stamina = 0.0;

    for ( double dir = min_dash_angle;
          dir < max_dash_angle;
          dir += dash_angle_step )
    {
        if ( std::fabs( dir ) < 0.5 ) continue;
        if ( std::fabs( dir ) > 100.0 ) continue;

        const AngleDeg dash_angle = SP.discretizeDashAngle( dir );

        if ( ( dash_angle - target_rel_angle ).abs() > 90.0 )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION,
                          "__ dir=%.1f, invalid direction. continue",
                          dash_angle.degree() );
#endif
            continue;
        }

        const double dash_rate = wm.self().dashRate() * SP.dashDirRate( dir );

#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      "__ dir=%.1f, dash_rate=%f",
                      dash_angle.degree(), dash_rate );
#endif
        //
        // check if player can adjust y diff within 3 dashes.
        //

        const int max_cycle = std::min( 2, M_cycle );

        Vector2D my_pos( 0.0, 0.0 );
        Vector2D my_vel = rel_vel;
        StaminaModel stamina_model = wm.self().staminaModel();
        double first_dash_power = -1.0;

        my_pos = wm.self().playerType().inertiaPoint( my_pos, rel_vel, max_cycle );

        int cycle = 1;
        for ( ; cycle <= max_cycle; ++cycle )
        {
            Vector2D required_move = target_rel - my_pos;
            required_move.rotate( - dash_angle );

            double required_x_accel
                = calc_first_term_geom_series( required_move.x,
                                               wm.self().playerType().playerDecay(),
                                               M_cycle );

            if ( required_x_accel < 0.01 )
            {
                break;
            }

            double required_dash_power = required_x_accel / dash_rate;
            double available_stamina = ( M_save_recovery
                                         ? std::max( 0.0, stamina_model.stamina() - SP.recoverDecThrValue() - 1.0 )
                                         : stamina_model.stamina() + wm.self().playerType().extraStamina() );

            double dash_power = std::min( available_stamina, M_max_dash_power );
            dash_power = std::min( dash_power, required_dash_power );
            dash_power = std::min( dash_power, SP.maxDashPower() );

            if ( cycle == 0 )
            {
                first_dash_power = dash_power;
            }

            Vector2D accel = Vector2D::polar2vector( dash_power * dash_rate,
                                                     dash_angle );

#ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION,
                          "____ cycle=%d requied_accel=%.3f required_power=%.3f available_stamina=%.1f dash_power=%.1f",
                          cycle, required_x_accel, required_dash_power,
                          available_stamina, dash_power );
#endif
            my_vel += accel;
            my_pos += my_vel;
            my_vel *= wm.self().playerType().playerDecay();

            stamina_model.simulateDash( wm.self().playerType(), dash_power );
        }

        if ( first_dash_power < 0.0 )
        {
            continue;
        }

        my_pos = wm.self().playerType().inertiaPoint( my_pos, my_vel, M_cycle - cycle );

#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      "__ dir=%.1f y_diff=%.3f cycle=%d requied_accel=%.3f required_power=%.3f dash_power=%.1f stamina=%.1f",
                      dir, target_rel.y - my_pos.y,
                      cycle, first_dash_power, best_stamina );
#endif

        if ( std::fabs( target_rel.y - my_pos.y ) < M_dist_thr )
        {
            if ( best_cycle > cycle )
            {
                best_dir = dir;
                best_cycle = cycle;
                best_dash_power = first_dash_power;
                best_stamina = stamina_model.stamina();
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ACTION,
                              "update(1) dir=%.1f cycle=%d power=%.1f stamina=%.1f",
                              dir, best_cycle, best_dash_power, best_stamina );
#endif
            }
            else if ( best_cycle == cycle )
            {
                if ( best_stamina < stamina_model.stamina() )
                {
                    best_dir = dir;
                    best_cycle = cycle;
                    best_dash_power = first_dash_power;
                    best_stamina = stamina_model.stamina();
#ifdef DEBUG_PRINT
                    dlog.addText( Logger::ACTION,
                                  "update(2) dir=%.1f cycle=%d power=%.1f stamina=%.1f",
                                  dir, best_cycle, best_dash_power, best_stamina );
#endif
                }
            }
        }
    }

    if ( best_dir != -360.0 )
    {
        AngleDeg dash_angle = SP.discretizeDashAngle( best_dir );
#ifdef DEBUG_PRINT
        std::cerr << wm.self().unum() << ' ' << wm.time()
                  << " GoToPoint omnidash power=" << best_dash_power << " dir="  << best_dir
                  << std::endl;
        agent->debugClient().addMessage( "OmniDash%.0f", best_dir );
        dlog.addText( Logger::ACTION,
                      __FILE__": (doOmniDash) power=%.3f dir=%.1f",
                      best_dash_power, dash_angle.degree() );
#endif
        return agent->doDash( best_dash_power, dash_angle );
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_GoToPoint2009::doTurn( PlayerAgent * agent )
{
    const ServerParam & SP = ServerParam::i();
    const WorldModel & wm = agent->world();

    const Vector2D inertia_pos = wm.self().inertiaPoint( M_cycle );
    Vector2D target_rel = M_target_point - inertia_pos;

    const double target_dist = target_rel.r();
    const double max_turn = wm.self().playerType().effectiveTurn( SP.maxMoment(),
                                                                  wm.self().vel().r() );

    AngleDeg turn_moment = target_rel.th() - wm.self().body();

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION,
                  __FILE__": (doTurn) inertia_pos=(%.1f %.1f ) target_rel=(%.1f %.1f) dist=%.3f turn_moment=%.1f",
                  inertia_pos.x, inertia_pos.y,
                  target_rel.x, target_rel.y,
                  target_dist,
                  turn_moment.degree() );
#endif

    // if target is very near && turn_angle is big && agent has enough stamina,
    // it is useful to reverse accel angle.
    if ( turn_moment.abs() > max_turn
         && turn_moment.abs() > 90.0
         && target_dist < 2.0
         && wm.self().stamina() > SP.recoverDecThrValue() + 500.0 )
    {
        double effective_power = SP.maxDashPower() * wm.self().dashRate();
        double effective_back_power = SP.minDashPower() * wm.self().dashRate();
        if ( std::fabs( effective_back_power ) > std::fabs( effective_power ) * 0.75 )
        {
            M_back_mode = true;
            turn_moment += 180.0;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION,
                          __FILE__": (doTurn) back mode. turn_moment=%.1f",
                          turn_moment.degree() );
#endif
        }
    }

    double turn_thr = 180.0;
    if ( M_dist_thr < target_dist )
    {
        turn_thr = AngleDeg::asin_deg( M_dist_thr / target_dist );
    }

    turn_thr = std::max( M_dir_thr, turn_thr );

    //
    // it is not necessary to perform turn action.
    //
    if ( turn_moment.abs() < turn_thr )
    {
        return false;
    }

    //
    // register turn command
    //
#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION,
                  __FILE__": (doTurn) turn to point. angle=%.1f",
                  turn_moment.degree() );
#endif
    return agent->doTurn( turn_moment );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_GoToPoint2009::doDash( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    const Vector2D inertia_pos = wm.self().inertiaPoint( M_cycle );
    Vector2D target_rel = M_target_point - inertia_pos;

    AngleDeg accel_angle = wm.self().body();
    if ( M_back_mode )
    {
        accel_angle += 180.0;
    }

    target_rel.rotate( -accel_angle ); // required_dash_dist == target_rel.x

    // consider inertia travel
    double first_speed
        = calc_first_term_geom_series( target_rel.x,
                                       wm.self().playerType().playerDecay(),
                                       M_cycle );
    first_speed = bound( - wm.self().playerType().playerSpeedMax(),
                         first_speed,
                         wm.self().playerType().playerSpeedMax() );
    if ( M_dash_speed > 0.0 )
    {
        first_speed = std::min( first_speed, M_dash_speed );
    }

    Vector2D rel_vel = wm.self().vel();
    rel_vel.rotate( -accel_angle );

    double required_accel = first_speed - rel_vel.x;

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION,
                  __FILE__": (doDash) target_rel=(%.2f %.2f) first_speed=%.3f accel=%.3f",
                  target_rel.x, target_rel.y, first_speed, required_accel );
#endif

    if ( std::fabs( required_accel ) < 0.05 )
    {
        // ------- no action -------
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": (doDash) required_accel=%.3f, too small. no dash",
                      required_accel );
#endif
        return false;
    }

    double dash_power = required_accel / wm.self().dashRate();
    dash_power = std::min( dash_power, M_max_dash_power );
    if ( M_back_mode )
    {
        dash_power = -dash_power;
    }
    dash_power = ServerParam::i().normalizeDashPower( dash_power );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION,
                  __FILE__": (doDash) required dash power = %.3f",
                  dash_power );
#endif

    if ( M_save_recovery )
    {
        dash_power = wm.self().getSafetyDashPower( dash_power );
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION,
                      __FILE__": (doDash) set recoverry save dash power=%.3f",
                      dash_power );
#endif
    }

    return agent->doDash( dash_power );
}

}
