// -*-c++-*-

/*!
  \file world_state.h
  \brief world state class Header File
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

#ifndef RCSC_PLAYER_WORLD_STATE_H
#define RCSC_PLAYER_WORLD_STATE_H

#include <rcsc/player/self_object.h>
#include <rcsc/player/ball_object.h>
#include <rcsc/player/player_object.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_mode.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <array>
#include <vector>
#include <map>
#include <memory>
#include <string>

namespace rcsc {

class WorldModel;

/*!
  \class WorldState
  \brief a snapshot of the player's mental model.
*/
class WorldState {
public:

    using Ptr = std::shared_ptr< WorldState >; //!< smart pointer type
    using ConstPtr = std::shared_ptr< const WorldState >; //!< const smart pointer type

    using Cont = std::vector< ConstPtr >;
    using Map = std::map< GameTime, ConstPtr, GameTime::Less >;

private:

    GameTime M_time; //!< game time of this state
    GameMode M_game_mode; //!< playmode data

    SelfObject M_self; //!< self instance
    BallObject M_ball; //!< ball instance

    std::vector< PlayerObject > M_teammates; //!< teammate instance
    std::vector< PlayerObject > M_opponents; //!< opponent instance
    std::vector< PlayerObject > M_unknown_players; //!< unknown players instance

    std::vector< const AbstractPlayerObject * > M_our_players; //!< all teammates includes self
    std::vector< const AbstractPlayerObject * > M_their_players; //!< all opponents
    std::array< const AbstractPlayerObject *, 12 > M_our_players_array; //!< player pointers indexed by uniform number
    std::array< const AbstractPlayerObject *, 12 > M_their_players_array; //!< player pointers indexed by uniform number

    const AbstractPlayerObject * M_kickable_teammate;
    const AbstractPlayerObject * M_kickable_opponent;
    const AbstractPlayerObject * M_maybe_kickable_teammate;
    const AbstractPlayerObject * M_maybe_kickable_opponent;

    double M_offside_line_x; //!< offside line x value

    double M_our_offense_line_x; //!< our offense line x value(consider ball x)
    double M_our_defense_line_x; //!< our defense line x value(consider ball x)
    double M_their_offense_line_x; //!< their offense line x value(consider ball x)
    double M_their_defense_line_x; //!< their defense line x value(consider ball x)

    double M_our_offense_player_line_x; //!< our offense player line x value (not consider ball)
    double M_our_defense_player_line_x; //!< our defense player line x value (not consider ball)
    double M_their_offense_player_line_x; //!< their offense player line x value (not consider ball)
    double M_their_defense_player_line_x; //!< their defense player line x value (not consider ball)

    // not used
    WorldState() = delete;
    WorldState( const WorldState & ) = delete;
    WorldState & operator=( const WorldState & ) = delete;

public:
    /*!
      \brief create a new instance
      \param wm reference to the WorldModel
    */
    explicit
    WorldState( const WorldModel & wm );

    /*!
      \brief destructor
    */
    ~WorldState() = default;


    /*!
      \brief get the game time of this state
    */
    const GameTime & time() const
    {
        return M_time;
    }

    /*!
      \brief get the playmode of this state
      \return const reference to the GameMode object instance
    */
    const GameMode & gameMode() const
    {
        return M_game_mode;
    }

    /*!
      \brief get self data.
      \return const reference to the SelfObject instance.
    */
    const SelfObject & self() const
    {
        return M_self;
    }

    /*!
      \brief get ball data.
      \return const reference to the BallObject instance.
    */
    const BallObject & ball() const
    {
        return M_ball;
    }

    const AbstractPlayerObject::Cont & ourPlayers() const
    {
        return M_our_players;
    }

    const AbstractPlayerObject::Cont & theirPlayers() const
    {
        return M_their_players;
    }

    /*!
      \brief get a pointer to the teammate (or self) object, specified by uniform number
      \param unum target uniform number
      \return const pointer to the AbstractPlayerObject instance or NULL
    */
    const AbstractPlayerObject * ourPlayer( const int unum ) const
    {
        if ( unum <= 0 || 11 < unum ) return M_our_players_array[0];
        return M_our_players_array[unum];
    }

    /*!
      \brief get a pointer to the opponent object, specified by uniform number
      \param unum target uniform number
      \return const pointer to the AbstractPlayerObject instance or NULL
    */
    const AbstractPlayerObject * theirPlayer( const int unum ) const
    {
        if ( unum <= 0 || 11 < unum ) return M_their_players_array[0];
        return M_their_players_array[unum];
    }

};

}

#endif
