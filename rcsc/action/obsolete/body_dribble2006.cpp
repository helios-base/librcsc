// -*-c++-*-

/*!
  \file body_dribble2006.cpp
  \brief advanced dribble action. player agent can avoid opponent
  players.
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

#include "body_dribble2006.h"
#include "intention_dribble2006.h"

#include "basic_actions.h"
#include "body_hold_ball.h"
#include "body_intercept2007.h"
#include "body_kick_to_relative.h"
#include "body_stop_ball.h"
#include "neck_scan_field.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/ray_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/sector_2d.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_Dribble2006::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": Body_Dribble. (%.1f, %.1f), dash_power= %.1f"
                  " dash_count=%d",
                  M_target_point.x, M_target_point.y,
                  M_dash_power, M_dash_count );

    if ( ! agent->world().self().isKickable() )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (execute) not kickable" );
        return Body_Intercept2007().execute( agent );
    }

    if ( ! agent->world().ball().velValid() )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (execute) invalid ball vel" );
        return Body_StopBall().execute( agent );
    }

    M_dash_power = agent->world().self().getSafetyDashPower( M_dash_power );

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (execute) dash_power=%.1f, dash_count=%d",
                  M_dash_power, M_dash_count );

    doAction( agent,
              M_target_point,
              M_dash_power,
              M_dash_count,
              M_dodge_mode );  // dodge mode;
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_Dribble2006::doAction( PlayerAgent * agent,
                            const Vector2D & target_point,
                            const double & dash_power,
                            const int dash_count,
                            const bool dodge )
{
    // try to create the action queue.
    // kick -> dash -> dash -> ...
    // the number of dash is specified by M_dash_count

    /*--------------------------------------------------------*/
    // do dodge dribble
    if ( dodge
         && isDodgeSituation( agent, target_point ) )
    {
        agent->debugClient().addMessage( "DribDodge" );
        return doDodge( agent, target_point );
    }

    /*--------------------------------------------------------*/
    // normal dribble

    const WorldModel& wm = agent->world();
    const Vector2D mylast
        = inertia_final_point( wm.self().pos(),
                               wm.self().vel(),
                               wm.self().playerType().playerDecay() );
    const Vector2D target_rpos = target_point - mylast;
    const double target_dist = target_rpos.r();

    // already reach the target point
    if ( target_dist < M_dist_thr )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doAction) already there. hold" );
        return Body_HoldBall().execute( agent );
    }

    agent->debugClient().setTarget( M_target_point );


    /*--------------------------------------------------------*/
    // decide dribble angle & dist

    const AngleDeg target_angle = target_rpos.th();

    const double dir_diff_abs
        = ( dash_power > 0.0
            ? ( target_angle - wm.self().body() ).abs()
            : ( target_angle - wm.self().body() - 180.0 ).abs() );

    const double dir_margin_abs
        = std::max( 15.0,
                    std::fabs( AngleDeg::atan2_deg( M_dist_thr, target_dist ) ) );

    /*--------------------------------------------------------*/
    // it is necessary to turn to target point
    if ( dir_diff_abs > dir_margin_abs )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doAction) need turn to (%.2f, %.2f) angle=%.1f, diff=%.1f",
                      target_point.x, target_point.y,
                      target_angle.degree(), dir_diff_abs );
        Vector2D ball_next_rel = wm.ball().rpos();
        ball_next_rel +=  wm.ball().vel();
        ball_next_rel -= wm.self().vel();

        double ball_next_dist = ball_next_rel.r();
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doAction) brpos=(%.2f, %.2f) bvel=(%.2f, %.2f), "
                      "myvel=(%.2f, %.2f), next_rel=(%.2f, %.2f) r=%.2f",
                      wm.ball().rpos().x, wm.ball().rpos().y,
                      wm.ball().vel().x, wm.ball().vel().y,
                      wm.self().vel().x, wm.self().vel().x,
                      ball_next_rel.x, ball_next_rel.y,
                      ball_next_dist );

#if 1
        // ball kickable at next cycle
        if ( ball_next_dist
             < ( wm.self().playerType().kickableArea()
                 - wm.ball().vel().r() * ServerParam::i().ballRand()
                 - wm.self().vel().r() * ServerParam::i().playerRand()
                 - 0.15 ) )
        {
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doAction) next kickable. next_dist=%.2f",
                          ball_next_dist );
            if ( wm.opponentsFromBall().empty()
                 || ( wm.opponentsFromBall().front()->distFromBall()
                      > ServerParam::i().defaultKickableArea()
                      + ServerParam::i().defaultPlayerSpeedMax() * 2.5 )
                 )
            {
                if ( doCollideForTurn( agent, dir_diff_abs, false ) )
                {
                    return true;
                }
                // turn only
                AngleDeg turn_rel_angle
                    = ( dash_power > 0.0
                        ? target_angle - wm.self().body()
                        : target_angle - wm.self().body() - 180.0 );

                dlog.addText( Logger::DRIBBLE,
                              __FILE__": next kickable. next_dist=%.2f,  turn=%.1f",
                              ball_next_dist, turn_rel_angle.degree() );
                return agent->doTurn( turn_rel_angle );
            }
        }
#else
        if ( doTurnOnly( agent,
                         target_point,
                         dash_power ) )
        {
            return true;
        }
#endif

        // or, if no kick at current, ball is not kickable at next
        return doKickTurnsDash( agent,
                                target_point,
                                dir_margin_abs,
                                dash_power );
    }

    /*--------------------------------------------------------*/
    // after one dash, ball will kickable
    double used_dash_power = dash_power;
    if ( canKickAfterDash( agent, &used_dash_power ) )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doAction) next kickable. after dash. dash_power=%.1f",
                      used_dash_power );
        return agent->doDash( used_dash_power );
    }

    /*--------------------------------------------------------*/
    // do kick first
#if 0
    if ( doKickDashesWithBall( agent, dash_power ) )
    {
        return true;
    }
#endif
    return doKickDashes( agent, target_point, dash_power, dash_count );
}

