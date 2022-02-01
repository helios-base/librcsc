// -*-c++-*-

/*!
  \file serializer.cpp
  \brief rcg serializer Source File.
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

#include "serializer.h"

#include "util.h"

#include "serializer_v1.h"
#include "serializer_v2.h"
#include "serializer_v3.h"
#include "serializer_v4.h"
#include "serializer_v5.h"


#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>

namespace rcsc {
namespace rcg {


/*-------------------------------------------------------------------*/
/*!

*/
Serializer::Creators &
Serializer::creators()
{
    static Creators s_instance;
    return s_instance;
}


/*-------------------------------------------------------------------*/
/*!

*/
Serializer::Ptr
Serializer::create( const int version )
{
    Serializer::Ptr ptr;

    Serializer::Creator creator;
    if ( Serializer::creators().getCreator( creator, version ) )
    {
        ptr = creator();
    }
    else if ( version == REC_VERSION_5 ) ptr = Serializer::Ptr( new SerializerV5() );
    else if ( version == REC_VERSION_4 ) ptr = Serializer::Ptr( new SerializerV4() );
    else if ( version == REC_VERSION_3 ) ptr = Serializer::Ptr( new SerializerV3() );
    else if ( version == REC_VERSION_2 ) ptr = Serializer::Ptr( new SerializerV2() );
    else if ( version == REC_OLD_VERSION ) ptr = Serializer::Ptr( new SerializerV1() );

    return ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
Serializer::Serializer()
    : M_playmode( static_cast< char >( 0 ) )
{
    for ( int i = 0; i < 2; ++i )
    {
        M_teams[i].clear();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const int version )
{
    if ( version == REC_OLD_VERSION )
    {
        // v1 protocl does not have header.
        return os;
    }

    if ( version >= REC_VERSION_4 )
    {
        os << "ULG" << version << '\n';
    }
    else
    {
        char buf[5];

        buf[0] = 'U';
        buf[1] = 'L';
        buf[2] = 'G';
        buf[3] = static_cast< char >( version );

        os.write( buf, 4 );
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const server_params_t & param )
{
    Int16 mode = htons( PARAM_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &param ),
              sizeof( server_params_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const player_params_t & pparam )
{
    Int16 mode = htons( PPARAM_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &pparam ),
              sizeof( player_params_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const player_type_t & type )
{
    Int16 mode = htons( PT_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &type ),
              sizeof( player_type_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const team_t & team_l,
                           const team_t & team_r )
{
    Int16 mode = htons( TEAM_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &team_l ),
              sizeof( team_t ) );

    os.write( reinterpret_cast< const char * >( &team_r ),
              sizeof( team_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const char pmode )
{
    Int16 mode = htons( PM_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &pmode ),
              sizeof( char ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const PlayMode pmode )
{
    Int16 mode = htons( PM_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    char pm = static_cast< char >( pmode );

    os.write( reinterpret_cast< char * >( &pm ),
              sizeof( char ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const dispinfo_t & disp )
{
    os.write( reinterpret_cast< const char * >( &disp ),
              sizeof( dispinfo_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const showinfo_t & show )
{
    Int16 mode = htons( SHOW_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &show ),
              sizeof( showinfo_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const showinfo_t2 & show2 )
{
    if ( M_playmode != show2.pmode )
    {
        M_playmode = show2.pmode;

        serializeImpl( os, show2.pmode );
    }

    if ( M_teams[0].name_.length() != std::strlen( show2.team[0].name )
         || M_teams[0].name_ != show2.team[0].name
         || M_teams[0].score_ != ntohs( show2.team[0].score )
         || M_teams[1].name_.length() != std::strlen( show2.team[1].name )
         || M_teams[1].name_ != show2.team[1].name
         || M_teams[1].score_ != ntohs( show2.team[1].score ) )
    {
        convert( show2.team[0], M_teams[0] );
        convert( show2.team[1], M_teams[1] );

        serializeImpl( os, show2.team[0], show2.team[1] );
    }

    short_showinfo_t2 short_show2;

    short_show2.ball = show2.ball;

    for ( int i = 0; i < MAX_PLAYER * 2; ++i )
    {
        short_show2.pos[i] = show2.pos[i];
    }

    short_show2.time = show2.time;

    serializeImpl( os, short_show2 );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const short_showinfo_t2 & show2 )
{
    Int16 mode = htons( SHOW_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &show2 ),
              sizeof( short_showinfo_t2 ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const msginfo_t & msg )
{
    Int16 mode = htons( MSG_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &msg.board ),
              sizeof( rcsc::rcg::Int16 ) );

    rcsc::rcg::Int16 len = 1;
    while ( msg.message[len-1] != '\0'
            && len < 2048 )
    {
        ++len;
    }

    rcsc::rcg::Int16 nlen = htons( len );

    os.write( reinterpret_cast< const char* >( &nlen ),
              sizeof( rcsc::rcg::Int16 ) );

    os.write( reinterpret_cast< const char * >( msg.message ),
              len );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const drawinfo_t & draw )
{
    Int16 mode = htons( DRAW_MODE );

    os.write( reinterpret_cast< char * >( &mode ),
              sizeof( Int16 ) );

    os.write( reinterpret_cast< const char * >( &draw ),
              sizeof( drawinfo_t ) );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
Serializer::serializeImpl( std::ostream & os,
                           const dispinfo_t2 & disp2 )
{
    switch ( ntohs( disp2.mode ) ) {
    case SHOW_MODE:
        serializeImpl( os, disp2.body.show );
        break;
    case MSG_MODE:
        serializeImpl( os, disp2.body.msg );
        break;
    case DRAW_MODE:
        //serializeImpl( os, disp2.body.draw );
        break;
    case BLANK_MODE:
        break;
    case PM_MODE:
        serializeImpl( os, disp2.body.show.pmode );
        break;
    case TEAM_MODE:
        serializeImpl( os,
                       disp2.body.show.team[0],
                       disp2.body.show.team[1] );
        break;
    case PT_MODE:
        serializeImpl( os, disp2.body.ptinfo );
        break;
    case PARAM_MODE:
        serializeImpl( os, disp2.body.sparams );
        break;
    case PPARAM_MODE:
        serializeImpl( os, disp2.body.pparams );
        break;
    default:
        break;
    }

    return os;
}

} // end of namespace
} // end of namespace
