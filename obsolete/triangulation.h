// -*-c++-*-

/*!
  \file triangulation.h
  \brief 2D triangulation Header File.
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

#ifndef RCSC_GEOM_TRIANGULATION_USING_TRIANGLE_H
#define RCSC_GEOM_TRIANGULATION_USING_TRIANGLE_H

#include <rcsc/geom/vector_2d.h>

#include <vector>
#include <set>

namespace rcsc {

/*!
  \class Triangulation
  \brief (Constrained Delaunay) triangulation class
 */
class Triangulation {
public:
    /*!
      \struct Triangle
      \brief triangle object type for Triangulation.
     */
    struct Triangle {
        size_t v0_; //!< index of first vertex
        size_t v1_; //!< index of second vertex
        size_t v2_; //!< index of third vertex

        /*!
          \brief construct with all indices
          \param v0 index of first vertex
          \param v1 index of second vertex
          \param v2 index of third vertex
         */
        Triangle( const size_t v0,
                  const size_t v1,
                  const size_t v2 )
            : v0_( v0 ),
              v1_( v1 ),
              v2_( v2 )
          { }
    };

    typedef std::vector< Vector2D > PointCont; //!< point container type.
    typedef std::vector< Triangle > TriangleCont; //!< triangle container type.
    typedef std::pair< size_t, size_t > Segment; //!< segment edge type.
    typedef std::set< Segment > SegmentSet; //!< segment edge set type.
    typedef std::vector< Segment > SegmentCont; //!< segment edge container type.

private:

    bool M_use_triangles; //!< switch to determine whether result triangules are stored or not (default: true).
    bool M_use_edges; //!< switch to determine whether result edges are stored or not (default: true).

#ifdef TRIANGULATION_STRICT_POINT_SET
    std::set< Vector2D, Vector2D::XYCmp > M_point_set; //!< input point set
#endif

    PointCont M_points; //! input points
    SegmentSet M_constraints; //!< input constraint segments

    TriangleCont M_triangles; //!< result triangles
    SegmentCont M_edges; //!< result triangle edges

public:
    /*!
      \brief create null triangulation object.
    */
    Triangulation()
        : M_use_triangles( true )
        , M_use_edges( true )
      { }

    /*!
      \brief clear all data.
     */
    void clear();

    /*!
      \brief clear result data.
     */
    void clearResults();

    /*!
      \brief get input point container.
      \return const reference to the point container.
     */
    const PointCont & points() const
      {
          return M_points;
      }

    /*!
      \brief get constrained edges.
      \return const reference to the segment container.
    */
    const SegmentSet & constraints() const
      {
          return M_constraints;
      }

    /*!
      \brief get result triangle set.
      \return const reference to the triangle container.
    */
    const TriangleCont & triangles() const
      {
          return M_triangles;
      }

    /*!
      \brief get result triangle edges.
      \return const reference to the segment container.
    */
    const SegmentCont & edges() const
      {
          return M_edges;
      }

    /*!
      \brief set use_triangles property.
      \param on property value.
     */
    void setUseTriangles( const bool on )
      {
          M_use_triangles = on;
      }

    /*!
      \brief set use_triangles property.
      \param on new property value.
     */
    void setUseEdges( const bool on )
      {
          M_use_edges = on;
      }

    /*!
      \brief add point to the input point container.
      \param p new point.
      \return result of adding operation.
    */
    bool addPoint( const Vector2D & p );

    /*!
      \brief add points to the input point container.
      \param v point container.
      \return size of successfully added points.
    */
    size_t addPoints( const PointCont & v );

    /*!
      \brief add constraint point indices for Constrained Delaunay triangulation.
      \param origin_index index of first point
      \param terminal_index index of second point
    */
    bool addConstraint( const size_t & origin_index,
                        const size_t & terminal_index );

    /*!
      \brief generates triangulation.
    */
    void compute();

    /*!
      \brief find the triangle contanes the input point.
      \param point input point
      \return pointer to the triangle. if not found, returns NULL.
     */
    const Triangle * findTriangleContains( const Vector2D & point ) const;

    /*!
      \brief find the point nearest to the input point.
      \param point input point
      \return index of the nearest point. if not found, returns -1.
     */
    int findNearestPoint( const Vector2D & point ) const;
};

}

#endif
