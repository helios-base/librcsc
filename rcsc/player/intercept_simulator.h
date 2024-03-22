// -*-c++-*-

/*!
  \file intercept_simulator.h
  \brief abstract intercep simulator manager Header File
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

#ifndef RCSC_PLAYER_INTERCEPT_SIMULATOR_H
#define RCSC_PLAYER_INTERCEPT_SIMULATOR_H

#include <rcsc/geom/vector_2d.h>

namespace rcsc {

class SelfObject;
class PlayerObject;
class WorldModel;

class InterceptSimulatorManager {
private:

protected:

    InterceptSimualtorManager() = default;

    virtual
    std::shared_ptr< InterceptSimulatorSelf > createSelfSimulator() = 0;

    virtual
    std::shared_ptr< InterceptSimulatorPlayer > createPlayerSimulator() = 0;

public:

    virtual
    ~InterceptSimulatorManager() = default;

    void createBallCache( const WorldModel & wm,
                          std::vector< Vector2D > & ball_cache );

    void simulateSelf( const SelfObject & self,
                       //const std::vector< Vector2D > & ball_cache,
                       std::vector< Intercept > & results );

    int simulatePlayer( const PlayerObject & player,
                        const std::vector< Vector2D > & ball_cache );
};

}

#endif
