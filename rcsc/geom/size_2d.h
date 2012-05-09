// -*-c++-*-

/*!
  \file size_2d.h
  \brief 2d size class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

#ifndef RCSC_GEOM_SIZE2D_H
#define RCSC_GEOM_SIZE2D_H

#include <ostream>
#include <cmath>

namespace rcsc {

/*!
  \class Size2D
  \brief 2D size definition class
 */
class Size2D {
private:
    //! x range
    double M_length;

    //! y range
    double M_width;

public:

    /*!
      \brief default constructor.
    */
    Size2D()
        : M_length( 0.0 ),
          M_width( 0.0 )
      { }

    /*!
      \brief constructor with variables
      \param length x range
      \param width y range
     */
    Size2D( const double length,
            const double width )
        : M_length( std::fabs( length ) ),
          M_width( std::fabs( width ) )
      { }

    /*!
      \brief assign new range directly.
      \param length new X range
      \param width new Y range
      \return reference to itself
    */
    const Size2D & assign( const double length,
                           const double width )
      {
          M_length = std::fabs( length );
          M_width = std::fabs( width );
          return *this;
      }

    /*!
      \brief set new X range
      \param length new X range
      \return reference to itself
    */
    const Size2D & setLength( const double length )
      {
          M_length = std::fabs( length );
          return *this;
      }

    /*!
      \brief set new Y range
      \param width new Y range
      \return reference to itself
    */
    const Size2D & setWidth( const double width )
      {
          M_width = std::fabs( width );
          return *this;
      }

    /*!
      \brief get the value of X range
      \return value of X range
     */
    double length() const
      {
          return M_length;
      }

    /*!
      \brief get the value of Y range
      \return value of Y range
     */
    double width() const
      {
          return M_width;
      }

    /*!
      \brief get the length of diagonal line
      \return length of diagonal line
     */
    double diagonal() const
      {
          return std::sqrt( length() * length()
                            + width() * width() );
      }

    /*!
      \brief output values to stream.
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const
      {
          os << "(" << length() << ", " << width() << ")";
          return os;
      }

};

} // end of namespace

#endif
