// -*-c++-*-

/*!
  \file test_line_parser.cpp
  \brief test code for parsing the lines of a game log in parallel (line_parser.h)
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

#include "line_parser.h"
#include "handler.h"
#include "parser.h"
#include "types.h"

#include <cppunit/extensions/HelperMacros.h>

#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.4 and 6.

using namespace rcsc::rcg;

namespace {

/*!
  \brief a handler that records the messages, in order
*/
class MsgHandler
    : public Handler {
public:
    std::vector< std::string > msgs_;
    std::vector< std::thread::id > threads_; //!< the threads that called handleMsg()
    bool result_ = true; //!< the value returned by handleMsg()
    std::string throw_at_; //!< handleMsg() throws for this message

    bool handleMsg( const int,
                    const int,
                    const std::string & msg ) override
      {
          if ( ! throw_at_.empty() && msg == throw_at_ )
          {
              throw std::runtime_error( "handler error" );
          }
          msgs_.push_back( msg );
          threads_.push_back( std::this_thread::get_id() );
          return result_;
      }

    bool handleEOF() override { return true; }
    bool handleShow( const ShowInfoT & ) override { return true; }
    bool handleDraw( const int, const drawinfo_t & ) override { return true; }
    bool handlePlayMode( const int, const rcsc::PlayMode ) override { return true; }
    bool handleTeam( const int, const TeamT &, const TeamT & ) override { return true; }
    bool handleServerParam( const ServerParamT & ) override { return true; }
    bool handlePlayerParam( const PlayerParamT & ) override { return true; }
    bool handlePlayerType( const PlayerTypeT & ) override { return true; }
    bool handleTeamGraphic( const char, const int, const int, const std::vector< std::string > & ) override { return true; }
};

/*!
  \brief the data of n lines: "line 1\nline 2\n..."
*/
std::string
make_lines( const int n )
{
    std::string data;
    for ( int i = 1; i <= n; ++i )
    {
        data += "line " + std::to_string( i ) + '\n';
    }
    return data;
}

/*!
  \brief the threads that parsed the lines, shared by the line parsers of the threads
*/
struct ParserThreads {
    std::mutex mutex_;
    std::set< std::thread::id > ids_;

    void add()
      {
          std::lock_guard< std::mutex > lock( mutex_ );
          ids_.insert( std::this_thread::get_id() );
      }
};

/*!
  \brief a factory of line parsers that pass each line as a message.
  The line "line <fail>" fails, and the line "line <thrown>" throws.
*/
LineParserFactory
msg_parser_factory( ParserThreads * threads,
                    const int fail = -1,
                    const int thrown = -1 )
{
    return [threads, fail, thrown]() -> LineParser
        {
            return [threads, fail, thrown]( const LineRef & line, Handler & handler ) -> bool
                {
                    if ( threads ) threads->add();
                    const std::string text( line.begin_, line.length_ );
                    if ( text == "line " + std::to_string( fail ) )
                    {
                        return false;
                    }
                    if ( text == "line " + std::to_string( thrown ) )
                    {
                        throw std::runtime_error( "parse error" );
                    }
                    return handler.handleMsg( line.number_, 0, text );
                };
        };
}

std::vector< std::string >
expected_msgs( const int n,
               const std::set< int > & skipped = std::set< int >() )
{
    std::vector< std::string > msgs;
    for ( int i = 1; i <= n; ++i )
    {
        if ( ! skipped.count( i ) )
        {
            msgs.push_back( "line " + std::to_string( i ) );
        }
    }
    return msgs;
}

}

/*-------------------------------------------------------------------*/
class LineParserTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( LineParserTest );
    CPPUNIT_TEST( testSplitLines );
    CPPUNIT_TEST( testCountThreads );
    CPPUNIT_TEST( testManyLinesInOrder );
    CPPUNIT_TEST( testOneThread );
    CPPUNIT_TEST( testFailedLinesSkipped );
    CPPUNIT_TEST( testHandlerResultIgnored );
    CPPUNIT_TEST( testHandlerException );
    CPPUNIT_TEST( testVersionAndTimestamp );
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
    void testSplitLines();
    void testCountThreads();
    void testManyLinesInOrder();
    void testOneThread();
    void testFailedLinesSkipped();
    void testHandlerResultIgnored();
    void testHandlerException();
    void testVersionAndTimestamp();
};

CPPUNIT_TEST_SUITE_REGISTRATION( LineParserTest );

