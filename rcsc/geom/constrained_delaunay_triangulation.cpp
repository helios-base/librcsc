// -*-c++-*-

/*!
  \file constrained_delaunay_triangulation.cpp
  \brief Constrained Delaunay Triangulation class Source File.
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

#include "constrained_delaunay_triangulation.h"

#include <rcsc/geom/segment_2d.h>
#include <rcsc/geom/triangle_2d.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <limits>
#include <utility>

namespace rcsc {

const double ConstrainedDelaunayTriangulation::EPSILON = 1.0e-10;

namespace {

const std::pair< std::size_t, std::size_t >
edge_pairs[3] = { std::pair< std::size_t, std::size_t >( 0, 1 ),
                  std::pair< std::size_t, std::size_t >( 1, 2 ),
                  std::pair< std::size_t, std::size_t >( 2, 0 ),
};

//! two vertices closer than this squared distance are treated as the same
//! coordinate.
constexpr double VERTEX_MERGE_DIST2 = 1.0e-6;

/*-------------------------------------------------------------------*/
/*!
  \brief check how 'pos' relates to a single triangle (point-in-triangle
  test, with a fast reject using the fact that the circumcircle always
  encloses the triangle).
 */
ConstrainedDelaunayTriangulation::ContainedType
test_triangle_contains( const ConstrainedDelaunayTriangulation::TrianglePtr tri,
                        const Vector2D & pos )
{
    if ( std::fabs( tri->circumcenter().x - pos.x ) > tri->circumradius()
         || std::fabs( tri->circumcenter().y - pos.y ) > tri->circumradius() )
    {
        return ConstrainedDelaunayTriangulation::NOT_CONTAINED;
    }

    const Vector2D rel0( tri->vertex( 0 )->pos() - pos );
    const Vector2D rel1( tri->vertex( 1 )->pos() - pos );
    const Vector2D rel2( tri->vertex( 2 )->pos() - pos );

    const double outer0 = rel0.outerProduct( rel1 );
    const double outer1 = rel1.outerProduct( rel2 );
    const double outer2 = rel2.outerProduct( rel0 );

    if ( std::fabs( outer0 ) <= ConstrainedDelaunayTriangulation::EPSILON )
    {
        if ( rel0.x * rel1.x > ConstrainedDelaunayTriangulation::EPSILON
             || rel0.y * rel1.y > ConstrainedDelaunayTriangulation::EPSILON )
        {
            return ConstrainedDelaunayTriangulation::NOT_CONTAINED;
        }
        return ConstrainedDelaunayTriangulation::ONLINE;
    }

    if ( std::fabs( outer1 ) <= ConstrainedDelaunayTriangulation::EPSILON )
    {
        if ( rel1.x * rel2.x > ConstrainedDelaunayTriangulation::EPSILON
             || rel1.y * rel2.y > ConstrainedDelaunayTriangulation::EPSILON )
        {
            return ConstrainedDelaunayTriangulation::NOT_CONTAINED;
        }
        return ConstrainedDelaunayTriangulation::ONLINE;
    }

    if ( std::fabs( outer2 ) <= ConstrainedDelaunayTriangulation::EPSILON )
    {
        if ( rel2.x * rel0.x > ConstrainedDelaunayTriangulation::EPSILON
             || rel2.y * rel0.y > ConstrainedDelaunayTriangulation::EPSILON )
        {
            return ConstrainedDelaunayTriangulation::NOT_CONTAINED;
        }
        return ConstrainedDelaunayTriangulation::ONLINE;
    }

    if ( ( outer0 >= 0.0 && outer1 >= 0.0 && outer2 >= 0.0 )
         || ( outer0 <= 0.0 && outer1 <= 0.0 && outer2 <= 0.0 ) )
    {
        return ConstrainedDelaunayTriangulation::CONTAINED;
    }

    return ConstrainedDelaunayTriangulation::NOT_CONTAINED;
}

}

