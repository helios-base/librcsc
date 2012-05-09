// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#ifndef RCSC_COLOR_GRAY_SCALE_PROVIDER_H
#define RCSC_COLOR_GRAY_SCALE_PROVIDER_H

#include <rcsc/color/gradation_color_provider.h>

namespace rcsc {

/*!
  \class ThermoColorProvider
  \brief thermo color provider class for converting float value to a color
*/
class GrayScaleProvider
    : public GradationColorProvider {
public:

    /*
      \brief constructor
     */
    GrayScaleProvider();
};

}

#endif
