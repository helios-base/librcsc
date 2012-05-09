// -*-c++-*-

/*!
  \file sector_2d.cpp
  \brief 2D sector region Source File.
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

#include "sector_2d.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
Sector2D::Sector2D( const Vector2D & c,
                    const double min_r,
                    const double max_r,
                    const AngleDeg & start,
                    const AngleDeg & end )
    : M_center( c ),
      M_min_radius( min_r ),
      M_max_radius( max_r ),
      M_angle_left_start( start ),
      M_angle_right_end( end )
{
    if ( min_r < 0.0 )
    {
        std::cerr << "Sector2D::Sector2D() radius must be positive value."
                  << std::endl;
        M_min_radius = 0.0;
    }
    if ( M_min_radius > M_max_radius )
    {
        std::cerr << "Sector2D::Sector2D(): max radius must be bigger than min radius."
                  << std::endl;
        M_max_radius = M_min_radius;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
const Sector2D &
Sector2D::assign( const Vector2D & c,
                  const double min_r,
                  const double max_r,
                  const AngleDeg & start,
                  const AngleDeg & end )
{
    M_center = c;
    M_min_radius = min_r;
    M_max_radius = max_r;
    M_angle_left_start = start;
    M_angle_right_end = end;

    if ( min_r < 0.0 )
    {
        std::cerr << "Sector2D::assign() radius must be positive value."
                  << std::endl;
        M_min_radius = 0.0;
    }
    if ( min_r > max_r )
    {
        std::cerr << "Sector2D::assign() max radius must be bigger than min radius."
                  << std::endl;
        M_max_radius = M_min_radius;
    }

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Sector2D::area() const
{
    double circle_area
        = ( radiusMax() * radiusMax() * M_PI )
        - ( radiusMin() * radiusMin() * M_PI );
    double angle_width
        = ( angleRightEnd() - angleLeftStart() ).degree();
    if ( angle_width < 0.0 )
    {
        angle_width += 360.0;
    }

    circle_area *= ( angle_width / 360.0 );
    return circle_area;
}

}
