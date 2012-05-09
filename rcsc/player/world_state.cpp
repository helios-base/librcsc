// -*-c++-*-

/*!
  \file world_state.cpp
  \brief world state class Source File
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

#include "world_state.h"

#include "world_model.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
WorldState::WorldState()
    : M_time( -1, 0 ),
      M_game_mode(),
      M_ball()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
WorldState::~WorldState()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
WorldState::update( const WorldModel & wm );
{
    M_time = wm.time();
    M_game_mode = wm.gameMode();

    M_ball.update( wm.ball() );
}
b
