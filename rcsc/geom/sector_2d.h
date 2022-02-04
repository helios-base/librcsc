// -*-c++-*-

/*!
  \file sector_2d.h
  \brief 2D sector region Header File.
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

#ifndef RCSC_GEOM_SECTOR2D_H
#define RCSC_GEOM_SECTOR2D_H

#include <rcsc/geom/region_2d.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

/*!
  \class Sector2D
  \brief 2D sector region(OUGI-GATA) class
 */
class Sector2D
    : public Region2D {
private:
    //! center point
    Vector2D M_center;

    //! smaller radius
    double M_min_radius;
    //! bigger radius
    double M_max_radius;

    //! start angle. (turn clockwise)
    AngleDeg M_angle_left_start;
    //! end angle. (turn clockwise)
    AngleDeg M_angle_right_end;

    //! not used
    Sector2D() = delete;
public:

    /*!
      \brief constructor with all variables
      \param c center point
      \param min_r smaller radius
      \param max_r bigger radius
      \param start start angle(turn clockwise)
      \param end end angle(turn clockwise)
    */
    Sector2D( const Vector2D & c,
              const double min_r,
              const double max_r,
              const AngleDeg & start,
              const AngleDeg & end );

   /*!
      \brief assign new value
      \param c center point
      \param min_r smaller radius
      \param max_r bigger radius
      \param start start angle(turn clockwise)
      \param end end angle(turn clockwise)
    */
    const Sector2D & assign( const Vector2D & c,
                             const double min_r,
                             const double max_r,
                             const AngleDeg & start,
                             const AngleDeg & end );

    /*!
      \brief get the center point
      \return const reference to the member variable
     */
    const Vector2D & center() const
      {
          return M_center;
      }

    /*!
      \brief get the small side radius
      \return const reference to the member variable
     */
    double radiusMin() const
      {
          return M_min_radius;
      }

    /*!
      \brief get the big side radius
      \return const reference to the member variable
     */
    double radiusMax() const
      {
          return M_max_radius;
      }

    /*!
      \brief get the left start angle
      \return const reference to the member variable
     */
    const AngleDeg & angleLeftStart() const
      {
          return M_angle_left_start;
      }

    /*!
      \brief get the right end angle
      \return const reference to the member variable
     */
    const AngleDeg & angleRightEnd() const
      {
          return M_angle_right_end;
      }

    /*!
      \brief calculate the area of this region
      \return the value of area
     */
    virtual
    double area() const;

    /*!
      \brief check if point is within this region
      \param point considered point
      \return true or false
     */
    virtual
    bool contains( const Vector2D & point ) const
      {
          Vector2D rel = point - center();
          double d2 = rel.r2();
          return ( M_min_radius * M_min_radius <= d2
                   && d2 <= M_max_radius * M_max_radius
                   && rel.th().isWithin( M_angle_left_start,
                                         M_angle_right_end ) );
      }

    /*!
      \brief get smaller side circumference(ENSYUU NO NAGASA)
      \return the length of circumference
    */
    double getCircumferenceMin() const
      {
          double div = ( M_angle_right_end - M_angle_left_start ).degree();
          if ( div < 0.0 )
          {
              div += 360.0;
          }
          return ( 2.0 * M_min_radius * M_PI ) * ( div / 360.0 );
      }

    /*!
      \brief get bigger side circumference(ENSYUU NO NAGASA)
      \return the length of circumference
    */
    double getCircumferenceMax() const
      {
          double div = ( M_angle_right_end - M_angle_left_start ).degree();
          if ( div < 0.0 )
          {
              div += 360.0;
          }
          return ( 2.0 * M_max_radius * M_PI ) * ( div / 360.0 );
      }
};

}

#endif
