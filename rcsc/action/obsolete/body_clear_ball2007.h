// -*-c++-*-

/*!
  \file body_clear_ball2007.h
  \brief kick the ball to escape from a dangerous situation header file
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

#ifndef RCSC_ACTION_BODY_CLEAR_BALL_2007_H
#define RCSC_ACTION_BODY_CLEAR_BALL_2007_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/geom/angle_deg.h>
#include <rcsc/game_time.h>

namespace rcsc {

/*!
  \class Body_ClearBall2007
  \brief kick the ball to escape from a dangerous situation
 */
class Body_ClearBall2007
    : public BodyAction {
private:
    static const double S_SEARCH_ANGLE;

    static GameTime S_last_calc_time;
    static AngleDeg S_cached_best_angle;

public:
    /*!
      \brief accessible from global.
    */
    Body_ClearBall2007()
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
    */
    bool execute( PlayerAgent * agent );

    /*!
      \brief calculate best kick angle
      \param agent const pointer to the agent itself
      \param lower_angle considered angle range min
      \param upper_angle considered angle range max
      \param clear_mode true if this search is done for the clear kick
      \return calculated angle
    */
    static
    AngleDeg get_best_angle( const PlayerAgent * agent,
                             const double & lower_angle,
                             const double & upper_angle,
                             const bool clear_mode = true );

    /*!
      \brief calculate score of the specified angle for the clear kick
      \param agent const pointer to the agent itself
      \param target_angle angle to be calculated
      \return calcurated score
     */
    static
    double calc_score( const PlayerAgent * agent,
                       const AngleDeg & target_angle );
};

}

#endif
