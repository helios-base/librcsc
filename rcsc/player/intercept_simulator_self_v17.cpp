// -*-c++-*-

/*!
  \file intercept_simulator_selv_v17.cpp
  \brief intercept simulator for self
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

#include "intercept_simulator_self_v17.h"

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

#include <sstream>

// #define DEBUG_PROFILE
// #define DEBUG_PRINT_RESULTS

// #define DEBUG_PRINT_ONE_STEP
// #define DEBUG_PRINT_TURN_DASH
// #define DEBUG_PRINT_OMNI_DASH

namespace rcsc {

namespace {
const double CONTROL_BUF = 0.15;
const double BALL_NOISE_RATE = 0.25;
const int BACK_DASH_COUNT_THR = 5;

#ifdef DEBUG_PRINT_RESULTS
/*-------------------------------------------------------------------*/
/*!

 */
char
type_char( const InterceptInfo::ActionType t )
{
    switch ( t ) {
    case InterceptInfo::OMNI_DASH:
        return 'o';
    case InterceptInfo::TURN_FORWARD_DASH:
        return 'f';
    case InterceptInfo::TURN_BACK_DASH:
        return 'b';
    default:
        break;
    }
    return 'u';
}

/*-------------------------------------------------------------------*/
/*!

 */
void
debug_print_results( const WorldModel & wm,
                     const std::vector< InterceptInfo > & self_cache )
{
    dlog.addText( Logger::INTERCEPT,
                  "(SelfIntercept) solution size = %zd kickable=%.3f catchable=%.3f:%.3f",
                  self_cache.size(),
                  wm.self().playerType().kickableArea(),
                  wm.self().playerType().reliableCatchableDist(),
                  wm.self().playerType().maxCatchableDist() );
    for ( size_t i = 0; i < self_cache.size(); ++i )
    {
        dlog.addText( Logger::INTERCEPT,
                      "%zd: mode=%d type=[%c] step=%d (t:%d d:%d)"
                      " power=%.2f angle=%.1f"
                      " self_pos=(%.2f %.2f) bdist=%.3f stamina=%.1f",
                      i,
                      self_cache[i].staminaType(),
                      type_char( self_cache[i].actionType() ),
                      self_cache[i].reachCycle(),
                      self_cache[i].turnCycle(),
                      self_cache[i].dashStep(),
                      self_cache[i].dashPower(),
                      self_cache[i].dashDir(),
                      self_cache[i].selfPos().x, self_cache[i].selfPos().y,
                      self_cache[i].ballDist(),
                      self_cache[i].stamina() );
    }
}
#endif

/*-------------------------------------------------------------------*/
/*!

 */
struct InterceptSorter {
    bool operator()( const Intercept & lhs,
                     const Intercept & rhs ) const
    {
        if ( lhs.reachStep() < rhs.reachStep() )
        {
            return true;
        }

        if ( lhs.reachStep() > rhs.reachStep() )
        {
            return false;
        }

        // reach steps are same

        if ( lhs.turnStep() < rhs.turnStep() )
        {
            return true;
        }

        if ( lhs.turnStep() > rhs.turnStep() )
        {
            return false;
        }

        // turn steps are same

        // if ( lhs.type() < rhs.type() )
        // {
        //     return true;
        // }

        if ( std::fabs( lhs.stamina() - rhs.stamina() ) < 200.0 )
        {
            return lhs.ballDist() < rhs.ballDist();
        }

        return lhs.stamina() > rhs.stamina();
    }
};

/*-------------------------------------------------------------------*/
/*!

 */
struct InterceptEqual {
    bool operator()( const Intercept & lhs,
                     const Intercept & rhs ) const
    {
        return lhs.staminaType() == rhs.staminaType()
            && lhs.actionType() == rhs.actionType()
            && lhs.turnStep() == rhs.turnStep()
            && lhs.dashStep() == rhs.dashStep()
            && lhs.dashDir() == rhs.dashDir();
    }
};

/*-------------------------------------------------------------------*/
/*!

 */
struct InterceptEqualSimple {
    bool operator()( const Intercept & lhs,
                     const Intercept & rhs ) const
    {
        return lhs.actionType() == rhs.actionType()
            && lhs.turnStep() == rhs.turnStep()
            && lhs.dashStep() == rhs.dashStep();
    }
};
}

/*-------------------------------------------------------------------*/
/*!

 */
void
InterceptSimulatorSelfV17::simulate( const WorldModel & wm,
                                     const int max_step,
                                     std::vector< Intercept > & self_cache )
{
#ifdef DEBUG_PROFILE
    rcsc::Timer timer;
#endif

    if ( wm.kickableOpponent()
         //|| wm.kickableTeammate()
         )
    {
        M_ball_vel.assign( 0.0, 0.0 );
    }
    else
    {
        M_ball_vel = wm.ball().vel();
    }

    simulateOneStep( wm, self_cache );
    simulateTurnDash( wm, max_step, false, self_cache ); // forward dash
    if ( ServerParam::i().minDashPower() < ServerParam::i().maxDashPower() * -0.7 )
    {
        simulateTurnDash( wm, max_step, true, self_cache ); // back dash
    }
    simulateOmniDash( wm, max_step, self_cache ); // omni dash

    if ( self_cache.empty() )
    {
        simulateFinal( wm, max_step, self_cache );
    }

    std::sort( self_cache.begin(), self_cache.end(), InterceptSorter() );
#ifdef DEBUG_PRINT_RESULTS
    debug_print_results( wm, self_cache );
#endif
    self_cache.erase( std::unique( self_cache.begin(), self_cache.end(),
                                   InterceptEqualSimple() ),
                      self_cache.end() );

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::INTERCEPT,
                  __FILE__" (simulate) elapsed %f [ms]",
                  timer.elapsedReal() );
#endif
#ifdef DEBUG_PRINT_RESULTS
    debug_print_results( wm, self_cache );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
