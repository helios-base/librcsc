// -*-c++-*-

/*!
  \file self_intercept.cpp
  \brief intercept predictor for agent itself Source File
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

#include "self_intercept.h"

#include "world_model.h"
#include "intercept_table.h"
#include "self_object.h"
#include "ball_object.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

// #define DEBUG1
// #define DEBUG

namespace rcsc {

const double SelfIntercept::MIN_TURN_THR = 12.5;
const double SelfIntercept::BACK_DASH_THR_ANGLE = 100.0;

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfIntercept::predict( const int max_cycle,
                        std::vector< InterceptInfo > & self_cache ) const
{
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  __FILE__": ------------- predict self ---------------" );
#endif
    predictOneStep( self_cache );
    predictLongStep( max_cycle, true, self_cache );
#ifdef SELF_INTERCEPT_USE_NO_SAVE_RECEVERY
    predictLongStep( max_cycle, false, self_cache );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfIntercept::predictOneStep( std::vector< InterceptInfo > & self_cache ) const
{
    const Vector2D ball_next = M_world.ball().pos() + M_world.ball().vel();
    const bool goalie_mode
        = ( M_world.self().goalie()
            && ball_next.x < ServerParam::i().ourPenaltyAreaLineX()
            && ball_next.absY() < ServerParam::i().penaltyAreaHalfWidth()
            );
    const double control_area = ( goalie_mode
                                  ? ServerParam::i().catchableArea()
                                  : M_world.self().playerType().kickableArea() );
    ///////////////////////////////////////////////////////////
    // current distance is too far. never reach by one dasy
    if ( M_world.ball().distFromSelf()
         > ( ServerParam::i().ballSpeedMax()
             + M_world.self().playerType().realSpeedMax()
             + control_area ) )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "__1 dash: too far. never reach" );
#endif
        return;
    }

    // get next ball relative pos, and set angle relative to my body angle
    Vector2D next_ball_rel = ball_next;
    next_ball_rel -= M_world.self().pos();
    next_ball_rel -= M_world.self().vel();
    next_ball_rel.rotate( - M_world.self().body() ); // X axis is player body angle

    // Now, next_ball_rel is required player's accel vector
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "__1 dash: ctrl_area=%f  next_ball_rel=(%.2f, %.2f)",
                  control_area, next_ball_rel.x, next_ball_rel.y );
#endif

    ///////////////////////////////////////////////////////////
    // if Y difference is over control_area,
    // I never reach the ball even if max power dash
    const double y_buf = ( goalie_mode ? 0.15 : 0.1 );
    if ( next_ball_rel.absY() > control_area - y_buf )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "__1 dash: big Y diff. never reach" );
#endif
        return;
    }

    ///////////////////////////////////////////////////////////
    // even if player does nothing, ball will be kickable.
    if ( predictNoDash( goalie_mode,
                        control_area,
                        next_ball_rel,
                        self_cache ) )
    {
        // can get the ball without any dash
        // player can turn to next target point
        return;
    }

    ///////////////////////////////////////////////////////////
    // check one step adjust dash
    predictOneDash( control_area, next_ball_rel, self_cache );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfIntercept::predictNoDash( const bool goalie_mode,
                              const double & control_area,
                              const Vector2D & next_ball_rel,
                              std::vector< InterceptInfo > & self_cache ) const
{
    double next_ball_dist = next_ball_rel.r();

    const double ball_noise = M_world.ball().vel().r() * ServerParam::i().ballRand();

    // over control area
    if ( next_ball_dist > control_area - 0.15 - ball_noise )
    {
        return false;
    }

    // if goalie, not needed to consider about collision and kick rate
    // immediately success!!
    if ( goalie_mode )
    {
        self_cache.emplace_back( InterceptInfo::NORMAL, 1, 0, 0.0 ); // 1 turn
        dlog.addText( Logger::INTERCEPT,
                      "--->Success! No dash goalie mode: nothing to do. next_dist = %f",
                      next_ball_dist );
        return true;
    }

    const double collide_dist
        = M_world.self().playerType().playerSize()
        + ServerParam::i().ballSize();

//     // check collision.
//     if ( next_ball_dist < collide_dist + 0.15 )
//     {
//         // maybe collision ...
//         dlog.addText( Logger::INTERCEPT,
//                       "____No dash kickable: but may cause collision next_dist = %f",
//                       next_ball_dist );
//         return false;
//     }

    // check kick rate
    Vector2D next_ball_vel
        = M_world.ball().vel()
        * ServerParam::i().ballDecay();
    Vector2D ball_rel = next_ball_rel;

    if ( next_ball_dist < collide_dist )
    {
        next_ball_dist = std::max( collide_dist, next_ball_dist );
        ball_rel.setLength( next_ball_dist );
        next_ball_vel *= -0.1;
    }

    double kick_rate
        = M_world.self().playerType().kickRate( ServerParam::i(),
                                                next_ball_dist,
                                                ball_rel.th().degree() );

    if ( ServerParam::i().maxPower() * kick_rate
         <= next_ball_vel.r() * ServerParam::i().ballDecay() * 1.1 )
    {
        // it has possibility that player cannot stop the ball
        dlog.addText( Logger::INTERCEPT,
                      "____No dash kickable: but may cause no control" );
        return false;
    }


    // at least, player can stop ball
    self_cache.emplace_back( InterceptInfo::NORMAL, 1, 0, 0.0 ); // 1 turn
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "-->Sucess! No dash: nothing to do. next_dist = %f",
                  next_ball_dist );
#endif
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfIntercept::predictOneDash( const double & control_area,
                               const Vector2D & next_ball_rel,
                               std::vector< InterceptInfo > & self_cache ) const
{
    // check possible max accel
    // possible forward dash accel distance
    double max_forward_accel_x
        = ServerParam::i().maxDashPower()
        * M_world.self().dashRate();
    M_world.self().playerType().normalizeAccel( M_world.self().vel(),
                                                M_world.self().body(),
                                                &max_forward_accel_x );

    if ( max_forward_accel_x + control_area - 0.15 < next_ball_rel.x )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "____1 dash: ball is front but over dash capacity."
                      "  max_forward_accel=%.3f control_buf=%.3f ball_x=%.3f",
                      max_forward_accel_x,
                      max_forward_accel_x + control_area - 0.15,
                      next_ball_rel.x );
#endif
        return;
    }

    // possible backward dash accel distance
    double max_back_accel_x = max_forward_accel_x;
    // normalize accel magnitude
    M_world.self().playerType().normalizeAccel( M_world.self().vel(),
                                                M_world.self().body() + 180.0,
                                                &max_back_accel_x );
    max_back_accel_x *= -1.0;

    ///////////////////////////////////////////////////////////
    if ( next_ball_rel.x < max_back_accel_x - control_area - 0.15 )

    {
        // over the reachable distance
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "____1 dash: ball is back but over dash capacity."
                      "  max_back_accel=%.3f control_buf=%.3f ball_x=%.3f",
                      max_back_accel_x,
                      max_back_accel_x - control_area - 0.15,
                      next_ball_rel.x );
#endif
        return;
    }

    const double safety_power_forward
        = min_max( 0.0,
                   M_world.self().stamina() - ServerParam::i().recoverDecThrValue() - 5.0,
                   ServerParam::i().maxDashPower() );
    double safety_forward_accel_x
        = std::min( max_forward_accel_x,
                    safety_power_forward * M_world.self().dashRate() );
    const double safety_power_back
        = min_max( 0.0,
                   M_world.self().stamina() - ServerParam::i().recoverDecThrValue() - 5.0,
                   2.0 * - ServerParam::i().minDashPower() );
    double safety_back_accel_x
        = std::max( max_back_accel_x,
                    -0.5 * safety_power_back * M_world.self().dashRate() );

#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "____1 dash: max_forward_accel = %f max_back_accel = %f",
                  max_forward_accel_x, max_back_accel_x );
    dlog.addText( Logger::INTERCEPT,
                  "____1 dash: safety_forward_accel = %f safety_back_accel = %f",
                  safety_forward_accel_x, safety_back_accel_x );
#endif

    ///////////////////////////////////////////////////////////
    // at first, check stamina safety dash
    if ( predictOneDashAdjustX( control_area,
                                next_ball_rel,
                                safety_back_accel_x,
                                safety_forward_accel_x,
                                true, // stamina safe
                                self_cache ) )
    {
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "<--- 1 dash. Success! and stamina safe" );
#endif
        return;
    }

    // second, check max accel dash
    if ( predictOneDashAdjustX( control_area,
                                next_ball_rel,
                                max_back_accel_x,
                                max_forward_accel_x,
                                false, // stamina not safe
                                self_cache ) )
    {
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "<--- 1 dash. Success! but not stamina safe" );
#endif
        return;
    }

#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "____1 dash. failure" );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfIntercept::predictOneDashAdjustX( const double & control_area,
                                      const Vector2D & next_ball_rel,
                                      const double & back_accel_x,
                                      const double & forward_accel_x,
                                      const bool stamina_safety,
                                      std::vector< InterceptInfo > & self_cache ) const
{
#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "______1 dash Adjust. back_acc_x = %.3f  next_ball_rel.x = %.3f"
                  "  forward_acc_x = %.3f",
                  back_accel_x, next_ball_rel.x, forward_accel_x );
#endif

    double dash_power = -1000.0;
    /////////////////////////////////////////////////////
    // x difference is small
    // player can put the ball on his side by this trial dash,
    if ( back_accel_x < next_ball_rel.x
         && next_ball_rel.x < forward_accel_x )
    {
        dash_power = getOneStepBestTrapPower( control_area,
                                              next_ball_rel,
                                              back_accel_x,
                                              forward_accel_x );
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "______1 dash Adjust(1). Best Trap. power = %.1f",
                      dash_power );
#endif
    }

    /////////////////////////////////////////////////////
    // x difference is bigger than acccelx. need to check ball dist after dash
    // ball is front
    if ( dash_power < -999.0
         && forward_accel_x < next_ball_rel.x )
    {
        double enable_ball_dist
            = std::sqrt( std::pow( next_ball_rel.x - forward_accel_x, 2 )
                         + std::pow( next_ball_rel.y, 2 ) );
        if ( enable_ball_dist < control_area - 0.2 )
        {
            // at least, reach the controllale distance
            dash_power = forward_accel_x / M_world.self().dashRate();
#ifdef DEBUG1
            dlog.addText( Logger::INTERCEPT,
                          "_____1 dash Adjust(2). Not Best. next_ball_dist = %.3f  power = %.1f",
                          enable_ball_dist, dash_power );
#endif
        }
    }

    // x difference is bigger than acccelx. need to check ball dist after dash
    // ball is back
    if ( dash_power < -999.0
         && next_ball_rel.x < back_accel_x )
    {
        double enable_ball_dist
            = std::sqrt( std::pow( back_accel_x - next_ball_rel.x, 2 )
                         + std::pow( next_ball_rel.y, 2 ) );
        if ( enable_ball_dist < control_area - 0.2 )
        {
            dash_power = back_accel_x / M_world.self().dashRate();
#ifdef DEBUG1
            dlog.addText( Logger::INTERCEPT,
                          "______1 dash Adjust(3).Not Best next_ball_dist = %.3f  power = %.1f",
                          enable_ball_dist, dash_power );
#endif
        }
    }

    // no solution but at least adjustable
    if ( dash_power < -999.0
         && back_accel_x < next_ball_rel.x
         && next_ball_rel.x < forward_accel_x )
    {
        dash_power = next_ball_rel.x / M_world.self().dashRate();
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "______1 dash Adjust(4).Not Best. just adjust X. dash power = %.1f",
                      dash_power );
#endif
    }

    /////////////////////////////////////////////////////
    // register
    if ( dash_power > -200.0 )
    {
        InterceptInfo::Mode mode = InterceptInfo::NORMAL;
        if ( ! stamina_safety )
        {
            double consume = ( dash_power > 0.0 ? dash_power : dash_power * -2.0 );
            if ( M_world.self().stamina() - consume
                 < ServerParam::i().recoverDecThrValue() )
            {
                mode = InterceptInfo::EXHAUST;
            }
        }
        self_cache.emplace_back( mode, 0, 1, dash_power );
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "--->Success! 1 dash Adjust. register new dash power = %f  mode = %d",
                      dash_power, mode );
#endif
        return true;
    }

#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "______1 dash Adjust. Failed" );
#endif
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
SelfIntercept::getOneStepBestTrapPower( const double & /*control_area*/,
                                        const Vector2D & next_ball_rel,
                                        const double & max_back_accel_x,
                                        const double & max_forward_accel_x ) const
{
    const double best_ctrl_dist_forward
        = M_world.self().playerType().playerSize()
        + 0.5 * M_world.self().playerType().kickableMargin()
        + ServerParam::i().ballSize();
    const double best_ctrl_dist_backward
        = M_world.self().playerType().playerSize()
        + 0.3 * M_world.self().playerType().kickableMargin()
        + ServerParam::i().ballSize();

#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "________1 dash Best Trap. best_ctrl_dist_f=%.3f best_ctrl_dist_b=%.3f next_ball_y=%.3f",
                  best_ctrl_dist_forward,
                  best_ctrl_dist_backward,
                  next_ball_rel.y );
