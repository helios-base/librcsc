// -*-c++-*-

/*!
  \file formation_parser_v3.cpp
  \brief v3 formation parser class Source File.
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

#include "formation_parser_v3.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserV3::parseImpl( std::istream & is )
{
    const std::string method = parseHeader( is );

    Formation::Ptr ptr = Formation::create( method );
    if ( ! ptr )
    {
        std::cerr << "(FormationParserV3::parse) Could not create the formation " << method << std::endl;
        return Formation::Ptr();
    }

    if ( ! parseRoles( is, ptr ) ) return Formation::Ptr();

    FormationData formation_data;
    if ( ! parseData( is, &formation_data ) ) return Formation::Ptr();
    if ( ! parseEnd( is ) ) return Formation::Ptr();

    if ( ! checkRoleNames( ptr ) ) return Formation::Ptr();
    if ( ! checkPositionPair( ptr ) ) return Formation::Ptr();

    if ( ! ptr->train( formation_data ) ) return Formation::Ptr();

    return ptr;
}

/*-------------------------------------------------------------------*/
std::string
FormationParserV3::parseHeader( std::istream & is )
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

    if ( std::sscanf( line.c_str(), "Formation %31s %d", method_name, &ver ) != 2 )
    {
        std::cerr << "(FormationParserV2::parseHeader) ERROR: illegal header"
                  << '[' << line << ']' << std::endl;
        return std::string();
    }

    if ( ver != 3 )
    {
        std::cerr << "(FormationParserV2::parseHeader) Illegas format version " << ver << std::endl;
        return std::string();
    }

    return method_name;
}

/*-------------------------------------------------------------------*/
namespace {
RoleType
create_role_type( const std::string & role_type,
                  const int paired_unum )
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

    if ( paired_unum == 0 )
    {
        result.setSide( RoleType::Center );
    }
    else if ( paired_unum == -1 )
    {
        result.setSide( RoleType::Left );
    }
    else
    {
        result.setSide( RoleType::Right );
    }

    return result;
}
}

/*-------------------------------------------------------------------*/
bool
FormationParserV3::parseRoles( std::istream & is,
                               Formation::Ptr result )
{
    if ( ! result ) return false;

    //
    // read Begin tag
    //

    if ( ! parseBeginRolesTag( is ) )
    {
        return false;
    }

    //
    // read role data
    //

    for ( int unum = 1; unum <= 11; ++unum )
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

        int read_unum = 0;
        char role_name[128];
        int paired_unum = 0;
        char role_type[4];
        char marker[32];
        char smarker[32];

        if ( std::sscanf( line.c_str(),
                          " %d %3s %127s %d %31s %31s ",
                          &read_unum, role_type, role_name, &paired_unum, marker, smarker ) != 6
             || read_unum != unum )
        {
            std::cerr << "(FormationParserV3::parseRoles). Illegal role data "
                      << "[" << line << "]" << std::endl;
            return false;
        }

        if ( ! result->setRoleName( unum, role_name ) )
        {
            return false;
        }

        if ( ! result->setRoleType( unum, create_role_type( role_type, paired_unum ) ) )
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
    if ( ! parseEndRolesTag( is ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationParserV3::parseBeginRolesTag( std::istream & is )
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

        if ( line != "Begin Roles" )
        {
            std::cerr << "(FormationParserV3::parseBeginRolesTag) unexpected string [" << line << ']' << std::endl;
            return false;
        }

        return true;
    }

    std::cerr << "(FormationParserV3::parseBeginRolesTag) 'End Roles' not found" << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationParserV3::parseEndRolesTag( std::istream & is )
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

        if ( line != "End Roles" )
        {
            std::cerr << "(FormationParserV3::parseEndRolesTag) unexpected string [" << line << ']' << std::endl;
            return false;
        }

        // found
        return true;
    }

    std::cerr << "(FormationParserV3::parseendRolesTag) 'End Roles' not found" << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV3::parseData( std::istream & is,
                              FormationData * formation_data )
{
    if ( ! formation_data ) return false;

    int data_size = 0;
    if ( ! parseDataHeader( is, &data_size ) )
    {
        return false;
    }

    for ( int i = 0; i < data_size; ++i )
    {
        if ( ! parseOneData( is, i, formation_data ) )
        {
            return false;
        }
    }

    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line.empty()
             || line[0] == '#'
             || ! line.compare( 0, 2, "//" ) )
        {
            continue;
        }

        if ( ! line.compare( 0, 13, "End Samples" ) )
        {
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
bool
FormationParserV3::parseDataHeader( std::istream & is,
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
                std::cerr << "(FormationParserV3::parseData) ERROR: "
                          << " Illegal data header [" << line << "]" << std::endl;
                return false;
            }

            if ( version != 2 )
            {
                std::cerr << "(FormationParserV3::parseData) ERROR: "
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
FormationParserV3::parseOneData( std::istream & is,
                                 const int index,
                                 FormationData * formation_data )
{
    if ( ! formation_data ) return false;

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
            std::cerr << "(FormationParaserV3::parseOneData) ERROR: Illegal data segment. "
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
        std::cerr << "(FormationParserV3::parseOneData) ERROR: failed to read ball data."
                  << " index=" << index << std::endl;
        return false;
    }

    if ( std::sscanf( line.c_str(),
                      " Ball %lf %lf ",
                      &read_x, &read_y ) != 2 )
    {
        std::cerr << "(FormationParserV3::parseOneData) ERROR: Illegal ball data."
                  << " index=" << index << " [" << line << "]" << std::endl;
        return false;
    }

    new_data.ball_ = FormationData::rounded_vector( read_x,  read_y );

    //
    // read player data
    //

    int read_unum = 0;

    for ( int unum = 1; unum <= 11; ++unum )
    {
        if ( ! std::getline( is, line ) )
        {
            std::cerr << "(FormationParserV3::parseOneData) ERROR: failed to read a line."
                      << " index" << index << " unum=" << unum << std::endl;
            return false;
        }

        if ( std::sscanf( line.c_str(),
                          " %d %lf %lf ",
                          &read_unum, &read_x, &read_y ) != 3
             || read_unum != unum )
        {
            std::cerr << "(FormationParserV3::parseOneData) ERROR: Illegal player data."
                      << " index=" << index << " unum=" << unum << " [" << line << "]" << std::endl;
            return false;
        }

        new_data.players_.emplace_back( FormationData::round_xy( read_x ),
                                        FormationData::round_xy( read_y ) );
    }

    const std::string err = formation_data->addData( new_data );
    if ( ! err.empty() )
    {
        std::cerr << "(FormationParserV3::parseOneData) ERROR: " << err << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationParserV3::parseEnd( std::istream & is )
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

        if ( line != "End" )
        {
            std::cerr << "(FormationParserV3::parseEnd) unexpected string [" << line << ']' << std::endl;
            return false;
        }

        // found
        return true;
    }

    std::cerr << "(FormationParserV3::parseEnd) 'End' not found" << std::endl;
    if ( is.eof() )
    {
        std::cerr << "(FormationParserV3::parseEnd) Input stream reaches EOF" << std::endl;
    }

    return false;
}

}
