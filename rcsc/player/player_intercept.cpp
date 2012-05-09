// -*-c++-*-

/*!
  \file player_intercept.cpp
  \brief intercept predictor for other players Source File
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

#include "player_intercept.h"
#include "world_model.h"
#include "ball_object.h"
#include "player_object.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/soccer_math.h>

// #define DEBUG
// #define DEBUG2

namespace rcsc {

namespace {

/*-------------------------------------------------------------------*/
inline
Vector2D
get_pos( const PlayerObject & p )
{
    return ( p.heardPosCount() < p.seenPosCount()
             ? p.heardPos()
             : p.seenPos() );
}

/*-------------------------------------------------------------------*/
inline
Vector2D
get_vel( const PlayerObject & p )
{
    return ( p.velCount() < p.seenVelCount()
             ? p.vel()
             : p.seenVel() );
}

/*-------------------------------------------------------------------*/
inline
double
get_control_area( const PlayerObject & p,
                  const WorldModel & wm,
                  const bool goalie )
{
    if ( p.side() == wm.ourSide() )
    {
        return ( goalie
                 ? p.playerTypePtr()->reliableCatchableDist() - 0.2
                 : p.playerTypePtr()->kickableArea() - 0.2 );
    }

    return ( goalie
             ? p.playerTypePtr()->reliableCatchableDist()
             : p.playerTypePtr()->kickableArea() );
}

/*-------------------------------------------------------------------*/
inline
int
get_bonus_step( const PlayerObject & p,
                const SideID our_side )
{
    return p.side() == our_side
        // ? std::min( 3, std::min( p.heardPosCount(), p.seenPosCount() ) )
        // : std::min( 6, std::min( p.heardPosCount(), p.seenPosCount() ) );
        ? std::min( 3, std::min( p.heardPosCount(), p.seenPosCount() ) )
        : std::min( 3, std::min( p.heardPosCount(), p.seenPosCount() ) );
}

/*-------------------------------------------------------------------*/
inline
int
get_penalty_step( const PlayerObject & p )
{
    return ( p.isTackling()
             ? std::max( 0, ServerParam::i().tackleCycles() - p.tackleCount() - 2 )
             : 0 );
}

}

/*-------------------------------------------------------------------*/
/*!

*/
inline
Vector2D
PlayerIntercept::PlayerData::inertiaPoint( const int step ) const
{
    return ptype_.inertiaPoint( pos_, vel_, step + bonus_step_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerIntercept::PlayerIntercept( const WorldModel & world,
                                  const std::vector< Vector2D > & ball_cache )
    : M_world( world ),
      M_ball_cache( ball_cache ),
      M_ball_move_angle( ( ball_cache.back() - ball_cache.front() ).th() )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::predict( const PlayerObject & player,
                          const bool goalie ) const
{
    const PlayerType * ptype = player.playerTypePtr();

    if ( ! ptype )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": ERROR NULL player type." << std::endl;
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": NULL player type. side=%c unum=%d",
                      side_char( player.side() ), player.unum() );
        return 1000;
    }

    const ServerParam & SP = ServerParam::i();

    const double pen_area_x = SP.pitchHalfLength() - SP.penaltyAreaLength();
    const double pen_area_y = SP.penaltyAreaHalfWidth();

    const PlayerData data( player,
                           *ptype,
                           get_pos( player ),
                           get_vel( player ),
                           get_control_area( player, M_world, goalie ),
                           get_bonus_step( player, M_world.ourSide() ),
                           get_penalty_step( player ) );

    const int min_step = estimateMinStep( data );
    const int max_step = M_ball_cache.size() - 1;

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "Intercept Player %c %d (%.1f %.1f) - min=%d max=%d pos=(%.1f %.1f) bonus=%d penalty=%d",
                  side_char( player.side() ),
                  player.unum(),
                  player.pos().x, player.pos().y,
                  min_step, max_step,
                  data.pos_.x, data.pos_.y,
                  data.bonus_step_, data.penalty_step_ );
#endif

    if ( min_step > max_step )
    {
        return predictFinal( data );
    }

    for ( int total_step = min_step; total_step < max_step; ++total_step )
    {
        const Vector2D & ball_pos = M_ball_cache[total_step];
#ifdef DEBUG2
        dlog.addText( Logger::INTERCEPT,
                      "*** step=%d  ball(%.2f %.2f)",
                      total_step, ball_pos.x, ball_pos.y );
#endif

        if ( goalie
             && ( ball_pos.absX() < pen_area_x
                  || pen_area_y < ball_pos.absY() ) )
        {
           // never reach
#ifdef DEBUG2
            dlog.addText( Logger::INTERCEPT,
                          "--->cycle=%d goalie. out of penalty area. ball(%.2f %.2f)",
                          total_step, ball_pos.x, ball_pos.y );
#endif
            continue;
        }

        if ( std::pow( data.control_area_
                       + data.ptype_.realSpeedMax() * ( total_step + data.bonus_step_ - data.penalty_step_ )
                       + 0.5, 2 )
             < data.pos_.dist2( ball_pos ) )
        {
            // never reach
#ifdef DEBUG2
            dlog.addText( Logger::INTERCEPT,
                          "--->step=%d  never reach! ball(%.2f %.2f) dist=%.3f",
                          total_step, ball_pos.x, ball_pos.y,
                          data.pos_.dist( ball_pos ) );
#endif
            continue;
        }

        if ( canReachAfterTurnDash( data,
                                    ball_pos,
                                    total_step ) )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "--->cycle=%d  Sucess! ball(%.2f %.2f)",
                          total_step, ball_pos.x, ball_pos.y );
