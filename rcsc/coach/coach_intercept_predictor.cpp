// -*-c++-*-

/*!
  \file coach_intercept_predictor.cpp
  \brief intercept cycle predictor class Source File
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

#include "coach_intercept_predictor.h"

#include "coach_ball_object.h"
#include "coach_player_object.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/logger.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CoachInterceptPredictor::CoachInterceptPredictor( const CoachBallObject & ball )
{
    const ServerParam & SP = ServerParam::i();
    const double max_x = ( SP.keepawayMode()
                           ? SP.keepawayLength() * 0.5
                           : SP.pitchHalfLength() + 5.0 );
    const double max_y = ( SP.keepawayMode()
                           ? SP.keepawayWidth() * 0.5
                           : SP.pitchHalfWidth() + 5.0 );
    const double bdecay = SP.ballDecay();

    Vector2D bpos = ball.pos();
    Vector2D bvel = ball.vel();
    double bspeed = bvel.r();

    for ( int i = 0; i < 100; ++i )
    {
        M_ball_cache.push_back( bpos );

        if ( bspeed < 0.005 )
        {
            break;
        }

        bpos += bvel;
        bvel *= bdecay;
        bspeed *= bdecay;

        if ( max_x < bpos.absX()
             || max_y < bpos.absY() )
        {
            break;
        }
    }
#ifdef DEBUG_PRINT
    dlog.addText( Logger::INTERCEPT,
                  "CoachInterceptPredict ball cache size=%d last pos=(%.2f %.2f)",
                  M_ball_cache.size(),
                  M_ball_cache.back().x, M_ball_cache.back().y );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
int
CoachInterceptPredictor::predict( const CoachPlayerObject & player ) const
{
    if ( ! player.isValid() )
    {
        return -1;
    }

    int step = predictReachStep( player, false );

    if ( player.goalie() )
    {
        int goalie_step = predictReachStep( player, true );
        step = std::min( step, goalie_step );
    }

    return step;

}

/*-------------------------------------------------------------------*/
/*!

*/
int
CoachInterceptPredictor::predictReachStep( const CoachPlayerObject & player,
                                           const bool goalie ) const
{
    const ServerParam & SP = ServerParam::i();
    const PlayerType * ptype = ( player.playerTypePtr()
                                 ? player.playerTypePtr()
                                 : PlayerTypeSet::i().get( Hetero_Default ) );
    const double pen_area_x = SP.pitchHalfLength() - SP.pitchHalfLength();
    const double pen_area_y = SP.penaltyAreaHalfWidth();

    const double control_area = ( goalie
                                  ? ServerParam::i().catchableArea()
                                  : ptype->kickableArea() );
    const int penalty_step = ( player.isTackling()
                               ? std::max( 0, SP.tackleCycles() - player.tackleCycle() )
                               : player.isCharged()
                               ? std::max( 0, SP.foulCycles() - player.chargedCycle() )
                               : 0 );

    const int min_step = predictMinStep( player, *ptype, control_area );
    const int max_step = M_ball_cache.size() - 1;

    //
    // cycle loop
    //
    for ( int total_step = min_step; total_step < max_step; ++total_step )
    {
        const Vector2D & ball_pos = M_ball_cache[total_step];

        if ( control_area + ptype->realSpeedMax() * ( total_step - penalty_step )
             < player.pos().dist( ball_pos ) )
        {
            continue;
        }

        if ( goalie
             && ( pen_area_y < ball_pos.absY()
                  || ball_pos.absX() < pen_area_x ) )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::INTERCEPT,
                          "FAILURE goalie. over the bounding area. cycle=%d bpos=(%.2f %.2f)",
                          total_step,
                          ball_pos.x, ball_pos.y );
#endif
            continue;
        }

        if ( canReachAfterTurnDash( total_step,
                                    penalty_step,
                                    player, *ptype, control_area,
                                    ball_pos ) )
        {
            return total_step;
        }

    }

    if ( goalie
         && ( pen_area_y < M_ball_cache.back().absY()
              || M_ball_cache.back().absX() < pen_area_x ) )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::INTERCEPT,
                      "FAILURE goalie. over the bounding area. final. bpos=(%.2f %.2f)",
                      M_ball_cache.back().x, M_ball_cache.back().y );
#endif
        return -1;
    }

    return predictFinal( penalty_step, player, *ptype, control_area );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
