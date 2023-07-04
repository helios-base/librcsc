// -*-c++-*-

/*!
  \file world_model.cpp
  \brief world model Source File
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

#include "world_model.h"

#include "action_effector.h"
#include "localization_default.h"
#include "body_sensor.h"
#include "visual_sensor.h"
#include "fullstate_sensor.h"
#include "debug_client.h"
#include "penalty_kick_state.h"
#include "player_command.h"
#include "player_predicate.h"

#include <rcsc/common/audio_memory.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/server_param.h>
#include <rcsc/time/timer.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

#include <set>
#include <algorithm>
#include <limits>
#include <cassert>
#include <cmath>

// #define DEBUG_PROFILE
// #define DEBUG_PRINT

// #define DEBUG_PRINT_SELF_UPDATE
// #define DEBUG_PRINT_BALL_UPDATE
// #define DEBUG_PRINT_PLAYER_UPDATE
// #define DEBUG_PRINT_PLAYER_UPDATE_DETAIL
// #define DEBUG_PRINT_GOALIE_UPDATE

// #define DEBUG_PRINT_LINES
// #define DEBUG_PRINT_LAST_KICKER

#define USE_VIEW_GRID_MAP

namespace rcsc {


namespace  {

/*!
  \brief create specific player reference set
  \param players player instance container
  \param players_from_self reference container
  \param players_from_ball reference container
  \param self_pos self position
  \param ball_pos ball position
*/
inline
void
create_player_set( PlayerObject::List & players,
                   PlayerObject::Cont & players_from_self,
                   PlayerObject::Cont & players_from_ball,
                   const Vector2D & self_pos,
                   const Vector2D & ball_pos )

{
    for ( PlayerObject & p : players )
    {
        p.updateSelfBallRelated( self_pos, ball_pos );
        players_from_self.push_back( &p );
        players_from_ball.push_back( &p );
    }
}

#if 0
/*!
  \brief check if player is ball kickable or not
  \param first first element in player container
  \param last last element in player container
  \param ball_count ball accuracy count
  \param ball_error effor of ball position
  \param dist_error_rate observation error rate
*/
inline
const PlayerObject *
get_kickable_player( PlayerObject::Cont::iterator first,
                     PlayerObject::Cont::iterator last,
                     const int ball_count,
                     const double & ball_error,
                     const double & dist_error_rate )
{
    for ( ; first != last; ++first )
    {
        if ( (*first)->isGhost()
             || (*first)->isTackling()
             || (*first)->posCount() > ball_count + 1 )
        {
            continue;
        }

        if ( (*first)->isKickable( - ( ball_error
                                       + std::min( 0.25,
                                                   (*first)->distFromSelf()
                                                   * dist_error_rate ) ) ) )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (check_player_kickable) exist %d-%d (%.1f %.1f)",
                          (*first)->side(),
                          (*first)->unum(),
                          (*first)->pos().x, (*first)->pos().y );
            return *first;
        }

        break;
    }

    return nullptr;
}
#endif

/*!

 */
bool
is_reverse_side( const WorldModel & wm,
                 const PenaltyKickState & pen_state )
{
    if ( pen_state.onfieldSide() == LEFT )
    {
        if ( pen_state.isKickTaker( wm.ourSide(), wm.self().unum() ) )
        {
            return true;
        }
        else if ( wm.self().goalie() )
        {
            return false;
        }
    }
    else if ( pen_state.onfieldSide() == RIGHT )
    {
        if ( pen_state.isKickTaker( wm.ourSide(), wm.self().unum() ) )
        {
            return false;
        }
        else if ( wm.self().goalie() )
        {
            return true;
        }
    }

    return ( wm.ourSide() == RIGHT );
}

#if 0
/*!

 */
double
get_self_face_angle( const WorldModel & wm,
                     const PenaltyKickState & pen_state,
                     const double & seen_face_angle )
{
    if ( pen_state.onfieldSide() == LEFT )
    {
        if ( pen_state.isKickTaker( wm.ourSide(), wm.self().unum() ) )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (get_self_face_angle) pen_onfield=LEFT && kicker -> reverse" );
            return AngleDeg::normalize_angle( seen_face_angle + 180.0 );
        }
        else if ( wm.self().goalie() )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (get_self_face_angle) pen_onfield=LEFT && goalie -> no reverse" );
            return seen_face_angle;
        }
    }
    else if ( pen_state.onfieldSide() == RIGHT )
    {
        if ( pen_state.isKickTaker( wm.ourSide(), wm.self().unum() ) )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (get_self_face_angle) pen_onfield=RIGHT && kicker -> no reverse" );
            return seen_face_angle;
        }
        else if ( wm.self().goalie() )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (get_self_face_angle) pen_onfield=RIGHT && goalie -> reverse" );
            return AngleDeg::normalize_angle( seen_face_angle + 180.0 );
        }
    }

    dlog.addText( Logger::WORLD,
                  __FILE__" (get_self_face_angle) normal " );

    return ( wm.ourSide() == LEFT
             ? seen_face_angle
             : AngleDeg::normalize_angle( seen_face_angle + 180.0 ) );
}
#endif

const
AbstractPlayerObject *
get_our_goalie_loop( const WorldModel & wm )
{
    if ( wm.self().goalie() )
    {
        return &wm.self();
    }

    for ( const PlayerObject * p : wm.teammates() )
    {
        if ( p->goalie() )
        {
            return p;
        }
    }

    return nullptr;
}

const
AbstractPlayerObject *
get_their_goalie_loop( const WorldModel & wm )
{
    for ( const PlayerObject * p : wm.opponents() )
    {
        if ( p->goalie() )
        {
            return p;
        }
    }

    return nullptr;
}


struct PlayerUpdater {
    void operator()( PlayerObject & player )
      {
          player.update();
      }
};

struct PlayerValidChecker {
    bool operator()( const PlayerObject & player ) const
      {
          return ( ! player.posValid() );
      }
};

struct PlayerUnumSorter {

    bool operator()( const PlayerObject & lhs,
                     const PlayerObject & rhs ) const
      {
          return lhs.unum() < rhs.unum();
      }
};

struct PlayerCountSorter {

    bool operator()( const PlayerObject & lhs,
                     const PlayerObject & rhs ) const
      {
          return lhs.posCount() + lhs.ghostCount() * 10
              < rhs.posCount() + rhs.ghostCount() * 10;
      }
};

struct PlayerPtrAccuracySorter {
    bool operator()( const PlayerObject * lhs,
                     const PlayerObject * rhs ) const
      {
          if ( lhs->goalie() ) return true;
          if ( rhs->goalie() ) return false;
          if ( lhs->unum() != Unum_Unknown
               && rhs->unum() == Unum_Unknown )
          {
              return true;
          }
          if ( lhs->unum() == Unum_Unknown
               && rhs->unum() != Unum_Unknown )
          {
              return false;
          }
          return lhs->posCount() + lhs->ghostCount() * 10
              < rhs->posCount() + rhs->ghostCount() * 10;
      }
};

struct PlayerPtrSelfDistSorter {
    bool operator()( const PlayerObject * lhs,
                     const PlayerObject * rhs ) const
      {
          return lhs->distFromSelf() < rhs->distFromSelf();
      }
};


struct PlayerPtrBallDistSorter {
    bool operator()( const PlayerObject * lhs,
                     const PlayerObject * rhs ) const
      {
          return lhs->distFromBall() < rhs->distFromBall();
      }
};

}


/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/

const double WorldModel::DIST_TOO_FAR = 1.0e+14;
const std::size_t WorldModel::MAX_RECORD = 30;
const double WorldModel::DIR_STEP = 360.0 / static_cast< double >( DIR_CONF_DIVS );

/*-------------------------------------------------------------------*/
/*!

 */
