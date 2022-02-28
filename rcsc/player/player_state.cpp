// -*-c++-*-

/*!
  \file coach_player_object.cpp
  \brief player object class for coach/trainer Source file.
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

#include "player_state.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PlayerState::PlayerState()
    : M_side( NEUTRAL ),
      M_unum( Unum_Unknown ),
      M_goalie( false ),
      M_player_type( nullptr ),
      M_pos( Vector2D::INVALIDATED ),
      M_vel( 0.0, 0.0 ),
      M_body( 0.0 ),
      M_face( 0.0 ),
      M_card( NO_CARD )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerState::setPlayerType( const int type )
{
    M_player_type = PlayerTypeSet::i().get( type );
}

}
