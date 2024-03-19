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
#include "util.h"

#include "nlohmann/json.hpp"

#include <iostream>
#include <stack>
#include <unordered_map>
#include <variant>
#include <memory>
#include <string>
#include <functional>

namespace rcsc {
namespace rcg {

// using PlayerValue = std::variant< int, double, std::string >;
// using PlayerSetter = std::function< void( PlayerT &, const PlayerValue & ) >;

static const std::unordered_map< std::string, std::function< void( PlayerT &, const int ) > > player_setters_int = {
    { "unum", []( PlayerT & p, const int val ) { p.unum_ = val; } },
    { "type", []( PlayerT & p, const int val ) { p.type_ = val; } },
    { "state", []( PlayerT & p, const int val ) { p.state_ = val; } },
    { "x", []( PlayerT & p, const int val ) { p.x_ = static_cast< float >( val ); } },
    { "y", []( PlayerT & p, const int val ) { p.y_ = static_cast< float >( val ); } },
    { "vx", []( PlayerT & p, const int val ) { p.vx_ = static_cast< float >( val ); } },
    { "vy", []( PlayerT & p, const int val ) { p.vy_ = static_cast< float >( val ); } },
    { "body", []( PlayerT & p, const int val ) { p.body_ = static_cast< float >( val ); } },
    { "neck", []( PlayerT & p, const int val ) { p.neck_ = static_cast< float >( val ); } },
    { "px", []( PlayerT & p, const int val ) { p.point_x_ = static_cast< float >( val ); } },
    { "py", []( PlayerT & p, const int val ) { p.point_y_ = static_cast< float >( val ); } },
    { "vw", []( PlayerT & p, const int val ) { p.view_width_ = static_cast< float >( val ); } },
    { "fdist", []( PlayerT & p, const int val ) { p.focus_dist_ = static_cast< float >( val ); } },
    { "fdir", []( PlayerT & p, const int val ) { p.focus_dir_ = static_cast< float >( val ); } },
    { "stamina", []( PlayerT & p, const int val ) { p.stamina_ = static_cast< float >( val ); } },
    { "effort", []( PlayerT & p, const int val ) { p.effort_ = static_cast< float >( val ); } },
    { "recovery", []( PlayerT & p, const int val ) { p.recovery_ = static_cast< float >( val ); } },
    { "capacity", []( PlayerT & p, const int val ) { p.stamina_capacity_ = static_cast< float >( val ); } },
    { "kick", []( PlayerT & p, const int val ) { p.kick_count_ = val; } },
    { "dash", []( PlayerT & p, const int val ) { p.dash_count_ = val; } },
    { "turn", []( PlayerT & p, const int val ) { p.turn_count_ = val; } },
    { "catch", []( PlayerT & p, const int val ) { p.catch_count_ = val; } },
    { "move", []( PlayerT & p, const int val ) { p.move_count_ = val; } },
    { "turn_neck", []( PlayerT & p, const int val ) { p.turn_neck_count_ = val; } },
    { "change_view", []( PlayerT & p, const int val ) { p.change_view_count_ = val; } },
    { "say", []( PlayerT & p, const int val ) { p.say_count_ = val; } },
    { "tackle", []( PlayerT & p, const int val ) { p.tackle_count_ = val; } },
    { "pointto", []( PlayerT & p, const int val ) { p.pointto_count_ = val; } },
    { "attentionto", []( PlayerT & p, const int val ) { p.attentionto_count_ = val; } },
    { "change_focus", []( PlayerT & p, const int val ) { p.change_focus_count_ = val; } },
};

static const std::unordered_map< std::string, std::function< void( PlayerT &, const double ) > > player_setters_float = {
    { "x", []( PlayerT & p, const double val ) { p.x_ = static_cast< float >( val ); } },
    { "y", []( PlayerT & p, const double val ) { p.y_ = static_cast< float >( val ); } },
    { "vx", []( PlayerT & p, const double val ) { p.vx_ = static_cast< float >( val ); } },
    { "vy", []( PlayerT & p, const double val ) { p.vy_ = static_cast< float >( val ); } },
    { "body", []( PlayerT & p, const double val ) { p.body_ = static_cast< float >( val ); } },
    { "neck", []( PlayerT & p, const double val ) { p.neck_ = static_cast< float >( val ); } },
    { "px", []( PlayerT & p, const double val ) { p.point_x_ = static_cast< float >( val ); } },
    { "py", []( PlayerT & p, const double val ) { p.point_y_ = static_cast< float >( val ); } },
    { "vw", []( PlayerT & p, const double val ) { p.view_width_ = static_cast< float >( val ); } },
    { "fdist", []( PlayerT & p, const double val ) { p.focus_dist_ = static_cast< float >( val ); } },
    { "fdir", []( PlayerT & p, const double val ) { p.focus_dir_ = static_cast< float >( val ); } },
    { "stamina", []( PlayerT & p, const double val ) { p.stamina_ = static_cast< float >( val ); } },
    { "effort", []( PlayerT & p, const double val ) { p.effort_ = static_cast< float >( val ); } },
    { "recovery", []( PlayerT & p, const double val ) { p.recovery_ = static_cast< float >( val ); } },
    { "capacity", []( PlayerT & p, const double val ) { p.stamina_capacity_ = static_cast< float >( val ); } },
};

static const std::unordered_map< std::string, std::function< void( PlayerT &, const std::string & ) > > player_setters_string = {
    { "side", []( PlayerT & p, const std::string & val ) { p.side_ = val[0]; } },
    { "vq", []( PlayerT & p, const std::string & val ) { p.view_quality_ = val[0]; } },
};



class Context;
//using ParamValue = std::variant< int, double, bool, std::string >;


//
//
//

/*!
  \brief abstract state class for handling RCG data types.
*/
class Builder {
public:
    using Ptr = std::shared_ptr< Builder >;

private:
    Builder() = delete;
    Builder( const Builder & ) = delete;
    const Builder & operator=( const Builder & ) = delete;
protected:
    Context & M_context;
    Builder * M_parent;
    Ptr M_child;

