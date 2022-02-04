// -*-c++-*-

/*!
  \file convex_hull.cpp
  \brief 2D convex hull Source File.
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

#include "convex_hull.h"

#include "triangle_2d.h"

#include <algorithm>

namespace rcsc {


namespace {

struct AngleSortPredicate {
    const Vector2D base_;

    AngleSortPredicate( const Vector2D & p )
        : base_( p )
      { }

    bool operator()( const Vector2D & lhs,
                     const Vector2D & rhs ) const
      {
          // check if "base_ - lhs - rhs" is clockwise order or not.

          double area = Triangle2D::double_signed_area( base_, lhs, rhs );

          if ( area < 0.0 )
          {
              return false;
          }

          if ( area < 1.0e-6 )
          {
              if ( base_.y < lhs.y )
              {
                  if ( base_.dist2( lhs ) > base_.dist2( rhs ) )
                  {
                      return false;
                  }
              }
              else
              {
                  if ( base_.dist2( lhs ) < base_.dist2( rhs ) )
                  {
                      return false;
                  }
              }
          }

          return true;
      }
};

}


/*-------------------------------------------------------------------*/
/*!

 */
ConvexHull::ConvexHull()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
ConvexHull::ConvexHull( const PointCont & v )
    : M_input_points( v )
{


}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::clear()
{
    clearResults();
    M_input_points.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::clearResults()
{
    M_vertices.clear();
    M_edges.clear();
}

/*-------------------------------------------------------------------*/
/*!

 */
// void
// ConvexHull::compute()
// {
//     //computeDirectMethod();
//     computeWrappingMethod();
//     //computeGrahamScan();
// }

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::compute( const MethodType type )
{
    switch ( type ) {
    case DirectMethod:
        computeDirectMethod();
        break;
    case WrappingMethod:
        computeWrappingMethod();
        break;
    case GrahamScan:
        computeGrahamScan();
        break;
    default:
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": unsupported method type(" << type << ")."
                  << std::endl;
        break;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::computeDirectMethod()
{
    clearResults();

    const size_t point_size = M_input_points.size();

    if ( point_size < 3 )
    {
        return;
    }

    for ( size_t i = 0; i < point_size - 1; ++i )
    {
        const Vector2D & p = M_input_points[i];

        for ( size_t j = i + 1; j < point_size; ++j )
        {
            const Vector2D & q = M_input_points[j];
            const Vector2D rel = q - p;

            bool valid = true;
            double last_value = 0.0;

            for ( size_t k = 0; k < point_size; ++k )
            {
                if ( k == i || k == j ) continue;

                const Vector2D & r = M_input_points[k];
                double outer_prod = rel.outerProduct( r - p );

                if ( std::fabs( outer_prod ) < 1.0e-6 )
                {
                    // point is on the line
                    if ( ( r - p ).r2() < rel.r2() )
                    {
                        // point is on the segment
                        valid = false;
                        break;
                    }
                }

                if ( ( outer_prod > 0.0 && last_value < 0.0 )
                     || ( outer_prod < 0.0 && last_value > 0.0 ) )
                {
                    // point exists in the opposite side
                    valid = false;
                    break;
                }

                last_value = outer_prod;
            }

            if ( valid )
            {
                M_vertices.push_back( p );
                M_vertices.push_back( q );

                if ( last_value < 0.0 )
                {
                    M_edges.emplace_back( p, q );
                }
                else
                {
                    M_edges.emplace_back( q, p );
                }
            }
        }
    }

    // sort vertices by counter clockwise order

    if ( ! M_vertices.empty() )
    {
        std::sort( M_vertices.begin() + 1,
                   M_vertices.end(),
                   AngleSortPredicate( M_vertices.front() ) );
        M_vertices.erase( std::unique( M_vertices.begin(), M_vertices.end(),
                                       //Vector2D::Equal() ),
                                       []( const Vector2D & lhs, const Vector2D & rhs )
                                         {
                                             return lhs.equals( rhs );
                                         }
                                       ),
                          M_vertices.end() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::computeWrappingMethod()
{
    clearResults();

    const size_t point_size = M_input_points.size();

    if ( point_size < 3 )
    {
        return;
    }

    size_t min_index = getMinPointIndex();

    if ( min_index == size_t( -1 ) )
    {
        return;
    }

#ifdef DEBUG_PRINT
    std::cerr << "min_point=" min_index << ":" << M_input_points[min_index] << std::endl;
#endif

    std::set< size_t > vertices; // temporal set for checking already used vertices.

    M_vertices.push_back( M_input_points[min_index] );

    size_t current_index = min_index;
    Vector2D current_point = M_input_points[min_index];

    for ( size_t loop_count = 0; loop_count <= point_size; ++loop_count ) // while ( 1 )
    {
        //std::cerr << "loop " << loop_count
        //          << " base=" << current_index << ':' << current_point << std::endl;

        size_t candidate = 0;
        for ( size_t i = 0; i < point_size; ++i )
        {
            if ( i == current_index ) continue;
            if ( vertices.find( i ) != vertices.end() ) continue;

            candidate = i;
            break;
        }

        for ( size_t i = candidate + 1; i < point_size; ++i )
        {
            if ( i == current_index ) continue;
            if ( vertices.find( i ) != vertices.end() ) continue;

            const Vector2D & p = M_input_points[candidate];
            const Vector2D & q = M_input_points[i];

            double area = Triangle2D::double_signed_area( current_point, p, q );

            if ( area < 0.0 )
            {
                candidate = i;
            }
            else if ( area < 1.0e-6 )
            {
                if ( current_point.dist2( p ) > current_point.dist2( q ) )
                {
                    candidate = i;
                }
            }
        }

        current_index = candidate;
        current_point = M_input_points[current_index];
        vertices.insert( current_index );
        M_vertices.push_back( current_point );

        if ( current_index == min_index )
        {
            break;
        }

        //std::cerr << "updated to " << current_index << ":" << M_input_points[current_index] << std::endl;
    }

    VertexCont::iterator p = M_vertices.begin();
    VertexCont::iterator n = p;
    ++n;
    for ( ; n != M_vertices.end(); ++n )
    {
        M_edges.emplace_back( *p, *n );
        p = n;
    }
    M_vertices.pop_back();
}

namespace {

inline
bool
is_clockwise( const Vector2D & p0,
              const Vector2D & p1,
              const Vector2D & p2 )
{
    double area = Triangle2D::double_signed_area( p0, p1, p2 );

    return ( area < 0.0
             || ( area < 1.0e-6
                  && p0.dist2( p1 ) > p0.dist2( p2 ) ) );
}

}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::computeGrahamScan()
{
    clearResults();

    const size_t point_size = M_input_points.size();

    if ( point_size < 3 )
    {
        return;
    }

    size_t min_index = getMinPointIndex();

    if ( min_index == size_t( -1 ) )
    {
        return;
    }

    sortPointsByAngleFrom( min_index );

    M_vertices = M_input_points;

    size_t top = 1;
    for ( size_t i = 2; i < point_size; ++i )
    {
        while ( is_clockwise( M_vertices[top-1],
                              M_vertices[top],
                              M_input_points[i] ) )
        {
            --top;
        }

        ++top;

        std::swap( M_vertices[top], M_vertices[i] );
    }

    ++top;

    M_vertices.erase( M_vertices.begin() + top,
                      M_vertices.end() );

    VertexCont::iterator p = M_vertices.begin();
    VertexCont::iterator n = p;
    ++n;
    for ( ; n != M_vertices.end(); ++n )
    {
        M_edges.emplace_back( *p, *n );
        p = n;
    }
    M_edges.emplace_back( M_vertices.back(), M_vertices.front() );
}

/*-------------------------------------------------------------------*/
/*!

 */
// void
// ConvexHull::computeIncrementalMethod()
// {
// }

/*-------------------------------------------------------------------*/
/*!

 */
// void
// ConvexHull::computeDivideAndConquer()
// {
// }

/*-------------------------------------------------------------------*/
/*!

 */
// void
// ConvexHull::computeQuickMethod()
// {
// }

/*-------------------------------------------------------------------*/
/*!

 */
// void
// ConvexHull::computeInnerPointsElimination()
// {
// }

/*-------------------------------------------------------------------*/
/*!

 */
size_t
ConvexHull::getMinPointIndex() const
{
    const size_t point_size = M_input_points.size();

    if ( point_size == 0 )
    {
        return size_t( -1 );
    }

    size_t min_index = 0;

    Vector2D min_point = M_input_points[0];
    for ( size_t i = 1; i < point_size; ++i )
    {
        const Vector2D & p = M_input_points[i];

        if ( min_point.x > p.x
             || ( min_point.x == p.x
                  && min_point.y > p.y ) )
        {
            min_point = p;
            min_index = i;
        }
    }

    return min_index;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHull::sortPointsByAngleFrom( const size_t index )
{
    if ( M_input_points.size() <= index )
    {
        return;
    }

    std::swap( M_input_points[0], M_input_points[index] );

    std::sort( M_input_points.begin() + 1,
               M_input_points.end(),
               AngleSortPredicate( M_input_points.front() ) );

#ifdef DEBUG_PRINT
    std::cout << '\n';
    std::cout << "min_point=" << M_input_points[0] << '\n';
    for ( size_t i = 1; i < M_input_points.size(); ++i )
    {
        std::cout << M_input_points[i]
                  << ": angle=" << ( M_input_points[i] - M_input_points[0] ).th()
                  << '\n';
    }
    std::cout << std::flush;
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
Polygon2D
ConvexHull::toPolygon() const
{
    return Polygon2D( M_vertices );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ConvexHull::printInputPoints( std::ostream & os ) const
{
    //os << "set xrange [0:5]\n";
    //os << "set yrange [0:3]\n;"
    //os << "plot \"-\" using 1:2 with points\n";

    for ( const Vector2D & p : M_input_points )
    {
        os << p.x << ' ' << p.y << '\n';
    }

    //os << "end\n";

    return os << std::flush;
}


/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ConvexHull::printVertices( std::ostream & os ) const
{

    for ( const Vector2D & v : vertices() )
    {
        os << v.x << ' ' << v.y << '\n';
    }

    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ConvexHull::printEdges( std::ostream & os ) const
{
    //os << "plot \"-\" using 1:2 with lines\n";

    for ( const Segment2D & e : edges() )
    {
        os << e.origin().x << ' ' << e.origin().y << '\n'
           << e.terminal().x << ' ' << e.terminal().y << '\n'
           << '\n';
    }

    //os << "end\n";
    //os << "pause -1\n";

    return os << std::flush;
}

}
