// -*-c++-*-

/*!
  \file test_parser.cpp
  \brief test code for the rcg parsers
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "handler.h"
#include "parser.h"
#include "parser_v4.h"
#include "parser_simdjson.h"
#include "types.h"

#include <rcsc/gz/gzfstream.h>
#include <rcsc/gz/gzread_file.h>

#include <cppunit/extensions/HelperMacros.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#ifndef TEST_DATA_DIR
#error "TEST_DATA_DIR is not defined"
#endif

using namespace rcsc;
using namespace rcsc::rcg;

namespace {

/*!
  \brief the path of a test data file (the first records of real game logs)

  head_v5.rcg and head_json.rcg are the same files as the test data of rcssmonitor.
  head_v6.rcg is made from the first records of a ULG6 game log.
  See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.1.
*/
std::string
data_path( const std::string & name )
{
    return std::string( TEST_DATA_DIR ) + "/" + name;
}

std::string
read_file( const std::string & path )
{
    std::ifstream fin( path, std::ios_base::in | std::ios_base::binary );
    std::ostringstream os;
    os << fin.rdbuf();
    return os.str();
}

std::string
replace_all( std::string str,
             const std::string & from,
             const std::string & to )
{
    for ( std::string::size_type pos = str.find( from );
          pos != std::string::npos;
          pos = str.find( from, pos + to.length() ) )
    {
        str.replace( pos, from.length(), to );
    }
    return str;
}

/*!
  \brief the records of head_json.rcg, without the "[" and "]" lines and the commas at the ends
*/
std::vector< std::string >
json_records()
{
    std::vector< std::string > records;
    std::istringstream is( read_file( data_path( "head_json.rcg" ) ) );
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( line == "[" || line == "]" ) continue;
        if ( ! line.empty() && line.back() == ',' ) line.pop_back();
        records.push_back( line );
    }
    return records;
}

/*!
  \brief a temporary file, removed by the destructor
*/
class TempFile {
private:
    std::string M_path;
public:
    explicit
    TempFile( const std::string & name )
        : M_path( ( std::filesystem::temp_directory_path()
                    / ( "rcsc_test_parser_" + std::to_string( ::getpid() ) + "_" + name ) ).string() )
      { }

    ~TempFile()
      {
          std::error_code ec;
          std::filesystem::remove( M_path, ec );
      }

    const std::string & path() const
      {
          return M_path;
      }
};

/*!
  \brief get the first line that starts with prefix in the file
*/
std::string
find_line( const std::string & path,
           const std::string & prefix )
{
    std::ifstream fin( path );
    std::string line;
    while ( std::getline( fin, line ) )
    {
        if ( ! line.compare( 0, prefix.length(), prefix ) )
        {
            return line;
        }
    }
    return std::string();
}

/*!
  \brief a handler that records what the parser passes, in order
*/
class RecordHandler
    : public Handler {
public:
    //! the events in order, e.g. "show 1", "playmode 5 2"
    std::vector< std::string > events_;

    std::vector< ShowInfoT > shows_;
    std::vector< std::string > msgs_;
    std::vector< std::vector< std::string > > team_graphics_;
    double ball_speed_max_ = 0.0; //!< a value of server_param
    int player_types_ = 0; //!< a value of player_param
    std::map< int, double > stamina_inc_max_; //!< the values of player_type, by the id

    bool show_result_ = true; //!< the value returned by handleShow()
    bool log_version_result_ = true; //!< the value returned by handleLogVersion()

    bool handleLogVersion( const int ver ) override
      {
          events_.push_back( "log_version " + std::to_string( ver ) );
          return Handler::handleLogVersion( ver )
              && log_version_result_;
      }

    bool handleEOF() override
      {
          events_.push_back( "eof" );
          return true;
      }

    bool handleShow( const ShowInfoT & show ) override
      {
          events_.push_back( "show " + std::to_string( show.time_ ) );
          shows_.push_back( show );
          return show_result_;
      }

    bool handleMsg( const int time,
                    const int board,
                    const std::string & msg ) override
      {
          events_.push_back( "msg " + std::to_string( time ) + ' ' + std::to_string( board ) );
          msgs_.push_back( msg );
          return true;
      }

    bool handleDraw( const int time,
                     const drawinfo_t & ) override
      {
          events_.push_back( "draw " + std::to_string( time ) );
          return true;
      }

    bool handlePlayMode( const int time,
                         const PlayMode pm ) override
      {
          events_.push_back( "playmode " + std::to_string( time ) + ' ' + std::to_string( pm ) );
          return true;
      }

    bool handleTeam( const int time,
                     const TeamT & team_l,
                     const TeamT & team_r ) override
      {
          events_.push_back( "team " + std::to_string( time )
                             + ' ' + team_l.name_ + ' ' + team_r.name_
                             + ' ' + std::to_string( team_l.score_ )
                             + ' ' + std::to_string( team_r.score_ ) );
          return true;
      }

    bool handleServerParam( const ServerParamT & param ) override
      {
          events_.push_back( "server_param" );
          ball_speed_max_ = param.ball_speed_max_;
          return true;
      }

    bool handlePlayerParam( const PlayerParamT & param ) override
      {
          events_.push_back( "player_param" );
          player_types_ = param.player_types_;
          return true;
      }

    bool handlePlayerType( const PlayerTypeT & param ) override
      {
          events_.push_back( "player_type " + std::to_string( param.id_ ) );
          stamina_inc_max_[param.id_] = param.stamina_inc_max_;
          return true;
      }

    bool handleTeamGraphic( const char side,
                            const int x,
                            const int y,
                            const std::vector< std::string > & xpm_data ) override
      {
          events_.push_back( std::string( "team_graphic " ) + side
                             + ' ' + std::to_string( x ) + ' ' + std::to_string( y )
                             + ' ' + std::to_string( xpm_data.size() ) );
          team_graphics_.push_back( xpm_data );
          return true;
      }
};

/*!
  \brief the events of the parameters at the beginning of a game log
*/
std::vector< std::string >
header_events( const int log_version )
{
    std::vector< std::string > events;
    events.push_back( "log_version " + std::to_string( log_version ) );
    events.push_back( "server_param" );
    events.push_back( "player_param" );
    for ( int id = 0; id < 18; ++id )
    {
        events.push_back( "player_type " + std::to_string( id ) );
    }
    return events;
}

