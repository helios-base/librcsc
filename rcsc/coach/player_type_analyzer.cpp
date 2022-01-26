// -*-c++-*-

/*!
  \file player_type_analyzer.cpp
  \brief player type analyzer class Source File
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
#include "config.h"
#endif

#include "player_type_analyzer.h"

#include "coach_world_model.h"
#include "coach_player_object.h"

#include <rcsc/common/player_param.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/logger.h>
#include <rcsc/game_mode.h>

//#define DEBUG_PRINT

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PlayerTypeAnalyzer::Data::Data()
    : turned_( false ),
      kicked_( false ),
      tackling_( false ),
      maybe_referee_( false ),
      maybe_collide_( false ),
      maybe_kick_( false ),
      pos_( Vector2D::INVALIDATED ),
      vel_( 0.0, 0.0 ),
      body_( -360 ),
      invalid_flags_( PlayerParam::i().playerTypes(), 0 ),
      type_( Hetero_Default )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::Data::setDefaultType()
{
    invalid_flags_.assign( PlayerParam::i().playerTypes(), 0 );

    type_ = Hetero_Default;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::Data::setUnknownType()
{
    invalid_flags_.assign( PlayerParam::i().playerTypes(), 0 );

    type_ = Hetero_Unknown;
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerTypeAnalyzer::PlayerTypeAnalyzer( const CoachWorldModel & world )
    : M_world( world ),
      M_updated_time( -1, 0 ),
      M_playmode( PM_BeforeKickOff )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::update()
{
    if ( M_updated_time == M_world.time() )
    {
        return;
    }

    const std::size_t max_types = static_cast< std::size_t >( PlayerParam::i().playerTypes() );
    for ( int i = 0; i < 11; ++i )
    {
        if ( M_teammate_data[i].invalid_flags_.size() != max_types )
        {
            M_teammate_data[i].invalid_flags_.resize( max_types, 0 );
        }

        if ( M_opponent_data[i].invalid_flags_.size() != max_types )
        {
            M_opponent_data[i].invalid_flags_.resize( max_types, 0 );
        }

#if 0
        // heterogeneous goalie is available in v14 or later
        if ( ! PlayerParam::i().allowMultDefaultType()
             && PlayerParam::i().ptMax() == 1
             && max_types > 0 )
        {
            // goalie is always default type.
            M_teammate_data[i].invalid_flags_[0] = 1;
            M_opponent_data[i].invalid_flags_[0] = 1;
        }
#endif
    }

    if ( M_opponent_type_used_count.size() != max_types )
    {
        M_opponent_type_used_count.resize( max_types, 0 );
#if 0
        // heterogeneous goalie is available in v14 or later
        if ( max_types > 0 )
        {
            M_opponent_type_used_count[0] = 1; // goalie is always default type.
        }
#endif
    }

    if ( M_updated_time.cycle() != M_world.time().cycle() - 1
         && M_updated_time.stopped() != M_world.time().stopped() - 1 )
    {
        // missed cycles??
//         if ( M_world.time().stopped() != 1 )
//         {
//             std::cerr << __FILE__ << ' ' << __LINE__
//                       << " missed cycles? last updated time = " << M_updated_time
//                       << " current = " << M_world.time()
//                       << std::endl;
//         }

        M_updated_time = M_world.time();
        updateLastData();
        return;
    }

    M_updated_time = M_world.time();

    const PlayMode pm = M_world.gameMode().getServerPlayMode();

    // just after the playmode change
    if ( M_playmode != pm )
    {
        M_playmode = pm;
        updateLastData();
        return;
    }

    switch ( M_world.gameMode().type() ) {
    case GameMode::PlayOn:
    case GameMode::KickIn_:
    case GameMode::FreeKick_:
    case GameMode::CornerKick_:
    case GameMode::GoalKick_:
        break;
    default:
        updateLastData();
        return; // not analyzed in other playmode
        break;
    }

    // analyzer player container

    analyze();

    updateLastData();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::reset( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << " Illegal unum = " << unum
                  << std::endl;
        return;
    }

    //if ( M_opponent_data[unum - 1].type_ != Hetero_Unknown )
    //{
    //    std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
    //              << " opponent " << unum << " changed."
    //              << std::endl;
    //}

    M_opponent_data[unum - 1].setUnknownType();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::updateLastData()
{
    M_prev_ball = M_world.ball();

    for ( const CoachPlayerObject * p : M_world.teammates() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_teammate_data[p->unum() - 1];

        data.pos_ = p->pos();
        data.vel_ = p->vel();
        data.body_ = p->body().degree();
    }

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_opponent_data[p->unum() - 1];

        data.pos_ = p->pos();
        data.vel_ = p->vel();
        data.body_ = p->body().degree();
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::analyze()
{
    checkTurn();
    checkTackle();
    checkReferee();
    checkCollisions();
    checkKick();
    checkPlayerDecay();
    checkPlayerSpeedMax();
    checkTurnMoment();

    const int max_types = PlayerParam::i().playerTypes();

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << " Illegal uniform number " << p->unum()
                      << std::endl;
            continue;
        }

        Data & data = M_opponent_data[p->unum() - 1];

#if 0
        // heterogeneous goalie is available in v14 or later
        // goalie is always the default type in v13 or previous.
        if ( p->goalie() )
        {
            data.type_ = Hetero_Default;
            continue;
        }
#endif

        // if player type is not changed, not need to analyze
        if ( data.type_ == Hetero_Default ) continue;
        // if player type has already been determined, not need to analyze
        if ( data.type_ != Hetero_Unknown ) continue;
        // if player might be moved by referee, we must not analyze
        if ( data.maybe_referee_ ) continue;

        int invalid_count = 0;
        for ( int t = 0; t < max_types; ++t )
        {
            if ( data.invalid_flags_[t] != 0 )
            {
                ++invalid_count;
            }
        }

#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (analyze) opponent %d. invalid count=%d",
                      p->unum(), invalid_count );
#endif

        if ( invalid_count == max_types )
        {
            // no candidate
            std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                      << " no player type for opponent " << p->unum()
                      << ". restart analysis."
                      << std::endl;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (analyze) opponent %d. no player type candidate. restart analysis.",
                          p->unum() );
#endif
            data.setUnknownType();
        }
        else if ( invalid_count == max_types - 1 )
        {
            // success! only 1 candidate.
            for ( int t = 0; t < max_types; ++t )
            {
                if ( data.invalid_flags_[t] == 0 )
                {
                    std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                              << " determined opponent "
                              << p->unum() << " type = " << t << std::endl;
#ifdef DEBUG_PRINT
                    dlog.addText( Logger::ANALYZER,
                                  __FILE__" (analyze) determined opponent %d. type=%d",
                                  p->unum(), t );
#endif
                    data.type_ = t;

                    M_opponent_type_used_count[t] += 1;
                    if ( M_opponent_type_used_count[t] >= PlayerParam::i().ptMax() )
                    {
                        for ( int i = 0; i < 11; ++i )
                        {
                            if ( M_opponent_data[i].type_ == Hetero_Unknown )
                            {
                                M_opponent_data[i].invalid_flags_[t] = 1;
                            }
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            // several candidates
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (analyze) opponent %d. several player type candidates = %d.",
                          p->unum(),
                          max_types - invalid_count );
#endif
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkTurn()
{
    for ( int i = 0; i < 11 ; ++i )
    {
        M_teammate_data[i].turned_ = false;
        M_opponent_data[i].turned_ = false;
    }

    for ( const CoachPlayerObject * t : M_world.teammates() )
    {
        if ( t->unum() < 1 || 11 < t->unum() ) continue;

        Data & data = M_teammate_data[t->unum() - 1];

        if ( data.body_ != -360.0 )
        {
            if ( std::fabs( data.body_ - t->body().degree() ) > 0.5 )
            {
                data.turned_ = true;
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkTurn) teammate %d turned 1",
                              (*p)->unum() );
#endif
            }
#ifdef DEBUG_PRINT
            else
            {
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkTurn) teammate %d turned 0",
                              (*p)->unum() );
            }
#endif
        }
    }

    for ( const CoachPlayerObject *o : M_world.opponents() )
    {
        if ( o->unum() < 1 || 11 < o->unum() ) continue;

        Data & data = M_opponent_data[ o->unum() - 1];

        if ( data.body_ != -360.0 )
        {
            if ( std::fabs( data.body_ - o->body().degree() ) > 0.5 )
            {
                data.turned_ = true;
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkTurn) opponent %d turned 1",
                              (*p)->unum() );
#endif
            }
#ifdef DEBUG_PRINT
            else
            {
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkTurn) opponent %d turned 0",
                              (*p)->unum() );
            }
#endif
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkTackle()
{
    for ( int i = 0; i < 11 ; ++i )
    {
        M_teammate_data[i].tackling_ = false;
        M_opponent_data[i].tackling_ = false;
    }

    for ( const CoachPlayerObject * p : M_world.teammates() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        M_teammate_data[p->unum() - 1].tackling_ = p->isTackling();
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkTackle) teammate %d tackling %d",
                      (*p)->unum(), (int)(*p)->isTackling() );
#endif
    }

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        M_opponent_data[p->unum() - 1].tackling_ = p->isTackling();
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkTackle) opponent %d tackling %d",
                      p->unum(), (int)(*p->isTackling()) );
#endif
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkReferee()
{
    for ( int i = 0; i < 11; ++i )
    {
        M_opponent_data[i].maybe_referee_ = false;
    }

    const double penalty_x
        = ServerParam::i().pitchHalfLength()
        - ServerParam::i().penaltyAreaLength()
        - 2.0;
    const double penalty_y
        = ServerParam::i().penaltyAreaWidth() * 0.5
        + 2.0;

    const bool our_set_play = M_world.gameMode().isOurSetPlay( M_world.ourSide() );

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_opponent_data[p->unum() - 1];

        // player may be moved by referee
        if ( our_set_play )
        {
            if ( p->pos().dist2( M_world.ball().pos() ) < 12.0 * 12.0
                 || ( M_world.gameMode().type() == GameMode::GoalKick_
                      && p->pos().absX() > penalty_x
                      && p->pos().absY() < penalty_y )
                 )
            {
                data.maybe_referee_ = true;
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkReferee) opponent %d. maybe moved by referee.",
                              p->unum() );
#endif
            }
        }

        // player may be moved by simulator
        if ( p->pos().absX() > ServerParam::i().pitchHalfLength() + 3.0
             || p->pos().absY() > ServerParam::i().pitchHalfWidth() + 3.0 )
        {
            data.maybe_referee_ = true;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (checkReferee) opponent %d. out of pitch. maybe moved by referee.",
                          p->unum() );
#endif
        }
    }
}

/*-------------------------------------------------------------------*/
/*!
  \todo strict player size check
*/
void
PlayerTypeAnalyzer::checkCollisions()
{
    for ( int i = 0; i < 11; ++i )
    {
        M_opponent_data[i].maybe_collide_ = false;
    }

    const double ball_collide_dist2
        = std::pow( ServerParam::i().defaultPlayerSize()
                    + ServerParam::i().ballSize()
                    + 0.02,
                    2 );
    const double player_collide_dist2
        = std::pow( ServerParam::i().defaultPlayerSize() * 2.0 + 0.02, 2 );
    const Vector2D pole_pos( ServerParam::i().pitchHalfLength()
                             - ServerParam::i().goalPostRadius(),
                             ServerParam::i().goalHalfWidth()
                             + ServerParam::i().goalPostRadius() );
    const double pole_collide_dist2
        = std::pow( ServerParam::i().defaultPlayerSize()
                    + ServerParam::i().goalPostRadius()
                    + 2.0,
                    2 );


    const CoachPlayerObject::Cont & opponents = M_world.opponents();

    // check ball
    for ( const CoachPlayerObject * p : opponents )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_opponent_data[p->unum() - 1];

        if ( p->pos().dist2( M_world.ball().pos() ) < ball_collide_dist2 )
        {
            data.maybe_collide_ = true;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (checkCollisions) opponent %d. may be collided with ball",
                          (*p)->unum() );
#endif
        }
    }

    // check other opponent players
    for ( CoachPlayerObject::Cont::const_iterator p = opponents.begin(), o_end = opponents.end();
          p != o_end;
          ++p )
    {
        if ( (*p)->unum() < 1 || 11 < (*p)->unum() ) continue;

        Data & data = M_opponent_data[(*p)->unum() - 1];

        for ( CoachPlayerObject::Cont::const_iterator pp = p + 1;
              pp != o_end;
              ++pp )
        {
            if ( (*pp)->unum() == (*p)->unum() ) continue;

            if ( (*pp)->pos().dist2( (*p)->pos() ) < player_collide_dist2 )
            {
                data.maybe_collide_ = true;
                if ( 1 <= (*pp)->unum()
                     && (*pp)->unum() <= 11 )
                {
                    M_opponent_data[(*pp)->unum() - 1].maybe_collide_ = true;
                }
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkCollisions) opponent %d. may be collided with opponent %d",
                              (*p)->unum(), (*pp)->unum() );
#endif
            }
        }
    }

    // check collision with teammate players
    const CoachPlayerObject::Cont & teammates = M_world.teammates();

    for ( const CoachPlayerObject * o : opponents )
    {
        if ( o->unum() < 1 || 11 < o->unum() ) continue;

        Data & data = M_opponent_data[ o->unum() - 1 ];

        if ( data.maybe_collide_ ) continue;

        for ( const CoachPlayerObject * t : teammates )
        {
            if ( t->pos().dist2( o->pos() ) < player_collide_dist2 )
            {
                data.maybe_collide_ = true;
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkCollisions) opponent %d. may be collided with teammate %d",
                              (*o)->unum(), (*t)->unum() );
#endif
                break;
            }
        }
    }

    // check goal post
    for ( const CoachPlayerObject * o : opponents )
    {
        if ( o->unum() < 1 || 11 < o->unum() ) continue;

        Data & data = M_opponent_data[o->unum() - 1];

        if ( data.maybe_collide_ ) continue;

        Vector2D abs_pos( o->pos().absX(), o->pos().absY() );
        if ( abs_pos.dist2( pole_pos ) < pole_collide_dist2 )
        {
            data.maybe_collide_ = true;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (checkCollisions) opponent %d. may be collided with goal post",
                          o->unum() );
#endif
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkKick()
{
    static double S_max_kickable_area2 = -1.0;

    for ( int i = 0; i < 11; ++i )
    {
        M_opponent_data[i].kicked_ = false;
        M_teammate_data[i].kicked_ = false;
        M_opponent_data[i].maybe_kick_ = false;
        M_teammate_data[i].maybe_kick_ = false;
    }

    const int max_types = PlayerParam::i().playerTypes();

    if ( S_max_kickable_area2 < 0.0 )
    {
        for ( int t = 0; t < max_types; ++t )
        {
            const PlayerType * player_type = PlayerTypeSet::i().get( t );
            if ( ! player_type ) continue;

            double k2 = std::pow( player_type->kickableArea(), 2 );
            if ( k2 > S_max_kickable_area2 )
            {
                S_max_kickable_area2 = k2;
            }
        }
    }

    bool ball_kicked = false;

    const Vector2D new_ball_pos = M_prev_ball.pos() + M_prev_ball.vel();
    const Vector2D new_ball_vel = M_prev_ball.vel() * ServerParam::i().ballDecay();
    const double rand_max = M_prev_ball.vel().r() * ServerParam::i().ballRand();

    if ( std::fabs( M_world.ball().pos().x - new_ball_pos.x ) > rand_max
         || std::fabs( M_world.ball().pos().y - new_ball_pos.y ) > rand_max )
    {
        ball_kicked = true;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkKick) too large ball move." );
#endif
    }

    if ( std::fabs( M_world.ball().vel().x - new_ball_vel.x )
         > ServerParam::i().ballDecay() * rand_max
         || ( std::fabs( M_world.ball().vel().y - new_ball_vel.y )
              > ServerParam::i().ballDecay() * rand_max )
         )
    {
        ball_kicked = true;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkKick) ball velocity changed." );
#endif
    }

    int count = 0;
    int kicker_idx = -1;

    if ( M_world.clientVersion() >= 13 )
    {
        for ( const CoachPlayerObject * p : M_world.teammates() )
        {
            if ( p->unum() < 1 || 11 < p->unum() ) continue;

            Data & data = M_teammate_data[p->unum() - 1];

            data.kicked_ = data.maybe_kick_ = p->isKicking();
            if ( p->isKicking() )
            {
                ++count;
            }
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (checkKick) teammate %d. kicking %d",
                          p->unum(), (int)p->kicked() );
#endif
        }

        for ( const CoachPlayerObject * p : M_world.opponents() )
        {
            if ( p->unum() < 1 || 11 < p->unum() ) continue;

            Data & data = M_opponent_data[ p->unum() - 1];

            data.kicked_ = data.maybe_kick_ = p->isKicking();
            if ( p->isKicking() )
            {
                ++count;
                kicker_idx = p->unum() - 1;
            }
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (checkKick) opponent %d. kicking %d",
                          p->unum(), (int)p->kicked() );