WorldModel::WorldModel()
    : M_client_version( 8 ),
      M_localize(),
      M_intercept_table(),
      M_audio_memory( new AudioMemory() ),
      M_penalty_kick_state( new PenaltyKickState() ),
      M_our_side( NEUTRAL ),
      M_time( -1, 0 ),
      M_sense_body_time( -1, 0 ),
      M_see_time( -1, 0 ),
      M_decision_time( -1, 0 ),
      M_last_set_play_start_time( 0, 0 ),
      M_setplay_count( 0 ),
      M_game_mode(),
      M_training_time( -1, 0 ),
      M_valid( true ),
      M_self(),
      M_ball(),
      M_our_goalie_unum( Unum_Unknown ),
      M_their_goalie_unum( Unum_Unknown ),
      M_offside_line_x( 0.0 ),
      M_prev_offside_line_x( 0.0 ),
      M_offside_line_count( 0 ),
      M_our_offense_line_x( 0.0 ),
      M_our_defense_line_x( 0.0 ),
      M_their_offense_line_x( 0.0 ),
      M_their_defense_line_x( 0.0 ),
      M_their_defense_line_count( 0 ),
      M_our_offense_player_line_x( 0.0 ),
      M_our_defense_player_line_x( 0.0 ),
      M_their_offense_player_line_x( 0.0 ),
      M_their_defense_player_line_x( 0.0 ),
      M_kickable_teammate( nullptr ),
      M_kickable_opponent( nullptr ),
      M_maybe_kickable_teammate( nullptr ),
      M_maybe_kickable_opponent( nullptr ),
      M_previous_kickable_teammate( false ),
      M_previous_kickable_teammate_unum( Unum_Unknown ),
      M_previous_kickable_opponent( false ),
      M_previous_kickable_opponent_unum( Unum_Unknown ),
      M_last_kicker_side( NEUTRAL ),
      M_last_kicker_unum( Unum_Unknown ),
      M_view_area_cont( MAX_RECORD, ViewArea() )
{
    assert( M_penalty_kick_state );

    for ( int i = 0; i < 11; ++i )
    {
        M_our_recovery[i] = 1.0;
        M_our_stamina_capacity[i] = ServerParam::i().staminaCapacity();
        M_our_card[i] = NO_CARD;
        M_their_card[i] = NO_CARD;
    }

    for ( int i = 0; i < DIR_CONF_DIVS; i++ )
    {
        M_dir_count[i] = 1000;
    }

    for ( int i = 0; i < 12; ++i )
    {
        M_our_player_array[i] = nullptr;
        M_their_player_array[i] = nullptr;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
WorldModel::~WorldModel()
{
    if ( M_penalty_kick_state )
    {
        delete M_penalty_kick_state;
        M_penalty_kick_state = nullptr;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
WorldModel::init( const std::string & teamname,
                  const SideID ourside,
                  const int my_unum,
                  const bool my_goalie,
                  const double client_version )
{
    if ( ! M_localize )
    {
        std::cerr << teamname << ' '
                  << my_unum << ':'
                  << " ***ERROR*** Failed to create localization object."
                  << std::endl;
        return false;
    }

    if ( ! M_audio_memory )
    {
        std::cerr << teamname << ' '
                  << my_unum << ':'
                  << " ***ERROR*** No audio message holder."
                  << std::endl;
        return false;
    }

    M_client_version = client_version;

    M_our_team_name = teamname;
    M_our_side = ourside;
    M_self.init( ourside, my_unum, my_goalie );

    if ( my_goalie )
    {
        M_our_goalie_unum = my_unum;
    }

    for ( int i = 0; i < 11; i++ )
    {
        M_our_player_type[i] = Hetero_Default;
        M_their_player_type[i] = Hetero_Default;
    }

    PlayerTypeSet::instance().resetDefaultType();
    M_self.setPlayerType( Hetero_Default );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
WorldModel::isValid() const
{
    return M_valid;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setValid( bool is_valid )
{
    M_valid = is_valid;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
PenaltyKickState *
WorldModel::penaltyKickState() const
{
    return M_penalty_kick_state;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setAudioMemory( std::shared_ptr< AudioMemory > memory )
{
    M_audio_memory = memory;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setLocalization( std::shared_ptr< Localization > localization )
{
    M_localize = localization;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setServerParam()
{
    for ( int i = 0; i < 11; ++i )
    {
        M_our_stamina_capacity[i] = ServerParam::i().staminaCapacity();
    }

    setOurPlayerType( self().unum(), Hetero_Default );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setOurGoalieUnum( const int unum )
{
    if ( 1 <= unum && unum <= 11 )
    {
        M_our_goalie_unum = unum;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setTheirGoalieUnum( const int unum )
{
    if ( 1 <= unum && unum <= 11 )
    {
        M_their_goalie_unum = unum;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setOurPlayerType( const int unum,
                              const int id )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** WorldModel:: setTeammatePlayerType "
                  << " Illegal uniform number" << unum
                  << std::endl;
        return;
    }

    dlog.addText( Logger::WORLD,
                  __FILE__" (setTeammatePlayerType) teammate %d to player_type %d",
                  unum, id );

    M_our_recovery[unum - 1] = 1.0;
    M_our_stamina_capacity[unum - 1] = ServerParam::i().staminaCapacity();

    M_our_player_type[unum - 1] = id;
    M_our_card[unum - 1] = NO_CARD;

    if ( unum == self().unum() )
    {
        const PlayerType * tmp = PlayerTypeSet::i().get( id );
        if ( ! tmp )
        {
            std::cerr << teamName() << " : " << self().unum()
                      << "WorldModel: Illega player type id??"
                      << " player type param not found, id = "
                      << id << std::endl;
            return;
        }
        M_self.setPlayerType( id );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setTheirPlayerType( const int unum,
                                const int id )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** WorldModel:: setOpponentPlayerType "
                  << " Illegal uniform number"
                  << unum << std::endl;
        return;
    }

    dlog.addText( Logger::WORLD,
                  __FILE__" (setOpponentPlayerType) opponent %d to player_type %d",
                  unum, id );

    if ( M_their_player_type[unum - 1] != Hetero_Unknown
         && M_their_player_type[unum - 1] != id )
    {
        M_their_card[unum - 1] = NO_CARD;
    }

    M_their_player_type[unum - 1] = id;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setCard( const SideID side,
                     const int unum,
                     const Card card )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** (WorldModel::setCard) "
                  << " Illegal uniform number"
                  << unum << std::endl;
        return;
    }

    if ( side == ourSide() )
    {
        if ( self().unum() == unum )
        {
            M_self.setCard( card );
        }
        M_our_card[unum - 1] = card;

        for ( PlayerObject & p : M_teammates )
        {
            if ( p.unum() == unum )
            {
                p.forget();
            }
        }

        dlog.addText( Logger::WORLD,
                      __FILE__" (setCard) teammate %d, card %d",
                      unum, card );
    }
    else if ( side == theirSide() )
    {
        M_their_card[unum - 1] = card;

        for ( PlayerObject & p : M_opponents )
        {
            if ( p.unum() == unum )
            {
                p.forget();
            }
        }

        dlog.addText( Logger::WORLD,
                      __FILE__" (setCard) opponent %d, card %d",
                      unum, card );
    }
    else
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** (WorldModel::setCard) "
                  << " Illegal side" << std::endl;
        return;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::setPenaltyKickTakerOrder( const std::vector< int > & unum_set )
{
    if ( gameMode().isPenaltyKickMode()
         && ( M_penalty_kick_state->ourTakerCounter() > 0
              && gameMode().type() != GameMode::PenaltySetup_ ) )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << " ***ERROR*** (WorldModel::setPenaltyKickTakerOrder) "
                  << " cannot change the kicker order during penalty kick."
                  << std::endl;
        return;
    }

    M_penalty_kick_state->setKickTakerOrder( unum_set );
}

/*-------------------------------------------------------------------*/
/*!

 */
const
PlayerType *
WorldModel::ourPlayerType( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        return PlayerTypeSet::i().get( Hetero_Default );
    }

    const PlayerType * p = PlayerTypeSet::i().get( ourPlayerTypeId( unum ) );
    if ( ! p )
    {
        p = PlayerTypeSet::i().get( Hetero_Default );
    }
    return p;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
PlayerType *
WorldModel::theirPlayerType( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        return PlayerTypeSet::i().get( Hetero_Unknown );
    }

    const PlayerType * p = PlayerTypeSet::i().get( theirPlayerTypeId( unum ) );
    if ( ! p )
    {
        p = PlayerTypeSet::i().get( Hetero_Unknown );
    }
    return p;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::update( const ActionEffector & act,
                    const GameTime & current )
{
    // this function called from updateAfterSense()
    // or, if player could not receive sense_body,
    // this function will be tried to be called at the top of each update method.

    if ( time() == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current << "internal update called twice ??"
                  << std::endl;
        return;
    }

    M_time = current;

    // playmode is updated in updateJustBeforeDecision

    // the last state is saved as the previous state
    M_prev_ball = M_ball;

    // internal update
    M_self.update( act, current );
    M_ball.update( act, gameMode() );

#ifdef DEBUG_PRINT
    if ( M_ball.rposValid() )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__" (update) internal update. bpos=(%.2f, %.2f)"
                      " brpos=(%.2f, %.2f) bvel=(%.2f, %.2f)",
                      M_ball.pos().x, M_ball.pos().y,
                      M_ball.rpos().x, M_ball.rpos().y,
                      M_ball.vel().x, M_ball.vel().y );
    }
    else
    {
        dlog.addText( Logger::WORLD,
                      __FILE__" (update) internal update. bpos=(%.2f, %.2f)"
                      " bvel=(%.2f, %.2f), invalid rpos",
                      M_ball.pos().x, M_ball.pos().y,
                      M_ball.vel().x, M_ball.vel().y );
    }
#endif

    M_previous_kickable_teammate = false;
    M_previous_kickable_teammate_unum = Unum_Unknown;
    if ( M_kickable_teammate )
    {
        M_previous_kickable_teammate = true;
        M_previous_kickable_teammate_unum = M_kickable_teammate->unum();
    }

    M_previous_kickable_opponent = false;
    M_previous_kickable_opponent_unum = Unum_Unknown;
    if ( M_kickable_opponent )
    {
        M_previous_kickable_opponent = true;
        M_previous_kickable_opponent_unum = M_kickable_opponent->unum();
    }

    M_kickable_teammate = nullptr;
    M_kickable_opponent = nullptr;
    M_maybe_kickable_teammate = nullptr;
    M_maybe_kickable_opponent = nullptr;

    // clear pointer reference container
    M_teammates_from_self.clear();
    M_opponents_from_self.clear();
    M_teammates_from_ball.clear();
    M_opponents_from_ball.clear();

    M_all_players.clear();
    M_our_players.clear();
    M_their_players.clear();

    for ( int i = 0; i < 12; ++i )
    {
        M_our_player_array[i] = nullptr;
        M_their_player_array[i] = nullptr;
    }

    if ( this->gameMode().type() == GameMode::BeforeKickOff
         || ( this->gameMode().type() == GameMode::AfterGoal_
              && this->time().stopped() <= 48 )
         )
    {
        M_teammates.clear();
        M_opponents.clear();
        M_unknown_players.clear();

        PlayerObject::reset_player_count();
    }

    // update teammates
    std::for_each( M_teammates.begin(), M_teammates.end(), PlayerUpdater() );
    M_teammates.remove_if( PlayerValidChecker() );

    // update opponents
    std::for_each( M_opponents.begin(), M_opponents.end(), PlayerUpdater() );
    M_opponents.remove_if( PlayerValidChecker() );

    // update unknown players
    std::for_each( M_unknown_players.begin(), M_unknown_players.end(), PlayerUpdater() );
    M_unknown_players.remove_if( PlayerValidChecker() );

    // update view area

    for ( int i = 0; i < DIR_CONF_DIVS; i++ )
    {
        M_dir_count[i] = std::min( 10, M_dir_count[i] + 1);
        //dlog.addText( Logger::WORLD,
        //            "  world.dirConf: %4.0f -> %d",
        //            (double)i * 360.0 / static_cast<double>(DIR_CONF_DIVS) - 180.0,
        //            M_dir_conf[i] );
    }

    M_view_area_cont.pop_back();
    M_view_area_cont.push_front( ViewArea( current ) );
#ifdef USE_VIEW_GRID_MAP
    M_view_grid_map.incrementAll();
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateAfterSenseBody( const BodySensor & sense_body,
                                  const ActionEffector & act,
                                  const GameTime & current )
{
    if ( M_sense_body_time == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current
                  << " world.updateAfterSense: called twice"
                  << std::endl;
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterSense) called twide" );
        return;
    }

    M_sense_body_time = sense_body.time();

    dlog.addText( Logger::WORLD,
                  "*************** updateAfterSense ***************" );

    if ( sense_body.time() == current )
    {
#ifdef DEBUG_PRINT_SELF_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterSense) update self" );
#endif
        M_self.updateAfterSenseBody( sense_body, act, current );
        M_localize->updateBySenseBody( sense_body );
    }

    M_our_recovery[self().unum() - 1] = self().recovery();
    M_our_stamina_capacity[self().unum() - 1] = self().staminaCapacity();

    M_our_card[self().unum() - 1] = sense_body.card();

    if ( time() != current )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterSense) call internal update" );
        // internal update
        update( act, current );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateBallCollision()
{
    if ( ! ball().posValid()
         || ! ball().velValid()
         || ! self().posValid()
         || ! self().velValid() )
    {

        return;
    }

    if ( ball().velCount() == 0 )
    {
        // already seen the ball velocity directly
        // nothing to do
        return;
    }

    bool collided_with_ball = false;

    if ( self().hasSensedCollision() )
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateBallCollision) agent has sensed collision info" );
#endif
        collided_with_ball = self().collidesWithBall();
        if ( collided_with_ball )
        {
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateBallCollision) detected by sense_body" );
#endif
        }
    }
    else
    {
        // internally updated positions
        const double self_ball_dist
            = ( ball().pos() - self().pos() ).r();

        if ( ( self().collisionEstimated()
               && self_ball_dist < ( self().playerType().playerSize()
                                     + ServerParam::i().ballSize()
                                     + 0.1 )
               )
             || ( ( self().collisionEstimated()
                    || self().vel().r() < ( self().playerType().realSpeedMax()
                                            * self().playerType().playerDecay()
                                            * 0.11 ) )
                  && ( self_ball_dist < ( self().playerType().playerSize()
                                          + ServerParam::i().ballSize()
                                          - 0.2 ) ) )
             )
        {
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateBallCollision) detected. ball_dist= %.3f",
                          self_ball_dist );
#endif
            collided_with_ball = true;
        }
    }

    if ( collided_with_ball )
    {
        if ( ball().posCount() > 0 )
        {
            Vector2D mid = ball().pos() + self().pos();
            mid *= 0.5;

            Vector2D mid2ball = ball().pos() - mid;
            Vector2D mid2self = self().pos() - mid;
            double ave_size = ( ServerParam::i().ballSize()
                                + self().playerType().playerSize() ) * 0.5;
            mid2ball.setLength( ave_size );
            mid2self.setLength( ave_size );

            Vector2D new_ball_pos = mid + mid2ball;
            Vector2D ball_add = new_ball_pos - ball().pos();
            Vector2D new_ball_rpos = ball().rpos() + ball_add;
            Vector2D new_ball_vel = ball().vel() * -0.1;

            M_ball.updateByCollision( new_ball_pos, ball().posCount() + 1,
                                      new_ball_rpos, ball().rposCount() + 1,
                                      new_ball_vel, ball().velCount() + 1 );
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateBallCollision) new bpos(%.2f %.2f) rpos(%.2f %.2f)"
                          " vel(%.2f %.2f)",
                          new_ball_pos.x, new_ball_pos.y,
                          new_ball_rpos.x, new_ball_rpos.y,
                          new_ball_vel.x, new_ball_vel.y );
#endif
            if ( self().posCount() > 0 )
            {
                Vector2D new_my_pos = mid + mid2self;
                double my_add_r = ( new_my_pos - self().pos() ).r();
                Vector2D new_my_pos_error = self().posError();
                new_my_pos_error.x += my_add_r;
                new_my_pos_error.y += my_add_r;

                M_self.updateByCollision( new_my_pos, new_my_pos_error );

#ifdef DEBUG_PRINT_SELF_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (updateBallCollision) new mypos(%.2f %.2f) error(%.2f %.2f)",
                              new_my_pos.x, new_my_pos.y,
                              new_my_pos_error.x, new_my_pos_error.y );
#endif
            }
        }
        else // ball().posCount() == 0
        {
            int vel_count = ( self().hasSensedCollision()
                              ? ball().velCount()
                              : ball().velCount() + 1 );

            M_ball.updateByCollision( ball().pos(), ball().posCount(),
                                      ball().rpos(), ball().rposCount(),
                                      ball().vel() * -0.1, vel_count );
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateBallCollision) seen ball. new_vel=(%.2f %.2f)",
                          ball().vel().x, ball().vel().y );
#endif
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayersCollision()
{
    if ( ! self().pos().isValid()
         || ! self().hasSensedCollision()
         || ! self().collidesWithPlayer() )
    {
        return;
    }

    // dlog.addText( Logger::WORLD,
    //               __FILE__"(updatePlayersCollision) detect collision" );

    for ( PlayerObject & p : M_teammates )
    {
        if ( p.velCount() > 0
             && p.pos().dist2( self().pos() ) < std::pow( self().playerType().playerSize()
                                                          + p.playerTypePtr()->playerSize()
                                                          + 0.15,
                                                          2 ) )
        {
            // dlog.addText( Logger::WORLD,
            //               __FILE__"(updatePlayersCollision) set collision to teammate %d (%.1f %.1f)",
            //               p.unum(), p.pos().x, p.pos().y );
            p.setCollisionEffect();
        }
    }

    for ( PlayerObject & p : M_opponents )
    {
        if ( p.velCount() > 0
             && p.pos().dist2( self().pos() ) < std::pow( self().playerType().playerSize()
                                                          + p.playerTypePtr()->playerSize()
                                                          + 0.15,
                                                          2 ) )
        {
            // dlog.addText( Logger::WORLD,
            //               __FILE__"(updatePlayersCollision) set collision to opponent %d (%.1f %.1f)",
            //               p.unum(), p.pos().x, p.pos().y );
            p.setCollisionEffect();
        }
    }

    for ( PlayerObject & p : M_unknown_players )
    {
        if ( p.velCount() > 0
             && p.pos().dist2( self().pos() ) < std::pow( self().playerType().playerSize()
                                                          + p.playerTypePtr()->playerSize()
                                                          + 0.15,
                                                          2 ) )
        {
            // dlog.addText( Logger::WORLD,
            //               __FILE__"(updatePlayersCollision) set collision to unknown player (%.1f %.1f)",
            //               p.pos().x, p.pos().y );
            p.setCollisionEffect();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateAfterSee( const VisualSensor & see,
                            const BodySensor & sense_body,
                            const ActionEffector & act,
                            const GameTime & current )
{
    //////////////////////////////////////////////////////////////////
    // check internal update time
    if ( time() != current )
    {
        update( act, current );
    }

    //////////////////////////////////////////////////////////////////
    // check last sight update time
    if ( M_see_time == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current << " (updateAfterSee) : called twice "
                  << std::endl;
        return;
    }

#ifdef DEBUG_PROFILE
    Timer timer;
#endif

    //////////////////////////////////////////////////////////////////
    // time update
    M_see_time = current;
    M_see_time_stamp.setNow();

    dlog.addText( Logger::WORLD,
                  "*************** updateAfterSee *****************" );

    //////////////////////////////////////////////////////////////////
    // set opponent teamname
    if ( M_their_team_name.empty()
         && ! see.theirTeamName().empty() )
    {
        M_their_team_name = see.theirTeamName();
    }

    //////////////////////////////////////////////////////////////////
    // already updated by fullstate

    if ( M_fullstate_time == current )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterSee) already updated by fullstate" );
#endif
        // stored info
        ViewArea varea( self().viewWidth().width(),
                        self().pos(),
                        self().face(),
                        current );
        // add to view area history
        M_view_area_cont.front() = varea;

        // check ghost object
        //checkGhost( varea );
        // update field grid map
        //M_view_grid_map.update( varea );
        // update dir accuracy
        updateDirCount( varea );
        return;
    }

    //////////////////////////////////////////////////////////////////
    // self localization
    localizeSelf( see, sense_body, act, current );

    //////////////////////////////////////////////////////////////////
    // ball localization
    localizeBall( see, act, current );

    //////////////////////////////////////////////////////////////////
    // player localization & matching
    localizePlayers( see );
    updatePlayerType();

    //////////////////////////////////////////////////////////////////
    // view cone & ghost check
    // my global position info is successfully updated.
    if ( self().posCount() <= 10
         && self().viewQuality() == ViewQuality::HIGH )
    {
        // stored info
        ViewArea varea( self().viewWidth().width(),
                        self().pos(),
                        self().face(),
                        current );
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterSee) view_area, origin=(%.2f, %.2f) angle=%.1f, width=%.1f vwidth=%d,%.2f",
                      varea.origin().x, varea.origin().y,
                      varea.angle().degree(), varea.viewWidth(),
                      self().viewWidth().type(),
                      self().viewWidth().width() );
#endif
        // add to view area history
        M_view_area_cont.front() = varea;

        // check ghost object
        checkGhost( varea );
        // update field grid map
#ifdef USE_VIEW_GRID_MAP
        M_view_grid_map.update( current, varea );
#endif
        // update dir accuracy
        updateDirCount( varea );
    }

    //////////////////////////////////////////////////////////////////
    // debug output
#ifdef DEBUG_PROFILE
    dlog.addText( Logger::WORLD,
                  __FILE__":(updaterAfterSee) elapsed %f [ms]",
                  timer.elapsedReal() );
#endif
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  "<--- mypos=(%.2f, %.2f) err=(%.3f, %.3f) vel=(%.2f, %.2f)",
                  self().pos().x, self().pos().y,
                  self().posError().x, self().posError().y,
                  self().vel().x, self().vel().y );
    dlog.addText( Logger::WORLD,
                  "<--- seen players t=%d: ut=%d: o=%d: uo=%d: u=%d",
                  see.teammates().size(),
                  see.unknownTeammates().size(),
                  see.opponents().size(),
                  see.unknownOpponents().size(),
                  see.unknownPlayers().size() );
    dlog.addText( Logger::WORLD,
                  "<--- internal players t=%d: o=%d: u=%d",
                  M_teammates.size(),
                  M_opponents.size(),
                  M_unknown_players.size() );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateAfterFullstate( const FullstateSensor & fullstate,
                                  const ActionEffector & act,
                                  const GameTime & current )
{
    // internal update
    if ( time() != current )
    {
        update( act, current );
    }

    if ( M_fullstate_time == current )
    {
        std::cerr << teamName() << " : " << self().unum()
                  << current << " (updateAfterFullstate) called twice "
                  << std::endl;
        return;
    }

    M_fullstate_time = current;

    dlog.addText( Logger::WORLD,
                  "*************** updateAfterFullstate ***************" );

    PlayerObject::reset_player_count();
    M_unknown_players.clear(); // clear unkown players

    // update teammates
    for ( const FullstateSensor::PlayerT & fp : fullstate.ourPlayers() )
    {
        if ( fp.unum_ < 1 || 11 < fp.unum_ )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateAfterFullstate) illegal teammate unum %d",
                          fp.unum_ );
            std::cerr << " (updateAfterFullstate) illegal teammate unum. " << fp.unum_
                      << std::endl;
            continue;
        }

        // #ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterFullstate) teammate %d type=%d card=%s",
                      fp.unum_, fp.type_,
                      fp.card_ == YELLOW ? "yellow" : fp.card_ == RED ? "red" : "no" );
        // #endif

        M_our_player_type[fp.unum_ - 1] = fp.type_;
        M_our_card[fp.unum_ - 1] = fp.card_;

        // update self
        if ( fp.unum_ == self().unum() )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateAfterFullstate) update self" );
#endif
            M_self.updateAfterFullstate( fp, act, current );
            continue;
        }

        // update teammate
        PlayerObject * player = nullptr;
        for ( PlayerObject & t : M_teammates )
        {
            if ( t.unum() == fp.unum_ )
            {
                player = &t;
                break;
            }
        }

        if ( ! player )
        {
            // create new player object
            M_teammates.push_back( PlayerObject() );
            player = &(M_teammates.back());
        }
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterFullstate) updated teammate %d",
                      fp.unum_ );
#endif
        player->updateByFullstate( fp, self().pos(), fullstate.ball().pos_ );
    }

    // update opponents
    for ( const FullstateSensor::PlayerT fp : fullstate.theirPlayers() )
    {
        if ( fp.unum_ < 1 || 11 < fp.unum_ )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateAfterFullstate) illegal opponent unum %d",
                          fp.unum_ );
            std::cerr << " (updateAfterFullstate) illegal opponent unum. " << fp.unum_
                      << std::endl;
            continue;
        }

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterFullstate) teammate %d type=%d card=%s",
                      fp.unum_, fp.type_,
                      fp.card_ == YELLOW ? "yellow" : fp.card_ == RED ? "red" : "no" );
#endif

        M_their_player_type[fp.unum_ - 1] = fp.type_;
        M_their_card[fp.unum_ - 1] = fp.card_;

        PlayerObject * player = nullptr;
        for ( PlayerObject & o : M_opponents )
        {
            if ( o.unum() == fp.unum_ )
            {
                player = &o;
                break;
            }
        }

        if ( ! player )
        {
            M_opponents.push_back( PlayerObject() );
            player = &(M_opponents.back());
        }

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateAfterFullstate) updated opponent %d",
                      fp.unum_ );
