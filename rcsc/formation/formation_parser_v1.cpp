// -*-c++-*-

/*!
  \file formation_parser_v1.cpp
  \brief v1 formation parser class Source File.
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

#include "formation_parser_v1.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserV1::parseImpl( std::istream & is )
{
    const std::string method = parseHeader( is );

    Formation::Ptr ptr = Formation::create( method );
    if ( ! ptr )
    {
        std::cerr << "(FormationParserV1::parse) Could not create the formation " << method << std::endl;
        return Formation::Ptr();
    }

    if ( ! parseRoles( is, ptr ) ) return Formation::Ptr();
    if ( ! parseData( is, ptr ) ) return Formation::Ptr();

    if ( ! checkRoleNames( ptr ) ) return Formation::Ptr();
    if ( ! checkPositionPair( ptr ) ) return Formation::Ptr();

    return ptr;
}

/*-------------------------------------------------------------------*/
std::string
FormationParserV1::parseHeader( std::istream & is )
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

        break;
    }

    char method_name[32];
    int ver = 0;

    int n = std::sscanf( line.c_str(), "Formation %31s %d", method_name, &ver );

    if ( n < 1 )
    {
        std::cerr << "(FormationParserV1::parseHeader) No method name" << std::endl;
        return std::string();
    }

    if ( n == 2
         && ver != 1 )
    {
        std::cerr << "(FormationParserV1::parseHeader) Illegas format version " << ver << std::endl;
        return std::string();
    }

    return method_name;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV1::parseRoles( std::istream & is,
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

    const char * msg = line.c_str();

    for ( int unum = 1; unum <= 11; ++unum )
    {
        char role_name[128];
        int paired_unum;
        int n_read = 0;

        if ( std::sscanf( msg, " %s %d %n ",
                          role_name, &paired_unum, &n_read ) != 2 )
        {
            std::cerr << "(FormationParserV1::parseRoles) Failed to read player " << unum << std::endl;
            return false;
        }
        msg += n_read;

        if ( ! result->setRoleName( unum, role_name ) )
        {
            return false;
        }

        if ( ! result->setPositionPair( unum, paired_unum ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV1::parseData( std::istream & is,
                              Formation::Ptr result )
{
    if ( ! result ) return false;

    FormationData formation_data;

    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line == "End" )
        {
            return true;
        }

        //formation::SampleData new_sample;

        const char * msg = line.c_str();

        FormationData::Data new_data;

        double read_x, read_y;
        int n_read = 0;

        // read ball pos
        if ( std::sscanf( msg, " %lf %lf %n ",
                          &read_x, &read_y, &n_read ) != 2 )
        {
            std::cerr << "(FormationParserV1::parseData) ERROR: Invalid ball data. "
                      <<  '[' << line << ']' << std::endl;
            return false;
        }
        msg += n_read;

        new_data.ball_.assign( read_x, read_y );

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( std::sscanf( msg, " %lf %lf %n ",
                              &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr <<  "(FormationParserV1::parseData) ERROR: Illegal player data. "
                          << "unum = " << unum << " [" << line << "]" << std::endl;
                return false;
            }
            msg += n_read;

            new_data.players_.emplace_back( read_x, read_y );
        }

        std::string err = formation_data.addData( new_data );
        if ( ! err.empty() )
        {
            std::cerr << "(FormationParserV1::parseData) ERROR: " << err << std::endl;
            return false;
        }
    }

    return result->train( formation_data );
}


}