#if 1
/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::doTurnOnly( PlayerAgent * agent,
                              const Vector2D & target_point,
                              const double & dash_power )
{
    const WorldModel & wm = agent->world();

    //---------------------------------------------------------//
    // check opponent
    if ( ! wm.opponentsFromBall().empty()
         && ( wm.opponentsFromBall().front()->distFromBall()
              < ServerParam::i().defaultKickableArea()
              + ServerParam::i().defaultPlayerSpeedMax() * 2.5 )
         )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doTurnOnly) exist near opponent" );
        return false;
    }

    //---------------------------------------------------------//
    // check next ball dist after turn
    Vector2D ball_next_rel = wm.ball().rpos();
    ball_next_rel +=  wm.ball().vel();
    ball_next_rel -= wm.self().vel();
    const double ball_next_dist = ball_next_rel.r();

    // not kickable at next cycle, if do turn at current cycle.
    if ( ball_next_dist
         > ( wm.self().playerType().kickableArea()
             - agent->world().ball().vel().r() * ServerParam::i().ballRand()
             - agent->world().self().vel().r() * ServerParam::i().playerRand()
             - 0.15 ) )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doTurnOnly) not kickable at next" );
        return false;
    }

    //---------------------------------------------------------//
    // check required turn step.
    const Vector2D mylast
        = inertia_final_point( wm.self().pos(),
                               wm.self().vel(),
                               wm.self().playerType().playerDecay() );
    const Vector2D target_rel = target_point - mylast;
    const double target_dist = target_rel.r();
    const AngleDeg target_angle = target_rel.th();

    double dir_diff_abs
        = ( dash_power > 0.0
            ? ( target_angle - agent->world().self().body() ).abs()
            : ( target_angle - agent->world().self().body() - 180.0 ).abs() );

    const double dir_margin_abs
        = std::max( 12.0,
                    std::fabs( AngleDeg::atan2_deg( M_dist_thr, target_dist ) ) );

    const double max_turn_moment
        = effective_turn( ServerParam::i().maxMoment(),
                          wm.self().vel().r(),
                          wm.self().playerType().inertiaMoment() );
    // it is necessary to turn more than one step.
    if ( dir_diff_abs - dir_margin_abs > max_turn_moment )
    {
        return false;
    }

    //---------------------------------------------------------//
    // turn only
    AngleDeg turn_rel_angle
        = ( dash_power > 0.0
            ? target_angle - agent->world().self().body()
            : target_angle - agent->world().self().body() - 180.0 );

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doTurnOnly) kickable at next and required only one turn.",
                  " next_ball_dist=%.2f",
                  ball_next_dist );
    agent->doTurn( turn_rel_angle );
    return true;
}
#endif

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::doCollideWithBall( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    Vector2D required_accel = wm.self().vel(); // target relative pos
    required_accel -= wm.ball().rpos(); // required vel
    required_accel -= wm.ball().vel(); // ball next rpos

    double required_power = required_accel.r()/ wm.self().kickRate();
    if ( required_power > ServerParam::i().maxPower() * 1.1 )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doCollideWithBall)  over max power(%f). never collide",
                      required_power );
        return false;
    }


    agent->doKick( std::min( required_power, ServerParam::i().maxPower() ),
                   required_accel.th() - wm.self().body() );
    return true;

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::doCollideForTurn( PlayerAgent * agent,
                                    const double & dir_diff_abs,
                                    const bool kick_first )
{
    const WorldModel& wm = agent->world();
    double my_speed = wm.self().vel().r();
    if ( kick_first )
    {
        my_speed *= wm.self().playerType().playerDecay();
    }
    const double max_turn_moment = effective_turn( ServerParam::i().maxMoment(),
                                                   my_speed,
                                                   wm.self().playerType().inertiaMoment() );
    if ( max_turn_moment > dir_diff_abs * 0.9 )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doCollideForTurn) can face to target by next turn" );
        return false;
    }

    if ( doCollideWithBall( agent ) )
    {
        agent->debugClient().addMessage( "CollideForTurn" );
        return true;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!
  if back dash mode, dash_power is negative value.
*/
bool
Body_Dribble2006::doKickTurnsDash( PlayerAgent * agent,
                                   const Vector2D & target_point,
                                   const double & /*dir_margin_abs*/,
                                   const double & dash_power )
{
    // try to create these action queue
    // kick -> turn -> turn -> ... -> one dash -> normal dribble kick

    // assume that ball kickable and require to turn to target.

    const WorldModel & wm = agent->world();
    const Vector2D mylast
        = inertia_final_point( wm.self().pos(),
                               wm.self().vel(),
                               wm.self().playerType().playerDecay() );
    const Vector2D target_rel = target_point - mylast;
    const AngleDeg target_angle = target_rel.th();

    // simulate kick - turn - dash

    double dir_diff_abs = ( target_angle - agent->world().self().body() ).degree();
    bool left_turn = ( dir_diff_abs < 0.0 );
    if ( dash_power < 0.0 )
    {
        dir_diff_abs = AngleDeg::normalize_angle( dir_diff_abs - 180.0 );
        left_turn = ! left_turn;
    }
    dir_diff_abs = std::fabs(dir_diff_abs);

    if ( doCollideForTurn( agent, dir_diff_abs, true ) )
    {
        // severa turns are required after kick.
        // try to collide with ball.
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doKickTurnsDash) Collide with ball" );
        return true;
    }

    // first step is kick
    double my_speed
        = wm.self().vel().r()
        * wm.self().playerType().playerDecay();
    int n_turn = 0;
    while ( dir_diff_abs > 0.0 )
    {
        // !!! it is necessary to consider about self inertia moment

        double moment_abs
            = effective_turn
            (ServerParam::i().maxMoment(),
             my_speed,
             wm.self().playerType().inertiaMoment());
        moment_abs = std::min( dir_diff_abs, moment_abs );
        dir_diff_abs -= moment_abs;
        my_speed *= wm.self().playerType().playerDecay();
        ++n_turn;
    }


    Vector2D my_pos
        = inertia_n_step_point( Vector2D( 0.0, 0.0 ),
                                wm.self().vel(),
                                1 + n_turn, // kick + turns
                                wm.self().playerType().playerDecay() );
    // plus dash move
#if 0
    my_pos += Vector2D::polar2vector( std::fabs( dash_power ) * wm.self().dashRate(),
                                      target_angle );
#endif
    double control_dist = wm.self().playerType().kickableArea() * 0.7;


    AngleDeg keep_global_angle;
    if ( ! existCloseOpponent( agent, &keep_global_angle ) )
    {
        if ( target_angle.isLeftOf( wm.ball().angleFromSelf() ) )
        {
            keep_global_angle = target_angle + 35.0;
        }
        else
        {
            keep_global_angle = target_angle - 35.0;
        }
    }

    // relative to current my pos, angle is global
    Vector2D required_ball_rel_pos
        = my_pos
        + Vector2D::polar2vector( control_dist, keep_global_angle );


    // travel = firstvel * (1 + dec + dec^2 + ...)
    // firstvel = travel / (1 + dec + dec^2 + ...)
    const double term
        = ( 1.0 - std::pow( ServerParam::i().ballDecay(), n_turn + 2 ) )
        / ( 1.0 - ServerParam::i().ballDecay() );
    const Vector2D required_first_vel
        = ( required_ball_rel_pos - wm.ball().rpos() ) / term;
    const Vector2D required_accel
        = required_first_vel
        - wm.ball().vel();

    // check power overflow
    const double required_kick_power
        = required_accel.r() / wm.self().kickRate();

    // cannot get the required accel using only one kick
    if ( required_kick_power > ServerParam::i().maxPower()
         || required_first_vel.r() > ServerParam::i().ballSpeedMax() )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doKickTurnsDash) power over= %f,  or speed range over= %f",
                      required_kick_power, required_first_vel.r() );
        agent->debugClient().addMessage( "DribPowRot" );
        return Body_KickToRelative( control_dist,
                                    keep_global_angle - wm.self().body(),
                                    false // not need to stop
                                    ).execute( agent );
    }

    // check collision
    {
        my_pos.assign( 0.0, 0.0 );
        Vector2D my_vel = wm.self().vel();
        Vector2D ball_pos = wm.ball().rpos();
        Vector2D ball_vel = required_first_vel;
        const double collide_dist2
            = square( wm.self().playerType().playerSize()
                      + ServerParam::i().ballSize() );
        for ( int i = 1; i < n_turn + 1; i++ )
        {
            my_pos += my_vel;
            ball_pos += ball_vel;

            if ( my_pos.dist2( ball_pos ) < collide_dist2 )
            {
                dlog.addText( Logger::DRIBBLE,
                              __FILE__": (doKickTurnsDash) maybe cause collision" );
                agent->debugClient().addMessage( "DribColRot" );
                return Body_KickToRelative( control_dist,
                                            keep_global_angle - wm.self().body(),
                                            false // not need to stop
                                            ).execute( agent );
            }

            my_vel *= wm.self().playerType().playerDecay();
            ball_vel *= ServerParam::i().ballDecay();
        }
    }

    // can archieve required vel

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickTurnsDash) kick -> turn[%d]",
                  n_turn );
    agent->debugClient().addMessage( "DribKT%dD", n_turn );

    //////////////////////////////////////////////////////////
    // register intention
    agent->setIntention
        ( new IntentionDribble2006( target_point,
                                    M_dist_thr,
                                    n_turn,
                                    1, // one dash
                                    std::fabs( dash_power ),
                                    ( dash_power < 0.0 ), // back_dash
                                    wm.time() ) );

    // execute first kick
    return agent->doKick( required_kick_power,
                          required_accel.th() - wm.self().body() );
}

