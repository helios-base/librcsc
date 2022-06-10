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

const double ViewGridMap::PITCH_MAX_X = ( std::ceil( +ServerParam::DEFAULT_PITCH_LENGTH*0.5 / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );
const double ViewGridMap::PITCH_MAX_Y = ( std::ceil( +ServerParam::DEFAULT_PITCH_WIDTH*0.5 / ViewGridMap::GRID_LENGTH )
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
    ix = bound( 0, ix, ViewGridMap::GRID_X_SIZE );
    iy = bound( 0, iy, ViewGridMap::GRID_Y_SIZE );

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
/*!

*/
struct ViewGridMap::Impl {

    std::vector< int > grid_map_;

    Impl()
        : grid_map_( ViewGridMap::GRID_X_SIZE * ViewGridMap::GRID_Y_SIZE, 0 )
      {

      }

    ~Impl()
      {

      }
};

/*-------------------------------------------------------------------*/
/*!

*/
ViewGridMap::ViewGridMap()
    : M_impl( new ViewGridMap::Impl() )
{
    // std::cerr << __FILE__"create ViewGridMap size=" << M_impl->grid_map_.size()
    //           << std::endl;
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
    for ( int & v : M_impl->grid_map_ )
    {
        v += 1;
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

    const AngleDeg left_angle = view_area.angle() - view_area.viewWidth() * 0.5;
    const AngleDeg right_angle = view_area.angle() + view_area.viewWidth() * 0.5;

    static const double VISIBLE_DIST = ServerParam::i().visibleDistance() - GRID_RADIUS;

    int i = 0;
    for ( std::vector< int >::iterator v = M_impl->grid_map_.begin(), end = M_impl->grid_map_.end();
          v != end;
          ++v, ++i )
    {
        const Vector2D pos = grid_center( i );

        const double dist = view_area.origin().dist( pos );
        if ( dist < VISIBLE_DIST )
        {
            *v = 0;
        }
        else
        {
            const AngleDeg angle = ( pos - view_area.origin() ).th();
            if ( angle.isRightOf( left_angle )
                 && angle.isLeftOf( right_angle ) )
            {
                const double angle_thr = AngleDeg::asin_deg( GRID_RADIUS / dist );
                if ( ( angle - left_angle ).abs() > angle_thr
                     && ( angle - right_angle ).abs() > angle_thr )
                {
                    *v = 0;
                }

            }
        }

    }

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::WORLD,
                  __FILE__" (update) PROFILE elapsed %f [ms] grid_size=%d",
                  timer.elapsedReal(),
                  M_impl->grid_map_.size() );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
int
ViewGridMap::seenCount( const Vector2D & pos )
{
    try
    {
        return M_impl->grid_map_.at( grid_index( pos ) );
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ ": exception caught! " << e.what()
                  << std::endl;
        return 1000;
    }
}

/*-------------------------------------------------------------------*/
void
ViewGridMap::debugOutput() const
{
    const int size = M_impl->grid_map_.size();
    for ( int i = 0; i < size; ++i )
    {
        const Vector2D pos = grid_center( i );

        // dlog.addText( Logger::WORLD,
        //               __FILE__" %05d (%04d %04d) pos=(%.2f %.2f) count=%d",
        //               i, i / ViewGridMap::GRID_Y_SIZE, i % ViewGridMap::GRID_Y_SIZE,
        //               pos.x, pos.y,
        //               M_impl->grid_map_[i] );

        const int col = std::max( 0, 255 - M_impl->grid_map_[i] * 20 );
        dlog.addRect( Logger::WORLD,
                      pos.x - GRID_LENGTH*0.125, pos.y - GRID_LENGTH*0.125,
                      GRID_LENGTH*0.25, GRID_LENGTH*0.25,
                      col, col, col,
                      true );
    }
}

}
