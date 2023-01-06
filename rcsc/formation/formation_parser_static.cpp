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

#include "formation_static.h"

#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserStatic::parseImpl( std::istream & is )
{
    Formation::Ptr ptr( new FormationStatic() );

    if ( ! parseHeader( is, ptr ) ) return Formation::Ptr();
    if ( ! parseData( is, ptr ) ) return Formation::Ptr();

    if ( ! checkRoleNames( ptr ) ) return Formation::Ptr();

    return ptr;
}

/*-------------------------------------------------------------------*/
bool
FormationParserStatic::parseHeader( std::istream & is,
                                    Formation::Ptr result )
{
    if ( ! result ) return false;

    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        break;
    }

    char method_name[32];
    if ( std::sscanf( line.c_str(), "Formation %s", method_name ) != 1 )
    {
        std::cerr << "(FormationParserStatic::parseHeader) ERROR: Illegal header [" << line << "]" << std::endl;
        return false;
    }

    if ( result->methodName() != method_name )
    {
        std::cerr << "(FormationParserStatic::parseHeader) ERROR: Unsupported method name " << method_name << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserStatic::parseData( std::istream & is,
                                  Formation::Ptr result )
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

    FormationData formation_data;

    std::string err = formation_data.addData( data );
    if ( ! err.empty() )
    {
        std::cerr << "(FormationParserStatic::parseData) ERROR: " << err << std::endl;
        return false;
    }

    return result->train( formation_data );
}


}
