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

#include <iostream>
#include <cstdlib>

using namespace rcsc;

namespace {
static const double DELTA = 0.001;

static const double MAG = 500.0;
static void print_postscript_rect( double start_x,
                                   double end_x,
                                   const RGBColor & c );

static void print_postscript_header();
static void print_postscript_footer();
}

int
main( void )
{
    const ThermoColorProvider thermo;

    print_postscript_header();

    for ( double value = 0.0; value <= 1.0 - DELTA; value += DELTA )
    {
        const RGBColor c = thermo.convertToColor( value );

        print_postscript_rect( value, value + DELTA, c );
    }

    print_postscript_footer();

    return EXIT_SUCCESS;
}


namespace {
static
void
print_postscript_header()
{
    std::cout << "%!PS-Adobe-3.0" << std::endl;
    std::cout << "%%BoundingBox: 0.0 0.0 " << 1.0 * MAG << " " << 1.0 * MAG
              << std::endl;
}

static
void
print_postscript_footer()
{
    std::cout << "showpage" << std::endl;
}

static
void
print_postscript_rect( double start_x,
                       double end_x,
                       const RGBColor & c )
{
    std::cout << c.red() << " " << c.green() << " " << c.blue()
              << " setrgbcolor" << std::endl
              << "newpath" << std::endl
              << start_x * MAG << " " << 0.0 * MAG  << " moveto" << std::endl
              << end_x   * MAG << " " << 0.0 * MAG  << " lineto" << std::endl
              << end_x   * MAG << " " << 1.0 * MAG  << " lineto" << std::endl
              << start_x * MAG << " " << 1.0 * MAG  << " lineto" << std::endl
              << "closepath" << std::endl
              << "fill" << std::endl
              << std::endl;
}

}