/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_Dribble2006::doKickDashes( PlayerAgent * agent,
                                const Vector2D & target_point,
                                const double & dash_power,
                                const int dash_count )
{
    // do dribble kick. simulate next action queue.
    // kick -> dash -> dash -> ...
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickDashes)" );

    const WorldModel & wm = agent->world();

    ////////////////////////////////////////////////////////
    // simulate my pos after one kick & dashes

    double my_effort = wm.self().effort();
    // next cycles my stamina
    double my_stamina
        = wm.self().stamina()
        + ( wm.self().playerType().staminaIncMax()
            * wm.self().recovery() );

    const AngleDeg accel_angle = ( dash_power > 0.0
                                   ? wm.self().body()
                                   : wm.self().body() - 180.0 );

    // first step is current kick.
    Vector2D my_pos = wm.self().vel();
    Vector2D my_vel = wm.self().vel();
    my_vel *= wm.self().playerType().playerDecay();

    for ( int i = 0; i < dash_count; i++ )
    {
        double available_stamina
            = std::max( 0.0,
                        my_stamina - ServerParam::i().recoverDecThrValue() - 305.0 );
        double used_stamina = dash_power * ( dash_power < 0.0 ? -2.0 : 1.0 );
        double used_power = std::min( available_stamina, used_stamina );
        used_stamina = used_power;
        if ( dash_power < 0.0 )
        {
            used_power *= -0.5;
        }
        const double max_accel_mag = ( std::fabs( used_power )
                                       * wm.self().playerType().dashPowerRate()
                                       * my_effort );
        double accel_mag = max_accel_mag;
        if ( wm.self().playerType().normalizeAccel( my_vel,
                                                    accel_angle,
                                                    &accel_mag ) )
        {
#ifdef DEBUG
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doKickDashes) estimate speed over. dash %d. power conserve"
                          " old_power=%.1f accel=%.2f  --> new_power %.1f new_accel=%.2f",
                          i + 1,
                          used_power, max_accel_mag,
                          used_power * (accel_mag / max_accel_mag), accel_mag );
#endif
            used_power *= accel_mag / max_accel_mag;
        }
        Vector2D dash_accel = Vector2D::polar2vector( accel_mag, accel_angle );
        my_vel += dash_accel;
        my_pos += my_vel;

#ifdef DEBUG
        Vector2D my_moved_pos = wm.self().pos() + my_pos;
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doKickDashes) predict my move. dash %d."
                      " accel=%.2f angle=%.1f speed=%.3f"
                      " used_stamina = %.1f  pos=(%.1f %.1f)",
                      i + 1,
                      accel_mag, accel_angle.degree(),
                      my_vel.r(),
                      used_stamina,
                      my_moved_pos.x, my_moved_pos.y );
#endif
        my_vel *= wm.self().playerType().playerDecay();
        my_stamina -= used_stamina;
        my_stamina += ( wm.self().playerType().staminaIncMax()
                        * wm.self().recovery() );
    }
    const double my_move_dist = my_pos.r();
    const AngleDeg my_move_dir = my_pos.th();

    ////////////////////////////////////////////////////////
    // estimate required kick param
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickDashes) my move dist = %.3f  dir = %.1f  accel_angle=%.1f",
                  my_move_dist, my_move_dir.degree(),
                  accel_angle.degree() );


    // decide next ball control point

    AngleDeg keep_global_angle;
    bool exist_close_opp = existCloseOpponent( agent, &keep_global_angle );

    // XXX Magic Number
#if 0
    double control_dist
        = wm.self().playerType().playerSize()
        + wm.self().playerType().kickableMargin() * 0.48;
    double angle_buf = std::min( 30.0, 9.0 * ( dash_count - 1 ) );
    double add_angle_abs = std::max( 0.0, 45.0 - angle_buf );
#else
    double control_dist;
    double add_angle_abs;
    {
        double y_dist
            = wm.self().playerType().playerSize()
            + ServerParam::i().ballSize()
            + 0.2;
        Vector2D cur_ball_rel = wm.ball().rpos().rotatedVector( - my_move_dir );
        if ( cur_ball_rel.absY() < y_dist )
        {
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doKickDashes) y_dist(%.2f) is"
                          " inner from keep Y(%2.f). correct.",
                          cur_ball_rel.absY(), y_dist );
            //y_dist = ( y_dist + cur_ball_rel.absY() ) * 0.5;
            y_dist += 0.1;
            y_dist = std::min( y_dist, cur_ball_rel.absY() );
        }
        double x_dist
            = std::sqrt( std::pow( wm.self().playerType().kickableArea(), 2 )
                         - std::pow( y_dist, 2 ) )
            - 0.2 - std::min( 0.6, my_move_dist * 0.05 );
        control_dist = std::sqrt( std::pow( x_dist, 2 )
                                  + std::pow( y_dist, 2 ) );
        add_angle_abs = std::fabs( AngleDeg::atan2_deg( y_dist, x_dist ) );
    }
