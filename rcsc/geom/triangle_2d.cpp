// -*-c++-*-

/*!
  \file triangle_2d.cpp
  \brief 2D triangle class Source File.
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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "triangle_2d.h"

#include "line_2d.h"
#include "ray_2d.h"
#include "segment_2d.h"

#include <cmath>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
bool
Triangle2D::contains( const Vector2D & point ) const
{
    Vector2D rel1( M_a - point );
    Vector2D rel2( M_b - point );
    Vector2D rel3( M_c - point );

    double outer1 = rel1.outerProduct( rel2 );
    double outer2 = rel2.outerProduct( rel3 );
    double outer3 = rel3.outerProduct( rel1 );

    if ( (outer1 >= 0.0 && outer2 >= 0.0 && outer3 >= 0.0)
         || (outer1 <= 0.0 && outer2 <= 0.0 && outer3 <= 0.0) )
    {
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Triangle2D::intersection( const Line2D & line,
                          Vector2D * sol1,
                          Vector2D * sol2 ) const
{
    int n_sol = 0;
    Vector2D tsol[2];

    if ( n_sol < 2
         && ( tsol[n_sol] = Segment2D( a(), b() ).intersection( line ) ).isValid() )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = Segment2D( b(), c() ).intersection( line ) ).isValid() )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = Segment2D( c(), a() ).intersection( line ) ).isValid() )
    {
        ++n_sol;
    }

    if ( n_sol == 2
         && std::fabs( tsol[0].x - tsol[1].x ) < 1.0e-5
         && std::fabs( tsol[0].y - tsol[1].y ) < 1.0e-5 )
    {
        n_sol = 1;
    }

    if ( n_sol > 0
         && sol1 )
    {
        *sol1 = tsol[0];
    }

    if ( n_sol > 1
         && sol2 )
    {
        *sol2 = tsol[1];
    }

    return n_sol;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Triangle2D::intersection( const Ray2D & ray,
                          Vector2D * sol1,
                          Vector2D * sol2 ) const
{
    Vector2D tsol1, tsol2;
    int n_sol = intersection( ray.line(), &tsol1, &tsol2 );

    if ( n_sol > 1
         && ! ray.inRightDir( tsol2, 1.0 ) )
    {
        --n_sol;
    }

    if ( n_sol > 0
         && ! ray.inRightDir( tsol1, 1.0 ) )
    {
        tsol1 = tsol2;
        --n_sol;
    }

    if ( n_sol > 0
         && sol1 )
    {
        *sol1 = tsol1;
    }

    if ( n_sol > 1
         && sol2 )
    {
        *sol2 = tsol2;
    }

    return n_sol;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Triangle2D::intersection( const Segment2D & segment,
                          Vector2D * sol1,
                          Vector2D * sol2 ) const
{
    Vector2D tsol1, tsol2;
    int n_sol = intersection( segment.line(), &tsol1, &tsol2 );

    if ( n_sol > 1
         && ! segment.contains( tsol2 ) )
    {
        --n_sol;
    }

    if ( n_sol > 0
         && ! segment.contains( tsol1 ) )
    {
        tsol1 = tsol2;
        --n_sol;
    }

    if ( n_sol > 0
         && sol1 )
    {
        *sol1 = tsol1;
    }

    if ( n_sol > 1
         && sol2 )
    {
        *sol2 = tsol2;
    }

    return n_sol;
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
Triangle2D::incenter( const Vector2D & a,
                      const Vector2D & b,
                      const Vector2D & c )
{
    Vector2D ab = b - a;
    Vector2D ac = c - a;
    Line2D bisect_a( a,
                     AngleDeg::bisect( ab.th(), ac.th() ) );

    Vector2D ba = a - b;
    Vector2D bc = c - b;
    Line2D bisect_b( b,
                     AngleDeg::bisect( ba.th(), bc.th() ) );

    return bisect_a.intersection( bisect_b );
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
Triangle2D::circumcenter( const Vector2D & a,
                          const Vector2D & b,
                          const Vector2D & c )
{
    Line2D perpendicular_ab
        = Line2D::perpendicular_bisector( a, b );

    Line2D perpendicular_bc
        = Line2D::perpendicular_bisector( b, c );

    Vector2D sol = perpendicular_ab.intersection( perpendicular_bc );

    if ( ! sol.isValid() )
    {
        Line2D perpendicular_ca
            = Line2D::perpendicular_bisector( c, a );
        sol = perpendicular_ab.intersection( perpendicular_ca );

        if ( sol.isValid() )
        {
            return sol;
        }

        sol = perpendicular_bc.intersection( perpendicular_ca );

        if ( sol.isValid() )
        {
            return sol;
        }
    }

    // Following algorithm seems faster than above method.
    // However, result is as:
    //   above method     10000000times 730 [ms]
    //   following method 10000000times 934 [ms]
    // So, I choose above method.

    ////////////////////////////////////////////////////////////////
    // Q : curcumcenter
    // M : center of AB
    // N : center of AC
    // s, t : parameter
    // <,> : inner product operator
    // S : area of triangle
    // a = |BC|, b = |CA|, c = |AB|

    // AQ = s*AB + t*AC

    // <MQ, AB> = <AQ - AM, AB>
    //          = <s*AB + t*AC - AB/2, AB >
    //          = <(s-1/2)*AB^2 + tAB, AC>
    //          = (s-1/2)*c^2 + t*b*c*cosA
    //          = 0
    // <NQ, AC> = s*b*c*cosA + (t-1/2)*b^2 = 0

    // c^2 * s + (b*c*cosA)*t = c^2 / 2
    // (b*c*cosA)*s + b^2 * t = b^2 / 2

    // s = b^2 * (c^2 + a^2 - b^2) / (16S^2)
    // t = c^2 * (a^2 + b^2 - c^2) / (16S^2)

    // AQ = {b^2 * (c^2 + a^2 - b^2) * AB + c^2 * (a^2 + b^2 - c^2)) * AC} /(16S^2)

    Vector2D ab = b - a;
    Vector2D ca = c - a;

    double tmp = ab.outerProduct( ca );
    if( std::fabs( tmp ) < 1.0e-10 )
    {
        // The area of parallelogram is 0.
        std::cerr << "Triangle2D::circumcenter()"
                  << " ***ERROR*** at least, two vertex points have same coordiante.\n"
                  << a << '\n'
                  << b << '\n'
                  << c << '\n'
                  << std::endl;
        return Vector2D( Vector2D::INVALIDATED );
    }

    double inv = 0.5 / tmp;
    double ab_len2 = ab.r2();
    double ca_len2 = ca.r2();
    double xcc = inv * ( ab_len2 * ca.y - ca_len2 * ab.y );
    double ycc = inv * ( ab.x * ca_len2 - ca.x * ab_len2 );


    // circle radius = xcc*xcc + ycc*ycc
    return Vector2D( a.x + xcc, a.y + ycc );
}

/*-------------------------------------------------------------------*/
/*!

 */
Vector2D
Triangle2D::orthocenter( const Vector2D & a,
                         const Vector2D & b,
                         const Vector2D & c )
{
    Line2D perpend_a = Line2D( b, c ).perpendicular( a );
    Line2D perpend_b = Line2D( c, a ).perpendicular( b );

    return perpend_a.intersection( perpend_b );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Triangle2D::contains( const Vector2D & a,
                      const Vector2D & b,
                      const Vector2D & c,
                      const Vector2D & point )
{
    Vector2D rel1( a - point );
    Vector2D rel2( b - point );
    Vector2D rel3( c - point );

    double outer1 = rel1.outerProduct( rel2 );
    double outer2 = rel2.outerProduct( rel3 );
    double outer3 = rel3.outerProduct( rel1 );

    if ( (outer1 >= 0.0 && outer2 >= 0.0 && outer3 >= 0.0)
         || (outer1 <= 0.0 && outer2 <= 0.0 && outer3 <= 0.0) )
    {
        return true;
    }

    return false;
}

}
