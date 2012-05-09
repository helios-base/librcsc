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

// #define USE_VECTOR
// #define USE_MULTI_ARRAY

#ifdef USE_VECTOR
#  include <vector>
#elif USE_MULTY_ARRAY
#  include <boost/multi_array.hpp>
#endif

#include <iostream>
#include <cstring>
#include <cmath>

#define DEBUG_PROFILE
// #define DEBUG_PRINT
// #define DEBUG_PRINT_TEXT
// #define DEBUG_PAINT_GRID_MAP

namespace rcsc {

const double ViewGridMap::GRID_LENGTH = 2.0;

const double ViewGridMap::PITCH_MIN_X = ( std::ceil( ( -ServerParam::DEFAULT_PITCH_LENGTH*0.5 + 1.5 )
                                                     / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );
const double ViewGridMap::PITCH_MAX_X = ( std::floor( ( +ServerParam::DEFAULT_PITCH_LENGTH*0.5 - 0.5 )
                                                      / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );
const double ViewGridMap::PITCH_LENGTH = ViewGridMap::PITCH_MAX_X - ViewGridMap::PITCH_MIN_X;

const double ViewGridMap::PITCH_MIN_Y = ( std::ceil( ( -ServerParam::DEFAULT_PITCH_WIDTH*0.5 + 1.5 )
                                                     / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );
const double ViewGridMap::PITCH_MAX_Y = ( std::floor( ( +ServerParam::DEFAULT_PITCH_WIDTH*0.5 - 1.5 )
                                                    / ViewGridMap::GRID_LENGTH )
                                          * ViewGridMap::GRID_LENGTH );
const double ViewGridMap::PITCH_WIDTH = ViewGridMap::PITCH_MAX_Y - ViewGridMap::PITCH_MIN_Y;


const int ViewGridMap::GRID_X_SIZE
= static_cast< int >( std::ceil( ViewGridMap::PITCH_LENGTH / ViewGridMap::GRID_LENGTH ) ) + 1;
const int ViewGridMap::GRID_Y_SIZE
= static_cast< int >( std::ceil( ViewGridMap::PITCH_WIDTH / ViewGridMap::GRID_LENGTH ) ) + 1;

const double ViewGridMap::GRID_RADIUS = ViewGridMap::GRID_LENGTH*0.5 * std::sqrt( 2.0 );


namespace {

inline
int
grid_x_index( const double & x )
{
    int ix = static_cast< int >( std::ceil( ( x - ViewGridMap::PITCH_MIN_X )
                                            / ViewGridMap::GRID_LENGTH ) );
    return bound( 0, ix, ViewGridMap::GRID_X_SIZE );
}

inline
int
grid_y_index( const double & y )
{
    int iy = static_cast< int >( std::ceil( ( y - ViewGridMap::PITCH_MIN_Y )
                                            / ViewGridMap::GRID_LENGTH ) );
    return bound( 0, iy, ViewGridMap::GRID_Y_SIZE );
}

inline
int
grid_index( const Vector2D & pos )
{
    int ix = static_cast< int >( std::ceil( ( pos.x - ViewGridMap::PITCH_MIN_X )
                                            / ViewGridMap::GRID_LENGTH ) );
    int iy = static_cast< int >( std::ceil( ( pos.y - ViewGridMap::PITCH_MIN_Y )
                                            / ViewGridMap::GRID_LENGTH ) );
    ix = bound( 0, ix, ViewGridMap::GRID_X_SIZE );
    iy = bound( 0, iy, ViewGridMap::GRID_Y_SIZE );

    return ix * ViewGridMap::GRID_Y_SIZE + iy;
}

inline
Vector2D
grid_center( const int idx )
{
    int ix = idx / ViewGridMap::GRID_Y_SIZE;
    int iy = idx % ViewGridMap::GRID_Y_SIZE;

    return Vector2D( ix * ViewGridMap::GRID_LENGTH + ViewGridMap::PITCH_MIN_X,
                     iy * ViewGridMap::GRID_LENGTH + ViewGridMap::PITCH_MIN_Y );
}

inline
Vector2D
grid_center( const int ix,
             const int iy )
{
    return Vector2D( ix * ViewGridMap::GRID_LENGTH + ViewGridMap::PITCH_MIN_X,
                     iy * ViewGridMap::GRID_LENGTH + ViewGridMap::PITCH_MIN_Y );
}

}


///////////////////////////////////////////////////////////////////////
/*!

*/
struct ViewGridMap::Impl {

#ifdef USE_VECTOR
    std::vector< int > grid_map_;
#elif defined (USE_MULTI_ARRAY)
    boost::multi_array< int, 2 > grid_map_;
#else
    int ** grid_map_;
#endif

    Impl()
        :
#ifdef USE_VECTOR
        grid_map_( ViewGridMap::GRID_X_SIZE * ViewGridMap::GRID_Y_SIZE, 0 )
#elif defined (USE_MULTI_ARRAY)
        grid_map_( boost::extents[ViewGridMap::GRID_X_SIZE][ViewGridMap::GRID_Y_SIZE] )
#else
        grid_map_( 0 )
#endif
      {
#ifdef USE_VECTOR
          // do nothing
#elif defined (USE_MULTI_ARRAY)
          for ( int x = 0; x < ViewGridMap::GRID_X_SIZE; ++x )
          {
              for ( int y = 0; y < ViewGridMap::GRID_Y_SIZE; ++y )
              {
                  grid_map_[x][y] = 0;
              }
          }
#else
          grid_map_ = new int*[ViewGridMap::GRID_X_SIZE];
          for ( int x = 0; x < ViewGridMap::GRID_X_SIZE; ++x )
          {
              grid_map_[x] = new int[ViewGridMap::GRID_Y_SIZE];
              for ( int y = 0; y < ViewGridMap::GRID_Y_SIZE; ++y )
              {
                  grid_map_[x][y] = 0;
              }
          }
#endif
      }

    ~Impl()
      {
#ifdef USE_VECTOR

#elif defined( USE_MULTI_ARRAY)

#else
          for ( int y = 0; y < ViewGridMap::GRID_Y_SIZE; ++y )
          {
              delete [] grid_map_[y];
          }

          delete [] grid_map_;
#endif
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
#ifdef USE_VECTOR
    const std::vector< int >::iterator end = M_impl->grid_map_.end();
    for ( std::vector< int >::iterator v = M_impl->grid_map_.begin();
          v != end;
          ++v )
    {
        *v += 1;
    }
#else
    for ( int x = 0; x < ViewGridMap::GRID_X_SIZE; ++x )
    {
        for ( int y = 0; y < ViewGridMap::GRID_Y_SIZE; ++y )
        {
            M_impl->grid_map_[x][y] += 1;
        }
    }
#endif
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

#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" PITCH_MIN_X=%.1f PITCH_MAX_X=%.1f GRID_X_SIZE=%d",
                  PITCH_MIN_X, PITCH_MAX_X,
                  GRID_X_SIZE );
    dlog.addText( Logger::WORLD,
                  __FILE__" PITCH_MIN_Y=%.1f PITCH_MAX_Y=%.1f GRID_Y_SIZE=%d",
                  PITCH_MIN_Y, PITCH_MAX_Y,
                  GRID_Y_SIZE );
    dlog.addText( Logger::WORLD,
                  __FILE__" grid map size=%d",
                  M_impl->grid_map_.size() );
#endif


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

#ifdef USE_VECTOR
    int i = 0;
    const std::vector< int >::iterator end = M_impl->grid_map_.end();
    for ( std::vector< int >::iterator v = M_impl->grid_map_.begin();
          v != end;
          ++v, ++i )
    {
        int & value = *v;;
        Vector2D pos = grid_center( i );
#else
        for ( int ix = 0; ix < ViewGridMap::GRID_X_SIZE; ++ix )
        {
            for ( int iy = 0; iy < ViewGridMap::GRID_Y_SIZE; ++iy )
            {
                int & value = M_impl->grid_map_[ix][iy];
                Vector2D pos = grid_center( ix, iy );
#endif

                double dist = view_area.origin().dist( pos );
                if ( dist > VISIBLE_DIST )
                {
                    AngleDeg angle = ( pos - view_area.origin() ).th();
                    if ( angle.isRightOf( left_angle )
                         && angle.isLeftOf( right_angle ) )
                    {
                        double angle_thr = AngleDeg::asin_deg( GRID_RADIUS / dist );
                        if ( ( angle - left_angle ).abs() > angle_thr
                             && ( angle - right_angle ).abs() > angle_thr )
                        {
                            value = 0;
                        }

                    }
                }
#ifdef DEBUG_PRINT_TEXT
#  ifdef USE_VECTOR
                dlog.addText( Logger::WORLD,
                              __FILE__" %05d (%04d %04d) pos=(%.2f %.2f) -> %d | count=%d",
                              i,
                              i / ViewGridMap::GRID_Y_SIZE,
                              i % ViewGridMap::GRID_Y_SIZE,
                              pos.x, pos.y,
                              grid_index( pos ),
                              *v );
                if ( grid_index( pos ) != i )
                {
                    dlog.addText( Logger::WORLD,
                                  __FILE__"---> index mismatch" );
                }
#  else
                dlog.addText( Logger::WORLD,
                              __FILE__" (%03d %03d) pos=(%.2f %.2f) -> (%d %d) | count=%d",
                              ix, iy,
                              pos.x, pos.y,
                              grid_x_index( pos.x ), grid_y_index( pos.y ),
                              value );
                if ( ix != grid_x_index( pos.x )
                     || iy != grid_y_index( pos.y ) )
                {
                    dlog.addText( Logger::WORLD,
                                  __FILE__"---> index mismatch" );
                }
#  endif
#endif
#ifdef DEBUG_PAINT_GRID_MAP
                // int vv = std::max( 0, 255 - *v * 20 );
                int vv = std::max( 0, 255 - value * 20 );
                dlog.addRect( Logger::WORLD,
                              pos.x - GRID_LENGTH*0.125, pos.y - GRID_LENGTH*0.125,
                              GRID_LENGTH*0.25, GRID_LENGTH*0.25,
                              vv, vv, vv,
                              true );
#endif
#ifndef USE_VECTOR
            }
        }
#else
    }
#endif

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::WORLD,
                  __FILE__" (update) PROFILE elapsed %f [ms] grid_size=%d",
                  timer.elapsedReal(),
#ifdef USE_VECTOR
                  M_impl->grid_map_.size()
#else
                  GRID_X_SIZE * GRID_Y_SIZE
#endif
                  );
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
int
ViewGridMap::seenCount( const Vector2D & pos )
{
#ifdef USE_VECTOR
    return M_impl->grid_map_.at( grid_index( pos ) );
    // try
    // {
    //     return M_impl->grid_map_.at( grid_index( pos ) );
    // }
    // catch ( std::exception & e )
    // {
    //     std::cerr << __FILE__ ": exception caught! " << e.what()
    //               << std::endl;
    //     return 1000;
    // }
#else
    return M_impl->grid_map_[grid_x_index( pos.x )][grid_y_index( pos.y )];
#endif
}

}
