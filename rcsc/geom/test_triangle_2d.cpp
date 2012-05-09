// -*-c++-*-

/*!
  \file test_triangle_2d.cpp
  \brief test code for rcsc::Triangle2D
*/

/*
 *Copyright:

 Copyright (C) Hiroki Shimora, Hidehisa Akiyama

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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include "triangle_2d.h"
#include "vector_2d.h"

#include <rcsc/math_util.h>

#include <cppunit/extensions/HelperMacros.h>

using rcsc::EPS;

class Triangle2DTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( Triangle2DTest );
    CPPUNIT_TEST( testSignedArea );
    CPPUNIT_TEST( testCentroid );
    CPPUNIT_TEST_SUITE_END();

public:

    void testSignedArea();
    void testCentroid();
};



CPPUNIT_TEST_SUITE_REGISTRATION( Triangle2DTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
Triangle2DTest::testSignedArea()
{
    //
    // basic checks
    //
    {
        const rcsc::Vector2D p1( 0.0, 0.0 );
        const rcsc::Vector2D p2( 3.0, 0.0 );
        const rcsc::Vector2D p3( 3.0, 4.0 );

        const rcsc::Triangle2D t1( p1, p2, p3 );
        const rcsc::Triangle2D t2( p3, p2, p1 );

        CPPUNIT_ASSERT_DOUBLES_EQUAL( + 6.0 - t1.signedArea(), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( +12.0 - t1.doubleSignedArea(), 0.0, EPS );

        CPPUNIT_ASSERT_DOUBLES_EQUAL( - 6.0 - t2.signedArea(), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( -12.0 - t2.doubleSignedArea(), 0.0, EPS );
    }


    //
    // points on a line
    //
    {
        const rcsc::Vector2D p1( -100, 200 );
        const rcsc::Vector2D p2(  600, 200 );
        const rcsc::Vector2D p3(    0, 200 );

        const rcsc::Triangle2D tri( p1, p2, p3 );

        // should be EXACTRY equal to 0
        CPPUNIT_ASSERT_DOUBLES_EQUAL( tri.doubleSignedArea(), 0.0, EPS );
    }


    //
    // same 2 points
    //
    {
        const rcsc::Vector2D p1( -100, 200 );
        const rcsc::Vector2D p2( + 50, 100 );

        const rcsc::Triangle2D tri1( p1, p1, p2 );
        const rcsc::Triangle2D tri2( p1, p2, p1 );
        const rcsc::Triangle2D tri3( p2, p1, p1 );

        // should be EXACTRY equal to 0
        CPPUNIT_ASSERT_DOUBLES_EQUAL( tri1.doubleSignedArea(), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( tri2.doubleSignedArea(), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( tri3.doubleSignedArea(), 0.0, EPS );
    }


    //
    // same 3 points
    //
    {
        const rcsc::Vector2D p( -100, 200 );

        const rcsc::Triangle2D tri( p, p, p );

        // should be EXACTRY equal to 0
        CPPUNIT_ASSERT_DOUBLES_EQUAL( tri.doubleSignedArea(), 0.0, EPS );
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Triangle2DTest::testCentroid()
{
    {
        rcsc::Vector2D p1( 5.1245, 9.1038 );
        rcsc::Vector2D p2( 3.0, -5.6978 );
        rcsc::Vector2D p3( 3.0, 4.0 );

        rcsc::Triangle2D tri( p1, p2, p3 );


        rcsc::Vector2D c = p1 + p2 + p3;

        CPPUNIT_ASSERT_DOUBLES_EQUAL( c.r() - tri.centroid().r() * 3.0, 0.0, EPS );
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