#endif

    // Y diff is longer than best distance.
    // just put the ball on player's side
    if ( next_ball_rel.absY() > best_ctrl_dist_forward )
    {
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "________1 dash Best Trap.  just put the ball on side" );
#endif
        return next_ball_rel.x / M_world.self().dashRate();
    }

#if 1

    const double forward_trap_accel_x
        = next_ball_rel.x
        - std::sqrt( std::pow( best_ctrl_dist_forward, 2 )
                     - std::pow( next_ball_rel.y, 2 ) );
    const double backward_trap_accel_x
        = next_ball_rel.x
        + std::sqrt( std::pow( best_ctrl_dist_backward, 2 )
                     - std::pow( next_ball_rel.y, 2 ) );

    double best_accel_x = 10000.0;
    double min_power = 10000.0;

    const double x_step = ( backward_trap_accel_x - forward_trap_accel_x ) / 5.0;
    for ( double accel_x = forward_trap_accel_x;
          accel_x < backward_trap_accel_x + 0.01;
          accel_x += x_step )
    {
        if ( ( accel_x >= 0.0
               && max_forward_accel_x > accel_x )
             || ( accel_x < 0.0
                  && max_back_accel_x < accel_x )
             )
        {
            double power = accel_x / M_world.self().dashRate();
            if ( std::fabs( power ) < std::fabs( min_power ) )
            {
                best_accel_x = accel_x;
                min_power = power;
            }
        }
    }

    if ( min_power < 1000.0 )
    {
#ifdef DEBUG1
        dlog.addText( Logger::INTERCEPT,
                      "________1 dash Best Trap.  best trap."
                      " accel_x=%.3f power==%.3f",
                      best_accel_x,
                      min_power );
#endif
        return min_power;
    }