#endif
        }
    }
    else
    {
        // v12 or older client cannot receive any kicking state information.

        for ( int i = 0; i < 11; ++i )
        {
            if ( ! M_teammate_data[i].turned_
                 && ! M_teammate_data[i].tackling_
                 && M_teammate_data[i].pos_.isValid() )
            {
                if ( M_prev_ball.pos().dist2( M_teammate_data[i].pos_ )
                     < S_max_kickable_area2 )
                {
                    M_teammate_data[i].maybe_kick_ = true;
                    ++count;
                }
            }

            if ( ! M_opponent_data[i].turned_
                 && ! M_opponent_data[i].tackling_
                 && M_opponent_data[i].pos_.isValid() )
            {
                if ( M_prev_ball.pos().dist2( M_opponent_data[i].pos_ )
                     < S_max_kickable_area2 )
                {
                    M_opponent_data[i].maybe_kick_ = true;
                    ++count;
                    kicker_idx = i;
                }
            }
        }
    }

    if ( ! ball_kicked )
    {
        // do nothing
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkKick) ball is not kicked." );
#endif
        return;
    }

    if ( count == 0 )
    {
        // ball may be tackled
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkKick) no kicker. ball is tackled?" );
#endif
    }
    else if ( count == 1 && kicker_idx != -1 )
    {
        Data & data = M_opponent_data[kicker_idx];

        if ( data.maybe_collide_ )
        {
            // cannot determine kick or collide.
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ANALYZER,
                          __FILE__" (checkKick) opponent kicker candidate = %d. but maybe collide",
                          kicker_idx + 1 );
#endif
        }
        else
        {
            const double ball_dist = M_prev_ball.pos().dist( data.pos_ );

            for ( int t = 0; t < max_types; ++t )
            {
                if ( data.invalid_flags_[t] != 0 ) continue;

                const PlayerType * player_type = PlayerTypeSet::i().get( t );
                if ( ! player_type ) continue;


                if ( ball_dist > player_type->kickableArea() + 0.001 )
                {
                    data.invalid_flags_[t] = 1;
#ifdef DEBUG_PRINT
                    std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                              << " opponent " << kicker_idx + 1
                              << "  detect invalid kickable area. type = "
                              << t
                              << std::endl;
                    dlog.addText( Logger::ANALYZER,
                                  __FILE__" (checkKick) opponent=%d type=%d,"
                                  " out of range kickable area."
                                  " ball_dist=%f kickable_area=%f",
                                  kicker_idx + 1, t,
                                  ball_dist, player_type->kickableArea() );
#endif
                }
            }
        }
    }
    else
    {
        // several kickers
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ANALYZER,
                      __FILE__" (checkKick) several kickers" );
