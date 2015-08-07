// -*-c++-*-

/*!
  \file intercept_table.cpp
  \brief interception info holder Source File
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

#include "intercept_table.h"
#include "self_intercept_simulator.h"
#include "player_intercept.h"
#include "world_model.h"
#include "player_object.h"
#include "abstract_player_object.h"

#include <rcsc/time/timer.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_time.h>

#include <algorithm>

// #define DEBUG_PRINT

namespace rcsc {

namespace {
const int MAX_STEP = 50;
}

/*-------------------------------------------------------------------*/
/*!

*/
InterceptTable::InterceptTable( const WorldModel & world )
    : M_world( world ),
      M_update_time( 0, 0 )
{
    M_ball_cache.reserve( MAX_STEP );
    M_self_cache.reserve( ( MAX_STEP + 1 ) * 2 );

    clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::clear()
{
    M_ball_cache.clear();

    M_self_reach_step = 1000;
    M_self_exhaust_reach_step = 1000;
    M_teammate_reach_step = 1000;
    M_second_teammate_reach_step = 1000;
    M_goalie_reach_step = 1000;
    M_opponent_reach_step = 1000;
    M_second_opponent_reach_step = 1000;

    M_fastest_teammate = static_cast< PlayerObject * >( 0 );
    M_second_teammate = static_cast< PlayerObject * >( 0 );
    M_fastest_opponent = static_cast< PlayerObject * >( 0 );
    M_second_opponent = static_cast< PlayerObject * >( 0 );

    M_self_cache.clear();

    M_player_map.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::update()
{
    if ( M_world.time() == M_update_time )
    {
        return;
    }
    M_update_time = M_world.time();

#ifdef DEBUG_PRINT
    dlog.addText( Logger::INTERCEPT,
                  __FILE__" (update)" );
    MSecTimer timer;
#endif

    // clear all data
    this->clear();

    // playmode check
    if ( M_world.gameMode().type() == GameMode::TimeOver
         || M_world.gameMode().type() == GameMode::BeforeKickOff )
    {
        return;
    }

    if ( ! M_world.self().posValid()
         || ! M_world.ball().posValid() )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__" (update) Invalid self or ball pos" );
        return;
    }

#ifdef DEBUG
    if ( M_world.self().isKickable()
         || M_world.kickableTeammate()
         || M_world.kickableOpponent() )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__" (update) Already exist kickable player" );
    }
#endif

    createBallCache();

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "==========Intercept Predict Self==========" );
#endif

    predictSelf();

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "==========Intercept Predict Opponent==========" );
#endif

    predictOpponent();

#ifdef DEBUG
    dlog.addText( Logger::INTERCEPT,
                  "==========Intercept Predict Teammate==========" );
