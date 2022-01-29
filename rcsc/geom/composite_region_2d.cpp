// -*-c++-*-

/*!
  \file composite_region_2d.cpp
  \brief composite 2D region class Source File.
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

#include "composite_region_2d.h"

#include "vector_2d.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
double
UnitedRegion2D::area() const
{
    return 0.0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
UnitedRegion2D::contains( const Vector2D & point ) const
{
    for ( const std::shared_ptr< const Region2D > & r : M_regions )
    {
        if ( r->contains( point ) )
        {
            return true;
        }
    }

    return false;
}


/*-------------------------------------------------------------------*/
/*!

*/
double
IntersectedRegion2D::area() const
{
    return 0.0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
IntersectedRegion2D::contains( const Vector2D & point ) const
{
    for ( const std::shared_ptr< const Region2D > & r : M_regions )
    {
        if ( ! r->contains( point ) )
        {
            return false;
        }
    }

    return true;
}

}
