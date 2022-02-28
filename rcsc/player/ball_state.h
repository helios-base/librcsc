// -*-c++-*-

/*!
  \file ball_state.h
  \brief ball state data class Header File
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

#ifndef RCSC_PLAYER_BALL_STATE_H
#define RCSC_PLAYER_BALL_STATE_H

#include <rcsc/geom/vector_2d.h>

namespace rcsc {

class BallObject;

/*!
  \class BallState
  \brief ball state data
*/
class BallState {
private:

    Vector2D M_pos; //!< estimated global position
    Vector2D M_vel; //!< estimated velocity

    // not used
    BallState( const BallState & ) = delete;

public:
    /*!
      \brief constructor. initialize member variables
    */
    BallState();

    /*!
      \brief initialize member variables with observed data
      \param b observed ball data
     */
    void update( const BallObject & b );

    /*!
      \brief get Vector2D valur as the global position
      \return const reference to the Vector2D instance
    */
    const Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get Vector2D value as the velocity
      \return const referenct to Vector2D instance
    */
    const Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief estimate the vector of ball movement.
      \param step calculated step
      \return estimated ball movement
    */
    Vector2D inertiaTravel( const int step ) const;

    /*!
      \brief estimate the ball reach point
      \param step calculated step
      \return estimated ball position
    */
    Vector2D inertiaPoint( const int cycle ) const;


    /*!
      \brief estimate the finally reached point
      \return estimated ball position
    */
    Vector2D inertiaFinalPoint() const;
};

}

#endif
