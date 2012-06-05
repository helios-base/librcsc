// -*-c++-*-

/*!
  \file line_2d.h
  \brief 2D straight line Header File.
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

#ifndef RCSC_GEOM_LINE2D_H
#define RCSC_GEOM_LINE2D_H

#include <rcsc/geom/vector_2d.h>

#include <cmath>

namespace rcsc {

/*!
  \class Line2D
  \brief 2d straight line class

  Line Fomula: aX + bY + c = 0
*/
class Line2D {
public:

    static const double EPSILON; //!< tolerance threshold
    static const double ERROR_VALUE; //!< error value

private:

    double M_a; //!< line fomula A, coefficient for x
    double M_b; //!< line fomula B, coefficient for y
    double M_c; //!< line fomula constant C

    // never used
    Line2D();

public:
    /*!
      \brief construct directly
      \param a Line formula A, coefficient for x
      \param b Line formula B, coefficient for y
      \param c constant C
     */
    Line2D( const double a,
            const double b,
            const double c )
        : M_a( a ),
          M_b( b ),
          M_c( c )
      { }

    /*!
      \brief construct from 2 points
      \param p1 first point
      \param p2 second point
    */
    Line2D( const Vector2D & p1,
            const Vector2D & p2 )
      {
          assign( p1, p2 );
      }

    /*!
      \brief construct from origin point + direction
      \param origin origin point
      \param linedir direction from origin point
    */
    Line2D( const Vector2D & origin,
            const AngleDeg & linedir )
      {
          assign( origin, linedir );
      }

   /*!
      \brief construct from 2 points
      \param p1 first point
      \param p2 second point
      \return const reference to itself
    */
    const Line2D & assign( const Vector2D & p1,
                           const Vector2D & p2 )
      {
          M_a = -( p2.y - p1.y );
          M_b = p2.x - p1.x;
          M_c = -M_a * p1.x - M_b * p1.y;
          return *this;
      }

   /*!
      \brief construct from origin point + direction
      \param origin origin point
      \param linedir direction from origin point
      \return const reference to itself
    */
    const Line2D & assign( const Vector2D & origin,
                           const AngleDeg & linedir )
      {
          M_a = - linedir.sin();
          M_b = linedir.cos();
          M_c = -M_a * origin.x - M_b * origin.y;
          return *this;
      }

    /*!
      \brief accessor
      \return coefficient 'A' of line formula
    */
    double a() const
      {
          return M_a;
      }

    /*!
      \brief accessor
      \return coefficient 'B'  of line formula
    */
    double b() const
      {
          return M_b;
      }

    /*!
      \brief accessor
      \return coefficient 'C'  of line formula
    */
    double c() const
      {
          return M_c;
      }

    /*!
      \brief get X-coordinate correspond to 'y'
      \param y considered Y
      \return X coordinate
    */
    double getX( const double y ) const
      {
          if ( std::fabs( M_a ) < EPSILON )
          {
              return ERROR_VALUE;
          }
          return -( M_b * y + M_c ) / M_a;
      }

    /*!
      \brief get Y-coordinate correspond to 'x'
      \param x considered X
      \return Y coordinate
    */
    double getY( const double x ) const
      {
          if ( std::fabs( M_b ) < EPSILON )
          {
              return ERROR_VALUE;
          }

          return -( M_a * x + M_c ) / M_b;
      }

    /*!
      \brief calculate distance from point to this line
      \param p considrered point
      \return distance value
    */
    double dist( const Vector2D & p ) const
      {
          return std::fabs( ( M_a * p.x + M_b * p.y + M_c )
                            / std::sqrt( M_a * M_a + M_b * M_b ) );
      }
    /*!
      \brief get squared distance from this line to point
      \param p considrered point
      \return squared distance value
    */
    double dist2( const Vector2D & p ) const
      {
          double d = M_a * p.x + M_b * p.y + M_c;
          return (d * d) / (M_a * M_a + M_b * M_b);
      }

    /*!
      \brief check if the slope of this line is same to the slope of 'line'
      \param line considered line
      \retval true almost same
      \retval false not same
    */
    bool isParallel( const Line2D & line ) const
      {
          return std::fabs( a() * line.b() - line.a() * b() ) < EPSILON;
      }

    /*!
      \brief get the intersection point with 'line'
      \param line considered line
      \return intersection point. if it does not exist,
      the invaidated value vector is returned.
    */
    Vector2D intersection( const Line2D & line ) const
      {
          return intersection( *this, line );
      }

    /*!
      \brief calc perpendicular line (SUI-SEN)
      \param p the point that perpendicular line pass through
      \return perpendicular line
     */
    Line2D perpendicular( const Vector2D & p ) const
      {
          return Line2D( b(), -a(), a() * p.y - b() * p.x );
      }

    /*!
      \brief calc projection point from p
      \param p base point
      \return projection point
     */
    Vector2D projection( const Vector2D & p ) const
      {
          return intersection( perpendicular( p ) );
      }

    // static utility

    /*!
      \brief get the intersection point of 2 lines
      \param line1 the first line
      \param line2 the second line
      \return the intersection point.
      if no intersection, invalid vector is returned.
     */
    static
    Vector2D intersection( const Line2D & line1,
                           const Line2D & line2 );

    /*!
      \brief make angle bisector line from two angles
      \param origin origin point that is passed through by result line
      \param left left angle
      \param right right angle
      \return line object
     */
    static
    Line2D angle_bisector( const Vector2D & origin,
                           const AngleDeg & left,
                           const AngleDeg & right )
      {
          return Line2D( origin, AngleDeg::bisect( left, right ) );
      }

    /*!
      \brief make perpendicular bisector line from twt points
      \param p1 1st point
      \param p2 2nd point
      \return line object
     */
    static
    Line2D perpendicular_bisector( const Vector2D & p1,
                                   const Vector2D & p2 );

};

}

#endif
