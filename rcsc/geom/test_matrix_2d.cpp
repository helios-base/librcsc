// -*-c++-*-

/*!
  \file test_matrix_2d.cpp
  \brief test code for rcsc::Matrix2D
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama, Hiroki Shimora

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

#include "matrix_2d.h"

#include <rcsc/math_util.h>

#include <cppunit/extensions/HelperMacros.h>

using rcsc::EPS;


class Matrix2DTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( Matrix2DTest );
    CPPUNIT_TEST( testTranslate );
    CPPUNIT_TEST( testScale );
    CPPUNIT_TEST( testRotate );
    CPPUNIT_TEST( testMultiplication );
    CPPUNIT_TEST_SUITE_END();

public:

    void testTranslate();
    void testScale();
    void testRotate();
    void testMultiplication();
};


CPPUNIT_TEST_SUITE_REGISTRATION( Matrix2DTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
Matrix2DTest::testTranslate()
{
    {
        rcsc::Matrix2D m;
        m.translate( 2.0, 4.0 );

        rcsc::Vector2D v( 0.0, 0.0 );
        m.transform( &v );

        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 2.0, 4.0 ).dist( v ), 0.0, EPS );
    }

    {
        rcsc::Matrix2D m = rcsc::Matrix2D::make_translation( 3.0, 4.0 );

        rcsc::Vector2D v( 0.0, 0.0 );
        m.transform( &v );

        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 3.0, 4.0 ).dist( v ), 0.0, EPS );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Matrix2DTest::testScale()
{
    {
        rcsc::Matrix2D m;
        m.scale( 2.0, 4.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( 1.0, 1.0 );
        rcsc::Vector2D v3( 2.0, 3.0 );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );

        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 2.0, 4.0 ).dist( v2 ), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 4.0, 12.0 ).dist( v3 ), 0.0, EPS );
    }

    {
        rcsc::Matrix2D m = rcsc::Matrix2D::make_scaling( -3.0, 1.0 );

        rcsc::Vector2D v1( 0.0, 0.0 );
        rcsc::Vector2D v2( -1.0, 1.0 );
        rcsc::Vector2D v3( 2.0, -3.0 );

        m.transform( &v1 );
        m.transform( &v2 );
        m.transform( &v3 );

        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 3.0, 1.0 ).dist( v2 ), 0.0, EPS );
        CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( -6.0, -3.0 ).dist( v3 ), 0.0, EPS );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Matrix2DTest::testRotate()
{
    rcsc::AngleDeg angle = 90.0;
    rcsc::Matrix2D m = rcsc::Matrix2D::make_rotation( angle );
    //rcsc::Matrix2D m;
    //m.rotate( 90.0 );

    rcsc::Vector2D v1( 0.0, 0.0 );
    rcsc::Vector2D v2( 1.0, 0.0 );
    rcsc::Vector2D v3( 0.0, 1.0 );
    rcsc::Vector2D v4( 2.0, 3.0 );

    rcsc::Vector2D rv1 = v1.rotatedVector( angle );
    rcsc::Vector2D rv2 = v2.rotatedVector( angle );
    rcsc::Vector2D rv3 = v3.rotatedVector( angle );
    rcsc::Vector2D rv4 = v4.rotatedVector( angle );

    m.transform( &v1 );
    m.transform( &v2 );
    m.transform( &v3 );
    m.transform( &v4 );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 0.0, 0.0 ).dist( v1 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( 0.0, 1.0 ).dist( v2 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( -1.0, 0.0 ).dist( v3 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rcsc::Vector2D( -3.0, 2.0 ).dist( v4 ), 0.0, EPS );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( rv1.dist( v1 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rv2.dist( v2 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rv3.dist( v3 ), 0.0, EPS );
    CPPUNIT_ASSERT_DOUBLES_EQUAL( rv4.dist( v4 ), 0.0, EPS );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Matrix2DTest::testMultiplication()
{
    rcsc::Vector2D v( 3.5821, -292.23 );
    rcsc::Vector2D scale( 5.62, 92.092 );
    rcsc::Vector2D translate( 3.2, 5.4 );
    rcsc::AngleDeg rotate = 14.0;

    rcsc::Matrix2D m1;
    m1.rotate( rotate );
    m1.translate( translate.x, translate.y );
    m1.scale( scale.x, scale.y );

    rcsc::Matrix2D m2
        = rcsc::Matrix2D::make_scaling( scale.x, scale.y )
        * rcsc::Matrix2D::make_translation( translate.x, translate.y )
        * rcsc::Matrix2D::make_rotation( rotate );

    CPPUNIT_ASSERT_DOUBLES_EQUAL( m1.transform( v ).dist( m2.transform( v ) ), 0.0, EPS );
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
