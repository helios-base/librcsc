// -*-c++-*-

/*!
  \file segment_2d.cpp
  \brief 2D segment line class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama, Hiroki Shimora

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

#include "segment_2d.h"
#include "triangle_2d.h"

#include <algorithm>
#include <iostream>

namespace rcsc {

const double Segment2D::EPSILON = 1.0e-6;
const double Segment2D::CALC_ERROR = 1.0e-9;

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::projection( const Vector2D & p ) const
{
    Vector2D dir = terminal() - origin();
    double len = dir.r();

    if ( len < EPSILON )
    {
        return origin();
    }

    dir /= len; // normalize

    double d = dir.innerProduct( p - origin() );
    if ( -EPSILON < d && d < len + EPSILON )
    {
        dir *= d;
        return Vector2D( origin() ) += dir;
    }

    return Vector2D::INVALIDATED;

#if 0
    Line2D my_line = this->line();
    Vector2D sol = my_line.projection( p );

    if ( ! sol.isValid()
         || ! this->contains( sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    return sol;
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::intersection( const Segment2D & other,
                         const bool allow_end_point ) const
{
    Vector2D sol = this->line().intersection( other.line() );

    if ( ! sol.isValid()
         || ! this->contains( sol )
         || ! other.contains( sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    if ( ! allow_end_point
         && ! existIntersectionExceptEndpoint( other ) )
    {
        return Vector2D::INVALIDATED;
    }

    return sol;

#if 0
    // Following algorithm seems faster ther above.
    // In fact, the following algorithm slower...

    Vector2D ab = terminal() - origin();
    Vector2D dc = other.origin() - other.terminal();
    Vector2D ad = other.terminal() - origin();

    double det = dc.outerProduct( ab );

    if ( std::fabs( det ) < 1.0e-9 )
    {
        // area size is 0.
        // segments has same slope.
        std::cerr << "Segment2D::intersection()"
                  << " ***ERROR*** parallel segments"
                  << std::endl;
        return Vector2D::INVALIDATED;
    }

    double s = ( dc.x * ad.y - dc.y * ad.x ) / det;
    double t = ( ab.x * ad.y - ab.y * ad.x ) / det;

    if ( s < 0.0 || 1.0 < s || t < 0.0 || 1.0 < t )
    {
        return Vector2D::INVALIDATED;
    }

    return Vector2D( origin().x + ab.x * s, origin().y + ab.y * s );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::intersection( const Line2D & l ) const
{
    Line2D my_line = this->line();

    Vector2D sol = my_line.intersection( l );

    if ( ! sol.isValid()
         || ! this->contains( sol ) )
    {
        return Vector2D::INVALIDATED;
    }

    return sol;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::existIntersectionExceptEndpoint( const Segment2D & other ) const
{
//     return ( Triangle2D( *this, other.origin() ).doubleSignedArea()
//              * Triangle2D( *this, other.terminal() ).doubleSignedArea()
//              < 0.0 )
//         && ( Triangle2D( other, this->origin() ).doubleSignedArea()
//              * Triangle2D( other, this->terminal() ).doubleSignedArea()
//              < 0.0 );
    return ( Triangle2D::double_signed_area( this->origin(), this->terminal(), other.origin() )
             * Triangle2D::double_signed_area( this->origin(), this->terminal(), other.terminal() )
             < 0.0 )
        && ( Triangle2D::double_signed_area( other.origin(), other.terminal(), this->origin() )
             * Triangle2D::double_signed_area( other.origin(), other.terminal(), this->terminal() )
             < 0.0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::existIntersection( const Segment2D & other ) const
{
//     double a0 = Triangle2D( *this, other.origin() ).doubleSignedArea();
//     double a1 = Triangle2D( *this, other.terminal() ).doubleSignedArea();
//     double b0 = Triangle2D( other, this->origin() ).doubleSignedArea();
//     double b1 = Triangle2D( other, this->terminal() ).doubleSignedArea();
    double a0 = Triangle2D::double_signed_area( this->origin(), this->terminal(), other.origin() );
    double a1 = Triangle2D::double_signed_area( this->origin(), this->terminal(), other.terminal() );
    double b0 = Triangle2D::double_signed_area( other.origin(), other.terminal(), this->origin() );
    double b1 = Triangle2D::double_signed_area( other.origin(), other.terminal(), this->terminal() );

    if ( a0 * a1 < 0.0 && b0 * b1 < 0.0 )
    {
        return true;
    }

    //if ( this->origin().equals( this->terminal() ) )
    if ( this->origin() == this->terminal() )
    {
        //if ( other.origin().equals( other.terminal() ) )
        if ( other.origin() == other.terminal() )
        {
            //return this->origin().equals( other.origin() );
            return this->origin() == other.origin();
        }

        return b0 == 0.0 && other.checkIntersectsOnLine( this->origin() );
    }
    //else if ( other.origin().equals( other.terminal() ) )
    else if ( other.origin() == other.terminal() )
    {
        return a0 == 0.0 && this->checkIntersectsOnLine( other.origin() );
    }

    if ( ( a0 == 0.0 && this->checkIntersectsOnLine( other.origin() ) )
         || ( a1 == 0.0 && this->checkIntersectsOnLine( other.terminal() ) )
         || ( b0 == 0.0 && other.checkIntersectsOnLine( this->origin() ) )
         || ( b1 == 0.0 && other.checkIntersectsOnLine( this->terminal() ) ) )
    {
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::existIntersection( const Line2D & l ) const
{
    double a0 = l.a() * origin().x + l.b() * origin().y + l.c();
    double a1 = l.a() * terminal().x + l.b() * terminal().y + l.c();

    return a0 * a1 <= 0.0;
}

/*-------------------------------------------------------------------*/
/*!

*/
// private
bool
Segment2D::checkIntersectsOnLine( const Vector2D & p ) const
{
    if ( origin().x == terminal().x )
    {
        return ( origin().y <= p.y && p.y <= terminal().y )
            || ( terminal().y <= p.y && p.y <= origin().y );
    }
    else
    {
        return ( origin().x <= p.x && p.x <= terminal().x )
            || ( terminal().x <= p.x && p.x <= origin().x );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
Segment2D::nearestPoint( const Vector2D & p ) const
{
    const Vector2D vec = terminal() - origin();

    const double len_square = vec.r2();

    if ( len_square == 0.0 )
    {
        return origin();
    }


    double inner_product = vec.innerProduct( (p - origin()) );

    //
    // A: p1 - p0
    // B: p - p0
    //
    // check if 0 <= |B|cos(theta) <= |A|
    //       -> 0 <= |A||B|cos(theta) <= |A|^2
    //       -> 0 <= A.B <= |A|^2  // A.B = |A||B|cos(theta)
    //
    if ( inner_product <= 0.0 )
    {
        return origin();
    }
    else if ( inner_product >= len_square )
    {
        return terminal();
    }

    return origin() + vec * inner_product / len_square;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Segment2D::dist( const Vector2D & p ) const
{
    double len = this->length();

    if ( len == 0.0 )
    {
        return origin().dist( p );
    }

    const Vector2D vec = terminal() - origin();
    const double prod = vec.innerProduct( p - origin() );

    //
    // A: p1 - p0
    // A: p - p0
    //
    // check if 0 <= |B|cos(theta) <= |A|
    //       -> 0 <= |A||b|cos(theta) <= |A|^2
    //       -> 0 <= A.B <= |A|^2  // A.B = |A||B|cos(theta)
    //
    if ( 0.0 <= prod && prod <= len * len )
    {
        // return perpendicular distance
        //return std::fabs( Triangle2D( *this, p ).doubleSignedArea() / len );
        return std::fabs( Triangle2D::double_signed_area( origin(), terminal(), p ) / len );
    }

    return std::sqrt( std::min( origin().dist2( p ),
                                terminal().dist2( p ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Segment2D::dist( const Segment2D & seg ) const
{
    if ( this->existIntersection( seg ) )
    {
        return 0.0;
    }

    return std::min( std::min( this->dist( seg.origin() ),
                               this->dist( seg.terminal() ) ),
                     std::min( seg.dist( origin() ),
                               seg.dist( terminal() ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
Segment2D::farthestDist( const Vector2D & p ) const
{
    return std::sqrt( std::max( origin().dist2( p ),
                                terminal().dist2( p ) ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Segment2D::onSegment( const Vector2D & p ) const
{
    //return Triangle2D( *this, p ).doubleSignedArea() == 0.0
    return Triangle2D::double_signed_area( origin(), terminal(), p ) == 0.0
            && checkIntersectsOnLine( p );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Segment2D::onSegmentWeakly( const Vector2D & p ) const
{
    Vector2D proj = projection( p );

    return ( proj.isValid()
             && p.equalsWeakly( proj ) );

#if 0
    Vector2D o = origin();
    Vector2D t = terminal();

    const double buf = ( allow_on_terminal
                         ? EPSILON
                         : 0.0 );

    if ( std::fabs( ( t - o ).outerProduct( p - o ) ) < EPSILON )
    {
        if ( std::fabs( o.x - t.x ) < EPSILON )
        {
            return ( ( o.y - buf < p.y && p.y < t.y + buf )
                     || ( t.y - buf < p.y && p.y < o.y + buf ) );
        }
        else
        {
            return ( ( o.x - buf < p.x && p.x < t.x + buf )
                     || ( t.x - buf < p.x && p.x < o.x + buf) );
        }
    }

    return false;
#endif
}

}