std::string
playmode_event( const int time,
                const PlayMode pm )
{
    return "playmode " + std::to_string( time ) + ' ' + std::to_string( pm );
}

void
assert_events( const std::vector< std::string > & expected,
               const std::vector< std::string > & actual )
{
    for ( size_t i = 0; i < expected.size() && i < actual.size(); ++i )
    {
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "event " + std::to_string( i ),
                                      expected[i], actual[i] );
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE( "the number of events",
                                  expected.size(), actual.size() );
}

/*!
  \brief parse the file by the parser that Parser::create() chooses for the stream
*/
bool
parse_stream( const std::string & path,
              const int expected_parser_version,
              RecordHandler & handler )
{
    std::ifstream fin( path, std::ios_base::in | std::ios_base::binary );
    CPPUNIT_ASSERT_MESSAGE( path, fin.is_open() );

    Parser::Ptr parser = Parser::create( fin );
    CPPUNIT_ASSERT( parser );
    CPPUNIT_ASSERT_EQUAL( expected_parser_version, parser->version() );

    return parser->parse( fin, handler );
}

/*!
  \brief parse the data in memory by the parser that Parser::create( const char *, std::size_t ) chooses
*/
bool
parse_memory( const std::string & data,
              const int expected_parser_version,
              RecordHandler & handler )
{
    Parser::Ptr parser = Parser::create( data.data(), data.size() );
    CPPUNIT_ASSERT( parser );
    CPPUNIT_ASSERT_EQUAL( expected_parser_version, parser->version() );

    return parser->parse( data.data(), data.size(), handler );
}

std::vector< std::string >
v5_events()
{
    std::vector< std::string > events = header_events( REC_VERSION_5 );
    events.push_back( playmode_event( 1, PM_KickOff_Left ) );
    events.push_back( "team 1 HELIOS2022 CYRUS 0 0" );
    events.push_back( "show 1" );
    events.push_back( "show 2" );
    events.push_back( playmode_event( 4, PM_PlayOn ) );
    events.push_back( "show 4" );
    events.push_back( "eof" );
    return events;
}

std::vector< std::string >
v6_events()
{
    std::vector< std::string > events = header_events( REC_VERSION_6 );
    events.push_back( "team_graphic l 31 7 10" );
    events.push_back( "team_graphic l 31 6 20" );
    events.push_back( playmode_event( 1, PM_KickOff_Left ) );
    events.push_back( "team 1 HELIOS2026 YuShan2026 0 0" );
    events.push_back( "show 1" );
    events.push_back( "show 2" );
    events.push_back( "show 3" );
    events.push_back( "show 4" );
    events.push_back( playmode_event( 5, PM_PlayOn ) );
    events.push_back( "show 5" );
    events.push_back( "msg 6000 1" );
    events.push_back( "eof" );
    return events;
}

std::vector< std::string >
json_events()
{
    std::vector< std::string > events = header_events( REC_VERSION_JSON );
    events.push_back( playmode_event( 1, PM_KickOff_Left ) );
    events.push_back( "team 1 HELIOS2023 CYRUS 0 0" );
    events.push_back( "show 1" );
    events.push_back( "show 2" );
    events.push_back( playmode_event( 5, PM_PlayOn ) );
    events.push_back( "show 5" );
    events.push_back( "eof" );
    return events;
}

const double EPS = 1.0e-4;
//! for the values of about 10^4 (e.g. stamina), stored as float
const double LARGE_EPS = 1.0e-2;

/*!
  \brief check the values of the last show in head_v5.rcg (time 4)
*/
void
check_v5_values( const RecordHandler & handler )
{
    CPPUNIT_ASSERT_EQUAL( 18, handler.player_types_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 3.0, handler.ball_speed_max_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 43.7219, handler.stamina_inc_max_.at( 17 ), EPS );

    CPPUNIT_ASSERT_EQUAL( size_t( 3 ), handler.shows_.size() );
    const ShowInfoT & show = handler.shows_.back();
    CPPUNIT_ASSERT_EQUAL( UInt32( 4 ), show.time_ );

    // ((b) -1.577 1.074 -1.4823 1.0095)
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.577, show.ball_.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.074, show.ball_.y_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.4823, show.ball_.vx_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0095, show.ball_.vy_, EPS );

    // ((l 1) 0 0x9 -49 0 0 0 78.652 -10 (v h 180) (s 8000 1 1 130600) (f l 6) (c 0 0 538 0 1 539 1 1 0 0 3))
    const PlayerT & l1 = show.player_[0];
    CPPUNIT_ASSERT_EQUAL( 'l', l1.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 1 ), l1.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 0 ), l1.type_ );
    CPPUNIT_ASSERT_EQUAL( Int32( 0x9 ), l1.state_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -49.0, l1.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 78.652, l1.body_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -10.0, l1.neck_, EPS );
    CPPUNIT_ASSERT_EQUAL( 'h', l1.view_quality_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 180.0, l1.view_width_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 8000.0, l1.stamina_, LARGE_EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 130600.0, l1.stamina_capacity_, LARGE_EPS );
    CPPUNIT_ASSERT_EQUAL( 'l', l1.focus_side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 6 ), l1.focus_unum_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 538 ), l1.turn_count_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 539 ), l1.turn_neck_count_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 3 ), l1.attentionto_count_ );

    // ((l 2) 6 0x1 -23.9234 -10.4522 0.2669 0.1359 29.269 -90 (v h 120) (s 7888.01 0.884735 1 130512) ...)
    const PlayerT & l2 = show.player_[1];
    CPPUNIT_ASSERT_EQUAL( Int16( 2 ), l2.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 6 ), l2.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -23.9234, l2.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -10.4522, l2.y_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.2669, l2.vx_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 7888.01, l2.stamina_, LARGE_EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.884735, l2.effort_, EPS );

    // ((r 1) 0 0x9 38.1613 0.0504 -0.0497 -0.0001 -179.908 -50 -8.0663 13.7707 (v h 120) ...)
    const PlayerT & r1 = show.player_[11];
    CPPUNIT_ASSERT_EQUAL( 'r', r1.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 1 ), r1.unum_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 38.1613, r1.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -8.0663, r1.point_x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 13.7707, r1.point_y_, EPS );

    // ((r 11) 17 0x1 11.0998 10.0524 ...)
    const PlayerT & r11 = show.player_[21];
    CPPUNIT_ASSERT_EQUAL( 'r', r11.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 11 ), r11.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 17 ), r11.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 11.0998, r11.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0524, r11.y_, EPS );
}

