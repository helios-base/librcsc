// -*-c++-*-

/*!
  \file delaunay_triangulation.cpp
  \brief Delaunay Triangulation class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 this library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "delaunay_triangulation.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

//#define DEBUG

namespace rcsc {

/*-------------------------------------------------------------------*/
DelaunayTriangulation::~DelaunayTriangulation()
{
    clear();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::clearResults()
{
    TriangulationMesh::clearResults();

    M_hint_triangle_id = -1;
    clearQuadTree();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::compute()
{
    if ( M_vertices.size() < 3 )
    {
        removeInitialVertices();
        return;
    }

    if ( M_triangles.empty()
         || M_triangles.size() > 3 )
    {
        clearResults();
        createInitialTriangle();
    }

    int loop = 0;
    for ( Vertex & v : M_vertices )
    {
        ++loop;
        // find triangle that contains 'vertex'
        TrianglePtr tri = nullptr;
        ContainedType type = findTriangleContains( v.pos(), &tri );

        ////////////////////////////////////////////////////
        if ( ! tri
             || type == NOT_CONTAINED )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " compute()"
                      << " could not determine ContainedType. "
                      << v.pos()
                      << std::endl;
            clearResults();
            return;
        }

        ////////////////////////////////////////////////////
        // new vertex is contained by old triangle
        if ( type == CONTAINED )
        {
            if ( ! updateContainedVertex( &v, tri ) )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ERROR in updateContainedVertex(). illegal vertex. index=" << loop
                          << std::endl;
                clearResults();
                return;
            }
        }
        else
        {
            // type == ONLINE
            if ( ! updateOnlineVertex( &v, tri ) )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ERROR in updateOnlineVertex(). illegal vertex. index=" << loop
                          << std::endl;
                clearResults();
                return;
            }
        }
    }

    removeInitialVertices();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::updateVoronoiVertex()
{
    for ( std::pair< const int, TrianglePtr > & v : M_triangles )
    {
        v.second->updateVoronoiVertex();
    }
}

/*-------------------------------------------------------------------*/
const
DelaunayTriangulation::Triangle *
DelaunayTriangulation::findTriangleContains( const Vector2D & pos ) const
{
    Triangle * tri = nullptr;

    if ( M_search_method == SearchMethod::QUAD_TREE )
    {
        const ContainedType result = quadTreeFindTriangleContains( pos, &tri );
        if ( result == CONTAINED || result == ONLINE )
        {
            return tri;
        }
        // the index could not conclusively resolve pos (e.g. pos outside
        // the triangulated region): fall back to the default search below.
        tri = nullptr;
    }

    findTriangleContains( pos, &tri );
    return tri;
}

