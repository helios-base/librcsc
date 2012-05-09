// -*-c++-*-

/*!
  \file coach_ball_object.h
  \brief ball object class for coach/trainer Header file.
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

#ifndef RCSC_COACH_BALL_OBJECT_H
#define RCSC_COACH_BALL_OBJECT_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

namespace rcsc {

/*!
  \class CoachBallObject
  \brief ball information for coath/trainer
*/
class CoachBallObject {
private:

    Vector2D M_pos; //!< global coordinate value
    Vector2D M_vel; //!< velocity vector

public:

    /*!
      \brief initialize all data with 0.
     */
    CoachBallObject();

    /*!
      \brief get global position
      \return const reference value
    */
    const Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get global velocity
      \return const reference value
    */
    const Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief set position
      \param x coordinage x
      \param y coordinage y
      \param vx velocity x
      \param vy velocity y
     */
    const CoachBallObject & setValue( const double x,
                                      const double y,
                                      const double vx,
                                      const double vy )
      {
          M_pos.assign( x, y );
          M_vel.assign( vx, vy );
          return *this;
      }

    /*!
      \brief reverse positional value.
     */
    void reverseSide()
      {
          M_pos.reverse();
          M_vel.reverse();
      }

    // inertia movement calculators

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