/*-------------------------------------------------------------------*/
ConstrainedDelaunayTriangulation::Triangle::Triangle( const int id,
                                                      EdgePtr e0,
                                                      EdgePtr e1,
                                                      EdgePtr e2 )
    : M_id( id )
{
    M_edges[0] = e0;
    M_edges[1] = e1;
    M_edges[2] = e2;

    for ( std::size_t i = 0; i < 3; ++i )
    {
        M_edges[i]->setTriangle( this );
    }

    M_vertices[0] = M_edges[0]->vertex( 0 );
    M_vertices[1] = M_edges[0]->vertex( 1 );
    M_vertices[2] = ( ( M_vertices[0] != M_edges[1]->vertex( 0 )
                        && M_vertices[1] != M_edges[1]->vertex( 0 ) )
                      ? M_edges[1]->vertex( 0 )
                      : M_edges[1]->vertex( 1 ) );

    M_circumcenter = Triangle2D::circumcenter( M_vertices[0]->pos(),
                                               M_vertices[1]->pos(),
                                               M_vertices[2]->pos() );
    M_circumradius = M_circumcenter.dist( M_vertices[0]->pos() );
}

/*-------------------------------------------------------------------*/
ConstrainedDelaunayTriangulation::~ConstrainedDelaunayTriangulation()
{
    clear();
}

/*-------------------------------------------------------------------*/
void
ConstrainedDelaunayTriangulation::clear()
{
    clearResults();
    M_vertices.clear();
    M_constraints.clear();
}

/*-------------------------------------------------------------------*/
void
ConstrainedDelaunayTriangulation::clearResults()
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
}

/*-------------------------------------------------------------------*/
int
ConstrainedDelaunayTriangulation::addVertex( const double x,
                                             const double y )
{
    for ( const Vertex & v : M_vertices )
    {
        if ( v.pos().dist2( Vector2D( x, y ) ) < VERTEX_MERGE_DIST2 )
        {
            return -1;
        }
    }

    const int id = static_cast< int >( M_vertices.size() );
    M_vertices.emplace_back( id, x, y );
    return id;
}

/*-------------------------------------------------------------------*/
void
ConstrainedDelaunayTriangulation::addVertices( const std::vector< Vector2D > & v )
{
    for ( const Vector2D & p : v )
    {
        addVertex( p );
    }
}

/*-------------------------------------------------------------------*/
int
ConstrainedDelaunayTriangulation::findOrCreateVertex( const Vector2D & p )
{
    for ( const Vertex & v : M_vertices )
    {
        if ( v.pos().dist2( p ) < VERTEX_MERGE_DIST2 )
        {
            return v.id();
        }
    }

    const int id = static_cast< int >( M_vertices.size() );
    M_vertices.emplace_back( id, p );
    return id;
}

