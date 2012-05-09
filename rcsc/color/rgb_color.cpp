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

#include <rcsc/math_util.h>

#include <cstdio>
// #include <sstream>
// #include <iomanip>

/*-------------------------------------------------------------------*/
/*!

 */
namespace {

inline
int
float_to_8bit( const double value )
{
    return rcsc::bound( 0, static_cast< int >( value * 256 ), 255 );
}
}

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
RGBColor::RGBColor( const double r,
                    const double g,
                    const double b )
    : M_red( bound( 0.0, r, 1.0 ) ),
      M_green( bound( 0.0, g, 1.0 ) ),
      M_blue( bound( 0.0, b, 1.0 ) )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
RGBColor
RGBColor::blend( const RGBColor & c1,
                 const RGBColor & c2,
                 const double c1_rate )
{
    const double c2_rate = 1.0 - c1_rate;

    return RGBColor( bound( 0.0, c1.red()   * c1_rate + c2.red()   * c2_rate, 1.0 ),
                     bound( 0.0, c1.green() * c1_rate + c2.green() * c2_rate, 1.0 ),
                     bound( 0.0, c1.blue()  * c1_rate + c2.blue()  * c2_rate, 1.0 ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::string
RGBColor::name() const
{
    char buf[8];
    snprintf( buf, 8, "#%02X%02X%02X", red8bit(), green8bit(), blue8bit() );
    return buf;

#if 0
    std::ostringstream buf;
    buf.setf( std::ios::hex, std::ios::basefield );
    buf.fill( '0' );

    buf << "#"
        << std::setw( 2 ) << red8bit()
        << std::setw( 2 ) << green8bit()
        << std::setw( 2 ) << blue8bit();

    return buf.str();
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
int
RGBColor::red8bit() const
{
    return float_to_8bit( red() );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
RGBColor::green8bit() const
{
    return float_to_8bit( green() );
}

/*-------------------------------------------------------------------*/
/*!

 */
int
RGBColor::blue8bit() const
{
    return float_to_8bit( blue() );
}

}
