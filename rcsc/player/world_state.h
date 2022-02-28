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
#define RCSC_PLAYER_WORLD_MODEL_H

//#include <rcsc/player/self_object.h>
#include <rcsc/player/ball_object.h>
//#include <rcsc/player/player_object.h>
//#include <rcsc/player/view_area.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_mode.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <memory>
#include <string>

namespace rcsc {

/*!
  \class WorldState
  \brief a snapshot of the player's mental model.
*/
class WorldState {
public:

    typedef std::shared_ptr< WorldStae > Ptr; //!< smart pointer type
    typedef std::shared_ptr< const WorldStae > ConstPtr; //!< const smart pointer type

    GameTime M_time; //!< game time of this state
    GameMode M_game_mode; //!< playmode data

    BallState M_ball; //!< ball instance

    // not used
    WorldState( const WorldState & ) = delete;
    WorldState & operator=( const WorldState & ) = delete;

public:
    /*!
      \brief initialize member variables
    */
    WorldState();

    /*!
      \brief destructor
    */
    ~WorldState();


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
      \brief get ball data.
      \return const reference to the ball object instance.
     */
    const BallState & ball() const
      {
          return M_ball;
      }

    /*!
      \brief this method estimates new state data using the agent's internal world model.
      \param wm world model instance
    */
    void update( const WorldModel & wm );

};

}

#endif
