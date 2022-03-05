// -*-c++-*-

/*!
  \file formation_parser.cpp
  \brief abstract formation parser class Source File.
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

#include "formation_parser.h"

#include <fstream>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
FormationData::Ptr
FormationParser::parse( const std::string & filepath )
{
    std::ifstream fin( filepath.c_str() );

    return parse( fin );
}

/*-------------------------------------------------------------------*/
bool
FormationParser::checkRoleNames( const FormationData::ConstPtr ptr )
{
    if ( ! ptr )
    {
        return false;
    }

    for ( size_t i = 0; i < ptr->roleNames().max_size(); ++i )
    {
        if ( ptr->roleNames()[i].empty() )
        {
            std::cerr << "(FormationParser::checkRoleNames) empty role name. unum= " << i + 1 << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParser::checkPositionPair( const FormationData::ConstPtr ptr )
{
    if ( ! ptr )
    {
        return false;
    }

    for ( size_t i = 0; i < ptr->positionPairs().max_size(); ++i )
    {

    }

    return true;
}

}

#include "formation_parser_v1.h"
#include "formation_parser_v2.h"
#include "formation_parser_v3.h"
#include "formation_parser_static.h"
#include "formation_parser_csv.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
FormationParser::Ptr
FormationParser::create( std::string & filepath )
{
    FormationParser::Ptr ptr;

    std::ifstream fin( filepath.c_str() );
    std::string line;
    if ( ! std::getline( fin, line ) ) return ptr;

    // {
    //     std::string::size_type first = line.find_first_not_of( ' ' );
    //     if ( first != line.std::string::npos
    //          && line[first] == '{' )
    //     {
    //         ptr = FormationParser::Ptr( new FormationParserJSON() );
    //     }
    // }

    char method_name[32];

    if ( std::sscanf( line.c_str(), " Formation , %31[^,] ", method_name ) == 1 )
    {
        ptr = FormationParser::Ptr( new FormationParserCSV() );
        return ptr;
    }

    int ver = 0;
    int n = std::sscanf( line.c_str(), " Formation %31[^,] %d ", method_name, &ver );
    if ( n == 2 )
    {
        if ( ver == 3 ) ptr = FormationParser::Ptr( new FormationParserV3() );
        if ( ver == 2 ) ptr = FormationParser::Ptr( new FormationParserV2() );
        if ( ver == 1 ) ptr = FormationParser::Ptr( new FormationParserV1() );
    }
    else if ( n == 1 )
    {
        if ( std::strncmp( method_name, "Static", std::strlen( "Static" ) ) == 0 )
        {
            ptr = FormationParser::Ptr( new FormationParserStatic() );
        }
        else
        {
            ptr = FormationParser::Ptr( new FormationParserV2() );
        }
    }

    return ptr;
}

}