#else

    double required_accel_x[2];
    // case  put ball front
    required_accel_x[0]
        = next_ball_rel.x
        - std::sqrt( std::pow( best_ctrl_dist_forward, 2 )
                     - std::pow( next_ball_rel.y, 2 ) );
    // case put ball back
    required_accel_x[1]
        = next_ball_rel.x
        + std::sqrt( std::pow( best_ctrl_dist_backward, 2 )
                     - std::pow( next_ball_rel.y, 2 ) );

    if ( M_world.self().body().abs() < 45.0 )
    {
        // forward dash has priority if player's body face to opponent side
        if ( required_accel_x[1] > required_accel_x[0] )
        {
            std::swap( required_accel_x[0], required_accel_x[1] );
        }
    }
    else if ( std::fabs( required_accel_x[1] )
              < std::fabs( required_accel_x[0] ) )
    {
        // nearest side has priority
        std::swap( required_accel_x[0], required_accel_x[1] );
    }

#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "________1 dash: best_ctrl accel_x[0] = %f  accel_x[1] = %f",
                  required_accel_x[0], required_accel_x[1] );
#endif

    for ( int i = 0; i < 2; ++i )
    {
        if ( required_accel_x[i] >= 0.0
             && max_forward_accel_x > required_accel_x[i] )
        {
#ifdef DEBUG1
            dlog.addText( Logger::INTERCEPT,
                          "________1 dash Best Trap.  best trap."
                          " forward dash[%d]. x = %f",
                          i, required_accel_x[i] );
#endif
            return required_accel_x[i] / M_world.self().dashRate();
        }
        if ( required_accel_x[i] < 0.0
             && max_back_accel_x < required_accel_x[i] )
        {
#ifdef DEBUG1
            dlog.addText( Logger::INTERCEPT,
                          "________1 dash Best Trap.  best trap."
                          " back dash[%d]. x = %f",
                          i, required_accel_x[i] );
#endif
            return required_accel_x[i] / M_world.self().dashRate();
        }
    }

