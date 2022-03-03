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

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserV2::parse( std::istream & is )
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
FormationParserV2::parseRoles( std::istream & is )
{
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
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRolesV2(). Illegal header ["
                      << line << ']'
                      << std::endl;
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
        int position_pair = 0;

        if ( std::sscanf( line.c_str(),
                          " %d %127s %d ",
                          &read_unum, role_name, &position_pair ) != 3
             || read_unum != unum )
        {
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRolesV2(). Illegal role data. num="
                      << unum
                      << " [" << line << "]"
                      << std::endl;
            return false;
        }

        //
        // create role or set position pair.
        //
        // const Formation::SideType type = ( position_pair == 0
        //                                    ? Formation::CENTER
        //                                    : position_pair < 0
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
        //     setSymmetryType( unum, position_pair, role_name );
        // }
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
            std::cerr << __FILE__ << ':' << __LINE__ << ':'
                      << " *** ERROR *** readRolesV2(). Failed getline "
                      << std::endl;
            return false;
        }

        break;
    }

    return true;
}


/*-------------------------------------------------------------------*/
bool
FormationParserV2::parseData( std::istream & is )
{
    FormationData::Ptr samples( new FormationData() );

    if ( ! samples->readOld( is ) )
    {
        return false;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
FormationParserV2::parseEnd( std::istream & is )
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
            std::cerr << "(FormationParserV2::parseEnd) unexpected string [" << line << ']' << std::endl;
            return false;
        }

        // found
        return true;
    }

    std::cerr << "(FormationParserV2::parseEnd) 'End' not found" << std::endl;
    if ( is.eof() )
    {
        std::cerr << "(FormationParserV2::parseEnd) Input stream reaches EOF" << std::endl;
    }

    return false;
}

}
