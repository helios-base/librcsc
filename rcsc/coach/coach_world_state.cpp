// -*-c++-*-

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

#include "coach_world_state.h"

#include "coach_intercept_predictor.h"
#include "coach_visual_sensor.h"

#include <rcsc/common/audio_memory.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/time/timer.h>
#include <rcsc/rcg/types.h>

// #define DEBUG_PRINT

namespace rcsc {

namespace {

struct BallReachStepSorter {

    bool operator()( const CoachPlayerObject * lhs,
                     const CoachPlayerObject * rhs ) const
      {
          return lhs->ballReachStep() < rhs->ballReachStep();
      }
};

}


/*-------------------------------------------------------------------*/
/*!

 */
CoachWorldState::CoachWorldState()
    : M_our_side( NEUTRAL ),
      M_time( -1, 0 ),
      M_game_mode(),
      M_our_offside_line_x( 0.0 ),
      M_their_offside_line_x( 0.0 ),
      M_kicker( nullptr ),
      M_ball_owner_side( NEUTRAL ),
      M_ball_owner( nullptr ),
      M_fastest_intercept_player( nullptr ),
      M_fastest_intercept_teammate( nullptr ),
      M_fastest_intercept_opponent( nullptr )
{
    M_all_players.reserve( 22 );
    M_teammates.reserve( 11 );
    M_opponents.reserve( 11 );
    std::fill( M_teammate_array, M_teammate_array + 11, nullptr );
    std::fill( M_opponent_array, M_opponent_array + 11, nullptr );
}

/*-------------------------------------------------------------------*/
/*!

 */
CoachWorldState::CoachWorldState( const CoachVisualSensor & see_global,
                                  const SideID our_side,
                                  const GameTime & current_time,
                                  const GameMode & current_mode,
                                  const CoachWorldState::Ptr & prev_state )
    : M_our_side( our_side ),
      M_time( current_time ),
      M_game_mode( current_mode ),
      M_our_offside_line_x( 0.0 ),
      M_their_offside_line_x( 0.0 ),
      M_kicker( nullptr ),
      M_ball_owner_side( NEUTRAL ),
      M_ball_owner( nullptr ),
      M_fastest_intercept_player( nullptr ),
      M_fastest_intercept_teammate( nullptr ),
      M_fastest_intercept_opponent( nullptr )
{
    M_all_players.reserve( 22 );
    M_teammates.reserve( 11 );
    M_opponents.reserve( 11 );
    std::fill( M_teammate_array, M_teammate_array + 11, nullptr );
    std::fill( M_opponent_array, M_opponent_array + 11, nullptr );

    //
    // ball
    //
    M_ball = see_global.ball();

    //
    // players
    //
    for ( const CoachPlayerObject & vp : see_global.players() )
    {
        CoachPlayerObject * p = nullptr;

        if ( prev_state )
        {
            const CoachPlayerObject * pp = prev_state->getPlayer( vp.side(), vp.unum() );

            if ( pp )
            {
                p = pp->clone();
                p->update( vp );
            }
        }

        if ( ! p )
        {
            p = new CoachPlayerObject();
            p->update( vp );
        }

        if ( p )
        {
            M_all_players.push_back( p );

            if ( p->unum() < 1 || 11 < p->unum() )
            {
                std::cerr << __FILE__ << ' ' << __LINE__
                          << ": illegal uniform number: "
                          << p->side() << ' ' << p->unum() << std::endl;
                dlog.addText( Logger::WORLD,
                              __FILE__":(CoachWorldState) illegal unum (%c %d) type=%d",
                              side_char( p->side() ), p->unum(), p->type() );
                continue;
            }

            if ( p->side() == NEUTRAL )
            {
                std::cerr << __FILE__ << ' ' << __LINE__
                          << ": illegal team side id: "
                          << p->side() << ' ' << p->unum() << std::endl;
                dlog.addText( Logger::WORLD,
                              __FILE__":(CoachWorldState) illegal side (%c %d) type=%d",
                              side_char( p->side() ), p->unum(), p->type() );
                continue;
            }

            if ( our_side == NEUTRAL ) // trainer or analyzer
            {
                if ( p->side() == LEFT )
                {
                    M_teammates.push_back( p );
                    M_teammate_array[p->unum() - 1] = p;
                }
                else
                {
                    M_opponents.push_back( p );
                    M_opponent_array[p->unum() - 1] = p;
                }
            }
            else // coach
            {
                if ( p->side() != our_side )
                {
                    M_opponents.push_back( p );
                    M_opponent_array[p->unum() - 1] = p;
                }
                else
                {
                    M_teammates.push_back( p );
                    M_teammate_array[p->unum() - 1] = p;
                }
            }
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__":(CoachWorldState) created player (%c %d) type=%d",
                          side_char( p->side() ), p->unum(), p->type() );
#endif
        }
    }