/*!
  \brief check the values of the last show in head_v6.rcg (time 5) and the other records
*/
void
check_v6_values( const RecordHandler & handler )
{
    CPPUNIT_ASSERT_EQUAL( 18, handler.player_types_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 3.0, handler.ball_speed_max_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 50.5031, handler.stamina_inc_max_.at( 17 ), EPS );

    CPPUNIT_ASSERT_EQUAL( size_t( 2 ), handler.team_graphics_.size() );
    CPPUNIT_ASSERT_EQUAL( std::string( "8 8 11 1" ), handler.team_graphics_[1].front() );
    CPPUNIT_ASSERT_EQUAL( std::string( "  c None" ), handler.team_graphics_[1][1] );
    CPPUNIT_ASSERT_EQUAL( std::string( "{ c #C6A12D" ), handler.team_graphics_[1][2] );

    CPPUNIT_ASSERT_EQUAL( size_t( 1 ), handler.msgs_.size() );
    CPPUNIT_ASSERT_EQUAL( std::string( "(result 202607050404 HELIOS2026_0-vs-YuShan2026_1)" ),
                          handler.msgs_.front() );

    CPPUNIT_ASSERT_EQUAL( size_t( 5 ), handler.shows_.size() );
    const ShowInfoT & show = handler.shows_.back();
    CPPUNIT_ASSERT_EQUAL( UInt32( 5 ), show.time_ );

    // ((b) -1.6889 -0.9919 -1.5876 -0.9324)
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.6889, show.ball_.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -0.9919, show.ball_.y_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.5876, show.ball_.vx_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -0.9324, show.ball_.vy_, EPS );

    // ((l 1) 0 0x9 -49 0 0 0 12.34 0 (v h 180) (fp 0 0) (s 8000 1 1 130600) (f l 11) (c 0 0 549 0 1 550 1 1 0 0 4 0))
    const PlayerT & l1 = show.player_[0];
    CPPUNIT_ASSERT_EQUAL( 'l', l1.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 1 ), l1.unum_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 12.34, l1.body_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0, l1.focus_dist_, EPS );
    CPPUNIT_ASSERT_EQUAL( 'l', l1.focus_side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 11 ), l1.focus_unum_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 549 ), l1.turn_count_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 4 ), l1.attentionto_count_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 0 ), l1.change_focus_count_ );

    // ((l 2) 3 0x1 -23.1805 -14.8571 0.2596 0.0275 2.259 31 (v h 120) (fp 0 0) (s 7898.23 0.995495 1 130453) ...)
    const PlayerT & l2 = show.player_[1];
    CPPUNIT_ASSERT_EQUAL( Int16( 3 ), l2.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -23.1805, l2.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -14.8571, l2.y_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 7898.23, l2.stamina_, LARGE_EPS );

    // ((r 1) 0 0x9 37.3135 -1.0814 ...)
    const PlayerT & r1 = show.player_[11];
    CPPUNIT_ASSERT_EQUAL( 'r', r1.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 1 ), r1.unum_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 37.3135, r1.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.0814, r1.y_, EPS );

    // ((r 11) 14 0x1 11.4126 8.7987 ...)
    const PlayerT & r11 = show.player_[21];
    CPPUNIT_ASSERT_EQUAL( Int16( 11 ), r11.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 14 ), r11.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 11.4126, r11.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 8.7987, r11.y_, EPS );
}

/*!
  \brief check the values of the last show in head_json.rcg (time 5), and the version and the timestamp
*/
void
check_json_values( const RecordHandler & handler )
{
    // B8 in docs/plan/2026-11-24-02_rcg-parser-speedup.md 2.6
    CPPUNIT_ASSERT_EQUAL( std::string( "18.1.3" ), handler.serverVersion() );
    CPPUNIT_ASSERT_EQUAL( std::string( "2024-03-19T18:13:33+0900" ), handler.timestamp() );

    CPPUNIT_ASSERT_EQUAL( 18, handler.player_types_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 3.0, handler.ball_speed_max_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 42.514, handler.stamina_inc_max_.at( 17 ), EPS );

    CPPUNIT_ASSERT_EQUAL( size_t( 3 ), handler.shows_.size() );
    const ShowInfoT & show = handler.shows_.back();
    CPPUNIT_ASSERT_EQUAL( UInt32( 5 ), show.time_ );

    // "ball":{"x":-1.6867,"y":1.077,"vx":-1.5855,"vy":1.0124}
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.6867, show.ball_.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.077, show.ball_.y_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.5855, show.ball_.vx_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0124, show.ball_.vy_, EPS );

    // {"side":"l","unum":1,"type":0,"state":9,"x":-49,"y":0,"vx":0,"vy":0,"body":-95.83,"neck":90,...,
    //  "stamina":8000,...,"capacity":130600,"fside":"l","fnum":7,"kick":0,"dash":0,"turn":83,...,"attentionto":3,...}
    const PlayerT & l1 = show.player_[0];
    CPPUNIT_ASSERT_EQUAL( 'l', l1.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 1 ), l1.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int32( 9 ), l1.state_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -49.0, l1.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -95.83, l1.body_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 90.0, l1.neck_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 180.0, l1.view_width_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 8000.0, l1.stamina_, LARGE_EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 130600.0, l1.stamina_capacity_, LARGE_EPS );
    CPPUNIT_ASSERT_EQUAL( 'l', l1.focus_side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 7 ), l1.focus_unum_ );
    CPPUNIT_ASSERT_EQUAL( 'h', l1.view_quality_ ); // B9
    CPPUNIT_ASSERT_EQUAL( UInt16( 83 ), l1.turn_count_ );
    CPPUNIT_ASSERT_EQUAL( UInt16( 3 ), l1.attentionto_count_ );

    // {"side":"l","unum":2,"type":2,"state":1,"x":-23.8813,"y":-14.9624,...,"stamina":7898.52,"effort":0.869862,...}
    const PlayerT & l2 = show.player_[1];
    CPPUNIT_ASSERT_EQUAL( Int16( 2 ), l2.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 2 ), l2.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -23.8813, l2.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -14.9624, l2.y_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 7898.52, l2.stamina_, LARGE_EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.869862, l2.effort_, EPS );

    // {"side":"r","unum":1,"type":0,"state":9,"x":39.3019,"y":-0.6752,...,"px":-7.5745,"py":-15.3841,...}
    const PlayerT & r1 = show.player_[11];
    CPPUNIT_ASSERT_EQUAL( 'r', r1.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 1 ), r1.unum_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 39.3019, r1.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -7.5745, r1.point_x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -15.3841, r1.point_y_, EPS );

    // {"side":"r","unum":11,"type":17,"state":1,"x":10.9049,"y":9.5786,...}
    const PlayerT & r11 = show.player_[21];
    CPPUNIT_ASSERT_EQUAL( 'r', r11.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 11 ), r11.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 17 ), r11.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.9049, r11.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 9.5786, r11.y_, EPS );
}

}