#endif
        player->updateByFullstate( fp, self().pos(), fullstate.ball().pos_ );
    }

    // update ball
    M_ball.updateByFullstate( fullstate.ball().pos_,
                              fullstate.ball().vel_,
                              self().pos() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateGameMode( const GameMode & game_mode,
                            const GameTime & current )
{
    bool pk_mode = game_mode.isPenaltyKickMode();

    if ( ! pk_mode
         && game_mode.type() != GameMode::PlayOn ) // not play_on
    {
        // playmode is changed
        if ( gameMode().type() != game_mode.type() )
        {
            // if ( game_mode.type() == GameMode::FreeKick_
            //      && ( gameMode().type() == GameMode::OffSide_
            //           || gameMode().type() == GameMode::FoulCharge_
            //           || gameMode().type() == GameMode::FoulPush_
            //           || gameMode().type() == GameMode::BackPass_
            //           || gameMode().type() == GameMode::FreeKickFault_
            //           || gameMode().type() == GameMode::CatchFault_
            //           || gameMode().type() == GameMode::IndFreeKick_
            //           )
            //      )
            // {
            //     // nothing to do
            // }
            // else
            {
                M_last_set_play_start_time = current;
                M_setplay_count = 0;
            }

            if ( game_mode.type() == GameMode::GoalKick_ )
            {
                M_ball.updateOnlyVel( Vector2D( 0.0, 0.0 ),
                                      Vector2D( 0.0, 0.0 ),
                                      0 );
            }
        }

        // check human referee's interaction
        if ( gameMode().type() == game_mode.type()
             && game_mode.type() == GameMode::FreeKick_ )
        {
            M_last_set_play_start_time = current;
            M_setplay_count = 0;
        }
    }

    if ( game_mode.type() == GameMode::BeforeKickOff )
    {
        int normal_time = ( ServerParam::i().halfTime() > 0
                            && ServerParam::i().nrNormalHalfs() > 0
                            ? ServerParam::i().actualHalfTime() * ServerParam::i().nrNormalHalfs()
                            : 0 );

        if ( current.cycle() < normal_time )
        {
            for ( int i = 0; i < 11; ++i )
            {
                M_our_recovery[i] = 1.0;
                M_our_stamina_capacity[i] = ServerParam::i().staminaCapacity();
            }
        }
        else
        {
            for ( int i = 0; i < 11; ++i )
            {
                M_our_stamina_capacity[i] = ServerParam::i().staminaCapacity();
            }
        }
    }

    M_game_mode = game_mode;

    //
    // update penalty kick status
    //
    if ( pk_mode )
    {
        M_penalty_kick_state->update( game_mode, ourSide(), current );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateBallByHear( const ActionEffector & act )
{
    if ( M_fullstate_time == this->time() )
    {
        return;
    }

    if ( M_audio_memory->ballTime() != this->time()
         || M_audio_memory->ball().empty() )
    {
        return;
    }

    // calculate average positin using all heard info

    Vector2D heard_pos = Vector2D::INVALIDATED;
    Vector2D heard_vel = Vector2D::INVALIDATED;


    double min_dist2 = 1000000.0;
    for ( const AudioMemory::Ball & b : M_audio_memory->ball() )
    {
        const PlayerObject * sender = nullptr;
        for ( const PlayerObject & t : M_teammates )
        {
            if ( t.unum() == b.sender_ )
            {
                sender = &t;
                break;
            }
        }

        if ( sender )
        {
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateBallByHear) sender=%d exists in memory",
                          b.sender_ );
#endif
            double d2 = sender->pos().dist2( ball().pos() );
            if ( d2 < min_dist2 )
            {
                min_dist2 = d2;
                heard_pos = b.pos_;
                if ( b.vel_.isValid() )
                {
                    heard_vel = b.vel_;
                }
            }
        }
        else if ( min_dist2 > 100000.0 )
        {
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateBallByHear) sender=%d, unknown",
                          b.sender_ );
#endif
            min_dist2 = 100000.0;
            heard_pos = b.pos_;
            if ( b.vel_.isValid() )
            {
                heard_vel = b.vel_;
            }
        }

        //heard_pos += b->pos_;
        //heard_vel += b->vel_;
    }

    //heard_pos /= static_cast< double >( M_audio_memory->ball().size() );
    //heard_vel /= static_cast< double >( M_audio_memory->ball().size() );

    if ( heard_pos.isValid() )
    {
        M_ball.updateByHear( act, std::sqrt( min_dist2 ), heard_pos, heard_vel,
                             M_audio_memory->passTime() == this->time() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateGoalieByHear()
{
    if ( M_fullstate_time == this->time() )
    {
        return;
    }

    if ( M_audio_memory->goalieTime() != this->time()
         || M_audio_memory->goalie().empty() )
    {
        return;
    }

    // if ( theirGoalieUnum() == Unum_Unknown )
    // {
    //     return;
    // }

    PlayerObject * goalie = nullptr;

    for( PlayerObject & o : M_opponents )
    {
        if ( o.goalie() )
        {
            goalie = &o;
            break;
        }
    }

    if ( goalie
         && goalie->posCount() == 0
         && goalie->bodyCount() == 0 )
    {
        // goalie is seen at the current time.
#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateGoalieByHear) but already seen" );
#endif
        return;
    }

    Vector2D heard_pos( 0.0, 0.0 );
    double heard_body = 0.0;

    for ( const AudioMemory::Goalie & g : M_audio_memory->goalie() )
    {
        heard_pos += g.pos_;
        heard_body += g.body_.degree();
    }

    heard_pos /= static_cast< double >( M_audio_memory->goalie().size() );
    heard_body /= static_cast< double >( M_audio_memory->goalie().size() );

#ifdef DEBUG_PRINT_PLAYER_UPDATE
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateGoalieByHear) pos=(%.1f %.1f) body=%.1f",
                  heard_pos.x, heard_pos.y,
                  heard_body );
#endif

    if ( goalie )
    {
        goalie->updateByHear( theirSide(),
                              theirGoalieUnum(),
                              true,
                              heard_pos,
                              heard_body );
        return;
    }

    // goalie not found

    // search the nearest unknown player

    const double goalie_speed_max = ServerParam::i().defaultPlayerSpeedMax();

    double min_dist = 1000.0;

    for( PlayerObject & o : M_opponents )
    {
        if ( o.unum() != Unum_Unknown ) continue;

        if ( o.pos().x < ServerParam::i().theirPenaltyAreaLineX()
             || o.pos().absY() > ServerParam::i().penaltyAreaHalfWidth() )
        {
            // out of penalty area
            continue;
        }

        double d = o.pos().dist( heard_pos );
        if ( d < min_dist
             && d < o.posCount() * goalie_speed_max + o.distFromSelf() * 0.06 )
        {
            min_dist = d;
            goalie = &o;
        }
    }

    for ( PlayerObject & u : M_unknown_players )
    {
        if ( u.pos().x < ServerParam::i().theirPenaltyAreaLineX()
             || u.pos().absY() > ServerParam::i().penaltyAreaHalfWidth() )
        {
            // out of penalty area
            continue;
        }

        double d = u.pos().dist( heard_pos );
        if ( d < min_dist
             && d < u.posCount() * goalie_speed_max + u.distFromSelf() * 0.06 )
        {
            min_dist = d;
            goalie = &u;
        }
    }


    if ( goalie )
    {
        // found a candidate unknown player
#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateGoalieByHear) found."
                      " heard_pos=(%.1f %.1f)",
                      heard_pos.x, heard_pos.y );
#endif
        goalie->updateByHear( theirSide(),
                              theirGoalieUnum(),
                              true,
                              heard_pos,
                              heard_body );
    }
    else
    {
        // register new object
#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateGoalieByHear) not found."
                      " add new goalie. heard_pos=(%.1f %.1f)",
                      heard_pos.x, heard_pos.y );
#endif
        M_opponents.push_back( PlayerObject() );
        goalie = &(M_opponents.back());
        goalie->updateByHear( theirSide(),
                              theirGoalieUnum(),
                              true,
                              heard_pos,
                              heard_body );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayerByHear()
{
    if ( M_fullstate_time == this->time() )
    {
        return;
    }

    if ( M_audio_memory->playerTime() != this->time()
         || M_audio_memory->player().empty() )
    {
        return;
    }

    // TODO: consider duplicated player

    for ( const AudioMemory::Player & heard_player : M_audio_memory->player() )
    {
        if ( heard_player.unum_ == Unum_Unknown )
        {
            continue;
        }

        const SideID side = ( heard_player.unum_ <= 11
                              ? ourSide()
                              : theirSide() );
        const int unum = ( heard_player.unum_ <= 11
                           ? heard_player.unum_
                           : heard_player.unum_ - 11 );

        if ( unum < 1 || 11 < unum )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << ": ***ERROR*** (updatePlayerByHear) Illegal unum "
                      << unum
                      << " heard_unum=" << heard_player.unum_
                      << " pos=" << heard_player.pos_
                      << std::endl;
            dlog.addText( Logger::WORLD,
                          __FILE__" (updatePlayerByHear). Illegal unum %d"
                          " pos=(%.1f %.1f)",
                          unum, heard_player.pos_.x, heard_player.pos_.y );
            continue;
        }

        if ( side == ourSide()
             && unum == self().unum() )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updatePlayerByHear) heard myself. skip" );
#endif
            continue;
        }

        PlayerObject * target_player = nullptr;

        PlayerObject::List & players = ( side == ourSide()
                                         ? M_teammates
                                         : M_opponents );
        for ( PlayerObject & p : players )
        {
            if ( p.unum() == unum )
            {
                target_player = &p;
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (updatePlayerByHear) found."
                              " side %s, unum %d",
                              side_str( side ), unum );
#endif
                break;
            }
        }

        PlayerObject::List::iterator unknown = M_unknown_players.end();
        double min_dist = 0.0;
        if ( ! target_player )
        {
            min_dist = 1000.0;
            for  ( PlayerObject & p : players )
            {
                if ( p.unum() != Unum_Unknown
                     && p.unum() != unum )
                {
                    continue;
                }

                double d = p.pos().dist( heard_player.pos_ );
                if ( d < min_dist
                     && d < p.posCount() * 1.2 + p.distFromSelf() * 0.06 )
                {
                    min_dist = d;
                    target_player = &p;
                }
            }

            for ( PlayerObject::List::iterator p = M_unknown_players.begin(), u_end = M_unknown_players.end();
                  p != u_end;
                  ++p )
            {
                double d = p->pos().dist( heard_player.pos_ );
                if ( d < min_dist
                     && d < p->posCount() * 1.2 + p->distFromSelf() * 0.06 )
                {
                    min_dist = d;
                    target_player = &(*p);
                    unknown = p;
                }
            }
        }

        if ( target_player )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updatePlayerByHear) exist candidate."
                          " heard_pos(%.1f %.1f) body=%.1f stamina=%.1f,  memory pos(%.1f %.1f) count %d  dist=%.2f",
                          heard_player.pos_.x,
                          heard_player.pos_.y,
                          heard_player.body_,
                          heard_player.stamina_,
                          target_player->pos().x, target_player->pos().y,
                          target_player->posCount(),
                          min_dist );
#endif
            target_player->updateByHear( side,
                                         unum,
                                         false,
                                         heard_player.pos_,
                                         heard_player.body_ );

            if ( unknown != M_unknown_players.end() )
            {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (updatePlayerByHear) splice unknown player to known player list" );
#endif
                players.splice( players.end(),
                                M_unknown_players,
                                unknown );
            }
        }
        else
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (updatePlayerByHear) not found."
                          " add new player heard_pos(%.1f %.1f) body=%.1f stamina=%.1f",
                          heard_player.pos_.x,
                          heard_player.pos_.y,
                          heard_player.body_,
                          heard_player.stamina_ );
#endif
            if ( side == ourSide() )
            {
                M_teammates.push_back( PlayerObject() );
                target_player = &( M_teammates.back() );
            }
            else
            {
                M_opponents.push_back( PlayerObject() );
                target_player = &( M_opponents.back() );
            }

            target_player->updateByHear( side,
                                         unum,
                                         false,
                                         heard_player.pos_,
                                         heard_player.body_ );
        }

        if ( target_player )
        {
            if ( side == ourSide() )
            {
                if ( 1 <= unum && unum <= 11 )
                {
                    target_player->setPlayerType( M_our_player_type[unum - 1] );
                }
                else
                {
                    target_player->setPlayerType( Hetero_Default );
                }
            }
            else
            {
                if ( 1 <= unum && unum <= 11 )
                {
                    target_player->setPlayerType( M_their_player_type[unum - 1] );
                }
                else
                {
                    target_player->setPlayerType( Hetero_Unknown );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayerStaminaByHear()
{
    // dlog.addText( Logger::WORLD,
    //               "(updatePlayerStaminaByHear) start" );

    if ( M_audio_memory->recoveryTime() == this->time() )
    {
        for ( const AudioMemory::Recovery & v : M_audio_memory->recovery() )
        {
            if ( 1 <= v.sender_ && v.sender_ <= 11 )
            {
                M_our_recovery[v.sender_ - 1] = v.rate_;
                dlog.addText( Logger::WORLD,
                              "(updatePlayerStaminaByHear) unum=%d recovery=%.3f",
                              v.sender_, v.rate_ );
            }
        }
    }

    if ( M_audio_memory->staminaCapacityTime() == this->time() )
    {
        for ( const AudioMemory::StaminaCapacity & v : M_audio_memory->staminaCapacity() )
        {
            if ( 1 <= v.sender_ && v.sender_ <= 11 )
            {
                M_our_stamina_capacity[v.sender_ - 1] = v.rate_ * ServerParam::i().staminaCapacity();
                dlog.addText( Logger::WORLD,
                              "(updatePlayerStaminaByHear) unum=%d capacity=%.2f (rate=%.3f)",
                              v.sender_, M_our_stamina_capacity[v.sender_ - 1], v.rate_ );
            }
        }
    }

#if 0
    for ( int i = 0; i < 11; ++i )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": teammate[%d] stamina capacity=%.2f",
                      i+1, M_our_stamina_capacity[i] );
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateJustBeforeDecision( const ActionEffector & act,
                                      const GameTime & current )
{
    if ( time() != current )
    {
        update( act, current );
    }

    ++M_setplay_count; // always increment

    updateBallByHear( act );
    updateGoalieByHear();
    updatePlayerByHear();
    updatePlayerStaminaByHear();

    updateBallCollision();

    M_ball.updateByGameMode( gameMode() );

    M_ball.updateSelfRelated( self(), prevBall() );
    M_self.updateBallInfo( ball() );

    updatePlayerStateCache();

    updatePlayerCard();
    updatePlayerType(); // have to be called after see message.

    updatePlayersCollision(); // have to be called after player type update.

#if 0
    // 2008-04-18: akiyama
    // set the effect of opponent kickable state to the ball velocity
    if ( ( M_kickable_opponent
           //|| M_kickable_teammate ) // 2012-06-08 added
         )
         && ! self().isKickable() )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateJustBeforeDecision) : exist kickable opponent. ball vel is set to 0." );

        M_ball.setPlayerKickable();
    }
#endif

    updateOurOffenseLine();
    updateOurDefenseLine();
    updateTheirOffenseLine();
    updateTheirDefenseLine();

    updatePlayerLines();

    updateLastKicker();

    updateInterceptTable();

    updateOffsideLine();

    estimateMaybeKickableTeammate();

    M_self.updateKickableState( M_ball,
                                interceptTable().selfStep(),
                                interceptTable().teammateStep(),
                                interceptTable().opponentStep() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateJustAfterDecision( const ActionEffector & act )
{
    M_decision_time = this->time();
    M_decision_time_stamp.setNow();

    if ( act.changeViewCommand() )
    {
        M_self.setViewMode( act.changeViewCommand()->width(),
                            act.changeViewCommand()->quality() );
    }

    if ( act.pointtoCommand() )
    {
        M_self.setPointto( act.getPointtoPos(),
                           time() );
    }

    const PlayerAttentiontoCommand * attentionto = act.attentiontoCommand();
    if ( attentionto )
    {
        if ( attentionto->isOn() )
        {
            if ( attentionto->side() == PlayerAttentiontoCommand::OUR )
            {
                M_self.setAttentionto( ourSide(),
                                       attentionto->number() );
            }
            else
            {
                SideID opp_side = ( ourSide() == LEFT
                                    ? RIGHT
                                    : LEFT );
                M_self.setAttentionto( opp_side,
                                       attentionto->number() );
            }
        }
        else
        {
            // off
            M_self.setAttentionto( NEUTRAL, 0 );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayer( const PlayerObject * player,
                          const SideID side,
                          const int unum,
                          const bool goalie )
{
    if ( side == ourSide() )
    {
        for ( PlayerObject::List::iterator p = M_teammates.begin(), end = M_teammates.end();
              p != end;
              ++p )
        {
            if ( &(*p) == player )
            {
                p->setTeam( side, unum, goalie );
                return;
            }
        }
    }
    else if ( side != NEUTRAL )
    {
        for ( PlayerObject::List::iterator p = M_opponents.begin(), end = M_opponents.end();
              p != end;
              ++p )
        {
            if ( &(*p) == player )
            {
                p->setTeam( side, unum, goalie );
                return;
            }
        }

        for ( PlayerObject::List::iterator p = M_unknown_players.begin(), end = M_unknown_players.end();
              p != end;
              ++p )
        {
            if ( &(*p) == player )
            {
                p->setTeam( side, unum, goalie );
                M_opponents.splice( M_opponents.end(), M_unknown_players, p );
                return;
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
WorldModel::localizeSelf( const VisualSensor & see,
                          const BodySensor & sense_body,
                          const ActionEffector & act,
                          const GameTime & current )
{
    const bool reverse_side = is_reverse_side( *this, *M_penalty_kick_state );

    double angle_face = -360.0;
    double angle_face_error = 0.0;
    Vector2D my_pos( Vector2D::INVALIDATED );
    Vector2D my_pos_error( 0.0, 0.0 );

    // estimate self face angle
    if ( ! M_localize->estimateSelfFace( *this, see, &angle_face, &angle_face_error ) )
    {
        return false;
    }

    //
    // set face angle to ignore coordinate system
    //
    double team_angle_face = ( reverse_side
                               ? AngleDeg::normalize_angle( angle_face + 180.0 )
                               : angle_face );

    //
    // set face dir to self
    //
    M_self.updateAngleBySee( team_angle_face, std::min( angle_face_error, 180.0 ),
                             current );

    // correct vel dir using seen self angle & sense_body's speed magnitude
    M_self.updateVelDirAfterSee( sense_body, current );


    // estimate self position
    if ( ! M_localize->localizeSelf( *this, see, act,
                                     angle_face, angle_face_error,
                                     &my_pos, &my_pos_error ) )
    {
        return false;
    }

    if ( reverse_side )
    {
        my_pos *= -1.0;
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (localizeSelf) reverse=%s face:(seen==%.1f use=%.1f) pos=(%f %f)",
                  ( reverse_side ? "on" : "off" ),
                  angle_face,
                  team_angle_face,
                  my_pos.x, my_pos.y );
#endif
#if 0
    Vector2D my_pos_new = Vector2D::INVALIDATED;
    Vector2D my_pos_error_new( 0.0, 0.0 );

    M_localize->updateParticles( self().lastMove(), current );
    M_localize->localizeSelf2( see,
                               angle_face, angle_face_error,
                               &my_pos_new, &my_pos_error_new,
                               current );
#endif

    if ( my_pos.isValid() )
    {
#ifdef DEBUG_PRINT_SELF_UPDATE
        dlog.addRect( Logger::WORLD,
                      my_pos.x - my_pos_error.x, my_pos.y - my_pos_error.y,
                      my_pos_error.x * 2.0, my_pos_error.y * 2.0,
                      "#ff0000" );
#endif
        M_self.updatePosBySee( my_pos, my_pos_error,
                               team_angle_face, std::min( angle_face_error, 180.0 ),
                               current );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::localizeBall( const VisualSensor & see,
                          const ActionEffector & act,
                          const GameTime & /*current*/ )
{
    if ( ! self().faceValid() )
    {
        //std::cerr << "localizeBall : my face invalid conf= "
        //          << self().faceCount() << std::endl;
        return;
    }

    //////////////////////////////////////////////////////////////////
    // calc relative info

    Vector2D rpos( Vector2D::INVALIDATED );
    Vector2D rpos_error( 0.0, 0.0 );
    Vector2D rvel( Vector2D::INVALIDATED );
    Vector2D vel_error( 0.0, 0.0 );

    if ( ! M_localize->localizeBallRelative( *this, see,
                                             self().face().degree(), self().faceError(),
                                             &rpos, &rpos_error,
                                             &rvel, &vel_error )  )
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBall) localization failed" );
#endif
        return;
    }

    if ( ! rpos.isValid() )
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBall) invalid rpos. cannot calc current seen pos" );
#endif
        return;
    }

    //////////////////////////////////////////////////////////////////
    // Case: invalid self localization
    // to estimate ball global position, self localization is required.
    // in this case, we can estimate only relative info
    if ( ! self().posValid() )
    {
        if ( prevBall().rposCount() == 0
             && see.balls().front().dist_ > self().playerType().playerSize() + ServerParam::i().ballSize() + 0.1
             && self().lastMove().isValid() )
        {
            Vector2D tvel = ( rpos - prevBall().rpos() ) + self().lastMove();
            Vector2D tvel_err = rpos_error + self().velError();
            // set only vel
            tvel *= ServerParam::i().ballDecay();
            tvel_err *= ServerParam::i().ballDecay();
            M_ball.updateOnlyVel( tvel, tvel_err, 1 );

#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizeBall) only vel (%.3f %.3f)",
                          tvel.x, tvel.y );
#endif
        }

        // set relative pos
        M_ball.updateOnlyRelativePos( rpos, rpos_error );

#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBall) only relative pos (%.3f %.3f)",
                      rpos.x, rpos.y );
#endif
        return;
    }

    //////////////////////////////////////////////////////////////////
    // calc global pos & vel using visual

    Vector2D pos = self().pos() + rpos;
    Vector2D pos_error = self().posError() + rpos_error;
    Vector2D gvel( Vector2D::INVALIDATED );
    int vel_count = 1000;


#ifdef DEBUG_PRINT_BALL_UPDATE
    dlog.addRect( Logger::WORLD,
                  pos.x - rpos_error.x, pos.y - rpos_error.y,
                  rpos_error.x * 2.0, rpos_error.y * 2.0,
                  "#ff0000" );
#endif

    if ( rvel.isValid()
         && self().velValid() )
    {
        gvel = self().vel() + rvel;
        vel_error += self().velError();
        vel_count = 0;
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBall) self_vel=(%.3f %.3f) ball_rvel=(%.3f %.3f) r=%.3f th=%.1f gvel=(%.3f %.3f)",
                      self().vel().x, self().vel().y, rvel.x, rvel.y, rvel.r(), rvel.th().degree(), gvel.x, gvel.y );
#endif
    }

    //////////////////////////////////////////////////////////////////
    // calc global velocity using rpos diff (if ball is out of view cone and within vis_dist)

    //if ( ! gvel.isValid() )
    {
        estimateBallVelByPosDiff( see, act, rpos, rpos_error,
                                  gvel, vel_error, vel_count );
    }

    if ( ! gvel.isValid() )
    {
        if ( see.balls().front().dist_ < 2.0
             && prevBall().seenPosCount() == 0
             && prevBall().rposCount() == 0
             && prevBall().rpos().r() < 5.0 )
        {
            gvel = pos - prevBall().pos();
            vel_error += pos_error + prevBall().posError() + prevBall().velError();
            vel_count = 2;
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizeBall) estimate velocity by position diff(1) vel(%.3f %.3f)",
                          gvel.x, gvel.y );
#endif
        }
#if 1
        // add 2013-05-30
        else if ( see.balls().front().dist_ < 2.0
                  && ! self().isKicking()
                  && M_ball.seenPosCount() <= 6
                  && M_ball.seenPosCount() >= 2 // ball is not seen at least 2 or more cycles
                  && self().lastMove( 0 ).isValid() // no collision in this cycle
                  && self().lastMove( 1 ).isValid() ) // no collision in previous cycle
        {
            const Vector2D prev_pos = M_ball.seenPos();
            const int move_step = M_ball.seenPosCount();
            Vector2D ball_move = pos - prev_pos;
            double dist = ball_move.r();
            double speed = ServerParam::i().firstBallSpeed( dist, move_step );
            if ( speed > ServerParam::i().ballSpeedMax() )
            {
                speed = ServerParam::i().ballSpeedMax();
            }
            speed *= std::pow( ServerParam::i().ballDecay(), move_step );

            gvel = ball_move.setLengthVector( speed );
            vel_count = move_step;

#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizeBall) estimate vel by pos diff(2) prev=(%.2f %.2f) move=(%.2f %.2f) dist=%.3f",
                          prev_pos.x, prev_pos.y, ball_move.x, ball_move.y, ball_move.r() );
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizeBall) estimate vel by pos diff(2) vel=(%.3f %.3f) count=%d",
                          gvel.x, gvel.y, vel_count );
#endif
        }
#endif
    }


    //////////////////////////////////////////////////////////////////
    // set data

    if ( gvel.isValid() )
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBall) updateAll. p(%.3f %.3f) rel(%.3f %.3f) v(%.3f %.3f)",
                      pos.x, pos.y, rpos.x, rpos.y, gvel.x, gvel.y );
