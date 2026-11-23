// -*-c++-*-

/*!
  \file test_server_param.cpp
  \brief test code for rcsc::ServerParam and rcsc::rcg::ServerParamT
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

#include "server_param.h"

#include <rcsc/rcg/types.h>

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <sstream>
#include <string>

using rcsc::ServerParam;
using rcsc::rcg::ServerParamT;

namespace {

/*!
  \brief capture std::cerr while this object is alive
*/
class CerrCapture {
private:
    std::ostringstream M_buf;
    std::streambuf * M_old;

public:
    CerrCapture()
        : M_buf(),
          M_old( std::cerr.rdbuf( M_buf.rdbuf() ) )
      { }

    ~CerrCapture()
      {
          std::cerr.rdbuf( M_old );
      }

    std::string str() const
      {
          return M_buf.str();
      }
};

/*!
  \brief check whether rcg::ServerParamT warned about the parameter
  \param err captured std::cerr
  \param name parameter name

  The warning is "Unsupported parameter. <whole message> (<name> <value>)".
  The parameters in the message are written without spaces between them, like ")(name",
  so ") (name " appears only in the warning.
*/
bool
has_warning( const std::string & err,
             const std::string & name )
{
    return err.find( ") (" + name + ' ' ) != std::string::npos;
}

// the parameters added in rcssserver 20.0
const char * NEW_PARAM_NAMES[] = {
    "tcp_port",
    "tcp_coach_port",
    "tcp_olcoach_port",
    "ws_port",
    "ws_allowed_origins",
};

}

/*-------------------------------------------------------------------*/
/*!

*/
class ServerParamTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( ServerParamTest );
    // ServerParam is a singleton. testDefault has to be run first.
    CPPUNIT_TEST( testDefault );
    CPPUNIT_TEST( testParse );
    CPPUNIT_TEST( testTcpPortSameAsUdp );
    CPPUNIT_TEST( testRcgFromServerString );
    CPPUNIT_TEST( testRcgSetValue );
    CPPUNIT_TEST( testConvert );
    CPPUNIT_TEST( testToServerString );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

protected:
    void testDefault();
    void testParse();
    void testTcpPortSameAsUdp();
    void testRcgFromServerString();
    void testRcgSetValue();
    void testConvert();
    void testToServerString();
};

// regist test
CPPUNIT_TEST_SUITE_REGISTRATION( ServerParamTest );

/*-------------------------------------------------------------------*/
void
ServerParamTest::setUp()
{

}

/*-------------------------------------------------------------------*/
void
ServerParamTest::tearDown()
{

}

