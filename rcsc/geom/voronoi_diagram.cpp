// -*-c++-*-

/*!
  \file voronoi_diagram.cpp
  \brief 2D voronoi Source File.
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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include "voronoi_diagram.h"

#include "triangle_2d.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
VoronoiDiagram::VoronoiDiagram()
    : M_bounding_rect( static_cast< Rect2D * >( 0 ) ),
      M_vertices(),
      M_segments(),
      M_rays()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
VoronoiDiagram::VoronoiDiagram( const std::vector< Vector2D > & v )
    : M_bounding_rect( static_cast< Rect2D * >( 0 ) ),
      M_vertices(),
      M_segments(),
      M_rays()
{
    M_triangulation.addVertices( v );
}

/*-------------------------------------------------------------------*/
/*!

 */
VoronoiDiagram::~VoronoiDiagram()
{
    clearBoundingRect();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagram::setBoundingRect( const Rect2D & rect )
{
    clearBoundingRect();
    M_bounding_rect = new Rect2D( rect );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagram::clear()
{
    clearBoundingRect();
    clearResults();
    M_triangulation.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagram::clearBoundingRect()
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
VoronoiDiagram::clearResults()
{
    M_vertices.clear();
    M_segments.clear();
    M_rays.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagram::compute()
{
    const size_t input_points_size = M_triangulation.vertices().size();

    //
    // check enough points exist or not
    //
    if ( input_points_size < 3 )
    {
        clearResults();
        return;
    }

    // #if 1
    //     std::cerr << "VoronoiDiagram input points:\n";
    //     int count = 0;
    //     for ( DelaunayTriangulation::VertexCont::const_iterator v = M_triangulation.vertices().begin();
    //           v != M_triangulation.vertices().end();
    //           ++v, ++count )
    //     {
    //         std::cerr << count << ": " << v->pos() << '\n';
    //     }
    //     std::cerr << std::endl;
    // #endif

    Vector2D centroid( 0.0, 0.0 );
    for ( DelaunayTriangulation::VertexCont::const_iterator v = M_triangulation.vertices().begin(),
              end = M_triangulation.vertices().end();
          v != end;
          ++v )
    {
        centroid += v->pos();
    }
    centroid /= static_cast< double >( input_points_size );


    M_triangulation.compute();
    M_triangulation.updateVoronoiVertex();

    // std::cerr << __FILE__" compute() triangulation \n"
    //           << "  vertex size = " << M_triangulation.vertices().size() << '\n'
    //           << "  edge size = " << M_triangulation.edges().size() << '\n'
    //           << "  triangle size = " << M_triangulation.triangles().size() << '\n'
    //           << std::endl;

    for ( DelaunayTriangulation::EdgeCont::const_iterator e = M_triangulation.edges().begin(),
              end = M_triangulation.edges().end();
          e != end;
          ++e )
    {
        const DelaunayTriangulation::Triangle * t0 = e->second->triangle( 0 );
        const DelaunayTriangulation::Triangle * t1 = e->second->triangle( 1 );

        if ( t0 && t1 )
        {
            if ( t0->voronoiVertex().equalsWeakly( t1->voronoiVertex() ) )
            {
                continue;
            }

            Segment2D s( t0->voronoiVertex(), t1->voronoiVertex() );

            if ( M_bounding_rect )
            {
                Vector2D intersect0, intersect1;
                const int n = M_bounding_rect->intersection( s, &intersect0, &intersect1 );

                if ( n == 0 )
                {
                    if ( M_bounding_rect->contains( s.origin() ) )
                    {
                        M_vertices.insert( s.origin() );
                        M_vertices.insert( s.terminal() );
                        M_segments.push_back( s );
                    }
                }
                else if ( n == 1 )
                {
                    if ( M_bounding_rect->contains( s.origin() ) )
                    {
                        M_vertices.insert( s.origin() );
                        M_vertices.insert( intersect0 );
                        M_segments.emplace_back( s.origin(), intersect0 );
                    }
                    else if ( M_bounding_rect->contains( s.terminal() ) )
                    {
                        M_vertices.insert( s.terminal() );
                        M_vertices.insert( intersect0 );
                        M_segments.emplace_back( s.terminal(), intersect0 );
                    }
                    else
                    {
                        // std::cerr << __FILE__ << ':' << __LINE__ << ':'
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
                M_vertices.insert( s.origin() );
                M_vertices.insert( s.terminal() );
                M_segments.push_back( s );
            }
        }
        else if ( t0 || t1 )
        {
            const DelaunayTriangulation::Triangle * t = ( t0 ? t0 : t1 );

            Vector2D mid
                = e->second->vertex( 0 )->pos()
                + e->second->vertex( 1 )->pos();
            mid *= 0.5;
            AngleDeg dir = ( mid - t->voronoiVertex() ).th();

            if ( ! Triangle2D::contains( t->vertex( 0 )->pos(),
                                         t->vertex( 1 )->pos(),
                                         t->vertex( 2 )->pos(),
                                         t->voronoiVertex() ) )
            {
                AngleDeg dir_centroid = ( centroid - t->voronoiVertex() ).th();
                if ( ( dir_centroid - dir ).abs() < 90.0 )
                {
                    dir += 180.0;
                }
            }

            Ray2D r( t->voronoiVertex(), dir );

            if ( M_bounding_rect )
            {
                Vector2D intersect0, intersect1;

                int n = M_bounding_rect->intersection( r, &intersect0, &intersect1 );

                if ( n == 2 )
                {
                    M_vertices.insert( intersect0 );
                    M_vertices.insert( intersect1 );
                    M_segments.emplace_back( intersect0, intersect1 );
                }
                else if ( n == 1 )
                {
                    M_vertices.insert( r.origin() );
                    M_vertices.insert( intersect0 );
                    M_segments.emplace_back( r.origin(), intersect0 );
                }
                else
                {

                }
            }
            else
            {
                M_vertices.insert( r.origin() );
                M_rays.push_back( r );
            }
        }
        else
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << "Unexpected reach. no triangle for edge" << std::endl;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
VoronoiDiagram::getPointsOnSegments( const double min_length,
                                     const unsigned int max_division,
                                     std::vector< Vector2D > * result ) const
{
    //
    // add points
    //
    result->insert( result->end(),
                    M_vertices.begin(), M_vertices.end() );

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
