// -*-c++-*-

/*!
  \file serializer_v2.cpp
  \brief v2 format rcg serializer Source File.
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

#include "serializer_v2.h"

#include "util.h"

#include <cstring>
#include <cmath>

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serializeHeader( std::ostream & os )
{
    return serializeImpl( os, REC_VERSION_2 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serializeParam( std::ostream & os,
                              const std::string & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const server_params_t & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const player_params_t & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const player_type_t & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const dispinfo_t & disp )
{
    switch ( ntohs( disp.mode ) ) {
    case SHOW_MODE:
        serialize( os, disp.body.show );
        break;
    case MSG_MODE:
        serialize( os, disp.body.msg );
        break;
    case DRAW_MODE:
        serialize( os, disp.body.draw );
        break;
    default:
        break;
    }

   return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const showinfo_t & show )
{
    M_playmode = show.pmode;

    convert( show.team[0], M_teams[0] );
    convert( show.team[1], M_teams[1] );

    return serializeImpl( os, show );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const showinfo_t2 & show2 )
{
    M_playmode = show2.pmode;

    convert( show2.team[0], M_teams[0] );
    convert( show2.team[1], M_teams[1] );

    showinfo_t show1;

    convert( show2, show1 );

    return serializeImpl( os, show1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const short_showinfo_t2 & show2 )
{
    showinfo_t show1;

    convert( M_playmode,
             M_teams[0], M_teams[1],
             show2,
             show1 );

    return serializeImpl( os, show1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const msginfo_t & msg )
{
    return serializeImpl( os, msg );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const Int16 board,
                         const std::string & msg )
{
    msginfo_t info;

    info.board = board;
    std::memset( info.message, 0, sizeof( info.message ) );
    std::strncpy( info.message, msg.c_str(),
                  std::min( sizeof( info.message ) - 1, msg.length() ) );

    return serializeImpl( os, info );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const drawinfo_t & draw )
{
    return serializeImpl( os, draw );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const char playmode )
{
    M_playmode = playmode;

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const team_t & team_l,
                         const team_t & team_r )
{
    convert( team_l, M_teams[0] );
    convert( team_r, M_teams[1] );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const TeamT & team_l,
                         const TeamT & team_r )
{
    M_teams[0] = team_l;
    M_teams[1] = team_r;

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const ShowInfoT & show )
{
    showinfo_t show1;

    convert( M_playmode,
             M_teams[0], M_teams[1],
             show,
             show1 );

    return serializeImpl( os, show1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV2::serialize( std::ostream & os,
                         const DispInfoT & disp )
{
    M_playmode = static_cast< char >( disp.pmode_ );
    M_teams[0] = disp.team_[0];
    M_teams[1] = disp.team_[1];

    return serialize( os, disp.show_ );
}


/*-------------------------------------------------------------------*/
/*!

*/
namespace {

Serializer::Ptr
create_v2()
{
    Serializer::Ptr ptr( new SerializerV2() );
    return ptr;
}

rcss::RegHolder v2 = Serializer::creators().autoReg( &create_v2, REC_VERSION_2 );

}

} // end of namespace
} // end of namespace