CoachInterceptPredictor::predictMinStep( const CoachPlayerObject & player,
                                         const PlayerType & ptype,
                                         const double control_area ) const
{
    Vector2D rel = player.pos() - M_ball_cache.front();
    AngleDeg move_angle = ( M_ball_cache.back() - M_ball_cache.front() ).th();
    rel.rotate( -move_angle );

    double move_dist = rel.absY() - control_area;
    return static_cast< int >( std::floor( move_dist / ptype.realSpeedMax() ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachInterceptPredictor::canReachAfterTurnDash( const int total_step,
                                                const int penalty_step,
                                                const CoachPlayerObject & player,
                                                const PlayerType & ptype,
                                                const double control_area,
                                                const Vector2D & ball_pos ) const
{
    /*
      TODO:

      if ( canReachAfterShortOmniDash() )
      {
          return true;
      }
    */


    int n_turn = predictTurnCycle( total_step,
                                   penalty_step,
                                   player, ptype, control_area,
                                   ball_pos );

    int max_dash = total_step - n_turn - penalty_step;
    if ( max_dash < 0 )
    {
        return false;
    }

    return canReachAfterDash( penalty_step, n_turn, max_dash,
                              player, ptype, control_area,
                              ball_pos );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
CoachInterceptPredictor::predictTurnCycle( const int total_step,
                                           const int penalty_step,
                                           const CoachPlayerObject & player,
                                           const PlayerType & ptype,
                                           const double control_area,
                                           const Vector2D & ball_pos ) const
{
    const Vector2D inertia_pos = ptype.inertiaPoint( player.pos(), player.vel(), total_step );
    const Vector2D target_rel = ball_pos - inertia_pos;
    const double target_dist = target_rel.r();

    double angle_diff = ( target_rel.th() - player.body() ).abs();

    double turn_margin = 180.0;
    if ( control_area < target_dist )
    {
        turn_margin = std::max( 15.0,
                                AngleDeg::asin_deg( control_area / target_dist ) );
    }

    if ( target_dist < 10.0
         && angle_diff > 90.0 )
    {
        angle_diff = 180.0 - angle_diff; // backward dash
    }

    int n_turn = 0;

    if ( angle_diff > turn_margin )
    {
        double speed = player.vel().r();

        // tackle/charge steps
        speed *= std::pow( ptype.playerDecay(), penalty_step );

        while ( angle_diff > turn_margin )
        {
            double max_turn = ptype.effectiveTurn( ServerParam::i().maxMoment(), speed );
            angle_diff -= max_turn;
            speed *= ptype.playerDecay();
            ++n_turn;
        }
    }

    return n_turn;
}


/*-------------------------------------------------------------------*/
/*!

 */
bool
CoachInterceptPredictor::canReachAfterDash( const int penalty_step,
                                            const int n_turn,
                                            const int n_dash,
                                            const CoachPlayerObject & player,
                                            const PlayerType & ptype,
                                            const double control_area,
                                            const Vector2D & ball_pos ) const
{
    Vector2D inertia_pos = ptype.inertiaPoint( player.pos(),
                                               player.vel(),
                                               penalty_step + n_turn + n_dash );
    double dash_dist = inertia_pos.dist( ball_pos ) - control_area;

    if ( dash_dist < 0.0 )
    {
        // already kickable/catchable
#ifdef DEBUG_PRINT
        dlog.addText( Logger::INTERCEPT,
                      "SUCCESS bpos=(%.2f %.2f) step=%d. kickable after inertia move.",
                      ball_pos.x, ball_pos.y,
                      penalty_step + n_turn + n_dash );
#endif
        return true;
    }

    int dash_step = ptype.cyclesToReachDistance( dash_dist );

    if ( dash_step <= n_dash )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::INTERCEPT,
                      "SUCCESS bpos=(%.2f %.2f) step=%d (penalty=%d turn=%d dash=%d max_dash=%d)",
                      ball_pos.x, ball_pos.y,
                      penalty_step + n_turn + dash_step,
                      penalty_step, n_turn, dash_step, n_dash );
#endif
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
CoachInterceptPredictor::predictFinal( const int penalty_step,
                                       const CoachPlayerObject & player,
                                       const PlayerType & ptype,
                                       const double control_area ) const
{

    Vector2D inertia_pos = ptype.inertiaPoint( player.pos(),
                                               player.vel(),
                                               100 );
    double dash_dist = inertia_pos.dist( M_ball_cache.back() ) - control_area;

    int n_turn = predictTurnCycle( 100,
                                   penalty_step,
                                   player,
                                   ptype,
                                   control_area,
                                   M_ball_cache.back() );

    int n_dash = ptype.cyclesToReachDistance( dash_dist );

    int final_step = penalty_step + n_turn + n_dash;
#ifdef DEBUG_PRINT
    dlog.addText( Logger::INTERCEPT,
                  "SUCCESS final. bpos=(%.2f %.2f) step=%d (penalty=%d turn=%d dash=%d)",
                  M_ball_cache.back().x, M_ball_cache.back().y,
                  final_step, penalty_step, n_turn, n_dash );
#endif
    return final_step;
}

}
