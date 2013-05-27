// -*-c++-*-

/*!
  \file self_intercept_simulator.cpp
  \brief self intercept simulator for creating candidate intercept actions
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

#include "self_intercept_simulator.h"

#include "ball_object.h"
#include "self_object.h"
#include "world_model.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/stamina_model.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/geom/matrix_2d.h>
#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>
#include <rcsc/timer.h>

#define DEBUG_PROFILE
#define DEBUG_PRINT_RESULTS

#define DEBUG_PRINT_ONE_STEP

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
void
SelfInterceptSimulator::simulate( const WorldModel & wm,
                                  const int max_step,
                                  std::vector< InterceptInfo > & self_cache )
{
#ifdef DEBUG_PROFILE
    rcsc::Timer timer;
#endif

    simulateOneStep( wm, self_cache );
    simulateTurnDash( wm, max_step, false, self_cache ); // forward dash
    simulateTurnDash( wm, max_step, true, self_cache ); // back dash

    std::sort( self_cache.begin(), self_cache.end(), InterceptInfo::Cmp() );

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::INTERCEPT,
                  __FILE__" (simulate) elapsed %f [ms]",
                  timer.elapsedReal() );
#endif
#ifdef DEBUG_PRINT_RESULTS
    dlog.addText( Logger::INTERCEPT,
                  "(SelfIntercept) solution size = %zd",
                  self_cache.size() );
    for ( std::vector< InterceptInfo >::iterator it = self_cache.begin(),
              end = self_cache.end();
          it != end;
          ++it )
    {
        dlog.addText( Logger::INTERCEPT,
                      "(SelfIntercept) type=%d step=%d (t:%d d:%d)"
                      " power=%.2f angle=%.1f"
                      " self_pos=(%.2f %.2f) bdist=%.3f stamina=%.1f",
                      it->mode(),
                      it->reachCycle(),
                      it->turnCycle(),
                      it->dashCycle(),
                      it->dashPower(),
                      it->dashAngle().degree(),
                      it->selfPos().x, it->selfPos().y,
                      it->ballDist(),
                      it->stamina() );
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SelfInterceptSimulator::simulateOneStep( const WorldModel & wm,
                                         std::vector< InterceptInfo > & self_cache )
{
    const Vector2D ball_next = wm.ball().pos() + wm.ball().vel();
    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < ServerParam::i().ourPenaltyAreaLineX() - 0.5
            && ball_next.absY() < ServerParam::i().penaltyAreaHalfWidth() - 0.5 );
    const double control_area = ( goalie_mode
                                  ? wm.self().playerType().reliableCatchableDist()
                                  : wm.self().playerType().kickableArea() );

#ifdef DEBUG_PRINT_ONE_STEP
    dlog.addText( Logger::INTERCEPT,
                  "(SelfIntercept) 1 step: start" );
#endif

    if ( wm.ball().distFromSelf() > ( ServerParam::i().ballSpeedMax()
                                      + wm.self().playerType().realSpeedMax()
                                      + control_area ) )
    {
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "xx 1 step: too far" );
#endif
        return;
    }

    if ( simulateNoDash( wm, self_cache ) )
    {
        return;
    }

    simulateOneDash( wm, self_cache );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SelfInterceptSimulator::simulateNoDash( const WorldModel & wm,
                                        std::vector< InterceptInfo > & self_cache )
{
    const PlayerType & ptype = wm.self().playerType();

    const Vector2D self_next = wm.self().pos() + wm.self().vel();
    const Vector2D ball_next = wm.ball().pos() + wm.ball().vel();

    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < ServerParam::i().ourPenaltyAreaLineX() - 0.5
            && ball_next.absY() < ServerParam::i().penaltyAreaHalfWidth() - 0.5
            );
    const double control_area = ( goalie_mode
                                  ? ptype.reliableCatchableDist()
                                  : ptype.kickableArea() );

    const double ball_noise = wm.ball().vel().r() * ServerParam::i().ballRand();
    const double ball_next_dist = self_next.dist( ball_next );

    if ( ball_next_dist > control_area - 0.15 - ball_noise )
    {
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "xx 0 dash: next_dist=%.3f catch=%.3f (catchable=%.3f noise=%.3f)",
                      ball_next_dist, control_area - 0.15 - ball_noise,
                      control_area, ball_noise );
#endif
        return false;
    }

    if ( goalie_mode )
    {
        StaminaModel stamina_model = wm.self().staminaModel();
        stamina_model.simulateWait( ptype );

        self_cache.push_back( InterceptInfo( InterceptInfo::NORMAL,
                                             1, 0, // 1 turn, 0 dash
                                             0.0, 0.0,
                                             self_next,
                                             ball_next_dist,
                                             stamina_model.stamina() ) );
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "OK 0 dash: OK next_dist=%.3f catchable=%.3f noise=%.3f",
                      ball_next_dist, control_area, ball_noise );
#endif
        return true;
    }


    if ( ball_next_dist > ptype.playerSize() + ServerParam::i().ballSize() )
    {
        AngleDeg ball_angle = ( ball_next - self_next ).th() - wm.self().body();
        double kick_rate = ptype.kickRate( ball_next_dist, ball_angle.abs() );
        Vector2D ball_next_vel = wm.ball().vel() * ServerParam::i().ballDecay();

        if ( ServerParam::i().maxPower() * kick_rate
             <= ball_next_vel.r() * ServerParam::i().ballDecay() * 1.1 )
        {
            // player may not be able to stop the ball
#ifdef DEBUG_PRINT_ONE_STEP
            dlog.addText( Logger::INTERCEPT,
                          "NG 0 dash: cannot stop the ball." );
#endif
            return false;
        }
    }

    //
    // at least, player can stop the ball
    //

    StaminaModel stamina_model = wm.self().staminaModel();
    stamina_model.simulateWait( ptype );

    self_cache.push_back( InterceptInfo( InterceptInfo::NORMAL,
                                         1, 0, // 1 turn, 0 dash
                                         0.0, 0.0, // power=0, dir=0
                                         self_next,
                                         ball_next_dist,
                                         stamina_model.stamina() ) );
#ifdef DEBUG_PRINT_ONE_STEP
    dlog.addText( Logger::INTERCEPT,
                  "OK 0 dash: ball_next_dist=%.3f",
                  ball_next_dist );
#endif
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SelfInterceptSimulator::simulateOneDash( const WorldModel & wm,
                                         std::vector< InterceptInfo > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();

    const Vector2D ball_next = wm.ball().pos() + wm.ball().vel();
    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < SP.ourPenaltyAreaLineX()
            && ball_next.absY() < SP.penaltyAreaHalfWidth()
            );
    const double control_area = ( goalie_mode
                                  ? ptype.reliableCatchableDist()
                                  : ptype.kickableArea() );

    const double dash_angle_step = std::max( 5.0, SP.dashAngleStep() );
    const size_t dash_angle_divs
        = static_cast< size_t >( std::floor( ( SP.maxDashAngle() - SP.minDashAngle() )
                                             / dash_angle_step ) );

    std::vector< InterceptInfo > tmp_cache;
    tmp_cache.reserve( dash_angle_divs );

    for ( size_t d = 0 ; d < dash_angle_divs; ++d )
    {
        const double dir = SP.minDashAngle() + ( dash_angle_step * d );
        const AngleDeg dash_angle = wm.self().body() + SP.discretizeDashAngle( dir );
        const double dash_rate = wm.self().dashRate() * SP.dashDirRate( dir );
        const Matrix2D rotate = Matrix2D::make_rotation( dash_angle );

#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "(simulateOneDash) dir=%.1f angle=%.1f dash_rate=%f",
                      dir, dash_angle.degree(), dash_rate );
#endif

        //
        // try recovery save dash
        //
        {
            const double forward_dash_power
                = wm.self().staminaModel().getSafetyDashPower( ptype,
                                                               SP.maxDashPower(),
                                                               1.0 );
            const double back_dash_power
                = wm.self().staminaModel().getSafetyDashPower( ptype,
                                                               SP.minDashPower(),
                                                               1.0 );
            Vector2D max_forward_accel = rotate.transform( forward_dash_power * dash_rate, 0.0 );
            Vector2D max_back_accel = rotate.transform( back_dash_power * dash_rate, 0.0 );

            InterceptInfo safe_dash = getOneAdjustDash( wm,
                                                        dash_angle,
                                                        max_forward_accel,
                                                        max_back_accel,
                                                        control_area );
            if ( safe_dash.isValid() )
            {
#ifdef DEBUG_PRINT_ONE_STEP
                dlog.addText( Logger::INTERCEPT,
                              "OK 1 dash: mode=%d power=%.1f dir=%.1f self_pos=(%.2f %.2f) stamina=%.1f",
                              safe_dash.mode(),
                              safe_dash.dashPower(),
                              safe_dash.dashAngle().degree(),
                              safe_dash.selfPos().x, safe_dash.selfPos().y,
                              safe_dash.stamina() );
#endif
                tmp_cache.push_back( safe_dash );
                continue;
            }

            if ( std::fabs( forward_dash_power - SP.maxDashPower() ) < 1.0
                 && std::fabs( back_dash_power - SP.minDashPower() ) < 1.0 )
            {
                continue;
            }
        }

        //
        // try exhaust mode dash
        //
        {
            Vector2D max_forward_accel = rotate.transform( SP.maxDashPower() * dash_rate, 0.0 );
            Vector2D max_back_accel = rotate.transform( SP.minDashPower() * dash_rate, 0.0 );

            InterceptInfo exhaust_dash = getOneAdjustDash( wm, dash_angle,
                                                           max_forward_accel,
                                                           max_back_accel,
                                                           control_area );
            if ( exhaust_dash.isValid() )
            {
#ifdef DEBUG_PRINT_ONE_STEP
                dlog.addText( Logger::INTERCEPT,
                              "OK 1 dash: mode=%d power=%.1f dir=%.1f self_pos=(%.2f %.2f) stamina=%.1f",
                              exhaust_dash.mode(),
                              exhaust_dash.dashPower(),
                              exhaust_dash.dashAngle().degree(),
                              exhaust_dash.selfPos().x, exhaust_dash.selfPos().y,
                              exhaust_dash.stamina() );
#endif
                tmp_cache.push_back( exhaust_dash );
                continue;
            }
        }
    }

    if ( tmp_cache.empty() )
    {
        return;
    }

    const double safe_ball_dist
        = std::max( control_area - 0.2 - wm.ball().vel().r() * SP.ballRand(),
                    ptype.playerSize() + SP.ballSize() + ptype.kickableMargin() * 0.4 );


    const InterceptInfo * best = &(tmp_cache.front());

    for ( std::vector< InterceptInfo >::iterator it = tmp_cache.begin() + 1,
              end = tmp_cache.end();
          it != end;
          ++it )
    {
        if ( best->ballDist() < safe_ball_dist
             && it->ballDist() < safe_ball_dist )
        {
            if ( best->stamina() < it->stamina() )
            {
                best = &(*it);
            }
        }
        else
        {
            if ( best->ballDist() > it->ballDist()
                 || ( std::fabs( best->ballDist() - it->ballDist() ) < 0.001
                      && best->stamina() < it->stamina() ) )
            {
                best = &(*it);
            }
        }
    }

#ifdef DEBUG_PRINT_ONE_STEP
    dlog.addText( Logger::INTERCEPT,
                  "<<<<< 1 dash: best (t=%d d=%d) self_pos=(%.2f %.2f) ball_dist=%.3f stamina=%.1f",
                  best->reachCycle(), best->turnCycle(), best->dashCycle(),
                  best->ballDist(),
                  best->selfPos().x, best->selfPos().y,
                  best->stamina() );
#endif

    self_cache.push_back( *best );
}

/*-------------------------------------------------------------------*/
/*!

 */