/*-------------------------------------------------------------------*/
const
ConstrainedDelaunayTriangulation::Vertex *
ConstrainedDelaunayTriangulation::getVertex( const int id ) const
{
    if ( id < 0 || static_cast< std::size_t >( id ) >= M_vertices.size() )
    {
        return nullptr;
    }
    return &M_vertices[id];
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::addConstraint( const int id0,
                                                 const int id1 )
{
    if ( id0 == id1 || ! getVertex( id0 ) || ! getVertex( id1 ) )
    {
        return false;
    }

    M_constraints.emplace_back( id0, id1 );
    return true;
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::addConstraint( const Vector2D & p0,
                                                 const Vector2D & p1 )
{
    if ( p0.dist2( p1 ) < VERTEX_MERGE_DIST2 )
    {
        return false;
    }

    const int id0 = findOrCreateVertex( p0 );
    const int id1 = findOrCreateVertex( p1 );
    return addConstraint( id0, id1 );
}

/*-------------------------------------------------------------------*/
std::size_t
ConstrainedDelaunayTriangulation::addConstraint( const std::vector< Vector2D > & points,
                                                 const bool closed )
{
    if ( points.size() < 2 )
    {
        return 0;
    }

    std::size_t count = 0;

    for ( std::size_t i = 0; i + 1 < points.size(); ++i )
    {
        if ( addConstraint( points[i], points[i + 1] ) )
        {
            ++count;
        }
    }

    if ( closed && points.size() >= 3 )
    {
        if ( addConstraint( points.back(), points.front() ) )
        {
            ++count;
        }
    }

    return count;
}

/*-------------------------------------------------------------------*/
void
ConstrainedDelaunayTriangulation::createInitialTriangle( const Rect2D & region )
{
    clearResults();

    const double max_size = std::max( region.size().length() + 1.0,
                                      region.size().width() + 1.0 );
    const Vector2D center = region.center();
    const double offset = std::max( 1000.0 * max_size, 1000.0 );

    M_initial_vertex[0] = Vertex( -1, center.x + offset, center.y );
    M_initial_vertex[1] = Vertex( -2, center.x, center.y + offset );
    M_initial_vertex[2] = Vertex( -3, center.x - offset, center.y - offset );

    EdgePtr edge0 = createEdge( &M_initial_vertex[0], &M_initial_vertex[1] );
    EdgePtr edge1 = createEdge( &M_initial_vertex[1], &M_initial_vertex[2] );
    EdgePtr edge2 = createEdge( &M_initial_vertex[2], &M_initial_vertex[0] );

    createTriangle( edge0, edge1, edge2 );
}

/*-------------------------------------------------------------------*/
void
ConstrainedDelaunayTriangulation::createInitialTriangle()
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
ConstrainedDelaunayTriangulation::removeInitialVertices()
{
    std::vector< EdgePtr > removed_edges;

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

    for ( EdgePtr edge : removed_edges )
    {
        removeTriangle( edge->triangle( 0 ) );
        removeTriangle( edge->triangle( 1 ) );
        removeEdge( edge->id() );
    }
}

/*-------------------------------------------------------------------*/
const
ConstrainedDelaunayTriangulation::Triangle *
ConstrainedDelaunayTriangulation::findTriangleContains( const Vector2D & pos ) const
{
    TrianglePtr tri = nullptr;
    findTriangleContains( pos, &tri );
    return tri;
}

/*-------------------------------------------------------------------*/
ConstrainedDelaunayTriangulation::ContainedType
ConstrainedDelaunayTriangulation::findTriangleContains( const Vector2D & pos,
                                                        TrianglePtr * sol ) const
{
    for ( const std::pair< const int, TrianglePtr > & v : M_triangles )
    {
        const ContainedType result = test_triangle_contains( v.second, pos );
        if ( result != NOT_CONTAINED )
        {
            *sol = v.second;
            return result;
        }
    }

    return NOT_CONTAINED;
}

/*-------------------------------------------------------------------*/
const
ConstrainedDelaunayTriangulation::Vertex *
ConstrainedDelaunayTriangulation::findNearestVertex( const Vector2D & pos ) const
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
bool
ConstrainedDelaunayTriangulation::isConstrainedEdge( const int id0,
                                                     const int id1 ) const
{
    const Vertex * v0 = getVertex( id0 );
    const Vertex * v1 = getVertex( id1 );
    if ( ! v0 || ! v1 )
    {
        return false;
    }

    for ( const std::pair< const int, EdgePtr > & kv : M_edges )
    {
        if ( kv.second->hasVertices( v0, v1 ) )
        {
            return kv.second->constrained();
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::updateContainedVertex( const Vertex * new_vertex,
                                                         const TrianglePtr tri )
{
    EdgePtr new_edges[3];
    for ( std::size_t i = 0; i < 3; ++i )
    {
        new_edges[i] = createEdge( new_vertex, tri->vertex( i ) );
    }

    EdgePtr old_edges[3];
    TrianglePtr new_tri[3];

    for ( std::size_t i = 0; i < 3; ++i )
    {
        old_edges[i] = tri->getEdgeInclude( new_edges[ edge_pairs[i].first ]->vertex( 1 ),
                                            new_edges[ edge_pairs[i].second ]->vertex( 1 ) );
        old_edges[i]->removeTriangle( tri );
        new_tri[i] = createTriangle( old_edges[i],
                                     new_edges[ edge_pairs[i].first ],
                                     new_edges[ edge_pairs[i].second ] );
        if ( ! new_tri[i]->circumcenter().isValid() )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::updateContainedVertex()"
                      << " detected an illegal (degenerate) triangle."
                      << std::endl;
            return false;
        }
    }

    removeTriangle( tri );

    for ( std::size_t i = 0; i < 3; ++i )
    {
        if ( ! legalizeEdge( old_edges[i] ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::updateOnlineVertex( const Vertex * new_vertex,
                                                      const TrianglePtr tri )
{
    int online_count = 0;
    EdgePtr online_edge = nullptr;
    for ( std::size_t i = 0; i < 3; ++i )
    {
        const Vector2D rel0( tri->edge( i )->vertex( 0 )->pos() - new_vertex->pos() );
        const Vector2D rel1( tri->edge( i )->vertex( 1 )->pos() - new_vertex->pos() );
        if ( std::fabs( rel0.outerProduct( rel1 ) ) <= EPSILON )
        {
            online_edge = tri->edge( i );
            ++online_count;
        }
    }

    if ( online_count >= 2 || ! online_edge )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ConstrainedDelaunayTriangulation::updateOnlineVertex()"
                  << " failed to find a single online edge."
                  << std::endl;
        return false;
    }

    if ( online_edge->constrained() )
    {
        // a newly inserted site landed exactly on an already constrained
        // edge. this can only happen for pathological (near-)duplicate
        // input coordinates since constraints are only recovered after all
        // sites have been inserted; refuse rather than silently splitting a
        // constraint edge.
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ConstrainedDelaunayTriangulation::updateOnlineVertex()"
                  << " new vertex lies on a constrained edge."
                  << std::endl;
        return false;
    }

    EdgePtr new_edge[2];
    for ( std::size_t i = 0; i < 2; ++i )
    {
        new_edge[i] = createEdge( new_vertex, online_edge->vertex( i ) );
    }

    TrianglePtr new_tri_in_tri[2];
    EdgePtr old_edge_in_tri[2];
    {
        const Vertex * tri_vertex = tri->getVertexExclude( online_edge );
        if ( ! tri_vertex )
        {
            return false;
        }

        EdgePtr new_edge_in_tri = createEdge( new_vertex, tri_vertex );

        for ( std::size_t i = 0; i < 2; ++i )
        {
            old_edge_in_tri[i] = tri->getEdgeInclude( new_edge[i]->vertex( 1 ), tri_vertex );
            old_edge_in_tri[i]->removeTriangle( tri );
            new_tri_in_tri[i] = createTriangle( old_edge_in_tri[i], new_edge[i], new_edge_in_tri );
            if ( ! new_tri_in_tri[i]->circumcenter().isValid() )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ConstrainedDelaunayTriangulation::updateOnlineVertex()"
                          << " detected an illegal (degenerate) triangle."
                          << std::endl;
                return false;
            }
        }
    }

    Triangle * adjacent = ( online_edge->triangle( 0 ) == tri
                            ? online_edge->triangle( 1 )
                            : online_edge->triangle( 0 ) );
    const bool exist_adjacent = ( adjacent != nullptr );

    TrianglePtr new_tri_in_adjacent[2];
    EdgePtr old_edge_in_adjacent[2];

    if ( exist_adjacent )
    {
        const Vertex * adjacent_vertex = adjacent->getVertexExclude( online_edge );
        if ( ! adjacent_vertex )
        {
            return false;
        }

        EdgePtr new_edge_in_adjacent = createEdge( new_vertex, adjacent_vertex );

        for ( std::size_t i = 0; i < 2; ++i )
        {
            old_edge_in_adjacent[i] = adjacent->getEdgeInclude( new_edge[i]->vertex( 1 ), adjacent_vertex );
            old_edge_in_adjacent[i]->removeTriangle( adjacent );
            new_tri_in_adjacent[i] = createTriangle( old_edge_in_adjacent[i], new_edge[i], new_edge_in_adjacent );
            if ( ! new_tri_in_adjacent[i]->circumcenter().isValid() )
            {
                std::cerr << __FILE__ << ':' << __LINE__
                          << " ConstrainedDelaunayTriangulation::updateOnlineVertex()"
                          << " detected an illegal (degenerate) triangle."
                          << std::endl;
                return false;
            }
        }
    }

    removeTriangle( tri );
    if ( exist_adjacent )
    {
        removeTriangle( adjacent );
    }
    removeEdge( online_edge );

    for ( std::size_t i = 0; i < 2; ++i )
    {
        if ( ! legalizeEdge( old_edge_in_tri[i] ) )
        {
            return false;
        }
    }

    if ( exist_adjacent )
    {
        for ( std::size_t i = 0; i < 2; ++i )
        {
            if ( ! legalizeEdge( old_edge_in_adjacent[i] ) )
            {
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::flipEdge( EdgePtr edge,
                                            EdgePtr outer_edges[4],
                                            EdgePtr * new_edge )
{
    TrianglePtr t0 = edge->triangle( 0 );
    TrianglePtr t1 = edge->triangle( 1 );

    const Vertex * v0 = t0->getVertexExclude( edge );
    const Vertex * v1 = t1->getVertexExclude( edge );

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
                  << " ConstrainedDelaunayTriangulation::flipEdge()"
                  << " detected an illegal (degenerate) triangle after flip."
                  << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::isLocallyDelaunay( const EdgePtr edge ) const
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
ConstrainedDelaunayTriangulation::legalizeEdge( EdgePtr edge )
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

    // a flip is only geometrically valid if the quadrilateral formed by the
    // two triangles is convex, i.e. the diagonal v0-v1 properly crosses the
    // shared edge.
    const Segment2D diagonal( v0->pos(), v1->pos() );
    const Segment2D shared( edge->vertex( 0 )->pos(), edge->vertex( 1 )->pos() );
    if ( ! diagonal.existIntersectionExceptEndpoint( shared ) )
    {
        return true;
    }

    EdgePtr outer_edges[4];
    EdgePtr new_edge = nullptr;
    if ( ! flipEdge( edge, outer_edges, &new_edge ) )
    {
        return false;
    }

    for ( int i = 0; i < 4; ++i )
    {
        if ( ! legalizeEdge( outer_edges[i] ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ConstrainedDelaunayTriangulation::legalizeAll()
{
    // one sweep is not enough in general: legalizeEdge()'s flip of an edge
    // found late in a sweep can re-illegalize an edge that was already
    // visited (and found legal) earlier in that same sweep, since a sweep
    // only takes a snapshot of edge ids up front. So sweep repeatedly until
    // a full pass finds nothing left to fix.
    const std::size_t max_sweeps = M_edges.size() + 64;

    for ( std::size_t sweep = 0; sweep < max_sweeps; ++sweep )
    {
        std::vector< int > ids;
        ids.reserve( M_edges.size() );
        for ( const std::pair< const int, EdgePtr > & v : M_edges )
        {
            ids.push_back( v.first );
        }

        bool any_illegal = false;

        for ( const int id : ids )
        {
            EdgeCont::const_iterator it = M_edges.find( id );
            if ( it == M_edges.end() )
            {
                // already removed by an earlier flip performed in this sweep.
                continue;
            }

            const EdgePtr edge = it->second;
            if ( ! isLocallyDelaunay( edge ) )
            {
                any_illegal = true;
            }
            if ( ! legalizeEdge( edge ) )
            {
                return false;
            }
        }

        if ( ! any_illegal )
        {
            return true;
        }
    }

    std::cerr << __FILE__ << ':' << __LINE__
              << " ConstrainedDelaunayTriangulation::legalizeAll()"
              << " exceeded the sweep budget."
              << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
/*!
  Segment recovery by edge flipping (Sloan 1992 / de Berg et al.): repeatedly
  flip the edges that cross the constraint segment until the segment itself
  appears as an edge, then restore local Delaunay-ness (skipping constrained
  edges) via legalizeAll().
*/
bool
ConstrainedDelaunayTriangulation::insertConstraint( const int id0,
                                                    const int id1 )
{
    const Vertex * v0 = getVertex( id0 );
    const Vertex * v1 = getVertex( id1 );

    if ( ! v0 || ! v1 || v0 == v1 )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ConstrainedDelaunayTriangulation::insertConstraint()"
                  << " invalid vertex id pair (" << id0 << ',' << id1 << ')'
                  << std::endl;
        return false;
    }

    for ( const std::pair< const int, EdgePtr > & kv : M_edges )
    {
        if ( kv.second->hasVertices( v0, v1 ) )
        {
            kv.second->setConstrained( true );
            return true;
        }
    }

    const Segment2D constraint_seg( v0->pos(), v1->pos() );

    std::deque< EdgePtr > queue;
    for ( const std::pair< const int, EdgePtr > & kv : M_edges )
    {
        EdgePtr e = kv.second;
        if ( e->hasVertex( v0 ) || e->hasVertex( v1 ) )
        {
            continue;
        }
        const Segment2D es( e->vertex( 0 )->pos(), e->vertex( 1 )->pos() );
        if ( constraint_seg.existIntersectionExceptEndpoint( es ) )
        {
            queue.push_back( e );
        }
    }

    if ( queue.empty() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ConstrainedDelaunayTriangulation::insertConstraint()"
                  << " could not locate any edge crossing constraint ("
                  << id0 << ',' << id1 << "). skipped."
                  << std::endl;
        return false;
    }

    const std::size_t max_iterations = queue.size() * queue.size() + 64;
    std::size_t iterations = 0;

    // 'ok' tracks whether the constraint itself was fully honored. Even when
    // it is not (e.g. it crosses an already-placed constraint, or a budget
    // is exceeded), flips already performed before the failure was detected
    // may have left the mesh with locally-illegal edges elsewhere -- so
    // legalizeAll() below always runs before returning, regardless of 'ok'.
    bool ok = true;

    while ( ! queue.empty() )
    {
        if ( ++iterations > max_iterations )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::insertConstraint()"
                      << " exceeded the flip iteration budget for constraint ("
                      << id0 << ',' << id1 << "). skipped."
                      << std::endl;
            ok = false;
            break;
        }

        EdgePtr e = queue.front();
        queue.pop_front();

        if ( e->constrained() )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::insertConstraint()"
                      << " constraint (" << id0 << ',' << id1 << ')'
                      << " crosses an already placed constraint edge. skipped."
                      << std::endl;
            ok = false;
            break;
        }

        TrianglePtr t0 = e->triangle( 0 );
        TrianglePtr t1 = e->triangle( 1 );
        if ( ! t0 || ! t1 )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::insertConstraint()"
                      << " a boundary edge was unexpectedly marked as crossing constraint ("
                      << id0 << ',' << id1 << "). skipped."
                      << std::endl;
            ok = false;
            break;
        }

        const Vertex * c = t0->getVertexExclude( e );
        const Vertex * d = t1->getVertexExclude( e );
        if ( ! c || ! d )
        {
            ok = false;
            break;
        }

        const Segment2D diagonal( c->pos(), d->pos() );
        const Segment2D shared( e->vertex( 0 )->pos(), e->vertex( 1 )->pos() );
        if ( ! diagonal.existIntersectionExceptEndpoint( shared ) )
        {
            // not (yet) convex: retry once other flips reshape this region.
            queue.push_back( e );
            continue;
        }

        EdgePtr outer_edges[4];
        EdgePtr new_edge = nullptr;
        if ( ! flipEdge( e, outer_edges, &new_edge ) )
        {
            ok = false;
            break;
        }

        if ( new_edge->hasVertices( v0, v1 ) )
        {
            new_edge->setConstrained( true );
            continue;
        }

        const Segment2D new_seg( new_edge->vertex( 0 )->pos(), new_edge->vertex( 1 )->pos() );
        if ( constraint_seg.existIntersectionExceptEndpoint( new_seg ) )
        {
            queue.push_back( new_edge );
        }
    }

    if ( ! legalizeAll() )
    {
        return false;
    }
    return ok;
}

/*-------------------------------------------------------------------*/
void
ConstrainedDelaunayTriangulation::compute()
{
    if ( M_vertices.size() < 3 )
    {
        removeInitialVertices();
        return;
    }

    if ( M_triangles.empty() || M_triangles.size() > 3 )
    {
        createInitialTriangle();
    }

    for ( Vertex & v : M_vertices )
    {
        TrianglePtr tri = nullptr;
        const ContainedType type = findTriangleContains( v.pos(), &tri );

        if ( ! tri || type == NOT_CONTAINED )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::compute()"
                      << " could not locate the triangle containing vertex "
                      << v.pos()
                      << std::endl;
            clearResults();
            return;
        }

        const bool ok = ( type == CONTAINED
                          ? updateContainedVertex( &v, tri )
                          : updateOnlineVertex( &v, tri ) );
        if ( ! ok )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::compute()"
                      << " failed to insert vertex " << v.pos()
                      << std::endl;
            clearResults();
            return;
        }
    }

    removeInitialVertices();

    for ( const ConstraintSegment & c : M_constraints )
    {
        if ( ! insertConstraint( c.first, c.second ) )
        {
            std::cerr << __FILE__ << ':' << __LINE__
                      << " ConstrainedDelaunayTriangulation::compute()"
                      << " could not honor constraint ("
                      << c.first << ',' << c.second << ')'
                      << std::endl;
            // non-fatal: keep the rest of the (still valid) triangulation
            // and continue with the remaining constraints.
        }
    }
}

}