#endif
            return total_step;
        }
    }

    if ( goalie
         && ( M_ball_cache.back().absX() < pen_area_x
              || pen_area_y < M_ball_cache.back().absY() ) )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "FAILURE goalie. final. over the penalty area. bpos=(%.2f %.2f)",
                      M_ball_cache.back().x, M_ball_cache.back().y );
#endif
        return 1000;
    }

    return predictFinal( data );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::estimateMinStep( const PlayerData & data ) const
{
    Vector2D rel = data.pos_ - M_ball_cache.front();
    rel.rotate( - M_ball_move_angle );

    double move_dist = std::max( 0.3, rel.absY() - data.control_area_ );
    int step = static_cast< int >( std::floor( move_dist / data.ptype_.realSpeedMax() ) );
    return std::max( 0, step - data.bonus_step_ + data.penalty_step_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerIntercept::canReachAfterTurnDash( const PlayerData & data,
                                        const Vector2D & ball_pos,
                                        const int total_step ) const
{
    /*
      TODO
      if ( canReachAfterOmniDash() )
      {
          return true;
      }
     */

    int n_turn = predictTurnCycle( data, ball_pos, total_step );
#ifdef DEBUG2
    dlog.addText( Logger::INTERCEPT,
                  "______ step %d  turn=%d",
                  total_step, n_turn );
#endif

    int max_dash = total_step - n_turn - data.penalty_step_;
    if ( max_dash < 0 )
    {
        return false;
    }

    return canReachAfterDash( data,
                              ball_pos,
                              total_step,
                              n_turn );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::predictTurnCycle( const PlayerData & data,
                                   const Vector2D & ball_pos,
                                   const int total_step ) const
{
    Vector2D inertia_pos = data.inertiaPoint( total_step );
    Vector2D ball_rel = ball_pos - inertia_pos;
    double ball_dist = ball_rel.r();

    double angle_diff = ( ball_rel.th() - data.player_.body() ).abs();

    double turn_margin = 180.0;
    if ( data.control_area_ < ball_dist )
    {
        turn_margin = std::max( 15.0, AngleDeg::asin_deg( data.control_area_ / ball_dist ) );
    }

    if ( ball_dist < 10.0 // XXX magic number XXX
         && angle_diff > 90.0 )
    {
        // assume back dash
        angle_diff = 180.0 - angle_diff;
    }

    int n_turn = 0;

    if ( angle_diff > turn_margin )
    {
        double speed = data.player_.vel().r();

        speed *= std::pow( data.ptype_.playerDecay(), data.penalty_step_ );

        do
        {
            double max_turn = data.ptype_.effectiveTurn( ServerParam::i().maxMoment(), speed );
            angle_diff -= max_turn;
            speed *= data.ptype_.playerDecay();
            ++n_turn;
        }
        while ( angle_diff > turn_margin );
    }

    return n_turn;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerIntercept::canReachAfterDash( const PlayerData & data,
                                    const Vector2D & ball_pos,
                                    const int total_step,
                                    const int n_turn ) const
{
    Vector2D inertia_pos = data.inertiaPoint( total_step );
    Vector2D ball_rel = ball_pos - inertia_pos;

    double dash_dist = ball_rel.r() - data.control_area_;

    if ( dash_dist < 0.0
         && total_step > data.penalty_step_ )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "______ %d (%.1f %.1f) can reach(1). inertia. total:%d turn:%d dash:0",
                      data.player_.unum(),
                      data.player_.pos().x, data.player_.pos().y,
                      total_step,
                      n_turn );
#endif
        return true;
    }

    int n_dash = data.ptype_.cyclesToReachDistance( dash_dist );
    int bonus_step = std::max( 0, data.bonus_step_ - n_turn );

    if ( n_turn + n_dash - bonus_step + data.penalty_step_ <= total_step )
    {
#ifdef DEBUG
        dlog.addText( Logger::INTERCEPT,
                      "______ %d (%.1f %.1f) can reach(2). total:%d(>=t:%d+d:%d-b:%d+p:%d) dist=%.2f",
                      data.player_.unum(),
                      data.player_.pos().x, data.player_.pos().y,
                      total_step,
                      n_turn, n_dash, bonus_step, data.penalty_step_,
                      dash_dist );
#endif
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerIntercept::predictFinal( const PlayerData & data ) const
{
    Vector2D ball_pos = M_ball_cache.back();
    int ball_step = M_ball_cache.size() - 1;

    Vector2D inertia_pos = data.inertiaPoint( 100 );

    int n_turn = predictTurnCycle( data, ball_pos, 100 );

    double dash_dist = inertia_pos.dist( ball_pos ) - data.control_area_;

    if ( dash_dist < 0.0
         && ball_step > data.penalty_step_ )
    {
        return ball_step;
    }

    int n_dash = data.ptype_.cyclesToReachDistance( dash_dist );
    int bonus_step = std::max( 0, data.bonus_step_ - n_turn );

    int step = std::max( ball_step, n_turn + n_dash - bonus_step + data.penalty_step_ );
#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "____No Solution. final point(%.2f %.2f)"
                  " step:%d(t:%d+d:%d-b:%d+p:%d dist=%.2f",
                  ball_pos.x, ball_pos.y,
                  step,
                  n_turn, n_dash, bonus_step, data.penalty_step_,
                  dash_dist );
#endif
    return step;
}

}
