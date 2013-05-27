// -*-c++-*-

/*!
  \file self_intercept_simulator.h
  \brief self intercept simulator for creating candidate intercept actions
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

#ifndef RCSC_PLAYER_SELF_INTERCEPT_SIMULATOR_H
#define RCSC_PLAYER_SELF_INTERCEPT_SIMULATOR_H

#include <rcsc/player/intercept_table.h>
#include <rcsc/geom/vector_2d.h>
#include <vector>

namespace rcsc {


class BallObject;
class SelfObject;
class WorldModel;
class StaminaModel;

class SelfInterceptSimulator {
private:

public:

    /*!
      \brief simulate self interception, and store the results to self_cache
      \param max_step max estimation cycle
      \param self_cache reference to the interception info container
      to store the result
    */
    void simulate( const WorldModel & wm,
                   const int max_step,
                   std::vector< InterceptInfo > & self_cache );

private:

    //
    // one step simulation
    //
    void simulateOneStep( const WorldModel & wm,
                          std::vector< InterceptInfo > & self_cache );
    bool simulateNoDash( const WorldModel & wm,
                         std::vector< InterceptInfo > & self_cache );
    void simulateOneDash( const WorldModel & wm,
                          std::vector< InterceptInfo > & self_cache );
    InterceptInfo getOneAdjustDash( const WorldModel & wm,
                                    const AngleDeg & dash_angle,
                                    const Vector2D & max_forward_accel,
                                    const Vector2D & max_back_accel,
                                    const double control_area );
    double getOneStepDashPower( const WorldModel & wm,
                                const Vector2D & next_ball_rel,
                                const AngleDeg & dash_angle,
                                const double max_forward_accel_x,
                                const double max_back_accel_x );

    //
    // two or longer step simulation
    //
    void simulateTurnDash( const WorldModel & wm,
                           const int max_step,
                           const bool back_dash,
                           std::vector< InterceptInfo > & self_cache );
    InterceptInfo getTurnDash( const WorldModel & wm,
                               const Vector2D & ball_pos,
                               const double control_area,
                               const int step,
                               const bool back_dash );

    void simulateOmniDash( const WorldModel & wm,
                           const Vector2D & ball_pos,
                           const double control_area,
                           const int step,
                           const bool back_dash,
                           std::vector< InterceptInfo > & self_cache );
};

}

#endif