namespace {

//! the number of the shows of the large game logs: more than 1024 lines for each of 4 threads
const int MANY_SHOWS = 5000;

/*!
  \brief a large text game log: the header, the parameters, the playmode and the team of head_v5.rcg,
  and MANY_SHOWS shows made from its last show, with the times 1, 2, ...
  \param broken the lines inserted after the show of the time (the key), which cannot be parsed
*/
std::string
many_text_lines( const std::map< int, std::string > & broken = std::map< int, std::string >() )
{
    std::istringstream is( read_file( data_path( "head_v5.rcg" ) ) );
    std::string data;
    std::string show; // the rest of the line after "(show 4 "
    std::string line;
    while ( std::getline( is, line ) )
    {
        if ( ! line.compare( 0, 8, "(show 4 " ) )
        {
            show = line.substr( 8 );
        }
        else if ( line.rfind( "(show ", 0 ) != 0
                  && line.rfind( "(playmode 4 ", 0 ) != 0 ) // the play mode at the time 4
        {
            data += line + '\n';
        }
    }
    CPPUNIT_ASSERT( ! show.empty() );

    for ( int t = 1; t <= MANY_SHOWS; ++t )
    {
        data += "(show " + std::to_string( t ) + ' ' + show + '\n';
        std::map< int, std::string >::const_iterator it = broken.find( t );
        if ( it != broken.end() )
        {
            data += it->second + '\n';
        }
    }
    return data;
}

/*!
  \brief a large JSON game log: the records of head_json.rcg except the shows,
  and MANY_SHOWS shows made from its last show, with the times 1, 2, ...
  \param broken the lines inserted after the show of the time (the key), which cannot be parsed
*/
std::string
many_json_lines( const std::map< int, std::string > & broken = std::map< int, std::string >() )
{
    const std::vector< std::string > records = json_records();
    const std::string prefix = "{\"show\":{\"time\":5,";
    std::string show; // the rest of the record after the time
    std::string data = "[\n";
    for ( const std::string & record : records )
    {
        if ( ! record.compare( 0, prefix.length(), prefix ) )
        {
            show = record.substr( prefix.length() );
        }
        else if ( record.compare( 0, 8, "{\"show\":" ) != 0
                  && record.find( "\"time\":5," ) == std::string::npos )
        {
            data += record + ",\n";
        }
    }
    CPPUNIT_ASSERT( ! show.empty() );

    for ( int t = 1; t <= MANY_SHOWS; ++t )
    {
        data += "{\"show\":{\"time\":" + std::to_string( t ) + ',' + show;
        data += ( t < MANY_SHOWS ? ",\n" : "\n" );
        std::map< int, std::string >::const_iterator it = broken.find( t );
        if ( it != broken.end() )
        {
            data += it->second + ",\n";
        }
    }
    data += "]\n";
    return data;
}

/*!
  \brief the events of many_text_lines() or many_json_lines()
*/
std::vector< std::string >
many_events( const int log_version,
             const std::string & team )
{
    std::vector< std::string > events = header_events( log_version );
    events.push_back( playmode_event( 1, PM_KickOff_Left ) );
    events.push_back( team );
    for ( int t = 1; t <= MANY_SHOWS; ++t )
    {
        events.push_back( "show " + std::to_string( t ) );
    }
    events.push_back( "eof" );
    return events;
}

}

/*-------------------------------------------------------------------*/
/*!
  \brief the regression tests of the rcg parsers on the first records of real game logs
*/
class RcgParserTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( RcgParserTest );
    CPPUNIT_TEST( testTextV5Stream );
    CPPUNIT_TEST( testTextV6Stream );
    CPPUNIT_TEST( testJsonStream );
    CPPUNIT_TEST( testTextV5Memory );
    CPPUNIT_TEST( testTextV6Memory );
    CPPUNIT_TEST( testJsonMemory );
    CPPUNIT_TEST( testTextWithoutLastNewline );
    CPPUNIT_TEST( testTextFilePath );
    CPPUNIT_TEST( testJsonFilePath );
#ifdef HAVE_LIBZ
    CPPUNIT_TEST( testGzipFilePath );
#endif
    CPPUNIT_TEST( testJsonWithoutClosingBracket );
    CPPUNIT_TEST( testJsonOneLine );
    CPPUNIT_TEST( testJsonPrettyPrinted );
    CPPUNIT_TEST( testJsonCRLF );
    CPPUNIT_TEST( testJsonLogVersionFalse );
    CPPUNIT_TEST( testTextLiveLine );
    CPPUNIT_TEST( testJsonLiveData );
    CPPUNIT_TEST( testJsonTeamGraphic );
    CPPUNIT_TEST( testTextHandlerFalseDoesNotStop );
    CPPUNIT_TEST( testCreateWithoutHeader );
    CPPUNIT_TEST( testTextManyLines );
    CPPUNIT_TEST( testJsonManyLines );
    CPPUNIT_TEST( testTextBrokenLinesSkipped );
    CPPUNIT_TEST( testJsonBrokenLinesSkipped );
    CPPUNIT_TEST( testJsonHandlerFalseDoesNotStop );
    CPPUNIT_TEST( testTextParseLine );
    CPPUNIT_TEST( testTextUnumZero );
    CPPUNIT_TEST( testTextBrokenHeader );
    CPPUNIT_TEST( testTextTeamGraphicLong );
    CPPUNIT_TEST( testTextTeamNull );
    CPPUNIT_TEST( testJsonPlayers );
    CPPUNIT_TEST( testJsonShowWithoutPlayers );
    CPPUNIT_TEST( testJsonUnknownRecords );
    CPPUNIT_TEST( testJsonLiveShowWithTeam );
    CPPUNIT_TEST( testJsonLiveBroken );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override
      {
          Parser::setMaxThreads( 0 );
      }

    void tearDown() override
      {
          Parser::setMaxThreads( 0 );
      }

