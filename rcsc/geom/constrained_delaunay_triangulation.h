// -*-c++-*-

/*!
  \file constrained_delaunay_triangulation.h
  \brief Constrained Delaunay Triangulation class Header File.
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

#ifndef RCSC_GEOM_CONSTRAINED_DELAUNAY_TRIANGULATION_H
#define RCSC_GEOM_CONSTRAINED_DELAUNAY_TRIANGULATION_H

#include <rcsc/geom/triangulation_mesh.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <utility>
#include <vector>

namespace rcsc {

/*!
  \class ConstrainedDelaunayTriangulation
  \brief 2D Constrained Delaunay Triangulation (CDT).

  This class builds a Delaunay triangulation over a set of vertices and then
  forces a given set of "constraint" segments (e.g. polygon/obstacle
  boundaries) to appear as edges of the resulting mesh, even where that
  requires a locally non-Delaunay configuration. Everywhere else the mesh
  remains locally Delaunay.

  The vertex/edge/triangle graph, incremental point insertion, and
  Lawson-flip legalization are implemented once in the TriangulationMesh
  base class (shared with DelaunayTriangulation). This class adds
  constraint-segment recovery (Sloan-style: force a segment in by flipping
  the edges that cross it, then re-legalize everywhere except constrained
  edges).

  This is a standalone, self-contained implementation. It does not use and
  is independent of rcsc::Triangulation (which wraps the third-party
  "triangle" library) and of the code under the geom/triangle directory.
*/
class ConstrainedDelaunayTriangulation
    : public TriangulationMesh {
public:

    typedef std::pair< int, int > ConstraintSegment; //!< constraint segment (pair of vertex Id)
    typedef std::vector< ConstraintSegment > ConstraintCont; //!< constraint segment container type

private:

    ConstraintCont M_constraints; //!< input constraint segments (pairs of vertex Id)

    // not used
    ConstrainedDelaunayTriangulation( const ConstrainedDelaunayTriangulation & ) = delete;
    ConstrainedDelaunayTriangulation & operator=( const ConstrainedDelaunayTriangulation & ) = delete;

public:

    /*!
      \brief nothing to do
    */
    ConstrainedDelaunayTriangulation() = default;

    /*!
      \brief construct with considerable rectangle region
      \param region considerable rectangle region.

      All vertices must be included in region.
    */
    explicit
    ConstrainedDelaunayTriangulation( const Rect2D & region )
      {
          clearResults();
          createInitialTriangle( region );
      }

    /*!
      \brief destruct
     */
    ~ConstrainedDelaunayTriangulation();

    /*!
      \brief initialize with target field rectangle data.
      All data are cleared. Initial triangle is created.
     */
    void init( const Rect2D & region )
      {
          clear();
          createInitialTriangle( region );
      }

    /*!
      \brief clear all vertices, constraints and all computed results.
     */
    void clear()
      {
          clearResults();
          M_vertices.clear();
          M_constraints.clear();
      }

    /*!
      \brief clear all computed results. Input vertices and constraints are kept.
     */
    void clearResults()
      {
          TriangulationMesh::clearResults();
      }

    /*!
      \brief get the input constraint segments
      \return const reference to the constraint container
     */
    const ConstraintCont & constraints() const
      {
          return M_constraints;
      }

    /*!
      \brief register a constraint segment between two already added vertices.
      The segment is only enforced when compute() is called afterward.
      \param id0 Id of the first vertex (as returned by addVertex()).
      \param id1 Id of the second vertex.
      \return false if id0/id1 do not refer to distinct existing vertices.
     */
    bool addConstraint( const int id0,
                        const int id1 );

    /*!
      \brief register a constraint segment, adding its endpoints as vertices
      first if necessary (an existing vertex within the merge tolerance is
      reused instead of creating a duplicate).
      \param p0 first endpoint
      \param p1 second endpoint
      \return false if p0 and p1 are (nearly) the same coordinate.
     */
    bool addConstraint( const Vector2D & p0,
                        const Vector2D & p1 );

    /*!
      \brief register the segments of a polyline (or, if closed==true, a
      closed polygon boundary) as constraints, adding vertices as necessary.
      \param points ordered points of the polyline.
      \param closed if true, an additional constraint segment connecting the
      last point back to the first one is also added.
      \return number of successfully added constraint segments.
     */
    std::size_t addConstraint( const std::vector< Vector2D > & points,
                               const bool closed );

    /*!
      \brief compute the Constrained Delaunay Triangulation.

      First an (unconstrained) Delaunay triangulation of all the added
      vertices is built by incremental insertion. Then every registered
      constraint segment is forced into the mesh by flipping the edges that
      cross it (Sloan-style segment recovery), after which the affected
      region is re-legalized (Delaunay edge flip) without ever touching a
      constrained edge. A constraint that geometrically crosses another
      already-inserted constraint cannot be honored and is skipped with a
      diagnostic message; every other requested constraint is still applied.
    */
    void compute();

    /*!
      \brief find triangle that contains pos from the computed triangle set.
      Uses the search strategy selected by setSearchMethod() (default: WALK).
      \param pos coordinates of the target point
      \return const pointer to the found triangle. if no triangle, NULL is returned.
     */
    const Triangle * findTriangleContains( const Vector2D & pos ) const;

    /*!
      \brief check whether the edge connecting the two vertices is registered
      as a constraint edge in the computed result.
      \param id0 Id of the first vertex.
      \param id1 Id of the second vertex.
      \return true if such an edge exists and is constrained.
     */
    bool isConstrainedEdge( const int id0,
                            const int id1 ) const;

private:

    /*!
      \brief find or add a vertex at (nearly) the given coordinates.
      \param p target coordinates
      \return Id of the existing or newly added vertex.
     */
    int findOrCreateVertex( const Vector2D & p );

    /*!
      \brief repeatedly sweep every edge currently in the mesh and
      legalizeEdge() it, until a full sweep finds nothing left to fix (or a
      generous sweep budget is exceeded). A single sweep is not enough in
      general: fixing one edge can re-illegalize an edge that was already
      checked earlier in the same sweep, so convergence needs to be
      re-verified. Used to restore the Delaunay property (outside of
      constrained edges) after a constraint segment has been forced into the
      mesh.
      \return false if an unrecoverable numerical error was detected, or the
      sweep budget was exceeded.
     */
    bool legalizeAll();

    /*!
      \brief force the constraint segment between v0 and v1 into the mesh,
      flipping any edge that crosses it, then re-legalize the affected
      region. If v0-v1 already exists as an edge, it is simply marked
      constrained.
      \param id0 Id of the first vertex.
      \param id1 Id of the second vertex.
      \return false if the constraint could not be honored (e.g. it crosses
      an already-placed constraint) or an unrecoverable numerical error was
      detected. The mesh is left in a consistent (if partially unlegalized)
      state either way.
     */
    bool insertConstraint( const int id0,
                           const int id1 );

};

}

#endif