    if ( our_side == RIGHT )
    {
        M_ball.reverseSide();
        for ( int i = 0; i < 11; ++i )
        {
            if ( M_teammate_array[i] ) M_teammate_array[i]->reverseSide();
            if ( M_opponent_array[i] ) M_opponent_array[i]->reverseSide();
        }
    }

    updateOffsideLines();
    updateKicker( prev_state );
    updateInterceptTable();
}

/*-------------------------------------------------------------------*/
/*!

 */
CoachWorldState::CoachWorldState( const rcg::DispInfoT & disp,
                                  const GameTime & current_time,
                                  const GameMode & current_mode,
                                  const CoachWorldState::Ptr & prev_state )
    : M_our_side( NEUTRAL ),
      M_time( current_time ),
      M_game_mode( current_mode ),
      M_our_offside_line_x( 0.0 ),
      M_their_offside_line_x( 0.0 ),
      M_kicker( nullptr ),
      M_ball_owner_side( NEUTRAL ),
      M_ball_owner( nullptr ),
      M_fastest_intercept_player( nullptr ),
      M_fastest_intercept_teammate( nullptr ),
      M_fastest_intercept_opponent( nullptr )
{
    //
    // ball
    //
    M_ball.setValue( disp.show_.ball_.x_,
                     disp.show_.ball_.y_,
                     disp.show_.ball_.vx_,
                     disp.show_.ball_.vy_ );

    //
    // players
    //
    M_all_players.reserve( 22 );
    M_teammates.reserve( 11 );
    M_opponents.reserve( 11 );
    std::fill( M_teammate_array, M_teammate_array + 11, nullptr );
    std::fill( M_opponent_array, M_opponent_array + 11, nullptr );

    for ( size_t i = 0; i < 22; ++i )
    {
        CoachPlayerObject * p = nullptr;

        if ( prev_state )
        {
            const CoachPlayerObject * pp = prev_state->getPlayer( disp.show_.player_[i].side(),
                                                                  disp.show_.player_[i].unum_ );
            if ( pp )
            {
                p = pp->clone();
                p->update( disp.show_.player_[i] );
            }
        }

        if ( ! p )
        {
            p = new CoachPlayerObject();
            p->update( disp.show_.player_[i] );
        }

        if ( p )
        {
            M_all_players.push_back( p );

            if ( p->unum() < 1 || 11 < p->unum() )
            {
                std::cerr << __FILE__ << ' ' << __LINE__
                          << ": illegal uniform number: " << i << ' '
                          << p->side() << ' ' << p->unum() << std::endl;
                continue;
            }

            if ( p->side() == LEFT )
            {
                M_teammates.push_back( p );
                M_teammate_array[p->unum() - 1] = p;
            }
            else if ( p->side() == RIGHT )
            {
                M_opponents.push_back( p );
                M_opponent_array[p->unum() - 1] = p;
            }
            else
            {
                std::cerr << __FILE__ << ' ' << __LINE__
                          << ": illegal side id: " << i << ' '
                          << p->side() << ' ' << p->unum() << std::endl;
            }

        }
    }

    updateOffsideLines();
    updateKicker( prev_state );
    updateInterceptTable();
}

/*-------------------------------------------------------------------*/
/*!

 */
