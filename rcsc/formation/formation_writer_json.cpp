// -*-c++-*-

/*!
  \file formation_writer_json.cpp
  \brief JSON style formation data writer class Source File.
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

#include "formation_writer_json.h"

namespace rcsc {

const std::string FormationWriterJSON::NAME = "json";

namespace {

const std::string tab = "  ";

/*-------------------------------------------------------------------*/
bool
print_version( std::ostream & os,
               Formation::ConstPtr f )
{
    os << tab << '"' << "version" << '"' << " : " << '"' <<  f->version() << '"';
    return true;
}


/*-------------------------------------------------------------------*/
bool
print_method_name( std::ostream & os,
                   Formation::ConstPtr & f )
{
    os << tab << '"' << "method" << '"' << " : " << '"' << f->methodName() << '"';
    return true;
}

/*-------------------------------------------------------------------*/
std::string
get_role_type_string( const RoleType & type )
{
    return ( type.isGoalie() ? "G"
             : type.isDefender() ? "DF"
             : type.isMidFielder() ? "MF"
             : type.isForward() ? "FW"
             : "Unknown" );
}

/*-------------------------------------------------------------------*/
bool
print_roles( std::ostream & os,
             Formation::ConstPtr f )
{
    os << tab << "\"role\"" << " : [\n";

    try
    {
        for ( size_t i = 0; i < 11; ++i )
        {
            if ( i != 0 ) os << ",\n";

            os << tab << tab << '{';
            os << "\"number\" : " << i + 1 << ','
               << "\"name\" : " << '"' << f->roleNames().at( i ) << '"' << ','
               << "\"type\" : " << '"' << get_role_type_string( f->roleTypes().at( i ) )
               << "\"pair\" : " << f->positionPairs().at( i )
               << '}';
        }
    }
    catch ( std::exception & e )
    {
        std::cerr << "(FormationWriterJSON..print_roles) " << e.what() << std::endl;
        return false;
    }

    os << tab << ']';
    return true;
}

/*-------------------------------------------------------------------*/
bool
print_data_element( std::ostream & os,
                    const size_t idx,
                    const FormationData::Data & data )
{
    os << tab << tab << "{\n";
    os << tab << tab << tab << "\"index\" : " << idx << ",\n";
    os << tab << tab << tab << "\"ball\" : "
       << "\"x\" : " << data.ball_.x << ", "
       << "\"y\" : " << data.ball_.y << " }";

    for ( size_t i = 0; i < data.players_.size(); ++i )
    {
        os << ",\n";
        os << tab << tab << tab;
        os << '"' << i + 1 << '"' // unum
           << " : { "
           << "\"x\" : " << data.players_[i].x << ", "
           << "\"y\" : " << data.players_[i].y
           << " }";
    }

    os << tab << tab << '}';
    return true;
}

/*-------------------------------------------------------------------*/
bool
print_data( std::ostream & os,
            Formation::ConstPtr f )
{
    os << tab << "\"data\"" << " : ";

    //f->printJSON( os );
    // size_t idx = 0;
    // for ( const auto & d : data->dataCont() )
    // {
    //     if ( idx != 0 ) os << ",\n";

    //     if ( ! print_data_element( os, idx, d ) )
    //     {
    //         return false;
    //     }
    //     ++idx;
    // }

    return true;
}

}

/*-------------------------------------------------------------------*/
bool
FormationWriterJSON::print( std::ostream & os,
                            Formation::ConstPtr f ) const

{
    os << "{\n";

    if ( ! print_version( os, f ) ) return false;
    os << ",\n";

    if ( ! print_method_name( os, f ) ) return false;
    os << ",\n";

    if ( ! print_roles( os, f ) ) return false;
    os << ",\n";

    if ( ! print_data( os, f ) ) return false;
    os << '\n';

    os << "}\n";
    os << std::flush;

    return true;
}

}
