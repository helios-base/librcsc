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

#include "thermo_color_provider.h"

#include "gradation_color_provider.h"
#include "rgb_color.h"

#include <vector>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
ThermoColorProvider::ThermoColorProvider()
    : GradationColorProvider()
{
    addColor( RGBColor( 0.0, 0.0, 0.0 ) ); // black
    addColor( RGBColor( 0.0, 0.0, 1.0 ) ); // blue
    addColor( RGBColor( 0.0, 1.0, 1.0 ) ); // light blue
    addColor( RGBColor( 0.0, 1.0, 0.0 ) ); // green
    addColor( RGBColor( 1.0, 1.0, 0.0 ) ); // yellow
    addColor( RGBColor( 1.0, 0.0, 0.0 ) ); // red
}

}
