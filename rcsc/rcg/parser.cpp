// -*-c++-*-

/*!
  \file parser.cpp
  \brief abstract rcg parser class Source File.
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

#include "parser.h"

#include "types.h"

#include "parser_v1.h"
#include "parser_v2.h"
#include "parser_v3.h"
#include "parser_v4.h"
//#include "parser_v5.h"

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

*/
Parser::Creators &
Parser::creators()
{
    static Creators s_instance;
    return s_instance;
}


/*-------------------------------------------------------------------*/
/*!

*/
Parser::Ptr
Parser::create( std::istream & is )
{
    char header[5];
    int version = REC_OLD_VERSION;

    is.read( header, 4 ); // read 'U', 'L', 'G', <version>

    if ( is.gcount() != 4 )
    {
        std::cerr << "(rcsc::rcg::Parser::create) no header." << std::endl;
        return Parser::Ptr();
    }

    if ( header[0] == 'J'
         && header[1] == 'S'
         && header[2] == 'O'
         && header[3] == 'N' )
    {
        std::cerr << "(rcsc::rcg::Parser::create) JSON not supported." << std::endl;
        return Parser::Ptr();
    }

    if ( header[0] == 'U'
         && header[1] == 'L'
         && header[2] == 'G' )
    {
        version = static_cast< int >( header[3] );
    }

    std::cerr << "(rcsc::rcg::Parser::create) rcg version = "
              << ( version == static_cast< int >( '0' ) + REC_VERSION_6 ? REC_VERSION_6
                   : version == static_cast< int >( '0' ) + REC_VERSION_5 ? REC_VERSION_5
                   : version == static_cast< int >( '0' ) + REC_VERSION_4 ? REC_VERSION_4
                   : version )
              << std::endl;

    Parser::Ptr ptr;
    Parser::Creator creator;
    if ( Parser::creators().getCreator( creator, version ) )
    {
        ptr = creator();
    }
    else if ( version == static_cast< int >( '0' ) + REC_VERSION_6 ) ptr = Parser::Ptr( new ParserV4() );
    else if ( version == static_cast< int >( '0' ) + REC_VERSION_5 ) ptr = Parser::Ptr( new ParserV4() );
    else if ( version == static_cast< int >( '0' ) + REC_VERSION_4 ) ptr = Parser::Ptr( new ParserV4() );
    else if ( version == REC_VERSION_3 ) ptr = Parser::Ptr( new ParserV3() );
    else if ( version == REC_VERSION_2 ) ptr = Parser::Ptr( new ParserV2() );
    else if ( version == REC_OLD_VERSION ) ptr = Parser::Ptr( new ParserV1() );

    return ptr;
}

}
}
