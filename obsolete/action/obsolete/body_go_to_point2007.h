// -*-c++-*-

/*!
  \file body_go_to_point2007.h
  \brief go to point action
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

#ifndef RCSC_ACTION_BODY_GO_TO_POINT_2007_H
#define RCSC_ACTION_BODY_GO_TO_POINT_2007_H

#include <rcsc/player/soccer_action.h>
#include <rcsc/common/player_type.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class Body_GoToPoint
  \brief run behavior which has target point.
 */
class Body_GoToPoint2007
    : public BodyAction {
private:
    //! target point to be reached
    const Vector2D M_target_point;
    //! distance threshold to the target point
    const double M_dist_thr;
    //! maximum power parameter (this value has to be positive)
    const double M_dash_power;
    //! dummy parameter to keep compatibilty
    const double M_dash_speed;
    //! recommended reach cycle
    const int M_cycle;
    //! if this is true, agent must save recover parameter.
    const bool M_save_recovery;
    //! minimal turn buffer
    const double M_dir_thr;

    //! internal variable. if this value is true, player will dash backward.
    bool M_back_mode;
public:

//     Body_GoToPoint( const Vector2D & point,
//                     const double & dist_thr,
//                     const double & dash_power,
//                     const int cycle = 100,
//                     const bool back_mode = false,
//                     const bool save_recovery = true,
//                     const double & dir_thr = 12.0 )
//         : M_target_point( point ),
//           M_dist_thr( dist_thr ),
//           M_dash_power( std::fabs( dash_power ) )
//           M_dash_speed( 1000.0 ),
//           M_cycle( cycle ),
//           M_back_mode( back_mode ),
//           M_save_recovery( save_recovery ),
//           M_dir_thr( dir_thr )
//       { }

    /*!
      \brief construct with all paramters
      \param point target point to be reached
      \param dist_thr distance threshold to the target point
      \param dash_power dash command parameter (positive valie)
      \param dash_speed dummy parameter to keep compatibility
      \param cycle preferred reach cycle
      \param save_recovery if this is true, player will keep its recovery.
      \param dir_thr turn angle threshold
    */
    Body_GoToPoint2007( const Vector2D & point,
                        const double & dist_thr,
                        const double & dash_power,
                        const double & dash_speed = -1.0,
                        const int cycle = 100,
                        const bool save_recovery = true,
                        const double & dir_thr = 12.0 )
        : M_target_point( point ),
          M_dist_thr( dist_thr ),
          M_dash_power( std::fabs( dash_power ) ),
          M_dash_speed( dash_speed ),
          M_cycle( cycle ),
          M_save_recovery( save_recovery ),
          M_dir_thr( dir_thr ),
          M_back_mode( false )
      { }

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
    */
    bool execute( PlayerAgent * agent );

private:
    /*!
      \brief if necesarry, perform turn action and return true
      \param agent pointer to agent itself
      \param target_rel relative coordinate value of target point
      \param accel_angle dash direction
      \return true if turn is performed
    */
    bool doTurn( PlayerAgent * agent,
                 const Vector2D & target_rel,
                 AngleDeg * accel_angle );

    /*!
      \brief if necesarry, perform dash action and return true
      \param agent pointer to agent itself
      \param target_rel relative coordinate value of target point
      \param accel_angle dash direction
      \return true if turn is performed
    */
    bool doDash( PlayerAgent * agent,
                 Vector2D target_rel,
                 const AngleDeg & accel_angle );
};

}

#endif
