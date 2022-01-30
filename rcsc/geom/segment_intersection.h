// -*-c++-*-

/*!
  \file segment_intersection.h
  \brief segment intersection class Header File.
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

#ifndef RCSC_GEOM_SEGMENT_INTERSECTION_H
#define RCSC_GEOM_SEGMENT_INTERSECTION_H

#include <rcsc/geom/segment_2d.h>

#include <vector>

namespace rcsc {

/*!
  \class SegmentIntersection
  \brief The class represents an intersection between two line segments.
*/
class SegmentIntersection {
private:
    Segment2D M_segment0; //!< first line segment
    Segment2D M_segment1; //!< second line segment

    // not used
    SegmentIntersection() = delete;

public:

    /*!
      \brief construct with two line segment
      \param s0 first line segment
      \param s1 second line segment
    */
    SegmentIntersection( const Segment2D & s0,
                         const Segment2D & s1 )
        : M_segment0( s0 ),
          M_segment1( s1 )
      { }


    /*!
      \brief get intersection point between line segments.
      \return if exists, intersection point. otherwise invalid vector
    */
    Vector2D intersection() const
      {
          return M_segment0.intersection( M_segment1, true );
      }

};


/*!
  \class SegmentIntersectionDetector
  \brief abstract intersection detector class
*/
class SegmentIntersectionDetector {
protected:
    /*!
      \brief protected constructor.
    */
    SegmentIntersectionDetector()
      { }

public:

    /*!
      \brief virtual destructor
    */
    virtual
    ~SegmentIntersectionDetector()
      { }


    /*!
      \brief pure virtual method. execute detection algorithm.
      \param segments input line segments
      \param intersections result intersections
    */
    virtual
    void execute( const std::vector< Segment2D > & segments,
                  std::vector< SegmentIntersection > * intersections ) const = 0;
};


/*!
  \class BruteForceSegmentIntersectionDetector
  \brief intersection detector using brute force algorithm
*/
class BruteForceSegmentIntersectionDetector
    : public SegmentIntersectionDetector {
public:

    /*!
      \brief execute brute force algorithm
      \param segments input line segments
      \param intersections result intersections
    */
    void execute( const std::vector< Segment2D > & segments,
                  std::vector< SegmentIntersection > * intersections ) const;

};

}

#endif
