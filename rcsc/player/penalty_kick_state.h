// -*-c++-*-

/*!
  \file penalty_kick_state.h
  \brief penalty kick state model Header File
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

#ifndef PENALTY_KICK_STATE_H
#define PENALTY_KICK_STATE_H

#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <vector>

namespace rcsc {

class GameMode;

/*!
  \class PenaltyKickState
  \brief penalty kick status manager
*/
class PenaltyKickState {
private:

    //! time when mode is changed
    GameTime M_time;

    //! side of used goal
    SideID M_onfield_side;

    //! side of current kick taker team
    SideID M_current_taker_side;

    //! trial count for our team
    int M_our_taker_counter;
    //! trial count for opponent team
    int M_their_taker_counter;

    //! our penalty score
    int M_our_score;

    //! opponent penalty score
    int M_their_score;

    //! current kick kicker's side id
    int M_kick_taker_side;

    //! kick taker order
    int M_kick_taker_order[11];

public:
    /*!
      \brief init member variables
    */
    PenaltyKickState();

    /*!
      \brief get time when playmode is changed
      \return game time
    */
    const
    GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get side of used goal
      \return side Id
    */
    SideID onfieldSide() const
      {
          return M_onfield_side;
      }

    /*!
      \brief get current kick taker side
      \return side Id
    */
    SideID currentTakerSide() const
      {
          return M_current_taker_side;
      }

    /*!
      \brief get trial count for our team
      \return current trial count
    */
    int ourTakerCounter() const
      {
          return M_our_taker_counter;
      }

    /*!
      \brief get trial count for opponent team
      \return current trial count
    */
    int theirTakerCounter() const
      {
          return M_their_taker_counter;
      }

    /*!
      \brief get current our PK score
      \return score value
    */
    int ourScore() const
      {
          return M_our_score;
      }

    /*!
      \brief get current opponent PK score
      \return score value
    */
    int theirScore() const
      {
          return M_their_score;
      }

    /*!
      \brief check if the player is current kick taker.
      \param unum checked uniform number.
      \return checked result.
     */
    bool isKickTaker( const SideID side,
                      const int unum ) const;

    /*!
      \brief update status using refeee message
      \param game_mode playmode info
      \param ourside our team side Id
      \param current current game time
    */
    void update( const GameMode & game_mode,
                 const SideID ourside,
                 const GameTime & current );

    /*!
      \brief set kick taker's uniform number
      \param order order number [0,10]
      \param unum uniform number
     */
    void setKickTakerOrder( const std::vector< int > & unum_set );

};

}

#endif