/*-------------------------------------------------------------------*/
/*!
  Point location is the hot path of compute() (called once per inserted
  vertex while the triangle set is being mutated) and of repeated external
  queries (e.g. formation interpolation every cycle). Rather than maintain a
  separate spatial index that would need to be kept in sync with every
  triangle creation/removal, this walks the triangle adjacency graph that
  the triangulation already maintains via Edge::triangle(0/1), starting from
  the triangle found by the previous call. This is the standard
  "visibility/orientation walk" technique for point location in a Delaunay
  triangulation and is expected O(sqrt(N)) instead of O(N) per query.
  Whenever the walk cannot conclusively resolve pos (it reached the
  boundary of the mesh, hit the step budget, or hit a degenerate direction)
  exhaustiveFindTriangleContains() is used instead, so the result is always
  identical to what the plain exhaustive search would have returned.
*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::findTriangleContains( const Vector2D & pos,
                                             TrianglePtr * sol ) const
{
    if ( M_triangles.empty() )
    {
        return NOT_CONTAINED;
    }

    TriangleCont::const_iterator hint = M_triangles.find( M_hint_triangle_id );
    if ( hint != M_triangles.end() )
    {
        TrianglePtr start = hint->second;
        ContainedType result = walkTriangleContains( pos, start, sol );
        if ( result == CONTAINED || result == ONLINE )
        {
            M_hint_triangle_id = (*sol)->id();
            return result;
        }
    }

    ContainedType result = exhaustiveFindTriangleContains( pos, sol );
    if ( result == CONTAINED || result == ONLINE )
    {
        M_hint_triangle_id = (*sol)->id();
    }
    return result;
}

/*-------------------------------------------------------------------*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::walkTriangleContains( const Vector2D & pos,
                                             TrianglePtr start,
                                             TrianglePtr * sol ) const
{
    TrianglePtr tri = start;

    // generous but bounded step budget: a converging walk on a Delaunay
    // triangulation takes O(sqrt(N)) steps on average, so hitting this
    // means the walk is not converging (degenerate input, numerical
    // corner case) and the exhaustive fallback should take over.
    const std::size_t max_steps = M_triangles.size() + 8;

    for ( std::size_t step = 0; tri && step < max_steps; ++step )
    {
        const Vector2D rel0( tri->vertex( 0 )->pos() - pos );
        const Vector2D rel1( tri->vertex( 1 )->pos() - pos );
        const Vector2D rel2( tri->vertex( 2 )->pos() - pos );

        const double outer0 = rel0.outerProduct( rel1 );
        const double outer1 = rel1.outerProduct( rel2 );
        const double outer2 = rel2.outerProduct( rel0 );

        // edge index whose opposite triangle should be visited next if pos
        // turns out not to be online/contained in 'tri'. 3 == "unknown".
        std::size_t cross_edge = 3;

        if ( std::fabs( outer0 ) <= EPSILON )
        {
            if ( ! ( rel0.x * rel1.x > EPSILON || rel0.y * rel1.y > EPSILON ) )
            {
                *sol = tri;
                return ONLINE;
            }
        }
        else if ( std::fabs( outer1 ) <= EPSILON )
        {
            if ( ! ( rel1.x * rel2.x > EPSILON || rel1.y * rel2.y > EPSILON ) )
            {
                *sol = tri;
                return ONLINE;
            }
        }
        else if ( std::fabs( outer2 ) <= EPSILON )
        {
            if ( ! ( rel2.x * rel0.x > EPSILON || rel2.y * rel0.y > EPSILON ) )
            {
                *sol = tri;
                return ONLINE;
            }
        }
        else if ( ( outer0 >= 0.0 && outer1 >= 0.0 && outer2 >= 0.0 )
                  || ( outer0 <= 0.0 && outer1 <= 0.0 && outer2 <= 0.0 ) )
        {
            *sol = tri;
            return CONTAINED;
        }
        else
        {
            // pos is outside 'tri'. vertices 0,1,2 are consistently wound
            // (all CW or all CCW), so the sign of the triangle's own
            // (unsigned by pos) signed area tells us, for each edge, which
            // side is "inside" -- an outer_i disagreeing with that sign
            // means pos is beyond edge i, so cross into its neighbor.
            const double signed_area
                = ( tri->vertex( 1 )->pos() - tri->vertex( 0 )->pos() )
                .outerProduct( tri->vertex( 2 )->pos() - tri->vertex( 0 )->pos() );

            if ( signed_area >= 0.0 )
            {
                if ( outer0 < -EPSILON ) cross_edge = 0;
                else if ( outer1 < -EPSILON ) cross_edge = 1;
                else if ( outer2 < -EPSILON ) cross_edge = 2;
            }
            else
            {
                if ( outer0 > EPSILON ) cross_edge = 0;
                else if ( outer1 > EPSILON ) cross_edge = 1;
                else if ( outer2 > EPSILON ) cross_edge = 2;
            }
        }

        if ( cross_edge == 3 )
        {
            // could not determine a walk direction (should not normally
            // happen); let the caller fall back to the exhaustive search.
            return NOT_CONTAINED;
        }

        const Vertex * va = tri->vertex( cross_edge );
        const Vertex * vb = tri->vertex( ( cross_edge + 1 ) % 3 );
        Edge * e = tri->getEdgeInclude( va, vb );

        // if the neighbor is null, 'e' is on the boundary of the
        // triangulated region and pos is outside it in this direction; the
        // loop condition (tri == nullptr) ends the walk on the next check.
        tri = ( e->triangle( 0 ) == tri ? e->triangle( 1 ) : e->triangle( 0 ) );
    }

    return NOT_CONTAINED;
}

/*-------------------------------------------------------------------*/
/*!
  Nested type backing SearchMethod::QUAD_TREE. Defined here (rather than in
  the header) since it is a pure implementation detail: a static index built
  once (lazily, on first use after the triangle set changes) over whichever
  triangles exist at that time, then queried read-only. This is a good
  trade-off only when the triangulation is computed once and then queried
  many times without further mutation -- compute() itself never uses it, so
  there is no cost paid for the frequent create/remove churn of incremental
  construction.
*/
class DelaunayTriangulation::QuadTreeNode {
public:
    static constexpr int MAX_DEPTH = 12;
    static constexpr std::size_t LEAF_CAPACITY = 8;

private:
    Rect2D M_bounds;
    bool M_leaf;
    std::array< QuadTreeNode *, 4 > M_children;
    std::vector< TrianglePtr > M_triangles;