/*-------------------------------------------------------------------*/
/*!
  \brief the lines are split as std::getline() does
*/
void
LineParserTest::testSplitLines()
{
    {
        const std::string data = "a\nb\r\n\nc";
        const std::vector< LineRef > lines = split_lines( data.data(), data.size() );
        CPPUNIT_ASSERT_EQUAL( size_t( 4 ), lines.size() );
        CPPUNIT_ASSERT_EQUAL( std::string( "a" ), std::string( lines[0].begin_, lines[0].length_ ) );
        CPPUNIT_ASSERT_EQUAL( std::string( "b\r" ), std::string( lines[1].begin_, lines[1].length_ ) );
        CPPUNIT_ASSERT_EQUAL( std::string( "" ), std::string( lines[2].begin_, lines[2].length_ ) );
        CPPUNIT_ASSERT_EQUAL( std::string( "c" ), std::string( lines[3].begin_, lines[3].length_ ) );
        for ( int i = 0; i < 4; ++i )
        {
            CPPUNIT_ASSERT_EQUAL( i + 1, lines[i].number_ );
        }
        CPPUNIT_ASSERT( lines[0].begin_ == data.data() ); // not copied
    }
    {
        const std::string data = "a\n";
        CPPUNIT_ASSERT_EQUAL( size_t( 1 ), split_lines( data.data(), data.size() ).size() );
    }
    {
        const std::string data = "\n";
        const std::vector< LineRef > lines = split_lines( data.data(), data.size() );
        CPPUNIT_ASSERT_EQUAL( size_t( 1 ), lines.size() );
        CPPUNIT_ASSERT_EQUAL( size_t( 0 ), lines[0].length_ );
    }
    {
        CPPUNIT_ASSERT( split_lines( "", 0 ).empty() );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief Parser::setMaxThreads(), Parser::maxThreads() and count_threads()
*/
void
LineParserTest::testCountThreads()
{
    const int hardware = std::max( 1u, std::thread::hardware_concurrency() );

    Parser::setMaxThreads( 0 );
    CPPUNIT_ASSERT_EQUAL( std::min( hardware, 8 ), Parser::maxThreads() );
    Parser::setMaxThreads( -3 );
    CPPUNIT_ASSERT_EQUAL( std::min( hardware, 8 ), Parser::maxThreads() );

    Parser::setMaxThreads( 1 );
    CPPUNIT_ASSERT_EQUAL( 1, Parser::maxThreads() );
    CPPUNIT_ASSERT_EQUAL( 1, count_threads( 1000000 ) );

    // an explicit limit is used even if it is larger than the default
    Parser::setMaxThreads( 12 );
    CPPUNIT_ASSERT_EQUAL( 12, Parser::maxThreads() );

    // at least 1024 lines for a thread
    Parser::setMaxThreads( 3 );
    CPPUNIT_ASSERT_EQUAL( 1, count_threads( 0 ) );
    CPPUNIT_ASSERT_EQUAL( 1, count_threads( 2047 ) );
    CPPUNIT_ASSERT_EQUAL( 2, count_threads( 2048 ) );
    CPPUNIT_ASSERT_EQUAL( 3, count_threads( 100000 ) );
}

/*-------------------------------------------------------------------*/
/*!
  \brief many lines are parsed in parallel, and passed to the handler in order, in the calling thread
*/
void
LineParserTest::testManyLinesInOrder()
{
    Parser::setMaxThreads( 4 );

    const int n = 20000;
    const std::string data = make_lines( n );
    const std::vector< LineRef > lines = split_lines( data.data(), data.size() );
    CPPUNIT_ASSERT_EQUAL( 4, count_threads( lines.size() ) );

    ParserThreads threads;
    MsgHandler handler;
    const std::vector< int > failed = parse_lines( lines, msg_parser_factory( &threads ), handler );

    CPPUNIT_ASSERT( failed.empty() );
    CPPUNIT_ASSERT( expected_msgs( n ) == handler.msgs_ );
    CPPUNIT_ASSERT_EQUAL( size_t( 4 ), threads.ids_.size() );
    CPPUNIT_ASSERT( threads.ids_.count( std::this_thread::get_id() ) == 0 );
    for ( const std::thread::id & id : handler.threads_ )
    {
        CPPUNIT_ASSERT( id == std::this_thread::get_id() );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief with Parser::setMaxThreads( 1 ), the lines are parsed in the calling thread
*/
void
LineParserTest::testOneThread()
{
    Parser::setMaxThreads( 1 );

    const int n = 20000;
    const std::string data = make_lines( n );
    const std::vector< LineRef > lines = split_lines( data.data(), data.size() );

    ParserThreads threads;
    MsgHandler handler;
    const std::vector< int > failed = parse_lines( lines, msg_parser_factory( &threads ), handler );

    CPPUNIT_ASSERT( failed.empty() );
    CPPUNIT_ASSERT( expected_msgs( n ) == handler.msgs_ );
    CPPUNIT_ASSERT_EQUAL( size_t( 1 ), threads.ids_.size() );
    CPPUNIT_ASSERT( threads.ids_.count( std::this_thread::get_id() ) == 1 );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a line that fails or throws is skipped and reported, and the others are passed in order
*/
void
LineParserTest::testFailedLinesSkipped()
{
    const int n = 20000;
    const std::string data = make_lines( n );
    const std::vector< LineRef > lines = split_lines( data.data(), data.size() );

    for ( const int max_threads : { 1, 4 } )
    {
        Parser::setMaxThreads( max_threads );
        MsgHandler handler;
        const std::vector< int > failed = parse_lines( lines, msg_parser_factory( nullptr, 5000, 15000 ), handler );

        CPPUNIT_ASSERT( std::vector< int >( { 5000, 15000 } ) == failed );
        CPPUNIT_ASSERT( expected_msgs( n, { 5000, 15000 } ) == handler.msgs_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief the return values of the handler are not used, whatever the number of the threads
  (docs/plan/2026-11-24-02_rcg-parser-speedup.md S13, S19)
*/
void
LineParserTest::testHandlerResultIgnored()
{
    const int n = 20000;
    const std::string data = make_lines( n );
    const std::vector< LineRef > lines = split_lines( data.data(), data.size() );

    for ( const int max_threads : { 1, 4 } )
    {
        Parser::setMaxThreads( max_threads );
        MsgHandler handler;
        handler.result_ = false;
        const std::vector< int > failed = parse_lines( lines, msg_parser_factory( nullptr ), handler );

        CPPUNIT_ASSERT( failed.empty() );
        CPPUNIT_ASSERT( expected_msgs( n ) == handler.msgs_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief an exception thrown by the handler goes out of parse_lines(), whatever the number of the threads
  (docs/plan/2026-11-24-02_rcg-parser-speedup.md S19)
*/
void
LineParserTest::testHandlerException()
{
    const int n = 20000;
    const std::string data = make_lines( n );
    const std::vector< LineRef > lines = split_lines( data.data(), data.size() );

    for ( const int max_threads : { 1, 4 } )
    {
        Parser::setMaxThreads( max_threads );
        MsgHandler handler;
        handler.throw_at_ = "line 12345";
        CPPUNIT_ASSERT_THROW( parse_lines( lines, msg_parser_factory( nullptr ), handler ),
                              std::runtime_error );
        CPPUNIT_ASSERT( expected_msgs( 12344 ) == handler.msgs_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  \brief handleServerVersion() and handleTimestamp() (virtual since this change) are passed,
  and the state of the handler (logVersion()) can be referred to in the threads
*/
void
LineParserTest::testVersionAndTimestamp()
{
    Parser::setMaxThreads( 4 );

    const int n = 20000;
    const std::string data = make_lines( n );
    const std::vector< LineRef > lines = split_lines( data.data(), data.size() );

    MsgHandler handler;
    handler.handleLogVersion( REC_VERSION_JSON );

    const LineParserFactory factory = []() -> LineParser
        {
            return []( const LineRef & line, Handler & h ) -> bool
                {
                    if ( h.logVersion() != REC_VERSION_JSON )
                    {
                        return false;
                    }
                    if ( line.number_ == 3 )
                    {
                        return h.handleServerVersion( "18.1.3" );
                    }
                    if ( line.number_ == 19000 )
                    {
                        return h.handleTimestamp( "2024-03-19T18:13:33+0900" );
                    }
                    return true;
                };
        };

    const std::vector< int > failed = parse_lines( lines, factory, handler );
    CPPUNIT_ASSERT( failed.empty() );
    CPPUNIT_ASSERT_EQUAL( std::string( "18.1.3" ), handler.serverVersion() );
    CPPUNIT_ASSERT_EQUAL( std::string( "2024-03-19T18:13:33+0900" ), handler.timestamp() );
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
