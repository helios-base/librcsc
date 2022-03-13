// -*-c++-*-

/*!
  \file formation_parser_json.cpp
  \brief json formation parser class Source File.
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

#include "formation_parser_json.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;

namespace rcsc {

namespace {

/*-------------------------------------------------------------------*/
std::string
parse_method_name( const ptree & doc )
{
    boost::optional< std::string > v = doc.get_optional< std::string >( "method" );

    if ( ! v )
    {
        std::cerr << "(FormationParserJSON..parse_method_name) No method name" << std::endl;
        return std::string();
    }

    return *v;
}

/*-------------------------------------------------------------------*/
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

/*-------------------------------------------------------------------*/
bool
parse_role( const ptree & doc,
            Formation::Ptr result )
{
    if ( ! result ) return false;

    boost::optional< const ptree & > role_array = doc.get_child_optional( "role" );
    if ( ! role_array )
    {
        std::cerr << "(FormationParserJSON..parse_role) No role array" << std::endl;
        return false;
    }

    for ( const ptree::value_type & child : *role_array )
    {
        const ptree & role = child.second;

        boost::optional< int > number = role.get_optional< int >( "number" );
        boost::optional< std::string > name = role.get_optional< std::string >( "name" );
        boost::optional< std::string > type = role.get_optional< std::string >( "type" );
        boost::optional< int > pair = role.get_optional< int >( "pair" );

        if ( ! number
             || *number < 1 || 11 < *number
             || ! name
             || ! type
             || ! pair )
        {
            return false;
        }

        if ( ! result->setRoleName( *number, *name ) )
        {
            return false;
        }

        const RoleType role_type = create_role_type( *type, *pair );
        if ( role_type.type() == RoleType::Unknown
             || ! result->setRoleType( *number, role_type ) )
        {
            return false;
        }

        if ( ! result->setPositionPair( *number, *pair ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
parse_data( const ptree & doc,
            Formation::Ptr result )
{
    if ( ! result ) return false;

    boost::optional< const ptree & > data_array = doc.get_child_optional( "data" );
    if ( ! data_array )
    {
        std::cerr << "(FormationParserJSON..parse_data) No data array" << std::endl;
        return false;
    }

    FormationData formation_data;

    for ( const ptree::value_type & child : *data_array )
    {
        const ptree & elem = child.second;

        FormationData::Data data;

        try
        {
            data.ball_.assign( FormationData::round_xy( elem.get< double >( "ball.x" ) ),
                               FormationData::round_xy( elem.get< double >( "ball.y" ) ) );

            for ( int i = 0; i < 11; ++i )
            {
                const std::string unum = std::to_string( i + 1 );
                data.players_.emplace_back( FormationData::round_xy( elem.get< double >( unum + ".x" ) ),
                                            FormationData::round_xy( elem.get< double >( unum + ".y" ) ) );
            }
        }
        catch ( std::exception & e )
        {
            std::cerr << "(FormationParserJSON..parse_data) " << e.what() << std::endl;
            return false;
        }

        const std::string err = formation_data.addData( data );
        if ( ! err.empty() )
        {
            std::cerr << "(FormationParserJSON..parse_data) ERROR: " << err << std::endl;
            return false;
        }
    }

    return result->train( formation_data );
}

}

/*-------------------------------------------------------------------*/
Formation::Ptr
FormationParserJSON::parse( std::istream & is )
{
    ptree doc;
    try
    {
        boost::property_tree::read_json( is, doc );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(FormationParserJSON::parse) ERROR: read_json. " << e.what() << std::endl;
        return Formation::Ptr();
    }


    const std::string method = parse_method_name( doc );

    Formation::Ptr ptr = Formation::create( method );
    if ( ! ptr )
    {
        std::cerr << "(FormationParserJSON::parse) Could not create the formation " << method << std::endl;
        return Formation::Ptr();
    }


    if ( ! parse_role( doc, ptr ) ) return Formation::Ptr();
    if ( ! parse_data( doc, ptr ) ) return Formation::Ptr();

    return ptr;
}

}
