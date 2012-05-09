// -*-c++-*-

/*!
  \file test_segment_2d.cpp
  \brief test code for rcsc::Segment2D
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

#include "segment_2d.h"
#include "vector_2d.h"

#include <rcsc/math_util.h>

#include <cppunit/extensions/HelperMacros.h>

using rcsc::EPS;
using rcsc::AngleDeg;
using rcsc::Vector2D;
using rcsc::Segment2D;


class Segment2DTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( Segment2DTest );
    CPPUNIT_TEST( testLength );
    CPPUNIT_TEST( testProjection );
    CPPUNIT_TEST( testIntersection );
    CPPUNIT_TEST( testExistIntersectionExceptTerminalPoint );
    CPPUNIT_TEST( testExistIntersection );
    CPPUNIT_TEST( testExistIntersectionAtTerminalPoints );
    CPPUNIT_TEST( testIntersectsAtTerminalPoints );
    CPPUNIT_TEST( testIntersectsAtTerminalPointsParallelHorizontal );
    CPPUNIT_TEST( testIntersectsAtTerminalPointsParallelVertical );
    CPPUNIT_TEST( testIntersectWithPointSegment );
    CPPUNIT_TEST( testNearestPoint );
    CPPUNIT_TEST( testDistanceFromPoint );
    CPPUNIT_TEST( testDistanceFromPointOnLine );
    CPPUNIT_TEST( testDistanceFromPointComplex );
    CPPUNIT_TEST( testDistanceFromSegment );
    CPPUNIT_TEST( testOnSegmentStrictly );
    CPPUNIT_TEST( testOnSegmentWeakly );
    CPPUNIT_TEST( testEquals );
    CPPUNIT_TEST_SUITE_END();

public:

    void testLength();
    void testProjection();
    void testIntersection();
    void testExistIntersectionExceptTerminalPoint();
    void testExistIntersection();
    void testExistIntersectionAtTerminalPoints();
    void testIntersectsAtTerminalPoints();
    void testIntersectsAtTerminalPointsParallelHorizontal();
    void testIntersectsAtTerminalPointsParallelVertical();
    void testIntersectWithPointSegment();
    void testNearestPoint();
    void testDistanceFromPoint();
    void testDistanceFromPointOnLine();
    void testDistanceFromPointComplex();
    void testDistanceFromSegment();
    void testOnSegmentStrictly();
    void testOnSegmentWeakly();
    void testEquals();
};



CPPUNIT_TEST_SUITE_REGISTRATION( Segment2DTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testLength()
{
    //
    // check length of segment
    //
    const Segment2D s1( Vector2D( 0.0, 0.0 ),
                        Vector2D( 3.0, 4.0 ) );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( 5.0 - s1.length(), 0.0, EPS );
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testProjection()
{
    rcsc::Segment2D s1( rcsc::Vector2D( 0.0, 0.0 ),
                        rcsc::Vector2D( 0.0, 4.0 ) );
    CPPUNIT_ASSERT( s1.projection( rcsc::Vector2D( 2.0, 0.0 ) ).isValid() );
    CPPUNIT_ASSERT( s1.projection( rcsc::Vector2D( 2.0, 2.0 ) ).isValid() );
    CPPUNIT_ASSERT( s1.projection( rcsc::Vector2D( 2.0, 4.0 ) ).isValid() );
    CPPUNIT_ASSERT( ! s1.projection( rcsc::Vector2D( 2.0, 4.0 + 1.0e-4 ) ).isValid() );

    rcsc::Segment2D s2( rcsc::Vector2D( 0.0, 1.0 ),
                        rcsc::Vector2D( 1.0, 0.0 ) );
    CPPUNIT_ASSERT( s2.projection( rcsc::Vector2D( 0.0, 1.0 ) ).isValid() );
    CPPUNIT_ASSERT( s2.projection( rcsc::Vector2D( 1.0, 1.0 ) ).isValid() );
    CPPUNIT_ASSERT( s2.projection( rcsc::Vector2D( 1.0, 0.0 ) ).isValid() );
    CPPUNIT_ASSERT( s2.projection( rcsc::Vector2D( 2.0, 1.0 ) ).isValid() );
    CPPUNIT_ASSERT( s2.projection( rcsc::Vector2D( 1.0, 2.0 ) ).isValid() );


    const double delta = 1.0e-6;
    const Segment2D segment( Vector2D( 0.0, 0.0 ),
                             Vector2D( 2.0, 0.0 ) );
    const Vector2D dir = ( segment.terminal() - segment.origin() ).normalizedVector();

    Vector2D proj;

    proj = segment.projection( Vector2D( 0.0, 0.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 0.0, EPS );

    proj = segment.projection( Vector2D( 1.0, 0.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 1.0, EPS );

    proj = segment.projection( Vector2D( 2.0, 0.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 2.0, EPS );

    proj = segment.projection( Vector2D( -delta * 0.5, -delta * 0.5 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 0.0, delta );

    proj = segment.projection( Vector2D( 1.0, delta * 0.5 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 1.0, EPS );

    proj = segment.projection( Vector2D( 1.0, -delta * 0.5 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 1.0, EPS );

    proj = segment.projection( Vector2D( 2.0 + delta * 0.5, delta * 0.5 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 2.0, delta );

    proj = segment.projection( Vector2D( -1.0, 0.0 ) );
    CPPUNIT_ASSERT( ! proj.isValid() );

    proj = segment.projection( Vector2D( -1.0, -1.0 ) );
    CPPUNIT_ASSERT( ! proj.isValid() );

    proj = segment.projection( Vector2D( 0.0, 1.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 0.0, EPS );

    proj = segment.projection( Vector2D( 2.0, -1.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 2.0, EPS );

    proj = segment.projection( Vector2D( 3.0, 0.0 ) );
    CPPUNIT_ASSERT( ! proj.isValid() );

    proj = segment.projection( Vector2D( 3.0, 1.0 ) );
    CPPUNIT_ASSERT( ! proj.isValid() );

    proj = segment.projection( Vector2D( -delta * 2.0, 0.0 ) );
    CPPUNIT_ASSERT( ! proj.isValid() );

    proj = segment.projection( Vector2D( 0.0, -delta * 2.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 0.0, EPS );

    proj = segment.projection( Vector2D( 2.0, delta * 2.0 ) );
    CPPUNIT_ASSERT( proj.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( dir.innerProduct( proj - segment.origin() ), 2.0, EPS );

    proj = segment.projection( Vector2D( 2.0 + delta * 2.0, 0.0 ) );
    CPPUNIT_ASSERT( ! proj.isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testIntersection()
{
    const double delta = 1.0e-6;
    const Segment2D segment( Vector2D( 0.0, 0.0 ),
                             Vector2D( 2.0, 0.0 ) );

    //
    Vector2D result;
    Segment2D s( Vector2D( 0.0, 0.0 ), Vector2D( 0.0, 0.0 ) );

    s.assign( Vector2D( 0.0, 0.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 0.0, EPS );

    s.assign( Vector2D( 0.0, 1.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 1.0, EPS );

    s.assign( Vector2D( 0.0, 2.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 2.0, EPS );

    s.assign( Vector2D( 1.0, 0.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 1.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 0.0, EPS );

    s.assign( Vector2D( 0.0, -1.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 1.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), std::sqrt( 2.0 ), EPS );

    s.assign( Vector2D( -1.0, -2.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 1.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), std::sqrt( 2.0 ) * 2.0, EPS );

    s.assign( Vector2D( 2.0, 0.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 2.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 0.0, EPS );

    s.assign( Vector2D( 2.0, -1.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 2.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 1.0, EPS );

    s.assign( Vector2D( 2.0, -2.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( result.isValid() );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( segment.origin() ), 2.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( result.dist( s.origin() ), 2.0, EPS );

    s.assign( Vector2D( 0.0, -delta * 2.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s =  Segment2D( Vector2D( -delta * 2.0, 1.0 ), 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s.assign( Vector2D( 0.0, 2.0 ), 2.0 - delta * 2.0, AngleDeg( -90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s.assign( Vector2D( 1.0 + delta * 2.0, delta * 2.0 ), std::sqrt( 2.0 ) * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s.assign( Vector2D( -1.0, -2.0 ), std::sqrt( 2.0 ) * 2.0 - delta * 2.0, AngleDeg( 45.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s.assign( Vector2D( 2.0, delta * 2.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s.assign( Vector2D( 2.0 + delta * 2.0, -1.0 ), 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );

    s.assign( Vector2D( 2.0, -2.0 ), 2.0 - delta * 2.0, AngleDeg( 90.0 ) );
    result = segment.intersection( s, true );
    CPPUNIT_ASSERT( ! result.isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testExistIntersectionExceptTerminalPoint()
{
    //
    // check existIntersectionExceptTerminalPoint()
    //
    const rcsc::Segment2D s1( rcsc::Vector2D( 0.0, 0.0 ),
                              rcsc::Vector2D( 3.0, 4.0 ) );

    const rcsc::Segment2D s2( rcsc::Vector2D( 0.0, 2.0 ),
                              rcsc::Vector2D( 5.0, 2.0 ) );

    CPPUNIT_ASSERT( s1.existIntersectionExceptEndpoint( s2 ) );
    CPPUNIT_ASSERT( s2.existIntersectionExceptEndpoint( s1 ) );

    CPPUNIT_ASSERT( s1.intersection( s2, true ).isValid() );
    CPPUNIT_ASSERT( s2.intersection( s1, true ).isValid() );
    CPPUNIT_ASSERT( s1.intersection( s2, false ).isValid() );
    CPPUNIT_ASSERT( s2.intersection( s1, false ).isValid() );

    //     const rcsc::Segment2D s3( rcsc::Vector2D( 100.0, 200.0 ),
    //                               rcsc::Vector2D( 300.0, 400.0 ) );
    //     const rcsc::Segment2D s3( rcsc::Vector2D( s1.origin().x - 1.0, s1.origin().y + 1.0 ),
    //                               rcsc::Vector2D( s1.origin().x + 1.0, s1.origin().y - 1.0 ) );
    const rcsc::Segment2D s3( rcsc::Vector2D( s1.terminal().x - 1.0, s1.terminal().y + 1.0 ),
                              rcsc::Vector2D( s1.terminal().x + 1.0, s1.terminal().y - 1.0 ) );

    CPPUNIT_ASSERT( ! s3.existIntersectionExceptEndpoint( s1 ) );
    CPPUNIT_ASSERT( ! s3.existIntersectionExceptEndpoint( s2 ) );
    CPPUNIT_ASSERT( ! s1.existIntersectionExceptEndpoint( s3 ) );
    CPPUNIT_ASSERT( ! s2.existIntersectionExceptEndpoint( s3 ) );

    CPPUNIT_ASSERT( s3.intersection( s1, true ).isValid() );
    CPPUNIT_ASSERT( ! s3.intersection( s2, true ).isValid() );
    CPPUNIT_ASSERT( s1.intersection( s3, true ).isValid() );
    CPPUNIT_ASSERT( ! s2.intersection( s3, true ).isValid() );
    CPPUNIT_ASSERT( ! s3.intersection( s1, false ).isValid() );
    CPPUNIT_ASSERT( ! s3.intersection( s2, false ).isValid() );
    CPPUNIT_ASSERT( ! s1.intersection( s3, false ).isValid() );
    CPPUNIT_ASSERT( ! s2.intersection( s3, false ).isValid() );


    // 2 segments on a line
    const rcsc::Segment2D s1_2( rcsc::Vector2D( 6.0,  8.0 ),
                                rcsc::Vector2D( 9.0, 12.0 ) );

    CPPUNIT_ASSERT( ! s1.existIntersectionExceptEndpoint( s1_2 ) );
    CPPUNIT_ASSERT( ! s1_2.existIntersectionExceptEndpoint( s1 ) );

    CPPUNIT_ASSERT( ! s1.intersection( s1_2, true ).isValid() );
    CPPUNIT_ASSERT( ! s1_2.intersection( s1, true ).isValid() );
    CPPUNIT_ASSERT( ! s1.intersection( s1_2, false ).isValid() );
    CPPUNIT_ASSERT( ! s1_2.intersection( s1, false ).isValid() );


    const rcsc::Segment2D s4( rcsc::Vector2D( -100.0, 4.0 ),
                              rcsc::Vector2D( +100.0, 4.0 ) );

    CPPUNIT_ASSERT( s1.existIntersection( s4 ) );
    CPPUNIT_ASSERT( s4.existIntersection( s1 ) );

    CPPUNIT_ASSERT( s1.intersection( s4, true ).isValid() );
    CPPUNIT_ASSERT( s4.intersection( s1, true ).isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testExistIntersection()
{
    //
    // check existIntersection()
    //
    const rcsc::Segment2D t1( rcsc::Vector2D( 100, 100 ),
                              rcsc::Vector2D(   0, 200 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D( -100, 200 ),
                              rcsc::Vector2D(  600, 200 ) );

    CPPUNIT_ASSERT( t1.existIntersection( t2 ) );
    CPPUNIT_ASSERT( t2.existIntersection( t1 ) );

    CPPUNIT_ASSERT( t1.intersection( t2, true ).isValid() );
    CPPUNIT_ASSERT( t2.intersection( t1, true ).isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testExistIntersectionAtTerminalPoints()
{
    // existIntersection at terminal points
    const rcsc::Segment2D t1( rcsc::Vector2D( -200.0, -100.0 ),
                              rcsc::Vector2D(    0.0, +100.0 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D(    0.0, +100.0 ),
                              rcsc::Vector2D( +200.0, -100.0 ) );

    const rcsc::Segment2D t_check( rcsc::Vector2D( 0.0, -300.0 ),
                                   rcsc::Vector2D( 0.0, +900.0 ) );

    CPPUNIT_ASSERT( t1.existIntersection( t_check ) );
    CPPUNIT_ASSERT( t_check.existIntersection( t1 ) );

    CPPUNIT_ASSERT( t1.intersection( t_check, true ).isValid() );
    CPPUNIT_ASSERT( t_check.intersection( t1, true ).isValid() );

    //

    CPPUNIT_ASSERT( t2.existIntersection( t_check ) );
    CPPUNIT_ASSERT( t_check.existIntersection( t2 ) );

    CPPUNIT_ASSERT( t2.intersection( t_check, true ).isValid() );
    CPPUNIT_ASSERT( t_check.intersection( t2, true ).isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testIntersectsAtTerminalPoints()
{
    // intersects at terminal points
    const rcsc::Segment2D t1( rcsc::Vector2D(  200, 100 ),
                              rcsc::Vector2D( 2000, 100 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D(  200, 100 ),
                              rcsc::Vector2D(  200, 500 ) );

    CPPUNIT_ASSERT( t1.existIntersection( t2 ) );
    CPPUNIT_ASSERT( t2.existIntersection( t1 ) );

    CPPUNIT_ASSERT( t1.intersection( t2, true ).isValid() );
    CPPUNIT_ASSERT( t2.intersection( t1, true ).isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testIntersectsAtTerminalPointsParallelHorizontal()
{
    // intersects at terminal points (parallel, horizontal)
    const rcsc::Segment2D t1( rcsc::Vector2D( +200, +100 ),
                              rcsc::Vector2D( +500, +100 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D( +200, +100 ),
                              rcsc::Vector2D( -100, +100 ) );

    CPPUNIT_ASSERT( t1.existIntersection( t2 ) );
    CPPUNIT_ASSERT( t2.existIntersection( t1 ) );

    CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
    CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testIntersectsAtTerminalPointsParallelVertical()
{
    // intersects with terminal points (parallel, vertical)
    const rcsc::Segment2D t1( rcsc::Vector2D( +100, +200 ),
                              rcsc::Vector2D( +100, +500 ) );

    const rcsc::Segment2D t2( rcsc::Vector2D( +100, +200 ),
                              rcsc::Vector2D( +100, -100 ) );

    CPPUNIT_ASSERT( t1.existIntersection( t2 ) );
    CPPUNIT_ASSERT( t2.existIntersection( t1 ) );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( t1.dist( t2 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( t2.dist( t1 ), 0.0, EPS );

    CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
    CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testIntersectWithPointSegment()
{
    // intersect with point segment 1
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( 0,    0 ),
                                  rcsc::Vector2D( 0, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +100, +500 ),
                                  rcsc::Vector2D( +100, +500 ) );

        CPPUNIT_ASSERT( ! t1.existIntersection( t2 ) );
        CPPUNIT_ASSERT( ! t2.existIntersection( t1 ) );

        CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
        CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
    }

    // intersect with point segment 2
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +300, +500 ),
                                  rcsc::Vector2D( +200, +400 ) );


        CPPUNIT_ASSERT( ! t1.existIntersection( t2 ) );
        CPPUNIT_ASSERT( ! t2.existIntersection( t1 ) );

        CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
        CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
    }

    // intersect with point segment 3
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +300, +300 ),
                                  rcsc::Vector2D( +300, +300 ) );


        CPPUNIT_ASSERT( ! t1.existIntersection( t2 ) );
        CPPUNIT_ASSERT( ! t2.existIntersection( t1 ) );

        CPPUNIT_ASSERT( t1.existIntersection( t1 ) );
        CPPUNIT_ASSERT( t2.existIntersection( t2 ) );

        CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
        CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
    }

    // intersect with point segment 4
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D(    0, +500 ),
                                  rcsc::Vector2D( +100, +500 ) );


        CPPUNIT_ASSERT( ! t1.existIntersection( t2 ) );
        CPPUNIT_ASSERT( ! t2.existIntersection( t1 ) );

        CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
        CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
    }

    // intersect with point segment 5
    {
        const rcsc::Segment2D t1( rcsc::Vector2D( +500, +500 ),
                                  rcsc::Vector2D( +500, +500 ) );

        const rcsc::Segment2D t2( rcsc::Vector2D( +500,    0 ),
                                  rcsc::Vector2D( +500, +100 ) );

        CPPUNIT_ASSERT( ! t1.existIntersection( t2 ) );
        CPPUNIT_ASSERT( ! t2.existIntersection( t1 ) );

        CPPUNIT_ASSERT( ! t1.intersection( t2, true ).isValid() );
        CPPUNIT_ASSERT( ! t2.intersection( t1, true ).isValid() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testNearestPoint()
{
    //
    // check nearestPoint()
    //
    const rcsc::Vector2D s1( -500, 100 );
    const rcsc::Vector2D s2( +500, 100 );

    const rcsc::Segment2D s( s1, s2 );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 0.0, 100.0 ).dist( s.nearestPoint( rcsc::Vector2D( 0.0, 0.0 ) ) ),
                                  0.0,
                                  EPS );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 200.0, 100.0 ).dist( s.nearestPoint( rcsc::Vector2D( 200.0, 0.0 ) ) ),
                                  0.0,
                                  EPS );

    for ( long i = 0 ; i < 100000 ; i += 10 )
    {
        const rcsc::Vector2D p( i, +500 );

        rcsc::Vector2D c;

        if ( i <= 500 )
        {
            c = s.nearestPoint( +p );
            CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( (+p).x, 100 ).dist( c ), 0.0, EPS );

            c = s.nearestPoint( -p );
            CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( (-p).x, 100 ).dist( c ), 0.0, EPS );
        }
        else
        {
            c = s.nearestPoint( +p );
            CPPUNIT_ASSERT_DOUBLES_EQUAL( s2.dist( c ), 0.0, EPS );

            c = s.nearestPoint( -p );
            CPPUNIT_ASSERT_DOUBLES_EQUAL( s1.dist( c ), 0.0, EPS );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testDistanceFromPoint()
{
    //
    // check distance of segment and point
    //
    const rcsc::Segment2D seg1( rcsc::Vector2D( -100.0, 0.0 ),
                                rcsc::Vector2D(    0.0, 0.0 ) );
    const rcsc::Segment2D seg2( rcsc::Vector2D(    0.0, 0.0 ),
                                rcsc::Vector2D( -100.0, 0.0 ) );

    const rcsc::Vector2D p( 400.0, 300.0 );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( 500.0 - seg1.dist( p ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 500.0 - seg2.dist( p ), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testDistanceFromPointOnLine()
{
    // distance from point (segment and point are on a line)
    const rcsc::Segment2D seg( rcsc::Vector2D( -100, 0.0 ),
                               rcsc::Vector2D( +100, 0.0 ) );

    const rcsc::Vector2D p( +150.0, 0.0 );

    CPPUNIT_ASSERT_DOUBLES_EQUAL(  50.0 - seg.dist( p ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 250.0 - seg.farthestDist( p ), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testDistanceFromPointComplex()
{
    // distance from point (complex)
    const rcsc::Vector2D s1( -100, 0 );
    const rcsc::Vector2D s2( +100, 0 );

    const rcsc::Segment2D seg( s1, s2 );

    const rcsc::Vector2D p1( 0, +150 );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.0 - seg.dist( +p1 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 150.0 - seg.dist( -p1 ), 0.0, EPS );

    const rcsc::Vector2D p2( 300, 0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 200.0 - seg.dist( +p2 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 200.0 - seg.dist( -p2 ), 0.0, EPS );

    const rcsc::Vector2D p3( 20000, 0 );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 19900.0 - seg.dist( +p3 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 19900.0 - seg.dist( -p3 ), 0.0, EPS );

    for ( long  i = 0  ;  i < 100000  ;  i += 10 )
    {
        const rcsc::Vector2D p( i, +500 );

        if ( i <= 100 )
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL( 500.0 - seg.dist( +p ), 0.0, EPS );

            CPPUNIT_ASSERT_DOUBLES_EQUAL( 500.0 - seg.dist( -p ), 0.0, EPS );
        }
        else
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL( (s2 - p).r() - seg.dist( +p ), 0.0, EPS );

            CPPUNIT_ASSERT_DOUBLES_EQUAL( (s1 - (-p)).r() - seg.dist( -p ), 0.0, EPS );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testDistanceFromSegment()
{
    //
    // distance segment and segment
    //
    const rcsc::Segment2D seg1( rcsc::Vector2D( +100.0, 100.0 ),
                                rcsc::Vector2D( -100.0, 100.0 ) );

    const rcsc::Segment2D seg2( rcsc::Vector2D(    0.0, 300.0 ),
                                rcsc::Vector2D( +100.0, 400.0 ) );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( 200.0 - seg1.dist( seg2 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( 200.0 - seg2.dist( seg1 ), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testOnSegmentStrictly()
{
    {
        rcsc::Segment2D s( rcsc::Vector2D( 0.0, 0.0 ), rcsc::Vector2D( 0.0, 10.0 ) );
        CPPUNIT_ASSERT( s.onSegment( rcsc::Vector2D( 0.0, 5.0 ) ) );
        CPPUNIT_ASSERT( ! s.onSegment( rcsc::Vector2D( 1.0e-7, 0.0 ) ) );
    }

    {
        rcsc::Segment2D s( rcsc::Vector2D( 0.0, 0.0 ),
                           rcsc::Vector2D( 10.0, 10.0 ) );
        CPPUNIT_ASSERT( s.onSegment( rcsc::Vector2D( 5.0, 5.0 ) ) );
        CPPUNIT_ASSERT( ! s.onSegment( rcsc::Vector2D( 6.0, 6.0 + 1.0e-7 ) ) );
    }

    {
        rcsc::Segment2D s( rcsc::Vector2D( 3.148595, 582.2 ),
                           rcsc::Vector2D( -1838.235, 23.21145 ) );
        rcsc::Vector2D dir = s.terminal() - s.origin();
        dir.normalize();
        CPPUNIT_ASSERT( ! s.onSegment( s.origin() + dir * 2.462134 ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testOnSegmentWeakly()
{
    {
        rcsc::Segment2D s( rcsc::Vector2D( 0.0, 0.0 ),
                           rcsc::Vector2D( 0.0, 10.0 ) );
        CPPUNIT_ASSERT( s.onSegmentWeakly( rcsc::Vector2D( 0.0, 5.0 ) ) );
        CPPUNIT_ASSERT( s.onSegmentWeakly( rcsc::Vector2D( 0.0, - 1.0e-7 ) ) );
    }

    {
        rcsc::Segment2D s( rcsc::Vector2D( 0.0, 0.0 ),
                           rcsc::Vector2D( 10.0, 10.0 ) );
        CPPUNIT_ASSERT( s.onSegmentWeakly( rcsc::Vector2D( 5.0, 5.0 ) ) );
        CPPUNIT_ASSERT( s.onSegmentWeakly( rcsc::Vector2D( 6.0, 6.0 + 1.0e-7 ) ) );
    }

    {
        rcsc::Segment2D s( rcsc::Vector2D( 3.148595, 582.2 ),
                           rcsc::Vector2D( -1838.235, 23.21145 ) );
        rcsc::Vector2D dir = s.terminal() - s.origin();
        dir.normalize();
        CPPUNIT_ASSERT( s.onSegmentWeakly( s.origin() + dir * 2.462134 ) );
    }

    {
        const double delta = 1.0e-6 * 0.5;
        const Segment2D segment( Vector2D( 0.0, 0.0 ),
                                 Vector2D( 2.0, 0.0 ) );

        rcsc::Vector2D pos;

        pos.assign( 0.0, 0.0 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 0.0, EPS );

        pos.assign( 1.0, 0.0 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 1.0, EPS );

        pos.assign( 2.0, 0.0 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 2.0, EPS );

        pos.assign( -delta * 0.5, -delta * 0.5 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 0.0, delta );

        pos.assign( 1.0, delta * 0.5 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 1.0, EPS );

        pos.assign( 1.0, -delta * 0.5 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 1.0, EPS );

        pos.assign( 2.0 + delta * 0.5, delta * 0.5 );
        CPPUNIT_ASSERT( segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 2.0, delta );

        pos.assign( -1.0, 0.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 1.0, EPS ); // -1.0

        pos.assign( -1.0, -1.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), std::sqrt( 2.0 ), EPS );

        pos.assign( 0.0, 1.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 1.0, EPS ); // 0.0

        pos.assign( 2.0, -1.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), std::sqrt( 5.0 ), EPS );

        pos.assign( 3.0, 0.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), 3.0, EPS );

        pos.assign( 3.0, 1.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), std::sqrt( 10.0 ), EPS );

        pos.assign( -delta * 2.0, 0.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), delta * 2.0, EPS );

        pos.assign( 0.0, -delta * 2.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( segment.origin().dist( pos ), delta * 2.0, EPS );

        pos.assign( 2.0, delta * 2.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );

        pos.assign( 2.0 + delta * 2.0, 0.0 );
        CPPUNIT_ASSERT( ! segment.onSegmentWeakly( pos ) );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Segment2DTest::testEquals()
{
    //
    const double delta = 1.0e-6;
    const Segment2D segment( Vector2D( 0.0, 0.0 ),
                             Vector2D( 2.0, 0.0 ) );

    Segment2D zero_line( Vector2D( 0.0, 0.0 ), Vector2D( 0.0, 0.0 ) );

    //
    CPPUNIT_ASSERT( ! segment.equals( zero_line ) );
    CPPUNIT_ASSERT( segment.equals( segment ) );
    CPPUNIT_ASSERT( segment.equalsWeakly( Segment2D( Vector2D( 0.0, 0.0 ),
                                                     2.0,
                                                     AngleDeg( 0.0 ) ) ) );

    CPPUNIT_ASSERT( ! segment.equalsWeakly( Segment2D( Vector2D( delta * 2.0, delta * 2.0 ),
                                                       2.0,
                                                       AngleDeg( 0.0 ) ) ) );

    CPPUNIT_ASSERT( ! segment.equalsWeakly( Segment2D( Vector2D( 0.0, 0.0 ),
                                                       2.0 + delta * 2.0,
                                                       AngleDeg( 0.0 ) ) ) );

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
