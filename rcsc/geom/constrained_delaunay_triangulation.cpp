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

#include <deque>
#include <iostream>

namespace rcsc {

namespace {

//! must match the vertex-merge tolerance used by TriangulationMesh::addVertex().
constexpr double VERTEX_MERGE_DIST2 = 1.0e-6;

}

/*-------------------------------------------------------------------*/
ConstrainedDelaunayTriangulation::~ConstrainedDelaunayTriangulation()
{
    clear();
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
        clearResults();
        createInitialTriangle();
    }

    for ( Vertex & v : M_vertices )
    {
        TrianglePtr tri = nullptr;
        const ContainedType type = exhaustiveFindTriangleContains( v.pos(), &tri );

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