InterceptInfo
SelfInterceptSimulator::getOneAdjustDash( const WorldModel & wm,
                                          const AngleDeg & dash_angle,
                                          const Vector2D & max_forward_accel,
                                          const Vector2D & max_back_accel,
                                          const double control_area )
{
    const ServerParam & SP = ServerParam::i();

    const double control_buf = control_area - 0.075;

    const Vector2D self_next = wm.self().pos() + wm.self().vel();
    const Vector2D ball_next = wm.ball().pos() + wm.ball().vel();
    const AngleDeg dash_dir = dash_angle - wm.self().body();

    const Matrix2D rotate = Matrix2D::make_rotation( -dash_angle );

    const Vector2D ball_rel = rotate.transform( ball_next - self_next );
    const Vector2D forward_accel_rel = rotate.transform( max_forward_accel );
    const Vector2D back_accel_rel = rotate.transform( max_back_accel );

    const double dash_rate = wm.self().dashRate() * SP.dashDirRate( dash_dir.degree() );

    if ( ball_rel.absY() > control_buf
         || Segment2D( forward_accel_rel, back_accel_rel ).dist( ball_rel ) > control_buf )
    {
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "__(predictOneDashAdjust) out of control area=%.3f"
                      " ball_absY=%.3f forward_dist=%.3f back_dist=%.3f",
                      control_buf, ball_rel.absY(),
                      ball_rel.dist( forward_accel_rel ),
                      ball_rel.dist( back_accel_rel ) );
#endif
        return InterceptInfo();
    }

    double dash_power = -1000.0;

    //
    // small x difference
    // player can put the ball on his side.
    //
    if ( back_accel_rel.x < ball_rel.x
         && ball_rel.x < forward_accel_rel.x )
    {
        dash_power = getOneStepDashPower( wm,
                                          ball_rel,
                                          dash_angle,
                                          forward_accel_rel.x,
                                          back_accel_rel.x );
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "__(predictOneDashAdjust) (1). dash power=%.1f",
                      dash_power );