#endif

#ifdef DEBUG1
    dlog.addText( Logger::INTERCEPT,
                  "________1 dash Best Trap. Failed" );
#endif

    return -1000.0;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfIntercept::predictLongStep( const int max_cycle,
                                const bool save_recovery,
                                std::vector< InterceptInfo > & self_cache ) const
{
    const ServerParam & param = ServerParam::i();

    // calc Y distance from ball line
    Vector2D ball_to_self = M_world.self().pos() - M_world.ball().pos();
    ball_to_self.rotate( - M_world.ball().vel().th() );
    int min_cycle = static_cast< int >( std::ceil( ( ball_to_self.absY()
                                                     - M_world.self().playerType().kickableArea()
                                                     - 0.2 )
                                                   / M_world.self().playerType().realSpeedMax() ) );

    if ( min_cycle < 2 )
    {
        min_cycle = 2;
    }
    if ( max_cycle <= min_cycle )
    {
        dlog.addText( Logger::INTERCEPT,
                      "__Long.  Too big Y difference = %f."
                      "  min_cycle = %d.  max_cycle = %d",
                      ball_to_self.y, min_cycle, max_cycle );
    }

    Vector2D ball_pos
        = inertia_n_step_point( M_world.ball().pos(),
                                M_world.ball().vel(),
                                min_cycle - 1,
                                param.ballDecay() );
    Vector2D ball_vel = M_world.ball().vel()
        * std::pow( param.ballDecay(), min_cycle - 1 );
    bool found = false;

    int max_loop = max_cycle;
    for ( int cycle = min_cycle; cycle < max_loop; ++cycle )
    {
        ball_pos += ball_vel;
        ball_vel *= param.ballDecay();
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "__cycle %d: bpos(%.3f, %.3f) bvel(%.3f, %.3f)",
                      cycle,
                      ball_pos.x, ball_pos.y,
                      ball_vel.x, ball_vel.y );

#endif
//         // ball is stopped
//         if ( ball_vel.r2() < 0.0001 )
//         {
// #ifdef DEBUG
//             dlog.addText( Logger::INTERCEPT,
//                           "____ball speed reach ZERO" );
// #endif
//             break;
//         }

        if ( ball_pos.absX() > param.pitchHalfLength() + 10.0
             || ball_pos.absY() > param.pitchHalfWidth() + 10.0
             )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "____ball is out of pitch" );
#endif
            break;
        }

        const bool goalie_mode
            = ( M_world.self().goalie()
                && ball_pos.x < param.ourPenaltyAreaLineX()
                && ball_pos.absY() < param.penaltyAreaHalfWidth()
                );
        const double control_area = ( goalie_mode
                                      ? param.catchableArea()
                                      : M_world.self().playerType().kickableArea() );

        ///////////////////////////////////////////////////////////
        // reach point is too far. never reach
        if ( control_area + ( M_world.self().playerType().realSpeedMax() * cycle )
             < M_world.self().pos().dist( ball_pos ) )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "____ball is too far. never reach by %d cycle",
                          cycle );
#endif
            continue;
        }

        ///////////////////////////////////////////////////////////
        bool back_dash = false;
        int n_turn = 0;
        double result_recovery = 0.0;
        if ( canReachAfterTurnDash( cycle,
                                    ball_pos,
                                    control_area,
                                    save_recovery, // save recovery
                                    &n_turn, &back_dash, &result_recovery ) )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          " --> can reach. cycle=%d, turn=%d, back=%c recovery=%f",
                          cycle, n_turn,
                          ( back_dash ? 'b' : 'f' ),
                          result_recovery );
