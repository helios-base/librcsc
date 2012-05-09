// -*-c++-*-

/*!
  \file test_rect_2d.cpp
  \brief test code for rcsc::Rect2D
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

#ifdef HAVE_CONFIG
#include <config.h>
#endif

#include "rect_2d.h"

#include <rcsc/math_util.h>

#include <cppunit/extensions/HelperMacros.h>

using rcsc::EPS;

class Rect2DTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( Rect2DTest );
    CPPUNIT_TEST( testMove );
    CPPUNIT_TEST( testSet  );
    CPPUNIT_TEST( testIntersected );
    CPPUNIT_TEST( testIntersectedIllegal );
    CPPUNIT_TEST( testUnited );
    CPPUNIT_TEST_SUITE_END();

public:

    void testMove();
    void testSet ();
    void testIntersected();
    void testIntersectedIllegal();
    void testUnited();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Rect2DTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
Rect2DTest::testMove()
{
    rcsc::Rect2D r( rcsc::Vector2D( 0.0, 0.0 ),
                    rcsc::Size2D( 10.0, 10.0 ) );
    rcsc::Rect2D r1 = r;

    r1.moveCenter( rcsc::Vector2D( -5.0, -5.0 ) );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 - r.size().length()*0.5, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 + r.size().length()*0.5, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 - r.size().width()*0.5,  r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 + r.size().width()*0.5,  r1.bottom(), EPS );

    r1 = r;
    r1.moveLeft( -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0,                     r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 + r.size().length(), r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.top(),                  r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.bottom(),               r1.bottom(), EPS );

    r1 = r;
    r1.moveRight( -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 - r.size().length(), r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0,                     r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.top(),                  r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.bottom(),               r1.bottom(), EPS );

    r1 = r;
    r1.moveTop( -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.left(),                r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.right(),               r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0,                    r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 + r.size().width(), r1.bottom(), EPS );

    r1 = r;
    r1.moveBottom( -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.left(),                r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r.right(),               r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0 - r.size().width(), r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0,                    r1.bottom(), EPS );

}

/*-------------------------------------------------------------------*/
/*!

 */
void
Rect2DTest::testSet()
{
    rcsc::Rect2D r( rcsc::Vector2D( 0.0, 0.0 ),
                    rcsc::Size2D( 10.0, 10.0 ) );
    rcsc::Rect2D r1 = r;

    r1.setTopLeft( -5.0, -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 15.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 15.0, r1.size().width(), EPS );

    r1 = r;
    r1.setBottomRight( 5.0, 5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().width(), EPS );

    r1 = r;
    r1.setBottomRight( -5.0, -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().width(), EPS );

    r1 = r;
    r1.setLeft( -5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -5.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 15.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.size().width(), EPS );

    r1 = r;
    r1.setRight( 5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.size().width(), EPS );

    r1 = r;
    r1.setTop( 5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().width(), EPS );

    r1 = r;
    r1.setBottom( 5.0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.left(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.right(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  0.0, r1.top(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.bottom(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10.0, r1.size().length(), EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(  5.0, r1.size().width(), EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Rect2DTest::testIntersected()
{
    //
    // intersected checks
    //
    rcsc::Rect2D r1( rcsc::Vector2D( 0.0, 0.0 ),
                     rcsc::Size2D( 10.0, 10.0 ) );
    rcsc::Rect2D r2( rcsc::Vector2D( 5.0, 5.0 ),
                     rcsc::Size2D( 10.0, 10.0 ) );

    rcsc::Rect2D b1 = r1;
    rcsc::Rect2D b2 = r2;

    rcsc::Rect2D r3 = r1.intersected( r2 );
    r1 &= r2;

    CPPUNIT_ASSERT( r1.isValid() );
    CPPUNIT_ASSERT( r3.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.left()   - r3.left(),   0.0, EPS);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.top()    - r3.top(),    0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.right()  - r3.right(),  0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.bottom() - r3.bottom(), 0.0, EPS );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( b2.left()   - r3.left(),   0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( b1.right()  - r3.right(),  0.0, EPS );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Rect2DTest::testIntersectedIllegal()
{
    //
    // illegal intersected checks
    //
    rcsc::Rect2D r1( rcsc::Vector2D( 10.0, 10.0 ),
                     rcsc::Size2D( 10.0, 10.0 ) );
    rcsc::Rect2D r2( rcsc::Vector2D( 0.0, 0.0 ),
                     rcsc::Size2D( 10.0, 10.0 ) );

    rcsc::Rect2D r3 = r1.intersected( r2 );
    r1 &= r2;

    CPPUNIT_ASSERT( ! r1.isValid() );
    CPPUNIT_ASSERT( ! r3.isValid() );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Rect2DTest::testUnited()
{
    //
    // united checks
    //
    rcsc::Rect2D r1( rcsc::Vector2D( 0.0, 0.0 ),
                     rcsc::Size2D( 10.0, 10.0 ) );
    rcsc::Rect2D r2( rcsc::Vector2D( 5.0, 5.0 ),
                     rcsc::Size2D( 10.0, 10.0 ) );

    rcsc::Rect2D b1 = r1;
    rcsc::Rect2D b2 = r2;

    rcsc::Rect2D r3 = r1.united( r2 );
    r1 |= r2;

    CPPUNIT_ASSERT( r1.isValid() );
    CPPUNIT_ASSERT( r3.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.left()   - r3.left(),   0.0, EPS);
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.top()    - r3.top(),    0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.right()  - r3.right(),  0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( r1.bottom() - r3.bottom(), 0.0, EPS );


    CPPUNIT_ASSERT_DOUBLES_EQUAL( b1.left()   - r3.left(),   0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( b2.right()  - r3.right(),  0.0, EPS );
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
