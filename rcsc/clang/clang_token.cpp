// -*-c++-*-

/*!
  \file clang_token.cpp
  \brief clang token class Source File
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

#include "clang_token.h"

#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CLangTokenClear::print( std::ostream & os ) const
{
    os << "(clear)";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
CLangTokenRule::print( std::ostream & os ) const
{
    os << '(' << ttl() << ' ';
    if ( M_condition )
    {
        os << *M_condition;
    }
    else
    {
        os << "(null)";
    }

    if ( M_directives.empty() )
    {
        os << "(null)";
    }
    else
    {
        for ( const auto & dir : M_directives )
        {
            if ( dir )
            {
                os << ' ' << *dir;
            }
            else
            {
                os << " (null)";
            }
        }
    }

    os << ')';
    return os;
}


}
