// -*-c++-*-

/*!
  \file interception.h
  \brief interception utility class Header File
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

#ifndef RCSC_PLAYER_INTERCEPTION_H
#define RCSC_PLAYER_INTERCEPTION_H

#include <rcsc/common/player_type.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class Interception
  \brief interception estimation utility using Newton Method
*/
class Interception {
public:
    static const double NEVER_CYCLE; //!< error value

private:
    //! first ball position
    const Vector2D M_ball_first_pos;
    //! first ball speed
    const double M_ball_first_speed;
    //! frist ball velocity direction
    const AngleDeg M_ball_vel_angle;

    //! constant value for the Newton Method
    const double M_ball_x_constant;
    //! constant value for the Newton Method
    const double M_ball_x_d_constant;

public:

    /*!
      \brief create with ball info
      \param ball_pos ball first poitiion
      \param ball_first_speed ball first speed
      \param ball_vel_angle ball velocity direction

      When we use this utility in pass route evaluation,
      next cycle info should be set to ball_pos and ball_vel.
    */
    Interception( const Vector2D & ball_pos,
                  const double & ball_first_speed,
                  const AngleDeg & ball_vel_angle );

    /*!
      \brief create with ball info
      \param ball_pos ball first poitiion
      \param ball_vel ball first velocity

      When we use this utility in pass route evaluation,
      next cycle info should be set to ball_pos and ball_vel.
    */
    Interception( const Vector2D & ball_pos,
                  const Vector2D & ball_vel );

    /*!
      \brief predict ball gettable cycle for player using Newton Method
      \param player_pos player's position
      \param player_vel pointer to player velocity. if unknown, set NULL
      \param player_angle pointer to player angle. if unknown, set NULL
      \param player_count player's confidence count
      \param control_buf player's control area buffer (kickable area or catchable area)
      \param player_max_speed player's reachable max speed. player type paramter
      \return gettable cycle

      When we use this utility in pass or shoot route evaluation,
      next cycle info is set to player_pos.
    */
    double getReachCycle( const Vector2D & player_pos,
                          const Vector2D * player_vel,
                          const AngleDeg * player_angle,
                          const int player_count,
                          const double & control_buf,
                          const double & player_max_speed ) const;

    /*!
      \brief predict ball gettable cycle for player using greedy method
      \param player_pos player's position
      \param player_vel pointer to player velocity. if unknown, set NULL
      \param player_angle pointer to player angle. if unknown, set NULL
      \param player_count player's confidence count
      \param control_buf player's control area buffer (kickable area or catchable area)
      \param player_max_speed player's reachable max speed. player type paramter

      When we use this utility in pass or shoot route evaluation,
      next cycle info is set to player_pos.
     */
    double getReachCycleGreedly( const Vector2D & player_pos,
                                 const Vector2D * player_vel, // if unknown, NULL
                                 const AngleDeg * player_angle, // if unknown, NULL
                                 const int player_count,
                                 const double & control_buf,
                                 const double & player_max_speed ) const; // if unknown, NULL

};

}

#endif
