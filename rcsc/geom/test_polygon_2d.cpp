// -*-c++-*-

/*!
  \file test_polygon_2d.cpp
  \brief test code for rcsc::Polygon2D
*/

/*
 *Copyright:

 Copyright (C) Hiroki Shimora

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

#include "polygon_2d.h"

#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/rect_2d.h>
#include <rcsc/math_util.h>

#include <cppunit/extensions/HelperMacros.h>

#include <vector>
#include <cstdlib>


using rcsc::EPS;


class Polygon2DTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( Polygon2DTest );
    CPPUNIT_TEST( testEmpty );
    CPPUNIT_TEST( testPointPolygon );
    CPPUNIT_TEST( testGetBoundingBox );
    CPPUNIT_TEST( testContains1 );
    CPPUNIT_TEST( testContains2 );
    CPPUNIT_TEST( testContains3 );
    CPPUNIT_TEST( testContains4 );
    CPPUNIT_TEST( testContains5 );
    CPPUNIT_TEST( testContains6 );
    CPPUNIT_TEST( testContains7 );
    CPPUNIT_TEST( testContains8 );
    CPPUNIT_TEST( testEmptyArea );
    CPPUNIT_TEST( testScissoring );
    CPPUNIT_TEST( testGetDistance );
    CPPUNIT_TEST( testXYCenter );
    CPPUNIT_TEST( testSignedArea2 );
    CPPUNIT_TEST_SUITE_END();

public:

    void testEmpty();
    void testPointPolygon();
    void testGetBoundingBox();
    void testContains1();
    void testContains2();
    void testContains3();
    void testContains4();
    void testContains5();
    void testContains6();
    void testContains7();
    void testContains8();
    void testEmptyArea();
    void testScissoring();
    void testGetDistance();
    void testXYCenter();
    void testSignedArea2();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Polygon2DTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testEmpty()
{
    //
    // empty
    //
    const rcsc::Polygon2D empty_polygon;

    CPPUNIT_ASSERT( !empty_polygon.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testPointPolygon()
{
    //
    // point polygon
    //
    const rcsc::Vector2D p( +100.0, +100.0 );

    std::vector< rcsc::Vector2D > v;
    v.push_back( p );

    const rcsc::Polygon2D point_polygon( v );

    CPPUNIT_ASSERT( !point_polygon.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );

    // strict checks
    CPPUNIT_ASSERT(  point_polygon.contains( p ) );
    CPPUNIT_ASSERT( !point_polygon.contains( p, false ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testGetBoundingBox()
{
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( +200.0, +100.0 );
    rect.emplace_back( -200.0, +100.0 );
    rect.emplace_back( -200.0, -100.0 );
    rect.emplace_back( +200.0, -100.0 );

    const rcsc::Polygon2D rectangle( rect );


    //
    // getBoundingBox()
    //
    const rcsc::Rect2D r = rectangle.getBoundingBox();

    CPPUNIT_ASSERT_DOUBLES_EQUAL( -200.0 - r.minX(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( +200.0 - r.maxX(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -100.0 - r.minY(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( +100.0 - r.maxY(), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains1()
{
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( +200.0, +100.0 );
    rect.emplace_back( -200.0, +100.0 );
    rect.emplace_back( -200.0, -100.0 );
    rect.emplace_back( +200.0, -100.0 );

    const rcsc::Polygon2D rectangle( rect );

    //
    // contains
    //
    CPPUNIT_ASSERT(  rectangle.contains( rcsc::Vector2D(    0.0,    0.0 ) ) );
    CPPUNIT_ASSERT(  rectangle.contains( rcsc::Vector2D(   50.0,   50.0 ) ) );
    CPPUNIT_ASSERT(  rectangle.contains( rcsc::Vector2D(  199.9,   99.9 ) ) );
    CPPUNIT_ASSERT(  rectangle.contains( rcsc::Vector2D( -199.9, - 99.9 ) ) );
    CPPUNIT_ASSERT( !rectangle.contains( rcsc::Vector2D(  200.1,  100.1 ) ) );
    CPPUNIT_ASSERT( !rectangle.contains( rcsc::Vector2D( -200.1, -100.1 ) ) );
    CPPUNIT_ASSERT( !rectangle.contains( rcsc::Vector2D( +500.0, +500.0 ) ) );
    CPPUNIT_ASSERT( !rectangle.contains( rcsc::Vector2D(    0.0, +500.0 ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains2()
{
    //
    // contains 2
    //
    std::vector< rcsc::Vector2D > tri;
    tri.emplace_back( -200.0, -100.0 );
    tri.emplace_back(    0.0, +100.0 );
    tri.emplace_back( +200.0, -100.0 );

    const rcsc::Polygon2D triangle( tri );

    CPPUNIT_ASSERT(  triangle.contains( rcsc::Vector2D( 0.0,    0.0 ) ) );
    CPPUNIT_ASSERT( !triangle.contains( rcsc::Vector2D( 0.0, -300.0 ) ) );
    CPPUNIT_ASSERT( !triangle.contains( rcsc::Vector2D( 0.1, -300.0 ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains3()
{
    //
    // contains 3
    //
    std::vector< rcsc::Vector2D > tri2;
    tri2.emplace_back(   0.0,   0.0 );
    tri2.emplace_back( 100.0, 100.0 );
    tri2.emplace_back(   0.0, 200.0 );

    const rcsc::Polygon2D triangle2( tri2 );

    CPPUNIT_ASSERT( !triangle2.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
    CPPUNIT_ASSERT(  triangle2.contains( rcsc::Vector2D(   50.0, 100.0 ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains4()
{
    //
    // contains 4
    //
    std::vector< rcsc::Vector2D > tri3;
    tri3.emplace_back(   0.0,   0.0 );
    tri3.emplace_back( 100.0, 100.0 );
    tri3.emplace_back( 100.0, 100.0 );
    tri3.emplace_back(   0.0, 200.0 );

    const rcsc::Polygon2D triangle3( tri3 );

    CPPUNIT_ASSERT( !triangle3.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains5()
{
    //
    // contains 5
    //
    std::vector< rcsc::Vector2D > tri4;
    tri4.emplace_back(   0.0,   0.0 );
    tri4.emplace_back( 100.0, 100.0 );
    tri4.emplace_back( 100.0, 100.0 );
    tri4.emplace_back( 100.0, 100.0 );
    tri4.emplace_back(   0.0, 200.0 );

    const rcsc::Polygon2D triangle4( tri4 );

    CPPUNIT_ASSERT( !triangle4.contains( rcsc::Vector2D( -100.0, 100.0 ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains6()
{
    //
    // contains 6
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back(  0,  0 );
    rect.emplace_back( 10,  0 );
    rect.emplace_back( 10, 10 );
    rect.emplace_back(  0, 10 );

    const rcsc::Polygon2D r( rect );

    CPPUNIT_ASSERT( ! r.contains( rcsc::Vector2D( -100, 0 ) ) );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains7()
{
    //
    // contains (grid)
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back(  0,  0 );
    rect.emplace_back( 10,  0 );
    rect.emplace_back( 10, 10 );
    rect.emplace_back(  0, 10 );

    const rcsc::Polygon2D r( rect );

    int count = 0;

    for ( int x = -100; x <= +100; ++x )
    {
        for ( int y = -100; y <= +100; ++y )
        {
            if (    0 <= x && x <= 10
                    && 0 <= y && y <= 10 )
            {
                continue;
            }

            if ( r.contains( rcsc::Vector2D( x, y ) ) )
            {
                ++count;
            }
        }
    }

    CPPUNIT_ASSERT_EQUAL( 0, count );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testContains8()
{
    //
    // contains
    //
    std::vector< rcsc::Vector2D > v;
    v.emplace_back( 100, 100 );
    v.emplace_back( 200, 100 );
    v.emplace_back( 200, 500 );

    const rcsc::Polygon2D tri( v );

    //                    //
    //  po1               //
    //                    //
    //  po2          p5   //
    //              /|    //
    //             / |    //
    //            /  |    //
    //           /   |    //
    //          /    |    //
    //         /     |    //
    //  po3  p7  p1  p6   //
    //       /       |    //
    //  po4 p4---p2--p3   //
    //                    //
    //  po5               //

    rcsc::Vector2D p1( 150, 150 );
    rcsc::Vector2D p2( 150, 100 );
    rcsc::Vector2D p3( 200, 100 );
    rcsc::Vector2D p4( 100, 100 );
    rcsc::Vector2D p5( 200, 500 );
    rcsc::Vector2D p6( 200, 150 );
    rcsc::Vector2D p7( 200, 150 );

    rcsc::Vector2D po1( 50, 600 );
    rcsc::Vector2D po2( 50, 500 );
    rcsc::Vector2D po3( 50, 150 );
    rcsc::Vector2D po4( 50, 100 );
    rcsc::Vector2D po5( 50,   0 );


    CPPUNIT_ASSERT( tri.contains( p1 ) );
    CPPUNIT_ASSERT( tri.contains( p1, false ) );

    CPPUNIT_ASSERT(  tri.contains( p2 ) );
    CPPUNIT_ASSERT( !tri.contains( p2, false ) );

    CPPUNIT_ASSERT(  tri.contains( p3 ) );
    CPPUNIT_ASSERT( !tri.contains( p3, false ) );

    CPPUNIT_ASSERT(  tri.contains( p4 ) );
    CPPUNIT_ASSERT( !tri.contains( p4, false ) );

    CPPUNIT_ASSERT(  tri.contains( p5 ) );
    CPPUNIT_ASSERT( !tri.contains( p5, false ) );

    CPPUNIT_ASSERT(  tri.contains( p6 ) );
    CPPUNIT_ASSERT( !tri.contains( p6, false ) );

    CPPUNIT_ASSERT(  tri.contains( p7 ) );
    CPPUNIT_ASSERT( !tri.contains( p7, false ) );


    CPPUNIT_ASSERT( !tri.contains( po1 ) );
    CPPUNIT_ASSERT( !tri.contains( po1, false ) );

    CPPUNIT_ASSERT( !tri.contains( po2 ) );
    CPPUNIT_ASSERT( !tri.contains( po2, false ) );

    CPPUNIT_ASSERT( !tri.contains( po3 ) );
    CPPUNIT_ASSERT( !tri.contains( po3, false ) );

    CPPUNIT_ASSERT( !tri.contains( po4 ) );
    CPPUNIT_ASSERT( !tri.contains( po4, false ) );

    CPPUNIT_ASSERT( !tri.contains( po5 ) );
    CPPUNIT_ASSERT( !tri.contains( po5, false ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testEmptyArea()
{
    //
    // empty area
    //
    std::vector< rcsc::Vector2D > a0;
    a0.emplace_back( 100.0, 100.0 );
    a0.emplace_back( 100.0, 100.0 );
    a0.emplace_back( 100.0, 100.0 );
    a0.emplace_back( 100.0, 100.0 );
    a0.emplace_back( 100.0, 100.0 );

    const rcsc::Polygon2D area_1( a0 );

    a0.emplace_back( 100.0, 100.0 );
    const rcsc::Polygon2D area_2( a0 );


    CPPUNIT_ASSERT( !area_1.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );
    CPPUNIT_ASSERT( !area_2.contains( rcsc::Vector2D( 0.0, 0.0 ) ) );

    // strict checks
    CPPUNIT_ASSERT(  area_1.contains( rcsc::Vector2D( 100.0, 100.0 ) ) );
    CPPUNIT_ASSERT( !area_1.contains( rcsc::Vector2D( 100.0, 100.0 ), false ) );

    // strict checks
    CPPUNIT_ASSERT(  area_2.contains( rcsc::Vector2D( 100.0, 100.0 ) ) );
    CPPUNIT_ASSERT( !area_2.contains( rcsc::Vector2D( 100.0, 100.0 ), false ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testScissoring()
{
    //
    // scissoring
    //
    const rcsc::Rect2D rectangle( rcsc::Vector2D( -100, -100 ),
                                  rcsc::Size2D( /* length of x */ 200, /* length of y */ 200 ) );

    //                         //
    //              (200,200)  //
    //           +---------+   //
    //           |         |   //
    //    -100   |         |   //
    // +100 +----|----+    |   //
    //      |    |    |    |   //
    //      |    |    |    |   //
    //      |    +---------+   //
    //      |   (0,0) |        //
    //      |         |        //
    // -100 +---------+        //
    //                         //

    std::vector< rcsc::Vector2D > v;
    v.emplace_back(   0,   0 );
    v.emplace_back( 200,   0 );
    v.emplace_back( 200, 200 );
    v.emplace_back(   0, 200 );
    v.emplace_back(   0,   0 );

    const rcsc::Polygon2D polygon( v );

    const rcsc::Polygon2D result = polygon.getScissoredConnectedPolygon( rectangle );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( 10000.0 - result.area(), 0.0, EPS );

    const rcsc::Rect2D bbox = result.getBoundingBox();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(   0.0 - bbox.minX(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 100.0 - bbox.maxX(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(   0.0 - bbox.minY(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 100.0 - bbox.maxY(), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testGetDistance()
{
    //
    // get_distance
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back(  0,  0 );
    rect.emplace_back( 10,  0 );
    rect.emplace_back( 10, 10 );
    rect.emplace_back(  0, 10 );

    const rcsc::Polygon2D r( rect );

    // out of polygon
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 1.0 - r.dist( rcsc::Vector2D( 11.0, 10.0 ) ), 0.0, EPS );

    // in polygon, check as plane
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 0.0 - r.dist( rcsc::Vector2D( 5.0, 5.0 ) ), 0.0, EPS );

    // in polygon, check as polyline
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 5.0 - r.dist( rcsc::Vector2D( 5.0, 5.0 ), false ), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testXYCenter()
{
    //
    // area, xyCenter
    //
    std::vector< rcsc::Vector2D > rect;
    rect.emplace_back( 10, 10 );
    rect.emplace_back( 20, 10 );
    rect.emplace_back( 20, 20 );
    rect.emplace_back( 10, 20 );

    const rcsc::Polygon2D r( rect );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( +100.0 - r.area(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( +200.0 - r.doubleSignedArea(), 0.0, EPS );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 15.0, 15.0 ).dist( r.xyCenter() ), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Polygon2DTest::testSignedArea2()
{
    //
    // counter clockwise/clockwise, doubleSignedArea
    //
    std::vector< rcsc::Vector2D > points;
    const rcsc::Polygon2D empty(points);

    points.emplace_back( 10, 10 );
    const rcsc::Polygon2D point(points);

    points.emplace_back( 20, 10 );
    const rcsc::Polygon2D line(points);

    points.emplace_back( 20, 20 );
    const rcsc::Polygon2D triangle(points);

    points.emplace_back( 10, 20 );
    const rcsc::Polygon2D rectangle(points);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(    0.0 - empty    .doubleSignedArea(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(    0.0 - point    .doubleSignedArea(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL(    0.0 - line     .doubleSignedArea(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( +100.0 - triangle .doubleSignedArea(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( +200.0 - rectangle.doubleSignedArea(), 0.0, EPS );


    CPPUNIT_ASSERT_EQUAL( false, empty.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( false, empty.isClockwise() );

    CPPUNIT_ASSERT_EQUAL( false, point.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( false, point.isClockwise() );

    CPPUNIT_ASSERT_EQUAL( false, line.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( false, line.isClockwise() );

    CPPUNIT_ASSERT_EQUAL( true , triangle.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( false, triangle.isClockwise() );

    CPPUNIT_ASSERT_EQUAL( true , triangle.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( false, triangle.isClockwise() );


    std::vector< rcsc::Vector2D > r_points;
    r_points.emplace_back( 10, 20 );
    r_points.emplace_back( 20, 20 );
    r_points.emplace_back( 20, 10 );
    const rcsc::Polygon2D r_triangle(r_points);

    r_points.emplace_back( 10, 10 );
    const rcsc::Polygon2D r_rectangle(r_points);

    CPPUNIT_ASSERT_DOUBLES_EQUAL( -100.0 - r_triangle .doubleSignedArea(), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( -200.0 - r_rectangle.doubleSignedArea(), 0.0, EPS );

    CPPUNIT_ASSERT_EQUAL( false, r_triangle.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( true , r_triangle.isClockwise() );

    CPPUNIT_ASSERT_EQUAL( false, r_rectangle.isCounterclockwise() );
    CPPUNIT_ASSERT_EQUAL( true , r_rectangle.isClockwise() );
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
