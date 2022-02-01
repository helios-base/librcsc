// -*-c++-*-

/*!
  \file coach_world_state.h
  \brief coach/trainer world state class Header File
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

#ifndef RCSC_COACH_WORLD_STATE_H
#define RCSC_COACH_WORLD_STATE_H

#include <rcsc/coach/coach_ball_object.h>
#include <rcsc/coach/coach_player_object.h>
#include <rcsc/game_time.h>
#include <rcsc/game_mode.h>

#include <memory>
#include <iostream>
#include <list>
#include <map>

namespace rcsc {

class AudioMemory;
class CoachVisualSensor;

namespace rcg {
struct DispInfoT;
}

/*!
  \class CoachWorldState
  \brief a state of coach's internal model
 */
class CoachWorldState {
public:

    typedef std::shared_ptr< CoachWorldState > Ptr; //!< smart pointer type
    typedef std::shared_ptr< const CoachWorldState > ConstPtr; //!< smart const pointer type

    typedef std::list< ConstPtr > List;
    typedef std::map< GameTime, ConstPtr, GameTime::Less > Map;

private:

    const SideID M_our_side; //!< team side id. if trainer or log analyzer, NEUTRAL is set.

    GameTime M_time; //!< the time of this state
    GameMode M_game_mode; //!< playmode of this state

    CoachBallObject M_ball; //!< ball instance
    CoachPlayerObject::Cont M_all_players; //!< all players (instance)
    CoachPlayerObject::Cont M_teammates; //!< teammate players (reference). if trainer, this container holds left side players.
    CoachPlayerObject::Cont M_opponents; //!< opponent players (reference). if trainer, this container holds right side players.

    //! teammate player array (reference). this may include NULL. if trainer, this holds left side players.
    CoachPlayerObject * M_teammate_array[11];
    //! opponent player array (reference). this may include NULL. if trainer, this holds right side players.
    CoachPlayerObject * M_opponent_array[11];

    double M_our_offside_line_x; //!< offside line x for our team
    double M_their_offside_line_x; //!< offside line x for their team

    const CoachPlayerObject * M_kicker; //!< estimated last kicker (reference). if no kicker, NULL is set.
    CoachPlayerObject::Cont M_kicker_candidates; //!< possible kickers (reference).

    SideID M_ball_owner_side; //!< estimated ball owner team side.
    const CoachPlayerObject * M_ball_owner; //!< estimated ball owner player (reference). this may be NULL.


    const CoachPlayerObject * M_fastest_intercept_player; //!< the player that has the smallest ball reach step in all players.
    const CoachPlayerObject * M_fastest_intercept_teammate; //!< the teammate that the smallest ball reach step in teammates.
    const CoachPlayerObject * M_fastest_intercept_opponent; //!< the opponent that the smallest ball reach step in opponents.

    // not used
    CoachWorldState( const CoachWorldState & ) = delete;
    CoachWorldState & operator=( const CoachWorldState & ) = delete;

public:

    /*!
      \brief create empty state
     */
    CoachWorldState();

    /*!
      \brief construct from sensor data
      \param see_global see information
      \param our_side team side id
      \param current_time current game time
      \param current_mode current play mode
      \param prev_state previous cycle's state
     */
    CoachWorldState( const CoachVisualSensor & see_global,
                     const SideID our_side,
                     const GameTime & current_time,
                     const GameMode & current_mode,
                     const CoachWorldState::Ptr & prev_state );


    /*!
      \brief construct from game log data.
      \param disp one cycle display data.
      \param current_time current game time
      \param current_mode current play mode
      \param prev_state previous cycle's state
     */
    CoachWorldState( const rcg::DispInfoT & disp,
                     const GameTime & current_time,
                     const GameMode & current_mode,
                     const CoachWorldState::Ptr & prev_state );

    /*!
      \brief nothing to do
    */
    ~CoachWorldState();

    /*!
      \brief set player type
      \param side player's side
      \param unum player's uniform number
      \param type player type id
     */
    void setPlayerType( const SideID side,
                        const int unum,
                        const int type );

    /*!
      \brief set teammate's player type
      \param unum player's uniform number
      \param type player type id
     */
    void setTeammatePlayerType( const int unum,
                                const int type );

    /*!
      \brief set opponent's player type
      \param unum player's uniform number
      \param type player type id
     */
    void setOpponentPlayerType( const int unum,
                                const int type );

    /*!
      \brief set yellow/red card state to the player
      \param side target player's team side
      \param unum target player's uniform number
     */
    void setCard( const SideID side,
                  const int unum,
                  const Card card );

    /*!
      \brief update players' stamina
      \param audio heard information
     */
    void updatePlayerStamina( const AudioMemory & audio );

private:

    /*!
      \brief calculate offside lines for both team
     */
    void updateOffsideLines();

    /*!
      \brief estimate last kicker (or possible tackler)
     */
    void updateKicker( const CoachWorldState::Ptr & prev_state );

    /*!
      \brief estimate all players' intercept step
     */
    void updateInterceptTable();

