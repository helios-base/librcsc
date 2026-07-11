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

#include <cstdint>
#include <iostream>

//#define DEBUG

namespace rcsc {

namespace {

inline std::uint32_t clampToGrid16( const double value,
                                    const double min_value,
                                    const double max_value )
{
    if ( max_value <= min_value )
    {
        return 0U;
    }

    const double normalized = ( value - min_value ) / ( max_value - min_value );
    const double scaled = std::max( 0.0,
                                    std::min( 65535.0,
                                              normalized * 65535.0 ) );
    return static_cast< std::uint32_t >( scaled );
}

inline std::uint32_t hilbertKey( std::uint32_t x,
                                 std::uint32_t y )
{
    std::uint32_t index = 0U;

    for ( std::uint32_t bit = 1U << 15; bit > 0U; bit >>= 1 )
    {
        const std::uint32_t rx = ( x & bit ) ? 1U : 0U;
        const std::uint32_t ry = ( y & bit ) ? 1U : 0U;

        index += bit * bit * ( ( 3U * rx ) ^ ry );

        if ( ry == 0U )
        {
            if ( rx == 1U )
            {
                x = static_cast< std::uint32_t >( ~x ) & 0xFFFFU;
                y = static_cast< std::uint32_t >( ~y ) & 0xFFFFU;
            }

            std::swap( x, y );
        }
    }

    return index;
}

} // unnamed namespace

/*-------------------------------------------------------------------*/
DelaunayTriangulation::~DelaunayTriangulation()
{
    clear();
}

/*-------------------------------------------------------------------*/
void
DelaunayTriangulation::clearResults()
{
    DelaunayTriangulationCore::clearResults();
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
    }

    Vector2D min_pos( M_vertices.front().pos() );
    Vector2D max_pos( M_vertices.front().pos() );
    for ( const Vertex & v : M_vertices )
    {
        if ( v.pos().x < min_pos.x ) min_pos.x = v.pos().x;
        if ( max_pos.x < v.pos().x ) max_pos.x = v.pos().x;
        if ( v.pos().y < min_pos.y ) min_pos.y = v.pos().y;
        if ( max_pos.y < v.pos().y ) max_pos.y = v.pos().y;
    }

    if ( M_triangles.empty()
         || M_triangles.size() > 3 )
    {
        createInitialTriangle( Rect2D( Vector2D( min_pos.x - 1.0, min_pos.y - 1.0 ),
                                       Vector2D( max_pos.x + 1.0, max_pos.y + 1.0 ) ) );
    }

    // sort vertices by hilbert curve order
    // this is to improve locality of reference and cache performance
    // https://en.wikipedia.org/wiki/Hilbert_curve
    // https://en.wikipedia.org/wiki/Locality_of_reference

    std::vector< std::pair< std::uint32_t, std::size_t > > order;
    order.reserve( M_vertices.size() );
    for ( std::size_t i = 0; i < M_vertices.size(); ++i )
    {
        const Vector2D & p = M_vertices[i].pos();
        const std::uint32_t qx = clampToGrid16( p.x, min_pos.x, max_pos.x );
        const std::uint32_t qy = clampToGrid16( p.y, min_pos.y, max_pos.y );
        order.emplace_back( hilbertKey( qx, qy ), i );
    }

    std::sort( order.begin(), order.end(),
               []( const std::pair< std::uint32_t, std::size_t > & lhs,
                   const std::pair< std::uint32_t, std::size_t > & rhs ) {
                   if ( lhs.first != rhs.first )
                   {
                       return lhs.first < rhs.first;
                   }
                   return lhs.second < rhs.second;
               } );

    int loop = 0;
    for ( const std::pair< std::uint32_t, std::size_t > & item : order )
    {
        ++loop;
        Vertex & v = M_vertices[item.second];

        // find triangle that contains 'vertex'
        TrianglePtr tri = nullptr;
        ContainedType type = findTriangleContainsFast( v.pos(), &tri );

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
    TrianglePtr tri = nullptr;
    findTriangleContainsFast( pos, &tri );
    return tri;
}

}
