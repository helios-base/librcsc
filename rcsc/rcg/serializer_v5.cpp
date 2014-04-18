// -*-c++-*-

/*!
  \file serializer_v5.cpp
  \brief v5 format rcg serializer Source File.
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

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "serializer_v5.h"

#include "util.h"

#include <cstring>
#include <cmath>

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV5::serializeHeader( std::ostream & os )
{
    return os << "ULG5\n";
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
SerializerV5::serialize( std::ostream & os,
                         const ShowInfoT & show )
{
    M_time = show.time_;

    os << "(show " << show.time_;

    // ball

    os << " ((b)"
       << ' ' << show.ball_.x_ << ' ' << show.ball_.y_;
    if ( show.ball_.hasVelocity() )
    {
        os << ' ' << show.ball_.vx_ << ' ' << show.ball_.vy_;
    }
    else
    {
        os << " 0 0";
    }
    os << ')';

    // players

    for ( int i = 0; i < MAX_PLAYER*2; ++i )
    {
        const PlayerT & p = show.player_[i];

        os << " ((" << p.side_ << ' ' << p.unum_ << ')';
        os << ' ' << p.type_;
        os << ' ' << std::hex << std::showbase
           << p.state_
           << std::dec << std::noshowbase;

        os << ' ' << p.x_ << ' ' << p.y_;
        if ( p.hasVelocity() )
        {
            os << ' ' << p.vx_ << ' ' << p.vy_;
        }
        else
        {
            os << " 0 0";
        }
        os << ' ' << p.body_
           << ' ' << ( p.hasNeck() ? p.neck_ : 0.0f );

        if ( p.isPointing() )
        {
            os << ' ' << p.point_x_ << ' ' << p.point_y_;
        }

        if ( p.hasView() )
        {
            os << " (v " << p.view_quality_ << ' ' << p.view_width_ << ')';
        }
        else
        {
            os << " (v h 90)";
        }

        if ( p.hasStamina() )
        {
            os << " (s " << p.stamina_
               << ' ' << p.effort_
               << ' ' << p.recovery_
               << ' ' << p.stamina_capacity_
               << ')';
        }
        else
        {
            os << " (s 4000 1 1 -1)";
        }

        if ( p.focus_side_ != 'n' )
        {
            os << " (f" << p.focus_side_ << ' ' << p.focus_unum_ << ')';
        }

        os << " (c"
           << ' ' << p.kick_count_
           << ' ' << p.dash_count_
           << ' ' << p.turn_count_
           << ' ' << p.catch_count_
           << ' ' << p.move_count_
           << ' ' << p.turn_neck_count_
           << ' ' << p.change_view_count_
           << ' ' << p.say_count_
           << ' ' << p.tackle_count_
           << ' ' << p.pointto_count_
           << ' ' << p.attentionto_count_
           << ')';
        os << ')';
    }

    os << ")\n";

    return os;
}


/*-------------------------------------------------------------------*/
/*!

*/
namespace {

Serializer::Ptr
create_v5()
{
    Serializer::Ptr ptr( new SerializerV5() );
    return ptr;
}

rcss::RegHolder v5 = Serializer::creators().autoReg( &create_v5, REC_VERSION_5 );

}

} // end of namespace
} // end of namespace
