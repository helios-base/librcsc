// -*-c++-*-

/*!
  \file formation_parser_v2.cpp
  \brief v2 formation parser class Source File.
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

#include "formation_parser_v2.h"

#include "formation_data.h"

#include <sstream>

namespace rcsc {

namespace {
inline
double
round_coord( const double val )
{
    return rint( val / FormationData::PRECISION ) * FormationData::PRECISION;
}
}

/*-------------------------------------------------------------------*/
FormationData::Ptr
FormationParserV2::parse( std::istream & is )
{
    FormationData::Ptr ptr( new FormationData() );

    if ( ! parseHeader( is ) ) return FormationData::Ptr();
    if ( ! parseRoles( is, ptr ) ) return FormationData::Ptr();
    if ( ! parseData( is, ptr ) ) return FormationData::Ptr();

    if ( ! checkRoleNames( ptr ) ) return FormationData::Ptr();
    if ( ! checkPositionPair( ptr ) ) return FormationData::Ptr();

    return ptr;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV2::parseHeader( std::istream & is )
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
            std::cerr << "(FormationParserV2::parseHeader) unknown tag [" << tag << "]" << std::endl;
            return false;
        }

        istr >> method_name;
        if ( ! istr )
        {
            std::cerr << "(FormationParserV2::parseHeader) No method name" << std::endl;
            return false;
        }

        if ( istr >> ver )
        {
            if ( ver != 2 )
            {
                std::cerr << "(FormationParserV2::parseHeader) Illegas format version " << ver << std::endl;
                return false;
            }
        }

        return true;
    }

    return false;
}


/*-------------------------------------------------------------------*/
bool
FormationParserV2::parseRoles( std::istream & is,
                               FormationData::Ptr result )
{
    if ( ! result ) return false;

    std::string line;

    //
    // read Begin tag
    //

    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( line != "Begin Roles" )
        {
            std::cerr << "(FormationParserV2::parseRoles) ERROR: "
                      << "Illegal header [" << line << ']' << std::endl;
            return false;
        }

        break;
    }

    //
    // read role data
    //

    for ( int unum = 1; unum <= 11; ++unum )
    {
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

        int read_unum = 0;
        char role_name[128];
        int paired_unum = 0;

        if ( std::sscanf( line.c_str(),
                          " %d %127s %d ",
                          &read_unum, role_name, &paired_unum ) != 3
             || read_unum != unum )
        {
            std::cerr << "(FormationParserV2::parseRoles) ERROR: "
                      << "Illegal role data. unum=" << unum << " [" << line << "]" << std::endl;
            return false;
        }


        if ( ! result->setRoleName( unum, role_name ) )
        {
            return false;
        }

        if ( ! result->setPositionPair( unum, paired_unum ) )
        {
            return false;
        }
    }

    //
    // read End tag
    //

    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( line != "End Roles" )
        {
            std::cerr << "(FormationParserV2::parseRoles) ERROR: "
                      << "Illegal End tag " << std::endl;
            return false;
        }

        break;
    }

    return true;
}


/*-------------------------------------------------------------------*/
bool
FormationParserV2::parseData( std::istream & is,
                              FormationData::Ptr result )
{
    if ( ! result ) return false;

    int data_size = 0;
    if ( ! parseDataHeader( is, &data_size ) )
    {
        return false;
    }

    for ( int i = 0; i < data_size; ++i )
    {
        if ( ! parseOneData( is, i, result ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV2::parseDataHeader( std::istream & is,
                                    int * data_size )
{
    std::string line;
    int n_line = 0;

    // read data header
    while ( std::getline( is, line ) )
    {
        ++n_line;
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( ! line.compare( 0, 13, "Begin Samples" ) )
        {
            int version = 0;
            int n_val = std::sscanf( line.c_str(),
                                     " Begin Samples %d %d ",
                                     &version, data_size );
            if ( n_val != 2 )
            {
                std::cerr << "(FormationParserV2::parseData) ERROR: "
                          << " Illegal data header [" << line << "]" << std::endl;
                return false;
            }

            if ( version != 2 )
            {
                std::cerr << "(FormationParserV2::parseData) ERROR: "
                          << " Illegal data version [" << version << "]" << std::endl;
                return false;
            }
        }

        break;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV2::parseOneData( std::istream & is,
                                 const int index,
                                 FormationData::Ptr result )
{
    std::string line;

    //
    // read index
    //
    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        int read_index = -1;
        if ( std::sscanf( line.c_str(),
                          "----- %d -----",
                          &read_index ) != 1
             || read_index != index )
        {
            std::cerr << "(FormationParaserV2::parseOneData) ERROR: Illegal data segment. "
                      << " index=" << index << " [" << line << ']' << std::endl;
            return false;
        }

        break;
    }


    //
    // read new data.
    //

    FormationData::Data new_data;

    double read_x = 0.0;
    double read_y = 0.0;

    //
    // read ball data
    //
    if ( ! std::getline( is, line ) )
    {
        std::cerr << "(FormationParserV2::parseOneData) ERROR: failed to read ball data."
                  << " index=" << index << std::endl;
        return false;
    }

    if ( std::sscanf( line.c_str(),
                      " Ball %lf %lf ",
                      &read_x, &read_y ) != 2 )
    {
        std::cerr << "(FormationParserV2::parseOneData) ERROR: Illegal ball data."
                  << " index=" << index << " [" << line << "]" << std::endl;
        return false;
    }

    new_data.ball_.assign( round_coord( read_x ), round_coord( read_y ) );

    //
    // read player data
    //

    int read_unum = 0;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        if ( ! std::getline( is, line ) )
        {
            std::cerr << "(FormationParserV2::parseOneData) ERROR: failed to read a line."
                      << " index" << index << " unum=" << unum << std::endl;
            return false;
        }

        if ( std::sscanf( line.c_str(),
                          " %d %lf %lf ",
                          &read_unum, &read_x, &read_y ) != 3
             || read_unum != unum )
        {
            std::cerr << "(FormationParserV2::parseOneData) ERROR: Illegal player data."
                      << " index=" << index << " unum=" << unum << " [" << line << "]" << std::endl;
            return false;
        }

        new_data.players_.emplace_back( round_coord( read_x ), round_coord( read_y ) );
    }

    const std::string err = result->addData( new_data );
    if ( ! err.empty() )
    {
        std::cerr << "(FormationParserV2::parseOneData) ERROR: " << err << std::endl;
        return false;
    }

    return true;
}

}
