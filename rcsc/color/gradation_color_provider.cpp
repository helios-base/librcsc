// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gradation_color_provider.h"

#include <rcsc/math_util.h>

#include <vector>


namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
void
GradationColorProvider::addColor( const RGBColor & color )
{
    M_colors.push_back( color );
}

/*-------------------------------------------------------------------*/
/*!

*/
RGBColor
GradationColorProvider::convertToColor( const double value ) const
{
    const int n_colors = M_colors.size();

    //
    // check color list size
    //
    if ( n_colors < 2 )
    {
        if ( n_colors == 1 )
        {
            return M_colors[0];
        }
        else
        {
            return RGBColor();
        }
    }

    //
    // calculate color index and rate
    //
    const int n_color_range = n_colors - 1;
    const double rate_split_width = 1.0 / n_color_range;
    const int n = bound( 0, static_cast< int >( value / rate_split_width ), n_color_range );
    const double rate = bound( 0.0, ( value - rate_split_width * n ) * n_color_range, 1.0 );

    //
    // blend colors
    //
    const RGBColor & c1 = M_colors[ n ];
    const RGBColor & c2 = M_colors[ n + 1 ];

    return RGBColor::blend( c2, c1, rate );
}

}