#endif
    }

    //
    // big x difference (>0)
    //
    if ( dash_power < -999.0
         && forward_accel_rel.x < ball_rel.x )
    {
        double enable_ball_dist = ball_rel.dist( forward_accel_rel );
        if ( enable_ball_dist < control_buf )
        {
            // at least, reach the controllale distance
            dash_power = forward_accel_rel.x / dash_rate;
#ifdef DEBUG_PRINT_ONE_STEP
            dlog.addText( Logger::INTERCEPT,
                          "__(predictOneDashAdjust) (2). Not Best. next_ball_dist=%.3f power=%.1f",
                          enable_ball_dist, dash_power );
#endif
        }
    }

    //
    // big x difference (<0)
    //
    if ( dash_power < -999.0
         && ball_rel.x < back_accel_rel.x )
    {
        double enable_ball_dist = ball_rel.dist( back_accel_rel );
        if ( enable_ball_dist < control_buf )
        {
            dash_power = back_accel_rel.x / dash_rate;
#ifdef DEBUG_PRINT_ONE_STEP
            dlog.addText( Logger::INTERCEPT,
                          "__(predictOneDashAdjust) (3). Not Best next_ball_dist=%.3f power=%.1f",
                          enable_ball_dist, dash_power );
#endif
        }
    }

    //
    // check if adjustable
    //
    if ( dash_power < -999.0
         && back_accel_rel.x < ball_rel.x
         && ball_rel.x < forward_accel_rel.x )
    {
        dash_power = ball_rel.x / dash_rate;
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "__(predictOneDashAdjust) (4). Not Best. just adjust X. power=%.1f",
                      dash_power );
