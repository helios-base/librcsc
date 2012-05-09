// -*-c++-*-

/*!
  \file serializer_v1.cpp
  \brief v1 format rcg serializer Source File.
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

#include "serializer_v1.h"

#include "util.h"

#include <cstring>
#include <cmath>

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serializeHeader( std::ostream & os )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const server_params_t & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serializeParam( std::ostream & os,
                              const std::string & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const player_params_t & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const player_type_t & )
{
    // nothing to do
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const dispinfo_t & disp )
{
    return serializeImpl( os, disp );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const showinfo_t & show )
{
    M_playmode = show.pmode;

    convert( show.team[0], M_teams[0] );
    convert( show.team[1], M_teams[1] );

    dispinfo_t disp1;

    disp1.mode = htons( SHOW_MODE );

    disp1.body.show = show;

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const showinfo_t2 & show2 )
{
    M_playmode = show2.pmode;

    convert( show2.team[0], M_teams[0] );
    convert( show2.team[1], M_teams[1] );

    dispinfo_t disp1;

    disp1.mode = htons( SHOW_MODE );

    convert( show2, disp1.body.show );

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const short_showinfo_t2 & show2 )
{
    dispinfo_t disp1;

    disp1.mode = htons( SHOW_MODE );

    convert( M_playmode,
             M_teams[0], M_teams[1],
             show2,
             disp1.body.show );

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const msginfo_t & msg )
{
    dispinfo_t disp1;

    disp1.mode = htons( MSG_MODE );

    disp1.body.msg = msg;

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const Int16 board,
                         const std::string & msg )
{
    dispinfo_t disp1;

    disp1.mode = htons( MSG_MODE );

    disp1.body.msg.board = board;
    std::memset( disp1.body.msg.message, 0,
                 sizeof( disp1.body.msg.message ) );
    std::strncpy( disp1.body.msg.message,
                  msg.c_str(),
                  std::min( sizeof( disp1.body.msg.message ) - 1,
                            msg.length() ) );

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const drawinfo_t & draw )
{
    dispinfo_t disp1;

    disp1.mode = htons( DRAW_MODE );

    disp1.body.draw = draw;

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
                         const char playmode )
{
    M_playmode = playmode;

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
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
SerializerV1::serialize( std::ostream & os,
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
SerializerV1::serialize( std::ostream & os,
                         const ShowInfoT & show )
{
    dispinfo_t disp1;

    disp1.mode = htons( SHOW_MODE );

    convert( M_playmode,
             M_teams[0], M_teams[1],
             show,
             disp1.body.show );

    return serializeImpl( os, disp1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SerializerV1::serialize( std::ostream & os,
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
create_v1()
{
    Serializer::Ptr ptr( new SerializerV1() );
    return ptr;
}

rcss::RegHolder v1 = Serializer::creators().autoReg( &create_v1, REC_OLD_VERSION );

}

} // end of namespace
} // end of namespace
