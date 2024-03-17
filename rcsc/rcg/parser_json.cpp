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
#include <variant>
#include <memory>
#include <string>

namespace rcsc {
namespace rcg {

class Context;
//using ParamValue = std::variant< int, double, bool, std::string >;


//
//
//

/*!
  \brief abstract state class for handling RCG data types.
*/
class State {
public:
    using Ptr = std::shared_ptr< State >;

private:
    State() = delete;
    State( const State & ) = delete;
    const State & operator=( const State & ) = delete;
protected:
    Context & M_context;

    State( Context & context )
        : M_context( context )
      { }

public:
    virtual
    ~State() = default;

    virtual bool onKey( const std::string & ) { return false; }
    virtual bool onNull() { return false; }
    virtual bool onBoolean( const bool ) { return false; }
    virtual bool onInteger( const int ) { return false; }
    virtual bool onUnsigned( const unsigned int ) { return false; }
    virtual bool onFloat( const double ) { return false; }
    virtual bool onString( const std::string & ) { return false; }
    virtual bool onStartObject( const size_t ) { return false; }
    virtual bool onEndObject() { return false; }
    virtual bool onStartArray( const size_t ) { return false; }
    virtual bool onEndArray() { return false; }
};

//
//
//

/*!

*/
class Context
    : public nlohmann::json::json_sax_t {
private:

    Handler & M_handler;

    std::shared_ptr< State > M_state;
    int M_depth;

    using StateCreator = std::function< std::shared_ptr< State >() >;
    //! key, state map
    std::unordered_map< std::string, StateCreator > M_state_map;

    Context() = delete;
    Context( const Context & ) = delete;
    const Context & operator=( const Context & ) = delete;
public:

    explicit
    Context( Handler & handler );

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

          std::unordered_map< std::string, StateCreator >::iterator it = M_state_map.find( val );
          if ( it != M_state_map.end() )
          {
              M_state = it->second();
              // std::cerr << "(key) new state " << it->first << std::endl;
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

    //
    //
    //

    void handleServerParam( const ServerParamT & param )
      {
          M_handler.handleServerParam( param );
          //param.toServerString( std::cout );
          //std::cout << std::endl;
      }

    void handlePlayerParam( const PlayerParamT & param )
      {
          M_handler.handlePlayerParam( param );
          // param.toServerString( std::cout );
          // std::cout << std::endl;
      }

    void handlePlayerType( const PlayerTypeT & param )
      {
          M_handler.handlePlayerType( param );
          // param.toServerString( std::cout );
          // std::cout << std::endl;
      }

    void handleTeamGraphic( const SideID side,
                            const int x,
                            const int y,
                            const std::vector< std::string > & xpm_data )
      {
          M_handler.handleTeamGraphic( side, x, y, xpm_data );
          // std::cout << "team_graphic " << side_char( side )
          //           << " (" << x << ',' << y << ") ";
          // for ( const std::string & s : xpm_data )
          // {
          //     std::cout << ' ' << '"' << s << '"';
          // }
          // std::cout << std::endl;
      }

    void handlePlaymode( const int time,
                         const int stime,
                         const std::string & playmode )
      {
          // M_handler.handlePlayMode( time, playmode );
          std::cout << '[' << time << ',' << stime << ']'
                    << " playmode " << playmode << std::endl;
      }
};


//
//
//

/*!

*/
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
          M_context.clearState();
          return true;
      }

};

//
//
//

/*!

*/
class StateTimeStamp
    : public State {
public:
    StateTimeStamp( Context & context )
        : State( context )
      { }

    bool onString( const std::string & val ) override
      {
          std::cerr << "(StateVersion::onString) " << val << std::endl;
          return true;
      }

    bool onEndObject() override
      {
          M_context.clearState();
          return true;
      }

};


//
//
//

class StateServerParam
    : public State {
private:
    int M_depth;
    std::string M_param_name;

    ServerParamT M_param;

public:

    StateServerParam( Context & context )
        : State( context ),
          M_depth( 0 )
      { }

    bool onStartObject( const size_t ) override
      {
          ++M_depth;
          return true;
      }

    bool onEndObject() override
      {
          if ( M_depth > 0 )
          {
              --M_depth;
          }

          if ( M_depth == 0 )
          {
              M_context.handleServerParam( M_param );
              M_context.clearState();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(StateServerParam::onKey) ERROR depth " << M_depth
                        << " val=" << val << std::endl;
              return false;
          }
          M_param_name = val;
          return true;
      }

    bool onBoolean( const bool val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StateServerParam::onBoolean) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setBool( M_param_name, val );
          M_param_name.clear();
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StateServerParam::onInteger) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setInt( M_param_name, val );
          M_param_name.clear();
          return true;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StateServerParam::onUnsingned) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setInt( M_param_name, static_cast< int >( val ) );
          M_param_name.clear();
          return true;
      }

    bool onFloat( const double val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StateServerParam::onFloat) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setDouble( M_param_name, val );
          M_param_name.clear();
          return true;
      }

    bool onString( const std::string & val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StateServerParam::onString) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setString( M_param_name, val );
          M_param_name.clear();
          return true;
      }
};