CoachWorldState::~CoachWorldState()
{
    M_teammates.clear();
    M_opponents.clear();

    std::fill( M_teammate_array, M_teammate_array + 11, nullptr );
    std::fill( M_opponent_array, M_opponent_array + 11, nullptr );

    for ( CoachPlayerObject::Cont::iterator p = M_all_players.begin(), end = M_all_players.end();
          p != end;
          ++p )
    {
        delete *p;
        *p = nullptr;
    }

    M_all_players.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::setPlayerType( const SideID side,
                                const int unum,
                                const int type )
{
    if ( side == NEUTRAL
         || unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": illegal player id. side=" << side
                  << " unum="<< unum << std::endl;
        return;
    }

    if ( ourSide() == side
         || ( ourSide() == NEUTRAL
              && side == LEFT ) )
    {
        if ( M_teammate_array[unum-1] )
        {
            M_teammate_array[unum-1]->setPlayerType( type );
        }
    }
    else
    {
        if ( M_opponent_array[unum-1] )
        {
            M_opponent_array[unum-1]->setPlayerType( type );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::setTeammatePlayerType( const int unum,
                                        const int type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": illegal uniform number. " << unum << std::endl;
        return;
    }

    if ( M_teammate_array[unum-1] )
    {
        M_teammate_array[unum-1]->setPlayerType( type );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::setOpponentPlayerType( const int unum,
                                        const int type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": illegal uniform number. " << unum << std::endl;
        return;
    }

    if ( M_opponent_array[unum-1] )
    {
        M_opponent_array[unum-1]->setPlayerType( type );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::setCard( const SideID side,
                          const int unum,
                          const Card card )
{
    if ( side == NEUTRAL
         || unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": illegal player id. side=" << side
                  << " unum="<< unum << std::endl;
        return;
    }


    if ( ourSide() == side
         || ( ourSide() == NEUTRAL
              && side == LEFT ) )
    {
        if ( M_teammate_array[unum-1] )
        {
            M_teammate_array[unum-1]->setCard( card );
        }
    }
    else
    {
        if ( M_opponent_array[unum-1] )
        {
            M_opponent_array[unum-1]->setCard( card );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::updatePlayerStamina( const AudioMemory & audio )
{
    const ServerParam & SP = ServerParam::i();

    const int half_time = SP.actualHalfTime();
    const int normal_time = half_time * SP.nrNormalHalfs();

    if ( time().cycle() < normal_time
         && gameMode().type() == GameMode::BeforeKickOff )
    {
        for ( int i = 0; i < 11; ++i )
        {
            if ( M_teammate_array[i] )
            {
                M_teammate_array[i]->recoverStamina();
            }
            if ( M_opponent_array[i] )
            {
                M_opponent_array[i]->recoverStamina();
            }
        }

        return;
    }

    if ( ourSide() == NEUTRAL )
    {
        return;
    }

    //
    // stamina
    //
    if ( audio.staminaTime() == this->time() )
    {
        for ( const AudioMemory::Stamina & v : audio.stamina() )
        {
            if ( v.sender_ < 1 || 11 < v.sender_ ) continue;
            if ( ! M_teammate_array[v.sender_ - 1] ) continue;

            double value = v.rate_ * SP.staminaMax();

            dlog.addText( Logger::WORLD,
                          __FILE__":(updateTeammateStamina) sender=%d stamina=%.3f",
                          v.sender_, value );

            M_teammate_array[v.sender_ - 1]->setStamina( value );
        }
    }

    //
    // recovery
    //
    if ( audio.recoveryTime() == this->time() )
    {
        // dlog.addText( Logger::WORLD,
        //               __FILE__":(updateTeammateStamina) heard recovery info" );

        for ( const AudioMemory::Recovery & v : audio.recovery() )
        {
            if ( v.sender_ < 1 || 11 < v.sender_ ) continue;
            if ( ! M_teammate_array[v.sender_ - 1] ) continue;

            double value
                = v.rate_ * ( SP.recoverInit() - SP.recoverMin() )
                + SP.recoverMin();

            dlog.addText( Logger::WORLD,
                          __FILE__":(updateTeammateStamina) sender=%d recovery=%.3f",
                          v.sender_, value );

            M_teammate_array[v.sender_ - 1]->setRecovery( value );
        }
    }

    //
    // stamina capacity
    //
    if ( audio.staminaCapacityTime() == this->time() )
    {
        for ( const AudioMemory::StaminaCapacity & v : audio.staminaCapacity() )
        {
            if ( v.sender_ < 1 || 11 < v.sender_ ) continue;
            if ( ! M_teammate_array[v.sender_ - 1] ) continue;

            double value = v.rate_ * SP.staminaCapacity();

            dlog.addText( Logger::WORLD,
                          __FILE__":(updateTeammateStamina) sender=%d staminaCapacity=%.3f",
                          v.sender_, value );

            M_teammate_array[v.sender_ - 1]->setStaminaCapacity( value );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
BallStatus
CoachWorldState::getBallStatus() const
{
    static const double WIDTH
        = ServerParam::i().goalHalfWidth()
        + ServerParam::DEFAULT_GOAL_POST_RADIUS;
    static const Rect2D GOAL_L( Vector2D( - ServerParam::DEFAULT_PITCH_LENGTH * 0.5
                                          - ServerParam::DEFAULT_GOAL_DEPTH
                                          - ServerParam::i().ballSize(),
                                          - WIDTH * 0.5 ),
                                Size2D( ServerParam::DEFAULT_GOAL_DEPTH,
                                        WIDTH ) );
    static const Rect2D GOAL_R( Vector2D( ServerParam::DEFAULT_PITCH_LENGTH * 0.5
                                          + ServerParam::i().ballSize(),
                                          - WIDTH * 0.5 ),
                                Size2D( ServerParam::DEFAULT_GOAL_DEPTH,
                                        WIDTH ) );
    static const Rect2D PITCH( Vector2D( - ServerParam::DEFAULT_PITCH_LENGTH * 0.5
                                         - ServerParam::i().ballSize() * 0.5,
                                         - ServerParam::DEFAULT_PITCH_WIDTH * 0.5
                                         - ServerParam::i().ballSize() * 0.5 ),
                               Size2D( ServerParam::DEFAULT_PITCH_LENGTH
                                       + ServerParam::i().ballSize(),
                                       ServerParam::DEFAULT_PITCH_WIDTH
                                       + ServerParam::i().ballSize() ) );


    if ( GOAL_L.contains( M_ball.pos() ) )
    {
        return Ball_GoalL;
    }

    if ( GOAL_R.contains( M_ball.pos() ) )
    {
        return Ball_GoalR;
    }

    if ( ! PITCH.contains( M_ball.pos() ) )
    {
        return Ball_OutOfField;
    }

    return Ball_InField;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::updateOffsideLines()
{
    //
    // offside line for our team
    //
    {
        double max_x = 0.0;
        double second_max_x = 0.0;

        for ( const CoachPlayerObject * p : opponents() )
        {
            second_max_x = std::max( second_max_x, p->pos().x );
            if ( second_max_x > max_x )
            {
                std::swap( max_x, second_max_x );
            }
        }

        M_our_offside_line_x = second_max_x;
    }

    //
    // offside line for their team
    //
    {
        double min_x = 0.0;
        double second_min_x = 0.0;

        for ( const CoachPlayerObject * p : teammates() )
        {
            second_min_x = std::min( second_min_x, p->pos().x );
            if ( second_min_x < min_x )
            {
                std::swap( min_x, second_min_x );
            }
        }

        M_their_offside_line_x = second_min_x;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::updateKicker( const CoachWorldState::Ptr & prev_state )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::ANALYZER,
                  "(CoachWorldState::updateKicker) ball=(%.3f %.3f)",
                  ball().pos().x, ball().pos().y );
#endif

    if ( ! prev_state )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      "(CoachWorldState::updateKicker) no previous state" );
#endif
        M_kicker = nullptr;
        return;
    }


    const double tacklable = std::sqrt( std::pow( ServerParam::i().tackleDist(), 2 )
                                        + std::pow( ServerParam::i().tackleWidth(), 2 ) )
        + 0.001;
    const double tackle_thr = tacklable + ServerParam::i().ballSpeedMax();

    double min_dist = 1000000.0;
    const CoachPlayerObject * candidate = nullptr;

    for ( const CoachPlayerObject * p : M_all_players )
    {
        if ( ! p->isKicking()
             && p->tackleCycle() != 1 )
        {
            // no kick/tackle observation
            continue;
        }

#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      "(CoachWorldState::updateKicker) found kicking/tackling player %s %d",
                      side_str( p->side() ), p->unum() );
#endif

        const CoachPlayerObject * prev_p = prev_state->getPlayer( p->side(), p->unum() );

        if ( ! prev_p )
        {
            // no previous observation.
            continue;
        }

        const double kickable = ( ( p->type() != Hetero_Unknown
                                    && p->playerTypePtr() )
                                  ? p->playerTypePtr()->kickableArea()
                                  : ServerParam::i().defaultKickableArea() )
            + 0.001;
        const double kick_thr = kickable + ServerParam::i().ballSpeedMax();

        const double current_dist = p->pos().dist( M_ball.pos() );
        const double prev_dist = prev_p->pos().dist( prev_state->ball().pos() );

#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      "(CoachWorldState::updateKicker) kickable=%.3f prev_dist=%.3f cur_dist=%.3f",
                      kickable, prev_dist, current_dist );
        dlog.addText( Logger::ANALYZER,
                      "(CoachWorldState::updateKicker) prev_ball_pos=(%.3f %.3f) prev_player=[%s %d](%.3f %.3f)",
                      prev_state->ball().pos().x, prev_state->ball().pos().y,
                      side_str( prev_p->side() ), prev_p->unum(),
                      prev_p->pos().x, prev_p->pos().y );
#endif

        if ( ( p->isKicking() && prev_dist < kickable && current_dist < kick_thr )
             || ( p->tackleCycle() == 1 && prev_dist <= tacklable && current_dist <= tackle_thr ) )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          "(CoachWorldState::updateKicker) found candidate",
                          kickable, prev_dist, current_dist );
#endif

            M_kicker_candidates.push_back( p );

            if ( prev_dist < min_dist )
            {
                candidate = p;
                min_dist = prev_dist;
            }
        }
    }

    M_kicker = candidate;

#ifdef DEBUG_PRINT
    if ( candidate )
    {
        dlog.addText( Logger::ANALYZER,
                      "(CoachWorldState::updateKicker) detecet candidate kicker %s %d",
                      side_str( candidate->side() ), candidate->unum() );
    }
#endif

    if ( M_kicker_candidates.size() > 1 )
    {
        CoachPlayerObject::Cont::iterator p = M_kicker_candidates.begin();
        CoachPlayerObject::Cont::iterator end = M_kicker_candidates.end();
        SideID side = (*p)->side();
        ++p;
        for ( ; p != end; ++p )
        {
            if ( side != (*p)->side() )
            {
                M_kicker = nullptr;
                break;
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachWorldState::updateInterceptTable()
{
    CoachInterceptPredictor predictor( M_ball );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::INTERCEPT,
                  "(CoachWorldState::updateInterceptTable)" );
#endif

    // Timer timer;

    for ( int i = 0; i < 11; ++i )
    {
        CoachPlayerObject * p = M_teammate_array[i];
        if ( ! p ) continue;

        int step = predictor.predict( *p );
        if ( step >= 0 )
        {
            p->setBallReachStep( step );
            // dlog.addText( Logger::INTERCEPT,
            //               "__ teammate %d step=%d",
            //               (*p)->unum(), step );
        }
    }

    for ( int i = 0; i < 11; ++i )
    {
        CoachPlayerObject * p = M_opponent_array[i];
        if ( ! p ) continue;

        int step = predictor.predict( *p );
        if ( step >= 0 )
        {
            p->setBallReachStep( step );
            // dlog.addText( Logger::INTERCEPT,
            //               "__ opponent %d step=%d",
            //               (*p)->unum(), step );
        }
    }

    std::sort( M_all_players.begin(), M_all_players.end(), BallReachStepSorter() );
    std::sort( M_teammates.begin(), M_teammates.end(), BallReachStepSorter() );
    std::sort( M_opponents.begin(), M_opponents.end(), BallReachStepSorter() );

    if ( ! M_all_players.empty() )
    {
        M_fastest_intercept_player = *M_all_players.begin();
    }

    if ( ! M_teammates.empty() )
    {
        M_fastest_intercept_teammate = *M_teammates.begin();
    }

    if ( ! M_opponents.empty() )
    {
        M_fastest_intercept_opponent = *M_opponents.begin();
    }

    // dlog.addText( Logger::INTERCEPT,
    //               "timer elapsed %.3f [ms]", timer.elapsedReal() );
#ifdef DEBUG_PRINT
    for ( const CoachPlayerPtrCont * p : M_all_players )
    {
        dlog.addText( Logger::INTERCEPT,
                      "__ player %c %d step=%d",
                      side_char( p->side() ), p->unum(),
                      p->ballReachStep() );
    }
#endif
}


/*-------------------------------------------------------------------*/
/*!

 */
const CoachPlayerObject *
CoachWorldState::getPlayerImpl( const SideID side,
                                const int unum ) const
{
    for ( const CoachPlayerObject * p : M_all_players )
    {
        if ( p->side() == side
             && p->unum() == unum )
        {
            return p;
        }
    }

    return nullptr;
}

}