#endif
        M_ball.updateAll( pos, pos_error, self().posCount(),
                          rpos, rpos_error,
                          gvel, vel_error, vel_count );
    }
    else
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizeBall) updatePos. p(%.3f %.3f) rel(%.3f %.3f)",
                      pos.x, pos.y, rpos.x, rpos.y );
#endif
        M_ball.updatePos( pos, pos_error, self().posCount(),
                          rpos, rpos_error );
    }

#ifdef DEBUG_PRINT_BALL_UPDATE
    dlog.addText( Logger::WORLD,
                  "<--- ball pos=(%.2f, %.2f) err=(%.3f, %.3f)"
                  " rpos=(%.2f, %.2f) rpos_err=(%.3f, %.3f)",
                  ball().pos().x, ball().pos().y,
                  ball().posError().x, ball().posError().y,
                  ball().rpos().x, ball().rpos().y,
                  ball().rposError().x, ball().rposError().y );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::estimateBallVelByPosDiff( const VisualSensor & see,
                                      const ActionEffector & act,
                                      const Vector2D & rpos,
                                      const Vector2D & rpos_error,
                                      Vector2D & vel,
                                      Vector2D & vel_error,
                                      int & vel_count )
{



    if ( self().hasSensedCollision() )
    {
        if ( self().collidesWithPlayer()
             || self().collidesWithPost() )
        {
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (estimateBallVelByPosDiff) canceled by collision.." );
#endif
            return;
        }
    }

    if ( ball().rposCount() == 1 ) // player saw the ball at prev cycle, too.
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (estimateBallVelByPosDiff) update by rpos diff(1)." );
#endif

        if ( see.balls().front().dist_ < 3.15 // ServerParam::i().visibleDistance()
             && prevBall().rpos().isValid()
             && self().velValid()
             && self().lastMove().isValid() )
        {
            Vector2D rpos_diff = rpos - prevBall().rpos();
            Vector2D tmp_vel = rpos_diff + self().lastMove();
            Vector2D tmp_vel_error = rpos_error + self().velError();
            tmp_vel *= ServerParam::i().ballDecay();
            tmp_vel_error *= ServerParam::i().ballDecay();

            // collision
            // if ( self().collidesWithBall() )
            // {
            //     tmp_vel *= -0.1;
            //     tmp_vel_error *= 0.1;
            // }
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          "________ rpos(%.3f %.3f) prev_rpos(%.3f %.3f)",
                          rpos.x, rpos.y,
                          prevBall().rpos().x, prevBall().rpos().y );
            dlog.addText( Logger::WORLD,
                          "________ diff(%.3f %.3f) my_move(%.3f %.3f) -> vel(%.2f, %2f)",
                          rpos_diff.x, rpos_diff.y,
                          self().lastMove().x, self().lastMove().y,
                          tmp_vel.x, tmp_vel.y );

            dlog.addText( Logger::WORLD,
                          "________ internal ball_vel(%.3f %.3f) polar(%.5f %.2f) ",
                          ball().vel().x, ball().vel().y,
                          ball().vel().r(), ball().vel().th().degree() );
            dlog.addText( Logger::WORLD,
                          "________ estimated ball_vel(%.3f %.3f) vel_error(%.5f %.2f) ",
                          tmp_vel.x, tmp_vel.y,
                          tmp_vel_error.x, tmp_vel_error.y );

#endif
            if ( ball().seenVelCount() <= 2
                 && prevBall().rpos().r() > 1.5
                 && see.balls().front().dist_ > 1.5
                 // && ! M_exist_kickable_teammate
                 // && ! M_exist_kickable_opponent
                 // && ( tmp_vel.th() - ball().vel().th() ).abs() < 5.0
                 && std::fabs( tmp_vel.x - ball().vel().x ) < 0.1
                 && std::fabs( tmp_vel.y - ball().vel().y ) < 0.1
                 )
            {
#ifdef DEBUG_PRINT_BALL_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (estimateBallVelByPosDiff) cancel" );
#endif
                return;
            }

#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (estimateBallVelByPosDiff) update" );
#endif
            if ( ! vel.isValid() )
            {
                vel = tmp_vel;
                vel_error = tmp_vel_error;
                vel_count = 1;
            }
            else
            {
                // the player has observed the ball velocity by see message
                if ( ! self().collidesWithBall()
                     && prevBall().rpos().r2() < std::pow( ServerParam::i().visibleDistance() - 0.2, 2 )
                     && tmp_vel.r() * 0.5 < vel.r() ) // if the ball collides with other players, the seen vel would be much smaller.
                {
                    vel = tmp_vel;
                    vel_error = tmp_vel_error;
                    vel_count = 1;
                }
            }
        }
    }
    else if ( ! vel.isValid()
              && ball().rposCount() == 2 )
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (estimateBallVelByPosDiff) update by rpos diff(2)." );
#endif

        if ( see.balls().front().dist_ < 3.15
             && act.lastBodyCommandType() != PlayerCommand::KICK
             && ball().seenRPos().isValid()
             && ball().seenRPos().r() < 3.15
             && self().velValid()
             && self().lastMove( 0 ).isValid()
             && self().lastMove( 1 ).isValid() )
        {
            Vector2D ball_move = rpos - ball().seenRPos();
            ball_move += self().lastMove( 0 );
            ball_move += self().lastMove( 1 );
            vel = ball_move * ( square( ServerParam::i().ballDecay() )
                                / ( 1.0 + ServerParam::i().ballDecay() ) );

            double vel_r = vel.r();
            double estimate_speed = ball().vel().r();

#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (estimateBallVelByPosDiff)"
                          " diff_vel=(%.2f %.2f)%.3f   estimate_vel=(%.2f %.2f)%.3f",
                          vel.x, vel.y, vel_r,
                          ball().vel().x, ball().vel().y, estimate_speed );
#endif

            if ( vel_r > estimate_speed + 0.1
                 || vel_r < estimate_speed * ( 1.0 - ServerParam::i().ballRand() * 2.0 ) - 0.1
                 || ( vel - ball().vel() ).r() > estimate_speed * ServerParam::i().ballRand() * 2.0 + 0.1 )
            {
#ifdef DEBUG_PRINT_BALL_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (estimateBallVelByPosDiff)"
                              " failed to update ball vel using pos diff(2) " );
#endif
                vel.invalidate();
            }
            else
            {
                vel_error = ( rpos_error * 2.0 ) + self().velError();
                vel_error *= ServerParam::i().ballDecay();
                vel_count = 2;

#ifdef DEBUG_PRINT_BALL_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (estimateBallVelByPosDiff)"
                              " cur_rpos(%.2f %.2f) prev_rpos(%.2f %.2f)",
                              rpos.x, rpos.y,
                              ball().seenRPos().x, ball().seenRPos().y );
                dlog.addText( Logger::WORLD,
                              "____ ball_move(%.2f %.2f) my_move0(%.2f %.2f) my_move1(%.2f %.2f)",
                              ball_move.x, ball_move.y,
                              self().lastMove( 0 ).x, self().lastMove( 0 ).y,
                              self().lastMove( 1 ).x, self().lastMove( 1 ).y );
                dlog.addText( Logger::WORLD,
                              "---> vel(%.2f, %2f)",
                              vel.x, vel.y );
#endif
            }

        }
    }
    else if ( ! vel.isValid()
              && ball().rposCount() == 3 )
    {
#ifdef DEBUG_PRINT_BALL_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (estimateBallVelByPosDiff) vel update by rpos diff(3) " );
#endif
        if ( see.balls().front().dist_ < 3.15
             && act.lastBodyCommandType( 0 ) != PlayerCommand::KICK
             && act.lastBodyCommandType( 1 ) != PlayerCommand::KICK
             && ball().seenRPos().isValid()
             && ball().seenRPos().r() < 3.15
             && self().velValid()
             && self().lastMove( 0 ).isValid()
             && self().lastMove( 1 ).isValid()
             && self().lastMove( 2 ).isValid() )
        {
            Vector2D ball_move = rpos - ball().seenRPos();
            ball_move += self().lastMove( 0 );
            ball_move += self().lastMove( 1 );
            ball_move += self().lastMove( 2 );

            vel = ball_move * ( std::pow( ServerParam::i().ballDecay(), 3 )
                                / ( 1.0 + ServerParam::i().ballDecay() + square( ServerParam::i().ballDecay() ) ) );

            double vel_r = vel.r();
            double estimate_speed = ball().vel().r();

#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (estimateBallVelByPosDiff)"
                          " diff_vel=(%.2f %.2f)%.3f   estimate_vel=(%.2f %.2f)%.3f",
                          vel.x, vel.y, vel_r,
                          ball().vel().x, ball().vel().y, estimate_speed );
#endif

            if ( vel_r > estimate_speed + 0.1
                 || vel_r < estimate_speed * ( 1.0 - ServerParam::i().ballRand() * 3.0 ) - 0.1
                 || ( vel - ball().vel() ).r() > estimate_speed * ServerParam::i().ballRand() * 3.0 + 0.1 )
            {
                dlog.addText( Logger::WORLD,
                              "world.localizeBall: .failed to update ball vel using pos diff(2) " );
                vel.invalidate();
            }
            else
            {
                vel_error = ( rpos_error * 3.0 ) + self().velError();
                vel_error *= ServerParam::i().ballDecay();
                vel_count = 3;

#ifdef DEBUG_PRINT_BALL_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (estimateBallVelByPosDiff)"
                              " cur_rpos(%.2f %.2f) prev_rpos(%.2f %.2f)"
                              " ball_move(%.2f %.2f)"
                              " my_move0(%.2f %.2f) my_move1(%.2f %.2f) my_move2(%.2f %.2f)"
                              " -> vel(%.2f, %2f)",
                              rpos.x, rpos.y,
                              ball().seenRPos().x, ball().seenRPos().y,
                              ball_move.x, ball_move.y,
                              self().lastMove( 0 ).x, self().lastMove( 0 ).y,
                              self().lastMove( 1 ).x, self().lastMove( 1 ).y,
                              self().lastMove( 2 ).x, self().lastMove( 2 ).y,
                              vel.x, vel.y );
#endif
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::localizePlayers( const VisualSensor & see )
{
#if 0
    PlayerObjectUpdater updater;
    if ( ! updater.localizePlayers( M_self, see, M_localize,
                                    M_teammates, M_opponents, M_unknown_players ) )
    {
        return;
    }
#else
    if ( ! self().faceValid()
         || ! self().posValid() )
    {
        return;
    }

    ////////////////////////////////////////////////////////////////
    // update policy
    //   for each (seen player objects)
    //       if exist matched player in memory list
    //           -> splice from memory to temporary
    //       else
    //           -> assign new data to temporary list
    //   after loop, copy from temporary to memory again

    // temporary data list
    PlayerObject::List new_teammates;
    PlayerObject::List new_opponents;
    PlayerObject::List new_unknown_players;

    const Vector2D MYPOS = self().pos();
    const Vector2D MYVEL = self().vel();
    const double MY_FACE = self().face().degree();
    const double MY_FACE_ERR = self().faceError();

    //////////////////////////////////////////////////////////////////
    // search order is very important !!
    //   If we replace the unknown player to unknown teammate,
    //   it may cause a mistake for pass target selection.

    // current version search order is
    //   [unum opp -> side opp -> unum mate -> side mate -> unknown]

    // if matched, that player is removed from memory list
    // and copy to temporary

    //////////////////////////////////////////////////////////////////
    // localize, matching and splice from memory list to temporary list

#ifdef DEBUG_PRINT_PLAYER_UPDATE
    dlog.addText( Logger::WORLD,
                  __FILE__" ========== (localizePlayers) ==========" );
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
    dlog.addText( Logger::WORLD,
                  "<<<<< old players start" );
    for ( const PlayerObject & p : M_teammates )
    {
        dlog.addText( Logger::WORLD,
                      "teammate %d (%.2f %.2f)", p.unum(), p.pos().x, p.pos().y );
    }
    for ( const PlayerObject & p : M_opponents )
    {
        dlog.addText( Logger::WORLD,
                      "opponent %d (%.2f %.2f)", p.unum(), p.pos().x, p.pos().y );
    }
    for ( const PlayerObject & p : M_unknown_players )
    {
        dlog.addText( Logger::WORLD,
                      "unknown %d (%.2f %.2f)", p.unum(), p.pos().x, p.pos().y );
    }
    dlog.addText( Logger::WORLD,
                  "<<<<< old players end" );
#endif
#endif

    //
    // opponent (side & unum)
    //
    for ( const VisualSensor::PlayerT & p : see.opponents() )
    {
        Localization::PlayerT player;
        if ( ! M_localize->localizePlayer( *this,
                                           p,
                                           MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                           &player ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          "(localizePlayers) failed opponent %d",
                          player.unum_ );
#endif
            continue;
        }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      "(localizePlayers)"
                      " opponent %d pos=(%.2f, %.2f) vel=(%.2f, %.2f)",
                      player.unum_,
                      player.pos_.x, player.pos_.y,
                      player.vel_.x, player.vel_.y );
#endif
        // matching, splice or create
        checkTeamPlayer( theirSide(),
                         player,
                         M_opponents,
                         M_unknown_players,
                         new_opponents );
    }

    //
    // unknown opponent (no uniform number)
    //
    for ( const VisualSensor::PlayerT & p : see.unknownOpponents() )
    {
        Localization::PlayerT player;
        if ( ! M_localize->localizePlayer( *this,
                                           p,
                                           MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                           &player ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          "(localizePlayers) failed unknown opponent" );
#endif
            continue;
        }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      "(localizePlayers)"
                      " unknown opponent pos=(%.2f, %.2f)",
                      player.pos_.x, player.pos_.y );
#endif
        // matching, splice or create
        checkTeamPlayer( theirSide(),
                         player,
                         M_opponents,
                         M_unknown_players,
                         new_opponents );
    }

    //
    // teammate (side & unum)
    //
    for ( const VisualSensor::PlayerT & p : see.teammates() )
    {
        Localization::PlayerT player;
        if ( ! M_localize->localizePlayer( *this,
                                           p,
                                           MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                           &player ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizePlayers) failed teammate %d",
                          player.unum_ );
#endif
            continue;
        }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      "(localizePlayers)"
                      " teammate %d pos=(%.2f, %.2f) vel=(%.2f, %.2f)",
                      player.unum_,
                      player.pos_.x, player.pos_.y,
                      player.vel_.x, player.vel_.y );
#endif
        // matching, splice or create
        checkTeamPlayer( ourSide(),
                         player,
                         M_teammates,
                         M_unknown_players,
                         new_teammates );
    }

    //
    // unknown teammate (no uniform number)
    //
    for ( const VisualSensor::PlayerT & p : see.unknownTeammates() )
    {
        Localization::PlayerT player;
        if ( ! M_localize->localizePlayer( *this,
                                           p,
                                           MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                           &player ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          "(localizePlayers) failed uunknown teammate" );
#endif
            continue;
        }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      "(localizePlayers)"
                      " unknown teammate pos=(%.2f, %.2f)",
                      player.pos_.x, player.pos_.y );
