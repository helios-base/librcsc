// -*-c++-*-

/*!
  \file body_advance_ball2007.h
  \brief kick the ball to forward direction to an avoid opponent
  player's interfare header file.
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

#ifndef RCSC_ACTION_BODY_ADVANCE_BALL_2007_H
#define RCSC_ACTION_BODY_ADVANCE_BALL_2007_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/angle_deg.h>
#include <rcsc/game_time.h>

namespace rcsc {

/*!
  \class Body_AdvanceBall2007
  \brief kick the ball to forward direction to an avoid opponent
  player's interfare
 */
class Body_AdvanceBall2007
    : public BodyAction {
private:
    //! last game time when calcuration is done.
    static GameTime S_last_calc_time;
    //! last calculated result
    static AngleDeg S_cached_best_angle;

public:
    /*!
      \brief accessible from global.
     */
    Body_AdvanceBall2007()
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
     */
    bool execute( PlayerAgent * agent );

private:

    /*!
      \brief calculate best kick angle
      \param agent const pointer to the agent itself
      \return calculated result
     */
    AngleDeg getBestAngle( const PlayerAgent * agent );
};

}

#endif
