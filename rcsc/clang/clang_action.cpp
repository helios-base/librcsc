// -*-c++-*-

/*!
  \file clang_action.cpp
  \brief clang action class Source File
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

#include "clang_action.h"

#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CLangActionMark::print( std::ostream & os ) const
{
    os << "(mark ";
    M_target_players->print( os );
    os << ')';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CLangActionHeteroType::print( std::ostream & os ) const
{
    os << "(htype " << M_player_type << ')';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CLangActionHold::print( std::ostream & os ) const
{
    os << "(hold)";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CLangActionBallTo::print( std::ostream & os ) const
{
    os << "(bto ";
    M_assigned_players->print( os );
    os << ')';
    return os;
}
}
