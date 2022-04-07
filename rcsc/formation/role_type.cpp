// -*-c++-*-

/*!
  \file role_tye.cpp
  \brief role type information class Source File.
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

#include "role_type.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
std::string
RoleType::to_string( const Type t )
{
    return ( t == Goalie ? "G"
             : t == Defender ? "DF"
             : t == MidFielder ? "MF"
             : t == Forward ? "FW"
             : "Unknown" );
}

/*-------------------------------------------------------------------*/
std::string
RoleType::to_string( const Side s )
{
    return ( s == RoleType::Center ? "C"
             : s == RoleType::Left ? "L"
             : s == RoleType::Right ? "R"
             : "U" );
}

/*-------------------------------------------------------------------*/
RoleType::Type
RoleType::to_type( const std::string & str )
{
    return ( str == "G" ? Goalie
             : str == "DF" ? Defender
             : str == "MF" ? MidFielder
             : str == "FW" ? Forward
             : Unknown );
}

/*-------------------------------------------------------------------*/
RoleType::Side
RoleType::to_side( const std::string & str )
{
    return ( str == "L" ? Left
             : str == "R" ? Right
             : Center );
}

}
