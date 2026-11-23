// -*-c++-*-

/*!
  \file test_types.cpp
  \brief test code for the data types of the rcg parser library
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

#include "types.h"

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <sstream>
#include <regex>
#include <map>
#include <string>

using rcsc::rcg::PlayerTypeT;

namespace {

/*!
  \brief the pairs of the parameter name and the printed value
*/
using PrintedValues = std::map< std::string, std::string >;

/*!
  \brief get the parameters from the output of PlayerTypeT::toServerString(), e.g. "(player_type (id 0)(player_speed_max 1.05)...)"
*/
PrintedValues
parse_server_string( const std::string & str )
{
    PrintedValues values;
    const std::regex re( "\\(([a-z_]+) ([^()]+)\\)" );
    for ( std::sregex_iterator it( str.begin(), str.end(), re ), end; it != end; ++it )
    {
        values[ (*it)[1].str() ] = (*it)[2].str();
    }
    return values;
}

/*!
  \brief get the parameters from the output of PlayerTypeT::toJSON(), e.g. {"player_type":{"id":0,"player_speed_max":1.05,...}}
*/
PrintedValues
parse_json( const std::string & str )
{
    PrintedValues values;
    const std::regex re( "\"([a-z_]+)\":([-+0-9.eE]+)" );
    for ( std::sregex_iterator it( str.begin(), str.end(), re ), end; it != end; ++it )
    {
        values[ (*it)[1].str() ] = (*it)[2].str();
    }
    return values;
}

std::string
to_server_string( const PlayerTypeT & param )
{
    std::ostringstream os;
    param.toServerString( os );
    return os.str();
}

std::string
to_json( const PlayerTypeT & param )
{
    std::ostringstream os;
    param.toJSON( os );
    return os.str();
}

/*!
  \brief check the printed value of the parameter
*/
void
check_value( const PrintedValues & values,
             const std::string & name,
             const double expected,
             const std::string & format )
{
    const PrintedValues::const_iterator it = values.find( name );
    CPPUNIT_ASSERT_MESSAGE( format + ": " + name + " is not printed",
                            it != values.end() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( format + ": " + name + " " + it->second,
                                          expected, std::stod( it->second ), 1.0e-9 );
}

}

/*-------------------------------------------------------------------*/
/*!

*/
class RcgTypesTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( RcgTypesTest );
    CPPUNIT_TEST( testPlayerTypeNoiseRate );
    CPPUNIT_TEST( testPlayerTypeAllParams );
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

protected:
    void testPlayerTypeNoiseRate();
    void testPlayerTypeAllParams();
};

// regist test
CPPUNIT_TEST_SUITE_REGISTRATION( RcgTypesTest );

/*-------------------------------------------------------------------*/
void
RcgTypesTest::setUp()
{

}

/*-------------------------------------------------------------------*/
void
RcgTypesTest::tearDown()
{

}

/*-------------------------------------------------------------------*/
/*!
  the noise rate parameters of v19
*/
void
RcgTypesTest::testPlayerTypeNoiseRate()
{
    PlayerTypeT param;
    param.dist_noise_rate_ = 0.0125;
    param.focus_dist_noise_rate_ = 0.0225;
    param.land_dist_noise_rate_ = 0.00125;
    param.land_focus_dist_noise_rate_ = 0.00225;

    const std::string str = to_server_string( param );
    {
        const PrintedValues values = parse_server_string( str );
        check_value( values, "dist_noise_rate", 0.0125, "server string" );
        check_value( values, "focus_dist_noise_rate", 0.0225, "server string" );
        check_value( values, "land_dist_noise_rate", 0.00125, "server string" );
        check_value( values, "land_focus_dist_noise_rate", 0.00225, "server string" );
    }
    {
        const PrintedValues values = parse_json( to_json( param ) );
        check_value( values, "dist_noise_rate", 0.0125, "JSON" );
        check_value( values, "focus_dist_noise_rate", 0.0225, "JSON" );
        check_value( values, "land_dist_noise_rate", 0.00125, "JSON" );
        check_value( values, "land_focus_dist_noise_rate", 0.00225, "JSON" );
    }

    // read the output again
    PlayerTypeT read;
    CPPUNIT_ASSERT( read.fromServerString( str ) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0125, read.dist_noise_rate_, 1.0e-9 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0225, read.focus_dist_noise_rate_, 1.0e-9 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.00125, read.land_dist_noise_rate_, 1.0e-9 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.00225, read.land_focus_dist_noise_rate_, 1.0e-9 );
}

/*-------------------------------------------------------------------*/
/*!
  Each parameter printed by toServerString() and toJSON() has its own value.
  A different value is set to each parameter by its name, so a parameter
  printed with the value of another member is detected.
*/
void
RcgTypesTest::testPlayerTypeAllParams()
{
    // the parameter names printed by toServerString()
    const PrintedValues names = parse_server_string( to_server_string( PlayerTypeT() ) );
    CPPUNIT_ASSERT( names.size() >= 30 ); // id and 29 parameters (v19)
    CPPUNIT_ASSERT( names.count( "id" ) == 1 );

    // set a different value to each parameter.
    // The values are exact in binary and are not changed by the quantization of the output.
    PlayerTypeT param;
    std::map< std::string, double > expected;
    double value = 1.0;
    for ( const PrintedValues::value_type & v : names )
    {
        if ( v.first == "id" )
        {
            CPPUNIT_ASSERT_MESSAGE( v.first, param.setInt( v.first, 100 ) );
            expected[ v.first ] = 100.0;
        }
        else
        {
            value += 0.125;
            CPPUNIT_ASSERT_MESSAGE( v.first, param.setDouble( v.first, value ) );
            expected[ v.first ] = value;
        }
    }

    const PrintedValues server_values = parse_server_string( to_server_string( param ) );
    const PrintedValues json_values = parse_json( to_json( param ) );

    // both formats print the same parameters
    CPPUNIT_ASSERT_EQUAL( names.size(), server_values.size() );
    CPPUNIT_ASSERT_EQUAL( names.size(), json_values.size() );

    for ( const std::map< std::string, double >::value_type & v : expected )
    {
        check_value( server_values, v.first, v.second, "server string" );
        check_value( json_values, v.first, v.second, "JSON" );
    }
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
