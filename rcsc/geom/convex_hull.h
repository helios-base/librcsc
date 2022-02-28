// -*-c++-*-

/*!
  \file convex_hull.h
  \brief 2D convex hull Header File.
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

#ifndef RCSC_GEOM_CONVEX_HULL_H
#define RCSC_GEOM_CONVEX_HULL_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/polygon_2d.h>

#include <vector>
#include <set>

namespace rcsc {

class ConvexHull {
public:
    typedef std::vector< Vector2D > PointCont; //!< input point container
    typedef std::vector< Vector2D > VertexCont; //!< result vertex container
    typedef std::vector< Segment2D > EdgeCont; //!< result edge container

    /*!
      \enum MethodType
      \brief algoritym type
     */
    enum MethodType {
        DirectMethod,
        WrappingMethod,
        GrahamScan,
        // IncrementalMethod,
        // DivideConquer,
        // QuickMethod,
        // InnerPointsElimination,
    };

private:

    PointCont M_input_points; //!< input points

    VertexCont M_vertices; //!< vertices of convex hull, sorted by counter clockwise order
    EdgeCont M_edges; //!< edges of convex hull (should be ordered by counter clockwise?)


    // not used
    ConvexHull( const ConvexHull & ) = delete;
    ConvexHull & operator=( const ConvexHull & ) = delete;

public:

    /*!
      \brief create empty convex hull
    */
    ConvexHull();

    /*!
      \brief create convex hull with given points
      \param v array of input points
     */
    ConvexHull( const PointCont & v );

    /*!
      \brief clear all data.
     */
    void clear();

    /*!
      \brief clear result variables.
     */
    void clearResults();

    /*!
      \brief add a new point to the set of input point
      \param p new point
    */
    void addPoint( const Vector2D & p )
      {
          M_input_points.push_back( p );
      }

    /*!
      \brief add new points to the set of input pointc
      \param v input point container
    */
    void addPoints( const PointCont & v )
      {
          M_input_points.insert( M_input_points.end(), v.begin(), v.end() );
      }

    /*!
      \brief generate convex hull by default method.
     */
    // void compute();

    /*!
      \brief generate convex hull by specified method
      \param type method type id
     */
    void compute( const MethodType type = WrappingMethod );

    /*!
      \brief get the reference to the input point container
      \return const reference to the input point container
     */
    const PointCont & inputPoints() const
      {
          return M_input_points;
      }

    /*!
      \brief get the reference to the vertex container ordered by counter clockwise
      \return const reference to the ordered vertex container
     */
    const VertexCont & vertices() const
      {
          return M_vertices;
      }

    /*!
      \brief get the reference to the result edge container
      \return const reference to the result edge container
     */
    const EdgeCont & edges() const
      {
          return M_edges;
      }

private:

    /*!
      \brief direct method version
     */
    void computeDirectMethod();

    /*!
      \brief wrapping method version
     */
    void computeWrappingMethod();

    /*!
      \brief Graham scan method version
     */
    void computeGrahamScan();

    /*!
      \brief incremental method version
     */
    // void computeIncrementalMethod();

    /*!
      \brief divide and conquer method version
     */
    // void computeDivideAndConquer();

    /*!
      \brief quick method version
     */
    // void computeQuickMethod();

    /*!
      \brief inner points elimination method version
     */
    // void computeInnerPointsElimination();

private:

    /*!
      \brief get the index of minimum coordinate point
     */
    size_t getMinPointIndex() const;

    void sortPointsByAngleFrom( const size_t index );

public:

    /*!
      \brief get the convex hull polygon
      \return new 2d polygon object
     */
    Polygon2D toPolygon() const;

    /*!
      \brief output input points to the stream in gnuplot format.
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printInputPoints( std::ostream & os ) const;

    /*!
      \brief output vertices to the stream in gnuplot format.
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printVertices( std::ostream & os ) const;

    /*!
      \brief output edges to the stream in gnuplot format.
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printEdges( std::ostream & os ) const;

};

}

#endif
