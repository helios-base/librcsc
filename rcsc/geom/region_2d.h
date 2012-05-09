// -*-c++-*-

/*!
  \file region_2d.h
  \brief abstract 2D region class Header File.
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

#ifndef RCSC_GEOM_REGION2D_H
#define RCSC_GEOM_REGION2D_H

namespace rcsc {

class Vector2D;

/*!
  \class Region2D
  \brief abstract 2D region class
*/
class Region2D {
protected:

    /*!
      \brief accessible only from derived classes
     */
    Region2D()
      { }

public:

    /*!
      \brief virtual destructor.
     */
    virtual
    ~Region2D()
      { }

    /*!
      \brief get the area of this region
      \return value of the area
     */
    virtual
    double area() const = 0;

    /*!
      \brief check if this region contains 'point'.
      \param point considerd point
      \return true or false
    */
    virtual
    bool contains( const Vector2D & point ) const = 0;

};

}

#endif
