// -*-c++-*-

/*!
  \file formation_parser_static.cpp
  \brief v1 static formation parser class Source File.
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

#include "formation_parser_static.h"

#include "formation_data.h"

#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
FormationData::Ptr
FormationParserStatic::parse( std::istream & is )
{
    FormationData::Ptr ptr( new FormationData() );

    if ( ! parseHeader( is ) ) return FormationData::Ptr();
    if ( ! parseData( is, ptr ) ) return FormationData::Ptr();

    if ( ! checkRoleNames( ptr ) ) return FormationData::Ptr();

    return ptr;
}

/*-------------------------------------------------------------------*/
bool
FormationParserStatic::parseHeader( std::istream & is )
{
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        char method_name[32];

        if ( std::sscanf( line.c_str(), "Formation %s", method_name ) != 1 )
        {
            std::cerr << "(FormationParserStatic::parseHeader) ERROR: Illegal header [" << line << "]" << std::endl;
            return false;
        }

        if ( std::strncmp( method_name, "Static", std::strlen( "Static" ) ) != 0 )
        {
            std::cerr << "(FormationParserStatic::parseHeader) ERROR: Unknown method name [" << line << "]" << std::endl;
            return true;
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FormationParserStatic::parseData( std::istream & is,
                                  FormationData::Ptr result )
{
    if ( ! result ) return false;

    FormationData::Data data;
    data.ball_.assign( 0.0, 0.0 );

    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        int unum = 0;
        char role_name[32];
        double read_x, read_y;

        // read ball pos
        if ( std::sscanf( line.c_str(), " %d %31s %lf %lf ",
                          &unum, role_name, &read_x, &read_y ) != 4 )
        {
            std::cerr << "(FormationParserStatic::parseData) ERROR: Illegal data. "
                      <<  '[' << line << ']' << std::endl;
            return false;
        }

        if ( ! result->setRoleName( unum, role_name ) )
        {
            std::cerr << "(FormationParserStatic::parseData) ERROR: Illegal role name. "
                      <<  '[' << line << ']' << std::endl;
            return false;
        }

        data.players_.emplace_back( read_x, read_y );
    }

    if ( data.players_.size() != 11 )
    {
        std::cerr << "(FormationParserStatic::parseData) ERROR: Illegal player size" << std::endl;
        return false;
    }

    std::string err = result->addData( data );
    if ( ! err.empty() )
    {
        std::cerr << "(FormationParserStatic::parseData) ERROR: " << err << std::endl;
        return false;
    }

    return false;
}


}