InterceptSimulatorSelfV17::simulateOneStep( const WorldModel & wm,
                                            std::vector< Intercept > & self_cache )
{
    const Vector2D ball_next = wm.ball().pos() + ballVel();
    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < ServerParam::i().ourPenaltyAreaLineX() - 0.5
            && ball_next.absY() < ServerParam::i().penaltyAreaHalfWidth() - 0.5 );
    const double control_area = ( goalie_mode
                                  ? wm.self().playerType().maxCatchableDist()
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
InterceptSimulatorSelfV17::simulateNoDash( const WorldModel & wm,
                                           std::vector< Intercept > & self_cache )
{
    const PlayerType & ptype = wm.self().playerType();

    const Vector2D self_next = wm.self().pos() + wm.self().vel();
    const Vector2D ball_next = wm.ball().pos() + ballVel();

    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < ServerParam::i().ourPenaltyAreaLineX() - 0.5
            && ball_next.absY() < ServerParam::i().penaltyAreaHalfWidth() - 0.5
            );
    const double control_area = ( goalie_mode
                                  ? ptype.maxCatchableDist()
                                  : ptype.kickableArea() );

    //const double ball_noise = wm.ball().vel().r() * ServerParam::i().ballRand() * BALL_NOISE_RATE;
    const double ball_noise = ballVel().r() * ServerParam::i().ballRand() * BALL_NOISE_RATE;
    const double ball_next_dist = self_next.dist( ball_next );

    if ( ball_next_dist > control_area - CONTROL_BUF - ball_noise )
    {
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "xx 0 dash: next_dist=%.3f catch=%.3f (catchable=%.3f noise=%.3f)",
                      ball_next_dist, control_area - CONTROL_BUF - ball_noise,
                      control_area, ball_noise );
#endif
        return false;
    }

    if ( goalie_mode )
    {
        StaminaModel stamina_model = wm.self().staminaModel();
        stamina_model.simulateWait( ptype );

        self_cache.emplace_back( Intercept::NORMAL,
                                 Intercept::TURN_FORWARD_DASH,
                                 1, 0.0, // 1 turn, but 0 moment
                                 0, 0.0, 0.0, // 0 dash
                                 self_next,
                                 ball_next_dist,
                                 stamina_model.stamina() );
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
        Vector2D ball_next_vel = ballVel() * ServerParam::i().ballDecay();

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

    self_cache.emplace_back( Intercept::NORMAL,
                             Intercept::TURN_FORWARD_DASH,
                             0, 0.0,  // 0 turn
                             1, 0.0, 0.0, // 1 dash, 0 power
                             self_next,
                             ball_next_dist,
                             stamina_model.stamina() );
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
InterceptSimulatorSelfV17::simulateOneDash( const WorldModel & wm,
                                            std::vector< Intercept > & self_cache )
{
    simulateOneDashOld( wm, self_cache );
    // if ( ServerParam::i().minDashPower() < ServerParam::i().maxDashPower() * -0.7 )
    // {
    //     simulateOneDashOld( wm, self_cache );
    // }
    // else
    // {
    //     simulateOneDashAnyDir( wm, self_cache );
    // }
}


/*-------------------------------------------------------------------*/
void
InterceptSimulatorSelfV17::simulateOneDashAnyDir( const WorldModel & wm,
                                                  std::vector< Intercept > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();

    //const Vector2D self_next = wm.self().pos() + wm.self().vel();
    const Vector2D ball_next = wm.ball().pos() + ballVel();
    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < SP.ourPenaltyAreaLineX()
            && ball_next.absY() < SP.penaltyAreaHalfWidth()
            );

    Vector2D required_vel = ball_next - wm.self().pos();
    {
        const double best_ball_dist = ptype.playerSize() + SP.ballSize() + ptype.kickableMargin() * 0.5;
        const double best_move_dist = wm.self().pos().dist( ball_next ) - best_ball_dist;
        if ( best_move_dist > 0.0 )
        {
            required_vel.setLength( best_move_dist );
        }
        else
        {
            required_vel *= -1.0;
            required_vel.setLength( std::fabs( best_move_dist ) );
        }
    }

    const Vector2D required_accel = required_vel - wm.self().vel();

    StaminaModel stamina_model = wm.self().staminaModel();

    const double dash_dir = SP.discretizeDashAngle( ( required_accel.th() - wm.self().body() ).degree() );
    const double dash_rate =  SP.dashDirRate( dash_dir ) * ptype.dashPowerRate() * stamina_model.effort();

    const double required_dash_power = std::min( required_accel.r() / dash_rate, SP.maxDashPower() );
    const double safe_dash_power = stamina_model.getSafetyDashPower( ptype, required_dash_power, 1.0 );

    Vector2D self_pos = wm.self().pos();

    double dash_power = safe_dash_power;
    bool ok = false;
    {
        // safety dash power case
        const Vector2D dash_accel = Vector2D::from_polar( safe_dash_power * dash_rate, wm.self().body() + dash_dir );
        const Vector2D self_vel = wm.self().vel() + dash_accel;
        self_pos = wm.self().pos() + self_vel;
        if ( goalie_mode
             && ptype.getCatchProbability( self_pos, wm.self().body(), ball_next, 0.05, 1.0 ) > 0.9 )
        {
            ok = true;
            stamina_model.simulateDash( ptype, safe_dash_power );
        }

        if ( ! ok
             && self_pos.dist( ball_next ) < ptype.kickableArea() - 0.075 )
        {
            ok = true;
            stamina_model.simulateDash( ptype, safe_dash_power );
        }
    }

    if ( ! ok
         && required_dash_power > safe_dash_power )
    {
        // exhaust case
        const Vector2D dash_accel = Vector2D::from_polar( required_dash_power * dash_rate, wm.self().body() + dash_dir );
        const Vector2D self_vel = wm.self().vel() + dash_accel;
        self_pos = wm.self().pos() + self_vel;

        if ( goalie_mode
             && ptype.getCatchProbability( self_pos, wm.self().body(), ball_next, 0.05, 1.0 ) > 0.9 )
        {
            ok = true;
            dash_power = required_dash_power;
            stamina_model.simulateDash( ptype, dash_power );
        }

        if ( ! ok
             && self_pos.dist( ball_next ) < ptype.kickableArea() - 0.075 )
        {
            ok = true;
            dash_power = required_dash_power;
            stamina_model.simulateDash( ptype, dash_power );
        }
    }

    if ( ! ok )
    {
        // not found
#ifdef DEBUG_PRINT_ONE_STEP
        dlog.addText( Logger::INTERCEPT,
                      "<<<<< 1 dash: not found" );
#endif
        return;
    }

    Intercept::StaminaType stamina_type = Intercept::NORMAL;
    if ( stamina_model.recovery() < wm.self().staminaModel().recovery() - 1.0e-5
         && ! stamina_model.capacityIsEmpty() )
    {
        stamina_type = Intercept::EXHAUST;
    }

    const double result_ball_dist = self_pos.dist( ball_next );
#ifdef DEBUG_PRINT_ONE_STEP
    dlog.addText( Logger::INTERCEPT,
                  "<<<<< 1 dash: power=%.1f dir=%.1f self_pos=(%.2f %.2f) ball_dist=%.3f",
                  dash_power, dash_dir,
                  self_pos.x, self_pos.y, result_ball_dist );
#endif

    self_cache.emplace_back( stamina_type,
                             Intercept::OMNI_DASH,
                             0, // turn step
                             0, // turn angle
                             1, // dash step
                             dash_power,
                             dash_dir,
                             self_pos,
                             result_ball_dist,
                             stamina_model.stamina() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
InterceptSimulatorSelfV17::simulateOneDashOld( const WorldModel & wm,
                                               std::vector< Intercept > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();

    const Vector2D ball_next = wm.ball().pos() + ballVel();
    const bool goalie_mode
        = ( wm.self().goalie()
            && wm.lastKickerSide() != wm.ourSide()
            && ball_next.x < SP.ourPenaltyAreaLineX()
            && ball_next.absY() < SP.penaltyAreaHalfWidth()
            );
    const double control_area = ( goalie_mode
                                  ? ptype.maxCatchableDist()
                                  : ptype.kickableArea() );

    const double dash_angle_step = std::max( 2.0, SP.dashAngleStep() );
    const size_t dash_angle_divs
                             = static_cast< size_t >( std::floor( ( SP.maxDashAngle() - SP.minDashAngle() )
                                                                  / dash_angle_step ) );

    std::vector< Intercept > tmp_cache;
    tmp_cache.reserve( dash_angle_divs );

    for ( size_t d = 0 ; d < dash_angle_divs; ++d )
    {
        const double dir = AngleDeg::normalize_angle( SP.minDashAngle() + ( dash_angle_step * d ) );
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

            Intercept safe_dash = getOneAdjustDash( wm,
                                                    dash_angle,
                                                    max_forward_accel,
                                                    max_back_accel,
                                                    control_area );
            if ( safe_dash.isValid() )
            {
#ifdef DEBUG_PRINT_ONE_STEP
                dlog.addText( Logger::INTERCEPT,
                              "OK 1 dash: mode=%d power=%.1f dir=%.1f self_pos=(%.2f %.2f) stamina=%.1f",
                              safe_dash.staminaType(),
                              safe_dash.dashPower(),
                              safe_dash.dashDir(),
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

            Intercept exhaust_dash = getOneAdjustDash( wm, dash_angle,
                                                       max_forward_accel,
                                                       max_back_accel,
                                                       control_area );
            if ( exhaust_dash.isValid() )
            {
#ifdef DEBUG_PRINT_ONE_STEP
                dlog.addText( Logger::INTERCEPT,
                              "OK 1 dash: mode=%d power=%.1f dir=%.1f self_pos=(%.2f %.2f) stamina=%.1f",
                              exhaust_dash.staminaType(),
                              exhaust_dash.dashPower(),
                              exhaust_dash.dashDir(),
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
        = std::max( control_area - 0.2 - ballVel().r() * SP.ballRand(),
                    ptype.playerSize() + SP.ballSize() + ptype.kickableMargin() * 0.4 );


    const Intercept * best = &(tmp_cache.front());

    for ( std::vector< Intercept >::iterator it = tmp_cache.begin() + 1, end = tmp_cache.end();
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
                  best->reachStep(), best->turnStep(), best->dashStep(),
                  best->ballDist(),
                  best->selfPos().x, best->selfPos().y,
                  best->stamina() );
#endif

    self_cache.push_back( *best );
}

/*-------------------------------------------------------------------*/
/*!

 */
Intercept
InterceptSimulatorSelfV17::getOneAdjustDash( const WorldModel & wm,
                                             const AngleDeg & dash_angle,
                                             const Vector2D & max_forward_accel,
                                             const Vector2D & max_back_accel,
                                             const double control_area )
{
    const ServerParam & SP = ServerParam::i();

    const double control_buf = control_area - 0.075;

    const Vector2D self_next = wm.self().pos() + wm.self().vel( );
    const Vector2D ball_next = wm.ball().pos() + ballVel();
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
                      "__(getOneAdjustDash) out of control area=%.3f"
                      " ball_absY=%.3f forward_dist=%.3f back_dist=%.3f",
                      control_buf, ball_rel.absY(),
                      ball_rel.dist( forward_accel_rel ),
                      ball_rel.dist( back_accel_rel ) );
#endif
        return Intercept();
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
                      "__(getOneAdjustDash) (1). dash power=%.1f",
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
                          "__(getOneAdjustDash) (2). Not Best. next_ball_dist=%.3f power=%.1f",
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
                          "__(getOneAdjustDash) (3). Not Best next_ball_dist=%.3f power=%.1f",
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
                      "__(getOneAdjustDash) (4). Not Best. just adjust X. power=%.1f",
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
                      "__(getOneAdjustDash) XXX Failed" );
#endif
        return Intercept();
    }

    //
    // register
    //
    Intercept::StaminaType stamina_type = Intercept::NORMAL;

    const Vector2D accel = Vector2D::polar2vector( dash_power * dash_rate, dash_angle );
    const Vector2D self_next_after_dash = wm.self().pos() + wm.self().vel() + accel;

    StaminaModel stamina_model = wm.self().staminaModel();
    stamina_model.simulateDash( wm.self().playerType(), dash_power );

    if ( stamina_model.recovery() < wm.self().staminaModel().recovery() - 1.0e-5
         && ! stamina_model.capacityIsEmpty() )
    {
        stamina_type = Intercept::EXHAUST;
    }

    Intercept info( stamina_type,
                    ( dash_power > 0.0
                      ? Intercept::TURN_FORWARD_DASH
                      : Intercept::TURN_BACK_DASH ),
                    0, 0.0,
                    1, dash_power, dash_dir.degree(),
                    self_next_after_dash,
                    self_next_after_dash.dist( ball_next ),
                    stamina_model.stamina() );

#ifdef DEBUG_PRINT_ONE_STEP
    dlog.addText( Logger::INTERCEPT,
                  "__*** (predictOneDashAdjust) --->Success! power=%.3f rel_dir=%.1f angle=%.1f"
                  " my_pos=(%.2f %.2f) ball_dist=%.3f stamina=%.1f",
                  info.dashPower(),
                  info.dashDir(),
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
InterceptSimulatorSelfV17::getOneStepDashPower( const WorldModel & wm,
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
get_min_step( const WorldModel & wm,
              const Vector2D & ball_vel )
{
    const Rect2D pitch_rect = Rect2D::from_center( Vector2D( 0.0, 0.0 ),
                                                   ServerParam::i().pitchLength() + 10.0,
                                                   ServerParam::i().pitchWidth() + 10.0 );
    //Vector2D final_pos = wm.ball().inertiaFinalPoint();
    Vector2D final_pos = inertia_final_point( wm.ball().pos(),
                                              ball_vel,
                                              ServerParam::i().ballDecay() );

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
                    const double ball_noise,
                    const int move_step,
                    const bool back_dash,
                    AngleDeg * result_dash_angle )
{
    const Vector2D inertia_self_pos = wm.self().inertiaPoint( move_step );
    const Vector2D inertia_rel = ball_pos - inertia_self_pos;
    const double inertia_dist = inertia_rel.r();

    int n_turn = 0;

    if ( control_area - CONTROL_BUF - ball_noise < inertia_dist )
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
InterceptSimulatorSelfV17::simulateTurnDash( const WorldModel & wm,
                                             const int max_step,
                                             const bool back_dash,
                                             std::vector< Intercept > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();
    const int min_step = get_min_step( wm, ballVel() );

    //Vector2D ball_pos = wm.ball().inertiaPoint( min_step - 1 );
    Vector2D ball_pos = inertia_n_step_point( wm.ball().pos(), ballVel(), min_step - 1, SP.ballDecay() );
    //Vector2D ball_vel = wm.ball().vel() * std::pow( SP.ballDecay(), min_step - 1 );
    Vector2D ball_vel = ballVel() * std::pow( SP.ballDecay(), min_step - 1 );
    double ball_speed = ball_vel.r();

    int success_count = 0;

    for ( int step = min_step; step <= max_step; ++step )
    {
        const double ball_noise = ball_speed * SP.ballRand() * BALL_NOISE_RATE;
#ifdef DEBUG_PRINT_TURN_DASH
        dlog.addText( Logger::INTERCEPT,
                      "==== %d: (simulateTurnDash) ball=(%.2f %.2f) speed=%.3f noise=%.3f back_dash=[%s]",
                      step, ball_pos.x, ball_pos.y, ball_speed, ball_noise,
                      ( back_dash ? "true" : "false" ) );
#endif
        ball_pos += ball_vel;
        ball_vel *= SP.ballDecay();
        ball_speed *= SP.ballDecay();

        const bool goalie_mode = ( wm.self().goalie()
                                   && wm.lastKickerSide() != wm.ourSide()
                                   && ball_pos.x < SP.ourPenaltyAreaLineX() - 0.5
                                   && ball_pos.absY() < SP.penaltyAreaHalfWidth() - 0.5 );

        if ( back_dash
             && ! goalie_mode
             && step >= BACK_DASH_COUNT_THR )
        {
            break;
        }

        const double control_area = ( goalie_mode
                                      ? ptype.maxCatchableDist()
                                      : ptype.kickableArea() );

        if ( wm.self().pos().dist2( ball_pos ) > std::pow( ptype.realSpeedMax() * step + control_area, 2 ) )
        {
#ifdef DEBUG_PRINT_TURN_DASH
            dlog.addText( Logger::INTERCEPT,
                          "%d: XX never reach move_dist=%.2f max_dist=%.2f",
                          step, wm.self().pos().dist( ball_pos ), ptype.realSpeedMax() * step + control_area );
#endif
            continue;
        }

        Intercept info = getTurnDash( wm, ball_pos, control_area, ball_noise, step, back_dash );
        if ( info.isValid() )
        {
#ifdef DEBUG_PRINT_TURN_DASH
            dlog.addText( Logger::INTERCEPT,
                          ">>>>> %d: (simulateTurnDash) OK turn=%d dash=%d",
                          step, info.turnStep(), info.dashStep() );
#endif
            self_cache.push_back( info );
            if ( ++success_count >= 10 )
            {
                break;
            }
        }
#ifdef DEBUG_PRINT_TURN_DASH
        else
        {
            dlog.addText( Logger::INTERCEPT,
                          "XXXXX %d: (simulateTurnDash) NG",
                          step );
        }
#endif
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
Intercept
InterceptSimulatorSelfV17::getTurnDash( const WorldModel & wm,
                                        const Vector2D & ball_pos,
                                        const double control_area,
                                        const double ball_noise,
                                        const int step,
                                        const bool back_dash )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();
    const bool goalie_mode = ( wm.self().goalie()
                               && wm.lastKickerSide() != wm.ourSide()
                               && ball_pos.x < ServerParam::i().ourPenaltyAreaLineX() - 0.5
                               && ball_pos.absY() < ServerParam::i().penaltyAreaHalfWidth() - 0.5 );
    const double control_buf = ( goalie_mode
                                 ? 0.0
                                 : CONTROL_BUF + ball_noise );

    AngleDeg dash_angle = ( back_dash
                            ? wm.self().body() + 180.0
                            : wm.self().body() );
    const int n_turn = simulate_turn_step( wm, ball_pos, control_area, ball_noise, step, back_dash,
                                           &dash_angle );

    if ( n_turn >= step )
    {
#ifdef DEBUG_PRINT_TURN_DASH
        dlog.addText( Logger::INTERCEPT,
                      "%d: xx (getTurnDash) n_turn=%d dash_angle=%.1f",
                      step, n_turn, dash_angle.degree() );
#endif
        return Intercept();
    }
    const AngleDeg body_angle = ( n_turn == 0
                                  ? wm.self().body()
                                  : back_dash
                                  ? dash_angle + 180.0
                                  : dash_angle );
#ifdef DEBUG_PRINT_TURN_DASH
    dlog.addText( Logger::INTERCEPT,
                  "%d: (getTurnDash) n_turn=%d dash_angle=%.1f body=%.1f back_dash=[%s]",
                  step, n_turn, dash_angle.degree(), body_angle.degree(),
                  ( back_dash ? "true" : "false" ) );
#endif

    const Matrix2D rotate_matrix = Matrix2D::make_rotation( -body_angle );

    {
        Vector2D self_inertia = wm.self().inertiaPoint( step );
        Vector2D ball_rel_to_inertia = rotate_matrix.transform( ball_pos - self_inertia );
#ifdef DEBUG_PRINT_TURN_DASH
        dlog.addText( Logger::INTERCEPT,
                      "%d: (getTurnDash) ball_rel after waits (%.2f %.2f)",
                      step, ball_rel_to_inertia.x, ball_rel_to_inertia.y );
#endif
        if ( ( back_dash && ball_rel_to_inertia.x > 0.0 )
             || ( ! back_dash && ball_rel_to_inertia.x < 0.0 ) )
        {
            return Intercept();
        }
    }

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
        stamina_model.simulateDash( ptype, dash_power );
#ifdef DEBUG_PRINT_TURN_DASH
        dlog.addText( Logger::INTERCEPT,
                      "%d: (getTurnDash) dash:%d power=%.1f req_acc_x=%.3f self_pos.x=%.2f ball_rel.x=%.2f dist=%.2f",
                      step, i + 1, dash_power, required_accel_x, self_pos.x, ball_rel.x, self_pos.dist( ball_rel ) );
#endif

        if ( i == 0 )
        {
            first_dash_power = dash_power;
        }
    }

    bool ok = false;

    if ( self_pos.absX() > ball_rel.absX() - 1.0e-5
         || self_pos.r2() > ball_rel.r2()
         || self_pos.dist2( ball_rel ) < std::pow( ptype.kickableArea() - control_buf, 2 ) )
    {
        ok = true;
    }

    if ( ! ok
         && goalie_mode
         && ptype.getCatchProbability( self_pos, 0.0, ball_rel, 0.05, 1.0 ) > 0.9 )
    {
        ok = true;
    }

    if ( ok )
    {
        Intercept::StaminaType stamina_type = ( stamina_model.recovery() < wm.self().staminaModel().recovery() - 1.0e-5
                                                && ! stamina_model.capacityIsEmpty()
                                                ? Intercept::EXHAUST
                                                : Intercept::NORMAL );
        return Intercept( stamina_type,
                          ( back_dash
                            ? Intercept::TURN_BACK_DASH
                            : Intercept::TURN_FORWARD_DASH ),
                          n_turn, ( body_angle - wm.self().body() ).degree(),
                          max_dash_step, first_dash_power, 0.0,
                          wm.self().pos() + self_pos.rotatedVector( body_angle ),
                          self_pos.dist( ball_rel ),
                          stamina_model.stamina() );
    }

    return Intercept();
}

/*-------------------------------------------------------------------*/
void
InterceptSimulatorSelfV17::simulateOmniDash( const WorldModel & wm,
                                             const int max_step,
                                             std::vector< Intercept > & self_cache )
{
    if ( ServerParam::i().dashAngleStep() > 1.5 )
    {
        simulateOmniDashOld( wm, max_step, self_cache );
    }
    else
    {
        simulateOmniDashAny( wm, max_step, self_cache );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
InterceptSimulatorSelfV17::simulateOmniDashAny( const WorldModel & wm,
                                                const int max_step,
                                                std::vector< Intercept > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType & ptype = wm.self().playerType();
    const double max_side_speed = ( SP.maxDashPower()
                                    * ptype.dashPowerRate()
                                    * ptype.effortMax()
                                    * SP.dashDirRate( 90.0 ) ) / ( 1.0 - ptype.playerDecay() );
    const Matrix2D rotate_matrix = Matrix2D::make_rotation( -wm.self().body() );
    //const double first_ball_speed = wm.ball().vel().r();
    const double first_ball_speed = ballVel().r();

#ifdef DEBUG_PRINT_OMNI_DASH
    dlog.addText( Logger::INTERCEPT,
                  "===== (simulateOmniDashAny) max_step=%d", max_step );
#endif

    int success_count = 0;
    for ( int ball_step = 1; ball_step <= max_step; ++ball_step )
    {
        //const Vector2D ball_pos = wm.ball().inertiaPoint( ball_step );
        const Vector2D ball_pos = inertia_n_step_point( wm.ball().pos(), ballVel(), ball_step, SP.ballDecay() );
        const bool goalie_mode = ( wm.self().goalie()
                                   && wm.lastKickerSide() != wm.ourSide()
                                   && ball_pos.x < SP.ourPenaltyAreaLineX() - 0.5
                                   && ball_pos.absY() < SP.penaltyAreaHalfWidth() - 0.5 );
        const double control_area = ( goalie_mode
                                      ? ptype.maxCatchableDist()
                                      : ptype.kickableArea() );
        const double ball_noise = ( first_ball_speed * std::pow( SP.ballDecay(), ball_step - 1 )
                                    * SP.ballRand()
                                    * BALL_NOISE_RATE );
        const double control_buf =  ( goalie_mode
                                      ? 0.0
                                      : CONTROL_BUF + ball_noise );

        const Vector2D self_inertia = wm.self().inertiaPoint( ball_step );

        const Vector2D ball_rel = rotate_matrix.transform( ball_pos - self_inertia );
        if ( ball_rel.absY() - control_area > max_side_speed * ball_step )
        {
            continue;
        }

        //const AngleDeg accel_angle = ( ball_pos - self_inertia ).th();

        double first_dash_power = 0.0;
        double first_dash_dir = 0.0;

        Vector2D self_pos = wm.self().pos();
        Vector2D self_vel = wm.self().vel();
        StaminaModel stamina_model = wm.self().staminaModel();

        for ( int step = 1; step <= ball_step; ++step )
        {
            const Vector2D required_vel = ( ball_pos - self_pos )
                * ( ( 1.0 - ptype.playerDecay() )
                    / ( 1.0 - std::pow( ptype.playerDecay(), ball_step - step + 1 ) ) );
            const Vector2D required_accel = required_vel - self_vel;

            const double dash_dir = SP.discretizeDashAngle( ( required_accel.th() - wm.self().body() ).degree() );
            const double dash_rate = SP.dashDirRate( dash_dir ) * ptype.dashPowerRate() * stamina_model.effort();
            double dash_power = std::min( SP.maxDashPower(), required_accel.r() / dash_rate );
            dash_power = stamina_model.getSafetyDashPower( ptype, dash_power, 1.0 );

            if ( step == 1 )
            {
                first_dash_power = dash_power;
                first_dash_dir = dash_dir;
            }

            const Vector2D dash_accel = Vector2D::from_polar( dash_power * dash_rate, wm.self().body() + dash_dir );
            self_vel += dash_accel;
            self_pos += self_vel;
            self_vel *= ptype.playerDecay();
            stamina_model.simulateDash( ptype, dash_power );

            bool ok = false;

            if ( self_pos.dist2( ball_pos ) < std::pow( ptype.kickableArea() - control_buf, 2 )
                 || self_inertia.dist2( self_pos ) > self_inertia.dist2( ball_pos ) )
            {
                ok = true;
            }

            if ( ! ok
                 && goalie_mode
                 && ptype.getCatchProbability( self_pos, wm.self().body(), ball_pos, 0.05, 1.0 ) > 0.9 )
            {
                ok = true;
#ifdef DEBUG_PRINT_OMNI_DASH
                dlog.addText( Logger::INTERCEPT,
                              "[omni]>>> found in goalie mode" );
#endif
            }

            if ( ok )
            {
                Intercept::StaminaType stamina_type = ( stamina_model.recovery() < wm.self().staminaModel().recovery() - 1.0e-5
                                                        && ! stamina_model.capacityIsEmpty()
                                                        ? Intercept::EXHAUST
                                                        : Intercept::NORMAL );
                self_cache.emplace_back( stamina_type,
                                         Intercept::OMNI_DASH,
                                         0, 0.0,
                                         ball_step, first_dash_power, first_dash_dir,
                                         self_pos,
                                         self_pos.dist( ball_pos ),
                                         stamina_model.stamina() );
                ++success_count;
#ifdef DEBUG_PRINT_OMNI_DASH
                dlog.addText( Logger::INTERCEPT,
                              "[omni]>>> found ball_step=%d self_step=%d", ball_step, step );
#endif
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
InterceptSimulatorSelfV17::simulateOmniDashOld( const WorldModel & wm,
                                                const int max_step,
                                                std::vector< Intercept > & self_cache )
{
    const ServerParam & SP = ServerParam::i();
    const double dash_angle_step = std::max( 5.0, SP.dashAngleStep() );
    const size_t dash_angle_divs = static_cast< size_t >( std::floor( 360.0 / dash_angle_step ) );

    const PlayerType & ptype = wm.self().playerType();
    const double max_side_speed = ( SP.maxDashPower()
                                    * ptype.dashPowerRate()
                                    * ptype.effortMax()
                                    * SP.dashDirRate( 90.0 ) ) / ( 1.0 - ptype.playerDecay() );
    const Matrix2D rotate_matrix = Matrix2D::make_rotation( -wm.self().body() );

    std::vector< double > dash_powers;
    std::vector< double > dash_base_rates;
    std::vector< AngleDeg > accel_angles;
    std::vector< Matrix2D > accel_rot_matrix;
    std::vector< Matrix2D > accel_inv_matrix;
    dash_powers.reserve( dash_angle_divs );
    dash_base_rates.reserve( dash_angle_divs );
    accel_angles.reserve( dash_angle_divs );
    accel_rot_matrix.reserve( dash_angle_divs );
    accel_inv_matrix.reserve( dash_angle_divs );

    for ( size_t d = 0; d < dash_angle_divs; ++d )
    {
        double dir = SP.discretizeDashAngle( SP.minDashAngle() + dash_angle_step * d );
        AngleDeg accel_angle = wm.self().body() + dir;
        double forward_dash_rate = SP.dashDirRate( dir );
        double back_dash_rate = SP.dashDirRate( AngleDeg::normalize_angle( dir + 180.0 ) );
        if ( std::fabs( forward_dash_rate * SP.maxDashPower() )
             > std::fabs( back_dash_rate * SP.minDashPower() ) - 0.001 )
        {
            dash_powers.push_back( SP.maxDashPower() );
            dash_base_rates.push_back( ptype.dashPowerRate() * forward_dash_rate );
        }
        else
        {
            dash_powers.push_back( SP.minDashPower() );
            dash_base_rates.push_back( ptype.dashPowerRate() * back_dash_rate );
        }
        accel_angles.push_back( accel_angle );
        accel_rot_matrix.push_back( Matrix2D::make_rotation( -accel_angle ) );
        accel_inv_matrix.push_back( Matrix2D::make_rotation( accel_angle ) );
    }

    //
    // simulation loop
    //
    const double first_ball_speed = ballVel().r();

#ifdef DEBUG_PRINT_OMNI_DASH
    dlog.addText( Logger::INTERCEPT,
                  "===== (simulateOmniDash) max_step=%d", max_step );
#endif

    int success_count = 0;
    int failed_count_after_success = 0;
    double last_y_diff = 100000.0;
    for ( int reach_step = 1; reach_step <= max_step; ++reach_step )
    {
        //const Vector2D ball_pos = wm.ball().inertiaPoint( reach_step );
        const Vector2D ball_pos = inertia_n_step_point( wm.ball().pos(), ballVel(), reach_step, SP.ballDecay() );
        const bool goalie_mode
            = ( wm.self().goalie()
                && wm.lastKickerSide() != wm.ourSide()
                && ball_pos.x < SP.ourPenaltyAreaLineX() - 0.5
                && ball_pos.absY() < SP.penaltyAreaHalfWidth() - 0.5 );
        const double control_area = ( goalie_mode
                                      ? ptype.maxCatchableDist()
                                      : ptype.kickableArea() );
        {
            const Vector2D ball_rel = rotate_matrix.transform( ball_pos - wm.self().pos() );
            if ( ball_rel.absY() - control_area > max_side_speed * reach_step )
            {
#ifdef DEBUG_PRINT_OMNI_DASH
                dlog.addText( Logger::INTERCEPT,
                              "xx %d: ball_rel=(%.2f %.2f) max_side_move=%.2f (max_side_speed=%.2f)",
                              reach_step, ball_rel.x, ball_rel.y,
                              max_side_speed * reach_step, max_side_speed );
#endif
                if ( success_count > 0 )
                {
#ifdef DEBUG_PRINT_OMNI_DASH
                    dlog.addText( Logger::INTERCEPT,
                                  "<<<<< (simulateOmniDash) exist success result. finish." );
#endif
                    break;
                }

                if ( last_y_diff < ball_rel.absY() )
                {
#ifdef DEBUG_PRINT_OMNI_DASH
                    dlog.addText( Logger::INTERCEPT,
                                  "<<<<< (simulateOmniDash) y diff will become larger. finish." );
#endif
                    break;
                }
                last_y_diff = ball_rel.absY();

                continue;
            }

            last_y_diff = ball_rel.absY();
        }

        const double ball_noise = ( first_ball_speed * std::pow( SP.ballDecay(), reach_step - 1 )
                                    * SP.ballRand()
                                    * BALL_NOISE_RATE );
        const double control_buf = ( goalie_mode
                                     ? 0.0
                                     : CONTROL_BUF + ball_noise );

        double first_dash_power = 0.0;
        double first_dash_dir = 0.0;

        Vector2D self_pos = wm.self().pos();
        Vector2D self_vel = wm.self().vel();
        StaminaModel stamina_model = wm.self().staminaModel();

        bool found = false;
        int back_dash_count = 0;
#ifdef DEBUG_PRINT_OMNI_DASH
        std::vector< std::pair< double, double > > dash_list;
        dlog.addText( Logger::INTERCEPT,
                      "%d: ball_pos=(%.2f %.2f) ball_noise=%.3f",
                      reach_step, ball_pos.x, ball_pos.y, ball_noise );
#endif
        for ( int step = 1; step <= reach_step; ++step )
        {
            const Vector2D required_vel = ( ball_pos - self_pos )
                * ( ( 1.0 - ptype.playerDecay() )
                    / ( 1.0 - std::pow( ptype.playerDecay(), reach_step - step + 1 ) ) );
            const Vector2D required_accel = required_vel - self_vel;

            double min_dist2 = 1000000.0;
            Vector2D best_self_pos = self_pos;
            Vector2D best_self_vel = self_vel;
            double best_dash_power = 0.0;
            double best_dash_dir = 0.0;

            for ( size_t d = 0; d < dash_angle_divs; ++d )
            {
                const Vector2D rel_accel = accel_rot_matrix[d].transform( required_accel );
                if ( rel_accel.x < 0.0 )
                {
                    continue;
                }

                const double dash_rate = dash_base_rates[d] * stamina_model.effort();
                double dash_power = rel_accel.x / dash_rate;
                dash_power = std::min( std::fabs( dash_powers[d] ), dash_power );
                if ( dash_powers[d] < 0.0 ) dash_power = -dash_power;
                dash_power = stamina_model.getSafetyDashPower( ptype, dash_power, 1.0 );

                double accel_mag = std::fabs( dash_power ) * dash_rate;
                Vector2D dash_accel = accel_inv_matrix[d].transform( Vector2D( accel_mag, 0.0 ) );
                Vector2D tmp_vel = self_vel + dash_accel;
                Vector2D tmp_pos = self_pos + tmp_vel;
#if 0
                double d2 = tmp_pos.dist2( ball_pos );
#else
                Vector2D rel_to_body = rotate_matrix.transform( ball_pos - tmp_pos );
                //double d2 = std::pow( rel_to_body.x, 2 ) + std::pow( rel_to_body.y * 5.0, 2 );
                //double d2 = std::pow( rel_to_body.x, 2 ) + std::pow( rel_to_body.y * 3.0, 2 );
                double d2 = std::pow( rel_to_body.x, 2 ) + std::pow( rel_to_body.y * 1.5, 2 );
#endif
                if ( d2 < min_dist2 )
                {
                    min_dist2 = d2;
                    best_self_pos = tmp_pos;
                    best_self_vel = tmp_vel;
                    best_dash_power = dash_power;
                    best_dash_dir = SP.minDashAngle() + dash_angle_step * d;
                    if ( dash_power < 0.0 )
                    {
                        best_dash_dir = AngleDeg::normalize_angle( best_dash_dir + 180.0 );
                    }
                }
            }

            self_pos = best_self_pos;
            self_vel = best_self_vel;
            self_vel *= ptype.playerDecay();
            stamina_model.simulateDash( ptype, best_dash_power );

#ifdef DEBUG_PRINT_OMNI_DASH
            dash_list.push_back( std::make_pair( best_dash_power, best_dash_dir ) );
#endif
            if ( step == 1 )
            {
                first_dash_power = best_dash_power;
                first_dash_dir = best_dash_dir;
            }

            if ( ! found )
            {
                if ( self_pos.dist2( ball_pos ) < std::pow( control_area - control_buf, 2 )
                     || ( wm.self().pos().dist2( self_pos ) > wm.self().pos().dist2( ball_pos )
                          && Line2D( wm.self().pos(), self_pos ).dist2( ball_pos ) < std::pow( control_area, 2 ) ) )
                {
                    found = true;
                }
            }

            if ( ! found
                 && best_dash_power < 0.0 )
            {
                ++back_dash_count;
                if ( ! goalie_mode
                     && back_dash_count >= BACK_DASH_COUNT_THR )
                {
                    break;
                }
            }
        }

        if ( found )
        {
#ifdef DEBUG_PRINT_OMNI_DASH
            dlog.addText( Logger::INTERCEPT,
                          "OK %d: (simulateOmniDash) power=%.1f dir=%.1f self=(%.2f %.2f)"
                          " ball=(%.2f %.2f) dist=%.3f control=%.3f(%.3f %.3f %.3f)",
                          reach_step, first_dash_power, first_dash_dir,
                          self_pos.x, self_pos.y,
                          ball_pos.x, ball_pos.y,
                          self_pos.dist( ball_pos ),
                          control_area - control_buf,
                          control_area, CONTROL_BUF, ball_noise );
            std::ostringstream ostr;
            for ( size_t i = 0; i < dash_list.size(); ++i )
            {
                ostr << '(' << dash_list[i].first << ' ' << dash_list[i].second << ')';
            }
            dlog.addText( Logger::INTERCEPT,
                          ">>> dash %s", ostr.str().c_str() );
#endif
            Intercept::StaminaType stamina_type = ( stamina_model.recovery() < wm.self().staminaModel().recovery() - 1.0e-5
                                                    && ! stamina_model.capacityIsEmpty()
                                                    ? Intercept::EXHAUST
                                                    : Intercept::NORMAL );
            self_cache.emplace_back( stamina_type,
                                     Intercept::OMNI_DASH,
                                     0, 0.0,
                                     reach_step, first_dash_power, first_dash_dir,
                                     self_pos,
                                     self_pos.dist( ball_pos ),
                                     stamina_model.stamina() );
            ++success_count;
        }
        else
        {
#ifdef DEBUG_PRINT_OMNI_DASH
            dlog.addText( Logger::INTERCEPT,
                          "xx %d: (simulateOmniDash) not found",
                          reach_step );
#endif
            if ( success_count > 0 )
            {
                if ( ++failed_count_after_success >= 2 )
                {
#ifdef DEBUG_PRINT_OMNI_DASH
                    dlog.addText( Logger::INTERCEPT,
                                  "<<<<< (simulateOmniDash) over failed count" );
#endif
                }
            }
        }

        if ( success_count >= 4 )
        {
#ifdef DEBUG_PRINT_OMNI_DASH
            dlog.addText( Logger::INTERCEPT,
                          "<<<<< (simulateOmniDash) over success count" );
#endif
            break;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
InterceptSimulatorSelfV17::simulateFinal( const WorldModel & wm,
                                          const int max_step,
                                          std::vector< Intercept > & self_cache )
{
    const PlayerType & ptype = wm.self().playerType();

    const Vector2D self_pos = wm.self().inertiaFinalPoint();
    //const Vector2D ball_pos = wm.ball().inertiaFinalPoint();
    const Vector2D ball_pos = inertia_final_point( wm.ball().pos(), ballVel(), ServerParam::i().ballDecay() );
    const bool goalie_mode = ( wm.self().goalie()
                               && wm.lastKickerSide() != wm.ourSide()
                               && ball_pos.x < ServerParam::i().ourPenaltyAreaLineX() - 0.5
                               && ball_pos.absY() < ServerParam::i().penaltyAreaHalfWidth() - 0.5 );
    const double control_area = ( goalie_mode
                                  ? ptype.reliableCatchableDist()
                                  : ptype.kickableArea() );

    AngleDeg dash_angle = wm.self().body();
    int n_turn = simulate_turn_step( wm, ball_pos, control_area, 0.0, 100, false, &dash_angle );
    double move_dist = self_pos.dist( ball_pos ) - control_area - 0.15;
    int n_dash = ptype.cyclesToReachDistance( move_dist );

    if ( max_step > n_turn + n_dash )
    {
        n_dash = max_step - n_turn;
    }

    StaminaModel stamina_model = wm.self().staminaModel();

    stamina_model.simulateWaits( ptype, n_turn );
    stamina_model.simulateDashes( ptype, n_dash, ServerParam::i().maxDashPower() );

    self_cache.emplace_back( Intercept::NORMAL,
                             Intercept::TURN_FORWARD_DASH,
                             n_turn, ( dash_angle - wm.self().body() ).degree(),
                             n_dash, ServerParam::i().maxDashPower(), 0.0,
                             ball_pos,
                             0.0,
                             stamina_model.stamina() );
}

}