#endif

    if ( exist_close_opp )
    {
        //if ( accel_angle.isLeftOf( keep_global_angle ) )
        if ( my_move_dir.isLeftOf( keep_global_angle ) )
        {
            //keep_global_angle = accel_angle + add_angle_abs;
            keep_global_angle = my_move_dir + add_angle_abs;
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doKickDashes) avoid. keep right" );
        }
        else
        {
            //keep_global_angle = accel_angle - add_angle_abs;
            keep_global_angle = my_move_dir - add_angle_abs;
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doKickDashes) avoid. keep left" );
        }
    }
    else
    {
        //if ( accel_angle.isLeftOf( wm.ball().angleFromSelf() ) )
        if ( my_move_dir.isLeftOf( wm.ball().angleFromSelf() ) )
        {
            //keep_global_angle = accel_angle + add_angle_abs;
            keep_global_angle = my_move_dir + add_angle_abs;
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doKickDashes) keep right."
                          " accel_angle= %.1f < ball_angle=%.1f",
                          accel_angle.degree(),
                          wm.ball().angleFromSelf().degree() );
        }
        else
        {
            //keep_global_angle = accel_angle - add_angle_abs;
            keep_global_angle = my_move_dir - add_angle_abs;
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (doKickDashes) keep left."
                          " accel_angle= %.1f > ball_angle=%.1f",
                          accel_angle.degree(),
                          wm.ball().angleFromSelf().degree() );
        }
    }

    const Vector2D next_ball_rel
        = Vector2D::polar2vector( control_dist, keep_global_angle );
    Vector2D next_ctrl_ball_pos = wm.self().pos() + my_pos + next_ball_rel;
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickDashes) next_ball_rel=(%.2f, %.2f) global(%.2f %.2f)"
                  " ctrl_dist= %.2f, keep_anggle=%.1f",
                  next_ball_rel.x, next_ball_rel.y,
                  next_ctrl_ball_pos.x, next_ctrl_ball_pos.y,
                  control_dist, keep_global_angle.degree() );

    // calculate required kick param

    // relative to current my pos
    const Vector2D required_ball_pos = my_pos + next_ball_rel;
    const double term
        = ( 1.0 - std::pow( ServerParam::i().ballDecay(), dash_count + 1 ) )
        / ( 1.0 - ServerParam::i().ballDecay() );
    const Vector2D required_first_vel
        = (required_ball_pos - wm.ball().rpos()) / term;
    const Vector2D required_accel
        = required_first_vel
        - wm.ball().vel();
    const double required_kick_power
        = required_accel.r() / wm.self().kickRate();

    ////////////////////////////////////////////////////////
    // never kickable
    if ( required_kick_power > ServerParam::i().maxPower()
         || required_first_vel.r() > ServerParam::i().ballSpeedMax() )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doKickDashes) never reach. rotate." );
        agent->debugClient().addMessage( "DribKDFail" );
        return Body_KickToRelative( wm.self().playerType().kickableArea() * 0.7,
                                    keep_global_angle - wm.self().body(),
                                    false  // not need to stop
                                    ).execute( agent );
    }


    ////////////////////////////////////////////////////////
    // check next collision

    const double collide_dist2
        = std::pow( wm.self().playerType().playerSize()
                    + ServerParam::i().ballSize()
                    + 0.15,
                    2 );
    if ( ( wm.ball().rpos()
           + required_first_vel - wm.self().vel() ).r2() // next rel pos
         < collide_dist2 )
    {
        AngleDeg rotate_global_angle = keep_global_angle;
        //if ( ( wm.ball().angleFromSelf() - accel_angle ).abs() > 90.0 )
        if ( ( wm.ball().angleFromSelf() - my_move_dir ).abs() > 90.0 )
        {
            //if ( keep_global_angle.isLeftOf( accel_angle ) )
            if ( keep_global_angle.isLeftOf( my_move_dir ) )
            {
                //rotate_global_angle = accel_angle + 90.0;
                rotate_global_angle = my_move_dir + 90.0;
            }
            else
            {
                //rotate_global_angle = accel_angle + 90.0;
                rotate_global_angle = my_move_dir + 90.0;
            }
        }
        AngleDeg rotate_rel_angle = rotate_global_angle - wm.self().body();
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doKickDashes) maybe collision. rotate. rel_angle=%f",
                      rotate_rel_angle.degree() );
        agent->debugClient().addMessage( "DribKDCol" );
        return Body_KickToRelative( wm.self().playerType().kickableArea() * 0.7,
                                    rotate_rel_angle,
                                    false  // not need to stop
                                    ).execute( agent );
    }

    agent->debugClient().addMessage( "DribKD%d:%.0f", dash_count, dash_power );
    agent->debugClient().addLine( wm.self().pos(), wm.self().pos() + my_pos );
    //////////////////////////////////////////////////////////
    // register intention
    agent->setIntention
        ( new IntentionDribble2006( target_point,
                                    M_dist_thr,
                                    0, // zero turn
                                    dash_count,
                                    std::fabs( dash_power ),
                                    ( dash_power < 0.0 ), // back_dash
                                    wm.time() ) );
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickDashes) register intention. dash_count=%d",
                  dash_count );
    // execute first kick
    return agent->doKick( required_kick_power,
                          required_accel.th() - wm.self().body() );
}