#endif
        // matching, splice or create
        checkTeamPlayer( ourSide(),
                         player,
                         M_teammates,
                         M_unknown_players,
                         new_teammates );
    }

    //
    // unknown player
    //
    for ( const VisualSensor::PlayerT & p : see.unknownPlayers() )
    {
        Localization::PlayerT player;
        // localize
        if ( ! M_localize->localizePlayer( *this,
                                           p,
                                           MY_FACE, MY_FACE_ERR, MYPOS, MYVEL,
                                           &player ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (localizePlayers) failed unknown player" );
#endif
            continue;
        }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      "(localizePlayers)"
                      " unknown player: pos=(%.2f, %.2f)",
                      player.pos_.x, player.pos_.y );
#endif
        // matching, splice or create
        checkUnknownPlayer( player,
                            M_teammates,
                            M_opponents,
                            M_unknown_players,
                            new_teammates,
                            new_opponents,
                            new_unknown_players );
    }

    //////////////////////////////////////////////////////////////////
    // splice temporary seen players to memory list
    // temporary lists are cleared
    M_teammates.splice( M_teammates.end(),
                        new_teammates );
    M_opponents.splice( M_opponents.end(),
                        new_opponents );
    M_unknown_players.splice( M_unknown_players.end(),
                              new_unknown_players );
#endif

    //////////////////////////////////////////////////////////////////
    // create team member pointer vector for sort

    std::vector< PlayerObject * > all_teammates_ptr;
    std::vector< PlayerObject * > all_opponents_ptr;

    all_teammates_ptr.reserve( M_teammates.size() );
    for ( PlayerObject & p : M_teammates )
    {
        all_teammates_ptr.push_back( &p );
    }

    all_opponents_ptr.reserve( M_opponents.size() );
    for ( PlayerObject & p : M_opponents )
    {
        all_opponents_ptr.push_back( &p );
    }


    /////////////////////////////////////////////////////////////////
    // sort by accuracy count
    std::sort( all_teammates_ptr.begin(),
               all_teammates_ptr.end(),
               PlayerPtrAccuracySorter() );
    std::sort( all_opponents_ptr.begin(),
               all_opponents_ptr.end(),
               PlayerPtrAccuracySorter() );
    M_unknown_players.sort( PlayerCountSorter() );

    //////////////////////////////////////////////////////////////////
    // check the number of players
    // if overflow is detected, player is removed based on confidence value

    // remove from teammates
    int teammate_count = all_teammates_ptr.size();
    while ( teammate_count > 11 - 1 )
    {
        // reset least confidence value player
#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizePlayers)"
                      " erase overflow teammate, %d pos=(%.2f, %.2f)",
                      all_teammates_ptr.back()->unum(),
                      all_teammates_ptr.back()->pos().x,
                      all_teammates_ptr.back()->pos().y );
#endif
        all_teammates_ptr.back()->forget();
        all_teammates_ptr.pop_back();
        --teammate_count;
    }

    // remove from not-teammates
    int opponent_count = all_opponents_ptr.size();
    while ( opponent_count > 11 )
    {
        // reset least confidence value player
#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizePlayers)"
                      " erase overflow opponent, %d pos=(%.2f, %.2f)",
                      all_opponents_ptr.back()->unum(),
                      all_opponents_ptr.back()->pos().x,
                      all_opponents_ptr.back()->pos().y );
#endif
        all_opponents_ptr.back()->forget();
        all_opponents_ptr.pop_back();
        --opponent_count;
    }

    // remove from unknown players
    int unknown_count = M_unknown_players.size();
    int total_count = unknown_count + teammate_count + opponent_count;
    while ( unknown_count > 0
            && total_count > 25 ) //11 * 2 - 1 )
    {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__" (localizePlayers)"
                      " erase over flow unknown player, pos=(%.2f, %.2f)",
                      M_unknown_players.back().pos().x,
                      M_unknown_players.back().pos().y );
#endif
        if ( M_unknown_players.back().posCount() == 0 )
        {
            // not remove !!!
            break;
        }
        // remove least confidence value player
        M_unknown_players.pop_back();
        --unknown_count;
        --total_count;
    }


    //////////////////////////////////////////////////////////////////
    // if overflow is detected, instance player must be forget.
    // that player must be removed from memory list.

    // check invalid player
    // if exist, that player is removed from instance list
    M_teammates.remove_if( PlayerValidChecker() );
    M_opponents.remove_if( PlayerValidChecker() );

    //////////////////////////////////////////////////////////////////
    // it is not necessary to check the all unknown list
    // because invalid unknown player is already removed.


    //////////////////////////////////////////////////////////////////
    // ghost check is done in checkGhost()
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::checkTeamPlayer( const SideID side,
                             const Localization::PlayerT & player,
                             PlayerObject::List & old_known_players,
                             PlayerObject::List & old_unknown_players,
                             PlayerObject::List & new_known_players )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    //  if matched player is found, that player is removed from old list
    //  and updated data is splice to new container
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //

    //////////////////////////////////////////////////////////////////
    // pre check
    // unum is seen -> search the player that has the same uniform number
    if ( player.unum_ != Unum_Unknown )
    {
        // search from old unum known players
        for ( PlayerObject::List::iterator it = old_known_players.begin(), end = old_known_players.end();
              it != end;
              ++it )
        {
            if ( it->unum() == player.unum_ )
            {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                dlog.addText( Logger::WORLD,
                              "(checkTeamPlayer)"
                              " >>> matched!"
                              " unum = %d pos =(%.1f %.1f)",
                              player.unum_, player.pos_.x, player.pos_.y );
#endif
                it->updateBySee( side, player );
                new_known_players.splice( new_known_players.end(),
                                          old_known_players,
                                          it );
                return; // success!!
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    // find nearest player

    const double dash_noise = 1.0 + ServerParam::i().playerRand();
    const double self_error = 0.5 * 2.0;

    double min_team_dist = 10.0 * 10.0;
    double min_unknown_dist = 10.0 * 10.0;

    PlayerObject::List::iterator candidate_team = old_known_players.end();
    PlayerObject::List::iterator candidate_unknown = old_unknown_players.end();

    //////////////////////////////////////////////////////////////////
    // search from old same team players
    for ( PlayerObject::List::iterator it = old_known_players.begin(), end = old_known_players.end();
          it != end;
          ++it )
    {
        if ( player.unum_ != Unum_Unknown
             && it->unum() != Unum_Unknown
             && it->unum() != player.unum_ )
        {
            // unum is seen
            // and it does not match with old player's unum.
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          __FILE__" (checkTeamPlayer)"
                          "___ known player: unum is not match."
                          " seen unum = %d, old_unum = %d",
                          player.unum_, it->unum() );
#endif
            continue;
        }

        int count = it->seenPosCount();
        Vector2D old_pos = it->seenPos();
        double heard_error = 0.0;
        if ( it->heardPosCount() < it->seenPosCount() )
        {
            count = it->heardPosCount();
            old_pos = it->heardPos();
            heard_error = 2.0;
        }

        const double d = player.pos_.dist( old_pos );

        if ( d > ( it->playerTypePtr()->realSpeedMax() * dash_noise * count
                   + heard_error
                   + self_error
                   + player.dist_error_ * 2.0 ) )
        {
            // TODO: inertia movement should be considered.
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkTeamPlayer)"
                          "___ known player: dist over."
                          " dist=%.2f > buf=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          it->playerTypePtr()->realSpeedMax() * dash_noise * count
                          + heard_error
                          + self_error
                          + player.dist_error_ * 2.0,
                          player.pos_.x, player.pos_.y,
                          it->pos().x, it->pos().y );
#endif
            continue;
        }

        if ( d < min_team_dist )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkTeamPlayer)"
                          "___ known player: update."
                          " dist=%.2f < min_team_dist=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          min_team_dist,
                          player.pos_.x, player.pos_.y,
                          it->pos().x, it->pos().y );
#endif
            min_team_dist = d;
            candidate_team = it;
        }
    }

    //////////////////////////////////////////////////////////////////
    // search from unknown players
    for ( PlayerObject::List::iterator it = old_unknown_players.begin(),
              end = old_unknown_players.end();
          it != end;
          ++it )
    {
        int count = it->seenPosCount();
        Vector2D old_pos = it->seenPos();
        double heard_error = 0.0;
        if ( it->heardPosCount() < it->seenPosCount() )
        {
            count = it->heardPosCount();
            old_pos = it->heardPos();
            heard_error = 2.0;
        }

        const double d = player.pos_.dist( old_pos );

        if ( d > ( it->playerTypePtr()->realSpeedMax() * dash_noise * count
                   + heard_error
                   + self_error
                   + player.dist_error_ * 2.0 ) )
        {
            // TODO: inertia movement should be considered.
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkTeamPlayer)"
                          "__ unknown player: dist over. "
                          "dist=%.2f > buf=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          it->playerTypePtr()->realSpeedMax() * dash_noise * count
                          + heard_error
                          + self_error
                          + player.dist_error_ * 2.0,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            continue;
        }

        if ( d < min_unknown_dist )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkTeamPlayer)"
                          "__ unknown player: update. "
                          " dist=%.2f < min_unknown_dist=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          min_unknown_dist,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            min_unknown_dist = d;
            candidate_unknown = it;
        }
    }

    PlayerObject::List::iterator candidate = old_unknown_players.end();
    PlayerObject::List * target_list = nullptr;
#ifdef DEBUG_PRINT_PLAYER_UPDATE
    double min_dist = 1000.0;
#endif

    if ( candidate_team != old_known_players.end()
         && min_team_dist < min_unknown_dist )
    {
        candidate = candidate_team;
        target_list = &old_known_players;

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        min_dist = min_team_dist;
        dlog.addText( Logger::WORLD,
                      "(checkTeamPlayer)"
                      ">>> %d (%.1f %.1f)"
                      " -> %s player %d %s (%.2f, %.2f) dist=%.2f",
                      player.unum_,
                      player.pos_.x, player.pos_.y,
                      side_str( candidate->side() ),
                      candidate->unum(),
                      ( candidate->goalie() ? "goalie" : "field" ),
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
#endif
    }

    if ( candidate_unknown != old_unknown_players.end()
         && min_unknown_dist < min_team_dist )
    {
        candidate = candidate_unknown;
        target_list = &old_unknown_players;

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        min_dist = min_unknown_dist;
        dlog.addText( Logger::WORLD,
                      "(checkTeamPlayer)"
                      ">>> %d (%.1f %.1f)"
                      " -> unknown player (%.2f, %.2f) dist=%.2f",
                      player.unum_,
                      player.pos_.x, player.pos_.y,
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
#endif
    }

    if ( candidate != old_unknown_players.end()
         && target_list )
    {
        // update & splice to new list
        candidate->updateBySee( side, player );

        new_known_players.splice( new_known_players.end(),
                                  *target_list,
                                  candidate );
        return;
    }

    //
    // not found -> generate new player
    //

#ifdef DEBUG_PRINT_PLAYER_UPDATE
    dlog.addText( Logger::WORLD,
                  "(checkTeamPlayer)"
                  " XXX unmatch. min_dist= %.2f"
                  " generate new known player pos=(%.2f, %.2f)",
                  min_dist,
                  player.pos_.x, player.pos_.y );
#endif

    new_known_players.emplace_back( side, player );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::checkUnknownPlayer( const Localization::PlayerT & player,
                                PlayerObject::List & old_teammates,
                                PlayerObject::List & old_opponents,
                                PlayerObject::List & old_unknown_players,
                                PlayerObject::List & new_teammates,
                                PlayerObject::List & new_opponents,
                                PlayerObject::List & new_unknown_players )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
    //  if matched player is found, that player is removed from old list
    //  and updated data is splice to new container
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //

    // matching start
    // search the nearest player

    double min_opponent_dist = 100.0;
    double min_teammate_dist = 100.0;
    double min_unknown_dist = 100.0;

    PlayerObject::List::iterator candidate_opponent = old_opponents.end();
    PlayerObject::List::iterator candidate_teammate = old_teammates.end();
    PlayerObject::List::iterator candidate_unknown = old_unknown_players.end();

    const double dash_noise = 1.0 + ServerParam::i().playerRand();
    const double self_error = 0.5 * 2.0;

    //////////////////////////////////////////////////////////////////
    // search from old opponents
    for ( PlayerObject::List::iterator it = old_opponents.begin(), end = old_opponents.end();
          it != end;
          ++it )
    {
        int count = it->seenPosCount();
        Vector2D old_pos = it->seenPos();
        double heard_error = 0.0;
        if ( it->heardPosCount() < it->seenPosCount() )
        {
            count = it->heardPosCount();
            old_pos = it->heardPos();
            heard_error = 2.0;
        }

        const double d = player.pos_.dist( old_pos );

        if ( d > ( it->playerTypePtr()->realSpeedMax() * dash_noise * count
                   + heard_error
                   + self_error
                   + player.dist_error_ * 2.0 ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkUnknownPlayer)"
                          "__ opp %d: dist over."
                          " dist=%.2f > buf=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          it->unum(),
                          d,
                          it->playerTypePtr()->realSpeedMax() * dash_noise * count
                          + heard_error
                          + self_error
                          + player.dist_error_ * 2.0,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            continue;
        }

        if ( d < min_opponent_dist )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkUnknownPlayer)"
                          "__ opp player: update."
                          " dist=%.2f < min_opp_dist=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          min_opponent_dist,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            min_opponent_dist = d;
            candidate_opponent = it;
        }
    }

    //////////////////////////////////////////////////////////////////
    // search from old teammates
    for ( PlayerObject::List::iterator it = old_teammates.begin(), end = old_teammates.end();
          it != end;
          ++it )
    {
        int count = it->seenPosCount();
        Vector2D old_pos = it->seenPos();
        double heard_error = 0.0;
        if ( it->heardPosCount() <= it->seenPosCount() )
        {
            count = it->heardPosCount();
            old_pos = it->heardPos();
            heard_error = 2.0;
        }

        const double d = player.pos_.dist( old_pos );

        if ( d > ( it->playerTypePtr()->realSpeedMax() * dash_noise * count
                   + heard_error
                   + self_error
                   + player.dist_error_ * 2.0 ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkUnknownPlayer)"
                          "__ our %d: dist over."
                          " dist=%.2f > buf=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          it->unum(),
                          d,
                          it->playerTypePtr()->realSpeedMax() * dash_noise * count
                          + heard_error
                          + self_error
                          + player.dist_error_ * 2.0,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            continue;
        }

        if ( d < min_teammate_dist )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkUnknownPlayer)"
                          "__ our player: update."
                          " dist=%.2f < min_our_dist=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          min_teammate_dist,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            min_teammate_dist = d;
            candidate_teammate = it;
        }
    }

    //////////////////////////////////////////////////////////////////
    // search from old unknown players
    for ( PlayerObject::List::iterator it = old_unknown_players.begin(), end = old_unknown_players.end();
          it != end;
          ++it )
    {
        int count = it->seenPosCount();
        Vector2D old_pos = it->seenPos();
        double heard_error = 0.0;
        if ( it->heardPosCount() < it->seenPosCount() )
        {
            count = it->heardPosCount();
            old_pos = it->heardPos();
            heard_error = 2.0;
        }

        const double d = player.pos_.dist( old_pos );

        if ( d > ( it->playerTypePtr()->realSpeedMax() * dash_noise * count
                   + heard_error
                   + self_error
                   + player.dist_error_ * 2.0 ) )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkUnknownPlayer)"
                          "__ unknown player: dist over."
                          " dist=%.2f > buf=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          it->playerTypePtr()->realSpeedMax() * dash_noise * count
                          + heard_error
                          + self_error
                          + player.dist_error_ * 2.0,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            continue;
        }

        if ( d < min_unknown_dist )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
            dlog.addText( Logger::WORLD,
                          "(checkUnknownPlayer)"
                          "__ unknown player: update."
                          " dist=%.2f < min_unknown_dist=%.2f"
                          " seen_pos(%.1f %.1f) old_pos(%.1f %.1f)",
                          d,
                          min_unknown_dist,
                          player.pos_.x, player.pos_.y,
                          old_pos.x, old_pos.y );
#endif
            min_unknown_dist = d;
            candidate_unknown = it;
        }
    }

    PlayerObject::List::iterator candidate = old_unknown_players.end();;
    PlayerObject::List * new_list = nullptr;
    PlayerObject::List * old_list = nullptr;
    SideID side = NEUTRAL;
#ifdef DEBUG_PRINT_PLAYER_UPDATE
    double min_dist = 1000.0;
