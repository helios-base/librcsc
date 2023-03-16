// -*-c++-*-

/*!
  \file intercept_simulator_self.h
  \brief base class of the self intercept simulator
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

#ifndef RCSC_PLAYER_INTERCEPT_SIMULATOR_SELF_H
#define RCSC_PLAYER_INTERCEPT_SIMULATOR_SELF_H

#include <rcsc/player/intercept_table.h>

namespace rcsc {

class WorldModel;

class InterceptSimulatorSelf {
private:

protected:

    //! protected constructor since this is an abstract class
    InterceptSimulatorSelf() = default;

public:

    //! virtual destructor
    virtual
    ~InterceptSimulatorSelf() = default;

    /*!
      \brief pure virtual function. simulate self interception, and store the results to self_results
      \param wm world model
      \param max_step max prediction step
      \param self_results reference to the result container
     */
    virtual
    void simulate( const WorldModel & wm,
                   const int max_step,
                   std::vector< Intercept > & self_results ) = 0;

};

}

#endif
