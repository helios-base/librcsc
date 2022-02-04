// -*-c++-*-

/*!
  \file coach_intercept_predictor.h
  \brief intercept cycle predictor class Header File
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

#ifndef RCSC_COACH_PLAYER_INTERCEPT_H
#define RCSC_COACH_PLAYER_INTERCEPT_H

#include <vector>

namespace rcsc {

class CoachBallObject;
class CoachPlayerObject;
class PlayerType;
class Vector2D;

/*!
  \class CoachInterceptPredictor
  \brief player intercept cycle predictor for coach
 */
class CoachInterceptPredictor {
private:

    std::vector< Vector2D > M_ball_cache;

    // not used
    CoachInterceptPredictor() = delete;
    CoachInterceptPredictor( const CoachInterceptPredictor & ) = delete;
    CoachInterceptPredictor & operator=( const CoachInterceptPredictor & ) = delete;

public:

    /*!
      \brief create ball position cache
      \param ball current ball data
     */
    explicit
    CoachInterceptPredictor( const CoachBallObject & ball );

    int predict( const CoachPlayerObject & player ) const;

private:

    int predictReachStep( const CoachPlayerObject & player,
                          const bool goalie ) const;
    int predictMinStep( const CoachPlayerObject & player,
                        const PlayerType & ptype,
                        const double control_area ) const;

    bool canReachAfterTurnDash( const int total_step,
                                const int penalty_step,
                                const CoachPlayerObject & player,
                                const PlayerType & ptype,
                                const double control_area,
                                const Vector2D & ball_pos ) const;

    int predictTurnCycle( const int total_step,
                          const int penalty_step,
                          const CoachPlayerObject & player,
                          const PlayerType & ptype,
                          const double control_area,
                          const Vector2D & ball_pos ) const;


    bool canReachAfterDash( const int penalty_step,
                            const int n_turn,
                            const int n_dash,
                            const CoachPlayerObject & player,
                            const PlayerType & ptype,
                            const double control_area,
                            const Vector2D & ball_pos ) const;

    int predictFinal( const int penalty_step,
                      const CoachPlayerObject & player,
                      const PlayerType & ptype,
                      const double control_area ) const;

};

}

#endif