protected:
    void testTextV5Stream();
    void testTextV6Stream();
    void testJsonStream();
    void testTextV5Memory();
    void testTextV6Memory();
    void testJsonMemory();
    void testTextWithoutLastNewline();
    void testTextFilePath();
    void testJsonFilePath();
#ifdef HAVE_LIBZ
    void testGzipFilePath();
#endif
    void testJsonWithoutClosingBracket();
    void testJsonOneLine();
    void testJsonPrettyPrinted();
    void testJsonCRLF();
    void testJsonLogVersionFalse();
    void testTextLiveLine();
    void testJsonLiveData();
    void testJsonTeamGraphic();
    void testTextHandlerFalseDoesNotStop();
    void testCreateWithoutHeader();
    void testTextManyLines();
    void testJsonManyLines();
    void testTextBrokenLinesSkipped();
    void testJsonBrokenLinesSkipped();
    void testJsonHandlerFalseDoesNotStop();
    void testTextParseLine();
    void testTextUnumZero();
    void testTextBrokenHeader();
    void testTextTeamGraphicLong();
    void testTextTeamNull();
    void testJsonPlayers();
    void testJsonShowWithoutPlayers();
    void testJsonUnknownRecords();
    void testJsonLiveShowWithTeam();
    void testJsonLiveBroken();
};

CPPUNIT_TEST_SUITE_REGISTRATION( RcgParserTest );

