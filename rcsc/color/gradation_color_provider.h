// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA, Hidehisa Akiyama

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

#ifndef RCSC_COLOR_GRADATION_COLOR_PROVIDER_H
#define RCSC_COLOR_GRADATION_COLOR_PROVIDER_H

#include <rcsc/color/rgb_color.h>

#include <vector>

namespace rcsc {

/*!
  \class GradationColorProvider
  \brief gradation color provider class for converting float value to a color
*/
class GradationColorProvider {
private:

    //! color set for gradation
    std::vector< RGBColor > M_colors;

    // not used
    GradationColorProvider( const GradationColorProvider & ) = delete;
    GradationColorProvider & operator=( const GradationColorProvider & ) = delete;

protected:

    /*!
      \brief protected constructor
     */
    GradationColorProvider() = default;

    /*!
      \brief add new color that means the highest value.
      \param color color value.
     */
    void addColor( const RGBColor & color );

public:

    /*!
      \brief virtual destructor.
     */
    virtual
    ~GradationColorProvider() = default;

    /*
      \brief convert [0.0, 1.0] value to a color
      \param value value to convert, value range should be in [0.0, 1.0].
      \return converted color
     */
    RGBColor convertToColor( const double value ) const;
};

}

#endif
