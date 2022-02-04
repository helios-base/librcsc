// -*-c++-*-

/*!
  \file segment_intersection.cpp
  \brief 2D segment line class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama, Hiroki Shimora

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

#include "segment_intersection.h"

#include <algorithm>
#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
void
BruteForceSegmentIntersectionDetector::execute( const std::vector< Segment2D > & segments,
                                                std::vector< SegmentIntersection > * intersections ) const
{
    const size_t size = segments.size();

    for ( size_t i = 0; i < size - 1; ++i )
    {
        const Segment2D & s_i = segments[i];

        for ( size_t j = i + 1; j < size; ++j )
        {
            if ( s_i.intersects( segments[j] ) )
            {
                intersections->emplace_back( s_i, segments[j] );
            }
        }
    }
}

}