//
//
//

class StatePlayerParam
    : public State {
private:
    int M_depth;
    std::string M_param_name;

    PlayerParamT M_param;

public:

    StatePlayerParam( Context & context )
        : State( context ),
          M_depth( 0 )
      { }

    bool onStartObject( const size_t ) override
      {
          ++M_depth;
          return true;
      }

    bool onEndObject() override
      {
          if ( M_depth > 0 )
          {
              --M_depth;
          }

          if ( M_depth == 0 )
          {
              M_context.handlePlayerParam( M_param );
              M_context.clearState();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(StatePlayerParam::onKey) ERROR depth " << M_depth
                        << " val=" << val << std::endl;
              return false;
          }
          M_param_name = val;
          //std::cerr << "(StateServerParam::onKey) " << val << std::endl;
          return true;
      }

    bool onBoolean( const bool val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerParam::onBoolean) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setBool( M_param_name, val );
          M_param_name.clear();
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerParam::onInteger) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setInt( M_param_name, val );
          M_param_name.clear();
          return true;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerParam::onUnsingned) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setInt( M_param_name, static_cast< int >( val ) );
          M_param_name.clear();
          return true;
      }

    bool onFloat( const double val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerParam::onFloat) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setDouble( M_param_name, val );
          M_param_name.clear();
          return true;
      }
};

//
//
//

class StatePlayerType
    : public State {
private:
    int M_depth;
    std::string M_param_name;

    PlayerTypeT M_param;
public:

    StatePlayerType( Context & context )
        : State( context ),
          M_depth( 0 )
      { }

    bool onStartObject( const size_t ) override
      {
          ++M_depth;
          return true;
      }

    bool onEndObject() override
      {
          if ( M_depth > 0 )
          {
              --M_depth;
          }

          if ( M_depth == 0 )
          {
              M_context.handlePlayerType( M_param );
              M_context.clearState();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(StatePlayerType::onKey) ERROR depth " << M_depth
                        << " val=" << val << std::endl;
              return false;
          }

          M_param_name = val;
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerType::onInteger) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setInt( M_param_name, val );
          M_param_name.clear();
          return true;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerType::onUnsingned) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setInt( M_param_name, static_cast< int >( val ) );
          M_param_name.clear();
          return true;
      }

    bool onFloat( const double val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(StatePlayerType::onFloat) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_param.setDouble( M_param_name, val );
          M_param_name.clear();
          return true;
      }
};


//
//
//

