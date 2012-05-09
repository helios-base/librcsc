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

#ifndef RCSC_COLOR_RGB_COLOR_H
#define RCSC_COLOR_RGB_COLOR_H

#include <string>

namespace rcsc {

/*!
  \class RGBColor
  \brief RGB color class
*/
class RGBColor {
private:
    //! red value
    double M_red;

    //! green value
    double M_green;

    //! blue value
    double M_blue;

public:
    /*
      \brief construct with black
    */
    RGBColor()
        : M_red( 0.0 ),
          M_green( 0.0 ),
          M_blue( 0.0 )
      { }

    /*
      \brief constructor from red/green/blue components
      \param r red value
      \param g green value
      \param b blue value
    */
    RGBColor( const double r,
              const double g,
              const double b );

    /*
      \brief retrieve red component
      \return red component value in range [0.0, 1.0]
    */
    double red() const
      {
          return M_red;
      }

    /*
      \brief retrieve green component
      \return green component value in range [0.0, 1.0]
    */
    double green() const
      {
          return M_green;
      }

    /*
      \brief retrieve blue component
      \return blue component value in range [0.0, 1.0]
    */
    double blue() const
      {
          return M_blue;
      }

    /*
      \brief retrieve red component in range [0, 255]
      \return red component value in range [0, 255]
    */
    int red8bit() const;

    /*
      \brief retrieve green component in range [0, 255]
      \return green component value in range [0, 255]
    */
    int green8bit() const;

    /*
      \brief retrieve blue component in range [0, 255]
      \return blue component value in range [0, 255]
    */
    int blue8bit() const;

    /*
      \brief get the color name string as "#RRGGBB"
      \return color name string
    */
    std::string name() const;

    /*
      \brief blend two colors
      \param c1 1st souece color to blend
      \param c2 2nd souece color to blend
      \param c1_rate rate of weight of 1st souece color
      \return blended color
    */
    static
    RGBColor blend( const RGBColor & c1,
                    const RGBColor & c2,
                    const double c1_rate );
};

}

#endif
