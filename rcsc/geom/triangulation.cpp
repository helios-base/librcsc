// -*-c++-*-

/*!
  \file triangulation.cpp
  \brief 2D triangulation Source File.
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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#define VOID int
#define REAL double

#include "triangulation.h"

#include "triangle/triangle.h"

#include <vector>
#include <limits>
#include <cstddef>
#include <cstdlib>
#include <cstring>

extern "C" {

void triangulate( char *,
                  struct triangulateio *,
                  struct triangulateio *,
                  struct triangulateio * );
}

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
void
Triangulation::clear()
{
    clearResults();

    M_points.clear();
    M_constraints.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Triangulation::clearResults()
{
    M_triangles.clear();
    // M_result_segments.clear();
    M_edges.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Triangulation::addPoint( const Vector2D & p )
{
#ifdef TRIANGULATION_STRICT_POINT_SET
    if ( M_point_set.find( p ) != M_point_set.end() )
    {
        return false;
    }

    M_point_set.insert( p );
#endif

    M_points.push_back( p );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
size_t
Triangulation::addPoints( const PointCont & v )
{
#ifdef TRIANGULATION_STRICT_POINT_SET
    if ( M_points.size() < v.size() )
    {
        M_points.reserve( v.size() );
    }

    size_t size = 0;

    for ( PointCont::const_iterator p = v.begin(), end = v.end();
          p != end;
          ++p )
    {
        if ( ! addPoint( *p ) )
        {
            break;
        }

        ++size;
    }

    return size;
#else

    M_points.insert( M_points.end(), v.begin(), v.end() );
    return v.size();
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Triangulation::addConstraint( const size_t & origin_index,
                              const size_t & terminal_index )
{
    if ( origin_index == terminal_index
         || M_points.size() <= origin_index
         || M_points.size() <= terminal_index )
    {
        return false;
    }

    std::pair< SegmentSet::iterator, bool > result
        = M_constraints.insert( Segment( std::min( origin_index, terminal_index ),
                                         std::max( origin_index, terminal_index ) ) );

    return result.second;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
Triangulation::compute()
{
    M_triangles.clear();
    M_edges.clear();

    const PointCont & points = M_points;
    const size_t points_size = points.size();

    const SegmentSet & constraints = M_constraints;
    const size_t constraints_size = constraints.size();

    //
    // check enough points exist or not
    //
    if ( points_size < 3 )
    {
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
    in.numberofpoints = points_size;
    in.pointlist = static_cast< REAL * >( std::malloc( in.numberofpoints * 2 * sizeof( REAL ) ) );

    for ( size_t i = 0; i < points_size; ++i )
    {
        in.pointlist[i * 2    ] = static_cast< REAL >( points[i].x );
        in.pointlist[i * 2 + 1] = static_cast< REAL >( points[i].y );
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
    // set input segments (constraints)
    //
    in.numberofsegments = constraints_size;
    if ( constraints_size > 0 )
    {
        in.segmentlist = static_cast< int * >( std::malloc( in.numberofsegments * 2 * sizeof( int ) ) );

        size_t i = 0;
        for ( SegmentSet::const_iterator c = constraints.begin(), end = constraints.end();
              c != end;
              ++c, ++i )
        {
            in.segmentlist[i * 2]     = static_cast< int >( c->first );
            in.segmentlist[i * 2 + 1] = static_cast< int >( c->second );

            //std::cerr << "set constraints " << c->first << ' ' << c->second << std::endl;
        }
    }


    //
    // set input holes, regions
    //
    in.numberofholes = 0;
    in.numberofregions = 0;
    in.regionlist = nullptr;


    //
    // initialize output buffers
    //
    struct triangulateio out;
    std::memset( &out, 0, sizeof( out ) );


    //
    // create triangulation
    //
    // z: start index from zero
    // B: no boundary marker output
    // N: no node outpu
    // P: no constraints output
    // Q: don't print debug information
    // p: PSLG(Planar Straight Line Graph) mode, generate constrained Delaunay triangulation
    // c: creates segments on the convec hull
    // E: no triangle output
    // e: edges output
    char opt[32];
    std::strcpy( opt, "zBNPQ" );
    if ( constraints_size > 0 ) std::strcat( opt, "pc" );
    if ( ! M_use_triangles ) std::strcat( opt, "E" );
    if ( M_use_edges ) std::strcat( opt, "e" );

    triangulate( opt, &in, &out, NULL );


    //
    // set result triangles
    //
    if ( M_use_triangles )
    {
        const int number_of_triangles = out.numberoftriangles;
        M_triangles.reserve( number_of_triangles );

        for ( int i = 0; i < number_of_triangles; ++i )
        {
            M_triangles.emplace_back( static_cast< size_t >( out.trianglelist[i * 3] ),
                                      static_cast< size_t >( out.trianglelist[i * 3 + 1] ),
                                      static_cast< size_t >( out.trianglelist[i * 3 + 2] ) );
        }
    }

//     if ( constraints_size > 0 )
//     {
//         const int number_of_segments = out.numberofsegments;
//         M_result_segments.reserve( number_of_segments );

//         for ( int i = 0; i < number_of_segments; ++i )
//         {
//             M_result_segments.emplace_back( static_cast< size_t >( out.segmentlist[i * 2] ),
//                                             static_cast< size_t >( out.segmentlist[i * 2 + 1] ) );
//         }
//     }

    //
    // set result edges
    //
    if ( M_use_edges )
    {
        const int number_of_edges = out.numberofedges;
        M_edges.reserve( number_of_edges );

        for ( int i = 0; i < number_of_edges; ++i )
        {
            M_edges.emplace_back( static_cast< size_t >( out.edgelist[i * 2] ),
                                  static_cast< size_t >( out.edgelist[i * 2 + 1] ) );
        }
    }


    //
    // finalize
    //
    std::free( in.pointlist );
    if ( constraints_size > 0 )
    {
        std::free( in.segmentlist );
    }

    //std::free( out.pointlist );
    if ( M_use_triangles )
    {
        std::free( out.trianglelist );
    }
    if ( constraints_size > 0 )
    {
        std::free( out.segmentlist );
    }
    if ( M_use_edges )
    {
        std::free( out.edgelist );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
const
Triangulation::Triangle *
Triangulation::findTriangleContains( const Vector2D & point ) const
{
    const PointCont & points = M_points;

    for ( TriangleCont::const_iterator t = M_triangles.begin(), end = M_triangles.end();
          t != end;
          ++t )
    {
        Vector2D rel1( points[t->v0_] - point );
        Vector2D rel2( points[t->v1_] - point );
        Vector2D rel3( points[t->v2_] - point );

        double outer1 = rel1.outerProduct( rel2 );
        double outer2 = rel2.outerProduct( rel3 );
        double outer3 = rel3.outerProduct( rel1 );

        if ( ( outer1 >= -1.0e-9 && outer2 >= -1.0e-9 && outer3 >= -1.0e-9 )
             || ( outer1 <= 1.0e-9 && outer2 <= 1.0e-9 && outer3 <= 1.0e-9 ) )
        {
            return &(*t);
        }
    }

    return nullptr;
}


/*-------------------------------------------------------------------*/
/*!

*/
int
Triangulation::findNearestPoint( const Vector2D & point ) const
{
    int index = -1;
    double min_dist2 = std::numeric_limits< double >::max();

    int i = 0;
    for ( PointCont::const_iterator p = M_points.begin(), end = M_points.end();
          p != end;
          ++p, ++i )
    {
        double d2 = p->dist2( point );
        if ( d2 < min_dist2 )
        {
            min_dist2 = d2;
            index = i;
        }
    }

    return index;
}

}
