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

#include "formation_data.h"

#include <sstream>

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserV3::parse( std::istream & is )
{
    if ( ! parseHeader( is ) ) return Formation::Ptr();
    if ( ! parseRoles( is ) ) return Formation::Ptr();
    if ( ! parseData( is ) ) return Formation::Ptr();
    if ( ! parseEnd( is ) ) return Formation::Ptr();

    if ( ! checkPositionPair() ) return Formation::Ptr();

    Formation::Ptr ptr;
    return ptr;
}



/*-------------------------------------------------------------------*/
bool
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

        std::istringstream istr( line );

        std::string tag;
        std::string method_name;
        int ver = 0;

        istr >> tag;
        if ( tag != "Formation" )
        {
            std::cerr << "(FormationParserV3::parseHeader) unknown tag [" << tag << "]" << std::endl;
            return false;
        }

        istr >> method_name;
        if ( ! istr )
        {
            std::cerr << "(FormationParserV3::parseHeader) No method name" << std::endl;
            return false;
        }

        if ( istr >> ver )
        {
            if ( ver != 3 )
            {
                std::cerr << "(FormationParserV3::parseHeader) Illegas format version " << ver << std::endl;
                return false;
            }
        }

        return true;
    }

    return false;
}


/*-------------------------------------------------------------------*/
bool
FormationParserV3::parseRoles( std::istream & is )
{
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
        int position_pair = 0;
        char role_type[4];
        char marker[32];
        char smarker[32];

        if ( std::sscanf( line.c_str(),
                          " %d %3s %127s %d %31s %31s ",
                          &read_unum, role_type, role_name, &position_pair, marker, smarker ) != 6
             || read_unum != unum )
        {
            std::cerr << "(FormationParserV3::parseRoles). Illegal role data [" << line << "]" << std::endl;
            return false;
        }

        // createRoleOrSetSymmetry( unum, role_name, position_pair );
        // setRoleType( unum, role_type );
        // setMarker( unum, marker, smarker );
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
FormationParserV3::parseData( std::istream & is )
{
    FormationData::Ptr data( new FormationData() );

    if ( ! data->readOld( is ) )
    {
        return false;
    }

    return false;
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