/*-------------------------------------------------------------------*/
/*!
  execute action
*/
bool
Body_Dribble2006::doKickDashesWithBall( PlayerAgent * agent,
                                        const double & dash_power )
{
    // do dribble kick. simulate next action queue.
    // kick -> dash -> dash -> ...
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickDashesWithBall)" );

    const WorldModel & wm = agent->world();

    const AngleDeg accel_angle = ( dash_power > 0.0
                                   ? wm.self().body()
                                   : wm.self().body() - 180.0 );

    // estimate my move positions

    std::vector< Vector2D > my_state;
    my_state.reserve( 20 );

    {
        StaminaModel stamina_model = wm.self().staminaModel();

        stamina_model.simulateWait( wm.self().playerType() );

        Vector2D my_pos = wm.self().pos();
        Vector2D my_vel = wm.self().vel();

        my_pos += my_vel;
        my_state.push_back( my_pos ); // first element is next cycle just after kick

        for ( int i = 0; i < 20; ++i )
        {
            double available_stamina
                =  std::max( 0.0,
                             stamina_model.stamina()
                             - ServerParam::i().recoverDecThrValue()
                             - 300.0 );
            double consumed_stamina = ( dash_power > 0.0
                                        ? dash_power
                                        : dash_power * -2.0 );
            consumed_stamina = std::min( available_stamina,
                                         consumed_stamina );
            double used_power = ( dash_power > 0.0
                                  ? consumed_stamina
                                  : consumed_stamina * -0.5 );
            double max_accel_mag = ( std::fabs( used_power )
                                     * wm.self().playerType().dashPowerRate()
                                     * stamina_model.effort() );
            double accel_mag = max_accel_mag;
            if ( wm.self().playerType().normalizeAccel( my_vel,
                                                        accel_angle,
                                                        &accel_mag ) )
            {
                used_power *= accel_mag / max_accel_mag;
            }

            Vector2D dash_accel
                = Vector2D::polar2vector( std::fabs( used_power )
                                          * stamina_model.effort()
                                          * wm.self().playerType().dashPowerRate(),
                                          accel_angle );
            my_vel += dash_accel;
            my_pos += my_vel;

            my_state.push_back( my_pos );

            my_vel *= wm.self().playerType().playerDecay();

            stamina_model.simulateDash( wm.self().playerType(), used_power );
        }
    }


    const int ANGLE_DIVS = 18;
    const double collide_dist
        = wm.self().playerType().playerSize()
        + ServerParam::i().ballSize()
        + 0.1;
    const double kickable_area
        = wm.self().playerType().kickableArea() - 0.25;
    const Rect2D pitch_rect( Vector2D( - ServerParam::i().pitchHalfLength(),
                                       - ServerParam::i().pitchHalfWidth() ),
                             Size2D( ServerParam::i().pitchLength(),
                                     ServerParam::i().pitchWidth() ) );

    const std::vector< Vector2D >::const_iterator my_pos_end = my_state.end();


    std::list< KeepDribbleInfo > dribble_info;

    const double max_dist = wm.self().playerType().kickableArea() - 0.1;
    const double dist_step = 0.15;

    double first_ball_dist = wm.self().playerType().playerSize() + 0.2;
    first_ball_dist -= dist_step;

    int dist_loop = 0;
    // distance loop
    while ( first_ball_dist < max_dist )
    {
        ++dist_loop;
        first_ball_dist += dist_step;
        if ( first_ball_dist > max_dist )
        {
            first_ball_dist = max_dist;
        }

        AngleDeg first_ball_angle = accel_angle - 90.0;

        // angle loop
        for ( int angle_loop = 0;
              angle_loop < ANGLE_DIVS + 1;
              ++angle_loop, first_ball_angle += 180.0 / ANGLE_DIVS )
        {

            Vector2D ball_pos
                = my_state.front()
                + Vector2D::polar2vector( first_ball_dist, first_ball_angle );
            double min_opp_dist = 1000.0;

            if ( ! pitch_rect.contains( ball_pos ) )
            {
                continue;
            }

            const Vector2D first_ball_vel
                = ball_pos - wm.ball().pos();

            if ( first_ball_vel.r() / wm.self().kickRate() > ServerParam::i().maxPower() )
            {
                // cannot acccelerate to the desired speed
                continue;
            }


            if ( existKickableOpponent( wm, ball_pos, &min_opp_dist ) )
            {
                continue;
            }

            Vector2D ball_vel = first_ball_vel;
            ball_vel *= ServerParam::i().ballDecay();

            int dash_count = 0;

            // future state loop
            for ( std::vector< Vector2D >::const_iterator my_pos = my_state.begin() + 1;
                  my_pos != my_pos_end;
                  ++my_pos )
            {
                ball_pos += ball_vel;

                if ( ! pitch_rect.contains( ball_pos ) )
                {
                    // out of pitch
                    break;
                }

                double new_ball_dist = my_pos->dist( ball_pos );
                if ( new_ball_dist < collide_dist
                     || new_ball_dist > kickable_area )
                {
                    // no kickable
                    break;
                }

                if ( existKickableOpponent( wm, ball_pos, &min_opp_dist ) )
                {
                    // exist interfare opponent
                    break;
                }

                ++dash_count;
                ball_vel *= ServerParam::i().ballDecay();
            }

            dribble_info.emplace_back( first_ball_vel,
                                       dash_count,
                                       min_opp_dist ) );
            dlog.addText( Logger::DRIBBLE,
                          "_____ add bdist=%.2f bangle=%.1f"
                          "vel=(%.1f %.1f) dash=%d  opp_dist=%.1f",
                          first_ball_dist,
                          first_ball_angle.degree(),
                          first_ball_vel.x, first_ball_vel.y,
                          dash_count,
                          min_opp_dist );
        }
    }

    dlog.addText( Logger::DRIBBLE,
                  "___ dist loop= %d,  solution = %d",
                  dist_loop, dribble_info.size() );

    if ( dribble_info.empty() )
    {
        return false;
    }

    dribble_info.sort( KeepDribbleCmp() );
    const KeepDribbleInfo & dribble = dribble_info.front();

    agent->debugClient().addMessage( "DribKDKeep%d:%.0f",
                                     dribble.dash_count_,
                                     dash_power );

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doKickDashesWithBall) dash_count=%d, vel=(%.1f %.1f)",
                  dribble.dash_count_,
                  dribble.first_ball_vel_.x,
                  dribble.first_ball_vel_.y );

#if 1
    {
        Vector2D ball_pos = wm.ball().pos();
        Vector2D ball_vel = dribble.first_ball_vel_;
        for ( int i = 0; i < dribble.dash_count_ + 1; ++i )
        {
            ball_pos += ball_vel;
            ball_vel *= ServerParam::i().ballDecay();
            agent->debugClient().addCircle( ball_pos, 0.2 );
            agent->debugClient().addCircle( my_state[i], wm.self().playerType().kickableArea() );
        }
    }
