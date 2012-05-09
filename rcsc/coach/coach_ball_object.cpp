// -*-c++-*-

/*!
  \file coach_ball_object.cpp
  \brief ball object class for coach/trainer Source file.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "coach_ball_object.h"

#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CoachBallObject::CoachBallObject()
    : M_pos( 0.0, 0.0 ),
      M_vel( 0.0, 0.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
CoachBallObject::inertiaTravel( const int step ) const
{
    return inertia_n_step_travel( M_vel,
                                  step,
                                  ServerParam::i().ballDecay() );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
CoachBallObject::inertiaPoint( const int step ) const
{
    return inertia_n_step_point( M_pos,
                                 M_vel,
                                 step,
                                 ServerParam::i().ballDecay() );
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
CoachBallObject::inertiaFinalPoint() const
{
    return inertia_final_point( M_pos,
                                M_vel,
                                ServerParam::i().ballDecay() );
}

}
