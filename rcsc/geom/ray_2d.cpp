// -*-c++-*-

/*!
  \file ray_2d.cpp
  \brief 2D ray line class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#include "ray_2d.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
Ray2D::intersection( const Line2D & other ) const
{
    Line2D my_line = this->line();

    Vector2D tmp_sol = my_line.intersection( other );

    if ( ! tmp_sol.isValid() )
    {
        return Vector2D::INVALIDATED;
    }

    if ( ! inRightDir( tmp_sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    return tmp_sol;
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
Ray2D::intersection( const Ray2D & other ) const
{
    Vector2D tmp_sol = this->line().intersection( other.line() );

    if ( ! tmp_sol.isValid() )
    {
        return Vector2D::INVALIDATED;
    }

    if ( ! this->inRightDir( tmp_sol )
        || ! other.inRightDir( tmp_sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    return tmp_sol;

}

}