#endif
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkPlayerDecay()
{
    const int max_types = PlayerParam::i().playerTypes();

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_opponent_data[p->unum() - 1];

        if ( data.maybe_collide_ ) continue;
        if ( data.maybe_referee_ ) continue;
        if ( ! data.turned_
             && ! data.kicked_
             //&& ! data.maybe_kick_
             && ! data.tackling_ )
        {
            continue;
        }
        if ( ! data.pos_.isValid() ) continue;
        if ( data.pos_.dist2( p->pos() ) < 0.0001 ) continue;

        double rand_max = data.vel_.r() * ServerParam::i().playerRand();
        if ( rand_max < 0.00001 ) continue;

        for ( int t = 0; t < max_types; ++t )
        {
            if ( data.invalid_flags_[t] != 0 ) continue;

            const PlayerType * player_type = PlayerTypeSet::i().get( t );
            if ( ! player_type ) continue;

#if 0
            // old noise model
            double rand_x
                = std::fabs( ( p->vel().x
                               - data.vel_.x * player_type->playerDecay() )
                             / player_type->playerDecay() );

            double rand_y
                = std::fabs( ( p->vel().y
                               - data.vel_.y * player_type->playerDecay() )
                             / player_type->playerDecay() );

            if ( rand_x > rand_max + 0.0000001
                 || rand_y > rand_max + 0.0000001 )
            {
                data.invalid_flags_[t] = 1;
                //std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                //          << "opponent " << p->unum()
                //          << "  detect invalid decay. type = "
                //          << t
                //          << std::endl;
            }
#else
            // rcssserver-13 or lator
            Vector2D rand_vec
                = ( p->vel() - data.vel_ * player_type->playerDecay() )
                / player_type->playerDecay();
            double rand_r = rand_vec.r();
            if ( rand_r > rand_max + 0.0000001 )
            {
                data.invalid_flags_[t] = 1;
#ifdef DEBUG_PRINT
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkPlayerDecay) opponent=%d type=%d"
                              " out of range player decay. rand_r=%f rand_max=%f",
                              p->unum(), t,
                              rand_r, rand_max );
#endif
            }
