// -*-c++-*-

/*!
  \file formation_parser_csv.cpp
  \brief csv formation parser class Source File.
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

#include "formation_parser_csv.h"

#include "formation_static.h"

#include <cstring>

namespace rcsc {

namespace {

/*-------------------------------------------------------------------*/
std::string
get_value_line( std::istream & is )
{
    std::string buf;
    while ( std::getline( is, buf ) )
    {
        if ( buf.empty()
             || buf.front() == '#' )
        {
            buf.clear();
            continue;
        }
        break;
    }

    return buf;
}

}

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserCSV::parseImpl( std::istream & is )
{
    const std::string method = parseMethodName( is );

    Formation::Ptr ptr = Formation::create( method );
    if ( ! ptr )
    {
        std::cerr << "(FormationParserCSV::parse) Could not create the formation " << method << std::endl;
        return Formation::Ptr();
    }

    if ( ! parseRoleNumbers( is ) ) return Formation::Ptr();
    if ( ! parseRoleNames( is, ptr ) ) return Formation::Ptr();

    if ( method == FormationStatic::NAME )
    {
        if ( ! parseStaticPositions( is, ptr ) ) return Formation::Ptr();
    }
    else
    {
        if ( ! parseRoleTypes( is, ptr ) ) return Formation::Ptr();
        if ( ! parsePositionPairs( is, ptr ) ) return Formation::Ptr();
        if ( ! parseMarkerFlags( is ) ) return Formation::Ptr();
        if ( ! parseSetplayMarkerFlags( is ) ) return Formation::Ptr();
        if ( ! parseData( is, ptr ) ) return Formation::Ptr();

        if ( ! checkRoleNames( ptr ) ) return Formation::Ptr();
        if ( ! checkPositionPair( ptr ) ) return Formation::Ptr();
    }

    return ptr;
}