/*-------------------------------------------------------------------*/
void
RcgParserTest::testTextV5Stream()
{
    RecordHandler handler;
    CPPUNIT_ASSERT( parse_stream( data_path( "head_v5.rcg" ), REC_VERSION_4, handler ) );

    assert_events( v5_events(), handler.events_ );
    check_v5_values( handler );
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testTextV6Stream()
{
    RecordHandler handler;
    CPPUNIT_ASSERT( parse_stream( data_path( "head_v6.rcg" ), REC_VERSION_4, handler ) );

    assert_events( v6_events(), handler.events_ );
    check_v6_values( handler );
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testJsonStream()
{
    RecordHandler handler;
    CPPUNIT_ASSERT( parse_stream( data_path( "head_json.rcg" ), REC_VERSION_JSON, handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief Parser::create( const char *, std::size_t ) and parse( const char *, std::size_t, Handler & )
*/
void
RcgParserTest::testTextV5Memory()
{
    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( read_file( data_path( "head_v5.rcg" ) ), REC_VERSION_4, handler ) );

    assert_events( v5_events(), handler.events_ );
    check_v5_values( handler );
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testTextV6Memory()
{
    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( read_file( data_path( "head_v6.rcg" ) ), REC_VERSION_4, handler ) );

    assert_events( v6_events(), handler.events_ );
    check_v6_values( handler );
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testJsonMemory()
{
    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( read_file( data_path( "head_json.rcg" ) ), REC_VERSION_JSON, handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief the last line without a line break is parsed, as std::getline() does
*/
void
RcgParserTest::testTextWithoutLastNewline()
{
    std::string data = read_file( data_path( "head_v5.rcg" ) );
    CPPUNIT_ASSERT( data.back() == '\n' );
    data.pop_back();

    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_4, handler ) );

    assert_events( v5_events(), handler.events_ );
    check_v5_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief Parser::parse( const std::string & filepath, ... ) is not hidden by ParserV4 (using Parser::parse)
*/
void
RcgParserTest::testTextFilePath()
{
    ParserV4 parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parse( data_path( "head_v6.rcg" ), handler ) );

    assert_events( v6_events(), handler.events_ );
    check_v6_values( handler );
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testJsonFilePath()
{
    ParserSimdJSON parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parse( data_path( "head_json.rcg" ), handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

#ifdef HAVE_LIBZ
/*-------------------------------------------------------------------*/
namespace {
/*!
  \brief write a test data file compressed by gzip
*/
void
write_gzip( const std::string & path,
            const std::string & data_name )
{
    const std::string text = read_file( data_path( data_name ) );
    rcsc::gzofstream fout( path.c_str() );
    fout.write( text.data(), text.size() );
    fout.close();
}
}

/*-------------------------------------------------------------------*/
/*!
  \brief Parser::parse( const std::string & filepath, ... ) reads a gzip file
*/
void
RcgParserTest::testGzipFilePath()
{
    TempFile v6_file( "head_v6.rcg.gz" );
    TempFile json_file( "head_json.rcg.gz" );
    write_gzip( v6_file.path(), "head_v6.rcg" );
    write_gzip( json_file.path(), "head_json.rcg" );

    {
        ParserV4 parser;
        RecordHandler handler;
        CPPUNIT_ASSERT( parser.parse( v6_file.path(), handler ) );
        assert_events( v6_events(), handler.events_ );
        check_v6_values( handler );
    }
    {
        ParserSimdJSON parser;
        RecordHandler handler;
        CPPUNIT_ASSERT( parser.parse( json_file.path(), handler ) );
        assert_events( json_events(), handler.events_ );
        check_json_values( handler );
    }
    {
        // the way of the tools: rcsc::gzread_file() and the parser for the data in memory
        std::string data;
        CPPUNIT_ASSERT( rcsc::gzread_file( v6_file.path(), &data ) );
        RecordHandler handler;
        CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_4, handler ) );
        assert_events( v6_events(), handler.events_ );
    }
}
#endif

/*-------------------------------------------------------------------*/
/*!
  \brief a JSON log whose last "]" is missing (rcssserver stopped abnormally) is read,
  because it is parsed line by line
*/
void
RcgParserTest::testJsonWithoutClosingBracket()
{
    std::string data = "[\n";
    for ( const std::string & record : json_records() )
    {
        data += record + ",\n";
    }

    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a JSON log in one line is parsed as one JSON document
*/
void
RcgParserTest::testJsonOneLine()
{
    std::string data = "[";
    for ( const std::string & record : json_records() )
    {
        if ( data.size() > 1 ) data += ',';
        data += record;
    }
    data += "]";

    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a JSON log whose first record does not end on its line (e.g. pretty-printed)
  is parsed as one JSON document, not line by line
*/
void
RcgParserTest::testJsonPrettyPrinted()
{
    const std::vector< std::string > records = json_records();
    CPPUNIT_ASSERT_EQUAL( std::string( "{\"version\":\"18.1.3\"}" ), records.front() );

    std::string data = "[\n{\n  \"version\": \"18.1.3\"\n},\n";
    for ( size_t i = 1; i < records.size(); ++i )
    {
        data += records[i];
        data += ( i + 1 < records.size() ? ",\n" : "\n" );
    }
    data += "]\n";

    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a JSON log with "\r\n" line breaks is parsed line by line
*/
void
RcgParserTest::testJsonCRLF()
{
    const std::string data = replace_all( read_file( data_path( "head_json.rcg" ) ), "\n", "\r\n" );

    RecordHandler handler;
    CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );

    assert_events( json_events(), handler.events_ );
    check_json_values( handler );
}

/*-------------------------------------------------------------------*/
/*!
  \brief the JSON parser stops if handleLogVersion() returns false, as the text parser does
  (docs/plan/2026-11-24-02_rcg-parser-speedup.md S12)
*/
void
RcgParserTest::testJsonLogVersionFalse()
{
    const std::string data = read_file( data_path( "head_json.rcg" ) );
    {
        RecordHandler handler;
        handler.log_version_result_ = false;
        CPPUNIT_ASSERT( ! parse_memory( data, REC_VERSION_JSON, handler ) );
        assert_events( { "log_version -1" }, handler.events_ );
    }
    {
        RecordHandler handler;
        handler.log_version_result_ = false;
        CPPUNIT_ASSERT( ! parse_memory( read_file( data_path( "head_v5.rcg" ) ), REC_VERSION_4, handler ) );
        assert_events( { "log_version 5" }, handler.events_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief ParserV4::parseLine() for one line of a live connection (soccerwindow2)
*/
void
RcgParserTest::testTextLiveLine()
{
    const std::string line = find_line( data_path( "head_v5.rcg" ), "(show 4 " );
    CPPUNIT_ASSERT( ! line.empty() );

    ParserV4 parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseLine( 0, line, handler ) );

    assert_events( { "show 4" }, handler.events_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.577, handler.shows_.front().ball_.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 38.1613, handler.shows_.front().player_[11].x_, EPS );
}

/*-------------------------------------------------------------------*/
/*!
  \brief ParserSimdJSON::parseData() for one message of a live connection (soccerwindow2)
*/
void
RcgParserTest::testJsonLiveData()
{
    std::string record = find_line( data_path( "head_json.rcg" ), "{\"show\":{\"time\":5," );
    CPPUNIT_ASSERT( ! record.empty() );
    if ( record.back() == ',' ) record.pop_back();

    ParserSimdJSON parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseData( record, handler ) );

    assert_events( { "show 5" }, handler.events_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -1.6867, handler.shows_.front().ball_.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 39.3019, handler.shows_.front().player_[11].x_, EPS );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a team_graphic record of JSON (the 27th line of the JSON game log of head_json.rcg)
*/
void
RcgParserTest::testJsonTeamGraphic()
{
    const std::string record = "{\"team_graphic\":{\"side\":\"l\",\"x\":31,\"y\":4,"
        "\"xpm\":[\"8 8 3 1\",\"  c None\",\"~ c #4A8132\",\"z c #FEFFFC\","
        "\"~zz     \",\"zzz     \",\"zz      \",\"zz      \",\"z       \",\"z       \",\"        \",\"        \"]}}";

    ParserSimdJSON parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseData( record, handler ) );

    assert_events( { "team_graphic l 31 4 12" }, handler.events_ );
    const std::vector< std::string > & xpm = handler.team_graphics_.front();
    CPPUNIT_ASSERT_EQUAL( std::string( "8 8 3 1" ), xpm[0] );
    CPPUNIT_ASSERT_EQUAL( std::string( "  c None" ), xpm[1] );
    CPPUNIT_ASSERT_EQUAL( std::string( "z c #FEFFFC" ), xpm[3] );
    CPPUNIT_ASSERT_EQUAL( std::string( "~zz     " ), xpm[4] );
    CPPUNIT_ASSERT_EQUAL( std::string( "        " ), xpm[11] );
}

/*-------------------------------------------------------------------*/
/*!
  \brief the text parser does not stop even if the handler returns false
*/
void
RcgParserTest::testTextHandlerFalseDoesNotStop()
{
    RecordHandler handler;
    handler.show_result_ = false;
    CPPUNIT_ASSERT( parse_stream( data_path( "head_v5.rcg" ), REC_VERSION_4, handler ) );

    assert_events( v5_events(), handler.events_ );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a large text game log is parsed in parallel (4 threads) and in one thread,
  and the same data are passed to the handler in the same order
  (docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.4)
*/
void
RcgParserTest::testTextManyLines()
{
    const std::string data = many_text_lines();
    const std::vector< std::string > expected = many_events( REC_VERSION_5, "team 1 HELIOS2022 CYRUS 0 0" );

    for ( const int max_threads : { 4, 1 } )
    {
        Parser::setMaxThreads( max_threads );
        RecordHandler handler;
        CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_4, handler ) );
        assert_events( expected, handler.events_ );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 38.1613, handler.shows_.back().player_[11].x_, EPS );
    }
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testJsonManyLines()
{
    const std::string data = many_json_lines();
    const std::vector< std::string > expected = many_events( REC_VERSION_JSON, "team 1 HELIOS2023 CYRUS 0 0" );

    for ( const int max_threads : { 4, 1 } )
    {
        Parser::setMaxThreads( max_threads );
        RecordHandler handler;
        CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );
        assert_events( expected, handler.events_ );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 39.3019, handler.shows_.back().player_[11].x_, EPS );
        // passed through the recording handler of a thread (B8)
        CPPUNIT_ASSERT_EQUAL( std::string( "18.1.3" ), handler.serverVersion() );
        CPPUNIT_ASSERT_EQUAL( std::string( "2024-03-19T18:13:33+0900" ), handler.timestamp() );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief the lines that cannot be parsed are skipped, and the parsing goes on
  (docs/plan/2026-11-24-02_rcg-parser-speedup.md S18)

  An empty line and an unknown record are also skipped (B5), and a broken show is rejected (B3).
*/
void
RcgParserTest::testTextBrokenLinesSkipped()
{
    const std::string data = many_text_lines( { { 100, "not a record" },
                                                { 1000, "(unknown_record 1 2)" },
                                                { 2000, "(show abc" },
                                                { 3000, "" },
                                                { 3500, "   \r" },
                                                { 4000, "(show 1 ((b) x y 0 0))" } } );
    const std::vector< std::string > expected = many_events( REC_VERSION_5, "team 1 HELIOS2022 CYRUS 0 0" );

    for ( const int max_threads : { 4, 1 } )
    {
        Parser::setMaxThreads( max_threads );
        RecordHandler handler;
        CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_4, handler ) );
        assert_events( expected, handler.events_ );
    }
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testJsonBrokenLinesSkipped()
{
    const std::string data = many_json_lines( { { 100, "{\"show\":{\"time\":" },
                                                { 3000, "not json" } } );
    const std::vector< std::string > expected = many_events( REC_VERSION_JSON, "team 1 HELIOS2023 CYRUS 0 0" );

    for ( const int max_threads : { 4, 1 } )
    {
        Parser::setMaxThreads( max_threads );
        RecordHandler handler;
        CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );
        assert_events( expected, handler.events_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief the JSON parser does not stop even if the handler returns false, as the text parser
  (docs/plan/2026-11-24-02_rcg-parser-speedup.md S13)
*/
void
RcgParserTest::testJsonHandlerFalseDoesNotStop()
{
    {
        RecordHandler handler;
        handler.show_result_ = false;
        CPPUNIT_ASSERT( parse_memory( read_file( data_path( "head_json.rcg" ) ), REC_VERSION_JSON, handler ) );
        assert_events( json_events(), handler.events_ );
    }
    {
        Parser::setMaxThreads( 4 );
        RecordHandler handler;
        handler.show_result_ = false;
        CPPUNIT_ASSERT( parse_memory( many_json_lines(), REC_VERSION_JSON, handler ) );
        assert_events( many_events( REC_VERSION_JSON, "team 1 HELIOS2023 CYRUS 0 0" ), handler.events_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief ParserV4::parseLine(): B1, B3 and B5 in docs/plan/2026-11-24-02_rcg-parser-speedup.md 2.6
*/
void
RcgParserTest::testTextParseLine()
{
    ParserV4 parser;
    RecordHandler handler;

    // B5: an empty line is not an error
    CPPUNIT_ASSERT( parser.parseLine( 1, "", handler ) );
    CPPUNIT_ASSERT( parser.parseLine( 2, " \t\r", handler ) );

    // B1: a long record name does not overflow the buffer. B5: an unknown record is skipped
    CPPUNIT_ASSERT( parser.parseLine( 3, "(" + std::string( 200, 'a' ) + " 1 2)", handler ) );
    CPPUNIT_ASSERT( parser.parseLine( 4, "(unknown_record 1 2)", handler ) );

    // not a record
    CPPUNIT_ASSERT( ! parser.parseLine( 5, "not a record", handler ) );

    // B3: a broken time or ball of a show
    CPPUNIT_ASSERT( ! parser.parseLine( 6, "(show abc", handler ) );
    CPPUNIT_ASSERT( ! parser.parseLine( 7, "(show 1 ((b) x y 0 0))", handler ) );

    CPPUNIT_ASSERT( handler.events_.empty() );
}

/*-------------------------------------------------------------------*/
/*!
  \brief B2: the player unum 0 is rejected (it would write player_[-1])
*/
void
RcgParserTest::testTextUnumZero()
{
    std::string line = find_line( data_path( "head_v5.rcg" ), "(show 4 " );
    const std::string::size_type pos = line.find( "((l 1) " );
    CPPUNIT_ASSERT( pos != std::string::npos );
    line.replace( pos, 7, "((l 0) " );

    ParserV4 parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( ! parser.parseLine( 1, line, handler ) );
    CPPUNIT_ASSERT( handler.events_.empty() );
}

/*-------------------------------------------------------------------*/
/*!
  \brief B4: a broken header does not throw
*/
void
RcgParserTest::testTextBrokenHeader()
{
    for ( const std::string header : { "ULGx\n", "ULG\n", "ULG9\n" } )
    {
        const std::string data = header + "(playmode 1 kick_off_l)\n";
        ParserV4 parser;
        RecordHandler handler;
        bool result = true;
        CPPUNIT_ASSERT_NO_THROW( result = parser.parse( data.data(), data.size(), handler ) );
        CPPUNIT_ASSERT( ! result );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief B6: the strings of a team graphic of any length (16 characters for 8 pixels of 2 characters)
*/
void
RcgParserTest::testTextTeamGraphicLong()
{
    const std::string row1 = "0123456789abcdef";
    const std::string row2 = "fedcba9876543210";
    const std::string line = "(msg 0 1 \"(team_graphic_r (3 2 \"8 8 1 2\" \"aa c None\" \""
        + row1 + "\" \"" + row2 + "\")\")";

    ParserV4 parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseLine( 1, line, handler ) );
    assert_events( { "team_graphic r 3 2 4" }, handler.events_ );
    const std::vector< std::string > & xpm = handler.team_graphics_.front();
    CPPUNIT_ASSERT_EQUAL( std::string( "8 8 1 2" ), xpm[0] );
    CPPUNIT_ASSERT_EQUAL( std::string( "aa c None" ), xpm[1] );
    CPPUNIT_ASSERT_EQUAL( row1, xpm[2] );
    CPPUNIT_ASSERT_EQUAL( row2, xpm[3] );
}

/*-------------------------------------------------------------------*/
/*!
  \brief B7: the team name "null" (before the team connects) is empty, as in the show line
*/
void
RcgParserTest::testTextTeamNull()
{
    ParserV4 parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseLine( 1, "(team 1 null null 0 0)", handler ) );
    CPPUNIT_ASSERT( parser.parseLine( 2, "(team 2 HELIOS null 0 0)", handler ) );
    assert_events( { "team 1   0 0", "team 2 HELIOS  0 0" }, handler.events_ );
}

/*-------------------------------------------------------------------*/
/*!
  \brief B9 and B10: the players of a JSON show are placed by side and unum,
  the omitted fields keep the default values, and illegal players are skipped
*/
void
RcgParserTest::testJsonPlayers()
{
    const std::string record = "{\"show\":{\"time\":7,\"ball\":{\"x\":1.5,\"y\":-2,\"vx\":0.1,\"vy\":0},\"players\":["
        // the right 11, first
        "{\"side\":\"r\",\"unum\":11,\"type\":3,\"state\":1,\"x\":10,\"y\":20,\"vx\":0,\"vy\":0,"
        "\"body\":45,\"neck\":0,\"vq\":\"l\",\"vw\":60,\"stamina\":7000,\"effort\":1,\"recovery\":1,\"capacity\":130600},"
        // the left 2, with few fields
        "{\"side\":\"l\",\"unum\":2,\"x\":-5,\"y\":3},"
        // illegal players
        "{\"side\":\"x\",\"unum\":1,\"x\":99},"
        "{\"side\":\"l\",\"unum\":0,\"x\":98},"
        "{\"side\":\"l\",\"unum\":12,\"x\":97},"
        "{\"unum\":3,\"x\":96},"
        // the left 2 again (replaces the first), with an unknown key
        "{\"side\":\"l\",\"unum\":2,\"x\":-6,\"y\":4,\"extra\":1}"
        "]}}";

    ParserSimdJSON parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseData( record, handler ) );
    assert_events( { "show 7" }, handler.events_ );

    const ShowInfoT & show = handler.shows_.front();
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.5, show.ball_.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -2.0, show.ball_.y_, EPS );

    const PlayerT & r11 = show.player_[21];
    CPPUNIT_ASSERT_EQUAL( 'r', r11.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 11 ), r11.unum_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 3 ), r11.type_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r11.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 45.0, r11.body_, EPS );
    CPPUNIT_ASSERT_EQUAL( 'l', r11.view_quality_ ); // B9
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 7000.0, r11.stamina_, LARGE_EPS );

    const PlayerT & l2 = show.player_[1];
    CPPUNIT_ASSERT_EQUAL( 'l', l2.side_ );
    CPPUNIT_ASSERT_EQUAL( Int16( 2 ), l2.unum_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -6.0, l2.x_, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 4.0, l2.y_, EPS );
    const PlayerT defaults;
    CPPUNIT_ASSERT_EQUAL( defaults.type_, l2.type_ );
    CPPUNIT_ASSERT_EQUAL( defaults.view_quality_, l2.view_quality_ );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( defaults.stamina_, l2.stamina_, EPS );

    // the others are not there
    for ( int i = 0; i < MAX_PLAYER * 2; ++i )
    {
        if ( i == 1 || i == 21 ) continue;
        CPPUNIT_ASSERT_EQUAL( Int16( 0 ), show.player_[i].unum_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief B10: a show needs only the time and the ball
*/
void
RcgParserTest::testJsonShowWithoutPlayers()
{
    ParserSimdJSON parser;
    {
        RecordHandler handler;
        CPPUNIT_ASSERT( parser.parseData( "{\"show\":{\"time\":3,\"ball\":{\"x\":1,\"y\":2}}}", handler ) );
        assert_events( { "show 3" }, handler.events_ );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( 2.0, handler.shows_.front().ball_.y_, EPS );
    }
    {
        // no ball
        RecordHandler handler;
        CPPUNIT_ASSERT( ! parser.parseData( "{\"show\":{\"time\":3,\"players\":[]}}", handler ) );
        CPPUNIT_ASSERT( handler.events_.empty() );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief B11: unknown records and keys are skipped, line by line and in one document
*/
void
RcgParserTest::testJsonUnknownRecords()
{
    const std::vector< std::string > records = json_records();

    std::string lines = "[\n";
    std::string document = "[";
    for ( size_t i = 0; i < records.size(); ++i )
    {
        if ( i == 3 )
        {
            lines += "{\"new_record\":{\"a\":[1,2,{\"b\":3}]}},\n";
            document += "{\"new_record\":{\"a\":[1,2,{\"b\":3}]}},";
        }
        lines += records[i] + ( i + 1 < records.size() ? ",\n" : "\n" );
        document += records[i] + ( i + 1 < records.size() ? "," : "" );
    }
    lines += "]\n";
    document += "]";

    for ( const std::string & data : { lines, document } )
    {
        RecordHandler handler;
        CPPUNIT_ASSERT( parse_memory( data, REC_VERSION_JSON, handler ) );
        assert_events( json_events(), handler.events_ );
        check_json_values( handler );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief a show of a live connection has the play mode and the teams
*/
void
RcgParserTest::testJsonLiveShowWithTeam()
{
    const std::string record = "{\"show\":{\"time\":10,\"mode\":\"play_on\","
        "\"team\":{\"l\":{\"name\":\"HELIOS\",\"score\":1},\"r\":{\"name\":null,\"score\":0}},"
        "\"ball\":{\"x\":0,\"y\":0,\"vx\":0,\"vy\":0},\"players\":[]}}";

    ParserSimdJSON parser;
    RecordHandler handler;
    CPPUNIT_ASSERT( parser.parseData( record, handler ) );
    assert_events( { playmode_event( 10, PM_PlayOn ), "team 10 HELIOS  1 0", "show 10" }, handler.events_ );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a broken message of a live connection does not throw
*/
void
RcgParserTest::testJsonLiveBroken()
{
    ParserSimdJSON parser;
    for ( const std::string data : { "{\"show\":{\"time\":", "not json", "", "[1,2]" } )
    {
        RecordHandler handler;
        bool result = true;
        CPPUNIT_ASSERT_NO_THROW( result = parser.parseData( data, handler ) );
        CPPUNIT_ASSERT( ! result );
    }
}

/*-------------------------------------------------------------------*/
void
RcgParserTest::testCreateWithoutHeader()
{
    std::istringstream empty( "" );
    CPPUNIT_ASSERT( ! Parser::create( empty ) );

    std::istringstream short_data( "UL" );
    CPPUNIT_ASSERT( ! Parser::create( short_data ) );
}

/*-------------------------------------------------------------------*/

#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>

int
main( int, char ** )
{
    // create the event manager and test controller
    CPPUNIT_NS::TestResult controller;

    // add a listner that collects test results
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener( &result );

    // add a listener that prints dots as test run.
    CPPUNIT_NS::BriefTestProgressListener progress;
    controller.addListener( &progress );

    // add the top suite to the test runner.
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run( controller );

    // output results in a compiler compatible format
    CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