#endif

    if ( candidate_teammate != old_teammates.end()
         && min_teammate_dist < min_opponent_dist
         && min_teammate_dist < min_unknown_dist )
    {
        candidate = candidate_teammate;
        new_list = &new_teammates;
        old_list = &old_teammates;
        side = ourSide();

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        min_dist = min_teammate_dist;
        dlog.addText( Logger::WORLD,
                      "(checkUnknownPlayer)"
                      ">>> (%.1f %.1f) -> teammate %d (%.1f %.1f) dist=%.2f",
                      player.pos_.x, player.pos_.y,
                      candidate->unum(),
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
#endif
    }

    if ( candidate_opponent != old_opponents.end()
         //&& min_opponent_dist * 0.5 - 3.0 < min_teammate_dist
         && min_opponent_dist < min_teammate_dist
         && min_opponent_dist < min_unknown_dist )
    {
        candidate = candidate_opponent;
        new_list = &new_opponents;
        old_list = &old_opponents;
        side = theirSide();

#ifdef DEBUG_PRINT_PLAYER_UPDATE
        min_dist = min_opponent_dist;
        dlog.addText( Logger::WORLD,
                      "(checkUnknownPlayer)"
                      ">>> (%.1f %.1f) -> opponent %d (%.1f %.1f) dist=%.2f",
                      player.pos_.x, player.pos_.y,
                      candidate->unum(),
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
#endif
    }

    if ( candidate_unknown != old_unknown_players.end()
         //&& min_unknown_dist * 0.5 - 3.0 < min_teammate_dist
         && min_unknown_dist < min_teammate_dist
         && min_unknown_dist < min_opponent_dist )
    {
        candidate = candidate_unknown;
        new_list = &new_unknown_players;
        old_list = &old_unknown_players;
        side = NEUTRAL;

#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
        min_dist = min_unknown_dist;
        dlog.addText( Logger::WORLD,
                      "(checkUnknownPlayer)"
                      ">>> (%.1f %.1f) -> unknown (%.1f %.1f) dist=%.2f",
                      player.pos_.x, player.pos_.y,
                      candidate->pos().x, candidate->pos().y,
                      min_dist );
#endif
    }


    //////////////////////////////////////////////////////////////////
    // check player movable radius
    if ( candidate != old_unknown_players.end()
         && new_list
         && old_list )
    {
        // update & splice to new list
        candidate->updateBySee( side, player );
        new_list->splice( new_list->end(),
                          *old_list,
                          candidate );
        return;
    }

    //////////////////////////////////////////////////////////////////
    // generate new player
#ifdef DEBUG_PRINT_PLAYER_UPDATE_DETAIL
    dlog.addText( Logger::WORLD,
                  "(checkUnknownPlayer)"
                  " XXX unmatch. dist_error=%f"
                  " generate new unknown player. pos=(%.2f, %.2f)",
                  player.dist_error_,
                  player.pos_.x, player.pos_.y );
#endif

    new_unknown_players.emplace_back( NEUTRAL, player );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayerType()
{
    for ( PlayerObject & p : M_teammates )
    {
        int n = p.unum() - 1;
        if ( 0 <= n && n < 11 )
        {
            p.setPlayerType( M_our_player_type[n] );
        }
        else
        {
            p.setPlayerType( Hetero_Default );
        }
    }

    for ( PlayerObject & p : M_opponents )
    {
        int n = p.unum() - 1;
        if ( 0 <= n && n < 11 )
        {
            p.setPlayerType( M_their_player_type[n] );
        }
        else
        {
            p.setPlayerType( Hetero_Unknown );
        }
    }

    for ( PlayerObject & p : M_unknown_players )
    {
        p.setPlayerType( Hetero_Unknown );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayerCard()
{
    for ( PlayerObject & p : M_teammates )
    {
        int n = p.unum() - 1;
        if ( 0 <= n && n < 11 )
        {
            p.setCard( M_our_card[n] );
        }
    }

    for ( PlayerObject & p : M_opponents )
    {
        int n = p.unum() - 1;
        if ( 0 <= n && n < 11 )
        {
            p.setCard( M_their_card[n] );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::estimateUnknownPlayerUnum()
{
    // 2008-07-03 akiyama
    // estimate unknown player's uniform number
    if ( M_teammates_from_self.size() == 10 )
    {
        std::set< int > unum_set;
        for ( int i = 1; i <= 11; ++i )
        {
            unum_set.insert( i );
        }
        unum_set.erase( self().unum() );

        PlayerObject * unknown_teammate = nullptr;
        for ( PlayerObject & t : M_teammates )
        {
            if ( t.unum() != Unum_Unknown )
            {
                unum_set.erase( t.unum() );
            }
            else
            {
                unknown_teammate = &t;
            }
        }

        if ( unum_set.size() == 1
             && unknown_teammate )
        {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (estimateUnknownPlayerUnum)"
                          " set teammate unum %d (%.1f %.1f)",
                          *unum_set.begin(),
                          unknown_teammate->pos().x, unknown_teammate->pos().y );
#endif
            int unum = *unum_set.begin();
            unknown_teammate->setTeam( ourSide(),
                                       unum,
                                       unum == M_our_goalie_unum );
        }
    }

    if ( M_teammates.size() == 10
         && M_opponents.size() >= 10 )
    {
        std::set< int > unum_set;
        for ( int i = 1; i <= 11; ++i )
        {
            unum_set.insert( i );
        }

        PlayerObject * unknown_opponent = nullptr;
        for ( PlayerObject & o : M_opponents )
        {
            if ( o.unum() != Unum_Unknown )
            {
                unum_set.erase( o.unum() );
            }
            else
            {
                unknown_opponent = &o;
            }
        }

        if ( unum_set.size() == 1 )
        {
            if ( unknown_opponent )
            {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__":(estimateUnknownPlayerUnum)"
                              " set opponent unum %d (%.1f %.1f)",
                              *unum_set.begin(),
                              unknown_opponent->pos().x, unknown_opponent->pos().y );
#endif
                int unum = *unum_set.begin();
                unknown_opponent->setTeam( theirSide(),
                                           unum,
                                           unum == M_their_goalie_unum );
            }
            else // if ( unknown_opponent == NULL )
            {
                if ( M_unknown_players.size() == 1 )
                {
                    int unum = *unum_set.begin();
                    M_unknown_players.begin()->setTeam( theirSide(),
                                                        unum,
                                                        unum == M_their_goalie_unum );
                    M_opponents.splice( M_opponents.end(), M_unknown_players );
                }
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayerStateCache()
{
    if ( ! self().posValid()
         || ! ball().posValid() )
    {
        return;
    }

    //
    // create player reference container
    //
    create_player_set( M_teammates,
                       M_teammates_from_self,
                       M_teammates_from_ball,
                       self().pos(),
                       ball().pos() );
    create_player_set( M_opponents,
                       M_opponents_from_self,
                       M_opponents_from_ball,
                       self().pos(),
                       ball().pos() );
    create_player_set( M_unknown_players,
                       M_opponents_from_self,
                       M_opponents_from_ball,
                       self().pos(),
                       ball().pos() );

    //
    // sort by distance from self or ball
    //
    std::sort( M_teammates_from_self.begin(), M_teammates_from_self.end(), PlayerPtrSelfDistSorter() );
    std::sort( M_opponents_from_self.begin(), M_opponents_from_self.end(), PlayerPtrSelfDistSorter() );

    std::sort( M_teammates_from_ball.begin(), M_teammates_from_ball.end(), PlayerPtrBallDistSorter() );
    std::sort( M_opponents_from_ball.begin(), M_opponents_from_ball.end(), PlayerPtrBallDistSorter() );

    estimateUnknownPlayerUnum();
    estimateGoalie();

    //
    // create known players array
    //
    {
        M_all_players.push_back( &M_self );
        M_our_players.push_back( &M_self );
        M_our_player_array[self().unum()] = &M_self;

        for ( PlayerObject & t : M_teammates )
        {
            M_all_players.push_back( &t );
            M_our_players.push_back( &t );

            if ( t.unum() != Unum_Unknown )
            {
                M_our_player_array[t.unum()] = &t;
            }
        }

        for ( PlayerObject & o : M_opponents )
        {
            M_all_players.push_back( &o );
            M_their_players.push_back( &o );

            if ( o.unum() != Unum_Unknown )
            {
                M_their_player_array[o.unum()] = &o;
            }
        }

    }

    //
    // update kickable player
    //
    updateKickablePlayers();

#ifdef DEBUG_PRINT_PLAYER_UPDATE
    dlog.addText( Logger::WORLD,
                  __FILE__" (updatePlayerStateCache) player set." );
    dlog.addText( Logger::WORLD,
                  " teammatesFromSelf %zd", M_teammates_from_self.size() );
    dlog.addText( Logger::WORLD,
                  " teammatesFromBall %zd", M_teammates_from_ball.size() );
    dlog.addText( Logger::WORLD,
                  " opponentsFromSelf %zd", M_opponents_from_self.size() );
    dlog.addText( Logger::WORLD,
                  " opponentsFromBall %zd", M_opponents_from_ball.size() );

    M_teammates.sort( PlayerUnumSorter() );
    for ( const PlayerObject & p : M_teammates )
    {
        dlog.addText( Logger::WORLD,
                      "teammate id=%d unum=%d (%.1f %.1f) count=%d %s",
                      p.id(), p.unum(), p.pos().x, p.pos().y, p.posCount(),
                      ( p.goalie() ? "goalie" : "" ) );
    }

    M_opponents.sort( PlayerUnumSorter() );
    for ( const PlayerObject & p : M_opponents )
    {
        dlog.addText( Logger::WORLD,
                      "opponent id=%d unum=%d (%.1f %.1f) count=%d %s ",
                      p.id(), p.unum(), p.pos().x, p.pos().y, p.posCount(),
                      ( p.goalie() ? "goalie" : "" ) );
    }

    //M_unknown_players.sort( PlayerCountSorter() );
    for ( const PlayerObject & p : M_unknown_players )
    {
        dlog.addText( Logger::WORLD,
                      "unknown id=%d unum=%d (%.1f %.1f) count=%d %s",
                      p.id(), p.unum(), p.pos().x, p.pos().y, p.posCount(),
                      ( p.goalie() ? "goalie" : "" ) );
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::estimateGoalie()
{
    const AbstractPlayerObject * our_goalie = get_our_goalie_loop( *this );
    const AbstractPlayerObject * their_goalie = get_their_goalie_loop( *this );
#ifdef DEBUG_PRINT_GOALIE_UPDATE
    dlog.addText( Logger::WORLD,
                  __FILE__": (estimateGoalie) our_goalie=%d their_goalie=%d",
                  M_our_goalie_unum, M_their_goalie_unum );
#endif
    //
    // update teammate goalie's unum
    //
    if ( our_goalie
         && our_goalie->unum() != M_our_goalie_unum )
    {
        M_our_goalie_unum = our_goalie->unum();
#ifdef DEBUG_PRINT_GOALIE_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__": (estimateGoalie) update our_goalie=%d",
                      M_our_goalie_unum );
#endif
    }

    //
    // update opponent goalie's unim
    //
    if ( their_goalie
         && their_goalie->unum() != M_their_goalie_unum )
    {
        M_their_goalie_unum = their_goalie->unum();
#ifdef DEBUG_PRINT_GOALIE_UPDATE
        dlog.addText( Logger::WORLD,
                      __FILE__": (estimateGoalie) update their_goalie=%d",
                      M_their_goalie_unum );
#endif
    }

    //
    // never estimate goalies during before-kick-off mode
    //
    if ( gameMode().type() == GameMode::BeforeKickOff
         || gameMode().type() == GameMode::AfterGoal_ )
    {
        return;
    }

    estimateOurGoalie();
    estimateTheirGoalie();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::estimateOurGoalie()
{
    const AbstractPlayerObject * our_goalie = get_our_goalie_loop( *this );

    if ( ! our_goalie
         && M_teammates.size() >= 9 )
    {
        PlayerObject::List::iterator candidate = M_unknown_players.end();
        double min_x = 0.0;
        double second_min_x = 0.0;
        for ( PlayerObject::List::iterator p = M_teammates.begin(), end = M_teammates.end();
              p != end;
              ++p )
        {
            if ( second_min_x > p->pos().x )
            {
                second_min_x = p->pos().x;
                if ( min_x > second_min_x )
                {
                    std::swap( min_x, second_min_x );
                    candidate = p;
                }
            }
        }

        bool from_unknown = false;
        for ( PlayerObject::List::iterator p = M_unknown_players.begin(), end = M_unknown_players.end();
              p != end;
              ++p )
        {
            if ( second_min_x > p->pos().x )
            {
                second_min_x = p->pos().x;
                if ( min_x > second_min_x )
                {
                    std::swap( min_x, second_min_x );
                    candidate = p;
                    from_unknown = true;
                }
            }
        }

        if ( candidate != M_unknown_players.end()
             && second_min_x > min_x + 10.0 )
        {
#ifdef DEBUG_PRINT_GOALIE_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__": (estimateOurGoalie) decide our goalie. %d (%.1f %.1f)",
                          M_our_goalie_unum,
                          candidate->pos().x, candidate->pos().y );
#endif
            candidate->setTeam( ourSide(),
                                M_our_goalie_unum,
                                true );
            if ( from_unknown )
            {
                M_teammates.splice( M_teammates.end(), M_unknown_players, candidate );
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::estimateTheirGoalie()
{
    const AbstractPlayerObject * their_goalie = get_their_goalie_loop( *this );

    if ( ! their_goalie
         && M_teammates.size() >= 10
         && M_opponents_from_self.size() >= 11 )
    {
        PlayerObject::List::iterator candidate = M_unknown_players.end();
        double max_x = 0.0;
        double second_max_x = 0.0;

        for ( PlayerObject::List::iterator p = M_opponents.begin(), end = M_opponents.end();
              p != end;
              ++p )
        {
            if ( second_max_x < p->pos().x )
            {
                second_max_x = p->pos().x;
                if ( max_x < second_max_x )
                {
                    std::swap( max_x, second_max_x );
                    candidate = p;
                }
            }
        }

        bool from_unknown = false;
        for ( PlayerObject::List::iterator p = M_unknown_players.begin(), end = M_unknown_players.end();
              p != end;
              ++p )
        {
            if ( second_max_x < p->pos().x )
            {
                second_max_x = p->pos().x;
                if ( max_x < second_max_x )
                {
                    std::swap( max_x, second_max_x );
                    candidate = p;
                    from_unknown = true;
                }
            }
        }

        if ( candidate != M_unknown_players.end()
             && second_max_x < max_x - 10.0 )
        {
#ifdef DEBUG_PRINT_GOALIE_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__": (estimateTheirGoalie) decide their goalie. %d (%.1f %.1f)",
                          M_their_goalie_unum,
                          candidate->pos().x, candidate->pos().y );
#endif
            candidate->setTeam( theirSide(),
                                M_their_goalie_unum,
                                true );
            if ( from_unknown )
            {
                M_opponents.splice( M_opponents.end(), M_unknown_players, candidate );
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::estimateMaybeKickableTeammate()
{
    static GameTime s_update_time( -1, 0 );
    static int s_previous_teammate_step = 1000;
    static GameTime s_previous_time( -1, 0 );

    if ( s_update_time == this->time() )
    {
        return;
    }
    s_update_time = this->time();

    M_maybe_kickable_teammate = nullptr;

    if ( this->kickableTeammate() )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__":(estimateMaybeKickableTeammate) exist normal" );
        s_previous_teammate_step = 0;
        s_previous_time = this->time();
        M_maybe_kickable_teammate = this->kickableTeammate();
        return;
    }

    if ( s_previous_time.stopped() == 0
         && s_previous_time.cycle() + 1 == this->time().cycle()
         && s_previous_teammate_step <= 1
         && ! this->teammatesFromBall().empty() )
    {
        const PlayerObject * t =  this->teammatesFromBall().front();

        if ( this->audioMemory().passTime() == this->time()
             && ! this->audioMemory().pass().empty()
             && this->audioMemory().pass().front().sender_ == t->unum() )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__":(estimateMaybeKickableTeammate) heard pass kick" );
            s_previous_teammate_step = this->interceptTable().teammateStep();
            s_previous_time = this->time();
            M_maybe_kickable_teammate = nullptr;
            return;
        }

        if ( t->distFromBall() < ( t->playerTypePtr()->kickableArea()
                                   + t->distFromSelf() * 0.05
                                   + this->ball().distFromSelf() * 0.05 ) )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__":(estimateMaybeKickableTeammate) found" );
            s_previous_teammate_step = 1; //this->interceptTable().teammateStep();
            s_previous_time = this->time();
            M_maybe_kickable_teammate = t;
            return;
        }
    }

    s_previous_teammate_step = this->interceptTable().teammateStep();
    s_previous_time = this->time();

    dlog.addText( Logger::WORLD,
                  __FILE__":(estimateMaybeKickableTeammate) not found" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateKickablePlayers()
{
    //
    // estimate teammate kickable state
    //
    for ( const PlayerObject * p : M_teammates_from_ball )
    {
        if ( p->isGhost()
             || p->isTackling()
             || p->posCount() > ball().posCount() )
        {
            continue;
        }

        if ( p->isKickable( 0.0 ) )
        {
            M_kickable_teammate = p;
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateKickablePlayers) found teammate %d (%.1f %.1f)",
                          p->unum(), p->pos().x, p->pos().y );
            break;
        }
    }

    //
    // estimate opponent kickable state
    //
    for ( const PlayerObject * p : M_opponents_from_ball )
    {
        if ( p->isGhost()
             || p->isTackling()
             || p->posCount() >= 10 ) //ball().posCount()
        {
            continue;
        }

        if ( p->distFromBall() > 5.0 ) // magic number
        {
            break;
        }

        double buf = 0.0;

        if ( ! M_maybe_kickable_opponent )
        {
            buf = std::min( 1.0,
                            p->distFromSelf() * 0.05 + ball().distFromSelf() * 0.05 );

            if ( p->isKickable( -buf ) )
            {
                M_maybe_kickable_opponent = p;
                dlog.addText( Logger::WORLD,
                              __FILE__" (updateKickablePlayers) maybe opponent %d (%.1f %.1f)",
                              p->unum(), p->pos().x, p->pos().y );
            }
        }

        buf = std::min( 0.5,
                        p->distFromSelf() * 0.02 + ball().distFromSelf() * 0.02 );

        if ( p->isKickable( -buf ) )
        {
            M_kickable_opponent = p;
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateKickablePlayers) found opponent %d (%.1f %.1f)",
                          p->unum(), p->pos().x, p->pos().y );
            break;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateOffsideLine()
{
    if ( ! ServerParam::i().useOffside() )
    {
        M_offside_line_count = 0;
        M_offside_line_x = M_prev_offside_line_x = ServerParam::i().pitchHalfLength();
        return;
    }

    if ( gameMode().type() == GameMode::KickIn_
         || gameMode().type() == GameMode::CornerKick_
         || ( gameMode().type() == GameMode::GoalKick_
              && gameMode().side() == ourSide() )
         )
    {
        M_offside_line_count = 0;
        M_offside_line_x = M_prev_offside_line_x = ServerParam::i().pitchHalfLength();
        return;
    }

    if ( gameMode().side() != ourSide()
         && ( gameMode().type() == GameMode::GoalieCatch_
              || gameMode().type() == GameMode::GoalKick_ )
         )
    {
        M_offside_line_count = 0;
        M_offside_line_x = M_prev_offside_line_x = ServerParam::i().pitchHalfLength();
        return;
    }

    double new_line = M_their_defense_line_x;
    int count = M_their_defense_line_count;

#if 1
    // add 2013-06-18
    Vector2D ball_pos = ball().inertiaPoint( std::min( {
                interceptTable().selfStep(),
                interceptTable().teammateStep(),
                interceptTable().opponentStep() } ) );
    if ( ball_pos.x > new_line )
    {
        new_line = ball_pos.x;
        count = ball().posCount();
    }

#endif

    if ( M_audio_memory->offsideLineTime() == this->time()
         && ! M_audio_memory->offsideLine().empty() )
    {
        double heard_x = 0.0;
        for ( const AudioMemory::OffsideLine & v : M_audio_memory->offsideLine() )
        {
            heard_x += v.x_;
        }
        heard_x /= static_cast< double >( M_audio_memory->offsideLine().size() );

        if ( new_line < heard_x - 1.0 )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateOffsideLine) by heard info. %.1f -> %.1f",
                          new_line, heard_x );
#endif

            new_line = heard_x;
            count = 30;
        }
    }

    M_prev_offside_line_x = M_offside_line_x;
    M_offside_line_x = new_line;
    M_offside_line_count = count;

#ifdef DEBUG_PRINT_LINES
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateOffsideLine) prev=%.2f x=%.2f count=%d",
                  M_prev_offside_line_x, new_line, count );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateOurOffenseLine()
{
    double new_line = -ServerParam::i().pitchHalfLength();

    for ( const AbstractPlayerObject * p : ourPlayers() )
    {
        new_line = std::max( new_line, p->pos().x );
    }

    if ( ourPlayers().empty() )
    {
        // new_line is used
    }
    else if ( ourPlayers().size() >= 11 )
    {
        // new_line is used
    }
    else if ( new_line < M_our_offense_line_x - 13.0 )
    {
        // new_line is used
    }
    else if ( new_line < M_our_offense_line_x - 5.0 )
    {
        new_line = M_our_offense_line_x - 1.0;
    }

    if ( ball().posValid()
         && ball().pos().x > new_line )
    {
        new_line = ball().pos().x;
    }

    M_our_offense_line_x = new_line;

#ifdef DEBUG_PRINT_LINES
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateOurOffenseLine) x=%.2f",
                  new_line );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateOurDefenseLine()
{
    double first = 0.0, second = 0.0;
    for ( const AbstractPlayerObject * p : ourPlayers() )
    {
        double x = p->pos().x;
        if ( x < second )
        {
            second = x;
            if ( second < first )
            {
                std::swap( first, second );
            }
        }
    }

    double new_line = second;

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateDefenseLine) base line=%.1f",
                  new_line );
#endif

#if 0
    const AbstractPlayerObject * goalie = getOurGoalie();
    if ( ! goalie )
    {
        if ( first > ServerParam::i().ourPenaltyAreaLineX() )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateDefenseLine) goalie not found,"
                          " assume goalie is back of defense line. %.1f -> %.1f",
                          new_line, first );
#endif
            new_line = first;
        }
    }
#endif

    // consider old line
    if ( ourPlayers().size() >= 11 )
    {
        // new_line is used
    }
    else if ( new_line > M_our_defense_line_x + 13.0 )
    {
        // new_line is used
    }
    else if ( new_line > M_our_defense_line_x + 5.0 )
    {
        new_line = M_our_defense_line_x + 1.0;
    }

    // ball exists on behind of our defense line
    if ( ball().posValid()
         && ball().pos().x < new_line )
    {
        new_line = ball().pos().x;
    }

    if ( M_audio_memory->defenseLineTime() == this->time()
         && ! M_audio_memory->defenseLine().empty() )
    {
        double heard_x = 0.0;
        for ( const AudioMemory::DefenseLine & v : M_audio_memory->defenseLine() )
        {
            heard_x += v.x_;
        }
        heard_x /= static_cast< double >( M_audio_memory->defenseLine().size() );

        if ( heard_x + 1.0 < new_line )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateDefenseLine) heard defense line is used. %.1f -> %.1f",
                          new_line, heard_x );
#endif

            new_line = heard_x;
        }
    }

    M_our_defense_line_x = new_line;

