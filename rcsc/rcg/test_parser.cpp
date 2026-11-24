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

#include <cppunit/extensions/HelperMacros.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

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

    bool handleLogVersion( const int ver ) override
      {
          events_.push_back( "log_version " + std::to_string( ver ) );
          return Handler::handleLogVersion( ver );
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
  \brief check the values of the last show in head_json.rcg (time 5)

  The view quality ("vq") is not checked here, because the JSON parser stores it in a wrong way.
  (B9 in docs/plan/2026-11-24-02_rcg-parser-speedup.md 2.6)
*/
void
check_json_values( const RecordHandler & handler )
{
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
    CPPUNIT_TEST( testTextFilePath );
    CPPUNIT_TEST( testJsonFilePath );
    CPPUNIT_TEST( testTextLiveLine );
    CPPUNIT_TEST( testJsonLiveData );
    CPPUNIT_TEST( testJsonTeamGraphic );
    CPPUNIT_TEST( testTextHandlerFalseDoesNotStop );
    CPPUNIT_TEST( testCreateWithoutHeader );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override { }
    void tearDown() override { }

protected:
    void testTextV5Stream();
    void testTextV6Stream();
    void testJsonStream();
    void testTextFilePath();
    void testJsonFilePath();
    void testTextLiveLine();
    void testJsonLiveData();
    void testJsonTeamGraphic();
    void testTextHandlerFalseDoesNotStop();
    void testCreateWithoutHeader();
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
void
RcgParserTest::testTextFilePath()
{
    ParserV4 parser;
    RecordHandler handler;
    // ParserV4::parse(std::istream&,...) hides Parser::parse(const std::string&,...)
    const Parser & base = parser;
    CPPUNIT_ASSERT( base.parse( data_path( "head_v6.rcg" ), handler ) );

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
