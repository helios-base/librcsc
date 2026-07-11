// -*-c++-*-

/*!
  \file delaunay_triangulation.h
  \brief Delaunay Triangulation class Header File.
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

#ifndef RCSC_GEOM_DELAUNAY_TRIANGULATION_H
#define RCSC_GEOM_DELAUNAY_TRIANGULATION_H

#include <rcsc/geom/delaunay_triangulation_core.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <vector>

namespace rcsc {

/*!
  \class DelaunayTriangulation
  \brief Delaunay triangulation

  The vertex/edge/triangle graph, incremental point insertion,
  Lawson-flip legalization, and the two point-location search strategies
  (hint-based adjacency walk and optional quadtree spatial index) are
  implemented once in the DelaunayTriangulationCore base class (shared with
  ConstrainedDelaunayTriangulation). This class adds Voronoi-vertex
  bookkeeping that is specific to plain (unconstrained) Delaunay
  triangulation.
*/
class DelaunayTriangulation
    : public DelaunayTriangulationCore {
public:

    //! backward-compatibility alias; prefer DelaunayTriangulationCore::SearchMethod.
    using SearchMethod = DelaunayTriangulationCore::SearchMethod;

private:

    // not used
    DelaunayTriangulation( const DelaunayTriangulation & ) = delete;
    DelaunayTriangulation & operator=( const DelaunayTriangulation & ) = delete;

public:

    /*!
      \brief nothing to do
    */
    DelaunayTriangulation() = default;

    /*!
      \brief construct with considerable rectangle region
      \param region considerable rectangle region.

      All verteices must be included in region.
    */
    explicit
    DelaunayTriangulation( const Rect2D & region )
    {
        clearResults();
        createInitialTriangle( region );
    }

    /*!
      \brief destruct
    */
    ~DelaunayTriangulation();

    /*!
      \brief initialize with target field rectangle data.
      All data are cleared.
      Initial triangle is crated.
    */
    void init( const Rect2D & region )
    {
        clear();
        createInitialTriangle( region );
    }

    /*!
      \brief clear all vertices and all computed results.
    */
    void clear()
    {
        clearResults();
        M_vertices.clear();
    }

    /*!
      \brief clear all computed results
    */
    void clearResults();

    /*!
      \brief compute the Delaunay Triangulation
    */
    void compute();

    /*!
      \brief calculate voronoi vertex point for each triangle
    */
    void updateVoronoiVertex();

    /*!
      \brief find triangle that contains pos from the computed triangle set.
      Uses the search strategy selected by setSearchMethod() (default: WALK).
      \param pos coordinates of the target point
      \return const pointer to the found triangle. if no triangle, NULL is returned.
    */
    const Triangle * findTriangleContains( const Vector2D & pos ) const;

};

}

#endif