#endif
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkPlayerSpeedMax()
{
    const int max_types = PlayerParam::i().playerTypes();

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_opponent_data[p->unum() - 1];

        if ( data.turned_ ) continue;
        if ( data.kicked_ ) continue;
        if ( data.maybe_referee_ ) continue;
        if ( data.tackling_ ) continue;
        if ( data.maybe_collide_ ) continue;
        if ( ! data.pos_.isValid() ) continue;

        const Vector2D last_move( p->pos().x - data.pos_.x,
                                  p->pos().y - data.pos_.y );
        const double last_move_dist = last_move.r();
        const Vector2D last_accel = last_move - data.vel_;
        const double last_accel_r = last_accel.r();
        const double current_speed = p->vel().r();

        for ( int t = 0; t < max_types; ++t )
        {
            if ( data.invalid_flags_[t] != 0 ) continue;

            const PlayerType * ptype = PlayerTypeSet::i().get( t );
            if ( ! ptype ) continue;

            //
            // accel range check
            //
            const double max_accel = ServerParam::i().maxDashPower() * ptype->dashRate( ptype->effortMax() );
            const double last_max_noise = ( current_speed / ptype->playerDecay()
                                            * ServerParam::i().playerRand()
                                            / ( 1.0 + ServerParam::i().playerRand() ) );

            if ( last_accel_r > max_accel + last_max_noise + 0.0001 )
            {
                data.invalid_flags_[t] = 1;
#ifdef DEBUG_PRINT
                std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                          << " opponent " << p->unum()
                          << " type = " << t
                          << "  detect out of range accel. last_accel=" << last_accel_r
                          << " max_accel=" << max_accel
                          << " max_noise=" << last_max_noise
                          << std::endl;
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkPlayerSpeedMax) opponent=%d type=%d"
                              " out of range accel. last_accel=%f max_accel=%f max_noize=%f",
                              p->unum(), t,
                              last_accel_r, max_accel, last_max_noise );
#endif
                continue;
            }

            //
            // speed range check
            //

            // XXX
            double max_move = ptype->realSpeedMax() * ( 1.0 + ServerParam::i().playerRand() );
            max_move *= ptype->playerDecay();
            max_move += ServerParam::i().maxDashPower() * ptype->dashRate( ptype->effortMax() );
            max_move *= ( 1.0 + ServerParam::i().playerRand() );

