// -*-c++-*-

/*!
  \file intercept_simulator_self_v17.h
  \brief intercept simulator for self
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

#ifndef RCSC_PLAYER_INTERCEPT_SIMULATOR_SELF_V17_H
#define RCSC_PLAYER_INTERCEPT_SIMULATOR_SELF_V17_H

#include <rcsc/player/intercept_simulator_self.h>

#include <rcsc/geom/vector_2d.h>
#include <vector>

namespace rcsc {

class BallObject;
class SelfObject;
class WorldModel;
class StaminaModel;

class InterceptSimulatorSelfV17
    : public InterceptSimulatorSelf {
private:

    Vector2D M_ball_vel;

public:

    /*!
      \brief simulate self interception, and store the results to self_results
      \param max_step max estimation cycle
      \param self_results reference to the result container
    */
    void simulate( const WorldModel & wm,
                   const int max_step,
                   std::vector< Intercept > & self_results ) override;

private:

    const Vector2D & ballVel() const
    {
        return M_ball_vel;
    }

    //
    // one step simulation
    //
    void simulateOneStep( const WorldModel & wm,
                          std::vector< Intercept > & self_cache );
    bool simulateNoDash( const WorldModel & wm,
                         std::vector< Intercept > & self_cache );

    void simulateOneDash( const WorldModel & wm,
                          std::vector< Intercept > & self_cache );
    void simulateOneDashAnyDir( const WorldModel & wm,
                                std::vector< Intercept > & self_cache );

    void simulateOneDashOld( const WorldModel & wm,
                             std::vector< Intercept > & self_cache );
    Intercept getOneAdjustDash( const WorldModel & wm,
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
                           std::vector< Intercept > & self_cache );
    Intercept getTurnDash( const WorldModel & wm,
                           const Vector2D & ball_pos,
                           const double control_area,
                           const double ball_noise,
                           const int step,
                           const bool back_dash );

    void simulateOmniDash( const WorldModel & wm,
                           const int max_step,
                           std::vector< Intercept > & self_cache );
    void simulateOmniDashAny( const WorldModel & wm,
                              const int max_step,
                              std::vector< Intercept > & self_cache );
    void simulateOmniDashOld( const WorldModel & wm,
                              const int max_step,
                              std::vector< Intercept > & self_cache );


    void simulateFinal( const WorldModel & wm,
                        const int max_step,
                        std::vector< Intercept > & self_cache );
};

}

#endif
