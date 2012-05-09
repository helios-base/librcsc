// -*-c++-*-

/*!
  \file view_area.h
  \brief player's view area type Header File
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

#ifndef RCSC_PLAYER_VIEW_AREA_H
#define RCSC_PLAYER_VIEW_AREA_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/angle_deg.h>
#include <rcsc/game_time.h>

#include <deque>
#include <cmath>

namespace rcsc {

/*!
  \class ViewArea
  \brief player's view area.
 */
class ViewArea {
private:
    double M_view_width; //!< the width of player's view area when see message is received.
    Vector2D M_origin; //!< estimated player's global position when see message is received.
    AngleDeg M_angle; //!< estimated player's head direction when see message is received.
    GameTime M_time; //!< the see message arrived time

public:

    /*!
      \brief construct invalid object
    */
    ViewArea()
        : M_view_width( -1.0 )
        , M_origin( Vector2D::INVALIDATED )
        , M_angle()
        , M_time( -1, 0 )
      { }

    /*!
      \brief construct invalid object
      \param t  game time
    */
    explicit
    ViewArea( const GameTime & t )
        : M_view_width( -1.0 )
        , M_origin( Vector2D::INVALIDATED )
        , M_angle()
        , M_time( t )
      { }


    /*!
      \brief construct with all variables
      \param view_width the width of player's view area
      \param origin the origin point of this view area
      \param angle the angle from origin point of this view area
      \param t time of this view area
     */
    ViewArea( const double & view_width,
              const Vector2D & origin,
              const AngleDeg & angle,
              const GameTime & t )
        : M_view_width( view_width )
        , M_origin( origin )
        , M_angle( angle )
        , M_time( t )
      { }

    /*!
      \brief get the width of view area
      \return  the width of view area (degree)
     */
    const
    double & viewWidth() const
      {
          return M_view_width;
      }

    /*!
      \brief get player's global position when see message is received.
      \return origin point (player's global position when see message is received.)
     */
    const
    Vector2D & origin() const
      {
          return M_origin;
      }

    /*!
      \brief get player's head angle when see message is received.
      \return player's head angle when see message is received.
     */
    const
    AngleDeg & angle() const
      {
          return M_angle;
      }

    /*!
      \brief get the game time when this information is generated
      \return game time
     */
    const
    GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief check if this object is valid or not
      \return the validity of the object
     */
    bool isValid() const
      {
          return M_view_width > 0.0;
      }

    /*!
      \brief check if point is contained by this view area or not
      \param point checked point
      \param dir_thr angle threshold value
      \param visible_dist2 squared visible distance value
     */
    bool contains( const Vector2D & point,
                   const double & dir_thr,
                   const double & visible_dist2 ) const
      {
          if ( ! isValid() )
          {
              return false;
          }

          Vector2D rpos( point - origin() );
          if ( rpos.r2() < visible_dist2 )
          {
              return true;
          }

          return ( rpos.th() - M_angle ).abs() < M_view_width*0.5 - dir_thr;
      }
};

//! typedef of the ViewArea container
typedef std::deque< ViewArea > ViewAreaCont;

}

#endif