#endif
            double dash_power = ServerParam::i().maxDashPower();
            if ( back_dash ) dash_power = -dash_power;
            self_cache.emplace_back( ( result_recovery >= M_world.self().recovery()
                                       ? InterceptInfo::NORMAL
                                       : InterceptInfo::EXHAUST ),
                                     n_turn,
                                     std::max( 0, cycle - n_turn ),
                                     dash_power );
            if ( ! found )
            {
                max_loop = std::min( max_cycle, cycle + 10 );
            }
            found = true;
        }
        ///////////////////////////////////////////////////////////
    }

    // not registered any interception
    if ( ! found
         && save_recovery )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": SelfIntercept. failed to predict? register ball final point" );
        predictFinal( max_cycle, self_cache );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
SelfIntercept::predictFinal( const int max_cycle,
                             std::vector< InterceptInfo > & self_cache ) const

{
    const Vector2D my_final_pos = M_world.self().inertiaPoint( 100 );
    const Vector2D ball_final_pos = M_world.ball().inertiaPoint( 100 );
    const bool goalie_mode =
        ( M_world.self().goalie()
          && ball_final_pos.x < ServerParam::i().ourPenaltyAreaLineX()
          && ball_final_pos.absY() < ServerParam::i().penaltyAreaHalfWidth()
          );
    const double control_area = ( goalie_mode
                                  ? ServerParam::i().catchableArea() - 0.15
                                  : M_world.self().playerType().kickableArea() );

    AngleDeg dash_angle = M_world.self().body();
    bool back_dash = false; // dummy
    int n_turn = predictTurnCycle( 100,
                                   ball_final_pos,
                                   control_area,
                                   &dash_angle, &back_dash );
    double dash_dist = my_final_pos.dist( ball_final_pos );
    dash_dist -= control_area;
    int n_dash = M_world.self().playerType().cyclesToReachDistance( dash_dist );

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "__Final. register ball final point. max_cycle=%d, turn=%d, dash=%d",
                  max_cycle,
                  n_turn, n_dash );
#endif
    if ( max_cycle > n_turn + n_dash )
    {
        n_dash = max_cycle - n_turn;
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "__Final(2) dash step is changed by max_cycle. max=%d turn=%d dash=%d",
                      max_cycle,
                      n_turn, n_dash );
#endif
    }
    self_cache.emplace_back( InterceptInfo::NORMAL,
                             n_turn, n_dash,
                             ServerParam::i().maxDashPower() );
}

/*-------------------------------------------------------------------*/
/*!
  \param ball_pos ball position after 'cycle'.
*/
bool
SelfIntercept::canReachAfterTurnDash( const int cycle,
                                      const Vector2D & ball_pos,
                                      const double & control_area,
                                      const bool save_recovery,
                                      int * n_turn,
                                      bool * back_dash,
                                      double * result_recovery ) const
{
    AngleDeg dash_angle = M_world.self().body();

    *n_turn = predictTurnCycle( cycle,
                                ball_pos,
                                control_area,
                                &dash_angle, back_dash );
    if ( *n_turn > cycle )
    {
        return false;
    }

    return canReachAfterDash( *n_turn, std::max( 0, cycle - (*n_turn) ),
                              ball_pos, control_area,
                              save_recovery,
                              dash_angle, *back_dash,
                              result_recovery );
}

/*-------------------------------------------------------------------*/
/*!
  predict & get teammate's ball gettable cycle
*/
int
SelfIntercept::predictTurnCycle( const int cycle,
                                 const Vector2D & ball_pos,
                                 const double & control_area,
                                 AngleDeg * dash_angle,
                                 bool * back_dash ) const
{
    int n_turn = 0;

    ///////////////////////////////////////////////////
    // prepare variables
    const Vector2D inertia_pos
        = M_world.self().playerType().inertiaPoint( M_world.self().pos(),
                                                    M_world.self().vel(),
                                                    cycle );
    const Vector2D target_rel = ball_pos - inertia_pos;
    const AngleDeg target_angle = target_rel.th();

    double angle_diff = ( target_angle - (*dash_angle) ).degree();
    const bool diff_is_positive = ( angle_diff > 0.0 ? true : false );
    angle_diff = std::fabs( angle_diff );
    // atan version
    const double target_dist = target_rel.r();
    double turn_margin = 180.0;
    double control_buf = control_area - 0.25;
    //- M_world.ball().vel().r() * ServerParam::i().ballRand() * 0.5
    //- target_dist * ServerParam::i().playerRand() * 0.5;
    control_buf = std::max( 0.5, control_buf );
    if ( control_buf < target_dist )
    {
        // turn_margin = AngleDeg::atan2_deg( control_buf, target_dist );
        turn_margin = AngleDeg::asin_deg( control_buf / target_dist );
    }
    turn_margin = std::max( turn_margin, MIN_TURN_THR );

    /* case buf == 1.085 - 0.15, asin_deg(buf / dist), atan2_deg(buf, dist)
       dist =  1.0  asin_deg = 69.23  atan2_deg = 43.08
       dist =  2.0  asin_deg = 27.87  atan2_deg = 25.06
       dist =  3.0  asin_deg = 18.16  atan2_deg = 17.31
       dist =  4.0  asin_deg = 13.52  atan2_deg = 13.16
       dist =  5.0  asin_deg = 10.78  atan2_deg = 10.59
       dist =  6.0  asin_deg =  8.97  atan2_deg =  8.86
       dist =  7.0  asin_deg =  7.68  atan2_deg =  7.61
       dist =  8.0  asin_deg =  6.71  atan2_deg =  6.67
       dist =  9.0  asin_deg =  5.96  atan2_deg =  5.93
       dist = 10.0  asin_deg =  5.36  atan2_deg =  5.34
       dist = 11.0  asin_deg =  4.88  atan2_deg =  4.86
       dist = 12.0  asin_deg =  4.47  atan2_deg =  4.46
       dist = 13.0  asin_deg =  4.12  atan2_deg =  4.11
       dist = 14.0  asin_deg =  3.83  atan2_deg =  3.82
       dist = 15.0  asin_deg =  3.57  atan2_deg =  3.57
       dist = 16.0  asin_deg =  3.35  atan2_deg =  3.34
       dist = 17.0  asin_deg =  3.15  atan2_deg =  3.15
       dist = 18.0  asin_deg =  2.98  atan2_deg =  2.97
       dist = 19.0  asin_deg =  2.82  atan2_deg =  2.82
       dist = 20.0  asin_deg =  2.68  atan2_deg =  2.68
    */
    ///////////////////////////////////////////////////
    // check back dash possibility
    if ( canBackDashChase( cycle, target_dist, angle_diff ) )
    {
        *back_dash = true;
        *dash_angle += 180.0;
        angle_diff = 180.0 - angle_diff;
    }

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "______control_buf=%.2f turn_margin=%.1f angle_diff=%.1f",
                  control_buf, turn_margin, angle_diff );
