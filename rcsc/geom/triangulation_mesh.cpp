// -*-c++-*-

/*!
  \file triangulation_mesh.cpp
  \brief shared planar triangle mesh (vertex/edge/triangle graph plus
  incremental point insertion and edge-flip legalization) Source File.
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

#include "triangulation_mesh.h"

#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/triangle_2d.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <utility>

namespace rcsc {

const double TriangulationMesh::EPSILON = 1.0e-10;

namespace {

const std::pair< std::size_t, std::size_t >
edge_pairs[3] = { std::pair< std::size_t, std::size_t >( 0, 1 ),
                  std::pair< std::size_t, std::size_t >( 1, 2 ),
                  std::pair< std::size_t, std::size_t >( 2, 0 ),
};

//! two vertices closer than this squared distance are treated as the same
//! coordinate by addVertex()/addVertices().
constexpr double VERTEX_MERGE_DIST2 = 1.0e-6;

//! grid cell coordinate type used by the spatial hash in addVertices().
typedef std::pair< long, long > GridCell;

/*-------------------------------------------------------------------*/
struct GridCellHash {
    std::size_t operator()( const GridCell & cell ) const
    {
        return std::hash< long >()( cell.first ) * 73856093u
            ^ std::hash< long >()( cell.second ) * 19349663u;
    }
};

/*-------------------------------------------------------------------*/
//! cell containing (x,y) when the plane is tiled with cell_size squares.
inline
GridCell
grid_cell( const double x,
           const double y,
           const double cell_size )
{
    return GridCell( static_cast< long >( std::floor( x / cell_size ) ),
                     static_cast< long >( std::floor( y / cell_size ) ) );
}

}

