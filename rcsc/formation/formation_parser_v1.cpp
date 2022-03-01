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

#include "sample_data.h"

#include <fstream>
#include <sstream>

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserV1::parse( std::istream & is )
{
    Formation::Ptr ptr;

    if ( ! parseHeader( is ) ) return ptr;
    if ( ! parseRoles( is ) ) return ptr;
    if ( ! parseData( is ) ) return ptr;

    return ptr;
}

/*-------------------------------------------------------------------*/
bool
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

        std::istringstream istr( line );

        std::string tag;
        std::string method_name;
        int ver = 0;

        istr >> tag;
        if ( tag != "Formation" )
        {
            std::cerr << "(FormationParserV1::parseHeader) unknown tag [" << tag << "]" << std::endl;
            return false;
        }

        istr >> method_name;
        if ( ! istr )
        {
            std::cerr << "(FormationParserV1::parseHeader) No method name" << std::endl;
            return false;
        }

        if ( istr >> ver )
        {
            if ( ver < 0 )
            {
                std::cerr << "(FormationParserV1::parseHeader) Illegas format version " << ver << std::endl;
                return false;
            }
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV1::parseRoles( std::istream & is )
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

        const char * msg = line.c_str();

        for ( int unum = 1; unum <= 11; ++unum )
        {
            char role_name[128];
            int position_pair;
            int n_read = 0;

            if ( std::sscanf( msg, " %s %d %n ",
                              role_name, &position_pair, &n_read ) != 2 )
            {
                std::cerr << "(FormationParserV1::parseRoles) Failed to read player " << unum << std::endl;
                return false;
            }
            msg += n_read;

            // const Formation::SideType type = ( symmetry_number == 0
            //                                    ? Formation::CENTER
            //                                    : symmetry_number < 0
            //                                    ? Formation::SIDE
            //                                    : Formation::SYMMETRY );
            // if ( type == Formation::CENTER )
            // {
            //     createNewRole( unum, role_name, type );
            // }
            // else if ( type == Formation::SIDE )
            // {
            //     createNewRole( unum, role_name, type );
            // }
            // else
            // {
            //     setSymmetryType( unum, symmetry_number, role_name );
            // }
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV1::parseData( std::istream & is )
{
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line == "End" )
        {
            return true;
        }

        //formation::SampleData new_sample;

        const char * msg = line.c_str();

        double read_x, read_y;
        int n_read = 0;

        // read ball pos
        if ( std::sscanf( msg, " %lf %lf %n ",
                          &read_x, &read_y, &n_read ) != 2 )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** Invalid ball data ["
                      << line << "]"
                      << std::endl;
            return false;
        }
        msg += n_read;

        //new_sample.ball_.assign( read_x, read_y );

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( std::sscanf( msg, " %lf %lf %n ",
                              &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr << __FILE__ << ':' << __LINE__ << ':'
                          << " *** ERROR *** Illegal player data. unum = "
                          << unum
                          << " [" << line << "]"
                          << std::endl;
                return false;
            }
            msg += n_read;

            //new_sample.players_.push_back( Vector2D( read_x, read_y ) );
        }

        //samples->addData( *this, new_sample, false );
    }

    return false;
}


}