#endif

    ///////////////////////////////////////////////////
    // predict turn cycles
    const double max_moment
        = ServerParam::i().maxMoment()
        * ( 1.0 - ServerParam::i().playerRand() );
    double player_speed = M_world.self().vel().r();
    while ( angle_diff > turn_margin )
    {
        double max_turnable
            = M_world.self().playerType().effectiveTurn( max_moment, player_speed );
        angle_diff -= max_turnable;
        player_speed *= M_world.self().playerType().playerDecay();
        ++n_turn;
    }

    ///////////////////////////////////////////////////
    // update dash angle
    if ( n_turn > 0 )
    {
        angle_diff = std::max( 0.0, angle_diff );
        *dash_angle = target_angle + ( diff_is_positive
                                       ? + angle_diff
                                       : - angle_diff );
    }

    return n_turn;
}

/*-------------------------------------------------------------------*/
/*!
  predict & get teammate's ball gettable cycle
*/
bool
SelfIntercept::canBackDashChase( const int cycle,
                                 const double & /*target_dist*/,
                                 const double & angle_diff ) const
{
    ///////////////////////////////////////////////
    // check angle threshold
    if ( angle_diff < BACK_DASH_THR_ANGLE )
    {
        return false;
    }
#if 0
    ///////////////////////////////////////////////
    // check distance threshold
    {
        double back_dash_thr
            = ServerParam::i().visibleDistance()
            + ( M_world.self().goalie() ? 10.0 : 5.0 );

        if ( target_dist > back_dash_thr )
        {
            return false;
        }
    }
#else
    if ( ! M_world.self().goalie() && cycle >= 5 )
    {
        return false;
    }

    if ( M_world.self().goalie()
         && cycle >= 5 )
    {
        Vector2D goal( - ServerParam::i().pitchHalfLength(), 0.0 );
        Vector2D bpos = M_world.ball().inertiaPoint( cycle );
        if ( goal.dist( bpos ) > 21.0 )
        {
            return false;
        }
    }
#endif

    ///////////////////////////////////////////////
    // check stamina threshold
    // consumed stamina by one step
    double total_consume = - ServerParam::i().minDashPower() * 2.0 * cycle;
    double total_recover
        = M_world.self().playerType().staminaIncMax() * M_world.self().recovery()
        * ( cycle - 1 );
    double result_stamina
        = M_world.self().stamina()
        - total_consume
        + total_recover;

    if ( result_stamina < ServerParam::i().recoverDecThrValue() + 205.0 )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "______cycle=%d goalie no stamina. no back. stamina=%f",
                      cycle, result_stamina );
#endif
        return false;
    }

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "______try back dash. result stamina=%f",
                  result_stamina );
#endif

    return true;
}

