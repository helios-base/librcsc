// -*-c++-*-

/*!
  \file test_vector_2d.cpp
  \brief test code for rcsc::Vector2D
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

#include "vector_2d.h"

#include <cppunit/extensions/HelperMacros.h>

#include <iostream>
#include <cmath>

using rcsc::Vector2D;
using rcsc::AngleDeg;

namespace {

//const double DISTANCE = 1.0e-9;
const double DISTANCE = 1.0e-6;
const Vector2D ZERO( 0.0, 0.0 );

inline
bool
in_distance( const double & x,
             const double & y )
{
    return std::fabs( x - y ) < DISTANCE;
}


inline
bool
in_distance2( const double & x,
              const double & y )
{
    return std::fabs( x - y ) < DISTANCE * DISTANCE;
}

}


/*!
  \class Vector2DTest
 */
class Vector2DTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( Vector2DTest );
    CPPUNIT_TEST( testAssign );
    CPPUNIT_TEST( testDistance );
    CPPUNIT_TEST( testEquals );
    CPPUNIT_TEST( testRotate );
    CPPUNIT_TEST_SUITE_END();

public:

    void setUp();
    void tearDown();

protected:

    void testAssign();
    void testDistance();
    void testEquals();
    void testRotate();
};



CPPUNIT_TEST_SUITE_REGISTRATION( Vector2DTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
Vector2DTest::setUp()
{
    //std::cerr << "Vector2DTest setUp" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Vector2DTest::tearDown()
{
    //std::cerr << "Vector2DTest tearDown" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Vector2DTest::testAssign()
{
    //
    const Vector2D p0;
    CPPUNIT_ASSERT( in_distance( p0.x, 0.0 ) );
    CPPUNIT_ASSERT( in_distance( p0.y, 0.0 ) );

    //
    const Vector2D p1( 1.0, -2.0 );
    CPPUNIT_ASSERT( in_distance( p1.x, 1.0 ) );
    CPPUNIT_ASSERT( in_distance( p1.y, -2.0 ) );

    //
    const Vector2D p2( -3.5, 4.5 );

    //
    const Vector2D p3 = p2;
    CPPUNIT_ASSERT( in_distance( p3.x, -3.5 ) );
    CPPUNIT_ASSERT( in_distance( p3.y, 4.5 ) );

    //
    Vector2D p4;
    p4 = p2;
    CPPUNIT_ASSERT( in_distance( p4.x, -3.5 ) );
    CPPUNIT_ASSERT( in_distance( p4.y, 4.5 ) );
}

void
Vector2DTest::testDistance()
{
    //
    const Vector2D p0;
    //CPPUNIT_ASSERT( in_distance( p0.dist( Vector2D::ORIGIN ), 0.0 ) );
    CPPUNIT_ASSERT( in_distance( p0.dist( ZERO ), 0.0 ) );
    CPPUNIT_ASSERT( in_distance( p0.dist( Vector2D() ), 0.0 ) );
    //CPPUNIT_ASSERT( in_distance2( p0.dist2( Vector2D::ORIGIN ), 0.0 ) );
    CPPUNIT_ASSERT( in_distance2( p0.dist2( ZERO ), 0.0 ) );
    CPPUNIT_ASSERT( in_distance2( p0.dist2( Vector2D() ), 0.0 ) );

    //
    const Vector2D p1( 1.0, -2.0 );
    //CPPUNIT_ASSERT( in_distance( p1.dist( Vector2D::ORIGIN ), std::sqrt( 5.0 ) ) );
    CPPUNIT_ASSERT( in_distance( p1.dist( ZERO ), std::sqrt( 5.0 ) ) );
    //CPPUNIT_ASSERT( in_distance2( p1.dist2( Vector2D::ORIGIN ), 5.0 ) );
    CPPUNIT_ASSERT( in_distance2( p1.dist2( ZERO ), 5.0 ) );
    //
    const Vector2D p2( 4.0, 2.0 );
    CPPUNIT_ASSERT( in_distance( p2.dist( p1 ), 5.0 ) );
    CPPUNIT_ASSERT( in_distance( p2.dist2( p1 ), 25.0 ) );
}

void
Vector2DTest::testEquals()
{
    //
    const Vector2D p0;
    //CPPUNIT_ASSERT( p0 == Vector2D::ORIGIN );
    CPPUNIT_ASSERT( p0 == ZERO );
    CPPUNIT_ASSERT( p0 == Vector2D() );
    CPPUNIT_ASSERT( p0 != Vector2D( DISTANCE * 2.0, DISTANCE * 2.0 ) );

    //
    const Vector2D p1( 1.0, -2.0 );
    //CPPUNIT_ASSERT( p1 != Vector2D::ORIGIN );
    CPPUNIT_ASSERT( p1 != ZERO );
    CPPUNIT_ASSERT( p1 == p1 );
    CPPUNIT_ASSERT( p1 == Vector2D( 1.0, -2.0 ) );
    CPPUNIT_ASSERT( p1 != Vector2D( 1.0 + DISTANCE * 2.0, -2.0 + DISTANCE * 2.0 ) );
}


void
Vector2DTest::testRotate()
{
    const Vector2D v( 1.0, 1.0 );
    const AngleDeg rot = -30.0;

    std::cerr << '\n';

    Vector2D v1 = v.rotatedVector( rot );
    std::cerr << "v1=" << v1 << " th=" << v1.th() << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( ( v.th() + rot ).degree(), v1.th().degree(), 1.0e-5 );

    Vector2D v2( v.x * rot.cos() - v.y * rot.sin(),
                 v.x * rot.sin() + v.y * rot.cos() );
    std::cerr << "v2=" << v2 << " th=" << v2.th() << std::endl;
    CPPUNIT_ASSERT_DOUBLES_EQUAL( ( v.th() + rot ).degree(), v2.th().degree(), 1.0e-5 );
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
