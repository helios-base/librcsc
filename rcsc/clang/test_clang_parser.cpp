// -*-c++-*-

/*!
  \file test_clang_parser.cpp
  \brief test code for rcsc::CLangparser
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

#include "clang_parser.h"

#include "clang_info_message.h"

#include <rcsc/time/timer.h>
#include <rcsc/game_time.h>

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <cmath>

/*!
  \class CLangParserTest
 */
class CLangParserTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( CLangParserTest );
    CPPUNIT_TEST( testInfoMessage );
    CPPUNIT_TEST_SUITE_END();

public:

    void setUp();
    void tearDown();

protected:

    void testInfoMessage();
};



CPPUNIT_TEST_SUITE_REGISTRATION( CLangParserTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
CLangParserTest::setUp()
{
    //std::cerr << "CLangParserTest setUp" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CLangParserTest::tearDown()
{
    //std::cerr << "CLangParserTest tearDown" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CLangParserTest::testInfoMessage()
{
    rcsc::CLangParser parser;

    //std::string msg = "(info (6000 (true) (do our {1} (mark {2 3}))))";
    //std::string msg = "(info (6000 (true) (do opp {1} (htype 1)))(0 (true) (do opp {2} (htype 2))))";
    // const std::string msg
    //     = "(info (6000 (true) "
    //     "(do opp {1} (htype 1))"
    //     "(do opp {2} (htype 2))"
    //     "(do our {1} (mark {2 3}))"
    //     "))";

    // const std::string msg = "(info (6000 (true) (do opp {1} (htype -1)) (do opp {2} (htype -1)) (do opp {3} (htype -1)) (do opp {4} (htype -1)) (do opp {5} (htype -1)) (do opp {6} (htype -1)) (do opp {7} (htype -1)) (do opp {8} (htype -1)) (do opp {9} (htype 10)) (do opp {10} (htype -1)) (do opp {11} (htype -1))))";

    const std::string msg = "(info (6000 (true) (dont our {1} (mark {0})) (dont our {2} (mark {0})) (dont our {3} (mark {0})) (dont our {4} (mark {0})) (dont our {5} (mark {0})) (dont our {6} (mark {0})) (dont our {7} (mark {0})) (dont our {8} (mark {0})) (dont our {9} (mark {0})) (dont our {10} (mark {0})) (dont our {11} (mark {0}))))";

    std::cout << '\n';
    std::cout << msg << '\n';

    rcsc::Timer timer;
    CPPUNIT_ASSERT( parser.parse( msg ) );
    std::cout << "elapsed " << timer.elapsedReal() << " [ms]" << std::endl;


    std::shared_ptr< const rcsc::CLangInfoMessage > info;

    try
    {
        info = std::dynamic_pointer_cast< const rcsc::CLangInfoMessage >( parser.message() );
        if ( ! info )
        {
            std::cerr << "Failed dynamic_pointer_cast " << std::endl;
        }

        std::cout << "parsed tokens:\n";
        for ( const rcsc::CLangToken & tok = info->tokens() )
        {
            std::cout << "    " << *tok << std::endl;
        }
    }
    catch ( std::exception & e )
    {
        std::cerr << e.what() << " CLangInfoMessage" << std::endl;
    }
}


/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/

#include <cppunit/BriefTestProgressListener.h>
//#include <cppunit/TextTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
//#include <cppunit/TextOutputter.h>
//#include <cppunit/XmlOutputter.h>
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

    //CPPUNIT_NS::TextTestProgressListener textprog;
    //controller.addListener( &textprog );

    // add the top suite to the test runner.
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );
    runner.run( controller );

    // output results in a compiler compatible format
    CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
    //CPPUNIT_NS::TextOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
    //CPPUNIT_NS::XmlOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
    outputter.write();

    return result.wasSuccessful() ? 0 : 1;
}
