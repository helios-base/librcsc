// -*-c++-*-

/*!
  \file circle_2d.h
  \brief 2D circle region Header File.
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

#ifndef RCSC_GEOM_CIRCLE2D_H
#define RCSC_GEOM_CIRCLE2D_H

#include <iostream>

#include <rcsc/geom/region_2d.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

class Line2D;
class Ray2D;
class Segment2D;

/*!
  \class Circle2D
  \brief 2d circle class
 */
class Circle2D
    : public Region2D {
private:

    //! center point
    Vector2D M_center;

    //! radius of this circle
    double M_radius;

    static const double EPSILON;


public:
    /*!
      \brief create a zero area circle at (0,0)
     */
    Circle2D()
        : M_center( 0.0, 0.0 ),
          M_radius( 0.0 )
      { }

    /*!
      \brief construct with center point and radius value.
      \param c center point
      \param r radius value
     */
    Circle2D( const Vector2D & c,
              const double r )
        : M_center( c ),
          M_radius( r )
      {
          if ( r < 0.0 )
          {
              std::cerr << "Circle2D::Circle2D(). radius must be positive value."
                        << std::endl;
              M_radius = 0.0;
          }
      }

    /*!
      \brief assign new value.
      \param c center point
      \param r radius value
      \return const reference to this
     */
    const Circle2D & assign( const Vector2D & c,
                       const double r )
      {
          M_center = c;
          M_radius = r;
          if ( r < 0.0 )
          {
              std::cerr << "Circle2D::assign(). radius must be positive value."
                        << std::endl;
              M_radius = 0.0;
          }
          return *this;
      }

    /*!
      \brief get the area value of this circle
      \return value of the area
     */
    virtual
    double area() const
      {
          return AngleDeg::PI * M_radius * M_radius;
      }

    /*!
      \brief check if point is within this region
      \param point considered point
      \return true if point is contained by this circle
     */
    virtual
    bool contains( const Vector2D & point ) const
      {
          return M_center.dist2( point ) < M_radius * M_radius;
      }

    /*!
      \brief get the center point
      \return center point coordinate value
     */
    const Vector2D & center() const
      {
          return M_center;
      }

    /*!
      \brief get the radius value
      \return radius value
     */
    double radius() const
      {
          return M_radius;
      }

    /*!
      \brief caluclate the intersection with straight line
      \param line considerd line
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return the number of solution
     */
    int intersection( const Line2D & line,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;

    /*!
      \brief calculate the intersection with ray line
      \param ray considerd ray
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return the number of solution
     */
    int intersection( const Ray2D & ray,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;

    /*!
      \brief calculate the intersection with segment line
      \param segment considerd segment line
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return the number of solution
     */
    int intersection( const Segment2D & segment,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;

    /*!
      \brief calculate the intersection with another circle
      \param circle considerd circle
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return the number of solution
     */
    int intersection( const Circle2D & circle,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;

    // static utility

    /*!
      \brief get the circle through three points (circumcircle of the triangle).
      \param p0 triangle's 1st vertex
      \param p1 triangle's 2nd vertex
      \param p2 triangle's 3rd vertex
      \return coordinates of circumcenter
    */
    static
    Circle2D circumcircle( const Vector2D & p0,
                           const Vector2D & p1,
                           const Vector2D & p2 );

    /*!
      \brief check if the circumcircle contains the input point
      \param point input point
      \param p0 triangle's 1st vertex
      \param p1 triangle's 2nd vertex
      \param p2 triangle's 3rd vertex
      \return true if circumcircle contains the point, otherwise false.
    */
    static
    bool contains( const Vector2D & point,
                   const Vector2D & p0,
                   const Vector2D & p1,
                   const Vector2D & p2 );
};

}

#endif
