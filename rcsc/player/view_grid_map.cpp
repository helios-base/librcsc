// -*-c++-*-

/*!
  \file view_grid_map.cpp
  \brief field grid map scored by see info Source File
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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "view_grid_map.h"

#include <rcsc/player/view_area.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/time/timer.h>
#include <rcsc/game_time.h>
#include <rcsc/math_util.h>


#include <vector>
#include <iostream>
#include <cstring>
#include <cmath>

#define DEBUG_PROFILE
#define DEBUG_PRINT

namespace rcsc {

const double ViewGridMap::GRID_LENGTH = 1.0;

const double ViewGridMap::PITCH_MAX_X = ( std::ceil( ( +ServerParam::DEFAULT_PITCH_LENGTH*0.5 - 3.0 ) / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );
const double ViewGridMap::PITCH_MAX_Y = ( std::ceil( ( +ServerParam::DEFAULT_PITCH_WIDTH*0.5 - 3.0 ) / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );

const double ViewGridMap::PITCH_LENGTH = ViewGridMap::PITCH_MAX_X * 2.0;
const double ViewGridMap::PITCH_WIDTH = ViewGridMap::PITCH_MAX_Y * 2.0;

const int ViewGridMap::GRID_X_SIZE = static_cast< int >( std::ceil( ViewGridMap::PITCH_LENGTH / ViewGridMap::GRID_LENGTH ) ) + 1;
const int ViewGridMap::GRID_Y_SIZE = static_cast< int >( std::ceil( ViewGridMap::PITCH_WIDTH / ViewGridMap::GRID_LENGTH ) ) + 1;

const double ViewGridMap::GRID_RADIUS = ViewGridMap::GRID_LENGTH*0.5 * std::sqrt( 2.0 );


namespace {

inline
int
grid_x_index( const double x )
{
    int ix = static_cast< int >( std::ceil( ( x + ViewGridMap::PITCH_MAX_X )
                                            / ViewGridMap::GRID_LENGTH ) );
    return bound( 0, ix, ViewGridMap::GRID_X_SIZE );
}

inline
int
grid_y_index( const double y )
{
    int iy = static_cast< int >( std::ceil( ( y + ViewGridMap::PITCH_MAX_Y )
                                            / ViewGridMap::GRID_LENGTH ) );
    return bound( 0, iy, ViewGridMap::GRID_Y_SIZE );
}

inline
int
grid_index( const Vector2D & pos )
{
    int ix = static_cast< int >( std::ceil( ( pos.x + ViewGridMap::PITCH_MAX_X )
                                            / ViewGridMap::GRID_LENGTH ) );
    int iy = static_cast< int >( std::ceil( ( pos.y + ViewGridMap::PITCH_MAX_Y )
                                            / ViewGridMap::GRID_LENGTH ) );
    ix = bound( 0, ix, ViewGridMap::GRID_X_SIZE - 1 );
    iy = bound( 0, iy, ViewGridMap::GRID_Y_SIZE - 1 );

    return ix * ViewGridMap::GRID_Y_SIZE + iy;
}

inline
Vector2D
grid_center( const int ix,
             const int iy )
{
    return Vector2D( ix * ViewGridMap::GRID_LENGTH - ViewGridMap::PITCH_MAX_X,
                     iy * ViewGridMap::GRID_LENGTH - ViewGridMap::PITCH_MAX_Y );
}

inline
Vector2D
grid_center( const int idx )
{
    return grid_center( idx / ViewGridMap::GRID_Y_SIZE,
                        idx % ViewGridMap::GRID_Y_SIZE );
}

}


///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
ViewGridMap::ViewGridMap()
{
    M_grid_map.reserve( GRID_X_SIZE * GRID_Y_SIZE );

    for ( int x = 0; x < GRID_X_SIZE; ++x )
    {
        for ( int y = 0; y < GRID_Y_SIZE; ++y )
        {
            M_grid_map.emplace_back( grid_center( x, y ) );
        }
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
ViewGridMap::~ViewGridMap()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
ViewGridMap::incrementAll()
{
    for ( Grid & p : M_grid_map )
    {
        p.seen_count_ += 1;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ViewGridMap::update( const GameTime & time,
                     const ViewArea & view_area )
{
    static GameTime s_update_time( 0, 0 );

    if ( s_update_time == time )
    {
        return;
    }
    s_update_time = time;


#ifdef DEBUG_PROFILE
    Timer timer;
#endif

    if ( ! view_area.isValid() )
    {
        return;
    }

    const AngleDeg left_angle = view_area.angle() - view_area.viewWidth() * 0.5 + 2.0;
    const AngleDeg right_angle = view_area.angle() + view_area.viewWidth() * 0.5 - 2.0;

    static const double VISIBLE_DIST = ServerParam::i().visibleDistance() - 0.5;

    for ( Grid & p : M_grid_map )
    {
        const double dist = view_area.origin().dist( p.center_ );
        if ( dist < VISIBLE_DIST )
        {
            p.seen_count_ = 0;
        }
        else
        {
            const AngleDeg angle = ( p.center_ - view_area.origin() ).th();
            if ( angle.isRightOf( left_angle )
                 && angle.isLeftOf( right_angle ) )
            {
                p.seen_count_ = 0;
            }
        }

    }

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::WORLD,
                  __FILE__" (update) PROFILE elapsed %f [ms] grid_size=%d",
                  timer.elapsedReal(),
                  M_grid_map.size() );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
int
ViewGridMap::seenCount( const Vector2D & pos ) const
{
    try
    {
        return M_grid_map.at( grid_index( pos ) ).seen_count_;
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ ": out of the grid range " << pos
                  << " " << e.what()
                  << std::endl;
        return 1000;
    }
}

/*-------------------------------------------------------------------*/
void
ViewGridMap::debugOutput() const
{
    for ( const Grid & p : M_grid_map )
    {
        const int col = std::max( 0, 255 - p.seen_count_ * 20 );
        dlog.addRect( Logger::WORLD,
                      p.center_.x - GRID_LENGTH*0.05, p.center_.y - GRID_LENGTH*0.05,
                      GRID_LENGTH*0.1, GRID_LENGTH*0.1,
                      col, col, col,
                      true );
    }
}

}