/*-------------------------------------------------------------------*/
/*!
  assume that players's dash angle is directed to ball reach point
  \param ball_pos ball position after 'cycle'.
*/
bool
SelfIntercept::canReachAfterDash( const int n_turn,
                                  const int n_dash,
                                  const Vector2D & ball_pos,
                                  const double & control_area,
                                  const bool save_recovery,
                                  const AngleDeg & dash_angle,
                                  const bool back_dash,
                                  double * result_recovery ) const
{
    static const double PLAYER_NOISE_RATE
        = ( 1.0 - ServerParam::i().playerRand() * 0.25 );
    static const double MAX_POWER = ServerParam::i().maxDashPower();

    const ServerParam & param = ServerParam::i();
    const PlayerType & my_type = M_world.self().playerType();

    const double recover_dec_thr = param.recoverDecThr() * param.staminaMax();
    //////////////////////////////////////////////////////////////
    const AngleDeg dash_angle_minus = -dash_angle;
    const Vector2D ball_rel
        = ( ball_pos - M_world.self().pos() ).rotatedVector( dash_angle_minus );
    const double ball_noise
        = M_world.ball().pos().dist( ball_pos )
        * param.ballRand()
        * 0.5;
    const double noised_ball_x = ball_rel.x + ball_noise;


    //////////////////////////////////////////////////////////////
    // prepare loop variables
    // ORIGIN: first player pos.
    // X-axis: dash angle
    //Vector2D tmp_pos = my_type.inertiaPoint( M_world.self().pos(), M_world.self().vel(), n_turn );
    //tmp_pos -= M_world.self().pos();
    Vector2D tmp_pos = my_type.inertiaTravel( M_world.self().vel(), n_turn );
    tmp_pos.rotate( dash_angle_minus );

    Vector2D tmp_vel = M_world.self().vel();
    tmp_vel *= std::pow( my_type.playerDecay(), n_turn );
    tmp_vel.rotate( dash_angle_minus );

    StaminaModel stamina_model = M_world.self().staminaModel();
    stamina_model.simulateWait( my_type, n_turn );

    double prev_effort = tmp_effort;
    double dash_power_abs = MAX_POWER;
    // only consider about x of dash accel vector,
    // because current orientation is player's dash angle (included back dash case)
    // NOTE: dash_accel_x must be positive value.
    double dash_accel_x
        = dash_power_abs
        * my_type.dashRate( tmp_effort );
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "______Try %d turn: %d dash:"
                  " angle=%.1f first_accel=%.2f first_vel=(%.2f %.2f)",
                  n_turn, n_dash,
                  dash_angle.degree(), dash_accel_x, tmp_vel.x, tmp_vel.y );
#endif

    //////////////////////////////////////////////////////////////
    bool can_over_speed_max = my_type.canOverSpeedMax( dash_power_abs,
                                                       tmp_effort );
    for ( int i = 0; i < n_dash; ++i )
    {
        /////////////////////////////////////////////////////////
        // update dash power & accel
        double available_power = ( save_recovery
                                   ? std::max( 0.0, stamina_model.stamina() - recover_dec_thr )
                                   : stamina_model.stamina() + my_type.extraStamina() );
        if ( back_dash ) available_power *= 0.5;
        available_power = min_max( 0.0, available_power, MAX_POWER );

        bool must_update_power = false;
        if ( available_power < dash_power_abs  // no enough stamina
             || stamina_model.effort() < prev_effort       // effort decreased.
             || ( ! can_over_speed_max         // can inclease dash power
                  && dash_power_abs < available_power )
             )
        {

            must_update_power = true;
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "________dash %d/%d: no enough?  stamina=%f  extra=%f "
                          "  cur_pow=%f  available_pow=%f",
                          i, n_dash,
                          tmp_stamina, my_type.extraStamina(),
                          dash_power_abs, available_power );
            dlog.addText( Logger::INTERCEPT,
                          "________dash %d/%d: effort decayed? %f -> %f",
                          i, n_dash, prev_effort, tmp_effort );
            dlog.addText( Logger::INTERCEPT,
                          "________dash %d/%d: reset max power?. curr_pow=%f"
                          "  available=%f",
                          i, n_dash, dash_power_abs, available_power );
#endif
        }

        if ( must_update_power )
        {
            dash_power_abs = available_power;
            dash_accel_x = dash_power_abs * my_type.dashRate( tmp_effort );
            can_over_speed_max = my_type.canOverSpeedMax( dash_power_abs,
                                                          tmp_effort );
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "________dash %d/%d: update dash_power_abs %f  accel_x =%f",
                          i, n_dash, dash_power_abs, dash_accel_x );
#endif
        }

        /////////////////////////////////////////////////////////
        // update vel
        tmp_vel.x += dash_accel_x;
        // power conservation, update accel magnitude & dash_power
        if ( can_over_speed_max
             && tmp_vel.r2() > my_type.playerSpeedMax2() )
        {
            tmp_vel.x -= dash_accel_x;
            // conserve power & reduce accel
            // sqr(rel_vel.y) + sqr(max_dash_x) == sqr(max_speed);
            // accel_mag = dash_x - rel_vel.x;
            double max_dash_x = std::sqrt( my_type.playerSpeedMax2()
                                           - ( tmp_vel.y * tmp_vel.y ) );
            dash_accel_x = max_dash_x - tmp_vel.x;
            dash_power_abs = std::fabs( dash_accel_x
                                        / my_type.dashRate( tmp_effort ) );
            // re-update vel
            tmp_vel.x += dash_accel_x;
            can_over_speed_max = my_type.canOverSpeedMax( dash_power_abs,
                                                          tmp_effort );
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "________dash %d/%d: power conserve. power=%f accel_x=%f",
                          i, n_dash, dash_power_abs, dash_accel_x );
