// -*-c++-*-

/*!
  \file player_change_state.h
  \brief player change state holder class Header File
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

#ifndef RCSC_COACH_PLAYER_CHANGE_STATE_H
#define RCSC_COACH_PLAYER_CHANGE_STATE_H

#include <unordered_map>

namespace rcsc {

class GameTime;
class GameMode;

/*!
  \class PlayerChangeState
  \brief the holder of player change status
 */
class PlayerChangeState {
private:
    int M_change_count; //!< total number of change_player_type

    /*!
      player type holder for teammate
      key: type id, value: used count of that type
    */
    std::unordered_map< int, int > M_teammate_type_count;

    int M_teammate_types[11]; //!< teammate player type table
    bool M_opponent_changed[11]; //!< opponent player type flags


    // nocopyable
    PlayerChangeState( const PlayerChangeState & );
    PlayerChangeState & operator=( const PlayerChangeState & );
public:
    /*!
      \brief constructor
     */
    PlayerChangeState();

    /*!
      \brief parse (ok change_player_type ...) or (change_player_type ...)
      \param msg server raw message
      \param game_mode current game mode
      \param current current game time
    */
    void parse( const char * msg,
                const GameMode & game_mode,
                const GameTime & current );
private:
    /*!
      \param unum changed player's uniform number
      \param type new player type. if this value is negative, opponent player type flag is set.
     */
    void change( const int unum,
                 const int type );

public:

    /*!
      \brief check if the specified player can be changed or not
      \param unum target player's uniform number
      \param type new player type
      \param game_mode current game mode
      \param current current game time
     */
    bool canChange( const int unum,
                    const int type,
                    const GameMode & game_mode,
                    const GameTime & current ) const;

    /*!
      \brief get a teammate player type
      \param unum target player's uniform number
      \return player type id
     */
    int getTeammateType( const int unum ) const;

    /*!
      \brief check if opponent player is changed or not
      \param unum target teammate's uniform number
      \return true if player type is changed.
     */
    bool isOpponentChanged( const int unum ) const;


};

}

#endif
