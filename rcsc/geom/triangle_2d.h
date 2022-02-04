// -*-c++-*-

/*!
  \file triangle_2d.h
  \brief 2D triangle class Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama, Hiroki Shimora

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

#ifndef RCSC_GEOM_TRIANGLE2D_H
#define RCSC_GEOM_TRIANGLE2D_H

#include <rcsc/geom/region_2d.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>

namespace rcsc {

class Line2D;
class Ray2D;

/*!
  \class Triangle2D
  \brief 2D triangle class
*/
class Triangle2D
    : public Region2D {
private:
    Vector2D M_a; //!< first vertex point
    Vector2D M_b; //!< second vertex point
    Vector2D M_c; //!< third vertex point

    //! not used
    Triangle2D() = delete;
public:
    /*!
      \brief constructor with variables
      \param v1 first vertex point
      \param v2 second vertex point
      \param v3 third vertex point
    */
    Triangle2D( const Vector2D & v1,
                const Vector2D & v2,
                const Vector2D & v3 )
        : M_a( v1 ),
          M_b( v2 ),
          M_c( v3 )
      { }

    /*!
      \brief constructor with a segment and a point
      \param seg segment consist of triangle, first and second vertex points
      \param v third vertex point
    */
    Triangle2D( const Segment2D & seg,
                const Vector2D & v )
        : M_a( seg.origin() ),
          M_b( seg.terminal() ),
          M_c( v )
      { }

    /*!
      \brief assign new vertex points
      \param v1 first vertex point
      \param v2 second vertex point
      \param v3 third vertex point
      \return const reference to itself
    */
    const Triangle2D & assign( const Vector2D & v1,
                               const Vector2D & v2,
                               const Vector2D & v3 )
      {
          M_a = v1;
          M_b = v2;
          M_c = v3;
          return *this;
      }

    /*!
      \brief check if this triangle is valid or not.
      \return true if triangle is valid.
    */
    bool isValid() const
      {
          return M_a.isValid()
              && M_b.isValid()
              && M_c.isValid()
              && M_a != M_b
              && M_b != M_c
              && M_a != M_a;
      }

    /*!
      \brief assign new segment and vertex point
      \param seg segment consist of triangle, first and second vertex points
      \param v third vertex point
      \return const reference to itself
    */
    const Triangle2D & assign( const Segment2D & seg,
                               const Vector2D & v )
      {
          M_a = seg.origin();
          M_b = seg.terminal();
          M_c = v;
          return *this;
      }

    /*!
      \brief get 1st point
      \return const reference to the member variable
     */
    const Vector2D & a() const
      {
          return M_a;
      }

    /*!
      \brief get 2nd point
      \return const reference to the member variable
     */
    const Vector2D & b() const
      {
          return M_b;
      }

    /*!
      \brief get 3rd point
      \return const reference to the member variable
     */
    const Vector2D & c() const
      {
          return M_c;
      }

    /*!
      \brief get the area of this region
      \return value of the area
     */
    virtual
    double area() const
      {
          // outer product == area of parallelogram(Heikou Shihenkei)
          // triangle area is a half of parallelogram area
          return std::fabs( ( b() - a() ).outerProduct( c() - a() ) ) * 0.5;
      }

    /*!
      \brief get a signed area. this method is equivalent to signed_area().
      \return signed area value
      If points a, b, c are placed counterclockwise order, returns positive number.
      If points a, b, c are placed clockwise order, returns negative number.
      If points a, b, c are placed on a line, returns 0.
     */
    double signedArea() const
      {
          //return doubleSignedArea() / 2.0;
          return signed_area( a(), b(), c() );
      }

    /*!
      \brief get a double of signed area value. this method is equivalent to double_signed_area().
      \return double of signed area value
      If points a, b, c are placed counterclockwise order, returns positive number.
      If points a, b, c are placed clockwise order, returns negative number.
      If points a, b, c are placed on a line, returns 0.
     */
    double doubleSignedArea() const
      {
          //return ( ( a().x - c().x ) * ( b().y - c().y )
          //         + ( b().x - c().x ) * ( c().y - a().y ) );
          return double_signed_area( a(), b(), c() );
      }

    /*!
      \brief check if this triangle's vertices are placed counterclockwise order.
      \return checked result
     */
    bool ccw() const
      {
          return ccw( a(), b(), c() );
      }

    /*!
      \brief check if this triangle contains 'point'.
      \param point considerd point
      \return true or false
    */
    virtual
    bool contains( const Vector2D & point ) const;

    /*!
      \brief get the center of gravity(centroid, JUU-SIN)
      \return coordinates of gravity center
     */
    Vector2D centroid() const
      {
          return centroid( a(), b(), c() );
      }

    /*!
      \brief get the center of inscribed circle(NAI-SIN)
      \return coordinates of inner center
    */
    Vector2D incenter() const
      {
          return incenter( a(), b(), c() );
      }

    /*!
      \brief get the center of circumscribed circle(GAI-SIN)
      \return coordinates of outer center
    */
    Vector2D circumcenter() const
      {
          return circumcenter( a(), b(), c() );
      }

    /*!
      \brief get the orthocenter(SUI-SIN)
      \return coordinates of ortho center
    */
    Vector2D orthocenter() const
      {
          return orthocenter( a(), b(), c() );
      }

    //brief get the excenter coordinates(BOU-SIN)
    //return coordinates of excenter
    //Vector2D excenter() const;

    /*!
      \brief calculate intersection point with line.
      \param line considerd line.
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return number of intersection
    */
    int intersection( const Line2D & line,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;

    /*!
      \brief calculate intersection point with ray.
      \param ray considerd ray line.
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return number of intersection
    */
    int intersection( const Ray2D & ray,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;

    /*!
      \brief calculate intersection point with line segment.
      \param segment considerd line segment.
      \param sol1 pointer to the 1st solution variable
      \param sol2 pointer to the 2nd solution variable
      \return number of intersection
    */
    int intersection( const Segment2D & segment,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const;


    //
    // static methods
    //

    /*!
      \brief get a double signed area value (== area of parallelogram)
      \param a 1st input point
      \param b 2nd input point
      \param c 3rd input point
      \return double sined area value.
      If points a, b, c are placed counterclockwise order, returns positive number.
      If points a, b, c are placed clockwise order, returns negative number.
      If points a, b, c are placed on a line, returns 0.
     */
    static
    double double_signed_area( const Vector2D & a,
                               const Vector2D & b,
                               const Vector2D & c )
      {
          return ( ( a.x - c.x ) * ( b.y - c.y )
                   + ( b.x - c.x ) * ( c.y - a.y ) );
      }

    /*!
      \brief get a signed area value
      \param a 1st input point
      \param b 2nd input point
      \param c 3rd input point
      \return signed area value
      If points a, b, c are placed counterclockwise order, returns positive number.
      If points a, b, c are placed clockwise order, returns negative number.
      If points a, b, c are placed on a line, returns 0.
     */
    static
    double signed_area( const Vector2D & a,
                        const Vector2D & b,
                        const Vector2D & c )
      {
          return double_signed_area( a, b, c ) * 0.5;
      }

    /*!
      \brief check if input vertices are placed counterclockwise order.
      \param a 1st input point
      \param b 2nd input point
      \param c 3rd input point
      \return checked result
     */
    static
    bool ccw( const Vector2D & a,
              const Vector2D & b,
              const Vector2D & c )
      {
          return double_signed_area( a, b, c ) > 0.0;
      }

    /*!
      \brief get the center of gravity(JUU-SIN)
      \param a triangle's 1st vertex
      \param b triangle's 2nd vertex
      \param c triangle's 3rd vertex
      \return coordinates of gravity center

      centroid = (a + b + c) / 3
     */
    static
    Vector2D centroid( const Vector2D & a,
                       const Vector2D & b,
                       const Vector2D & c )
      {
          //Vector2D g( a );
          //g += b;
          //g += c;
          //g /= 3.0;
          //return g;
          return Vector2D( a ).add( b ).add( c ) /= 3.0;
      }

    /*!
      \brief get the incenter point(NAI-SIN)
      \param a triangle's 1st vertex
      \param b triangle's 2nd vertex
      \param c triangle's 3rd vertex
      \return coordinates of incenter
     */
    static
    Vector2D incenter( const Vector2D & a,
                       const Vector2D & b,
                       const Vector2D & c );

    /*!
      \brief get the circumcenter point(GAI-SIN)
      \param a triangle's 1st vertex
      \param b triangle's 2nd vertex
      \param c triangle's 3rd vertex
      \return coordinates of circumcenter
     */
    static
    Vector2D circumcenter( const Vector2D & a,
                           const Vector2D & b,
                           const Vector2D & c );

    /*!
      \brief get the orthomcenter point(SUI-SIN)
      \param a triangle's 1st vertex
      \param b triangle's 2nd vertex
      \param c triangle's 3rd vertex
      \return coordinates of orthocenter

      orthocenter = a + b + c - 2 * circumcenter
     */
    static
    Vector2D orthocenter( const Vector2D & a,
                          const Vector2D & b,
                          const Vector2D & c );

    /*!
      \brief check if triangle(a,b,c) contains the point 'p'.
      \param a vertex1
      \param b vertex2
      \param c vertex3
      \param point checked point
      \return checked result
     */
    static
    bool contains( const Vector2D & a,
                   const Vector2D & b,
                   const Vector2D & c,
                   const Vector2D & point );

};

}

#endif
