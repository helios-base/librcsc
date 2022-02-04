// -*-c++-*-

/*!
  \file cycle_data.cpp
  \brief global cycle information class Source File
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

#include "cycle_data.h"

#include "global_visual_sensor.h"

#include <rcsc/param/server_param.h>
#include <rcsc/rcg/util.h>
#include <rcsc/rcg/types.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/game_mode.h>

#include <functional>
#include <algorithm>

namespace {

/*-------------------------------------------------------------------*/
/*!

*/
void
set_team( const rcsc::GlobalVisualSensor::PlayerCont & from,
          rcsc::CycleData::PlayerCont & to )
{
    to.reserve( from.size() );

    for ( const rcsc::GlobalVisualSensor::PlayerT & v : from )
    {
        to.push_back( rcsc::CycleData::PlayerT() );

        rcsc::CycleData::PlayerT & p = to.back();

        p.unum_ = v.unum_;

        p.pos_ = v.pos_;
        p.vel_ = v.vel_;
        p.body_ = v.body_;
        p.neck_ = v.neck_;

        if ( v.pointto_dist_ != 0.0 )
        {
            p.pointto_pos_
                = v.pos_
                + rcsc::Vector2D::polar2vector( v.pointto_dist_,
                                                v.pointto_angle_ );
        }

        p.goalie_ = v.goalie_;
        p.tackle_ = v.tackle_;
    }
}

}

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CycleData::PlayerT::print( std::ostream & os ) const
{
    if ( ! pos_.valid() )
    {
        return os;
    }

    os << '(' << unum_;

    if ( goalie_ )
    {
        os << " g";
    }

    os << ' ' << pos_ << ' ' << vel_
       << ' ' << body_ << ' ' << neck_;

    if ( pointto_pos_.valid() )
    {
        os << ' ' << pointto_pos_;
    }

    if ( tackle_ )
    {
        os << " tackle";
    }

    return os << ')';
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CycleData::assign( const GlobalVisualSensor & see_global,
                   const GameMode & game_mode,
                   const GameTime & current )
{
    M_time = current;
    M_game_mode = game_mode;

    M_ball.pos_ = see_global.ball().pos_;
    M_ball.vel_ = see_global.ball().vel_;

    M_players_left.clear();
    M_players_right.clear();

    set_team( see_global.playersLeft(), M_players_left );
    set_team( see_global.playersRight(), M_players_right );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
CycleData::getOffsideLineForLeft() const
{
    double first_x = 0.0, second_x = 0.0;
    for ( const PlayerT & p = M_players_right )
    {
        second_x = std::max( second_x, p.pos_.x );
        if ( first_x < second_x )
        {
            std::swap( first_x, second_x );
        }
    }

    return second_x;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
CycleData::getOffsideLineForRight() const
{
    double first_x = 0.0, second_x = 0.0;
    for ( const PlayerT & p : M_players_left )
    {
        second_x = std::min( second_x, p.pos_.x );
        if ( first_x > second_x )
        {
            std::swap( first_x, second_x );
        }
    }

    return second_x;
}

/*-------------------------------------------------------------------*/
/*!

*/
double
CycleData::getOffsideLineX( const SideID my_side ) const
{
    switch ( my_side ) {
    case LEFT:
        return getOffsideLineForLeft();
    case RIGHT:
        return getOffsideLineForRight();
    case NEUTRAL:
    default:
        break;
    }

    return getOffsideLineForLeft();
}

/*-------------------------------------------------------------------*/
/*!

*/
BallStatus
CycleData::getBallStatus() const
{
    static const double WIDTH
        = ServerParam::i().goalHalfWidth()
        + ServerParam::DEFAULT_GOAL_POST_RADIUS;
    static const Rect2D GOAL_L( ( - ServerParam::DEFAULT_PITCH_LENGTH * 0.5
                                  - ServerParam::DEFAULT_GOAL_DEPTH
                                  - ServerParam::i().ballSize() ),
                                - WIDTH * 0.5,
                                ServerParam::DEFAULT_GOAL_DEPTH,
                                WIDTH );
    static const Rect2D GOAL_R( ( ServerParam::DEFAULT_PITCH_LENGTH * 0.5
                                  + ServerParam::i().ballSize() ),
                                - WIDTH * 0.5,
                                ServerParam::DEFAULT_GOAL_DEPTH,
                                WIDTH );
    static const Rect2D PITCH( ( - ServerParam::DEFAULT_PITCH_LENGTH * 0.5
                                 - ServerParam::i().ballSize() * 0.5 ),
                               ( - ServerParam::DEFAULT_PITCH_WIDTH * 0.5
                                 - ServerParam::i().ballSize() * 0.5 ),
                               ( ServerParam::DEFAULT_PITCH_LENGTH
                                 + ServerParam::i().ballSize() ),
                               ( ServerParam::DEFAULT_PITCH_WIDTH
                                 + ServerParam::i().ballSize() ) );

    if ( GOAL_L.contains( M_ball.pos_ ) )
    {
        return Ball_GoalL ;
    }

    if ( GOAL_R.contains( M_ball.pos_ ) )
    {
        return Ball_GoalR ;
    }

    if ( ! PITCH.contains( M_ball.pos_ ) )
    {
        return Ball_OutOfField ;
    }

    return Ball_InField ;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
CycleData::PlayerT *
CycleData::getPlayerNearestTo( const PlayerCont & players,
                               const Vector2D & point ) const
{
    const PlayerT * result = nullptr;

    double min_dist = 100000.0;
    for ( const PlayerT & p : players )
    {
        double d2 = p.pos_.dist2( point );
        if ( d2 < min_dist )
        {
            min_dist = d2;
            result = &p;
        }
    }

    return result;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CycleData::print( std::ostream & os ) const
{
    os << "(b " << M_ball.pos_ << ' ' << M_ball.vel_ << ")\n";

    for ( const PlayerT & p : M_players_left )
    {
        os << "l ";
        p.print( os );
        os << '\n';
    }

    for ( const PlayerT & p : M_players_right )
    {
        os << "r ";
        p.print( os );
        os << '\n';
    }

    return os;
}

}
