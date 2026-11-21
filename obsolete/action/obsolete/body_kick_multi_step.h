// -*-c++-*-

/*!
  \file body_kick_multi_step.h
  \brief multiple kick behavior to accelerate the ball to the
  desired speed.
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

#ifndef RCSC_ACTION_BODY_KICK_MULTI_STEP_H
#define RCSC_ACTION_BODY_KICK_MULTI_STEP_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class Body_KickMultiStep
  \brief multiple kick planning and behavior to accelerate the ball to
  the desired speed.
  NOTE!!  please use carefully enforce mode.
*/
class Body_KickMultiStep
    : public BodyAction {
private:
    //! target point where the ball should reach or pass through
    const Vector2D M_target_point;
    //! ball first speed when ball is released
    double M_first_speed;
    //! if true, ball should be released enforcely
    const bool M_enforce_kick;

    //! result ball position
    Vector2D M_ball_result_pos;
    //! result ball velocity
    Vector2D M_ball_result_vel;
    //! estimated kick step
    int M_kick_step;
public:
    /*!
      \brief construct with all parameters
      \param target_point target point where the ball should reach or pass
      through
      \param first_speed desired ball first speed
      \param enforce if true, ball should be released enforcely
    */
    Body_KickMultiStep( const Vector2D & target_point,
                        const double & first_speed,
                        const bool enforce = false )
        : M_target_point( target_point )
        , M_first_speed( first_speed )
        , M_enforce_kick( enforce )
        , M_ball_result_pos( Vector2D::INVALIDATED )
        , M_ball_result_vel( Vector2D::INVALIDATED )
        , M_kick_step( 0 )
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
    */
    bool execute( PlayerAgent * agent );


    /*!
      \brief get the result ball position
      \return ball position after kick
     */
    const
    Vector2D & ballResultPos() const
      {
          return M_ball_result_pos;
      }

    /*!
      \brief get the result ball velocity
      \return ball velocity after kick
     */
    const
    Vector2D & ballResultVel() const
      {
          return M_ball_result_vel;
      }

    /*!
      \brief get the estimated kick steps
      \return estimated kick steps
     */
    int kickStep() const
      {
          return M_kick_step;
      }


    /*!
      \brief static method. search the appropriate kick queue.
      \param achieved_vel max reachable ball velocity.
      \param next_vel next ball velocity when this kick is done. this value is
      used to re-calculate the required kick power.
      \param target_rpos target position relative to current agent position
      \param first_speed the desired ball first speed
      \param my_rpos estimated agent position relative to current agent position
      \param my_vel estimated agent velocity
      \param my_body agent body angle
      \param ball_rpos ball position relative to current agent position.
      \param ball_vel ball velocity
      \param agent pointer to agent itself
      \param enforce if true, planning is done to kick the ball enforcely
      \return true if ball can be accelerated to the desired vel  or enforce mode
    */
    static
    bool simulate_three_kick( Vector2D * achieved_vel,
                              Vector2D * next_vel,
                              const Vector2D & target_rpos,
                              const double & first_speed,
                              const Vector2D & my_rpos,
                              const Vector2D & my_vel,
                              const AngleDeg & my_body,
                              const Vector2D & ball_rpos,
                              const Vector2D & ball_vel,
                              const PlayerAgent * agent,
                              const bool enforce );

};

}

#endif
