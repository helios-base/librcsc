// -*-c++-*-

/*!
  \file line_2d.cpp
  \brief 2D straight line class Source File.
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

#include "line_2d.h"

#include <iostream>
#include <limits>

namespace rcsc {

const double Line2D::EPSILON = 1.0e-10;
const double Line2D::ERROR_VALUE = std::numeric_limits< double >::max();

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
Line2D::intersection( const Line2D & line1,
                      const Line2D & line2 )
{
    double tmp = line1.a() * line2.b() - line1.b() * line2.a();
    if ( std::fabs( tmp ) < EPSILON )
    {
        return Vector2D::INVALIDATED;
    }

    return Vector2D( (line1.b() * line2.c() - line2.b() * line1.c()) / tmp,
                     (line2.a() * line1.c() - line1.a() * line2.c()) / tmp );
}

/*-------------------------------------------------------------------*/
/*!

 */
Line2D
Line2D::perpendicular_bisector( const Vector2D & p1,
                                const Vector2D & p2 )
{
    if( std::fabs( p2.x - p1.x ) < EPSILON
        && std::fabs( p2.y - p1.y ) < EPSILON )
    {
        // input points have same coordiate values.
        std::cerr << "(Line2D::perpendicular_bisector)"
                  << " ***ERROR*** input points have same coordinate values "
                  << p1 << p2
                  << std::endl;
        return Line2D( p1, Vector2D( p1.x + 1.0, p1.y ) );
    }

    double tmp = ( p2.x*p2.x - p1.x*p1.x
                   + p2.y*p2.y - p1.y*p1.y ) * -0.5 ;
    return Line2D( p2.x - p1.x,
                   p2.y - p1.y,
                   tmp );
}

}