/*-------------------------------------------------------------------*/
TriangulationMesh::Triangle::Triangle( const int id,
                                       EdgePtr e0,
                                       EdgePtr e1,
                                       EdgePtr e2 )
    : M_id( id ),
      M_voronoi_vertex( Vector2D::INVALIDATED )
{
    M_edges[0] = e0;
    M_edges[1] = e1;
    M_edges[2] = e2;

    // set this pointer to edges
    for ( std::size_t i = 0; i < 3; ++i )
    {
        M_edges[i]->setTriangle( this );
    }

    // set vertices
    M_vertices[0] = M_edges[0]->vertex( 0 );
    M_vertices[1] = M_edges[0]->vertex( 1 );
    M_vertices[2] = ( ( M_vertices[0] != M_edges[1]->vertex( 0 )
                        && M_vertices[1] != M_edges[1]->vertex( 0 ) )
                      ? M_edges[1]->vertex( 0 )
                      : M_edges[1]->vertex( 1 ) );

    // set circumcircle data
    M_circumcenter = Triangle2D::circumcenter( M_vertices[0]->pos(),
                                               M_vertices[1]->pos(),
                                               M_vertices[2]->pos() );

    M_circumradius = M_circumcenter.dist( M_vertices[0]->pos() );
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::Triangle::updateVoronoiVertex()
{
    Line2D l1 = Line2D::perpendicular_bisector( M_vertices[0]->pos(),
                                                M_vertices[1]->pos() );
    Line2D l2 = Line2D::perpendicular_bisector( M_vertices[1]->pos(),
                                                M_vertices[2]->pos() );

    M_voronoi_vertex = l1.intersection( l2 );
    if ( ! M_voronoi_vertex.isValid() )
    {
        l2 = Line2D::perpendicular_bisector( M_vertices[0]->pos(),
                                             M_vertices[2]->pos() );
        M_voronoi_vertex = l1.intersection( l2 );

        if ( ! M_voronoi_vertex.isValid() )
        {
            std::cerr << "(TriangulationMesh::Triangle::updateVoronoiVertex):"
                      << " Could not calculate the vertex candidate point."
                      << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::clearResults()
{
    M_edge_count = 0;
    M_tri_count = 0;

    for ( std::pair< const int, TrianglePtr > & t : M_triangles )
    {
        delete t.second;
    }
    for ( std::pair< const int, EdgePtr > & e : M_edges )
    {
        delete e.second;
    }

    M_triangles.clear();
    M_edges.clear();

    M_topology_dirty = true;
    M_hint_triangle_id = -1;
    clearQuadTree();
}

/*-------------------------------------------------------------------*/
int
TriangulationMesh::addVertex( const double x,
                              const double y )
{
    for ( const Vertex & v : M_vertices )
    {
        if ( v.pos().dist2( Vector2D( x, y ) ) < VERTEX_MERGE_DIST2 )
        {
            // detect same coordinate vertex
            return -1;
        }
    }

    const int id = static_cast< int >( M_vertices.size() );
    M_vertices.emplace_back( id, x, y );
    return id;
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::addVertices( const std::vector< Vector2D > & v )
{
    if ( v.empty() )
    {
        return;
    }

    M_vertices.reserve( M_vertices.size() + v.size() );

    // duplicate-coordinate check equivalent to addVertex(), but backed by a
    // spatial hash instead of a linear scan: a vertex duplicated within 'v'
    // (or already present in M_vertices) must still be rejected here, since
    // the incremental insertion algorithm would otherwise later hit a
    // vertex sitting exactly on top of an existing one, which trips the
    // "same vertex in old triangle" safety check in updateOnlineVertex()
    // and discards the whole triangulation. Bucketing by a cell size equal
    // to the merge distance guarantees any existing vertex within that
    // distance of a candidate lies in one of the 3x3 neighboring cells, so
    // each lookup is O(1) on average instead of addVertex()'s O(current
    // vertex count) scan repeated for every point in 'v'.
    const double cell_size = std::sqrt( VERTEX_MERGE_DIST2 );

    std::unordered_map< GridCell, std::vector< std::size_t >, GridCellHash > grid;
    grid.reserve( M_vertices.size() + v.size() );

    for ( std::size_t i = 0; i < M_vertices.size(); ++i )
    {
        const Vector2D & p = M_vertices[i].pos();
        grid[ grid_cell( p.x, p.y, cell_size ) ].push_back( i );
    }

    for ( const Vector2D & d : v )
    {
        const GridCell c = grid_cell( d.x, d.y, cell_size );

        bool duplicate = false;
        for ( long dx = -1; dx <= 1 && ! duplicate; ++dx )
        {
            for ( long dy = -1; dy <= 1 && ! duplicate; ++dy )
            {
                const auto it = grid.find( GridCell( c.first + dx, c.second + dy ) );
                if ( it == grid.end() )
                {
                    continue;
                }

                for ( const std::size_t idx : it->second )
                {
                    if ( M_vertices[idx].pos().dist2( d ) < VERTEX_MERGE_DIST2 )
                    {
                        duplicate = true;
                        break;
                    }
                }
            }
        }

        if ( duplicate )
        {
            // detect same coordinate vertex
            continue;
        }

        const int id = static_cast< int >( M_vertices.size() );
        M_vertices.emplace_back( id, d.x, d.y );
        grid[c].push_back( M_vertices.size() - 1 );
    }
}

/*-------------------------------------------------------------------*/
const
TriangulationMesh::Vertex *
TriangulationMesh::getVertex( const int id ) const
{
    if ( M_vertices.empty()
         || id < 0
         || static_cast< int >( M_vertices.size() ) <= id )
    {
        return nullptr;
    }
    return &M_vertices[id];
}

/*-------------------------------------------------------------------*/
const
TriangulationMesh::Vertex *
TriangulationMesh::findNearestVertex( const Vector2D & pos ) const
{
    const Vertex * candidate = nullptr;

    double min_dist2 = std::numeric_limits< double >::max();
    for ( const Vertex & v : M_vertices )
    {
        const double d2 = v.pos().dist2( pos );
        if ( d2 < min_dist2 )
        {
            candidate = &v;
            min_dist2 = d2;
        }
    }

    return candidate;
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::createInitialTriangle( const Rect2D & region )
{
    // create double size rectangle
    const double max_size = std::max( region.size().length() + 1.0,
                                      region.size().width() + 1.0 );
    const Vector2D center = region.center();

    M_initial_vertex[0].assign( -1,
                                center.x + std::max( 1000.0 * max_size, 1000.0 ),
                                center.y );

    M_initial_vertex[1].assign( -2,
                                center.x,
                                center.y + std::max( 1000.0 * max_size, 1000.0 ) );

    M_initial_vertex[2].assign( -3,
                                center.x - std::max( 1000.0 * max_size, 1000.0 ),
                                center.y - std::max( 1000.0 * max_size, 1000.0 ) );

    EdgePtr edge0 = createEdge( &M_initial_vertex[0],
                                &M_initial_vertex[1] );
    EdgePtr edge1 = createEdge( &M_initial_vertex[1],
                                &M_initial_vertex[2] );
    EdgePtr edge2 = createEdge( &M_initial_vertex[2],
                                &M_initial_vertex[0] );

    createTriangle( edge0, edge1, edge2 );
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::createInitialTriangle()
{
    if ( M_vertices.empty() )
    {
        return;
    }

    double min_x = M_vertices.front().pos().x;
    double max_x = min_x;
    double min_y = M_vertices.front().pos().y;
    double max_y = min_y;

    for ( const Vertex & v : M_vertices )
    {
        if ( v.pos().x < min_x ) min_x = v.pos().x;
        else if ( max_x < v.pos().x ) max_x = v.pos().x;

        if ( v.pos().y < min_y ) min_y = v.pos().y;
        else if ( max_y < v.pos().y ) max_y = v.pos().y;
    }

    createInitialTriangle( Rect2D( Vector2D( min_x - 1.0, min_y - 1.0 ),
                                   Vector2D( max_x + 1.0, max_y + 1.0 ) ) );
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::removeInitialVertices()
{
    std::vector< EdgePtr > removed_edges;

    // search removed edges that has initial vertex
    for ( std::pair< const int, EdgePtr > & v : M_edges )
    {
        for ( std::size_t i = 0; i < 3; ++i )
        {
            if ( v.second->vertex( 0 ) == &M_initial_vertex[i]
                 || v.second->vertex( 1 ) == &M_initial_vertex[i] )
            {
                removed_edges.push_back( v.second );
                break;
            }
        }
    }

    // remove edges and triangles
    for ( EdgePtr edge : removed_edges )
    {
        removeTriangle( edge->triangle( 0 ) );
        removeTriangle( edge->triangle( 1 ) );

        removeEdge( edge->id() );
    }
}

/*-------------------------------------------------------------------*/
bool
TriangulationMesh::updateContainedVertex( const Vertex * new_vertex,
                                          const TrianglePtr tri )
{
    // split 'tri' to 3 pieces
    // --> create new 3 triangle in 'tri'

    // create new edge
    EdgePtr new_edges[3];
    for ( std::size_t i = 0; i < 3; ++i )
    {
        // *** tri->vertex(i) must be the second argument!!
        new_edges[i] = createEdge( new_vertex, tri->vertex( i ) );
    }

    // create child triangles
    EdgePtr old_edges[3]; // edges of 'tri'
    TrianglePtr new_tri[3];

    for ( std::size_t i = 0; i < 3; ++i )
    {
        old_edges[i]
            = tri->getEdgeInclude( new_edges[ edge_pairs[i].first ]->vertex( 1 ),
                                   new_edges[ edge_pairs[i].second ]->vertex( 1 ) );
        old_edges[i]->removeTriangle( tri );
        new_tri[i] = createTriangle( old_edges[i],
                                     new_edges[ edge_pairs[i].first ],
                                     new_edges[ edge_pairs[i].second ] );
        if ( ! new_tri[i]->circumcenter().isValid() )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " TriangulationMesh::updateContainedVertex()"
                      << " detected an illegal (degenerate) triangle."
                      << std::endl;
            return false;
        }
    }

    // remove old triangle
    removeTriangle( tri );

    // legalize new triangles
    for ( std::size_t i = 0; i < 3; ++i )
    {
        if ( ! legalizeEdge( old_edges[i], new_vertex ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
TriangulationMesh::updateOnlineVertex( const Vertex * new_vertex,
                                       const TrianglePtr tri )
{
    // find edge that vertex is on-line
    int online_count = 0;
    EdgePtr online_edge = nullptr;
    for ( std::size_t i = 0; i < 3; ++i )
    {
        const Vector2D rel0( tri->edge( i )->vertex( 0 )->pos() - new_vertex->pos() );
        const Vector2D rel1( tri->edge( i )->vertex( 1 )->pos() - new_vertex->pos() );
        // check area value of sub triangle
        if ( std::fabs( rel0.outerProduct( rel1 ) ) <= EPSILON )
        {
            online_edge = tri->edge( i );
            ++online_count;
        }
    }

    if ( online_count >= 2 )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** TriangulationMesh::updateOnlineVertex()."
                  << " detect the same vertex in old triangle="
                  << tri->vertex( 0 )->pos()
                  << tri->vertex( 1 )->pos()
                  << tri->vertex( 2 )->pos()
                  << " illegal_vertex=" << new_vertex->pos()
                  << std::endl;
        return false;
    }

    if ( ! online_edge )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** TriangulationMesh::updateOnlineVertex()."
                  << " failed to find online edge."
                  << " illegal_vertex=" << new_vertex->pos()
                  << std::endl;
        return false;
    }

    if ( online_edge->constrained() )
    {
        // a newly inserted site landed exactly on an already constrained
        // edge. this can only happen for pathological (near-)duplicate
        // input coordinates, since constraints (when used at all, by
        // ConstrainedDelaunayTriangulation) are only recovered after all
        // sites have been inserted; refuse rather than silently splitting a
        // constraint edge.
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** TriangulationMesh::updateOnlineVertex()."
                  << " new vertex lies on a constrained edge."
                  << std::endl;
        return false;
    }

    ////////////////////////////////////////////////////////////////

    // create child edge of 'online_edge'
    EdgePtr new_edge[2]; // edges that is shared by 'tri' and 'adjacent_tri'
    for ( std::size_t i = 0; i < 2; ++i )
    {
        new_edge[i] = createEdge( new_vertex, online_edge->vertex( i ) );
    }

    ////////////////////////////////////////////////////////////////

    TrianglePtr new_tri_in_tri[2];
    EdgePtr old_edge_in_tri[2];

    // create new child triangles in 'tri'
    {
        // get vertex that is not on the online_edge.
        const Vertex * tri_vertex = tri->getVertexExclude( online_edge );

        if ( ! tri_vertex )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " TriangulationMesh::updateOnlineVertex()."
                      << " failed to find vertex of tri."
                      << std::endl;
            return false;
        }

        EdgePtr new_edge_in_tri = createEdge( new_vertex, tri_vertex );

        // create new child triangles in tri
        for ( std::size_t i = 0; i < 2; ++i )
        {
            old_edge_in_tri[i] = tri->getEdgeInclude( new_edge[i]->vertex( 1 ),
                                                      tri_vertex );
            // remove old triangle from old edge
            old_edge_in_tri[i]->removeTriangle( tri );
            // first argument edge must be old existing edge
            new_tri_in_tri[i] = createTriangle( old_edge_in_tri[i],
                                                new_edge[i],
                                                new_edge_in_tri );
            if ( ! new_tri_in_tri[i]->circumcenter().isValid() )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " TriangulationMesh::updateOnlineVertex()"
                          << " detected an illegal (degenerate) triangle."
                          << std::endl;
                return false;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    // get adjacent triangle that shares online_edge
    Triangle * adjacent = ( online_edge->triangle( 0 ) == tri
                            ? online_edge->triangle( 1 )
                            : online_edge->triangle( 0 ) );
    const bool exist_adjacent = ( adjacent != nullptr );

    TrianglePtr new_tri_in_adjacent[2];
    EdgePtr old_edge_in_adjacent[2];

    // create new child triangles in 'adjacent'
    if ( exist_adjacent )
    {
        const Vertex * adjacent_vertex = adjacent->getVertexExclude( online_edge );

        if ( ! adjacent_vertex )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " TriangulationMesh::updateOnlineVertex()."
                      << " failed to find vertex of adjacent."
                      << std::endl;
            return false;
        }

        EdgePtr new_edge_in_adjacent = createEdge( new_vertex, adjacent_vertex );

        // create new child triangles in tri
        for ( std::size_t i = 0; i < 2; ++i )
        {
            old_edge_in_adjacent[i]
                = adjacent->getEdgeInclude( new_edge[i]->vertex( 1 ),
                                            adjacent_vertex );
            // remove old triangle from old edge
            old_edge_in_adjacent[i]->removeTriangle( adjacent );
            // first argument edge must be old existing edge
            new_tri_in_adjacent[i] = createTriangle( old_edge_in_adjacent[i],
                                                     new_edge[i],
                                                     new_edge_in_adjacent );
            if ( ! new_tri_in_adjacent[i]->circumcenter().isValid() )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " TriangulationMesh::updateOnlineVertex()"
                          << " detected an illegal (degenerate) triangle."
                          << std::endl;
                return false;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    // remove old triangles & edge from memory map
    removeTriangle( tri );
    if ( exist_adjacent )
    {
        removeTriangle( adjacent );
    }
    removeEdge( online_edge );

    ////////////////////////////////////////////////////////////////

    // legalize new triangles
    for ( std::size_t i = 0; i < 2; ++i )
    {
        if ( ! legalizeEdge( old_edge_in_tri[i], new_vertex ) )
        {
            return false;
        }
    }

    if ( exist_adjacent )
    {
        for ( std::size_t i = 0; i < 2; ++i )
        {
            if ( ! legalizeEdge( old_edge_in_adjacent[i], new_vertex ) )
            {
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
TriangulationMesh::isLocallyDelaunay( const EdgePtr edge ) const
{
    if ( ! edge || edge->constrained() )
    {
        return true;
    }

    TrianglePtr t0 = edge->triangle( 0 );
    TrianglePtr t1 = edge->triangle( 1 );
    if ( ! t0 || ! t1 )
    {
        // boundary edge: nothing on the other side to compare against.
        return true;
    }

    const Vertex * v1 = t1->getVertexExclude( edge );
    if ( ! v1 )
    {
        return true;
    }

    return ! t0->contains( v1->pos() );
}

/*-------------------------------------------------------------------*/
bool
TriangulationMesh::legalizeEdge( EdgePtr edge,
                                 const Vertex * new_vertex )
{
    if ( isLocallyDelaunay( edge ) )
    {
        return true;
    }

    TrianglePtr t0 = edge->triangle( 0 );
    TrianglePtr t1 = edge->triangle( 1 );

    const Vertex * v0 = t0->getVertexExclude( edge );
    const Vertex * v1 = t1->getVertexExclude( edge );
    if ( ! v0 || ! v1 )
    {
        return true;
    }

    if ( ! new_vertex )
    {
        // a flip is only geometrically valid if the quadrilateral formed by
        // the two triangles is convex, i.e. the diagonal v0-v1 properly
        // crosses the shared edge. When new_vertex is given (the
        // point-insertion path), this is skipped: the reference algorithm
        // this recursion replicates never performs this check, because
        // convexity is *provably* guaranteed there (the illegal triangle's
        // opposite point is being tested from inside what was, before this
        // insertion, a single Delaunay-legal triangle). Skipping it here
        // matters in practice, not just for fidelity: for near-degenerate
        // (near-collinear/near-cocircular) inputs the quadrilateral can be
        // so thin that this test's own floating-point evaluation becomes
        // unreliable right at its threshold, and a spurious "not convex"
        // would silently leave a genuinely illegal edge unflipped -- for
        // ConstrainedDelaunayTriangulation's relaxation (new_vertex is
        // null), convexity is *not* guaranteed (constraint edges can force
        // locally non-convex neighborhoods), so the check stays essential
        // there.
        const Segment2D diagonal( v0->pos(), v1->pos() );
        const Segment2D shared( edge->vertex( 0 )->pos(), edge->vertex( 1 )->pos() );
        if ( ! diagonal.existIntersectionExceptEndpoint( shared ) )
        {
            return true;
        }
    }

    EdgePtr outer_edges[4];
    EdgePtr new_edge = nullptr;
    if ( ! flipEdge( edge, outer_edges, &new_edge, new_vertex ) )
    {
        return false;
    }

    if ( new_vertex && ( v0 == new_vertex || v1 == new_vertex ) )
    {
        // replicate the textbook point-insertion algorithm's recursion
        // exactly: only re-check the two edges of the new triangle that
        // does *not* contain new_vertex (the other two are provably
        // already legal). flipEdge() canonicalized the ordering above, so
        // that triangle's edges are always outer_edges[2]/[3] here.
        return legalizeEdge( outer_edges[2], new_vertex )
            && legalizeEdge( outer_edges[3], new_vertex );
    }

    // generic fallback: neither triangle's opposite vertex is new_vertex
    // (the ConstrainedDelaunayTriangulation relaxation path, where there is
    // no such vertex at all) -- re-check everything a flip could affect.
    for ( int i = 0; i < 4; ++i )
    {
        if ( ! legalizeEdge( outer_edges[i], new_vertex ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
TriangulationMesh::flipEdge( EdgePtr edge,
                             EdgePtr outer_edges[4],
                             EdgePtr * new_edge,
                             const Vertex * new_vertex )
{
    TrianglePtr t0 = edge->triangle( 0 );
    TrianglePtr t1 = edge->triangle( 1 );

    const Vertex * v0 = t0->getVertexExclude( edge );
    const Vertex * v1 = t1->getVertexExclude( edge );

    if ( new_vertex && v1 == new_vertex )
    {
        // canonicalize so that, whenever new_vertex is one of the two
        // triangles' opposite vertices, it ends up as v0/t0: this matches
        // the exact vertex/edge argument order the reference point-
        // insertion algorithm always uses (new_vertex first), which in turn
        // determines the operand order fed to the circumcenter formula for
        // the two new triangles. Mathematically the result is the same
        // either way, but for a near-degenerate (near-collinear/near-
        // cocircular) triangle the formula's floating-point rounding is
        // sensitive to operand order -- matching this convention is what
        // makes legalizeEdge() converge to the same fixed point as the
        // reference algorithm among several inputs that are all "locally
        // Delaunay legal" for such inputs. It also lets the caller always
        // find the "far from new_vertex" side at outer_edges[2]/[3].
        std::swap( t0, t1 );
        std::swap( v0, v1 );
    }

    EdgePtr e_t0[2];
    EdgePtr e_t1[2];

    {
        std::size_t idx = 0;
        for ( std::size_t i = 0; i < 3; ++i )
        {
            if ( t0->edge( i ) != edge )
            {
                e_t0[idx] = t0->edge( i );
                e_t0[idx]->removeTriangle( t0 );
                ++idx;
            }
        }

        idx = 0;
        for ( std::size_t i = 0; i < 3; ++i )
        {
            if ( t1->edge( i ) != edge )
            {
                e_t1[idx] = t1->edge( i );
                e_t1[idx]->removeTriangle( t1 );
                ++idx;
            }
        }

        // pair up the edges so that e_t0[i] and e_t1[i] share a common
        // vertex (one of the two endpoints of the flipped-away edge), which
        // is required for createTriangle() to form a valid triangle.
        if ( e_t1[1]->hasVertex( e_t0[0]->vertex( 0 ) )
             || e_t1[1]->hasVertex( e_t0[0]->vertex( 1 ) ) )
        {
            std::swap( e_t1[0], e_t1[1] );
        }
    }

    EdgePtr new_diagonal = createEdge( v0, v1 );

    removeTriangle( t0 );
    removeTriangle( t1 );
    removeEdge( edge );

    TrianglePtr nt0 = createTriangle( new_diagonal, e_t0[0], e_t1[0] );
    TrianglePtr nt1 = createTriangle( new_diagonal, e_t0[1], e_t1[1] );

    outer_edges[0] = e_t0[0];
    outer_edges[1] = e_t0[1];
    outer_edges[2] = e_t1[0];
    outer_edges[3] = e_t1[1];
    *new_edge = new_diagonal;

    if ( ! nt0->circumcenter().isValid() || ! nt1->circumcenter().isValid() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " TriangulationMesh::flipEdge()"
                  << " detected an illegal (degenerate) triangle after flip."
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
TriangulationMesh::ContainedType
TriangulationMesh::classifyPoint( const TrianglePtr tri,
                                  const Vector2D & pos )
{
    if ( std::fabs( tri->circumcenter().x - pos.x ) > tri->circumradius()
         || std::fabs( tri->circumcenter().y - pos.y ) > tri->circumradius() )
    {
        // out of circumcircle; since the circumcircle always encloses the
        // triangle, pos cannot be inside (or on) the triangle either.
        return NOT_CONTAINED;
    }

    const Vector2D rel0( tri->vertex( 0 )->pos() - pos );
    const Vector2D rel1( tri->vertex( 1 )->pos() - pos );
    const Vector2D rel2( tri->vertex( 2 )->pos() - pos );

    const double outer0 = rel0.outerProduct( rel1 );
    const double outer1 = rel1.outerProduct( rel2 );
    const double outer2 = rel2.outerProduct( rel0 );

    if ( std::fabs( outer0 ) <= EPSILON )
    {
        if ( rel0.x * rel1.x > EPSILON
             || rel0.y * rel1.y > EPSILON )
        {
            // not online
            return NOT_CONTAINED;
        }
        return ONLINE;
    }

    if ( std::fabs( outer1 ) <= EPSILON )
    {
        if ( rel1.x * rel2.x > EPSILON
             || rel1.y * rel2.y > EPSILON )
        {
            return NOT_CONTAINED;
        }
        return ONLINE;
    }

    if ( std::fabs( outer2 ) <= EPSILON )
    {
        if ( rel2.x * rel0.x > EPSILON
             || rel2.y * rel0.y > EPSILON )
        {
            return NOT_CONTAINED;
        }
        return ONLINE;
    }

    if ( ( outer0 >= 0.0 && outer1 >= 0.0 && outer2 >= 0.0 )
         || ( outer0 <= 0.0 && outer1 <= 0.0 && outer2 <= 0.0 ) )
    {
        return CONTAINED;
    }

    return NOT_CONTAINED;
}

/*-------------------------------------------------------------------*/
TriangulationMesh::ContainedType
TriangulationMesh::exhaustiveFindTriangleContains( const Vector2D & pos,
                                                   TrianglePtr * sol ) const
{
    for ( const std::pair< const int, TrianglePtr > & v : M_triangles )
    {
        const TrianglePtr tri = v.second;
        const ContainedType result = classifyPoint( tri, pos );
        if ( result != NOT_CONTAINED )
        {
            *sol = tri;
            return result;
        }
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
class TriangulationMesh::QuadTreeNode {
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
              Rect2D( bounds.topLeft(), c ),
              Rect2D( Vector2D( c.x, bounds.top() ), Vector2D( bounds.right(), c.y ) ),
              Rect2D( Vector2D( bounds.left(), c.y ), Vector2D( c.x, bounds.bottom() ) ),
              Rect2D( c, bounds.bottomRight() ),
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
      'pos' to 'candidates'.
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
TriangulationMesh::buildQuadTree() const
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

    const double pad = 1.0;
    const Rect2D root_bounds( Vector2D( min_x - pad, min_y - pad ),
                              Vector2D( max_x + pad, max_y + pad ) );

    M_quad_tree_root = QuadTreeNode::build( root_bounds, items, 0 );
}

/*-------------------------------------------------------------------*/
void
TriangulationMesh::clearQuadTree() const
{
    delete M_quad_tree_root;
    M_quad_tree_root = nullptr;
}

/*-------------------------------------------------------------------*/
TriangulationMesh::ContainedType
TriangulationMesh::quadTreeFindTriangleContains( const Vector2D & pos,
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

/*-------------------------------------------------------------------*/
/*!
  Point location hot path: walks the triangle adjacency graph starting from
  the triangle found by the previous call (the "hint"), falling back to
  exhaustiveFindTriangleContains() if the walk cannot reach a conclusive
  answer. Result is always identical to the exhaustive search.
*/
TriangulationMesh::ContainedType
TriangulationMesh::walkTriangleContains( const Vector2D & pos,
                                         TrianglePtr start,
                                         TrianglePtr * sol ) const
{
    TrianglePtr tri = start;

    const std::size_t max_steps = M_triangles.size() + 8;

    for ( std::size_t step = 0; tri && step < max_steps; ++step )
    {
        const Vector2D rel0( tri->vertex( 0 )->pos() - pos );
        const Vector2D rel1( tri->vertex( 1 )->pos() - pos );
        const Vector2D rel2( tri->vertex( 2 )->pos() - pos );

        const double outer0 = rel0.outerProduct( rel1 );
        const double outer1 = rel1.outerProduct( rel2 );
        const double outer2 = rel2.outerProduct( rel0 );

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
            return NOT_CONTAINED;
        }

        const Vertex * va = tri->vertex( cross_edge );
        const Vertex * vb = tri->vertex( ( cross_edge + 1 ) % 3 );
        Edge * e = tri->getEdgeInclude( va, vb );

        tri = ( e->triangle( 0 ) == tri ? e->triangle( 1 ) : e->triangle( 0 ) );
    }

    return NOT_CONTAINED;
}

/*-------------------------------------------------------------------*/
TriangulationMesh::ContainedType
TriangulationMesh::findTriangleContainsFast( const Vector2D & pos,
                                             TrianglePtr * sol ) const
{
    if ( M_triangles.empty() )
    {
        return NOT_CONTAINED;
    }

    if ( M_search_method == SearchMethod::QUAD_TREE )
    {
        const ContainedType result = quadTreeFindTriangleContains( pos, sol );
        if ( result == CONTAINED || result == ONLINE )
        {
            return result;
        }
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

}
