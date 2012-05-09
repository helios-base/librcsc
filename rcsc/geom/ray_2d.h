// -*-c++-*-

/*!
  \file ray_2d.h
  \brief 2D ray line class Header File.
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

#ifndef RCSC_GEOM_RAY2D_H
#define RCSC_GEOM_RAY2D_H

#include <rcsc/geom/line_2d.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class Ray2D
  \brief 2D ray line class
*/
class Ray2D {
private:
    //! ray's origin point
    Vector2D M_origin;
    //! ray's direction.
    AngleDeg M_direction;

public:
    /*!
      \brief defalut constructor. all values are set to 0.
     */
    Ray2D()
        : M_origin( 0.0, 0.0 ),
          M_direction( 0.0 )
      { }

    /*!
      \brief constructor with origin and direction
      \param origin origin point
      \param direction direction angle
     */
    Ray2D( const Vector2D & origin,
           const AngleDeg & direction )
        : M_origin( origin ),
          M_direction( direction )
      { }
    /*!
      \brief constructor with origin and other point
      \param origin origin point
      \param dir_point direction point
     */
    Ray2D( const Vector2D & origin,
           const Vector2D & dir_point )
        : M_origin( origin ),
          M_direction( ( dir_point - origin ).th() )
      { }

    /*!
      \brief get origin point
      \return const referenct to the member variable
     */
    const Vector2D & origin() const
      {
          return M_origin;
      }

    /*!
      \brief get the angle of this ray line
      \return const referenct to the member variable
     */
    const AngleDeg & dir() const
      {
          return M_direction;
      }

    /*!
      \brief get line generated from this ray
      \return new line object
    */
    Line2D line() const
      {
          return Line2D( origin(), dir() );
      }

    /*!
      \brief check whether p is on the direction of this Ray
      \param point considered point
      \param thr threshold angle buffer
      \return true or false
    */
    bool inRightDir( const Vector2D & point,
                     const double thr = 10.0 ) const
      {
          return ( ( point - origin() ).th() - dir() ).abs() < thr;
      }

    /*!
      \brief get the intersection point with 'line'
      \param other considered line
      \return intersection point. if it does not exist,
      the invaidated value vector is returned.
    */
    Vector2D intersection( const Line2D & other ) const;

    /*!
      \brief get the intersection point with 'ray'
      \param other considered ray
      \return intersection point. if it does not exist,
      the invaidated value vector is returned.
     */
    Vector2D intersection( const Ray2D & other ) const;

};

}

#endif
