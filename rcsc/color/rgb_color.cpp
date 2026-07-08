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

#include "rgb_color.h"

#include <algorithm>
#include <cstdio>

/*-------------------------------------------------------------------*/
/*!

 */
namespace {

inline
int
to_8bit( const double value )
{
    return std::clamp( static_cast< int >( value * 256 ), 0, 255 );
}

}

namespace rcsc {

/*-------------------------------------------------------------------*/
RGBColor::RGBColor( const double r,
                    const double g,
                    const double b,
                    const double a )
    : M_red( std::clamp( r, 0.0, 1.0 ) ),
      M_green( std::clamp( g, 0.0, 1.0 ) ),
      M_blue( std::clamp( b, 0.0, 1.0 ) ),
      M_alpha( std::clamp( a, 0.0, 1.0 ) )
{

}

/*-------------------------------------------------------------------*/
RGBColor
RGBColor::blend( const RGBColor & c1,
                 const RGBColor & c2,
                 const double c1_rate )
{
    const double c2_rate = 1.0 - c1_rate;

    return RGBColor( c1.red()   * c1_rate + c2.red()   * c2_rate,
                     c1.green() * c1_rate + c2.green() * c2_rate,
                     c1.blue()  * c1_rate + c2.blue()  * c2_rate,
                     c1.alpha() * c1_rate + c2.alpha() * c2_rate );
}

/*-------------------------------------------------------------------*/
RGBColor
RGBColor::blend( const RGBColor & c1,
                 const RGBColor & c2,
                 const double c1_rate,
                 const double alpha )
{
    const double c2_rate = 1.0 - c1_rate;

    return RGBColor( c1.red()   * c1_rate + c2.red()   * c2_rate,
                     c1.green() * c1_rate + c2.green() * c2_rate,
                     c1.blue()  * c1_rate + c2.blue()  * c2_rate,
                     alpha );
}

/*-------------------------------------------------------------------*/
std::string
RGBColor::name() const
{
    char buf[10];

    if ( alpha() == 1.0 )
    {
        snprintf( buf, sizeof( buf ), "#%02X%02X%02X", red8bit(), green8bit(), blue8bit() );
    }
    else
    {
        snprintf( buf, sizeof( buf ), "#%02X%02X%02X%02X", alpha8bit(), red8bit(), green8bit(), blue8bit() );
    }

    return buf;
}

/*-------------------------------------------------------------------*/
int
RGBColor::red8bit() const
{
    return to_8bit( red() );
}

/*-------------------------------------------------------------------*/
int
RGBColor::green8bit() const
{
    return to_8bit( green() );
}

/*-------------------------------------------------------------------*/
int
RGBColor::blue8bit() const
{
    return to_8bit( blue() );
}

/*-------------------------------------------------------------------*/
int
RGBColor::alpha8bit() const
{
    return to_8bit( alpha() );
}

}
