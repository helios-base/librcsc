// -*-c++-*-

/*!
  \file player_state.h
  \brief player state data class Header file.
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

#ifndef RCSC_COMMON_PLAYER_STATE_H
#define RCSC_COMMON_PLAYER_STATE_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <vector>
#include <iostream>

namespace rcsc {

class PlayerType;

/*-------------------------------------------------------------------*/

/*!
  \class PlayerState
  \brief player state data
*/
class PlayerState {
public:

    //! container type of the const PlayerState pointer
    typedef std::vector< const PlayerState * > Cont;

private:

    SideID M_side; //!< LEFT or RIGHT
    int M_unum; //!< uniform number
    bool M_goalie; //!< goalie or not
    const PlayerType * M_player_type; //!< const point to the player type instance

    Vector2D M_pos; //!< global position
    Vector2D M_vel; //!< velocity
    AngleDeg M_body; //!< body angle
    AngleDeg M_face; //!< global neck angle

    // StaminaModel M_stamina; //!< estimated stamina value

    // bool M_kicking; //!< true if player performed the kick.
    // int M_tackle_cycle; //!< if player is tackling, this value is incremented
    // int M_charged_cycle; //!< if player is charged, this value is incremented

    Card M_card; //!< player's card status

public:

    /*!
      \brief inititialize all value with 0 or invalid values
    */
    PlayerState();

    /*!
      \brief check if this player is valid or not.
      \return checked result.
     */
    bool isValid() const
      {
          return M_side != NEUTRAL;
      }

    /*!
      \brief get side info
      \return side ID
     */
    SideID side() const
      {
          return M_side;
      }

    /*!
      \brief get player's uniform number
      \return uniform number
     */
    int unum() const
      {
          return M_unum;
      }

    /*!
      \brief check if this player is goalie
      \return if goalie, true
     */
    bool goalie() const
      {
          return M_goalie;
      }

    /*!
      \brief get the pointer to player type object
      \return const pointer
     */
    const PlayerType * playerType() const
      {
          return M_player_type;
      }

    /*!
      \brief get player's position
      \return const reference to the position variable
     */
    const Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get player's velocity
      \return const reference to the velocity variable
     */
    const Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief get player's body angle
      \return const reference to the body angle variable
     */
    const AngleDeg & body() const
      {
          return M_body;
      }

    /*!
      \brief get player's global face angle
      \return const reference to the face angle variable
     */
    const AngleDeg & face() const
      {
          return M_face;
      }

    /*!
      \brief get current card status.
      \return card type
     */
    Card card() const
      {
          return M_card;
      }

    //////////////////////////////////////////////////////////////

    /*!
      \brief set team data with seen information
      \param side side ID
      \param unum uniform number
      \param goalie goalie flag
     */
    void setTeam( const SideID side,
                  const int unum,
                  const bool goalie )
      {
          M_side = side;
          M_unum = unum;
          M_goalie = goalie;
      }

    /*!
      \brief set player type by giving type id
      \param type player type id
     */
    void setPlayerType( const int type );

    /*!
      \brief set player type
      \param type pointer to the player type instance
     */
    void setPlayerType( const PlayerType * type )
      {
          M_player_type = type;
      }

    /*!
      \brief set position with seen information
      \param x seen x
      \param y seen y
     */
    void setPos( const double x,
                 const double y )
      {
          M_pos.assign( x, y );
      }

    /*!
      \brief set velocity with seen information
      \param vx seen velocity x
      \param vy seen velocity y
     */
    void setVel( const double vx,
                 const double vy )
      {
          M_vel.assign( vx, vy );
      }

    /*!
      \brief set body and neck angle with seen information
      \param b seen body angle
      \param n seen neck angle relative to body
     */
    void setAngle( const double b,
                   const double n )
      {
          M_body = b;
          M_face = b + n;
      }

    /*!
      \brief set card status
     */
    void setCard( const Card card )
      {
          M_card = card;
      }

};

}

#endif