    QuadTreeNode( const QuadTreeNode & ) = delete;
    QuadTreeNode & operator=( const QuadTreeNode & ) = delete;

public:
    explicit
    QuadTreeNode( const Rect2D & bounds )
        : M_bounds( bounds ),
          M_leaf( true )
      {
          M_children.fill( nullptr );
      }

    ~QuadTreeNode()
      {
          for ( QuadTreeNode * c : M_children )
          {
              delete c;
          }
      }

    /*!
      \brief build a (sub)tree covering 'bounds' that indexes 'items'
      (triangle + its axis-aligned bounding box).
     */
    static
    QuadTreeNode * build( const Rect2D & bounds,
                         const std::vector< std::pair< TrianglePtr, Rect2D > > & items,
                         const int depth )
      {
          QuadTreeNode * node = new QuadTreeNode( bounds );

          if ( items.size() <= LEAF_CAPACITY || depth >= MAX_DEPTH )
          {
              node->M_triangles.reserve( items.size() );
              for ( const auto & item : items )
              {
                  node->M_triangles.push_back( item.first );
              }
              return node;
          }

          const Vector2D c = bounds.center();
          const Rect2D quad_bounds[4] = {
              Rect2D( bounds.topLeft(), c ),                                       // x<=c.x, y<=c.y
              Rect2D( Vector2D( c.x, bounds.top() ), Vector2D( bounds.right(), c.y ) ), // x>c.x, y<=c.y
              Rect2D( Vector2D( bounds.left(), c.y ), Vector2D( c.x, bounds.bottom() ) ), // x<=c.x, y>c.y
              Rect2D( c, bounds.bottomRight() ),                                   // x>c.x, y>c.y
          };

          std::array< std::vector< std::pair< TrianglePtr, Rect2D > >, 4 > buckets;
          for ( const auto & item : items )
          {
              for ( int q = 0; q < 4; ++q )
              {
                  if ( quad_bounds[q].left() <= item.second.right()
                       && item.second.left() <= quad_bounds[q].right()
                       && quad_bounds[q].top() <= item.second.bottom()
                       && item.second.top() <= quad_bounds[q].bottom() )
                  {
                      buckets[q].push_back( item );
                  }
              }
          }

          bool shrank = false;
          for ( int q = 0; q < 4; ++q )
          {
              if ( buckets[q].size() < items.size() )
              {
                  shrank = true;
                  break;
              }
          }

          if ( ! shrank )
          {
              // subdividing did not shrink the candidate set in any
              // quadrant (e.g. every remaining triangle straddles the
              // center): stop here instead of recursing without end.
              node->M_triangles.reserve( items.size() );
              for ( const auto & item : items )
              {
                  node->M_triangles.push_back( item.first );
              }
              return node;
          }

          node->M_leaf = false;
          for ( int q = 0; q < 4; ++q )
          {
              if ( ! buckets[q].empty() )
              {
                  node->M_children[q] = build( quad_bounds[q], buckets[q], depth + 1 );
              }
          }
          return node;
      }

