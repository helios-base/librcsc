// -*-c++-*-

/*!
  \file test_convex_hull.cpp
  \brief test code for rcsc::ConvexHull
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

#include "convex_hull.h"

#include <rcsc/time/timer.h>

#include <cppunit/extensions/HelperMacros.h>

#define DEBUG_PRINT

class ConvexHullTest
    : public CPPUNIT_NS::TestFixture {

    CPPUNIT_TEST_SUITE( ConvexHullTest );
    CPPUNIT_TEST( testEmpty );
    CPPUNIT_TEST( testPoints );
    CPPUNIT_TEST( testCircle );
    CPPUNIT_TEST_SUITE_END();

public:

    void testEmpty();
    void testPoints();
    void testCircle();
};


CPPUNIT_TEST_SUITE_REGISTRATION( ConvexHullTest );


/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHullTest::testEmpty()
{
    rcsc::ConvexHull c;

    CPPUNIT_ASSERT_EQUAL( static_cast< size_t >( 0 ), c.vertices().size() );
    CPPUNIT_ASSERT_EQUAL( static_cast< size_t >( 0 ), c.edges().size() );

    c.compute();

    CPPUNIT_ASSERT_EQUAL( static_cast< size_t >( 0 ), c.vertices().size() );
    CPPUNIT_ASSERT_EQUAL( static_cast< size_t >( 0 ), c.edges().size() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHullTest::testPoints()
{
    const rcsc::Vector2D p0(   0.0,   0.0 );
    const rcsc::Vector2D p1( +10.0, +10.0 );
    const rcsc::Vector2D p2( -10.0, +10.0 );
    const rcsc::Vector2D p3( -10.0, -10.0 );
    const rcsc::Vector2D p4( +10.0, -10.0 );

    const rcsc::Vector2D p5( +20.0,   0.0 );
    const rcsc::Vector2D p6(   0.0, +20.0 );
    const rcsc::Vector2D p7( -20.0,   0.0 );
    const rcsc::Vector2D p8(   0.0, -20.0 );

    //
    // input points
    //

    //                               //
    //                 |             //
    // +20             *p6           //
    //                 |             //
    //                 |             //
    //           p2    |    p1       //
    // +10        *    |    *        //
    //                 |             //
    //                 |             //
    //                 |             //
    //      p7         |             //
    //   0 --*---------*---------*-- //
    //                 |p0       p5  //
    //                 |             //
    //                 |             //
    //                 |             //
    // -10        *    |    *        //
    //           p3    |    p4       //
    //                 |             //
    //                 |             //
    //                 |             //
    // -20             *p8           //
    //                 |             //
    //                               //
    //      -20  -10   0   +10  +20  //


    rcsc::ConvexHull c;

    c.addPoint( p0 );
    c.addPoint( p1 );
    c.addPoint( p2 );
    c.addPoint( p3 );
    c.addPoint( p4 );
    c.addPoint( p5 );
    c.addPoint( p6 );
    c.addPoint( p7 );
    c.addPoint( p8 );

    c.compute();


    //
    // result
    //


    //                               //
    //                 |             //
    // +20             *p6           //
    //                /|\            //
    //              /  |  \          //
    //           p2    |    p1       //
    // +10        *    |    *        //
    //          /      |     \       //
    //         /       |      \      //
    //        /        |       \     //
    //      p7         |        \    //
    //   0 --*---------*---------*-- //
    //       \         |p0       p5  //
    //        \        |        /    //
    //         \       |       /     //
    //          \      |      /      //
    // -10        *    |    *        //
    //           p3    |    p4       //
    //             \   |  /          //
    //              \  | /           //
    //               \ |/            //
    // -20             *p8           //
    //                 |             //
    //                               //
    //      -20  -10   0   +10  +20  //

    //
    // check vertices
    //
#ifdef DEBUG_PRINT
    std::cout << '\n';
#endif
    int n_vertices = 0;
    for ( rcsc::ConvexHull::VertexCont::const_iterator v = c.vertices().begin();
          v != c.vertices().end();
          ++v )
    {
        ++n_vertices;

#ifdef DEBUG_PRINT
        std::cout << "vertex " << n_vertices << ": " << *v << std::endl;
#endif
    }
    CPPUNIT_ASSERT_EQUAL( 8, n_vertices );

    //
    // check edges
    //
    int n_edges = 0;
    for ( rcsc::ConvexHull::EdgeCont::const_iterator e = c.edges().begin();
          e != c.edges().end();
          ++e )
    {
        ++n_edges;

#ifdef DEBUG_PRINT
        std::cout << "edge " << n_edges << ": "
                  << e->origin() << ", " << e->terminal()
                  << std::endl;
#endif
    }
    CPPUNIT_ASSERT_EQUAL( 8, n_edges );

    //c.printInputPoints( std::cout );
    //c.printEdges( std::cout );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ConvexHullTest::testCircle()
{
    const int max_loop = 1000;

    rcsc::ConvexHull c;

    for ( int i = 0; i < max_loop; ++i )
    {
        rcsc::Vector2D p = rcsc::Vector2D::from_polar( 10.0, (360.0/max_loop)*i );
        c.addPoint( p );
    }

    int n_vertices = 0;
    int n_edges = 0;

    {
        rcsc::Timer timer;
        c.compute( rcsc::ConvexHull::WrappingMethod );
        std::cout << "\nWrappingMethod elapsed " << timer.elapsedReal() << " [ms]" << std::endl;;
    }

    n_vertices = c.vertices().size();
    n_edges = c.edges().size();
    CPPUNIT_ASSERT_EQUAL( 1000, n_vertices );
    CPPUNIT_ASSERT_EQUAL( 1000, n_edges );

    c.clearResults();

    {
        rcsc::Timer timer;
        c.compute( rcsc::ConvexHull::GrahamScan );
        std::cout << "\nGrahamScan elapsed " << timer.elapsedReal() << " [ms]" << std::endl;;
    }

    n_vertices = c.vertices().size();
    n_edges = c.edges().size();
    CPPUNIT_ASSERT_EQUAL( 1000, n_vertices );
    CPPUNIT_ASSERT_EQUAL( 1000, n_edges );
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