#endif
    }

    //
    // not found
    //
    if ( dash_power < -999.0 )
    {
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "__(predictOneDashAdjust) XXX Failed" );
#endif
        return InterceptInfo();
    }

    //
    // register
    //
    InterceptInfo::Mode mode = InterceptInfo::NORMAL;

    const Vector2D accel = Vector2D::polar2vector( dash_power * dash_rate, dash_angle );
    const Vector2D self_next_after_dash = wm.self().pos() + wm.self().vel() + accel;

    StaminaModel stamina_model = wm.self().staminaModel();
    stamina_model.simulateDash( wm.self().playerType(), dash_power );

    if ( stamina_model.stamina() < SP.recoverDecThrValue()
         && ! stamina_model.capacityIsEmpty() )
    {
        mode = InterceptInfo::EXHAUST;
    }

    InterceptInfo info( mode, 0, 1, dash_power, dash_dir,
                        self_next_after_dash,
                        self_next_after_dash.dist( ball_next ),
                        stamina_model.stamina() );

#ifdef DEBUG_PRINT_ONE_STEP
    dlog.addText( Logger::INTERCEPT,
                  "__*** (predictOneDashAdjust) --->Success! power=%.3f rel_dir=%.1f angle=%.1f"
                  " my_pos=(%.2f %.2f) ball_dist=%.3f stamina=%.1f",
                  info.dashPower(),
                  info.dashAngle().degree(),
                  dash_angle.degree(),
                  self_next_after_dash.x, self_next_after_dash.y,
                  info.ballDist(),
                  stamina_model.stamina() );
#endif
    return info;

}

/*-------------------------------------------------------------------*/
/*!

 */
