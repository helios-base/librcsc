// -*-c++-*-

/*!
  \file segment_2d.h
  \brief 2D segment line Header File.
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

#ifndef RCSC_GEOM_SEGMENT2D_H
#define RCSC_GEOM_SEGMENT2D_H

#include <rcsc/geom/line_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <cmath>

namespace rcsc {

/*!
  \class Segment2D
  \brief 2d segment line class
*/
class Segment2D {
private:

    static const double EPSILON; //!< threshold value
    static const double CALC_ERROR; //!< calculation error threshold value

    Vector2D M_origin; //!< origin point
    Vector2D M_terminal; //!< terminal point

    //! not used
    Segment2D() = delete;

    bool checkIntersectsOnLine( const Vector2D & p ) const;

public:
    /*!
      \brief construct from 2 points
      \param origin 1st point of segment edge
      \param terminal 2nd point of segment edge
     */
    Segment2D( const Vector2D & origin,
               const Vector2D & terminal )
        : M_origin( origin ),
          M_terminal( terminal )
      { }

    /*!
      \brief construct directly using raw coordinate values
      \param origin_x 1st point x value of segment edge
      \param origin_y 1st point x value of segment edge
      \param terminal_x 2nd point y value of segment edge
      \param terminal_y 2nd point y value of segment edge
     */
    Segment2D( const double origin_x,
               const double origin_y,
               const double terminal_x,
               const double terminal_y )
        : M_origin( origin_x, origin_y ),
          M_terminal( terminal_x, terminal_y )
      { }

    /*!
      \brief construct using origin, direction and length
      \param origin origin point
      \param length length of line segment
      \param dir line direction from origin point
     */
    Segment2D( const Vector2D & origin,
               const double length,
               const AngleDeg & dir )
        : M_origin( origin ),
          M_terminal( origin + Vector2D::from_polar( length, dir ) )
      { }

    /*!
      \brief construct from 2 points
      \param origin first point
      \param terminal second point
      \return const reference to this object
    */
    const Segment2D & assign( const Vector2D & origin,
                              const Vector2D & terminal )
      {
          M_origin = origin;
          M_terminal = terminal;
          return *this;
      }

    /*!
      \brief construct directly using raw coordinate values
      \param origin_x 1st point x value of segment edge
      \param origin_y 1st point x value of segment edge
      \param terminal_x 2nd point y value of segment edge
      \param terminal_y 2nd point y value of segment edge
      \return const reference to this object
    */
    const Segment2D & assign( const double origin_x,
                              const double origin_y,
                              const double terminal_x,
                              const double terminal_y )
      {
          M_origin.assign( origin_x, origin_y );
          M_terminal.assign( terminal_x, terminal_y );
          return *this;
      }

    /*!
      \brief construct using origin, direction and length
      \param origin origin point
      \param length length of line segment
      \param dir line direction from origin point
      \return const reference to this object
    */
    const Segment2D & assign( const Vector2D & origin,
                              const double length,
                              const AngleDeg & dir )
      {
          M_origin = origin;
          M_terminal = origin + Vector2D::from_polar( length, dir );
          return *this;
      }

    /*!
      \brief check if this line segment is valid or not.
      origin's coodinates value have to be different from terminal's one.
      \return checked result.
    */
    bool isValid() const
      {
          return ! origin().equalsWeakly( terminal() );
      }

    /*!
      \brief get 1st point of segment edge
      \return const reference to the vector object
    */
    const Vector2D & origin() const
      {
          return M_origin;
      }

    /*!
      \brief get 2nd point of segment edge
      \return const reference to the vector object
    */
    const Vector2D & terminal() const
      {
          return M_terminal;
      }

    /*!
      \brief get line generated from segment
      \return new line object
    */
    Line2D line() const
      {
          return Line2D( origin(), terminal() );
      }

    /*!
      \brief get the length of this segment
      \return distance value
     */
    double length() const
      {
          return origin().dist( terminal() );
      }

    /*!
      \brief get the direction angle of this line segment
      \return angle object
     */
    AngleDeg direction() const
      {
          return ( terminal() - origin() ).th();
      }



    /*!
      \brief swap segment edge point
      \return const reference to this object
    */
    const Segment2D & swap()
      {
          // std::swap( M_origin, M_terminal );
          Vector2D tmp = M_origin;
          M_origin = M_terminal;
          M_terminal = tmp;
          return *this;
      }

    /*!
      \brief swap segment edge point. This method is equivalent to swap(), provided for convenience.
      \return const reference to this object
    */
    const Segment2D & reverse()
      {
          return swap();
      }

    /*!
      \brief get the reversed line segment.
      \return const reference to this object
    */
    Segment2D reversedSegment() const
      {
          return Segment2D( *this ).reverse();
      }

