// -*-c++-*-

/*!
  \file formation.cpp
  \brief formation data classes Source File.
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

#include "formation.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Formation()
{
    M_position_pairs.fill( 0 );
}

/*-------------------------------------------------------------------*/
bool
Formation::setVersion( const std::string & ver )
{
    M_version = ver;
    return true;
}

/*-------------------------------------------------------------------*/
bool
Formation::setRoleName( const int num,
                        const std::string & name )
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << "(Formation::setRoleName) illegal number " << num << std::endl;
        return false;
    }

    if ( name.empty() )
    {
        std::cerr << "(Formation::setRoleName) empty role name" << std::endl;
        return false;
    }

    M_role_names[num - 1] = name;
    return true;
}

/*-------------------------------------------------------------------*/
bool
Formation::setRoleType( const int num,
                        const RoleType & type )
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << "(Formation::setRoleType) illegal number " << num << std::endl;
        return false;
    }

    M_role_types[num - 1] = type;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::setPositionPair( const int num,
                            const int paired_num )
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << "(Formation::setPositionPair) illegal number " << num << std::endl;
        return false;
    }

    if ( paired_num < -1 || 11 < paired_num )
    {
        std::cerr << "(FormationData;:setPositionPair) illegal paired number " << paired_num << std::endl;
        return false;
    }

    if ( num == paired_num )
    {
        std::cerr << "(FormationData;:setPositionPair) num " << num << " == paired " << paired_num << std::endl;
        return false;
    }

    if ( paired_num >= 1 )
    {
        // check doubling registration
        for ( int i = 0; i < 11; ++i )
        {
            if ( i + 1 == num ) continue;

            if ( paired_num == M_position_pairs[i] )
            {
                std::cerr << "(Formation::setPositionPair) " << paired_num << " already registered "<< std::endl;
                return false;
            }
        }

        if ( 1 <= paired_num && paired_num <= 11
             && M_position_pairs[paired_num - 1] > 0
             && M_position_pairs[paired_num - 1] != num )
        {
            std::cerr << "(Formation::setPositionPair) already has the pair. num = " << num << " paired = " << paired_num << std::endl;
            return false;
        }
    }

    M_position_pairs[num - 1] = paired_num;

    if ( 1 <= paired_num && paired_num <= 11 )
    {
        M_position_pairs[paired_num - 1] = num;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
Formation::setRole( const int num,
                    const std::string & name,
                    const RoleType & type,
                    const int paired_num )
{
    if ( ! setRoleName( num, name ) ) return false;
    if ( ! setRoleType( num, type ) ) return false;
    if ( ! setPositionPair( num, paired_num ) ) return false;

    return true;
}

/*-------------------------------------------------------------------*/
bool
Formation::print( std::ostream & os ) const
{
    os << "{\n";

    if ( ! printVersion( os ) ) return false;
    os << ",\n";

    if ( ! printMethodName( os ) ) return false;
    os << ",\n";

    if ( ! printRoles( os ) ) return false;
    os << ",\n";

    if ( ! printData( os ) ) return false;
    os << "\n";

    os << "}" << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
namespace {
const std::string tab = "  ";
}

/*-------------------------------------------------------------------*/
bool
Formation::printVersion( std::ostream & os ) const
{
    os << tab << '"' << "version" << '"' << " : " << '"' <<  version() << '"';
    return true;
}

/*-------------------------------------------------------------------*/
bool
Formation::printMethodName( std::ostream & os ) const
{
    os << tab << '"' << "method" << '"' << " : " << '"' << methodName() << '"';
    return true;
}

/*-------------------------------------------------------------------*/
bool
Formation::printRoles( std::ostream & os ) const
{
    os << tab << "\"role\"" << " : [\n";

    for ( size_t i = 0; i < 11; ++i )
    {
        if ( i != 0 ) os << ",\n";

        os << tab << tab << "{\n"
           << tab << tab << tab << " \"number\" : " << i + 1 << ",\n"
           << tab << tab << tab << " \"name\" : " << '"' << M_role_names[i] << '"' << ",\n"
           << tab << tab << tab << " \"type\" : " << '"' << RoleType::to_string( M_role_types[i].type() ) << '"' << ",\n"
           << tab << tab << tab << " \"side\" : " << '"' << RoleType::to_string( M_role_types[i].side() ) << '"' << ",\n"
           << tab << tab << tab << " \"pair\" : " << M_position_pairs[i] << '\n'
           << tab << tab << "}";
    }

    os << '\n' << tab << ']';
    return true;
}


}

#include "formation_dt.h"
#include "formation_static.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
Formation::Ptr
Formation::create( const std::string & name )
{
    Formation::Ptr ptr;

    if ( name == FormationDT::NAME ) ptr = FormationDT::create();
    else if ( name == FormationStatic::NAME ) ptr = FormationStatic::create();

    return ptr;
}

}
