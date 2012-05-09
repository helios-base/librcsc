// -*-c++-*-

/*!
  \file view_grid_map.h
  \brief field grid map scored by see info Header File
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

#ifndef RCSC_PLAYER_VIEW_GRID_MAP_H
#define RCSC_PLAYER_VIEW_GRID_MAP_H

#include <boost/scoped_ptr.hpp>

namespace rcsc {

class GameTime;
class Vector2D;
class ViewArea;

/*!
  \class ViewGridMap
  \brief grid map that stores field accuracy information
 */
class ViewGridMap {
private:

    struct Impl;

    boost::scoped_ptr< Impl > M_impl;

public:

    static const double GRID_LENGTH;

    static const double PITCH_MIN_X;
    static const double PITCH_MAX_X;
    static const double PITCH_LENGTH;

    static const double PITCH_MIN_Y;
    static const double PITCH_MAX_Y;
    static const double PITCH_WIDTH;

    static const int GRID_X_SIZE;
    static const int GRID_Y_SIZE;
    static const double GRID_RADIUS;
    static const double GRID_RADIUS2;

public:

    /*!
      \brief default constructor
    */
    ViewGridMap();

    /*!
      \brief destructor.
     */
    ~ViewGridMap();

    /*!
      \brief simply increment all grid values.
     */
    void incrementAll();

    /*!
      \brief update by new see info
     */
    void update( const GameTime & time,
                 const ViewArea & view_area );

    /*!
      \brief get the count since last observation
      \param pos target point
      \return count value
     */
    int seenCount( const Vector2D & pos );

};

}

#endif