    /*!
      \brief make perpendicular bisector line from segment points
      \return line object
     */
    Line2D perpendicularBisector() const
      {
          return Line2D::perpendicular_bisector( origin(), terminal() );
      }

    /*!
      \brief check if the point is within the rectangle defined by this
      segment as a diagonal line.
      \return true if rectangle contains p
     */
    bool contains( const Vector2D & p ) const
      {
          return ( ( p.x - origin().x ) * ( p.x - terminal().x ) <= CALC_ERROR //EPSILON
                   && ( p.y - origin().y ) * ( p.y - terminal().y ) <= CALC_ERROR ); //EPSILON );
      }

    /*!
      \brief check if this line segment has completely same value as input line segment.
      \param other compared object.
      \return checked result.
     */
    bool equals( const Segment2D & other ) const
      {
          return this->origin().equals( other.origin() )
              && this->terminal().equals( other.terminal() );
      }

    /*!
      \brief check if this line segment has weakly same value as input line segment.
      \param other compared object.
      \return checked result.
     */
    bool equalsWeakly( const Segment2D & other ) const
      {
          return this->origin().equalsWeakly( other.origin() )
              && this->terminal().equalsWeakly( other.terminal() );
      }


    /*!
      \brief calculates projection point from p
      \param p input point
      \return projection point from p. if it does not exist,
      the invalidated value vector is returned.
     */
    Vector2D projection( const Vector2D & p ) const;

    /*!
      \brief check & get the intersection point with other line segment
      \param other checked line segment
      \param allow_end_point if this value is false, an end point is disallowed as an intersection.
      \return intersection point. if it does not exist,
      the invalidated value vector is returned.
    */
    Vector2D intersection( const Segment2D & other,
                           const bool allow_end_point ) const;

    /*!
      \brief check & get the intersection point with other line
      \param l checked line object
      \return intersection point. if it does not exist,
      the invalidated value vector is returned.
    */
    Vector2D intersection( const Line2D & l ) const;

    /*!
      \brief check if segments cross each other or not.
      \param other segment for cross checking
      \return true if this segment crosses, otherwise returns false.
    */
    bool existIntersection( const Segment2D & other ) const;

    /*!
      \brief check if segments cross each other or not.
      \param other segment for cross checking
      \return true if this segment crosses, otherwise returns false.
    */
    bool intersects( const Segment2D & other ) const
      {
          return existIntersection( other );
      }

    /*!
      \brief check if segments intersect each other on non terminal point.
      \param other segment for cross checking
      \return true if segments intersect and intersection point is not a
      terminal point of segment.
      false if segments do not intersect or intersect on terminal point of segment.
    */
    bool existIntersectionExceptEndpoint( const Segment2D & other ) const;

    /*!
      \brief check if segments intersect each other on non terminal point.
      \param other segment for cross checking
      \return true if segments intersect and intersection point is not a
      terminal point of segment.
      false if segments do not intersect or intersect on terminal point of segment.
    */
    bool intersectsExceptEndpoint( const Segment2D & other ) const
      {
          return existIntersectionExceptEndpoint( other );
      }

    /*!
      \brief check if this line segment intersects with target line.
      \param l checked line
      \return checked result
     */
    bool existIntersection( const Line2D & l ) const;

    /*!
      \brief check if this line segment intersects with target line.
      \param l checked line
      \return checked result
     */
    bool intersects( const Line2D & l ) const
      {
          return existIntersection( l );
      }

    /*!
      \brief get a point on segment where distance of point is minimal.
      \param p point
      \return nearest point on segment. if multiple nearest points found.
       returns one of them.
    */
    Vector2D nearestPoint( const Vector2D & p ) const;

    /*!
      \brief get minimum distance between this segment and point
      \param p point
      \return minimum distance between this segment and point
    */
    double dist( const Vector2D & p ) const;

    /*!
      \brief get minimum distance between 2 segments
      \param seg segment
      \return minimum distance between 2 segments
    */
    double dist( const Segment2D & seg ) const;

    /*!
      \brief get maximum distance between this segment and point
      \param p point
      \return maximum distance between this segment and point
    */
    double farthestDist( const Vector2D & p ) const;

    /*!
      \brief strictly check if point is on segment or not
      \param p checked point
      \return true if point is on this segment
    */
    bool onSegment( const Vector2D & p ) const;

    /*!
      \brief weakly check if point is on segment or not
      \param p checked point
      \return true if point is on this segment
    */
    bool onSegmentWeakly( const Vector2D & p ) const;

    /*!
      \brief print data to an output stream.
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & print( std::ostream & os ) const
      {
          os << '[' << origin() << '-' << terminal() << ']';
          return os;
      }

};

}

#endif
