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

#include "parser_json.h"

#include "handler.h"
#include "types.h"

#include "nlohmann/json.hpp"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <climits>
#include <cstdio>

namespace rcsc {
namespace rcg {


class Context
    : public nlohmann::json::json_sax_t {
private:

    Handler & M_handler;

    Context() = delete;
    Context( const Context & ) = delete;
    const Context & operator=( const Context & ) = delete;
public:

    Context( Handler & handler )
        : M_handler( handler )
      { }

    bool key( string_t & /*val*/ ) override
      {
          return true;
      }

    bool null() override
      {
          return true;
      }

    bool boolean( bool /*val*/ ) override
      {
          return true;
      }

    bool number_integer( number_integer_t /*val*/ ) override
      {
          return true;
      }

    bool number_unsigned( number_unsigned_t /*val*/ ) override
      {
          return true;
      }

    bool number_float( number_float_t /*val*/,
                       const string_t & /*s*/ ) override
      {
          return true;
      }

    bool string( string_t & /*val*/ ) override
      {
          return true;
      }

    bool start_object( std::size_t /*elements*/ ) override
      {
          return true;
      }

    bool end_object() override
      {
          return true;
      }
    bool start_array( std::size_t /*elements*/ ) override
      {
          return true;
      }

    bool end_array() override
      {
          return true;
      }

    bool binary( nlohmann::json::binary_t & /*val*/ ) override
      {
          return true;
      }


    bool parse_error( std::size_t /*position*/,
                      const std::string & /*last_token*/,
                      const nlohmann::json::exception & /*ex*/ ) override
      {
          return false;
      }
};

//
// class ParserJSON
//

/*-------------------------------------------------------------------*/
ParserJSON::ParserJSON()
{

}

/*-------------------------------------------------------------------*/
bool
ParserJSON::parse( std::istream & is,
                   Handler & handler ) const
{
    // streampos must be the first position.
    is.seekg( 0 );

    if ( ! is.good() )
    {
        return false;
    }


    Context context( handler );
    bool result = nlohmann::json::sax_parse( is, &context );

    return result;
}

/*-------------------------------------------------------------------*/
bool
ParserJSON::parseData( const std::string & input,
                       Handler & handler ) const
{
    Context context( handler );
    bool result = nlohmann::json::sax_parse( input, &context );

    return result;
}

} // end of namespace
} // end of namespace