//             if ( p->unum() == 1 )
//             {
//                 dlog.addText( Logger::ANALYZER,
//                               __FILE__" (checkPlayerSpeedMax) type=%d real_speed_max=%f max_move=%f",
//                               t, ptype->realSpeedMax(), max_move );
//             }

            //double max_move = ptype->realSpeedMax() * ( 1.0 + ServerParam::i().playerRand() );

            if ( last_move_dist > max_move )
            {
                data.invalid_flags_[t] = 1;
#ifdef DEBUG_PRINT
                std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                          << " opponent " << p->unum()
                          << " type = " << t
                          << " detect out of range speed. last_move_dist=" << last_move_dist
                          << " max_move_dist=" << max_move
                          << std::endl;
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkPlayerSpeedMax) opponent=%d type=%d"
                              " out of range speed. last_move_dist=%f max_move=%f",
                              p->unum(), t,
                              last_move_dist, max_move );
#endif
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerTypeAnalyzer::checkTurnMoment()
{
    const int max_types = PlayerParam::i().playerTypes();
    const double max_moment = std::max( std::fabs( ServerParam::i().minMoment() ),
                                        std::fabs( ServerParam::i().maxMoment() ) );

    for ( const CoachPlayerObject * p : M_world.opponents() )
    {
        if ( p->unum() < 1 || 11 < p->unum() ) continue;

        Data & data = M_opponent_data[p->unum() - 1];

        if ( ! data.turned_ ) continue;

        const double player_speed = data.vel_.r();
        const double turn_angle = ( p->body() - data.body_ ).abs();

        for ( int t = 0; t < max_types; ++t )
        {
            if ( data.invalid_flags_[t] != 0 ) continue;

            const PlayerType * ptype = PlayerTypeSet::i().get( t );
            if ( ! ptype ) continue;

            const double max_turn = max_moment / ( 1.0 + ptype->inertiaMoment() * player_speed );

            if ( turn_angle > max_turn * ( 1.0 + ServerParam::i().playerRand() ) + 1.0001 )
            {
                data.invalid_flags_[t] = 1;
#ifdef DEBUG_PRINT
                std::cout << M_world.ourTeamName() << " coach: " << M_world.time()
                          << " opponent " << p->unum()
                          << " type = " << t
                          << "  detect out of range turn."
                          << " turn_angle=" << turn_angle
                          << " max_turn=" << max_turn * ( 1.0 + ServerParam::i().playerRand() )
                          << std::endl;
                dlog.addText( Logger::ANALYZER,
                              __FILE__" (checkTurnMoment) opponent=%d type=%d,"
                              " out of range turn moment."
                              " turn_angle=%f max_turn=%f",
                              p->unum(), t,
                              turn_angle, max_turn );
#endif
            }
        }
    }
}

}