#ifdef DEBUG_PRINT_LINES
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateOurDefenseLine) %.2f",
                  new_line );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateTheirOffenseLine()
{
    double new_line = ServerParam::i().pitchHalfLength();
    for ( const AbstractPlayerObject * p : theirPlayers() )
    {
        new_line = std::min( new_line, p->pos().x );
    }

    // consider old line
    if ( theirPlayers().size() >= 11 )
    {
        // new_line is used
    }
    else if ( new_line > M_their_offense_line_x + 13.0 )
    {
        // new_line is used
    }
    else if ( new_line > M_their_offense_line_x + 5.0 )
    {
        new_line = M_their_offense_line_x + 1.0;
    }

    if ( ball().posValid()
         && ball().pos().x < new_line )
    {
        new_line = ball().pos().x;
    }

    M_their_offense_line_x = new_line;

#ifdef DEBUG_PRINT_LINES
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateTheirOffenseLine) x=%.2f",
                  new_line );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateTheirDefenseLine()
{
    double first_x = 0.0, second_x = 0.0;
    int first_count = 1000, second_count = 1000;

    const PlayerObject * first_player = nullptr;
    const PlayerObject * second_player = nullptr;

    for ( const PlayerObject * p : M_opponents_from_self )
    {
        // 2015-07-14
        // 2023-06-24
        double player_x = p->pos().x;
        if ( p->posCount() > 0
             && player_x > ball().pos().x + 3.0 )
        {
            const PlayerType * ptype = p->playerTypePtr();
#if 1
            Vector2D opponent_pos = p->pos();
            Vector2D opponent_vel = p->vel();
            Vector2D accel_unit = ( p->bodyCount() <= 3
                                    ? Vector2D::from_polar( 1.0, p->body() )
                                    : Vector2D( -1.0, 0.0 ) );
            const int max_count = std::min( 3, p->posCount() );
            // dlog.addText( Logger::WORLD,
            //               "(updateTheirDefenseLine) opponent=%d accel_unit=(%.3f %.3f) max_count=%d pos=(%.1f %.1f)",
            //               p->unum(), accel_unit.x, accel_unit.y, max_count, opponent_pos.x, opponent_pos.y );
            for ( int i = 0; i < max_count; ++i )
            {
                if ( i == 0
                     && p->bodyCount() <= 3
                     && accel_unit.th().abs() < 160.0 )
                {
                    // turn
                    opponent_pos += opponent_vel;
                    opponent_vel *= ptype->playerDecay();
                    accel_unit.assign( -1.0, 0.0 );
                    continue;
                }
                opponent_vel += accel_unit * ( 0.7 * ( ServerParam::i().maxDashPower() * ptype->dashRate( ptype->effortMax() ) ) );
                opponent_pos += opponent_vel;
                // dlog.addText( Logger::WORLD,
                //               "(updateTheirDefenseLine) opponent=%d accel_unit=(%.3f %.3f) loop=%d pos=(%.1f %.1f) vel=(%.2f %.2f)",
                //               p->unum(), accel_unit.x, accel_unit.y, i, opponent_pos.x, opponent_pos.y,
                //               opponent_vel.x, opponent_vel.y );
                opponent_vel *= ptype->playerDecay();
            }
            player_x = opponent_pos.x;
            dlog.addText( Logger::WORLD,
                          "(updateTheirDefenseLine) opponent=%d world_x=%.1f predict_x=%.1f",
                          p->unum(), p->pos().x, player_x );
#else
            double rate = 0.1;
            if ( p->vel().x < -ptype->realSpeedMax()*ptype->playerDecay() * 0.8
                 || ball().pos().x > 25.0 )
            {
                rate = 0.8;
            }
            // dlog.addText( Logger::WORLD,
            //               "(updateTheirDefenseLine) %d rate=%.1f",
            //               p->unum(), rate );
            double adjust = rate * ptype->realSpeedMax() * std::min( 3, p->posCount() );
            // dlog.addText( Logger::WORLD,
            //               "(updateTheirDefenseLine) %d x=%.1f adjust=%.1f",
            //               (*it)->unum(), x, adjust );
            player_x -= adjust;
#endif
        }

        if ( player_x > second_x )
        {
            second_x = player_x;
            second_count = p->posCount();
            second_player = p;
            if ( second_x > first_x )
            {
                std::swap( first_x, second_x );
                std::swap( first_count, second_count );
                std::swap( first_player, second_player );
            }
        }
    }

    double new_line = second_x;
    int count = second_count;

    // dlog.addText( Logger::WORLD,
    //               "(updateTheirDefenseLine) new_line=%.1f", new_line );

    const AbstractPlayerObject * goalie = getTheirGoalie();
    if ( ! goalie )
    {
        if ( 20.0 < ball().pos().x
             && ball().pos().x < ServerParam::i().theirPenaltyAreaLineX() )
        {
            if ( first_x < ServerParam::i().theirPenaltyAreaLineX() )
            {
                // dlog.addText( Logger::WORLD,
                //               "(updateTheirDefenseLine) no goalie. %.1f -> %.1f",
                //               second, first );
                new_line = first_x;
                count = 30;
            }
        }
    }

    if ( M_opponents_from_self.size() >= 11 )
    {
        // new_line is used directly
    }
    else if ( new_line < M_their_defense_line_x - 13.0 )
    {
        // new_line is used directly
    }
    else if ( new_line < M_their_defense_line_x - 5.0 )
    {
        new_line = M_their_defense_line_x - 1.0;
    }

    if ( new_line < 0.0 )
    {
        new_line = 0.0;
    }

    // ball is more forward than opponent defender line
    if ( gameMode().type() != GameMode::BeforeKickOff
         && gameMode().type() != GameMode::AfterGoal_
         && ball().posCount() <= 3 )
    {
        Vector2D ball_next = ball().pos() + ball().vel();
        if ( ball_next.x > new_line )
        {
            new_line = ball_next.x;
            count = ball().posCount();
        }
    }

    M_their_defense_line_x = new_line;
    M_their_defense_line_count = count;

    //#ifdef DEBUG_PRINT_LINES
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateTheirDefenseLine) x=%.2f count=%d",
                  new_line, count );
    //#endif
}


/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updatePlayerLines()
{
    const ServerParam & SP = ServerParam::i();
    {
        double max_x = -SP.pitchHalfLength();
        double min_x = +SP.pitchHalfLength();
        double second_min_x = +SP.pitchHalfLength();

        for ( const AbstractPlayerObject * p : ourPlayers() )
        {
            double x = p->pos().x;

            if ( x > max_x )
            {
                max_x = x;
            }

            if ( x < second_min_x )
            {
                second_min_x = x;
                if ( second_min_x < min_x )
                {
                    std::swap( min_x, second_min_x );
                }
            }
        }

        M_our_offense_player_line_x = max_x;
        M_our_defense_player_line_x = second_min_x;

        const AbstractPlayerObject * goalie = getOurGoalie();
        if ( ! goalie )
        {
            if ( min_x > SP.ourPenaltyAreaLineX() )
            {
                M_our_defense_player_line_x = min_x;
            }
        }
#ifdef DEBUG_PRINT_LINES
        dlog.addText( Logger::WORLD,
                      __FILE__" (updatePlayerLines) our_offensex=%.2f our_defense=%.2f",
                      M_our_offense_player_line_x,
                      M_our_defense_player_line_x );
#endif
    }

    {
        double min_x = +SP.pitchHalfLength();
        double max_x = -SP.pitchHalfLength();
        double second_max_x = -SP.pitchHalfLength();

        for ( const AbstractPlayerObject * p : theirPlayers() )
        {
            double x = p->pos().x;

            if ( x < min_x )
            {
                min_x = x;
            }

            if ( x > second_max_x )
            {
                second_max_x = x;
                if ( second_max_x > max_x )
                {
                    std::swap( max_x, second_max_x );
                }
            }
        }

        M_their_offense_player_line_x = min_x;
        M_their_defense_player_line_x = second_max_x;

        const AbstractPlayerObject * goalie = getTheirGoalie();
        if ( ! goalie )
        {
            if ( max_x < SP.theirPenaltyAreaLineX() )
            {
                M_their_defense_player_line_x = max_x;
            }
        }
#ifdef DEBUG_PRINT_LINES
        dlog.addText( Logger::WORLD,
                      __FILE__" (updatePlayerLines) their_offensex=%.2f their_defense=%.2f",
                      M_their_offense_player_line_x,
                      M_their_defense_player_line_x );
#endif
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateLastKicker()
{
    // if ( ! self().goalie() )
    // {
    //     return;
    // }

    if ( gameMode().type() != GameMode::PlayOn )
    {
        if ( gameMode().isOurSetPlay( ourSide() ) )
        {
            M_last_kicker_side = ourSide();
            M_last_kicker_unum = ( teammatesFromBall().empty()
                                   ? Unum_Unknown
                                   : teammatesFromBall().front()->unum() );
        }
        else if ( gameMode().isTheirSetPlay( ourSide() ) )
        {
            M_last_kicker_side = theirSide();
            M_last_kicker_unum = ( opponentsFromBall().empty()
                                   ? Unum_Unknown
                                   : opponentsFromBall().front()->unum() );
        }
        else
        {
            M_last_kicker_side = NEUTRAL;
            M_last_kicker_unum = Unum_Unknown;
        }

        return;
    }

    if ( self().isKicking() )
    {
        M_last_kicker_side = ourSide();
        M_last_kicker_unum = self().unum();
#ifdef DEBUG_PRINT_LAST_KICKER
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateLastKicker) self kicked" );
#endif
        return;
    }

    if ( ! prevBall().vel().isValid() )
    {
#ifdef DEBUG_PRINT_LAST_KICKER
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateLastKicker) no previous ball data" );
#endif
        return;
    }

    const ServerParam & SP = ServerParam::i();

    //
    // check seen kicker or tackler
    //

    AbstractPlayerObject::Cont kickers;

    for ( int i = 0; i < 2; ++i )
    {
        const PlayerObject::Cont & players = ( i == 0
                                               ? teammatesFromBall()
                                               : opponentsFromBall() );

        for ( const PlayerObject * p : players )
        {
            if ( p->isKicking()
                 && p->distFromBall() < SP.ballSpeedMax() * 2.0 )
            {
                kickers.push_back( p );
#ifdef DEBUG_PRINT_LAST_KICKER
                dlog.addText( Logger::WORLD,
                              __FILE__" (updateLastKicker) see kicking side=%c unum=%d",
                              ( p->side() == LEFT ? 'L' : p->side() == RIGHT ? 'R' : 'N' ),
                              p->unum() );
#endif
            }
            else if ( p->tackleCount() == 0
                      && p->distFromBall() < SP.ballSpeedMax() * 2.0 )
            {
                kickers.push_back( p );
#ifdef DEBUG_PRINT_LAST_KICKER
                dlog.addText( Logger::WORLD,
                              __FILE__" (updateLastKicker) see tackling side=%c unum=%d",
                              ( p->side() == LEFT ? 'L' : p->side() == RIGHT ? 'R' : 'N' ),
                              p->unum() );
#endif
            }
        }
    }

    //
    // check ball velocity change
    //

    double angle_diff = ( ball().vel().th() - prevBall().vel().th() ).abs();
    double prev_speed = prevBall().vel().r();
    double cur_speed = ball().vel().r();

    bool ball_vel_changed = false;
    if ( cur_speed > prev_speed + 0.1
         || cur_speed < prev_speed * SP.ballDecay() * 0.5
         || ( prev_speed > 0.5         // Magic Number
              && angle_diff > 20.0 ) ) // Magic Number
    {
        ball_vel_changed = true;
#ifdef DEBUG_PRINT_LAST_KICKER
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateLastKicker) ball vel changed." );
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateLastKicker) speed=%.3f prev_seed=%.3f angle_diff=%.1f",
                      cur_speed, prev_speed, angle_diff );
#endif
    }

    //
    // check kicker
    //

    if ( ball_vel_changed
         && ! kickers.empty() )
    {
        if ( kickers.size() == 1 )
        {
            const AbstractPlayerObject * kicker = kickers.front();
            if ( kicker->distFromBall() < SP.ballSpeedMax() * 2.0 )
            {
                if ( kicker->side() != theirSide() )
                {
                    M_last_kicker_side = ourSide();
                    M_last_kicker_unum = kicker->unum();
#ifdef DEBUG_PRINT_LAST_KICKER
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (updateLastKicker) set by 1 seen kicker. side=%d unum=%d -> teammate",
                                  ( kicker->side() == LEFT ? 'L' : kicker->side() == RIGHT ? 'R' : 'N' ),
                                  kicker->unum() );
#endif
                }
                else
                {
                    M_last_kicker_side = theirSide();
                    M_last_kicker_unum = kicker->unum();
#ifdef DEBUG_PRINT_LAST_KICKER
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (updateLastKicker) set by 1 seen kicker. side=%d unum=%d -> opponent",
                                  kicker->side(),
                                  kicker->unum() );
#endif
                }
                return;
            }
        }

        bool exist_teammate_kicker = false;
        int teammate_kicker_unum = Unum_Unknown;
        bool exist_opponent_kicker = false;
        int opponent_kicker_unum = Unum_Unknown;
        for ( const AbstractPlayerObject * p : kickers )
        {
            if ( p->distFromBall() > SP.ballSpeedMax() * 2.0 )
            {
                continue;
            }

            if ( p->side() == ourSide() )
            {
                // teammate
                exist_teammate_kicker = true;
                teammate_kicker_unum = p->unum();
            }
            else
            {
                // opponent
                exist_opponent_kicker = true;
                opponent_kicker_unum = p->unum();
            }
        }

        if ( exist_teammate_kicker
             && exist_opponent_kicker )
        {
            M_last_kicker_side = NEUTRAL;
            M_last_kicker_unum = Unum_Unknown;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by seen kicker(s). NEUTRAL"
                          " kicked by teammate and opponent" );
#endif
        }
        else if ( ! exist_opponent_kicker )
        {
            M_last_kicker_side = ourSide();
            M_last_kicker_unum = teammate_kicker_unum;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by seen kicker(s). TEAMMATE"
                          " kicked by teammate or unknown" );
#endif
        }
        else if ( ! exist_teammate_kicker )
        {
            M_last_kicker_side = theirSide();
            M_last_kicker_unum = opponent_kicker_unum;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by seen kicker(s). OPPONENT"
                          " kicked by opponent" );
#endif
        }

        return;
    }

    if ( ball_vel_changed )
    {
        if ( M_previous_kickable_teammate
             && ! M_previous_kickable_opponent )
        {
            M_last_kicker_side = ourSide();
            M_last_kicker_unum = M_previous_kickable_teammate_unum;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by prev teammate kicker %d",
                          M_previous_kickable_teammate_unum );
#endif
            return;
        }
        else if ( ! M_previous_kickable_teammate
                  && M_previous_kickable_opponent )
        {
            M_last_kicker_side = theirSide();
            M_last_kicker_unum = M_previous_kickable_opponent_unum;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by prev opponent kicker %d",
                          M_previous_kickable_opponent_unum );
#endif
            return;
        }
        else if ( M_previous_kickable_teammate
                  && M_previous_kickable_opponent )
        {
            M_last_kicker_side = NEUTRAL;
            M_last_kicker_unum = Unum_Unknown;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) both side kickable in previous cycle. NEUTRAL" );