#endif

    predictTeammate();

    dlog.addText( Logger::INTERCEPT,
                  "<-----Intercept Self reach step = %d. exhaust reach step = %d ",
                  M_self_reach_step,
                  M_self_exhaust_reach_step );
    if ( M_fastest_teammate )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<-----Intercept Teammate  fastest reach step = %d."
                      " teammate %d (%.1f %.1f)",
                      M_teammate_reach_step,
                      M_fastest_teammate->unum(),
                      M_fastest_teammate->pos().x,
                      M_fastest_teammate->pos().y );

    }

    if ( M_second_teammate )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<-----Intercept Teammate  2nd     reach step = %d."
                      " teammate %d (%.1f %.1f)",
                      M_second_teammate_reach_step,
                      M_second_teammate->unum(),
                      M_second_teammate->pos().x,
                      M_second_teammate->pos().y );
    }

    if ( M_fastest_opponent )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<-----Intercept Opponent  fastest reach step = %d."
                      " opponent %d (%.1f %.1f)",
                      M_opponent_reach_step,
                      M_fastest_opponent->unum(),
                      M_fastest_opponent->pos().x,
                      M_fastest_opponent->pos().y );
    }

    if ( M_second_opponent )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<-----Intercept Opponent  2nd     reach step = %d."
                      " opponent %d (%.1f %.1f)",
                      M_second_opponent_reach_step,
                      M_second_opponent->unum(),
                      M_second_opponent->pos().x,
                      M_second_opponent->pos().y );
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::INTERCEPT,
                  __FILE__":(update) elapsed %.3f [ms]", timer.elapsedReal() );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::hearTeammate( const int unum,
                              const int step )
{
    if ( M_fastest_teammate
         && step >= M_teammate_reach_step )
    {
        return;
    }

    const PlayerObject * p = static_cast< const PlayerObject * >( 0 );

    for ( PlayerObject::Cont::const_iterator it = M_world.teammates().begin(), end = M_world.teammates().end();
          it != end;
          ++it )
    {
        if ( (*it)->unum() == unum )
        {
            p = *it;
            break;
        }
    }

    if ( p )
    {
        M_fastest_teammate = p;
        M_teammate_reach_step = step;

        M_player_map[ p ] = step;

        dlog.addText( Logger::INTERCEPT,
                      "<----- Hear Intercept Teammate  fastest reach step = %d."
                      " teammate %d (%.1f %.1f)",
                      M_teammate_reach_step,
                      M_fastest_teammate->unum(),
                      M_fastest_teammate->pos().x,
                      M_fastest_teammate->pos().y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::hearOpponent( const int unum,
                              const int step )
{
    if ( M_fastest_opponent )
    {
        if ( step >= M_opponent_reach_step )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<----- Hear Intercept Opponent. no update."
                          " exist faster reach step %d >= %d",
                          step, M_opponent_reach_step );
            return;
        }

        if ( M_fastest_opponent->unum() == unum
             && M_fastest_opponent->posCount() == 0 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<----- Hear Intercept Opponent . no update."
                          " opponent %d (%.1f %.1f) is seen",
                          M_fastest_opponent->unum(),
                          M_fastest_opponent->pos().x,
                          M_fastest_opponent->pos().y );
            return;
        }
    }

    const PlayerObject * p = static_cast< const PlayerObject * >( 0 );

    for ( PlayerObject::Cont::const_iterator it = M_world.opponents().begin(), end = M_world.opponents().end();
          it != end;
          ++it )
    {
        if ( (*it)->unum() == unum )
        {
            p = *it;
            break;
        }
    }


    if ( p )
    {
        M_fastest_opponent = p;
        M_opponent_reach_step = step;

        M_player_map[ p ] = step;

        dlog.addText( Logger::INTERCEPT,
                      "<----- Hear Intercept Opponent  fastest reach step = %d."
                      " opponent %d (%.1f %.1f)",
                      M_opponent_reach_step,
                      M_fastest_opponent->unum(),
                      M_fastest_opponent->pos().x,
                      M_fastest_opponent->pos().y );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::createBallCache()
{
    const ServerParam & SP = ServerParam::i();
    const double max_x = ( SP.keepawayMode()
                           ? SP.keepawayLength() * 0.5
                           : SP.pitchHalfLength() + 5.0 );
    const double max_y = ( SP.keepawayMode()
                           ? SP.keepawayWidth() * 0.5
                           : SP.pitchHalfWidth() + 5.0 );
    const double bdecay = SP.ballDecay();

    Vector2D bpos = M_world.ball().pos();
    Vector2D bvel = M_world.ball().vel();
    double bspeed = bvel.r();

    for ( int i = 0; i < MAX_STEP; ++i )
    {
        M_ball_cache.push_back( bpos );

        if ( bspeed < 0.005 && i >= 10 )
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
                  "(InterceptTable::createBallCache) size=%d last pos=(%.2f %.2f)",
                  M_ball_cache.size(),
                  M_ball_cache.back().x, M_ball_cache.back().y );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::predictSelf()
{
    if ( M_world.self().isKickable() )
    {
        dlog.addText( Logger::INTERCEPT,
                      "Intercept Self. already kickable. no estimation loop!" );
        M_self_reach_step = 0;
        M_self_exhaust_reach_step = 0;
        return;
    }

    int max_step = std::min( MAX_STEP, static_cast< int >( M_ball_cache.size() ) );

    // SelfInterceptV13 predictor( M_world );
    // predictor.predict( max_step, M_self_cache );
    SelfInterceptSimulator sim;
    sim.simulate( M_world, max_step, M_self_cache );

    if ( M_self_cache.empty() )
    {
        std::cerr << M_world.self().unum() << ' '
                  << M_world.time()
                  << " Interecet Self cache is empty!"
                  << std::endl;
        dlog.addText( Logger::INTERCEPT,
                      "Intercept Self. Self cache is empty!" );
        // if self cache is empty,
        // reach point should be the inertia final point of the ball
        return;
    }

// #ifdef SELF_INTERCEPT_USE_NO_SAVE_RECEVERY
//     std::sort( M_self_cache.begin(),
//                M_self_cache.end(),
//                InterceptInfo::Cmp() );
//     M_self_cache.erase( std::unique( M_self_cache.begin(),
//                                      M_self_cache.end(),
//                                      InterceptInfo::Equal() ),
//                         M_self_cache.end() );
// #endif

    int min_step = M_self_reach_step;
    int exhaust_min_step = M_self_exhaust_reach_step;

    for ( std::vector< InterceptInfo >::iterator it = M_self_cache.begin(),
              end = M_self_cache.end();
          it != end;
          ++it )
    {
        if ( it->staminaType() == InterceptInfo::NORMAL )
        {
            if ( it->reachStep() < min_step )
            {
                min_step = it->reachStep();
            }
        }
        else if ( it->staminaType() == InterceptInfo::EXHAUST )
        {
            if ( it->reachStep() < exhaust_min_step )
            {
                exhaust_min_step = it->reachStep();
            }
        }
    }

    dlog.addText( Logger::INTERCEPT,
                  "Intercept Self. solution size = %d",
                  M_self_cache.size() );

    M_self_reach_step = min_step;
    M_self_exhaust_reach_step = exhaust_min_step;

    //M_player_map.insert( std::pair< const AbstractPlayerObject *, int >( &(M_world.self()), min_step ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::predictTeammate()
{
    int min_step = 1000;
    int second_min_step = 1000;

    if ( M_world.kickableTeammate() )
    {
        M_teammate_reach_step = 0;
        min_step = 0;
        M_fastest_teammate = M_world.kickableTeammate();

        dlog.addText( Logger::INTERCEPT,
                      "Intercept Teammate. exist kickable teammate" );
        dlog.addText( Logger::INTERCEPT,
                      "---> set fastest teammate %d (%.1f %.1f)",
                      M_fastest_teammate->unum(),
                      M_fastest_teammate->pos().x, M_fastest_teammate->pos().y );
    }

    PlayerIntercept predictor( M_world, M_ball_cache );

    for ( PlayerObject::Cont::const_iterator it = M_world.teammatesFromBall().begin(),
              end = M_world.teammatesFromBall().end();
          it != end;
          ++it )
    {
        if ( *it == M_world.kickableTeammate() )
        {
            M_player_map[ *it ] = 0;
            continue;
        }

        if ( (*it)->posCount() >= 10 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "Intercept Teammate %d.(%.1f %.1f) Low accuracy %d. skip...",
                          (*it)->unum(),
                          (*it)->pos().x, (*it)->pos().y,
                          (*it)->posCount() );
            continue;
        }

        int step = predictor.predict( **it, false );
        int goalie_step = 1000;
        if ( (*it)->goalie() )
        {
            goalie_step = predictor.predict( **it, true );
            if ( step > goalie_step )
            {
                step = goalie_step;
            }
        }

        dlog.addText( Logger::INTERCEPT,
                      "---> Teammate %d.(%.1f %.1f) step=%d",
                      (*it)->unum(),
                      (*it)->pos().x, (*it)->pos().y,
                      step );

        if ( (*it)->goalie() )
        {
            M_goalie_reach_step = goalie_step;
        }

        if ( step < second_min_step )
        {
            second_min_step = step;
            M_second_teammate = *it;

            if ( second_min_step < min_step )
            {
                std::swap( min_step, second_min_step );
                std::swap( M_fastest_teammate, M_second_teammate );
            }
        }

        M_player_map[ *it ] = step;
    }

    if ( M_second_teammate && second_min_step < 1000 )
    {
        M_second_teammate_reach_step = second_min_step;
    }

    if ( M_fastest_teammate && min_step < 1000 )
    {
        M_teammate_reach_step = min_step;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptTable::predictOpponent()
{
    int min_step = 1000;
    int second_min_step = 1000;

    if ( M_world.kickableOpponent() )
    {
        M_opponent_reach_step = 0;
        min_step = 0;
        M_fastest_opponent = M_world.kickableOpponent();

        dlog.addText( Logger::INTERCEPT,
                      "Intercept Opponent. exist kickable opponent" );
        dlog.addText( Logger::INTERCEPT,
                      "---> set fastest opponent %d (%.1f %.1f)",
                      M_fastest_opponent->unum(),
                      M_fastest_opponent->pos().x, M_fastest_opponent->pos().y );
    }

    PlayerIntercept predictor( M_world, M_ball_cache );

    for ( PlayerObject::Cont::const_iterator it = M_world.opponentsFromBall().begin(),
              end = M_world.opponentsFromBall().end();
          it != end;
          ++it )
    {
        if ( *it == M_world.kickableOpponent() )
        {
            M_player_map[ *it ] = 0;
            continue;
        }

        if ( (*it)->posCount() >= 15 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "Intercept Opponent %d.(%.1f %.1f) Low accuracy %d. skip...",
                          (*it)->unum(),
                          (*it)->pos().x, (*it)->pos().y,
                          (*it)->posCount() );
            continue;
        }

        int step = predictor.predict( **it, false );
        if ( (*it)->goalie() )
        {
            int goalie_step = predictor.predict( **it, true );
            if ( goalie_step > 0
                 && step > goalie_step )
            {
                step = goalie_step;
            }
        }

        dlog.addText( Logger::INTERCEPT,
                      "---> Opponent.%d (%.1f %.1f) step=%d",
                      (*it)->unum(),
                      (*it)->pos().x, (*it)->pos().y,
                      step );

        if ( step < second_min_step )
        {
            second_min_step = step;
            M_second_opponent = *it;

            if ( second_min_step < min_step )
            {
                std::swap( min_step, second_min_step );
                std::swap( M_fastest_opponent, M_second_opponent );
            }
        }

        M_player_map[ *it ] = step;
    }

    if ( M_second_opponent && second_min_step < 1000 )
    {
        M_second_opponent_reach_step = second_min_step;
    }

    if ( M_fastest_opponent && min_step < 1000 )
    {
        M_opponent_reach_step = min_step;
    }
}

}
