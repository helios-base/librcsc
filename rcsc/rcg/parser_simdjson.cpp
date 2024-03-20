// -*-c++-*-

/*!
  \file parser_json.cpp
  \brief rcg v6 (json) parser Source File.
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

#include "parser_simdjson.h"

#include "handler.h"
#include "types.h"
#include "util.h"

#include "simdjson/simdjson.h"

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::parse( std::istream & is,
                       Handler & handler ) const
{
    // streampos must be the first point!!!
    is.seekg( 0 );

    if ( ! is.good() )
    {
        return false;
    }

    simdjson::ondemand::parser parser;
    simdjson::padded_string json( std::string( std::istreambuf_iterator< char >( is ), {} ) );
    simdjson::ondemand::document rcg = parser.iterate( json );
    simdjson::ondemand::array root_array = rcg.get_array();
    for ( simdjson::ondemand::object data : root_array )
    {
        for ( simdjson::ondemand::field f : data )
        {
            //std::cout << "key = " << f.key() << std::endl;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::parseData( const std::string & input,
                           Handler & handler ) const
{
    return true;
}

}
}
