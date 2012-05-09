// -*-c++-*-

/*!
  \file voronoi_diagram_triangle.h
  \brief 2D voronoi diagram Header File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki Shimora

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

#ifndef RCSC_GEOM_VORONOI_DIAGRAM_TRIANGLE_H
#define RCSC_GEOM_VORONOI_DIAGRAM_TRIANGLE_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/ray_2d.h>
#include <rcsc/geom/rect_2d.h>

#include <vector>
#include <set>

namespace rcsc {

/*!
  \class VoronoiDiagramTriangle
  \brief 2D voronoi diagram class usint triangle library
 */
class VoronoiDiagramTriangle {
public:
    typedef std::set< Vector2D, Vector2D::XYCmp > Vector2DCont;
    typedef std::vector< Segment2D > Segment2DCont;
    typedef std::vector< Ray2D > Ray2DCont;

private:

    Rect2D * M_bounding_rect;

    //! input points
    std::vector< Vector2D > M_input_points;

    Vector2DCont M_vertices; //!< vertices of voronoi regions
    Segment2DCont M_segments; //!< edges of voronoi regions
    Ray2DCont M_rays; //!< edges in outside of convex hull

public:
    /*!
      \brief create voronoi diagram handler
    */
    VoronoiDiagramTriangle();

    /*!
      \brief create voronoi diagram handler with points
      \param v array of input points
    */
    VoronoiDiagramTriangle( const std::vector< Vector2D > & v );

    /*!
      \brief destructor.
     */
    ~VoronoiDiagramTriangle();

    /*!
      \brief set bounding rectangle.
      \param rect input rectangle.
     */
    void setBoundingRect( const Rect2D & rect );

    /*!
      \brief add point to voronoi diagram as one of input points
      \param p new point to add
    */
    void addPoint( const Vector2D & p )
      {
          M_input_points.push_back( p );
      }

    /*!
      \brief add points
      \param v input point container
    */
    void addPoint( const std::vector< Vector2D > & v )
      {
          M_input_points.insert( M_input_points.end(), v.begin(), v.end() );
      }

    /*!
      \brief clear all variables.
     */
    void clear();

    /*!
      \brief delete bounding rectangle object if exists.
     */
    void clearBoundingRect();

    /*!
      \brief clear result variables.
     */
    void clearResults();

    /*!
      \brief generates voronoi diagram
    */
    void compute();

    /*!
      \brief get result set of points
      \return const reference to point list
    */
    const Vector2DCont & vertices() const
      {
          return M_vertices;
      }
    const Vector2DCont & resultPoints() const
      {
          return M_vertices;
      }

    /*!
      \brief get result set of segments
      \return const reference to segment list
    */
    const Segment2DCont & segments() const
      {
          return M_segments;
      }
    const Segment2DCont & resultSegments() const
      {
          return M_segments;
      }

    /*!
      \brief get result set of rays
      \return const reference to ray list
    */
    const Ray2DCont & rays() const
      {
          return M_rays;
      }
    const Ray2DCont & resultRays() const
      {
          return M_rays;
      }

    /*!
      \brief get point set on segments
      \param min_length minimum length between points
      \param max_division max number of point on a segment
      \param result point set on segments
    */
    void getPointsOnSegments( const double min_length,
                              const unsigned int max_division,
                              std::vector< Vector2D > * result ) const;
};

}

#endif
