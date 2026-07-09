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

#include <rcsc/geom/triangulation_mesh.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/geom/vector_2d.h>

#include <vector>

namespace rcsc {

/*!
  \class DelaunayTriangulation
  \brief Delaunay triangulation

  The vertex/edge/triangle graph, incremental point insertion, and
  Lawson-flip legalization are implemented once in the TriangulationMesh
  base class (shared with ConstrainedDelaunayTriangulation). This class adds
  the point-location search strategies (hint-based adjacency walk, and an
  optional quadtree spatial index) and Voronoi-vertex bookkeeping that are
  specific to plain (unconstrained) Delaunay triangulation.
*/
class DelaunayTriangulation
    : public TriangulationMesh {
public:

    ////////////////////////////////////////////////////////////////
    /*!
      \enum SearchMethod
      \brief strategy used by findTriangleContains() to locate the triangle
      that contains a query point.
     */
    enum class SearchMethod {
        WALK,      //!< walk the triangle adjacency graph from a cached hint
                   //!< triangle (default). cheap to keep correct while the
                   //!< triangle set is being mutated, so this is what
                   //!< compute() always uses internally.
        QUAD_TREE, //!< static quadtree spatial index over the current
                   //!< triangle set, (re)built lazily the first time it is
                   //!< queried after the triangle set changes. faster than
                   //!< WALK when a triangulation is computed once and then
                   //!< queried many times without further mutation (e.g. a
                   //!< loaded/trained model used read-only).
    };

private:

    //! id of the triangle found by the previous findTriangleContains() call.
    //! used as the start triangle ("hint") of the next point location walk,
    //! since queries (and vertex insertion order) tend to be spatially coherent.
    mutable int M_hint_triangle_id = -1;

    //! search strategy used by the public findTriangleContains( const Vector2D & ).
    SearchMethod M_search_method = SearchMethod::WALK;

    //! opaque quadtree node type, defined in the .cpp file.
    class QuadTreeNode;

    //! root of the quadtree spatial index. only built/used when
    //! M_search_method == SearchMethod::QUAD_TREE. owning raw pointer,
    //! released by clearQuadTree().
    mutable QuadTreeNode * M_quad_tree_root = nullptr;

    //! reused across quadTreeFindTriangleContains() calls to avoid a heap
    //! allocation per query.
    mutable std::vector< TrianglePtr > M_quad_tree_candidates;

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
      \param pos coordinates of the target point
      \return const pointer to the found triangle. if no triangle, NULL is returned.
     */
    const
    Triangle * findTriangleContains( const Vector2D & pos ) const;

    /*!
      \brief select the search strategy used by the public
      findTriangleContains( const Vector2D & ). Does not affect the search
      compute() performs internally while building the triangulation, which
      always uses SearchMethod::WALK.
      \param method the new search strategy.
     */
    void setSearchMethod( SearchMethod method )
      {
          M_search_method = method;
      }

    /*!
      \brief get the search strategy currently used by the public
      findTriangleContains( const Vector2D & ).
      \return the current search strategy
     */
    SearchMethod searchMethod() const
      {
          return M_search_method;
      }

private:

    /*!
      \brief find triangle that contains pos from the computed triangle set.
      Internally walks the triangle adjacency graph starting from a cached
      hint triangle (O(sqrt(N)) expected), falling back to
      exhaustiveFindTriangleContains() if the walk cannot reach a conclusive
      answer, so the result is always identical to the exhaustive search.
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return how the vertex is contained.
     */
    ContainedType findTriangleContains( const Vector2D & pos,
                                        TrianglePtr * sol ) const;

    /*!
      \brief try to find the triangle that contains pos by walking through
      triangle adjacency (crossing from one triangle to its neighbor across
      whichever edge separates the current triangle from pos), starting from
      'start'.
      \param pos coordinates of the target point
      \param start triangle to start the walk from
      \param sol pointer to the solution variable.
      \return CONTAINED or ONLINE if conclusively found. NOT_CONTAINED means
      either pos is confirmed outside the triangulated region, or the walk
      could not reach a conclusive answer (e.g. floating point degeneracy or
      the step budget was exceeded) -- callers must not treat NOT_CONTAINED
      from this method as a definitive answer and should fall back to
      exhaustiveFindTriangleContains().
     */
    ContainedType walkTriangleContains( const Vector2D & pos,
                                        TrianglePtr start,
                                        TrianglePtr * sol ) const;

    /*!
      \brief find triangle that contains pos using the quadtree spatial
      index, (re)building it first if it is missing or stale. Intended for
      the case where the triangle set is computed once and then queried
      many times without further mutation.
      \param pos coordinates of the target point
      \param sol pointer to the solution variable.
      \return CONTAINED or ONLINE if conclusively found via the index.
      NOT_CONTAINED means the index could not conclusively resolve pos and
      the caller should fall back to findTriangleContains().
     */
    ContainedType quadTreeFindTriangleContains( const Vector2D & pos,
                                                TrianglePtr * sol ) const;

    /*!
      \brief (re)build the quadtree spatial index from the current triangle
      set. Any previous index is discarded first.
     */
    void buildQuadTree() const;

    /*!
      \brief release the quadtree spatial index, if any.
     */
    void clearQuadTree() const;

};

}

#endif
