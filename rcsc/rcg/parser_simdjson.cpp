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

#include <string_view>
#include <functional>

namespace rcsc {
namespace rcg {


struct ParserSimdJSON::Impl {

    using Func = std::function< bool( simdjson::ondemand::value & val, Handler & handler ) >;
    std::unordered_map< std::string, Func > funcs_;
    //std::unordered_map< simdjson::ondemand::raw_json_string, Func > funcs_;

    Impl();

    bool parseData( simdjson::ondemand::field & field,
                    Handler & handler );

    bool parseVersion( simdjson::ondemand::value & val,
                       Handler & handler );
    bool parseTimestamp( simdjson::ondemand::value & val,
                         Handler & handler );
    bool parseServerParam( simdjson::ondemand::value & val,
                           Handler & handler );
    bool parsePlayerParam( simdjson::ondemand::value & val,
                           Handler & handler );
    bool parsePlayerType( simdjson::ondemand::value & val,
                          Handler & handler );
    bool parseTeamGraphic( simdjson::ondemand::value & val,
                           Handler & handler );
    bool parsePlaymode( simdjson::ondemand::value & val,
                        Handler & handler );
    bool parseTeam( simdjson::ondemand::value & val,
                    Handler & handler );
    bool parseMsg( simdjson::ondemand::value & val,
                   Handler & handler );
    bool parseShow( simdjson::ondemand::value & val,
                    Handler & handler );
};

/*-------------------------------------------------------------------*/
ParserSimdJSON::Impl::Impl()
{
    funcs_["\"version\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseVersion( val, handler );
          };
    funcs_["\"timestamp\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseTimestamp( val, handler );
          };

    funcs_["\"server_param\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseServerParam( val, handler );
          };
    funcs_["\"player_param\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parsePlayerParam( val, handler );
          };
    funcs_["\"player_type\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parsePlayerType( val, handler );
          };
    funcs_["\"team_graphic\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseTeamGraphic( val, handler );
          };
    funcs_["\"playmode\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parsePlaymode( val, handler );
          };
    funcs_["\"team\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseTeam( val, handler );
          };
    funcs_["\"msg\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseMsg( val, handler );
          };
    funcs_["\"show\""]
        = [this]( simdjson::ondemand::value & val, Handler & handler ) -> bool
          {
              return this->parseShow( val, handler );
          };
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseData( simdjson::ondemand::field & field,
                                 Handler & handler )
{
    //std::string_view key = field.key_raw_json_token();
    //std::string str_key( key.substr( 1, key.length() - 2 ) );
    //std::string str_key( key );
    //std::cerr << "(ParserSimdJSON::Impl::parseData) key=" << field.key() << " str=" << str_key << std::endl;
    //std::unordered_map< std::string, Func >::iterator it = funcs_.find( str_key );
    std::string key( field.key_raw_json_token() );
    std::unordered_map< std::string, Func >::iterator it = funcs_.find( key );
    if ( it == funcs_.end() )
    {
        std::cerr << "(ParserSimdJSON::Impl::parseData) func not found. key=" << key
                  << std::endl;
        return false;
    }

    if ( ! it->second( field.value(), handler ) )
    {
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseVersion( simdjson::ondemand::value & val,
                                    Handler & /*handler*/ )
{
    std::cerr << "version " << val << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseTimestamp( simdjson::ondemand::value & val,
                                    Handler & /*handler*/ )
{
    std::cerr << "timestamp " << val << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseServerParam( simdjson::ondemand::value & val,
                                        Handler & handler )
{
    ServerParamT server_param;

    for ( simdjson::ondemand::field param : val.get_object() )
    {
        std::string_view key = param.key_raw_json_token();
        if ( key.length() < 3
             || *key.begin() != '\"'
             || *key.rbegin() != '\"' )
        {
            std::cerr << "(ParserSimdJSON::parseServerParam) Error: unexpected key [" << key << "]" << std::endl;
            return false;
        }

        const std::string name( key.substr( 1, key.length() - 2 ) );
        const std::string value( param.value().raw_json_token() );

        server_param.setValue( name, value );
    }

    return handler.handleServerParam( server_param );
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parsePlayerParam( simdjson::ondemand::value & val,
                                        Handler & handler )
{
    PlayerParamT player_param;

    for ( simdjson::ondemand::field param : val.get_object() )
    {
        std::string_view key = param.key_raw_json_token();
        if ( key.length() < 3
             || *key.begin() != '\"'
             || *key.rbegin() != '\"' )
        {
            std::cerr << "(ParserSimdJSON::parsePlayerParam) Error: unexpected key [" << key << "]" << std::endl;
            return false;
        }

        const std::string name( key.substr( 1, key.length() - 2 ) );
        const std::string value( param.value().raw_json_token() );

        player_param.setValue( name, value );
    }

    return handler.handlePlayerParam( player_param );
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parsePlayerType( simdjson::ondemand::value & val,
                                       Handler & handler )
{
    PlayerTypeT player_type;

    for ( simdjson::ondemand::field param : val.get_object() )
    {
        std::string_view key = param.key_raw_json_token();
        if ( key.length() < 3
             || *key.begin() != '\"'
             || *key.rbegin() != '\"' )
        {
            std::cerr << "(ParserSimdJSON::parsePlayerType) Error: unexpected key [" << key << "]" << std::endl;
            return false;
        }

        const std::string name( key.substr( 1, key.length() - 2 ) );
        const std::string value( param.value().raw_json_token() );

        player_type.setValue( name, value );
    }

    return handler.handlePlayerType( player_type );
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseTeamGraphic( simdjson::ondemand::value & val,
                                        Handler & /*handler*/ )
{
    std::cerr << "team_graphic " << val << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parsePlaymode( simdjson::ondemand::value & val,
                                     Handler & /*handler*/ )
{
    std::cerr << "playmode = " << val << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseTeam( simdjson::ondemand::value & val,
                                 Handler & /*handler*/ )
{
    std::cerr << "team = " << val << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseMsg( simdjson::ondemand::value & val,
                                Handler & /*handler*/ )
{
    std::cerr << "msg = " << val << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::Impl::parseShow( simdjson::ondemand::value & val,
                                 Handler & /*handler*/ )
{
    std::cerr << "show " << val["time"] << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

/*-------------------------------------------------------------------*/
ParserSimdJSON::ParserSimdJSON()
    : M_impl( new Impl() )
{
    std::cerr << "ParserSimdJSON" << std::endl;
}

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
        for ( simdjson::ondemand::field field : data )
        {
            //std::cout << "key = " << f.key() << std::endl;
            if ( ! M_impl->parseData( field, handler ) )
            {
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
bool
ParserSimdJSON::parseData( const std::string & input,
                           Handler & handler ) const
{
    simdjson::ondemand::parser parser;
    simdjson::padded_string json( input );
    simdjson::ondemand::document data = parser.iterate( json );

    for ( simdjson::ondemand::field field : data.get_object() )
    {
        //std::cout << "key = " << f.key() << std::endl;
        if ( ! M_impl->parseData( field, handler ) )
        {
            return false;
        }
    }

    return true;
}


namespace {
Parser::Ptr
create_json()
{
    Parser::Ptr ptr( new ParserSimdJSON() );
    return ptr;
}

rcss::RegHolder vjson = Parser::creators().autoReg( &create_json, REC_VERSION_JSON );
}

}
}