double
SelfInterceptSimulator::getOneStepDashPower( const WorldModel & wm,
                                             const Vector2D & ball_next_rel,
                                             const AngleDeg & dash_angle,
                                             const double max_forward_accel_x,
                                             const double max_back_accel_x )
{
    const PlayerType & ptype = wm.self().playerType();

    const double dash_dir = ( dash_angle - wm.self().body() ).degree();
    const double dash_rate = ( wm.self().dashRate()
                               * ServerParam::i().dashDirRate( dash_dir ) );

    const double best_ctrl_dist_forward = ( ptype.playerSize()
                                            + 0.5 * ptype.kickableMargin()
                                            + ServerParam::i().ballSize() );
    const double best_ctrl_dist_backward = ( ptype.playerSize()
                                             + 0.3 * ptype.kickableMargin()
                                             + ServerParam::i().ballSize() );

    if ( ball_next_rel.absY() > best_ctrl_dist_forward )
    {
        // just put the ball at player's side
        return ball_next_rel.x / dash_rate;
    }

    const double forward_trap_accel_x
        = ball_next_rel.x
        - std::sqrt( std::pow( best_ctrl_dist_forward, 2 )
                     - std::pow( ball_next_rel.y, 2 ) );
    const double backward_trap_accel_x
        = ball_next_rel.x
        + std::sqrt( std::pow( best_ctrl_dist_backward, 2 )
                     - std::pow( ball_next_rel.y, 2 ) );

    //
    // get the minimum dash power
    //

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
            double power = accel_x / dash_rate;
            if ( std::fabs( power ) < std::fabs( min_power ) )
            {
                min_power = power;
            }
        }
    }

    if ( min_power < 1000.0 )
    {
        return min_power;
    }

    return -1000.0;
}