    /*!
      \brief get the specified player.
      \param side team side id
      \param unum uniform number
      \return player object pointer. NULL may be returned.
     */
    const CoachPlayerObject * getPlayerImpl( const SideID side,
                                             const int unum ) const;

public:

    //
    //
    //

    bool isValid() const
      {
          return M_time.cycle() >= 0;
      }

    /*!
      \return get our team side id. if trainer or log analyzer, NEUTRAL is returned.
      \return our team side id.
     */
    SideID ourSide() const
      {
          return M_our_side;
      }

    /*!
      \brief get updated time
      \return const reference to the game time object
    */
    const GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get the playmode of this state
     */
    GameMode gameMode() const
      {
          return M_game_mode;
      }

    /*!
      \brief get analyzed ball info
      \return const reference to the analyzed data
    */
    const CoachBallObject & ball() const
      {
          return M_ball;
      }

    /*!
      \brief get all players
      \return const pointer container
    */
    const CoachPlayerObject::Cont & allPlayers() const
      {
          return M_all_players;
      }

    /*!
      \brief get teammate players. array element may contain NULL pointer.
      \return const pointer container
    */
    const CoachPlayerObject::Cont & teammates() const
      {
          return M_teammates;
      }

    /*!
      \brief get opponent players. array element may contain NULL pointer.
      \return const pointer container
    */
    const CoachPlayerObject::Cont & opponents() const
      {
          return M_opponents;
      }

    /*!
      \brief get the specified player.
      \param side team side id
      \param unum uniform number
      \return player object pointer. NULL may be returned.
     */
    const CoachPlayerObject * getPlayer( const SideID side,
                                         const int unum ) const
      {
          return ( ourSide() == NEUTRAL
                   ? getPlayerImpl( side, unum )
                   : side != ourSide()
                   ? opponent( unum )
                   : teammate( unum ) );
      }

    /*!
      \brief get the teammate player
      \param unum uniform number
      \return player object pointer. NULL may be returned.
     */
    const CoachPlayerObject * teammate( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return nullptr;
          return M_teammate_array[unum-1];
      }

    /*!
      \brief get the opponent player
      \param unum uniform number
      \return player object pointer. NULL may be returned.
     */
    const CoachPlayerObject * opponent( const int unum ) const
      {
          if ( unum < 1 || 11 < unum ) return nullptr;
          return M_opponent_array[unum-1];
      }

    /*!
      \brief get the offside line x for our team
      \return x-coordinate value
     */
    double ourOffsideLineX() const
      {
          return M_our_offside_line_x;
      }

    /*!
      \brief get the offside line x for their team
      \return x-coordinate value
     */
    double theirOffsideLineX() const
      {
          return M_their_offside_line_x;
      }

    /*!
      \brief get pointer to the estimated kicker player. if no kicker, NULL is returned.
      \return pointer to the kicker player object or NULL.
     */
    const CoachPlayerObject * kicker() const
      {
          return M_kicker;
      }

    /*!
      \brief get the possible kicker player container.
      \return container of player object pointer
     */
    const CoachPlayerObject::Cont & kickerCandidates() const
      {
          return M_kicker_candidates;
      }

    /*!
      \brief get the estimated ball owner team side
      \return side id
     */
    SideID ballOwnerSide() const
      {
          return M_ball_owner_side;
      }

    /*!
      \brief get the estimated ball owner.
      \return const pointer to the player object
     */
    const CoachPlayerObject * ballOwner() const
      {
          return M_ball_owner;
      }

    /*!
      \brief get the player that has the smallest ball reach step in all players.
      \return player object pointer. if no player, NULL is returned.
     */
    const CoachPlayerObject * fastestInterceptPlayer() const
      {
          return M_fastest_intercept_player;
      }

    /*!
      \brief get the smallest ball interception step.
      \return estimated ball interception step value.
     */
    int ballReachStep() const
      {
          return ( M_fastest_intercept_player
                   ? M_fastest_intercept_player->ballReachStep()
                   : 1000 );
      }

    /*!
      \brief get the player that has the smallest ball reach step in teammates.
      \return player object pointer. if no player, NULL is returned.
     */
    const CoachPlayerObject * fastestInterceptTeammate() const
      {
          return M_fastest_intercept_teammate;
      }

    /*!
      \brief get the smallest ball interception step.
      \return estimated ball interception step value.
     */
    int teammateBallReachStep() const
      {
          return ( M_fastest_intercept_teammate
                   ? M_fastest_intercept_teammate->ballReachStep()
                   : 1000 );
      }

    /*!
      \brief get the player that has the smallest ball reach step in opponents.
      \return player object pointer. if no player, NULL is returned.
     */
    const CoachPlayerObject * fastestInterceptOpponent() const
      {
          return M_fastest_intercept_opponent;
      }

    /*!
      \brief get the smallest ball interception step.
      \return estimated ball interception step value.
     */
    int opponentBallReachStep() const
      {
          return ( M_fastest_intercept_opponent
                   ? M_fastest_intercept_opponent->ballReachStep()
                   : 1000 );
      }

    /*!
      \brief get the current ball position status
      \return ball status type
     */
    BallStatus getBallStatus() const;

};

}

#endif