/*-------------------------------------------------------------------*/
std::string
FormationParserCSV::parseMethodName( std::istream & is )
{
    const std::string line = get_value_line( is );

    char method_name[32];
    if ( std::sscanf( line.c_str(), " Method , %31s ", method_name ) != 1 )
    {
        std::cerr << "(FormationParserCSV::parseMethodName) No method name" << std::endl;
        return std::string();
    }

    return method_name;
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseRoleNumbers( std::istream & is )
{
    const std::string line = get_value_line( is );

    char type[32];
    if ( std::sscanf( line.c_str(), " %31[^,] ", type ) != 1 )
    {
        std::cerr << "(FormationParserCSV::parseRoleNumbers) Illegal line"
                  << '[' << line << ']' << std::endl;
        return false;
    }

    if ( std::strcmp( type, "RoleNumber" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parseRoleNumbers) Unexpected line"
                  << '[' << line << ']' << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseRoleNames( std::istream & is,
                                    Formation::Ptr result )
{
    const std::string line = get_value_line( is );
    const char * msg = line.c_str();

    int n_read = 0;
    char type[32];
    if ( std::sscanf( msg, " %31[^,] %n ", type, &n_read ) != 1
         || std::strcmp( type, "RoleName" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parseRoleNames) Illegal line"
                  << '[' << line << ']' << std::endl;
        return false;
    }
    msg += n_read;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        char role_name[32];
        if ( std::sscanf( msg, " , %[^,] %n ", role_name, &n_read ) != 1 )
        {
            std::cerr << "(FormationParserCSV::parseRoleNames) Illegal role name"
                      << '[' << line << ']' << std::endl;
            return false;
        }
        msg += n_read;

        if ( ! result->setRoleName( unum, role_name ) )
        {
            std::cerr << "(FormationParserCSV::parseRoleNames) Could not set the role name"
                      << '[' << line << ']' << std::endl;
            return false;
        }
    }

    return true;
}


/*-------------------------------------------------------------------*/
namespace {
RoleType
create_role_type( const std::string & role_type )
{
    RoleType result;
    if ( role_type == "G" )
    {
        result.setType( RoleType::Goalie );
    }
    else if ( role_type == "DF" )
    {
        result.setType( RoleType::Defender );
    }
    else if ( role_type == "MF" )
    {
        result.setType( RoleType::MidFielder );
    }
    else if ( role_type == "FW" )
    {
        result.setType( RoleType::Forward );
    }

    return result;
}
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseRoleTypes( std::istream & is,
                                    Formation::Ptr result )
{
    const std::string line = get_value_line( is );
    const char * msg = line.c_str();

    int n_read = 0;
    char type[32];
    if ( std::sscanf( msg, " %31[^,] %n ", type, &n_read ) != 1
         || std::strcmp( type, "RoleType" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parseRoleNames) Illegal line"
                  << '[' << line << ']' << std::endl;
        return false;
    }
    msg += n_read;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        char role_type[4];
        if ( std::sscanf( msg, " , %3[^,] %n ", role_type, &n_read ) != 1 )
        {
            std::cerr << "(FormationParserCSV::parseRoleNames) Illegal role name"
                      << '[' << line << ']' << std::endl;
            return false;
        }
        msg += n_read;

        if ( ! result->setRoleType( unum, create_role_type( role_type ) ) )
        {
            std::cerr << "(FormationParserCSV::parseRoleNames) Could not set the role name"
                      << '[' << line << ']' << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parsePositionPairs( std::istream & is,
                                        Formation::Ptr result )
{
    const std::string line = get_value_line( is );
    const char * msg = line.c_str();

    int n_read = 0;
    char type[32];
    if ( std::sscanf( msg, " %31[^,] %n ", type, &n_read ) != 1
         || std::strcmp( type, "SymmetryNumber" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parsePositionPairs) Illegal line"
                  << '[' << line << ']' << std::endl;
        return false;
    }
    msg += n_read;


    for ( int unum = 1; unum <= 11; ++unum )
    {
        int paired_unum = 0;
        if ( std::sscanf( msg, " , %d %n ", &paired_unum, &n_read ) != 1 )
        {
            std::cerr << "(FormationParserCSV::parsePositionPairs) Illegal number"
                      << '[' << line << ']' << std::endl;
            return false;
        }
        msg += n_read;

        RoleType role_type = result->roleType( unum );
        role_type.setSide( paired_unum == 0 ? RoleType::Center :
                           paired_unum < 0 ? RoleType::Left :
                           RoleType::Right );

        if ( ! result->setRoleType( unum, role_type ) )
        {
            std::cerr << "(FormationParserCSV::parsePositionPairs) Could not set the role type"
                      << '[' << line << ']' << std::endl;
            return false;
        }

        if ( ! result->setPositionPair( unum, paired_unum ) )
        {
            std::cerr << "(FormationParserCSV::parsePositionPairs) Could not set the position pair"
                      << '[' << line << ']' << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseMarkerFlags( std::istream & is )
{
    const std::string line = get_value_line( is );
    const char * msg = line.c_str();

    int n_read = 0;
    char type[32];
    if ( std::sscanf( msg, " %31[^,] %n ", type, &n_read ) != 1
         || std::strcmp( type, "Marker" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parseMarkerFlags) Illegal line"
                  << '[' << line << ']' << std::endl;
        return false;
    }
    msg += n_read;

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseSetplayMarkerFlags( std::istream & is )
{
    const std::string line = get_value_line( is );
    const char * msg = line.c_str();

    int n_read = 0;
    char type[32];
    if ( std::sscanf( msg, " %31[^,] %n ", type, &n_read ) != 1
         || std::strcmp( type, "SetplayMarker" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parseSetplayMarkerFlags) Illegal line"
                  << '[' << line << ']' << std::endl;
        return false;
    }
    msg += n_read;

    return true;
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseData( std::istream & is,
                               Formation::Ptr result )
{
    // read tag name
    {
        const std::string line = get_value_line( is );
        if ( line != "SampleData" )
        {
            std::cerr << "(FormationParserCSV::parseData) Illegal line"
                      << '[' << line << ']' << std::endl;
            return false;
        }
    }

    // read data size
    int data_size = 0;
    {
        const std::string line = get_value_line( is );
        if ( std::sscanf( line.c_str(), "S ize , %d ", &data_size ) != 1
             || data_size <= 0 )
        {
            std::cerr << "(FormationParserCSV::parseData) Illegal data size"
                      << '[' << line << ']' << std::endl;
            return false;
        }
    }
    // read data header
    {
        const std::string line = get_value_line( is );
        if ( line.compare( 0, 3, "idx" ) != 0 )
        {
            std::cerr << "(FormationParserCSV::parseData) Illegal header line"
                      << '[' << line << ']' << std::endl;
            return false;

        }
    }

    FormationData formation_data;

    // read data loop
    for ( int i = 0; i < data_size; ++i )
    {
        const std::string line = get_value_line( is );
        const char * msg = line.c_str();
        int n_read = 0;

        FormationData::Data data;

        // read index
        {
            int idx;
            if ( std::sscanf( msg, " %d %n ", &idx, &n_read ) != 1
                 || idx != i )
            {
                std::cerr << "(FormationParserCSV::parseData) Illegal index"
                          << '[' << line << ']' << std::endl;
                return false;
            }
            msg += n_read;
        }
        // read ball
        {
            double read_x, read_y;
            if ( std::sscanf( msg, " , %lf , %lf %n ", &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr << "(FormationParserCSV::parseData) Illegal ball data"
                          << '[' << line << ']' << std::endl;
                return false;
            }
            msg += n_read;

            data.ball_ = FormationData::rounded_vector( read_x, read_y );
        }
        // read players
        for ( int unum = 1; unum <= 11; ++unum )
        {
            double read_x, read_y;
            if ( std::sscanf( msg, " , %lf , %lf %n ", &read_x, &read_y, &n_read ) != 2 )
            {
                std::cerr << "(FormationParserCSV::parseData) Illegal player data. unum=" << unum
                          << '[' << msg << ']' << std::endl;
                return false;
            }
            msg += n_read;

            data.players_.emplace_back( FormationData::round_xy( read_x ),
                                        FormationData::round_xy( read_y ) );
        }

        const std::string err = formation_data.addData( data );
        if ( ! err.empty() )
        {
            std::cerr << "(FormationParserCSV::parseData) ERROR: " << err << std::endl;
            return false;
        }
    }

    return result->train( formation_data );
}

/*-------------------------------------------------------------------*/
bool
FormationParserCSV::parseStaticPositions( std::istream & is,
                                          Formation::Ptr result )
{
    if ( ! result ) return false;

    const std::string line = get_value_line( is );
    const char * buf = line.c_str();

    int n_read = 0;
    char type[32];
    if ( std::sscanf( buf, " %31[^,] %n ", type, &n_read ) != 1
         || std::strcmp( type, "Position" ) != 0 )
    {
        std::cerr << "(FormationParserCSV::parseStaticPosition) Illegal line "
                  << '[' << line << ']' << std::endl;
        return false;
    }
    buf += n_read;

    FormationData::Data data;
    data.ball_.assign( 0.0, 0.0 );

    for ( int num = 1; num <= 11; ++num )
    {
        double read_x, read_y;
        if ( std::sscanf( buf, " ,  %lf , %lf %n ", &read_x, &read_y, &n_read ) != 2 )
        {
            std::cerr << "(FormationParserCSV::parseStaticPosition) Illegal value "
                      << '[' << buf << ']' << std::endl;
            return false;
        }
        buf += n_read;

        data.players_.emplace_back( FormationData::round_xy( read_x ),
                                    FormationData::round_xy( read_y ) );
    }

    FormationData formation_data;
    const std::string err = formation_data.addData( data );
    if ( ! err.empty() )
    {
        std::cerr << "(FormationParserCSV::parseStaticPosition) ERROR: " << err << std::endl;
    }

    return result->train( formation_data );
}

}
