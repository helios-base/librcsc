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
#include <unordered_map>
#include <memory>
#include <string>

namespace rcsc {
namespace rcg {

class Context;

//
//
//

class State {
public:
    using Ptr = std::shared_ptr< State >;

private:
    State() = delete;
    State( const State & ) = delete;
    const State & operator=( const State & ) = delete;
protected:
    Context & M_context;
    bool M_finished;

    State( Context & context )
        : M_context( context ),
          M_finished( false )
      { }
public:
    virtual
    ~State() = default;

    virtual bool onKey( const std::string & ) { return false; }
    virtual bool onNull() { return false; }
    virtual bool onBoolean( bool ) { return false; }
    virtual bool onInteger( int ) { return false; }
    virtual bool onUnsigned( unsigned int ) { return false; }
    virtual bool onFloat( unsigned int ) { return false; }
    virtual bool onString( const std::string & ) { return false; }
    virtual bool onStartObject( const size_t ) { return false; }
    virtual bool onEndObject() { return false; }
    virtual bool onStartArray( const size_t ) { return false; }
    virtual bool onEndArray() { return false; }

    virtual bool handle( Handler & ) { return false; }

    virtual bool isFinished() = 0;
};

//
//
//

class StateVersion
    : public State {
public:
    StateVersion( Context & context )
        : State( context )
      { }

    bool onString( const std::string & val ) override
      {
          std::cerr << "(StateVersion::onString) " << val << std::endl;
          return true;
      }

    bool onEndObject() override
      {
          M_finished = true;
          return true;
      }

    bool isFinished() override
      {
          return M_finished;
      }
};

//
//
//

class StateTimeStamp
    : public State {
public:
    StateTimeStamp( Context & context )
        : State( context )
      { }

    virtual bool onString( const std::string & val )
      {
          std::cerr << "(StateVersion::onString) " << val << std::endl;
          return true;
      }


    bool onEndObject() override
      {
          M_finished = true;
          return true;
      }

    bool isFinished() override
      {
          return M_finished;
      }
};

//
//
//

class Context
    : public nlohmann::json::json_sax_t {
private:

    Handler & M_handler;

    std::shared_ptr< State > M_state;
    int M_depth;

    //! key, state map
    std::unordered_map< std::string, std::shared_ptr< State > > M_state_map;

    Context() = delete;
    Context( const Context & ) = delete;
    const Context & operator=( const Context & ) = delete;
public:

    Context( Handler & handler )
        : M_handler( handler ),
          M_depth( 0 )
      {
          M_state_map["version"] = State::Ptr( new StateVersion( *this ) );
          M_state_map["timestamp"] = State::Ptr( new StateTimeStamp( *this ) );

      }

    void clearState()
      {
          M_state.reset();
      }

    bool key( string_t & val ) override
      {
          if ( M_state )
          {
              return M_state->onKey( val );
          }

          std::unordered_map< std::string, std::shared_ptr< State > >::iterator it = M_state_map.find( val );
          if ( it != M_state_map.end() )
          {
              M_state = it->second;
              std::cerr << "(key) new state " << it->first << std::endl;
          }

          return true;
      }

    bool null() override
      {
          if ( M_state )
          {
              return M_state->onNull();
          }
          return true;
      }

    bool boolean( bool val ) override
      {
          if ( M_state )
          {
              return M_state->onBoolean( val );
          }
          return true;
      }

    bool number_integer( number_integer_t val ) override
      {
          if ( M_state )
          {
              return M_state->onInteger( val );
          }
          return true;
      }

    bool number_unsigned( number_unsigned_t val ) override
      {
          if ( M_state )
          {
              return M_state->onUnsigned( val );
          }
          return true;
      }

    bool number_float( number_float_t val,
                       const string_t & /*s*/ ) override
      {
          if ( M_state )
          {
              return M_state->onFloat( val );
          }

          return true;
      }

    bool string( string_t & val ) override
      {
          if ( M_state )
          {
              return M_state->onString( val );
          }
          return true;
      }

    bool start_object( std::size_t elements ) override
      {
          if ( M_state )
          {
              return M_state->onStartObject( elements );
          }
          return true;
      }

    bool end_object() override
      {
          if ( M_state )
          {
              if ( ! M_state->onEndObject() )
              {
                  return false;
              }

              if ( M_state->isFinished() )
              {
                  clearState();
              }
          }
          return true;
      }

    bool start_array( std::size_t elements ) override
      {
          if ( M_state )
          {
              return M_state->onStartArray( elements );
          }
          return true;
      }

    bool end_array() override
      {
          if ( M_state )
          {
              return M_state->onEndArray();
          }
          return true;
      }

    bool binary( nlohmann::json::binary_t & /*val*/ ) override
      {
          return false;
      }


    bool parse_error( std::size_t position,
                      const std::string & last_token,
                      const nlohmann::json::exception & ex ) override
      {
          std::cerr << ex.what()
                    << " pos=" << position
                    << " last_token=" << last_token
                    << std::endl;
          return false;
      }
};

//
// class ParserJSON
//

/*-------------------------------------------------------------------*/
ParserJSON::ParserJSON()
{
    std::cerr << "(ParserJSON) create" << std::endl;
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

    std::cerr << "(ParserJSON::parse)" << std::endl;
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


namespace {
Parser::Ptr
create_json()
{
    Parser::Ptr ptr( new ParserJSON() );
    return ptr;
}

rcss::RegHolder vjson = Parser::creators().autoReg( &create_json, REC_VERSION_JSON );
}

} // end of namespace
} // end of namespace