    /*!
      \brief append every triangle indexed by the leaf cell that contains
      'pos' to 'candidates'. 'candidates' is a superset of triangles that
      might contain 'pos'; the caller must still verify each one.
     */
    void collectCandidates( const Vector2D & pos,
                            std::vector< TrianglePtr > & candidates ) const
      {
          if ( M_leaf )
          {
              candidates.insert( candidates.end(), M_triangles.begin(), M_triangles.end() );
              return;
          }

          const Vector2D c = M_bounds.center();
          const int q = ( pos.x > c.x ? 1 : 0 ) + ( pos.y > c.y ? 2 : 0 );
          if ( M_children[q] )
          {
              M_children[q]->collectCandidates( pos, candidates );
          }
      }
};

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::buildQuadTree() const
{
    clearQuadTree();
    M_topology_dirty = false;

    if ( M_triangles.empty() )
    {
        return;
    }

    double min_x = std::numeric_limits< double >::max();
    double max_x = std::numeric_limits< double >::lowest();
    double min_y = std::numeric_limits< double >::max();
    double max_y = std::numeric_limits< double >::lowest();

    std::vector< std::pair< TrianglePtr, Rect2D > > items;
    items.reserve( M_triangles.size() );

    for ( const std::pair< const int, TrianglePtr > & v : M_triangles )
    {
        const TrianglePtr tri = v.second;

        double tminx = tri->vertex( 0 )->pos().x;
        double tmaxx = tminx;
        double tminy = tri->vertex( 0 )->pos().y;
        double tmaxy = tminy;
        for ( std::size_t i = 1; i < 3; ++i )
        {
            const Vector2D & p = tri->vertex( i )->pos();
            tminx = std::min( tminx, p.x );
            tmaxx = std::max( tmaxx, p.x );
            tminy = std::min( tminy, p.y );
            tmaxy = std::max( tmaxy, p.y );
        }

        min_x = std::min( min_x, tminx );
        max_x = std::max( max_x, tmaxx );
        min_y = std::min( min_y, tminy );
        max_y = std::max( max_y, tmaxy );

        items.emplace_back( tri, Rect2D( Vector2D( tminx, tminy ), Vector2D( tmaxx, tmaxy ) ) );
    }

    // small padding so a query point exactly on the outer boundary still
    // falls strictly inside the root cell.
    const double pad = 1.0;
    const Rect2D root_bounds( Vector2D( min_x - pad, min_y - pad ),
                              Vector2D( max_x + pad, max_y + pad ) );

    M_quad_tree_root = QuadTreeNode::build( root_bounds, items, 0 );
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::clearQuadTree() const
{
    delete M_quad_tree_root;
    M_quad_tree_root = nullptr;
}

/*-------------------------------------------------------------------*/
DelaunayTriangulation::ContainedType
DelaunayTriangulation::quadTreeFindTriangleContains( const Vector2D & pos,
                                                     TrianglePtr * sol ) const
{
    if ( M_topology_dirty || ! M_quad_tree_root )
    {
        buildQuadTree();
    }

    if ( ! M_quad_tree_root )
    {
        return NOT_CONTAINED;
    }

    M_quad_tree_candidates.clear();
    M_quad_tree_root->collectCandidates( pos, M_quad_tree_candidates );

    for ( TrianglePtr tri : M_quad_tree_candidates )
    {
        const ContainedType result = classifyPoint( tri, pos );
        if ( result != NOT_CONTAINED )
        {
            *sol = tri;
            return result;
        }
    }

    return NOT_CONTAINED;
}

}
