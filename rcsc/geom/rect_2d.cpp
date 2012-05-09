// -*-c++-*-

/*!
  \file rect_2d.cpp
  \brief 2D rectangle region Source File.
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

#include "rect_2d.h"

#include "segment_2d.h"
#include "ray_2d.h"

#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::setTopLeft( const double x,
                    const double y )
{
    double new_left = std::min( right(), x );
    double new_right = std::max( right(), x );
    double new_top = std::min( bottom(), y );
    double new_bottom = std::max( bottom(), y );

    return assign( new_left,
                   new_top,
                   new_right - new_left,
                   new_bottom - new_top );
}

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::setBottomRight( const double x,
                        const double y )
{
    double new_left = std::min( left(), x );
    double new_right = std::max( left(), x );
    double new_top = std::min( top(), y );
    double new_bottom = std::max( top(), y );

    return assign( new_left,
                   new_top,
                   new_right - new_left,
                   new_bottom - new_top );
}

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::setLeft( const double x )
{
    double new_left = std::min( right(), x );
    double new_right = std::max( right(), x );

    M_top_left.x = new_left;
    M_size.setLength( new_right - new_left );
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::setRight( const double x )
{
    double new_left = std::min( left(), x );
    double new_right = std::max( left(), x );

    M_top_left.x = new_left;
    M_size.setLength( new_right - new_left );
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::setTop( const double y )
{
    double new_top = std::min( bottom(), y );
    double new_bottom = std::max( bottom(), y );

    M_top_left.y = new_top;
    M_size.setWidth( new_bottom - new_top );
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::setBottom( const double y )
{
    double new_top = std::min( top(), y );
    double new_bottom = std::max( top(), y );

    M_top_left.y = new_top;
    M_size.setWidth( new_bottom - new_top );
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
Rect2D::intersection( const Line2D & line,
                      Vector2D * sol1,
                      Vector2D * sol2 ) const
{
    int n_sol = 0;
    Vector2D tsol[2];

    const double left_x = left();
    const double right_x = right();
    const double top_y = top();
    const double bottom_y = bottom();

    if ( n_sol < 2
         && ( tsol[n_sol] = leftEdge().intersection( line ) ).isValid()
         && top_y <= tsol[n_sol].y && tsol[n_sol].y <= bottom_y )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = rightEdge().intersection( line ) ).isValid()
         && top_y <= tsol[n_sol].y && tsol[n_sol].y <= bottom_y )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = topEdge().intersection( line ) ).isValid()
         && left_x <= tsol[n_sol].x && tsol[n_sol].x <= right_x )
    {
        ++n_sol;
    }

    if ( n_sol < 2
         && ( tsol[n_sol] = bottomEdge().intersection( line ) ).isValid()
         && left_x <= tsol[n_sol].x && tsol[n_sol].x <= right_x )
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
Rect2D::intersection( const Ray2D & ray,
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
Rect2D::intersection( const Segment2D & segment,
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
const Rect2D &
Rect2D::operator&=( const Rect2D & other )
{
    if ( ! this->isValid()
         || ! other.isValid() )
    {
        M_top_left.assign( 0.0, 0.0 );
        M_size.assign( 0.0, 0.0 );
        return *this;
    }

    double l = std::max( this->left(), other.left() );
    double t = std::max( this->top(), other.top() );
    double w = std::min( this->right(), other.right() ) - l;
    double h = std::min( this->bottom(), other.bottom() ) - t;

    if ( w <= 0.0 || h <= 0.0 )
    {
        M_top_left.assign( 0.0, 0.0 );
        M_size.assign( 0.0, 0.0 );
        return *this;
    }

    M_top_left.assign( l, t );
    M_size.assign( w, h );
    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
const Rect2D &
Rect2D::operator|=( const Rect2D & other )
{
    if ( ! this->isValid()
         && ! other.isValid() )
    {
        M_top_left.assign( 0.0, 0.0 );
        M_size.assign( 0.0, 0.0 );
        return *this;
    }

    double l = std::min( this->left(), other.left() );
    double t = std::min( this->top(), other.top() );
    double w = std::max( this->right(), other.right() ) - l;
    double h = std::max( this->bottom(), other.bottom() ) - t;

    if ( w <= 0.0 || h <= 0.0 )
    {
        M_top_left.assign( 0.0, 0.0 );
        M_size.assign( 0.0, 0.0 );
        return *this;
    }

    M_top_left.assign( l, t );
    M_size.assign( w, h );
    return *this;
}

}