#endif
        }
        /////////////////////////////////////////////////////////
        // velocity reached max speed
        if ( tmp_vel.x > my_type.realSpeedMax() - 0.005 )
        {
            tmp_vel.x = my_type.realSpeedMax();
            double real_power = dash_power_abs;
            if ( back_dash ) real_power *= -1.0;
            int n_safety_dash
                = my_type.getMaxDashCyclesSavingStamina( param,
                                                         real_power,
                                                         tmp_stamina,
                                                         tmp_recovery );
            n_safety_dash = std::min( n_safety_dash, n_dash - i );
            n_safety_dash = std::max( 0, n_safety_dash - 1 ); // -1 is very important
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "________dash %d/%d: reach real speed max. safety dash= %d",
                          i, n_dash, n_safety_dash );
#endif
            tmp_pos.x += tmp_vel.x * n_safety_dash;
            double one_cycle_consume = ( real_power > 0.0
                                         ? real_power
                                         : real_power * -2.0 );
            one_cycle_consume -= my_type.staminaIncMax() * tmp_recovery;
            tmp_stamina -= one_cycle_consume * n_safety_dash;
            i += n_safety_dash;
        }

        /////////////////////////////////////////////////////////
        // update pos & vel
        tmp_pos += tmp_vel;
        tmp_vel *= my_type.playerDecay();
        /////////////////////////////////////////////////////////
        // update stamina

        my_type.predictStaminaAfterOneDash( param,
                                            dash_power_abs * ( back_dash ? -1.0 : 1.0 ),
                                            &tmp_stamina, &tmp_effort, &tmp_recovery );

        /////////////////////////////////////////////////////////
        // check run over
        // it is not necessary to consider about Y difference,
        // because dash_angle is corrected for ball_reach_point
        //if ( tmp_pos.x * PLAYER_NOISE_RATE > noised_ball_x )
        if ( tmp_pos.x * PLAYER_NOISE_RATE + 0.1 > noised_ball_x )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "____dash %d/%d: can run over. rel_move_pos=(%.2f, %.2f)"
                          " ball_x=%.3f over=%.3f y_diff=%.3f",
                          i, n_dash,
                          tmp_pos.x, tmp_pos.y,
                          noised_ball_x,
                          //tmp_pos.x * PLAYER_NOISE_RATE - noised_ball_x,
                          tmp_pos.x * PLAYER_NOISE_RATE - 0.1 - noised_ball_x,
                          std::fabs( tmp_pos.y - ball_rel.y ) );
#endif
            *result_recovery = tmp_recovery;
            return true;
        }
    }

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "______dash %d: no run over. pmove=(%.2f(noised=%.2f), %.2f)  ball_x=%.3f"
                  " x_diff=%.3f y_diff=%.3f",
                  n_dash,
                  tmp_pos.x, tmp_pos.x * PLAYER_NOISE_RATE, tmp_pos.y,
                  noised_ball_x,
                  tmp_pos.x * PLAYER_NOISE_RATE - noised_ball_x,
                  std::fabs( tmp_pos.y - ball_rel.y ) );
#endif

    //////////////////////////////////////////////////////////
    // when cycle is small, do strict check
    if ( n_turn + n_dash <= 6 )
    {
        // tmp_pos is relative to playerPos() --> tmp_pos.r() == player_travel
        double player_travel = tmp_pos.r();
        double player_noise = player_travel * param.playerRand() * 0.5;
        double last_ball_dist = ball_rel.dist( tmp_pos );
        double buf = 0.15;
        //if ( n_turn > 0 ) buf += 0.3;
        buf += player_noise;
        buf += ball_noise;
        if ( last_ball_dist < control_area - buf )
//         if ( std::fabs( tmp_pos.x - ball_rel.x ) < control_area - 0.15
//              && std::fabs( tmp_pos.y - ball_rel.y ) < control_area - buf
//              && last_ball_dist < control_area )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "____dash %d: can reach. last ball dist=%.3f. noised_ctrl_area=%.3f/%.3f",
                          n_dash,
                          last_ball_dist,
                          control_area - buf,
                          control_area );
#endif
            *result_recovery = tmp_recovery;
            return true;
        }

#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "______dash %d: failed. last_ball_dist=%.3f noised_ctrl_area=%.3f/%.3f",
                      n_dash,
                      last_ball_dist,
                      control_area - buf,
                      control_area );
        dlog.addText( Logger::INTERCEPT,
                      "______player_rel=(%.3f %.3f) ball_rel=(%.3f %.3f)  p_noise=%.3f b_noise=%.3f",
                      tmp_pos.x, tmp_pos.y,
                      ball_rel.x, ball_rel.y,
                      player_noise,
                      ball_noise );
        dlog.addText( Logger::INTERCEPT,
                      "______noised_ball_x=%.3f  bnoise=%.3f  pnoise=%.3f",
                      noised_ball_x,
                      ball_noise, player_noise );
#endif
        return false;
    }

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "____dash %d: cannot reach. player_rel=(%f %f)  ball_rel=(%f %f)"
                  "  noise_ball_x=%f",
                  n_dash,
                  tmp_pos.x, tmp_pos.y,
                  ball_rel.x, ball_rel.y,
                  noised_ball_x );
#endif

    return false;
}

}