#endif
            return;
        }
    }

    //
    // check ball nearest player
    //
    if ( ball_vel_changed )
    {
        const double dist_thr2 = std::pow( SP.ballSpeedMax(), 2 );
        bool exist_teammate_kicker = false;
        bool exist_opponent_kicker = false;

        const AbstractPlayerObject * nearest = nullptr;
        double min_dist = std::numeric_limits< double >::max();
        double second_min_dist = std::numeric_limits< double >::max();

        for ( const AbstractPlayerObject * p : allPlayers() )
        {
            if ( p->side() == ourSide()
                 && p->unum() == self().unum() )
            {
                continue;
            }

            double d2 = p->pos().dist2( prevBall().pos() );
            if ( d2 < dist_thr2 )
            {
                if ( p->side() != theirSide() )
                {
                    exist_teammate_kicker = true;
                }
                else
                {
                    exist_opponent_kicker = true;
                }
            }

            if ( d2 < second_min_dist )
            {
                second_min_dist = d2;
                if ( second_min_dist < min_dist )
                {
                    std::swap( min_dist, second_min_dist );
                    nearest = p;
                }
            }
        }

        min_dist = std::sqrt( min_dist );
        second_min_dist = std::sqrt( second_min_dist );

        //
        // exist ball nearest player
        //
        if ( nearest
             && min_dist < SP.ballSpeedMax()
             && min_dist < second_min_dist - 3.0 )
        {
            const PlayerType * ptype = nearest->playerTypePtr();
            double kickable_move_dist
                = ptype->kickableArea() + ( ptype->realSpeedMax()
                                            * std::pow( ptype->playerDecay(), 2 ) );
            double tackle_dist = ( nearest->isTackling()
                                   ? SP.tackleDist()
                                   : 0.0 );
            if ( nearest->pos().dist( prevBall().pos() ) < std::max( kickable_move_dist, tackle_dist ) )
            {
                if ( nearest->side() != theirSide() )
                {
                    M_last_kicker_side = ourSide();
                    M_last_kicker_unum = nearest->unum();
#ifdef DEBUG_PRINT_LAST_KICKER
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (updateLastKicker) set by nearest teammate or unknown."
                                  " side=%c unum=%d",
                                  ( nearest->side() == LEFT ? 'L'
                                    : nearest->side() == RIGHT ? 'R'
                                    : 'N' ),
                                  nearest->unum() );
#endif
                }
                else
                {
                    M_last_kicker_side = theirSide();
                    M_last_kicker_unum = nearest->unum();
#ifdef DEBUG_PRINT_LAST_KICKER
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (updateLastKicker) set by nearest opponent."
                                  " side=%c unum=%d",
                                  ( nearest->side() == LEFT ? 'L'
                                    : nearest->side() == RIGHT ? 'R'
                                    : 'N' ),
                                  nearest->unum() );
#endif
                }

                return;
            }
        }

        if ( exist_teammate_kicker
             && exist_opponent_kicker )
        {
            if ( M_last_kicker_side == ourSide()
                 && M_last_kicker_unum != Unum_Unknown )
            {
#ifdef DEBUG_PRINT_LAST_KICKER
                dlog.addText( Logger::WORLD,
                              __FILE__" (updateLastKicker) keep last kicker. teammate %d",
                              M_last_kicker_unum );
#endif
            }
            else
            {
                M_last_kicker_side = NEUTRAL;
                M_last_kicker_unum = Unum_Unknown;
#ifdef DEBUG_PRINT_LAST_KICKER
                dlog.addText( Logger::WORLD,
                              __FILE__" (updateLastKicker) set NEUTRAL." );
#endif
            }
            return;
        }
    }

    if ( ! kickers.empty() )
    {
        bool exist_teammate_kicker = false;
        int teammate_kicker_unum = Unum_Unknown;
        bool exist_opponent_kicker = false;
        int opponent_kicker_unum = Unum_Unknown;

        for ( const AbstractPlayerObject * p : kickers )
        {
            if ( p->side() == ourSide() )
            {
                exist_teammate_kicker = true;
                teammate_kicker_unum = p->unum();
            }
            else if ( p->side() == theirSide() )
            {
                exist_opponent_kicker = true;
                opponent_kicker_unum = p->unum();
            }
        }

        if ( exist_teammate_kicker
             && exist_opponent_kicker )
        {
            M_last_kicker_side = NEUTRAL;
            M_last_kicker_unum = Unum_Unknown;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by seen both side kickers." );
#endif
            return;
        }
        else if ( exist_teammate_kicker )
        {
            M_last_kicker_side = ourSide();
            M_last_kicker_unum = teammate_kicker_unum;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by seen teammate kicker." );
#endif
            return;
        }
        else if ( exist_opponent_kicker )
        {
            M_last_kicker_side = theirSide();
            M_last_kicker_unum = opponent_kicker_unum;
#ifdef DEBUG_PRINT_LAST_KICKER
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateLastKicker) set by seen opponent kicker." );
#endif
            return;
        }

    }
#ifdef DEBUG_PRINT_LAST_KICKER
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateLastKicker) no updated. last_kicker_side=%c",
                  ( M_last_kicker_side == LEFT  ? 'L'
                    : M_last_kicker_side == RIGHT ? 'R'
                    : 'N' ) );
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateInterceptTable()
{
    // update interception table
    M_intercept_table.update( *this );

    if ( M_audio_memory->ourInterceptTime() == time() )
    {
        for ( const AudioMemory::OurIntercept & v : M_audio_memory->ourIntercept() )
        {
            M_intercept_table.hearTeammate( *this, v.interceptor_, v.cycle_ );
        }
    }

    if ( M_audio_memory->oppInterceptTime() == time()
         && ! M_audio_memory->oppIntercept().empty() )
    {
        for ( const AudioMemory::OppIntercept & v : M_audio_memory->oppIntercept() )
        {
            M_intercept_table.hearOpponent( *this, v.interceptor_, v.cycle_ );
        }
    }

    M_self.setBallReachStep( std::min( interceptTable().selfStep(),
                                       interceptTable().selfExhaustStep() ) );

    const std::map< const AbstractPlayerObject *, int > & m = interceptTable().playerMap();

    for ( PlayerObject & p : M_teammates )
    {
        const std::map< const AbstractPlayerObject *, int >::const_iterator it = m.find( &p );
        if ( it != m.end() )
        {
            p.setBallReachStep( it->second );
        }
    }

    for ( PlayerObject & p : M_opponents )
    {
        const std::map< const AbstractPlayerObject *, int >::const_iterator it = m.find( &p );
        if ( it != m.end() )
        {
            p.setBallReachStep( it->second );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::checkGhost( const ViewArea & varea )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  NOTE: this method is called from updateAfterSee

    const double angle_buf = 5.0;

    //////////////////////////////////////////////////////////////////
    // ball
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (checkGhost) ball_count=%d, rpos_count=%d",
                  ball().posCount(), ball().rposCount() );
#endif

    if ( ball().rposCount() > 0
         && ball().posValid() )
    {
        const double BALL_VIS_DIST2
            = square( ServerParam::i().visibleDistance()
                      - ( self().vel().r() / self().playerType().playerDecay() ) * 0.1
                      - ( ball().vel().r() / ServerParam::i().ballDecay() ) * 0.05
                      - ( 0.12 * std::min( 4, ball().posCount() ) )
                      - 0.25 );

#ifdef DEBUG_PRINT_BALL_UPDATE
        Vector2D ballrel = ball().pos() - varea.origin();
        dlog.addText( Logger::WORLD,
                      __FILE__" (checkGhost) check ball. global_dist=%.2f."
                      "  visdist=%.2f.  ",
                      ballrel.r(), std::sqrt( BALL_VIS_DIST2 ) );
#endif

        if ( varea.contains( ball().pos(), angle_buf, BALL_VIS_DIST2 ) )
        {
#ifdef DEBUG_PRINT_BALL_UPDATE
            dlog.addText( Logger::WORLD,
                          __FILE__" (checkGhost) forget ball." );
#endif
            M_ball.setGhost();
        }
    }

    const double VIS_DIST2
        = square( ServerParam::i().visibleDistance()
                  - ( self().vel().r() / self().playerType().playerDecay() ) * 0.1
                  - 0.25 );
    //////////////////////////////////////////////////////////////////
    // players

    {
        std::list< PlayerObject >::iterator it = M_teammates.begin();
        while ( it != M_teammates.end() )
        {
            if ( it->posCount() > 0
                 && varea.contains( it->pos(), angle_buf, VIS_DIST2 ) )
            {
                if ( it->unum() == Unum_Unknown
                     && it->posCount() >= 10
                     && it->ghostCount() >= 2 )
                {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (checkGhost) erase teammate (%.1f %.1f)",
                                  it->pos().x, it->pos().y );
#endif
                    it = M_teammates.erase( it );
                    continue;
                }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (checkGhost) setGhost to teammate %d (%.1f %.1f).",
                              it->unum(), it->pos().x, it->pos().y );
#endif
                it->setGhost();
            }

            ++it;
        }
    }

    {
        std::list< PlayerObject >::iterator it = M_opponents.begin();
        while ( it != M_opponents.end() )
        {
            if ( it->posCount() > 0
                 && varea.contains( it->pos(), 1.0, VIS_DIST2 ) )
            {
                if ( it->unum() == Unum_Unknown
                     && it->posCount() >= 10
                     && it->ghostCount() >= 2 )
                {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (checkGhost) erase opponent (%.1f %.1f)",
                                  it->pos().x, it->pos().y );
#endif
                    it = M_opponents.erase( it );
                    continue;
                }

                dlog.addText( Logger::WORLD,
                              __FILE__" (checkGhost) setGhost to opponent %d (%.1f %.1f).",
                              it->unum(), it->pos().x, it->pos().y );
                it->setGhost();
            }

            ++it;
        }
    }

    {
        std::list< PlayerObject >::iterator it = M_unknown_players.begin();
        while ( it != M_unknown_players.end() )
        {
            if ( it->posCount() > 0
                 && varea.contains( it->pos(), 1.0, VIS_DIST2 ) )
            {
                if ( it->distFromSelf() < 40.0 * 1.06
                     || it->isGhost() ) // detect twice
                {
#ifdef DEBUG_PRINT_PLAYER_UPDATE
                    dlog.addText( Logger::WORLD,
                                  __FILE__" (checkGhost) erase unknown player (%.1f %.1f)",
                                  it->pos().x, it->pos().y );
#endif
                    it = M_unknown_players.erase( it );
                    continue;
                }

#ifdef DEBUG_PRINT_PLAYER_UPDATE
                dlog.addText( Logger::WORLD,
                              __FILE__" (checkGhost) setGhost to unknown player (%.1f %.1f)",
                              it->pos().x, it->pos().y );
#endif
                it->setGhost();
            }

            ++it;
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::updateDirCount( const ViewArea & varea )
{
    const double dir_buf// = DIR_STEP * 0.5;
        = ( ( self().lastMove().isValid()
              && self().lastMove().r() > 0.5 )
            ? DIR_STEP * 0.5 + 1.0
            : DIR_STEP * 0.5 );

    const AngleDeg left_limit = varea.angle() - varea.viewWidth() * 0.5 + dir_buf;
    const AngleDeg right_limit = varea.angle() + varea.viewWidth() * 0.5 - dir_buf;

    AngleDeg left_dir = varea.angle() - varea.viewWidth() * 0.5;
    int idx = static_cast< int >( ( left_dir.degree() - 0.5 + 180.0 ) / DIR_STEP );

    AngleDeg dir = -180.0 + DIR_STEP * idx;

    while ( dir.isLeftOf( left_limit ) )
    {
        dir += DIR_STEP;
        idx += 1;
        if ( idx > DIR_CONF_DIVS ) idx = 0;
    }

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateDirCount) left=%.1f right=%.1f dir buf=%.3f start_dir=%.1f start_idx=%d",
                  left_limit.degree(), right_limit.degree(),
                  dir_buf, dir.degree(), idx );
#endif

    while ( dir.isLeftOf( right_limit ) )
    {
        idx = static_cast< int >( ( dir.degree() - 0.5 + 180.0 ) / DIR_STEP );
        if ( idx > DIR_CONF_DIVS - 1 )
        {
            std::cerr << teamName() << " : " << self().unum()
                      << " DIR_CONF over flow  " << idx << std::endl;
            idx = DIR_CONF_DIVS - 1;
        }
        else if ( idx < 0 )
        {
            std::cerr << teamName() << " : " << self().unum()
                      << " DIR_CONF down flow  " << idx << std::endl;
            idx = 0;
        }
        //#ifdef DEBUG
#if 0
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateDirCount) update dir. index=%d : angle=%.0f",
                      idx, dir.degree() );
#endif
        M_dir_count[idx] = 0;
        dir += DIR_STEP;
    }

    //#ifdef DEBUG
#if 0
    if ( dlog.isLogFlag( Logger::WORLD ) )
    {
        double d = -180.0;
        for ( int i = 0; i < DIR_CONF_DIVS; ++i, d += DIR_STEP )
        {
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateDirCount) __ dir count: %.0f - %d",
                          d, M_dir_count[i] );
        }
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
int
WorldModel::dirRangeCount( const AngleDeg & angle,
                           const double & width,
                           int * max_count,
                           int * sum_count,
                           int * ave_count ) const
{
    if ( width <= 0.0 || 360.0 < width )
    {
        std::cerr << M_time << " " << self().unum() << ":"
                  << " invalid dir range"
                  << std::endl;
        return 1000;
    }

    int counter = 0;
    int tmp_sum_count = 0;
    int tmp_max_count = 0;

    AngleDeg tmp_angle = angle;
    if ( width > DIR_STEP ) tmp_angle -= width * 0.5;

    double add_dir = 0.0;
    while ( add_dir < width )
    {
        int c = dirCount( tmp_angle );

        tmp_sum_count += c;

        if ( c > tmp_max_count )
        {
            tmp_max_count = c;
        }

        add_dir += DIR_STEP;
        tmp_angle += DIR_STEP;
        ++counter;
    }

    if ( max_count )
    {
        *max_count = tmp_max_count;
    }

    if ( sum_count )
    {
        *sum_count = tmp_sum_count;
    }

    if ( ave_count )
    {
        *ave_count = tmp_sum_count / counter;
    }

    return counter;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
WorldModel::getPointCount( const Vector2D & point,
                           const double & dir_thr ) const
{
    const double vis_dist2 = square( ServerParam::i().visibleDistance() - 0.1 );

    int count = 0;
    for ( ViewAreaCont::const_iterator it = viewAreaCont().begin(), end = viewAreaCont().end();
          it != end;
          ++it, ++count )
    {
        if ( it->contains( point, dir_thr, vis_dist2 ) )
        {
            return count;
        }
    }

    return 1000;
}

/*-------------------------------------------------------------------*/
/*!

 */
AbstractPlayerObject::Cont
WorldModel::getPlayers( const PlayerPredicate * predicate ) const
{
    AbstractPlayerObject::Cont rval;

    if ( ! predicate ) return rval;

    for( const AbstractPlayerObject * p : allPlayers() )
    {
        if ( (*predicate)( *p ) )
        {
            rval.push_back( p );
        }
    }

    delete predicate;
    return rval;
}

/*-------------------------------------------------------------------*/
/*!

 */
AbstractPlayerObject::Cont
WorldModel::getPlayers( std::shared_ptr< const PlayerPredicate > predicate ) const
{
    AbstractPlayerObject::Cont rval;

    if ( ! predicate ) return rval;

    for ( const AbstractPlayerObject * p : allPlayers() )
    {
        if ( (*predicate)( *p ) )
        {
            rval.push_back( p );
        }
    }

    return rval;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::getPlayers( AbstractPlayerObject::Cont & cont,
                        const PlayerPredicate * predicate ) const
{
    if ( ! predicate ) return;

    for ( const AbstractPlayerObject * p : allPlayers() )
    {
        if ( (*predicate)( *p ) )
        {
            cont.push_back( p );
        }
    }

    delete predicate;
}


/*-------------------------------------------------------------------*/
/*!

 */
void
WorldModel::getPlayers( AbstractPlayerObject::Cont & cont,
                        std::shared_ptr< const PlayerPredicate > predicate ) const
{
    if ( ! predicate ) return;

    for ( const AbstractPlayerObject * p : allPlayers() )
    {
        if ( (*predicate)( *p ) )
        {
            cont.push_back( p );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
size_t
WorldModel::countPlayer( const PlayerPredicate * predicate ) const
{
    size_t count = 0;

    if ( ! predicate ) return count;

    for ( const AbstractPlayerObject * p : allPlayers() )
    {
        if ( (*predicate)( *p ) )
        {
            ++count;
        }
    }

    delete predicate;
    return count;
}

/*-------------------------------------------------------------------*/
/*!

 */
size_t
WorldModel::countPlayer( std::shared_ptr< const PlayerPredicate > predicate ) const
{
    size_t count = 0;

    if ( ! predicate ) return count;

    for ( const AbstractPlayerObject * p : allPlayers() )
    {
        if ( (*predicate)( *p ) )
        {
            ++count;
        }
    }

    return count;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
AbstractPlayerObject *
WorldModel::getOurGoalie() const
{
    if ( M_self.goalie() )
    {
        return &M_self;
    }

    if ( M_our_goalie_unum != Unum_Unknown )
    {
        return ourPlayer( M_our_goalie_unum );
    }

    for ( const PlayerObject & p : M_teammates )
    {
        if ( p.goalie() )
        {
            return &p;
        }
    }

    return nullptr;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
AbstractPlayerObject *
WorldModel::getTheirGoalie() const
{
    if ( M_their_goalie_unum != Unum_Unknown )
    {
        return theirPlayer( M_their_goalie_unum );
    }

    for ( const PlayerObject & p : M_opponents )
    {
        if ( p.goalie() )
        {
            return &p;
        }
    }

    return nullptr;
}

/*-------------------------------------------------------------------*/
/*!

 */
const PlayerObject *
WorldModel::getPlayerNearestTo( const Vector2D & point,
                                const PlayerObject::Cont & players,
                                const int count_thr,
                                double * dist_to_point ) const
{
    const PlayerObject * result = nullptr;
    double min_dist2 = 40000.0;

    for ( const PlayerObject * p : players )
    {
        if ( p->posCount() > count_thr )
        {
            continue;
        }

        double d2 = p->pos().dist2(point);
        if ( d2 < min_dist2 )
        {
            result = p;
            min_dist2 = d2;
        }
    }

    if ( result
         && dist_to_point )
    {
        *dist_to_point = std::sqrt( min_dist2 );
    }

    return result;
}

}