#endif

    Vector2D kick_accel = dribble.first_ball_vel_ - wm.ball().vel();

    // execute first kick
    return agent->doKick( kick_accel.r() / wm.self().kickRate(),
                          kick_accel.th() - wm.self().body() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::existKickableOpponent( const WorldModel & wm,
                                         const Vector2D & ball_pos,
                                         double * min_opp_dist ) const
{
    static const double kickable_area
        = ServerParam::i().defaultKickableArea() + 0.2;

    const PlayerPtrCont::const_iterator end = wm.opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = wm.opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->posCount() > 5 )
        {
            continue;
        }

        if ( (*it)->distFromSelf() > 30.0 )
        {
            break;
        }

        // goalie's catchable check
        if ( (*it)->goalie() )
        {
            if ( ball_pos.x > ServerParam::i().theirPenaltyAreaLineX()
                 && ball_pos.absY() < ServerParam::i().penaltyAreaHalfWidth() )
            {
                double d = (*it)->pos().dist( ball_pos );
                if ( d < ServerParam::i().catchableArea() )
                {
                    return true;
                }

                d -= ServerParam::i().catchableArea();
                if ( *min_opp_dist > d )
                {
                    *min_opp_dist = d;
                }
            }
        }

        // normal kickable check
        double d = (*it)->pos().dist2( ball_pos );
        if ( d < kickable_area )
        {
            return true;
        }

        d -= kickable_area;
        if ( *min_opp_dist > d )
        {
            *min_opp_dist = d;
        }
    }

    return false;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::doDodge( PlayerAgent * agent,
                           const Vector2D & target_point )
{
    const WorldModel & wm = agent->world();

    const double new_target_dist = 6.0;

    AngleDeg avoid_angle
        = getAvoidAngle( agent,
                         ( target_point - wm.self().pos() ).th() );

    const Vector2D new_target_rel
        = Vector2D::polar2vector( new_target_dist, avoid_angle );
    Vector2D new_target
        = wm.self().pos()
        + new_target_rel;

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doDodge) avoid_angle=%.1f",
                  avoid_angle.degree() );
    agent->debugClient().addCircle( new_target, 0.7 );

    const PlayerPtrCont & opponents = wm.opponentsFromSelf();
    {
        const PlayerPtrCont::const_iterator end = opponents.end();
        int counter = 0;
        for ( PlayerPtrCont::const_iterator it = opponents.begin();
              it != end;
              ++it )
        {
            if ( (*it)->distFromSelf()
                 < ( ServerParam::i().defaultKickableArea() * 0.8
                     * static_cast< double >( counter ) ) )
            {
                dlog.addText( Logger::DRIBBLE,
                              __FILE__": (doDodge) emergency avoidance" );
                agent->debugClient().addMessage( "AvoidKick" );
                return doAvoidKick( agent, avoid_angle );
            }

            if ( ++counter >= 2 )
            {
                break;
            }
        }
    }

    double min_opp_dist = ( opponents.empty()
                            ? 100.0
                            : opponents.front()->distFromSelf() );

    double dir_diff_abs = ( avoid_angle - wm.self().body() ).abs();
    double avoid_dash_power;
    if ( min_opp_dist > 3.0
         || dir_diff_abs < 120.0
         || agent->world().self().stamina() < ServerParam::i().staminaMax() * 0.5
         )
    {
        avoid_dash_power
            = agent->world().self().getSafetyDashPower( ServerParam::i().maxDashPower() );
    }
    else
    {
        // backward
        avoid_dash_power
            = agent->world().self().getSafetyDashPower( ServerParam::i().minDashPower() );

    }

    const double pitch_buffer = 1.0;
    if ( new_target.absX() > ServerParam::i().pitchHalfLength() - pitch_buffer )
    {
        double diff
            = new_target.absX()
            - (ServerParam::i().pitchHalfLength() - pitch_buffer);
        double rate = 1.0 - diff / new_target_rel.absX();
        new_target
            = wm.self().pos()
            + Vector2D::polar2vector( new_target_dist * rate,
                                      avoid_angle );
    }
    if ( new_target.absY() > ServerParam::i().pitchHalfWidth() - pitch_buffer )
    {
        double diff
            = new_target.absY()
            - (ServerParam::i().pitchHalfWidth() - pitch_buffer);
        double rate = 1.0 - diff / new_target_rel.absY();
        new_target
            = wm.self().pos()
            + Vector2D::polar2vector( new_target_dist * rate,
                                      avoid_angle );
    }
    int n_dash = 2;

    if ( avoid_dash_power > 0.0
         && wm.self().pos().x > -20.0
         && new_target.absY() > 15.0 )
    {
        double dist_to_target = wm.self().pos().dist(new_target);
        n_dash = static_cast< int >( std::floor( dist_to_target
                                                 / wm.self().playerType().realSpeedMax()
                                                 * 0.9 ) );
        //n_dash = min_max( 2, n_dash, 5 );
        n_dash = std::min( 3, n_dash );

        dlog.addText( Logger::DRIBBLE,
                      __FILE__": doDodge. dash step = %d",
                      n_dash );
    }

    {
        Ray2D drib_ray( wm.self().pos(), avoid_angle );
        Rect2D pitch_rect( Vector2D( - ServerParam::i().pitchHalfLength() + 0.5,
                                     - ServerParam::i().pitchHalfWidth() + 0.5 ),
                           Size2D( ServerParam::i().pitchLength() - 1.0,
                                   ServerParam::i().pitchWidth() - 1.0 ) );
        Vector2D pitch_intersect;
        if ( pitch_rect.intersection( drib_ray, &pitch_intersect, NULL ) == 1 )
        {
            if ( wm.self().pos().dist( pitch_intersect ) < 7.0 )
            {
                dlog.addText( Logger::DRIBBLE,
                              __FILE__": (doDodge) pitch intersection near."
                              " enforce 1 dash step" );
                n_dash = 1;
            }
        }
    }

    return doAction( agent, new_target, avoid_dash_power,
                     n_dash, false ); // no dodge
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::doAvoidKick( PlayerAgent * agent,
                               const AngleDeg & avoid_angle )
{
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": doAvoidKick" );

    const WorldModel& wm = agent->world();

    const double ball_move_radius = 2.0;

    const Vector2D target_rel_point
        = Vector2D::polar2vector( ball_move_radius, avoid_angle );

    // my max turnable moment with current speed
    const double next_turnable
        = ServerParam::i().maxMoment()
        / ( 1.0
            + wm.self().playerType().inertiaMoment()
            * (wm.self().vel().r()
               * wm.self().playerType().playerDecay()) );
    // my inernia move vector
    const Vector2D my_inertia_final_rel_pos
        = wm.self().vel()
        / ( 1.0 - wm.self().playerType().playerDecay() );

    AngleDeg target_angle = ( target_rel_point - my_inertia_final_rel_pos ).th();
    double dir_diff_abs = ( target_angle - wm.self().body() ).abs();
    double dir_margin_abs
        = std::max( 12.0,
                    std::fabs( AngleDeg::atan2_deg( wm.self().playerType().kickableArea() * 0.8,
                                                    ball_move_radius ) ) );

    double ball_first_speed;
    // kick -> dash -> dash -> dash -> ...
    if ( dir_diff_abs < dir_margin_abs
         || dir_diff_abs > 180.0 - dir_margin_abs ) // backward dash
    {
        ball_first_speed = 0.7;
    }
    // kick -> turn -> dash -> dash -> ...
    else if ( dir_diff_abs < next_turnable
              || dir_diff_abs > 180.0 - next_turnable )
    {
        ball_first_speed = 0.5;
    }
    // kick -> turn -> turn -> dash -> ...
    else
    {
        ball_first_speed = 0.3;
    }

    Vector2D required_first_vel
        = Vector2D::polar2vector( ball_first_speed,
                                  ( target_rel_point - wm.ball().rpos() ).th() );
    Vector2D required_accel = required_first_vel - wm.ball().vel();
    double required_kick_power = required_accel.r() / wm.self().kickRate();

    // over max power
    if ( required_kick_power > ServerParam::i().maxPower() )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doAvoidKick) power over. hold" );
        Vector2D face_point
            = wm.self().pos()
            + Vector2D::polar2vector( 20.0, target_angle );
        return Body_HoldBall( true, face_point ).execute( agent );
    }

    // check collision
    if ( ( wm.ball().rpos() + required_first_vel ).dist( wm.self().vel() )
         < wm.self().playerType().playerSize() + ServerParam::i().ballSize() )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (doAvoidKick) detect collision. hold" );
        Vector2D face_point
            = wm.self().pos()
            + Vector2D::polar2vector(20.0, target_angle);
        return Body_HoldBall( true, face_point ).execute( agent );
    }

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (doAvoidKick) done" );

    return agent->doKick( required_kick_power,
                          required_accel.th() - wm.self().body() );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::isDodgeSituation( const PlayerAgent * agent,
                                    const Vector2D & target_point )
{
    const WorldModel& wm = agent->world();
    //////////////////////////////////////////////////////
    const AngleDeg target_angle = (target_point - wm.self().pos()).th();
    // check if opponent on target dir
    const Sector2D sector( wm.self().pos(),
                           0.6,
                           std::min( 5, M_dash_count )
                           * ServerParam::i().defaultPlayerSpeedMax()
                           * 1.5, //10.0,
                           target_angle - 20.0, target_angle + 20.0 );
    const double base_safety_dir_diff = 60.0;
    double dodge_consider_dist = ( static_cast< double >( M_dash_count )
                                   * ServerParam::i().defaultPlayerSpeedMax() * 2.0 )
        + 4.0;

    if ( dodge_consider_dist > 10.0 ) dodge_consider_dist = 10.0;


    const PlayerPtrCont::const_iterator
        end = wm.opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator
              it = wm.opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->posCount() >= 10 ) continue;

        if ( sector.contains( (*it)->pos() ) )
        {
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (isDodgeSituation) exist obstacle on dir" );
            return true;
        }

        const double dir_diff = ( (*it)->angleFromSelf() - target_angle ).abs();
        double add_buf = 0.0;
        if ( (*it)->distFromSelf() < dodge_consider_dist
             && (*it)->distFromSelf() > 3.0 )
        {
            add_buf = 30.0 / (*it)->distFromSelf();
        }

        if ( (*it)->distFromSelf() < 1.0
             || ( (*it)->distFromSelf() < 1.5 && dir_diff < 120.0 )
             || ( (*it)->distFromSelf() < dodge_consider_dist
                  && dir_diff < base_safety_dir_diff + add_buf ) )
        {
            dlog.addText( Logger::DRIBBLE,
                          __FILE__": (isDodgeSituation) exist obstacle (%.1f, %.1f) dist=%.2f"
                          " dir_diff=%,1f dir_buf=%.1f",
                          (*it)->pos().x, (*it)->pos().y,
                          (*it)->distFromSelf(),
                          dir_diff, base_safety_dir_diff + add_buf );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::canKickAfterDash( const PlayerAgent * agent,
                                    double * dash_power )
{
    const WorldModel & wm = agent->world();

    const Vector2D ball_next
        = wm.ball().pos()
        + wm.ball().vel();

    const PlayerObject * opp = wm.getOpponentNearestToSelf( 5 );
    if ( opp
         && ( opp->pos().dist( ball_next )
              < ( ServerParam::i().defaultKickableArea()
                  + ServerParam::i().defaultPlayerSpeedMax()
                  + 0.3 ) )
         )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (canKickAfterDash)"
                      " next_ball=(%.2f %.2f)"
                      " exist near opp(%.1f %.1f)",
                      ball_next.x, ball_next.y,
                      opp->pos().x, opp->pos().y );
        return false;
    }

    // check safety
    AngleDeg accel_angle = ( *dash_power < 0.0
                             ? wm.self().body() - 180.0
                             : wm.self().body() );
    Vector2D my_pos(0.0, 0.0);
    Vector2D my_vel = wm.self().vel();

    const double max_accel_mag = ( std::fabs( *dash_power )
                                   * wm.self().playerType().dashPowerRate()
                                   * wm.self().effort() );
    double accel_mag = max_accel_mag;
    if ( wm.self().playerType().normalizeAccel( wm.self().vel(),
                                                accel_angle,
                                                &accel_mag ) )
    {
        *dash_power *= accel_mag / max_accel_mag;
    }
    Vector2D dash_accel
        = Vector2D::polar2vector( accel_mag, accel_angle );

    Vector2D ball_pos = wm.ball().rpos();
    Vector2D ball_vel = wm.ball().vel();

    my_vel += dash_accel;
    my_pos += my_vel;
    ball_pos += ball_vel;

    double ball_dist = my_pos.dist( ball_pos );
    double noise_buf
        = my_vel.r() * ServerParam::i().playerRand() * 0.5
        + ball_vel.r() * ServerParam::i().ballRand() * 0.5;

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (canKickAfterDash) ball_dist= %.2f, noise= %.2f",
                  ball_dist, noise_buf );

    if ( ( ( ball_pos - my_pos ).th() - accel_angle ).abs() < 150.0
         && ball_dist + noise_buf < wm.self().playerType().kickableArea() - 0.1
         && ( ball_dist - noise_buf
              > ( wm.self().playerType().playerSize()
                  + ServerParam::i().ballSize() ) ) )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (canKickAfterDash) kickable after one dash" );
        return true;
    }

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (canKickAfterDash) ball must be kicked." );
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Dribble2006::existCloseOpponent( const PlayerAgent * agent,
                                      AngleDeg * keep_angle )
{
    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (existCloseOppnent) check start." );

    const WorldModel & wm = agent->world();

    const PlayerObject * opp = wm.getOpponentNearestToBall( 5 );
    if ( ! opp )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (existCloseOppnent) No opponent." );
        return false;
    }

    // opp is in dangerous range
    if ( opp->distFromBall()
         < ( ServerParam::i().defaultPlayerSpeedMax()
             + ServerParam::i().tackleDist() * 0.75 )
         )
    {
        Vector2D my_next = wm.self().pos() + wm.self().vel();
        Vector2D opp_next = opp->pos() + opp->vel();
        AngleDeg opp_angle = ( opp_next - my_next ).th();

        *keep_angle = opp_angle - 180.0;

        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (existCloseOpponent) found interfere opponent."
                      " keep_angle=%.1f",
                      keep_angle->degree() );
        return true;
    }

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (existCloseOpponent) No dangarous opponent" );
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
AngleDeg
Body_Dribble2006::getAvoidAngle( const PlayerAgent * agent,
                                 const AngleDeg & target_angle )
{
    const WorldModel& wm = agent->world();

    if ( wm.opponentsFromSelf().empty() )
    {
        return target_angle;
    }

    const double avoid_radius = 5.0;
    const double safety_opp_dist = 5.0;
    const double safety_space_body_ang_radius2 = 3.0 * 3.0;


    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerPtrCont::const_iterator opps_end = opps.end();

    // at first, check my body dir and opposite dir of my body
    if ( ! opps.empty()
         && opps.front()->distFromSelf() < 3.0 )
    {
        dlog.addText( Logger::DRIBBLE,
                      __FILE__": (getAvoidAngle) check body line." );

        AngleDeg new_target_angle = wm.self().body();
        for ( int i = 0; i < 2; i++ )
        {
            const Vector2D sub_target
                = wm.self().pos()
                + Vector2D::polar2vector( avoid_radius,
                                          new_target_angle );

            if ( sub_target.absX() > ServerParam::i().pitchHalfLength() - 1.8
                 || sub_target.absY() > ServerParam::i().pitchHalfWidth() - 1.8 )
            {
                // out of pitch
                continue;
            }

            bool success = true;
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                if ( (*it)->distFromSelf() > 20.0 )
                {
                    break;
                }
                if ( (*it)->distFromSelf() < safety_opp_dist
                     && ((*it)->angleFromSelf() - new_target_angle).abs() < 40.0 )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "____ body line dir=%.1f"
                                  " exist near opp(%.1f, %.1f)",
                                  new_target_angle.degree(),
                                  (*it)->pos().x, (*it)->pos().y );
                    success = false;
                    break;
                }

                if ( sub_target.dist2( (*it)->pos() )
                     < safety_space_body_ang_radius2 )
                {
                    dlog.addText( Logger::DRIBBLE,
                                  "____ body line dir=%.1f"
                                  " exist opp(%.1f, %.1f) "
                                  "close to subtarget(%.1f, %.1f)",
                                  new_target_angle.degree(),
                                  (*it)->pos().x, (*it)->pos().y,
                                  sub_target.x, sub_target.y );
                    success = false;
                    break;
                }
            }

            if ( success )
            {
                dlog.addText( Logger::DRIBBLE,
                              "__ avoid to body line. angle=%.1f",
                              new_target_angle.degree() );
                return new_target_angle;
            }

            new_target_angle -= 180.0;
        }
    }

    // search divisions

    const int search_divs = 10;
    const double div_dir = 360.0 / static_cast< double >( search_divs );
    const double safety_angle = 60.0;
    const double safety_space_radius2 = avoid_radius * avoid_radius;

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (getAvoidAngle) search angles." );
    double angle_sign = 1.0;
    if ( agent->world().self().pos().y < 0.0 ) angle_sign = -1.0;
    for ( int i = 1; i < search_divs; i++ )
    {
        const AngleDeg new_target_angle
            = target_angle
            + (angle_sign * static_cast< double >((i+1)/2) * div_dir);

        const Vector2D sub_target
            = wm.self().pos()
            + Vector2D::polar2vector( avoid_radius,
                                      new_target_angle );
        if ( sub_target.absX()
             > ServerParam::i().pitchHalfLength() - wm.self().playerType().kickableArea() - 0.2
             || sub_target.absY()
             > ServerParam::i().pitchHalfWidth() - wm.self().playerType().kickableArea() - 0.2 )
        {
            // out of pitch
            continue;
        }

        if ( sub_target.x < 30.0
             && sub_target.x < wm.self().pos().x - 2.0 )
        {
            continue;
        }

        bool success = true;
        for ( PlayerPtrCont::const_iterator it = opps.begin();
              it != opps_end;
              ++it )
        {
            if ( (*it)->posCount() >= 10 )
            {
                continue;
            }
            if ( (*it)->distFromSelf() > 20.0 )
            {
                break;
            }

            double add_dir = 5.8 / (*it)->distFromSelf();
            add_dir = std::min( 180.0 - safety_angle, add_dir );
            if ( (*it)->distFromSelf() < safety_opp_dist
                 && ( ( (*it)->angleFromSelf() - new_target_angle ).abs()
                      < safety_angle + add_dir ) )
            {
                dlog.addText( Logger::DRIBBLE,
                              "____ opp angle close. cannot avoid to %.1f",
                              new_target_angle.degree() );
                success = false;
                break;
            }
            if ( sub_target.dist2( (*it)->pos() ) < safety_space_radius2 )
            {
                dlog.addText( Logger::DRIBBLE,
                              "____ dist close. cannot avoid to %.1f",
                              new_target_angle.degree() );
                success = false;
                break;
            }
        }
        if ( success )
        {
            dlog.addText( Logger::DRIBBLE,
                          "__ avoid to angle= %.1f",
                          new_target_angle.degree() );
            return new_target_angle;
        }

        angle_sign *= -1.0;
    }


    // Best angle is not found.
    // go to the least congestion point

    dlog.addText( Logger::DRIBBLE,
                  __FILE__": (getAvoidAngle) search least congestion point." );

    Rect2D target_rect( Vector2D( wm.self().pos().x - 4.0,
                                  wm.self().pos().y - 4.0 ),
                        Size2D( 8.0, 8.0 ) );

    double best_score = 10000.0;
    Vector2D best_target = wm.self().pos();

    double x_i = 30.0 - wm.self().pos().x;
    if ( x_i > 0.0 ) x_i = 0.0;
    if ( x_i < -8.0 ) x_i = -8.0;

    for ( ; x_i < 8.5; x_i += 1.0 )
    {
        for ( double y_i = - 8.0; y_i < 8.5; y_i += 1.0 )
        {
            Vector2D candidate = wm.self().pos();
            candidate.add( x_i, y_i );

            if ( candidate.absX() > ServerParam::i().pitchHalfLength() - 1.0
                 || candidate.absY() > ServerParam::i().pitchHalfWidth() - 1.0 )
            {
                continue;
            }

            double tmp_score = 0.0;
            for ( PlayerPtrCont::const_iterator it = opps.begin();
                  it != opps_end;
                  ++it )
            {
                tmp_score += 1.0 / (*it)->pos().dist2(candidate);
            }

            if ( tmp_score < best_score )
            {
                dlog.addText( Logger::DRIBBLE,
                              "____ update least congestion point to"
                              " (%.2f, %.2f) score=%.4f",
                              candidate.x, candidate.y, tmp_score );
                best_target = candidate;
                best_score = tmp_score;
            }
        }
    }

    dlog.addText( Logger::DRIBBLE,
                  "__ avoid to point (%.2f, %.2f)",
                  best_target.x, best_target.y );

    return ( best_target - wm.self().pos() ).th();
}

}