class StateTeamGraphic
    : public State {
private:
    std::string M_key;
    int M_depth;
    bool M_in_array;

    // xpm tile information
    SideID M_side;
    int M_x;
    int M_y;
    std::vector< std::string > M_xpm_data;
public:

    StateTeamGraphic( Context & context )
        : State( context ),
          M_depth( 0 ),
          M_in_array( false ),
          M_side( NEUTRAL ),
          M_x( -1 ),
          M_y( -1 )
      { }

    bool onStartObject( const size_t ) override
      {
          ++M_depth;
          return true;
      }

    bool onEndObject() override
      {
          if ( M_depth > 0 )
          {
              --M_depth;
          }

          if ( M_depth == 0 )
          {
              M_context.handleTeamGraphic( M_side, M_x, M_y, M_xpm_data );
              M_context.clearState();
          }

          return true;
      }

    bool onStartArray( const size_t )
      {
          if ( M_key == "xpm" )
          {
              M_in_array = true;
              return true;
          }

          std::cerr << "(StateTeamGraphic::onStartArray) ERROR unknown key=" << M_key << std::endl;
          return false;
      }

    bool onEndArray()
      {
          M_in_array = false;
          M_key.clear();
          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(StateTeamGraphic::onKey) ERROR depth " << M_depth
                        << " val=" << val << std::endl;
              return false;
          }

          M_key = val;
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_key == "x" )
          {
              M_x = val;
              M_key.clear();
              return true;
          }

          if ( M_key == "y" )
          {
              M_y = val;
              M_key.clear();
              return true;
          }

          std::cerr << "(StateTeamGraphic::onInteger) ERROR. unknown key=" << M_key << std::endl;
          return false;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          return onInteger( static_cast< int >( val ) );
      }

    bool onString( const std::string & val ) override
      {
          if ( M_key == "xpm" )
          {
              if ( ! M_in_array )
              {
                  std::cerr << "(StateServerParam::onString) ERROR. illegal xpm state. " << val << std::endl;
                  return false;
              }

              M_xpm_data.push_back( val );
              return true;
          }

          if ( M_key == "side" )
          {
              M_side = ( val == "l" ? LEFT : val == "r" ? RIGHT : NEUTRAL );
              M_key.clear();
              return true;
          }

          std::cerr << "(StateServerParam::onString) ERROR. unknown key " << M_key << std::endl;
          return false;
      }

};

//
//
//

class PlaymodeState
    : public State {
private:
    std::string M_key;
    int M_depth;

    int M_time;
    int M_stime;
    std::string M_playmode;
public:
    PlaymodeState( Context & context )
        : State( context ),
          M_depth( 0 ),
          M_time( 0 ),
          M_stime( 0 )
      { }

    bool onStartObject( const size_t ) override
      {
          ++M_depth;
          return true;
      }

    bool onEndObject() override
      {
          if ( M_depth > 0 )
          {
              --M_depth;
          }

          if ( M_depth == 0 )
          {
              M_context.handlePlaymode( M_time, M_stime, M_playmode );
              M_context.clearState();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(PlaymodeState::onKey) ERROR depth " << M_depth
                        << " val=" << val << std::endl;
              return false;
          }

          M_key = val;
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_key.empty() )
          {
              std::cerr << "(PlaymodeState::onInteger) ERROR no name. val=" << val << std::endl;
              return false;
          }

          if ( M_key == "time" )
          {
              M_time = val;
              M_key.clear();
              return true;
          }

          if ( M_key == "stime" )
          {
              M_stime = val;
              M_key.clear();
              return true;
          }

          M_key.clear();
          return false;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          return onInteger( static_cast< int >( val ) );
      }

    bool onString( const std::string & val ) override
      {
          if ( M_key.empty() )
          {
              std::cerr << "(PlaymodeState::onString) ERROR no name. val=" << val << std::endl;
              return false;
          }

          M_playmode = val;
          M_key.clear();
          return true;
      }
};

//
//
//

/*-------------------------------------------------------------------*/
Context::Context( Handler & handler )
    : M_handler( handler ),
      M_depth( 0 )
{
    M_state_map["version"] = [this]() { return State::Ptr( new StateVersion( *this ) ); };
    M_state_map["timestamp"] = [this]() { return State::Ptr( new StateTimeStamp( *this ) ); };
    M_state_map["server_param"] = [this]() { return State::Ptr( new StateServerParam( *this ) ); };
    M_state_map["player_param"] = [this]() { return State::Ptr( new StatePlayerParam( *this ) ); };
    M_state_map["player_type"] = [this]() { return State::Ptr( new StatePlayerType( *this ) ); };
    M_state_map["team_graphic"] = [this]() { return State::Ptr( new StateTeamGraphic( *this ) ); };
    M_state_map["playmode"] = [this]() { return State::Ptr( new PlaymodeState( *this ) ); };
}

/*-------------------------------------------------------------------*/


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
