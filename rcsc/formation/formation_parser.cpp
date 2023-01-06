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
Formation::Ptr
FormationParser::parse( const std::string & filepath )
{
    FormationParser::Ptr parser = create( filepath );

    if ( ! parser )
    {
        std::cerr << "ERROR (FormationParser::parse) could not create the formation parser instance."
                  << std::endl;
        return Formation::Ptr();
    }

    std::ifstream fin( filepath );
    return parser->parseImpl( fin );
}

/*-------------------------------------------------------------------*/
bool
FormationParser::checkRoleNames( const Formation::ConstPtr ptr )
{
    if ( ! ptr )
    {
        std::cerr << "(FormationParser::checkRoleNames) null ptr " << std::endl;
        return false;
    }

    int num = 0;
    for ( const std::string & name : ptr->roleNames() )
    {
        ++num;
        if ( name.empty() )
        {
            std::cerr << "(FormationParser::checkRoleNames) empty role name. num= " << num << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParser::checkPositionPair( const Formation::ConstPtr ptr )
{
    if ( ! ptr )
    {
        std::cerr << "(FormationParser::checkPositionPairs) null ptr " << std::endl;
        return false;
    }

    const std::array< int, 11 > & pairs = ptr->positionPairs();

    for ( size_t i = 0; i < pairs.size(); ++i )
    {
        if ( 1 <= pairs[i] && pairs[i] <= 11 )
        {
            const int p = pairs[ pairs[i] - 1 ];
            if ( p != -1
                 && p != static_cast< int >( i + 1 ) )
            {
                std::cerr << "(FormationParser::checkPositionPairs) illegal pair "
                          << " pair=" << pairs[i] << " - " << p
                          << std::endl;
                return false;
            }
        }
    }

    return true;
}

}

#include "formation_parser_v1.h"
#include "formation_parser_v2.h"
#include "formation_parser_v3.h"
#include "formation_parser_static.h"
#include "formation_parser_csv.h"
#include "formation_parser_json.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
FormationParser::Ptr
FormationParser::create( const std::string & filepath )
{
    FormationParser::Ptr ptr;

    std::ifstream fin( filepath.c_str() );
    std::string line;
    while ( std::getline( fin, line ) )
    {
        if ( line.empty() ) continue;
        if ( line[0] == '#' ) continue;
        break;
    }

    // std::cerr << "(FormationParser::create) first line = [" << line << "]" << std::endl;
    {
        std::string::size_type first = line.find_first_not_of( ' ' );
        if ( first != line.std::string::npos
             && line[first] == '{' )
        {
            // std::cerr << "(FormationParser::create) JSON" << std::endl;
            ptr = FormationParser::Ptr( new FormationParserJSON() );
            return ptr;
        }
    }

    char method_name[32];

    if ( std::sscanf( line.c_str(), " Method , %31[^,] ", method_name ) == 1 )
    {
        // std::cerr << "(FormationParser::create) CSV" << std::endl;
        ptr = FormationParser::Ptr( new FormationParserCSV() );
        return ptr;
    }

    int ver = 0;
    int n = std::sscanf( line.c_str(), " Formation %31s %d ", method_name, &ver );

    if ( n == 2 )
    {
        // std::cerr << "(FormationParser::create) method =" << method_name << " ver = " << ver << std::endl;
        if ( ver == 3 ) ptr = FormationParser::Ptr( new FormationParserV3() );
        if ( ver == 2 ) ptr = FormationParser::Ptr( new FormationParserV2() );
        if ( ver == 1 ) ptr = FormationParser::Ptr( new FormationParserV1() );
    }
    else if ( n == 1 )
    {
        // std::cerr << "(FormationParser::create) method =" << method_name << std::endl;
        if ( std::strncmp( method_name, "Static", std::strlen( "Static" ) ) == 0 )
        {
            ptr = FormationParser::Ptr( new FormationParserStatic() );
        }
        else
        {
            ptr = FormationParser::Ptr( new FormationParserV2() );
        }
    }
    // else
    // {
    //     std::cerr << "(FormationParser::create) unknown" << std::endl;
    // }

    return ptr;
}

}
