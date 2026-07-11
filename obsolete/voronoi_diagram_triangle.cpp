// -*-c++-*-

/*!
  \file voronoi_diagram_triangle.cpp
  \brief 2D voronoi diagram Source File.
*/

/*
 *Copyright:

 Copyright (C) Hiroki Shimora

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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#define VOID int
#define REAL double

#include "voronoi_diagram_triangle.h"

#include "triangle/triangle.h"

#include <vector>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <algorithm>

extern "C" {

    void triangulate( char *, struct triangulateio *,
                      struct triangulateio *, struct triangulateio * );
}

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
VoronoiDiagramTriangle::VoronoiDiagramTriangle()
    : M_bounding_rect( nullptr ),
      M_input_points(),
      M_vertices(),
      M_segments(),
      M_rays()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
VoronoiDiagramTriangle::VoronoiDiagramTriangle( const std::vector< Vector2D > & v )
    : M_bounding_rect( nullptr ),
      M_input_points( v ),
      M_vertices(),
      M_segments(),
      M_rays()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
VoronoiDiagramTriangle::~VoronoiDiagramTriangle()
{
    clearBoundingRect();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagramTriangle::setBoundingRect( const Rect2D & rect )
{
    clearBoundingRect();
    M_bounding_rect = new Rect2D( rect );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagramTriangle::clear()
{
    clearBoundingRect();
    clearResults();
    M_input_points.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagramTriangle::clearBoundingRect()
{
    if ( M_bounding_rect )
    {
        delete M_bounding_rect;
        M_bounding_rect = nullptr;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagramTriangle::clearResults()
{
    M_vertices.clear();
    M_segments.clear();
    M_rays.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagramTriangle::compute()
{
    const size_t input_points_size = M_input_points.size();

    //
    // check enough points exist or not
    //
    if ( input_points_size < 3 )
    {
        clearResults();
        return;
    }

    //
    // make input data
    //
    struct triangulateio in;
    std::memset( &in, 0, sizeof( in ) );

    //
    // set point list
    //
    in.numberofpoints = input_points_size;
    in.pointlist = static_cast< REAL * >( std::malloc( in.numberofpoints * 2 * sizeof( REAL ) ) );

    for ( size_t i = 0; i < input_points_size; ++i )
    {
        in.pointlist[i * 2    ] = static_cast< REAL >( M_input_points[i].x );
        in.pointlist[i * 2 + 1] = static_cast< REAL >( M_input_points[i].y );
    }

    //
    // set attribute
    //
    in.numberofpointattributes = 0;
    in.pointattributelist = nullptr;


    //
    // set marker
    //
    in.pointmarkerlist = nullptr;


    //
    // set input segments, holes, regions
    //
    in.numberofsegments = 0;
    in.numberofholes = 0;
    in.numberofregions = 0;
    in.regionlist = nullptr;


    //
    // initialize output buffers
    //
    struct triangulateio mid;
    struct triangulateio out;
    std::memset( &mid, 0, sizeof( mid ) );
    std::memset( &out, 0, sizeof( out ) );


    //
    // create voronoi diagram
    //
    // v: create voronoi diagram
    // B: no boundary marker output
    // E: no triangle output
    // N: no point output
    // P: no constraint output
    // Q: don't print debug information
    triangulate( const_cast< char * >( "vBENPQ" ), &in, &mid, &out );


    if ( M_bounding_rect )
    {
        const Rect2D rect = *M_bounding_rect;

        //
        // set result points
        //
        const int number_of_points = out.numberofpoints;
        for ( int i = 0; i < number_of_points; ++i )
        {
            Vector2D p( out.pointlist[ i * 2 ],
                        out.pointlist[ i * 2 + 1 ] );
            if ( rect.contains( p ) )
            {
                M_vertices.insert( p );
            }
        }

        //
        // set result segments
        //
        const int number_of_edges = out.numberofedges;
        M_segments.reserve( number_of_edges );

        for ( int i = 0; i < number_of_edges; ++i )
        {
            const int start_point_index = out.edgelist[ i * 2 ] - 1;
            const int end_point_index = out.edgelist[ i * 2 + 1 ] - 1;

            if ( start_point_index >= 0 && end_point_index >= 0 )
            {
                const Vector2D p0( out.pointlist[ start_point_index * 2 ],
                                   out.pointlist[ start_point_index * 2 + 1 ] );
                const Vector2D p1( out.pointlist[ end_point_index * 2 ],
                                   out.pointlist[ end_point_index * 2 + 1 ] );

                if ( p0.equalsWeakly( p1 ) )
                {
                    continue;
                }

                Segment2D segment( p0, p1 );
                Vector2D intersect0, intersect1;
                const int n = rect.intersection( segment, &intersect0, &intersect1 );

                if ( n == 0 )
                {
                    if ( rect.contains( segment.origin() ) )
                    {
                        M_segments.push_back( segment );
                    }
                }
                else if ( n == 1 )
                {
                    if ( rect.contains( segment.origin() ) )
                    {
                        M_vertices.insert( intersect0 );
                        M_segments.emplace_back( segment.origin(), intersect0 );
                    }
                    else if ( rect.contains( segment.terminal() ) )
                    {
                        M_vertices.insert( intersect0 );
                        M_segments.emplace_back( segment.terminal(), intersect0 );
                    }
                    else
                    {
                        //std::cerr << __FILE__ << ':' << __LINE__ << ':'
                        //          << "Unexpected reach." << std::endl;
                        continue;
                    }
                }
                else if ( n == 2 )
                {
                    M_vertices.insert( intersect0 );
                    M_vertices.insert( intersect1 );
                    M_segments.emplace_back( intersect0, intersect1 );
                }
            }
            else
            {
                if ( start_point_index < 0 )
                {
                    std::cerr << __FILE__ << ':' << __LINE__ << ':'
                              << "start_point_index has to be a positive value. "
                              << start_point_index
                              << std::endl;
                    continue;
                }

                Vector2D origin( out.pointlist[ start_point_index * 2 ],
                                 out.pointlist[ start_point_index * 2 + 1] );

                if ( ! rect.contains( origin ) )
                {
                    continue;
                }

                Ray2D ray( origin,
                           AngleDeg::atan2_deg( out.normlist[ i * 2 + 1],
                                                out.normlist[ i * 2 ] ) );

                Vector2D terminal;
                if ( rect.intersection( ray, &terminal, nullptr ) != 1 )
                {
                    std::cerr << __FILE__ << ':' << __LINE__ << ':'
                              << "Unexpected reach." << std::endl;
                    continue;
                }

                M_vertices.insert( terminal );
                M_segments.emplace_back( origin, terminal );
            }
        }
    }
    else
    {
        //
        // set result points
        //
        const int number_of_points = out.numberofpoints;
        for ( int i = 0; i < number_of_points; ++i )
        {
            M_vertices.insert( Vector2D( out.pointlist[ i * 2 ],
                                              out.pointlist[ i * 2 + 1 ] ) );
        }

        //
        // set result segments
        //
        const int number_of_edges = out.numberofedges;
        M_segments.reserve( number_of_edges );

        for ( int i = 0; i < number_of_edges; ++i )
        {
            const int start_point_index = out.edgelist[ i * 2 ] - 1;
            const int end_point_index = out.edgelist[ i * 2 + 1 ] - 1;

            if ( start_point_index >= 0 && end_point_index >= 0 )
            {
                const Vector2D p0( out.pointlist[ start_point_index * 2 ],
                                   out.pointlist[ start_point_index * 2 + 1 ] );
                const Vector2D p1( out.pointlist[ end_point_index * 2 ],
                                   out.pointlist[ end_point_index * 2 + 1 ] );

                if ( ! p0.equalsWeakly( p1 ) )
                {
                    M_segments.emplace_back( p0, p1 );
                }
            }
            else
            {
                //assert( start_point_index >= 0 );
                if ( start_point_index < 0 )
                {
                    std::cerr << __FILE__ << ':' << __LINE__ << ':'
                              << "start_point_index have to be positive value. "
                              << start_point_index
                              << std::endl;
                    continue;
                }

                // ray
                M_rays.emplace_back( Vector2D( out.pointlist[ start_point_index * 2 ],
                                               out.pointlist[ start_point_index * 2 + 1] ),
                                     AngleDeg::atan2_deg( out.normlist[ i * 2 + 1],
                                                          out.normlist[ i * 2 ] ) );
            }
        }
    }

    //
    // finalize
    //
    std::free( in.pointlist );
    std::free( mid.pointlist );
    std::free( mid.pointmarkerlist );
    std::free( mid.trianglelist );
    std::free( out.pointlist );
    std::free( out.pointattributelist );
    std::free( out.edgelist );
    std::free( out.normlist );
    std::free( out.trianglelist );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagramTriangle::getPointsOnSegments( const double min_length,
                                             const unsigned int max_division,
                                             std::vector< Vector2D > * result ) const
{
    //
    // add voronoi vertices
    //
    result->insert( result->end(),
                    M_vertices.begin(),
                    M_vertices.end() );

    //
    // add points on segment
    //
    for ( Segment2DCont::const_iterator it = M_segments.begin(),
              end = M_segments.end();
          it != end;
          ++it )
    {
        const double len = it->length();
        if ( len < min_length )
        {
            continue;
        }

        const int div = std::min( max_division,
                                  static_cast< unsigned int >( len / min_length ) );

        for ( int d = 1; d < div; ++d )
        {
            result->push_back( it->origin() * ( static_cast< double >( d ) / div )
                               + it->terminal() * ( static_cast< double >( div - d ) / div ) );
        }
    }
}

}
