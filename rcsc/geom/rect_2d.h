// -*-c++-*-

/*!
  \file rect_2d.h
  \brief 2D rectangle region Header File.
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

#ifndef RCSC_GEOM_RECT2D_H
#define RCSC_GEOM_RECT2D_H

#include <rcsc/geom/region_2d.h>
#include <rcsc/geom/size_2d.h>
#include <rcsc/geom/line_2d.h>
#include <rcsc/geom/vector_2d.h>

namespace rcsc {

class Ray2D;
class Segment2D;

/*!
  \class Rect2D
  \brief 2D rectangle regin class.

  The model and naming rules are depend on soccer simulator environment
          -34.0
            |
            |
-52.5 ------+------- 52.5
            |
            |
          34.0
*/
class Rect2D
    : public Region2D {
private:
    //! top left point
    Vector2D M_top_left;

    //! XY range
    Size2D M_size;

public:
    /*!
      \brief default constructor creates a zero area rectanble at (0,0)
     */
    Rect2D()
        : M_top_left( 0.0, 0.0 )
        , M_size( 0.0, 0.0 )
      { }
private:
    /*!
      \brief constructor
      \param left_x left x
      \param top_y top y
      \param length length (x-range)
      \param width width (y-range)
     */
    Rect2D( const double left_x,
            const double top_y,
            const double length,
            const double width )
        : M_top_left( left_x, top_y )
        , M_size( length, width )
      { }

    /*!
      \brief constructor with variables
      \param top_left top left point
      \param length X range
      \param width Y range
     */
    Rect2D( const Vector2D & top_left,
            const double length,
            const double width )
        : M_top_left( top_left )
        , M_size( length, width )
      { }

public:
    /*!
      \brief constructor with variables
      \param top_left top left point
      \param size XY range
     */
    Rect2D( const Vector2D & top_left,
            const Size2D & size )
        : M_top_left( top_left )
        , M_size( size )
      { }

    /*!
      \brief constructor with 2 points.
      \param top_left top left vertex
      \param bottom_right bottom right vertex

      Even if argument point has incorrect values,
      the assigned values are normalized automatically.
    */
    Rect2D( const Vector2D & top_left,
            const Vector2D & bottom_right )
        : M_top_left( top_left )
        , M_size( bottom_right.x - top_left.x,
                  bottom_right.y - top_left.y )
      {
          if ( bottom_right.x - top_left.x < 0.0 )
          {
              M_top_left.x = bottom_right.x;
          }
          if ( bottom_right.y - top_left.y < 0.0 )
          {
              M_top_left.y = bottom_right.y;
          }
      }

    /*!
      \brief create rectangle with center point and size.
      \param center center point of new rectangle.
      \param length length(x-range) of new rectangle.
      \param width width(y-range) of new rectangle.
     */
    static
    Rect2D from_center( const Vector2D & center,
                        const double length,
                        const double width )
      {
          return Rect2D( center.x - length*0.5,
                         center.y - width*0.5,
                         length,
                         width );
      }

    /*!
      \brief create rectangle with center point and size.
      \param center_x x value of center point of new rectangle.
      \param center_y y value of center point of new rectangle.
      \param length length(x-range) of new rectangle.
      \param width width(y-range) of new rectangle.
     */
    static
    Rect2D from_center( const double center_x,
                        const double center_y,
                        const double length,
                        const double width )
      {
          return Rect2D( center_x - length*0.5,
                         center_y - width*0.5,
                         length,
                         width );
      }

    /*!
      \brief create rectangle with 2 corner points. just call one of constructor.
      \param top_left top left vertex
      \param bottom_right bottom right vertex
    */
    static
    Rect2D from_corners( const Vector2D & top_left,
                         const Vector2D & bottom_right )
      {
          return Rect2D( top_left, bottom_right );
      }

    /*!
      \brief create rectangle with 2 corner points. just call one of constructor.
      \param l left x
      \param t top y
      \param r right x
      \param b bottom y
    */
    static
    Rect2D from_corners( const double l,
                         const double t,
                         const double r,
                         const double b )
      {
          return Rect2D( Vector2D( l, t ), Vector2D( r, b ) );
      }

private:
    /*!
      \brief assign new values
      \param left_x left x
      \param top_y top y
      \param length X range
      \param width Y range
     */
    const
    Rect2D & assign( const double left_x,
                     const double top_y,
                     const double length,
                     const double width )
      {
          M_top_left.assign( left_x, top_y );
          M_size.assign( length, width );
          return *this;
      }

    /*!
      \brief assign new values
      \param top_left top left point
      \param length X range
      \param width Y range
      \return const referenct to itself
     */
    const
    Rect2D & assign( const Vector2D & top_left,
                     const double length,
                     const double width )
      {
          M_top_left = top_left;
          M_size.assign( length, width );
          return *this;
      }

public:
    /*!
      \brief assign new values
      \param top_left top left
      \param size XY range
      \return const referenct to itself
     */
    const Rect2D & assign( const Vector2D & top_left,
                           const Size2D & size )
      {
          M_top_left = top_left;
          M_size = size;
          return *this;
      }

    /*!
      \brief move the rectangle.
      the center point is set to the given position.
      the size is unchanged.
      \param point new center coordinates
      \return const referenct to itself
     */
    const Rect2D & moveCenter( const Vector2D & point )
      {
          M_top_left.assign( point.x - M_size.length() * 0.5,
                             point.y - M_size.width() * 0.5 );
          return *this;
      }

    /*!
      \brief move the rectangle.
      the top-left coner is set to the given position.
      the size is unchanged.
      \param point new top-left corner
      \return const referenct to itself
     */
    const Rect2D & moveTopLeft( const Vector2D & point )
      {
          M_top_left = point;
          return *this;
      }

    /*!
      \brief move the rectangle.
      the bottom-right coner is set to the given position.
      the size is unchanged.
      \param point new bottom-right conrer
      \return const referenct to itself
     */
    const Rect2D & moveBottomRight( const Vector2D & point )
      {
          M_top_left.assign( point.x - M_size.length(),
                             point.y - M_size.width() );
          return *this;
      }

    /*!
      \brief move the rectangle.
      the left line is set to the given position.
      the size is unchanged.
      \param x new left value
      \return const referenct to itself
     */
    const Rect2D & moveLeft( const double x )
      {
          M_top_left.x = x;
          return *this;
      }

    /*!
      \brief alias of moveLeft.
      \param x new left value
      \return const referenct to itself
     */
    const Rect2D & moveMinX( const double x )
      {
          return moveLeft( x );
      }

    /*!
      \brief move the rectangle.
      the right line is set to the given value.
      the size is unchanged.
      \param x new right value
      \return const referenct to itself
     */
    const Rect2D & moveRight( const double x )
      {
          M_top_left.x = x - M_size.length();
          return *this;
      }

    /*!
      \brief alias of moveRight.
      \param x new right value
      \return const referenct to itself
     */
    const Rect2D & moveMaxX( const double x )
      {
          return moveRight( x );
      }

    /*!
      \brief move the rectangle.
      the top line is set to the given value.
      the size is unchanged.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & moveTop( const double y )
      {
          M_top_left.y = y;
          return *this;
      }

    /*!
      \brief alias of moveTop.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & moveMinY( const double y )
      {
          return moveTop( y );
      }

    /*!
      \brief move the rectangle.
      the top line is set to the given value.
      the size is unchanged.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & moveBottom( const double y )
      {
          M_top_left.y = y - M_size.width();
          return *this;
      }

    /*!
      \brief alias of moveTop.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & moveMaxY( const double y )
      {
          return moveBottom( y );
      }


    /*!
      \brief set the top-left corner of the rectangle.
      the size may be changed, but the bottom-right corner will never be changed.
      \param x new x coordinate
      \param y new y coordinate
      \return const referenct to itself
     */
    const Rect2D & setTopLeft( const double x,
                               const double y );

    /*!
      \brief set the top-left corner of the rectangle.
      the size may be changed, but the bottom-right corner will never be changed.
      \param point new coordinate
      \return const referenct to itself
     */
    const Rect2D & setTopLeft( const Vector2D & point )
      {
          return setTopLeft( point.x, point.y );
      }

    /*!
      \brief set the bottom-right corner of the rectangle.
      the size may be changed, but the top-left corner will never be changed.
      \param x new x coordinate
      \param y new y coordinate
      \return const referenct to itself
     */
    const Rect2D & setBottomRight( const double x,
                                   const double y );

    /*!
      \brief set the bottom-right corner of the rectangle.
      the size may be changed, but the top-left corner will never be changed.
      \param point new coordinate
      \return const referenct to itself
     */
    const Rect2D & setBottomRight( const Vector2D & point )
      {
          return setBottomRight( point.x, point.y );
      }

    /*!
      \brief set the left of rectangle.
      the size may be changed, but the right will never be changed.
      \param x new left value
      \return const referenct to itself
     */
    const Rect2D & setLeft( const double x );

    /*!
      \brief alias of setLeft.
      \param x new left value
      \return const referenct to itself
     */
    const Rect2D & setMinX( const double x )
      {
          return setLeft( x );
      }

    /*!
      \brief set the right of rectangle.
      the size may be changed, but the left will never be changed.
      \param x new right value
      \return const referenct to itself
     */
    const Rect2D & setRight( const double x );

    /*!
      \brief alias of setRight.
      \param x new right value
      \return const referenct to itself
     */
    const Rect2D & setMaxX( const double x )
      {
          return setRight( x );
      }

    /*!
      \brief set the top of rectangle.
      the size may be changed, but the bottom will never be changed.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & setTop( const double y );

    /*!
      \brief alias of setTop.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & setMinY( const double y )
      {
          return setTop( y );
      }

    /*!
      \brief set the bottom of rectangle.
      the size may be changed, but the top will never be changed.
      \param y new bottom value
      \return const referenct to itself
     */
    const Rect2D & setBottom( const double y );

    /*!
      \brief alias of setBottom.
      \param y new top value
      \return const referenct to itself
     */
    const Rect2D & setMaxY( const double y )
      {
          return setBottom( y );
      }

    /*!
      \brief set a new x-range
      \param length new range
      \return const referenct to itself
     */
    const Rect2D & setLength( const double length )
      {
          M_size.setLength( length );
          return *this;
      }

    /*!
      \brief set a new y-range
      \param width new range
      \return const referenct to itself
     */
    const Rect2D & setWidth( const double width )
      {
          M_size.setWidth( width );
          return *this;
      }

    /*!
      \brief set a new size
      \param length new range
      \param width new range
      \return const referenct to itself
     */
    const Rect2D & setSize( const double length,
                            const double width )
      {
          M_size.assign( length, width );
          return *this;
      }

    /*!
      \brief set a new size
      \param size new range
      \return const referenct to itself
     */
    const Rect2D & setSize( const Size2D & size )
      {
          M_size = size;
          return *this;
      }

    /*!
      \brief check if this rectangle is valid or not.
      \return true if the area of this rectangle is not 0.
     */
    bool isValid() const
      {
          return M_size.length() > 0.0
              && M_size.width() > 0.0;
      }

    /*!
      \brief get the area value of this rectangle.
      \return value of the area
     */
    virtual
    double area() const
      {
          return M_size.length() * M_size.width();
      }

    /*!
      \brief check if point is within this region.
      \param point considered point
      \return true or false
     */
    virtual
    bool contains( const Vector2D & point ) const
      {
          return ( left() <= point.x
                   && point.x <= right()
                   && top() <= point.y
                   && point.y <= bottom() );
      }

    /*!
      \brief check if point is within this region with error threshold.
      \param point considered point
      \param error_thr error threshold
      \return true or false
     */
    bool contains( const Vector2D & point,
                   const double error_thr ) const
      {
          return ( left() - error_thr <= point.x
                   && point.x <= right() + error_thr
                   && top() - error_thr <= point.y
                   && point.y <= bottom() + error_thr );
      }

    /*!
      \brief get the left x coordinate of this rectangle.
      \return x coordinate value
    */
    double left() const
      {
          return M_top_left.x;
      }

    /*!
      \brief get the right x coordinate of this rectangle.
      \return x coordinate value
    */
    double right() const
      {
          return left() + size().length();
      }

    /*!
      \brief get the top y coordinate of this rectangle.
      \return y coordinate value
    */
    double top() const
      {
          return M_top_left.y;
      }

    /*!
      \brief get the bottom y coordinate of this rectangle.
      \return y coordinate value
    */
    double bottom() const
      {
          return top() + size().width();
      }

    /*!
      \brief get minimum value of x coordinate of this rectangle
      \return x coordinate value (equivalent to left())
    */
    double minX() const
      {
          return left();
      }

    /*!
      \brief get maximum value of x coordinate of this rectangle
      \return x coordinate value (equivalent to right())
    */
    double maxX() const
      {
          return right();
      }

    /*!
      \brief get minimum value of y coordinate of this rectangle
      \return y coordinate value (equivalent to top())
    */
    double minY() const
      {
          return top();
      }

    /*!
      \brief get maximum value of y coordinate of this rectangle
      \return y coordinate value (equivalent to bottom())
    */
    double maxY() const
      {
          return bottom();
      }

    /*!
      \brief get the XY range of this rectangle
      \return size object
    */
    const Size2D & size() const
      {
          return M_size;
      }

    /*!
      \brief get center point
      \return coordinate value by vector object
     */
    Vector2D center() const
      {
          return Vector2D( ( left() + right() ) * 0.5,
                           ( top() + bottom() ) * 0.5 );
      }

    /*!
      \brief get the top-left corner point
      \return coordiante value by vector object
    */
    const Vector2D & topLeft() const
      {
          return M_top_left;
      }

    /*!
      \brief get the top-right corner point
      \return coordiante value by vector object
    */
    Vector2D topRight() const
      {
          return Vector2D( right(), top() );
      }

    /*!
      \brief get the bottom-left corner point
      \return coordiante value by vector object
    */
    Vector2D bottomLeft() const
      {
          return Vector2D( left(), bottom() );
      }

    /*!
      \brief get the bottom-right corner point
      \return coordiante value by vector object
    */
    Vector2D bottomRight() const
      {
          return Vector2D( right(), bottom() );
      }

    /*!
      \brief get the left edge line
      \return line object
    */
    Line2D leftEdge() const
      {
          return Line2D( topLeft(), bottomLeft() );
      }

    /*!
      \brief get the right edge line
      \return line object
    */
    Line2D rightEdge() const
      {
          return Line2D( topRight(), bottomRight() );
      }

    /*!
      \brief get the top edge line
      \return line object
    */
    Line2D topEdge() const
      {
          return Line2D( topLeft(), topRight() );
      }

    /*!
      \brief get the bottom edge line
      \return line object
    */
    Line2D bottomEdge() const
      {
          return Line2D( bottomLeft(), bottomRight() );
      }

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

    /*!
      \brief convert this rectangle to the intersection rectangle with other.
      If no intersection between rectangles, this rectangle becomes empty.
      \param other other rectangle.
      \return const reference to this.
     */
    const Rect2D & operator&=( const Rect2D & other );

    /*!
      \brief get the intersected rectangle of this rectangle and the other rectangle.
      This method is equivalent to operator '&'. If no intersection between rectangles,
      empty rectangle is returned.
      \param other other rectangle
      \return new rectangle instance.
     */
    Rect2D intersected( const Rect2D & other ) const
      {
          return Rect2D( *this ) &= other;
      }

    /*!
      \brief convert this rectangle to the united rectangle with other.
      \param other other rectangle.
      \return const reference to this.
     */
    const Rect2D & operator|=( const Rect2D & other );

    /*!
      \brief get the united rectangle of this rectangle and the other rectangle.
      This method is equivalent to operator '|'.
      \param other other rectangle
      \return new rectangle instance.
     */
    Rect2D united( const Rect2D & other ) const
      {
          return Rect2D( *this ) |= other;
      }
};

/*!
  \brief get the intersected rectangle of this rectangle and the other rectangle.
  If no intersection between rectangles, empty rectangle is returned.
  \param other other rectangle
  \return new rectangle instance.
*/
inline
const
rcsc::Rect2D
operator&( const rcsc::Rect2D & lhs,
           const rcsc::Rect2D & rhs )
{
    return rcsc::Rect2D( lhs ) &= rhs;
}

/*!
  \brief get the united rectangle of this rectangle and the other rectangle.
  \param other other rectangle
  \return new rectangle instance.
*/
inline
const
rcsc::Rect2D
operator|( const rcsc::Rect2D & lhs,
           const rcsc::Rect2D & rhs )
{
    return rcsc::Rect2D( lhs ) |= rhs;
}

}

#endif