    Builder( Context & context )
        : M_context( context ),
          M_parent( nullptr )
      { }


    Builder( Context & context,
             Builder * parent )
        : M_context( context ),
          M_parent( parent )
      { }

public:
    virtual
    ~Builder() = default;

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

    void clearChild()
      {
          M_child.reset();
      }
};

//
//
//

// class RCGBuilder
//     : public Builder {


// };

// };

//
//
//

/*!

*/
class Context
    : public nlohmann::json::json_sax_t {
private:

    Handler & M_handler;

    std::shared_ptr< Builder > M_builder;
    int M_depth;

    using BuilderCreator = std::function< std::shared_ptr< Builder >() >;
    //! key, creator
    std::unordered_map< std::string, BuilderCreator > M_builder_map;

    Context() = delete;
    Context( const Context & ) = delete;
    const Context & operator=( const Context & ) = delete;
public:

    explicit
    Context( Handler & handler );

    void clearBuilder()
      {
          M_builder.reset();
      }

    bool key( string_t & val ) override
      {
          if ( M_builder )
          {
              return M_builder->onKey( val );
          }

          std::unordered_map< std::string, BuilderCreator >::iterator it = M_builder_map.find( val );
          if ( it != M_builder_map.end() )
          {
              M_builder = it->second();
              return true;
          }

          std::cerr << "Unsupported key " << val << std::endl;
          return false;
      }

    bool null() override
      {
          if ( M_builder )
          {
              return M_builder->onNull();
          }
          return true;
      }

    bool boolean( bool val ) override
      {
          if ( M_builder )
          {
              return M_builder->onBoolean( val );
          }
          return true;
      }

    bool number_integer( number_integer_t val ) override
      {
          if ( M_builder )
          {
              return M_builder->onInteger( val );
          }
          return true;
      }

    bool number_unsigned( number_unsigned_t val ) override
      {
          if ( M_builder )
          {
              return M_builder->onUnsigned( val );
          }
          return true;
      }

    bool number_float( number_float_t val,
                       const string_t & /*s*/ ) override
      {
          if ( M_builder )
          {
              return M_builder->onFloat( val );
          }

          return true;
      }

    bool string( string_t & val ) override
      {
          if ( M_builder )
          {
              return M_builder->onString( val );
          }
          return true;
      }

    bool start_object( std::size_t elements ) override
      {
          if ( M_builder )
          {
              return M_builder->onStartObject( elements );
          }
          return true;
      }

    bool end_object() override
      {
          if ( M_builder )
          {
              if ( ! M_builder->onEndObject() )
              {
                  return false;
              }
          }
          return true;
      }

    bool start_array( std::size_t elements ) override
      {
          if ( M_builder )
          {
              return M_builder->onStartArray( elements );
          }
          return true;
      }

    bool end_array() override
      {
          if ( M_builder )
          {
              return M_builder->onEndArray();
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
                         const int /*stime*/,
                         const std::string & playmode )
      {
          M_handler.handlePlayMode( time, playmode );
          // std::cout << '[' << time << ',' << stime << ']'
          //           << " playmode " << playmode << std::endl;
      }

    void handleTeam( const int time,
                     const int /*stime*/,
                     const TeamT & team_l,
                     const TeamT & team_r )
      {
          M_handler.handleTeam( time, team_l, team_r );
          // std::cout << "handleTeam " << time
          //           << " " << team_l.name_ << " " << team_l.score_
          //           << " " << team_r.name_ << " " << team_r.score_ << std::endl;
      }

    void handleShow( const ShowInfoT & show )
      {
          M_handler.handleShow( show );
      }

    void handleMsg( const int time,
                    const int board,
                    const std::string & message )
      {
          M_handler.handleMsg( time, board, message );
          //std::cerr << "handleMsg [" << message << "]" << std::endl;
      }


};


//
//
//

/*!

*/
class VersionBuilder
    : public Builder {
public:
    VersionBuilder( Context & context )
        : Builder( context )
      { }

    bool onString( const std::string & val ) override
      {
          std::cerr << "(VersionBuilder::onString) " << val << std::endl;
          return true;
      }

    bool onEndObject() override
      {
          M_context.clearBuilder();
          return true;
      }

};

//
//
//

/*!

*/
class TimeStampBuilder
    : public Builder {
public:
    TimeStampBuilder( Context & context )
        : Builder( context )
      { }

    bool onString( const std::string & val ) override
      {
          std::cerr << "(VersionBuilder::onString) " << val << std::endl;
          return true;
      }

    bool onEndObject() override
      {
          M_context.clearBuilder();
          return true;
      }

};


//
//
//

class ServerParamBuilder
    : public Builder {
private:
    int M_depth;
    std::string M_param_name;

    ServerParamT M_param;

public:

    ServerParamBuilder( Context & context )
        : Builder( context ),
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
              M_context.clearBuilder();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(ServerParamBuilder::onKey) ERROR depth " << M_depth
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
              std::cerr << "(ServerParamBuilder::onBoolean) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(ServerParamBuilder::onInteger) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(ServerParamBuilder::onUnsingned) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(ServerParamBuilder::onFloat) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(ServerParamBuilder::onString) ERROR no name. val=" << val << std::endl;
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

class PlayerParamBuilder
    : public Builder {
private:
    int M_depth;
    std::string M_param_name;

    PlayerParamT M_param;

public:

    PlayerParamBuilder( Context & context )
        : Builder( context ),
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
              M_context.clearBuilder();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(PlayerParamBuilder::onKey) ERROR depth " << M_depth
                        << " val=" << val << std::endl;
              return false;
          }
          M_param_name = val;
          //std::cerr << "(ServerParamBuilder::onKey) " << val << std::endl;
          return true;
      }

    bool onBoolean( const bool val ) override
      {
          if ( M_param_name.empty() )
          {
              std::cerr << "(PlayerParamBuilder::onBoolean) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(PlayerParamBuilder::onInteger) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(PlayerParamBuilder::onUnsingned) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(PlayerParamBuilder::onFloat) ERROR no name. val=" << val << std::endl;
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

class PlayerTypeBuilder
    : public Builder {
private:
    int M_depth;
    std::string M_param_name;

    PlayerTypeT M_param;
public:

    PlayerTypeBuilder( Context & context )
        : Builder( context ),
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
              M_context.clearBuilder();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(PlayerTypeBuilder::onKey) ERROR depth " << M_depth
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
              std::cerr << "(PlayerTypeBuilder::onInteger) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(PlayerTypeBuilder::onUnsingned) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(PlayerTypeBuilder::onFloat) ERROR no name. val=" << val << std::endl;
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

class TeamGraphicBuilder
    : public Builder {
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

    TeamGraphicBuilder( Context & context )
        : Builder( context ),
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
              M_context.clearBuilder();
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

          std::cerr << "(TeamGraphicBuilder::onStartArray) ERROR unknown key=" << M_key << std::endl;
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
              std::cerr << "(TeamGraphicBuilder::onKey) ERROR depth " << M_depth
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

          std::cerr << "(TeamGraphicBuilder::onInteger) ERROR. unknown key=" << M_key << std::endl;
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
                  std::cerr << "(ServerParamBuilder::onString) ERROR. illegal xpm builder. " << val << std::endl;
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

          std::cerr << "(ServerParamBuilder::onString) ERROR. unknown key " << M_key << std::endl;
          return false;
      }

};


//
//
//

class PlaymodeBuilder
    : public Builder {
private:
    std::string M_key;
    int M_depth;

    int M_time;
    int M_stime;
    std::string M_playmode;
public:
    PlaymodeBuilder( Context & context )
        : Builder( context ),
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
              M_context.clearBuilder();
          }

          return true;
      }

    bool onKey( const std::string & val ) override
      {
          if ( M_depth != 1 )
          {
              std::cerr << "(PlaymodeBuilder::onKey) ERROR depth " << M_depth
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
              std::cerr << "(PlaymodeBuilder::onInteger) ERROR no name. val=" << val << std::endl;
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
              std::cerr << "(PlaymodeBuilder::onString) ERROR no name. val=" << val << std::endl;
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

class ShowBuilder
    : public Builder {
private:
    std::string M_key;

    DispInfoT M_disp;
public:
    ShowBuilder( Context & context )
        : Builder( context )
      { }

    bool onKey( const std::string & val ) override;

    bool onNull() override
      {
          if ( M_child )
          {
              bool result = M_child->onNull();
              M_key.clear();
              return result;
          }
          return true;
      }

    bool onBoolean( const bool val ) override
      {
          if ( M_child )
          {
              bool result = M_child->onBoolean( val );
              M_key.clear();
              return result;
          }
          M_key.clear();
          return false;
      }

    bool onInteger( const int val ) override
      {
          if ( M_child )
          {
              return M_child->onInteger( val );
          }

          if ( M_key == "time" )
          {
              //std::cerr << "(ShowBuilder) time = " << val << std::endl;
              M_disp.show_.time_ = val;
              M_key.clear();
              return true;
          }

          if ( M_key == "stime" )
          {
              M_disp.show_.stime_ = val;
              M_key.clear();
              return true;
          }

          M_key.clear();
          return false;
      }

    bool onUnsigned( const unsigned int val )
      {
          return onInteger( val );
      }

    bool onFloat( const double val ) override
      {
          if ( M_child )
          {
              return M_child->onFloat( val );
          }

          M_key.clear();
          return false;
      }

    bool onString( const std::string & val ) override
      {
          if ( M_child )
          {
              return M_child->onString( val );
          }

          if ( M_key == "mode" )
          {
              M_disp.pmode_ = to_enum( val );
              M_key.clear();
              return true;
          }

          return true;
      }

    bool onStartObject( const size_t val ) override
      {
          if ( M_child )
          {
              return M_child->onStartObject( val );
          }
          return true;
      }
    bool onEndObject() override
      {
          if ( M_child )
          {
              return M_child->onEndObject();
          }

          M_context.handleShow( M_disp.show_ );
          M_context.clearBuilder();
          //M_key.clear();
          return true;
      }

    bool onStartArray( const size_t val ) override
      {
          if ( M_child )
          {
              return M_child->onStartArray( val );
          }
          return true;
      }
    bool onEndArray() override
      {
          if ( M_child )
          {
              return M_child->onEndArray();
          }
          return true;
      }


    void clearChild()
      {
          //std::cerr << "(ShowBuilder::clearChild)" << std::endl;
          M_child.reset();
          M_key.clear();
      }
};


//
//
//

class TeamBuilder
    : public Builder {
private:

    int M_time;
    int M_stime;
    TeamT M_left_team;
    TeamT M_right_team;

    TeamT * M_current_team;

    std::string M_key;
public:
    TeamBuilder( Context & context )
        : Builder( context ),
          M_time( -1 ),
          M_stime( -1 ),
          M_current_team( nullptr )
      { }

    TeamBuilder( Context & context,
                 Builder * parent )
        : Builder( context, parent ),
          M_time( -1 ),
          M_stime( -1 ),
          M_current_team( nullptr )
      { }

    bool onKey( const std::string & val ) override
      {
          if ( val == "l" )
          {
              M_current_team = &M_left_team;
          }
          else if ( val == "r" )
          {
              M_current_team = &M_right_team;
          }

          M_key = val;
          return true;
      }

     bool onNull() override
      {
          if ( M_key == "name" )
          {
              if ( M_current_team )
              {
                  M_current_team->name_.clear();
              }
          }

          M_key.clear();
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_key == "time" )
          {
              M_time = val;
          }
          else if ( M_key == "stime" )
          {
              M_stime = val;
          }
          else if ( M_key == "score" )
          {
              if ( M_current_team )
              {
                  M_current_team->score_ = val;
              }
          }
          else if ( M_key == "pen_score" )
          {
              if ( M_current_team )
              {
                  M_current_team->pen_score_ = val;
              }
          }
          else if ( M_key == "pen_miss" )
          {
              if ( M_current_team )
              {
                  M_current_team->pen_miss_ = val;
              }
          }

          M_key.clear();
          return true;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          return onInteger( static_cast< int >( val ) );
      }

    bool onString( const std::string &  val ) override
      {
          if ( M_key == "name" )
          {
              if ( M_current_team )
              {
                  M_current_team->name_ = val;
              }
          }

          M_key.clear();
          return true;
      }

    bool onStartObject( const size_t ) override
      {
          return true;
      }

    bool onEndObject() override
      {
          if ( ! M_current_team )
          {
              M_context.handleTeam( M_time, M_stime, M_left_team, M_right_team );
              if ( M_parent )
              {
                  M_parent->clearChild();
              }
              else
              {
                  M_context.clearBuilder();
              }
          }

          if ( M_current_team )
          {
              M_current_team = nullptr;
          }

          M_key.clear();
          return true;
      }
};

//
//
//

class BallBuilder
    : public Builder {
private:
    DispInfoT * M_disp;

    std::string M_key;
public:
    BallBuilder( Context & context,
                 Builder * parent,
                 DispInfoT * disp )
        : Builder( context, parent ),
          M_disp( disp )
      { }

    bool onKey( const std::string & val ) override
      {
          //std::cerr << "(BallBuilder::onKey) " << val << std::endl;
          M_key = val;
          return true;
      }

    bool onInteger( const int val )
      {
          //std::cerr << "(BallBuilder::onInteger) " << val << std::endl;
          if ( M_key == "x" ) M_disp->show_.ball_.x_ = static_cast< double >( val );
          else if ( M_key == "y" ) M_disp->show_.ball_.y_ = static_cast< double >( val );
          else if ( M_key == "vx" ) M_disp->show_.ball_.vx_ = static_cast< double >( val );
          else if ( M_key == "vy" ) M_disp->show_.ball_.vy_ = static_cast< double >( val );
          M_key.clear();
          return true;
      }
    bool onUnsigned( const unsigned int val )
      {
          return onInteger( val );
      }

    bool onFloat( const double val ) override
      {
          //std::cerr << "(BallBuilder::onFloat) " << val << std::endl;
          if ( M_key == "x" ) M_disp->show_.ball_.x_ = static_cast< float >( val );
          else if ( M_key == "y" ) M_disp->show_.ball_.y_ = static_cast< float >( val );
          else if ( M_key == "vx" ) M_disp->show_.ball_.vx_ = static_cast< float >( val );
          else if ( M_key == "vy" ) M_disp->show_.ball_.vy_ = static_cast< float >( val );
          M_key.clear();
          return true;
      }

    bool onStartObject( const size_t ) override
      {
          //std::cerr << "(BallBuilder::onStartObject)" << std::endl;
          return true;
      }

    bool onEndObject()
      {
          if ( M_parent )
          {
              M_parent->clearChild();
          }
          return true;
      }
};

//
//
//

class PlayerArrayBuilder
    : public Builder {
private:
    DispInfoT * M_disp;

    std::string M_key;
    size_t M_index;
public:
    PlayerArrayBuilder( Context & context,
                        Builder * parent,
                        DispInfoT * disp )
        : Builder( context, parent ),
          M_disp( disp ),
          M_index( 0 )
      { }

    bool onKey( const std::string & val ) override
      {
          //std::cerr << "(PlayerBuilder::onKey) " << val << std::endl;
          M_key = val;
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_index < 1 || MAX_PLAYER*2 < M_index )
          {
              M_key.clear();
              return false;
          }

          {
              decltype( player_setters_int )::const_iterator it = player_setters_int.find( M_key );
              if ( it != player_setters_int.end() )
              {
                  it->second( M_disp->show_.player_[M_index-1], val );
                  M_key.clear();
                  return true;
              }
          }
          {
              decltype( player_setters_float )::const_iterator it = player_setters_float.find( M_key );
              if ( it != player_setters_float.end() )
              {
                  it->second( M_disp->show_.player_[M_index-1], val );
                  M_key.clear();
                  return true;
              }
          }

          M_key.clear();
          return true;
      }
    bool onUnsigned( const unsigned int val ) override
      {
          return onInteger( val );
      }

    bool onFloat( const double val ) override
      {
          if ( M_index < 1 || MAX_PLAYER*2 < M_index )
          {
              M_key.clear();
              return false;
          }

          {
              decltype( player_setters_float )::const_iterator it = player_setters_float.find( M_key );
              if ( it != player_setters_float.end() )
              {
                  it->second( M_disp->show_.player_[M_index-1], val );
                  M_key.clear();
                  return true;
              }
          }

          M_key.clear();
          return true;
      }

    bool onString( const std::string & val ) override
      {
          if ( M_index < 1 || MAX_PLAYER*2 < M_index )
          {
              M_key.clear();
              return false;
          }

          {
              decltype( player_setters_string )::const_iterator it = player_setters_string.find( M_key );
              if ( it != player_setters_string.end() )
              {
                  it->second( M_disp->show_.player_[M_index-1], val );
                  M_key.clear();
                  return true;
              }
          }

          M_key.clear();
          return true;
      }

    bool onStartObject( const size_t ) override
      {
          if ( M_key.empty() )
          {
              ++M_index;
              //std::cerr << "(PlayerArrayBuilder::onStartObject) index=" << M_index << std::endl;
              if ( M_index > MAX_PLAYER*2 )
              {
                  return false;
              }
          }
          return true;
      }
    bool onEndObject() override
      {
          //std::cerr << "(PlayerArrayBuilder::onEndObject) index=" << M_index << std::endl;
          M_key.clear();
          return true;
      }

    bool onStartArray( const size_t ) override
      {
          return true;
      }

    bool onEndArray() override
      {
          std::cerr << "(PlayerArrayBuilder::onEndArray)" << std::endl;
          if ( M_parent )
          {
              M_parent->clearChild();
          }
          return true;
      }

};


//
//
//


//
//
//

/*-------------------------------------------------------------------*/
bool
ShowBuilder::onKey( const std::string & val )
{
    if ( M_child )
    {
        std::cerr << "ShowBuilder child on key" << std::endl;
        return M_child->onKey( val );
    }

    if ( val == "ball" )
    {
        M_child = Ptr( new BallBuilder( M_context, this, &M_disp ) );
    }
    else if ( val == "players" )
    {
        M_child = Ptr( new PlayerArrayBuilder( M_context, this, &M_disp ) );
    }
    // else if ( val == "mode" )
    // {
    //     M_key = val;
    // }
    else if ( val == "team" )
    {
        M_child = Ptr( new TeamBuilder( M_context, this ) );
    }

    M_key = val;
    return true;
}

//
//
//

class MsgBuilder
    : public Builder {
private:

    std::string M_key;
    int M_time;
    int M_stime;
    int M_board;
    std::string M_message;
public:
    MsgBuilder( Context & context )
        : Builder( context ),
          M_time( 0 ),
          M_stime( 0 ),
          M_board( 0 )
      { }

    bool onKey( const std::string & val ) override
      {
          M_key = val;
          return true;
      }

    bool onInteger( const int val ) override
      {
          if ( M_key == "time" )
          {
              M_time = val;
          }
          else if ( M_key == "stime" )
          {
              M_stime = val;
          }
          else if ( M_key == "board" )
          {
              M_board = val;
          }

          M_key.clear();
          return true;
      }

    bool onUnsigned( const unsigned int val ) override
      {
          return onInteger( val );
      }

    bool onString( const std::string & val ) override
      {
          if ( M_key == "message" )
          {
              M_message = val;
          }

          M_key.clear();
          return true;
      }

    bool onStartObject( const size_t ) override
      {
          return true;
      }

    bool onEndObject() override
      {
          M_context.handleMsg( M_time, M_board, M_message );
          M_context.clearBuilder();
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
    M_builder_map["version"] = [this]() { return Builder::Ptr( new VersionBuilder( *this ) ); };
    M_builder_map["timestamp"] = [this]() { return Builder::Ptr( new TimeStampBuilder( *this ) ); };
    M_builder_map["server_param"] = [this]() { return Builder::Ptr( new ServerParamBuilder( *this ) ); };
    M_builder_map["player_param"] = [this]() { return Builder::Ptr( new PlayerParamBuilder( *this ) ); };
    M_builder_map["player_type"] = [this]() { return Builder::Ptr( new PlayerTypeBuilder( *this ) ); };
    M_builder_map["team_graphic"] = [this]() { return Builder::Ptr( new TeamGraphicBuilder( *this ) ); };
    M_builder_map["playmode"] = [this]() { return Builder::Ptr( new PlaymodeBuilder( *this ) ); };
    M_builder_map["team"] = [this]() { return Builder::Ptr( new TeamBuilder( *this ) ); };
    M_builder_map["show"] = [this]() { return Builder::Ptr( new ShowBuilder( *this ) ); };
    M_builder_map["msg"] = [this]() { return Builder::Ptr( new MsgBuilder( *this ) ); };
}

/*-------------------------------------------------------------------*/


//
// class ParserJSON
//

/*-------------------------------------------------------------------*/
ParserJSON::ParserJSON()
{
    // std::cerr << "(ParserJSON) create" << std::endl;
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
    //std::cerr << "(ParserJSON::parseData) " << input << std::endl;
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