/*-------------------------------------------------------------------*/
/*!
  the default values: TCP and WebSocket are disabled
*/
void
ServerParamTest::testDefault()
{
    {
        const ServerParam & SP = ServerParam::i();
        CPPUNIT_ASSERT_EQUAL( 0, SP.tcpPlayerPort() );
        CPPUNIT_ASSERT_EQUAL( 0, SP.tcpTrainerPort() );
        CPPUNIT_ASSERT_EQUAL( 0, SP.tcpOnlineCoachPort() );
        CPPUNIT_ASSERT_EQUAL( 0, SP.wsPort() );
        CPPUNIT_ASSERT_EQUAL( std::string(), SP.wsAllowedOrigins() );
    }
    {
        const ServerParamT param;
        CPPUNIT_ASSERT_EQUAL( 0, param.tcp_port_ );
        CPPUNIT_ASSERT_EQUAL( 0, param.tcp_coach_port_ );
        CPPUNIT_ASSERT_EQUAL( 0, param.tcp_olcoach_port_ );
        CPPUNIT_ASSERT_EQUAL( 0, param.ws_port_ );
        CPPUNIT_ASSERT_EQUAL( std::string(), param.ws_allowed_origins_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  the server_param message (protocol version 8 or later)
*/
void
ServerParamTest::testParse()
{
    const std::string msg = "(server_param (port 6100)(coach_port 6101)(olcoach_port 6102)"
        "(tcp_port 7000)(tcp_coach_port 7001)(tcp_olcoach_port 7002)"
        "(ws_port 8080)(ws_allowed_origins \"http://localhost:8000,null\"))";

    CerrCapture err;
    CPPUNIT_ASSERT( ServerParam::instance().parse( msg.c_str(), 20.0 ) );
    CPPUNIT_ASSERT_EQUAL( std::string(), err.str() );

    const ServerParam & SP = ServerParam::i();
    CPPUNIT_ASSERT_EQUAL( 6100, SP.playerPort() );
    CPPUNIT_ASSERT_EQUAL( 7000, SP.tcpPlayerPort() );
    CPPUNIT_ASSERT_EQUAL( 7001, SP.tcpTrainerPort() );
    CPPUNIT_ASSERT_EQUAL( 7002, SP.tcpOnlineCoachPort() );
    CPPUNIT_ASSERT_EQUAL( 8080, SP.wsPort() );
    CPPUNIT_ASSERT_EQUAL( std::string( "http://localhost:8000,null" ), SP.wsAllowedOrigins() );
}

/*-------------------------------------------------------------------*/
/*!
  -1 selects the UDP port number. 0 disables TCP.
*/
void
ServerParamTest::testTcpPortSameAsUdp()
{
    const ServerParam & SP = ServerParam::i();

    {
        const std::string msg = "(server_param (port 6100)(coach_port 6101)(olcoach_port 6102)"
            "(tcp_port -1)(tcp_coach_port -1)(tcp_olcoach_port -1))";
        CPPUNIT_ASSERT( ServerParam::instance().parse( msg.c_str(), 20.0 ) );
        CPPUNIT_ASSERT_EQUAL( 6100, SP.tcpPlayerPort() );
        CPPUNIT_ASSERT_EQUAL( 6101, SP.tcpTrainerPort() );
        CPPUNIT_ASSERT_EQUAL( 6102, SP.tcpOnlineCoachPort() );
    }
    {
        // follow the UDP port numbers
        const std::string msg = "(server_param (port 6200)(coach_port 6201)(olcoach_port 6202))";
        CPPUNIT_ASSERT( ServerParam::instance().parse( msg.c_str(), 20.0 ) );
        CPPUNIT_ASSERT_EQUAL( 6200, SP.tcpPlayerPort() );
        CPPUNIT_ASSERT_EQUAL( 6201, SP.tcpTrainerPort() );
        CPPUNIT_ASSERT_EQUAL( 6202, SP.tcpOnlineCoachPort() );
    }
    {
        const std::string msg = "(server_param (tcp_port 0)(tcp_coach_port 0)(tcp_olcoach_port 0))";
        CPPUNIT_ASSERT( ServerParam::instance().parse( msg.c_str(), 20.0 ) );
        CPPUNIT_ASSERT_EQUAL( 0, SP.tcpPlayerPort() );
        CPPUNIT_ASSERT_EQUAL( 0, SP.tcpTrainerPort() );
        CPPUNIT_ASSERT_EQUAL( 0, SP.tcpOnlineCoachPort() );
    }
}

/*-------------------------------------------------------------------*/
/*!
  server_param in the text rcg (rcssserver writes all the parameters to the game log)
*/
void
ServerParamTest::testRcgFromServerString()
{
    {
        const std::string msg = "(server_param (port 6000)"
            "(tcp_port -1)(tcp_coach_port 7001)(tcp_olcoach_port 0)"
            "(ws_port 8080)(ws_allowed_origins \"http://localhost:8000,null\"))";

        CerrCapture err;
        ServerParamT param;
        CPPUNIT_ASSERT( param.fromServerString( msg ) );
        CPPUNIT_ASSERT_EQUAL( std::string(), err.str() );

        CPPUNIT_ASSERT_EQUAL( -1, param.tcp_port_ );
        CPPUNIT_ASSERT_EQUAL( 7001, param.tcp_coach_port_ );
        CPPUNIT_ASSERT_EQUAL( 0, param.tcp_olcoach_port_ );
        CPPUNIT_ASSERT_EQUAL( 8080, param.ws_port_ );
        CPPUNIT_ASSERT_EQUAL( std::string( "http://localhost:8000,null" ), param.ws_allowed_origins_ );
    }
    {
        // the default values written by rcssserver
        const std::string msg = "(server_param (tcp_port 0)(tcp_coach_port 0)(tcp_olcoach_port 0)"
            "(ws_port 0)(ws_allowed_origins \"\"))";

        CerrCapture err;
        ServerParamT param;
        param.ws_allowed_origins_ = "dummy";
        CPPUNIT_ASSERT( param.fromServerString( msg ) );
        CPPUNIT_ASSERT_EQUAL( std::string(), err.str() );
        CPPUNIT_ASSERT_EQUAL( std::string(), param.ws_allowed_origins_ );
    }
    {
        // an empty string without quotes, written by ServerParam::toServerString()
        const std::string msg = "(server_param (ws_allowed_origins ))";

        CerrCapture err;
        ServerParamT param;
        param.ws_allowed_origins_ = "dummy";
        CPPUNIT_ASSERT( param.fromServerString( msg ) );
        CPPUNIT_ASSERT_EQUAL( std::string(), err.str() );
        CPPUNIT_ASSERT_EQUAL( std::string(), param.ws_allowed_origins_ );
    }
}

/*-------------------------------------------------------------------*/
/*!
  the setters used by the JSON rcg parser
*/
void
ServerParamTest::testRcgSetValue()
{
    CerrCapture err;
    ServerParamT param;

    CPPUNIT_ASSERT( param.setInt( "tcp_port", -1 ) );
    CPPUNIT_ASSERT( param.setInt( "tcp_coach_port", 7001 ) );
    CPPUNIT_ASSERT( param.setInt( "tcp_olcoach_port", 7002 ) );
    CPPUNIT_ASSERT( param.setInt( "ws_port", 8080 ) );
    CPPUNIT_ASSERT( param.setString( "ws_allowed_origins", "http://localhost:8000,null" ) );
    CPPUNIT_ASSERT_EQUAL( std::string(), err.str() );

    CPPUNIT_ASSERT_EQUAL( -1, param.tcp_port_ );
    CPPUNIT_ASSERT_EQUAL( 7001, param.tcp_coach_port_ );
    CPPUNIT_ASSERT_EQUAL( 7002, param.tcp_olcoach_port_ );
    CPPUNIT_ASSERT_EQUAL( 8080, param.ws_port_ );
    CPPUNIT_ASSERT_EQUAL( std::string( "http://localhost:8000,null" ), param.ws_allowed_origins_ );
}

/*-------------------------------------------------------------------*/
/*!
  ServerParam::convertFrom() and convertTo() with rcg::ServerParamT
*/
void
ServerParamTest::testConvert()
{
    const ServerParam & SP = ServerParam::i();

    ServerParamT from;
    from.port_ = 6100;
    from.coach_port_ = 6101;
    from.online_coach_port_ = 6102;
    from.tcp_port_ = -1;
    from.tcp_coach_port_ = 7001;
    from.tcp_olcoach_port_ = 0;
    from.ws_port_ = 8080;
    from.ws_allowed_origins_ = "http://localhost:8000,null";

    ServerParam::instance().convertFrom( from );

    CPPUNIT_ASSERT_EQUAL( 6100, SP.tcpPlayerPort() );
    CPPUNIT_ASSERT_EQUAL( 7001, SP.tcpTrainerPort() );
    CPPUNIT_ASSERT_EQUAL( 0, SP.tcpOnlineCoachPort() );
    CPPUNIT_ASSERT_EQUAL( 8080, SP.wsPort() );
    CPPUNIT_ASSERT_EQUAL( std::string( "http://localhost:8000,null" ), SP.wsAllowedOrigins() );

    // the raw values are kept
    ServerParamT to;
    SP.convertTo( to );

    CPPUNIT_ASSERT_EQUAL( -1, to.tcp_port_ );
    CPPUNIT_ASSERT_EQUAL( 7001, to.tcp_coach_port_ );
    CPPUNIT_ASSERT_EQUAL( 0, to.tcp_olcoach_port_ );
    CPPUNIT_ASSERT_EQUAL( 8080, to.ws_port_ );
    CPPUNIT_ASSERT_EQUAL( std::string( "http://localhost:8000,null" ), to.ws_allowed_origins_ );
}

/*-------------------------------------------------------------------*/
/*!
  ServerParam::toServerString() (used by soccerwindow2 to save the rcg) can be read by rcg::ServerParamT
*/
void
ServerParamTest::testToServerString()
{
    const std::string msg = "(server_param (port 6000)(coach_port 6001)(olcoach_port 6002)"
        "(tcp_port -1)(tcp_coach_port 7001)(tcp_olcoach_port 7002)"
        "(ws_port 8080)(ws_allowed_origins \"http://localhost:8000,null\"))";
    CPPUNIT_ASSERT( ServerParam::instance().parse( msg.c_str(), 20.0 ) );

    const std::string str = ServerParam::i().toServerString();

    CPPUNIT_ASSERT( str.find( "(tcp_port -1)" ) != std::string::npos );
    CPPUNIT_ASSERT( str.find( "(tcp_coach_port 7001)" ) != std::string::npos );
    CPPUNIT_ASSERT( str.find( "(tcp_olcoach_port 7002)" ) != std::string::npos );
    CPPUNIT_ASSERT( str.find( "(ws_port 8080)" ) != std::string::npos );
    CPPUNIT_ASSERT( str.find( "(ws_allowed_origins http://localhost:8000,null)" ) != std::string::npos );

    CerrCapture err;
    ServerParamT param;
    CPPUNIT_ASSERT( param.fromServerString( str ) );

    // The other parameters unknown to rcg::ServerParamT may be warned.
    for ( const char * name : NEW_PARAM_NAMES )
    {
        CPPUNIT_ASSERT_MESSAGE( name, ! has_warning( err.str(), name ) );
    }

    CPPUNIT_ASSERT_EQUAL( -1, param.tcp_port_ );
    CPPUNIT_ASSERT_EQUAL( 7001, param.tcp_coach_port_ );
    CPPUNIT_ASSERT_EQUAL( 7002, param.tcp_olcoach_port_ );
    CPPUNIT_ASSERT_EQUAL( 8080, param.ws_port_ );
    CPPUNIT_ASSERT_EQUAL( std::string( "http://localhost:8000,null" ), param.ws_allowed_origins_ );
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