namespace {
/*-------------------------------------------------------------------*/
/*!

 */
int
get_min_step( const WorldModel & wm )
{
    const Rect2D pitch_rect = Rect2D::from_center( Vector2D( 0.0, 0.0 ),
                                                   ServerParam::i().pitchLength() + 10.0,
                                                   ServerParam::i().pitchWidth() + 10.0 );
    Vector2D final_pos = wm.ball().inertiaFinalPoint();

    if ( ! pitch_rect.contains( final_pos ) )
    {
        Vector2D sol1, sol2;
        int n = pitch_rect.intersection( Segment2D( wm.ball().pos(), final_pos ),
                                         &sol1, &sol2 );
        if ( n == 1 )
        {
            final_pos = sol1;
        }
    }

    const Segment2D ball_move( wm.ball().pos(), final_pos );
    const double dist
        = ball_move.dist( wm.self().pos() )
        - wm.self().playerType().kickableArea();
    int min_step
        = static_cast< int >( std::ceil( dist / wm.self().playerType().realSpeedMax() ) );

    return std::max( 2, min_step - 2 );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
simulate_turn_step( const WorldModel & wm,
                    const Vector2D & ball_pos,
                    const double control_area,
                    const int move_step,
                    const bool back_dash,
                    AngleDeg * result_dash_angle )
{
    const Vector2D inertia_self_pos = wm.self().inertiaPoint( move_step );
    const Vector2D inertia_rel = ball_pos - inertia_self_pos;
    const double inertia_dist = inertia_rel.r();

    int n_turn = 0;

    if ( control_area < inertia_dist )
    {
        const ServerParam & SP = ServerParam::i();
        const PlayerType & ptype = wm.self().playerType();

        AngleDeg dash_angle = wm.self().body();
        if ( back_dash ) dash_angle += 180.0;

        const AngleDeg target_angle = inertia_rel.th();
        const double turn_margin = std::max( 12.5, // Magic Number
                                             AngleDeg::asin_deg( control_area / inertia_dist ) );

        double angle_diff = ( target_angle - dash_angle ).abs();
        double speed = wm.self().vel().r();
        while ( angle_diff > turn_margin )
        {
            angle_diff -= ptype.effectiveTurn( SP.maxMoment(), speed );
            speed *= ptype.playerDecay();
            ++n_turn;
        }

        if ( result_dash_angle )
        {
            if ( angle_diff <= 0.0 )
            {
                *result_dash_angle = target_angle;
            }
            else
            {
                AngleDeg rel_angle = dash_angle - target_angle;
                if ( rel_angle.degree() > 0.0 )
                {
                    *result_dash_angle = target_angle + angle_diff;
                }
                else
                {
                    *result_dash_angle = target_angle - angle_diff;
                }
            }
        }
    }

    return n_turn;
}
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SelfInterceptSimulator::simulateTurnDash( const WorldModel & wm,
                                          const int max_step,
                                          const bool back_dash,
                                          std::vector< InterceptInfo > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const int min_step = get_min_step( wm );

    Vector2D ball_pos = wm.ball().inertiaPoint( min_step - 1 );
    Vector2D ball_vel = wm.ball().vel() * std::pow( SP.ballDecay(), min_step - 1 );

    for ( int step = min_step; step <= max_step; ++step )
    {
        ball_pos += ball_vel;
        ball_vel *= SP.ballDecay();

        bool goalie_mode
            = ( wm.self().goalie()
                && wm.lastKickerSide() != wm.ourSide()
                && ball_pos.x < SP.ourPenaltyAreaLineX() - 0.5
                && ball_pos.absY() < SP.penaltyAreaHalfWidth() - 0.5 );

        double control_area = ( goalie_mode
                                ? wm.self().playerType().reliableCatchableDist()
                                : wm.self().playerType().kickableArea() );

        InterceptInfo info = getTurnDash( wm, ball_pos, control_area, step, back_dash );
        if ( info.isValid() )
        {
            self_cache.push_back( info );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
InterceptInfo
SelfInterceptSimulator::getTurnDash( const WorldModel & wm,
                                     const Vector2D & ball_pos,
                                     const double control_area,
                                     const int step,
                                     const bool back_dash )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();

    AngleDeg dash_angle;
    const int n_turn = simulate_turn_step( wm, ball_pos, control_area, step, back_dash,
                                           &dash_angle );

    if ( n_turn >= step )
    {
        return InterceptInfo();
    }

    const AngleDeg body_angle = ( back_dash
                                  ? dash_angle + 180.0
                                  : dash_angle );

    const Matrix2D rotate_matrix = Matrix2D::make_rotation( -body_angle );

    Vector2D self_pos( 0.0, 0.0 );
    Vector2D self_vel = rotate_matrix.transform( wm.self().vel() );
    StaminaModel stamina_model = wm.self().staminaModel();

    for ( int i = 0; i < n_turn; ++i )
    {
        self_pos += self_vel;
        self_vel *= ptype.playerDecay();
        stamina_model.simulateWait( ptype );
    }

    const Vector2D ball_rel = rotate_matrix.transform( ball_pos - wm.self().pos() );

    if ( self_pos.dist2( ball_rel ) < std::pow( control_area, 2 ) )
    {
        InterceptInfo info( InterceptInfo::NORMAL, n_turn, 0,
                            0.0, 0.0,
                            wm.self().inertiaPoint( n_turn ),
                            self_pos.dist( ball_rel ),
                            stamina_model.stamina() );
        return info;
    }

    const int max_dash_step = step - n_turn;
    double first_dash_power = 0.0;
    for ( int i = 0; i < max_dash_step; ++i )
    {
        double required_vel_x = ( ball_rel.x - self_pos.x )
            * ( 1.0 - ptype.playerDecay() )
            / ( 1.0 - std::pow( ptype.playerDecay(), max_dash_step - i ) );
        double required_accel_x = required_vel_x - self_vel.x;
        double dash_power = required_accel_x / ( ptype.dashPowerRate() * stamina_model.effort() );
        dash_power = bound( SP.minDashPower(), dash_power, SP.maxDashPower() );
        dash_power = stamina_model.getSafetyDashPower( ptype, dash_power, 1.0 );

        double accel_x = dash_power * ptype.dashPowerRate() * stamina_model.effort();

        self_vel.x += accel_x;
        self_pos += self_vel;
        self_vel *= ptype.playerDecay();
        stamina_model.simulateDash( ptype, ( back_dash ? -dash_power : dash_power ) );

        if ( i == 0 )
        {
            first_dash_power = dash_power;
        }

        if ( std::fabs( required_accel_x ) < 1.0e-5
             || self_pos.absX() > ball_rel.absX() - 1.0e-5
             || self_pos.r2() > ball_rel.r2()
             || self_pos.dist2( ball_rel ) < std::pow( control_area, 2 ) )
        {
            InterceptInfo::Mode mode = ( stamina_model.recovery() < SP.recoverInit()
                                         && ! stamina_model.capacityIsEmpty()
                                         ? InterceptInfo::EXHAUST
                                         : InterceptInfo::NORMAL );
            return InterceptInfo( mode, n_turn, i + 1,
                                  first_dash_power, 0.0,
                                  wm.self().pos() + self_pos.rotatedVector( body_angle ),
                                  self_pos.dist( ball_rel ),
                                  stamina_model.stamina() );
        }
    }

    return InterceptInfo();
}


/*-------------------------------------------------------------------*/
/*!

 */
void
SelfInterceptSimulator::simulateOmniDash( const WorldModel & wm,
                                          const Vector2D & ball_pos,
                                          const double control_area,
                                          const int step,
                                          const bool back_dash,
                                          std::vector< InterceptInfo > & self_cache )
{
    (void)wm;
    (void)ball_pos;
    (void)control_area;
    (void)step;
    (void)back_dash;
    (void)self_cache;
}

}
