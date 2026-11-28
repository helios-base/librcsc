// -*-c++-*-

/*!
  \file test_formation.cpp
  \brief test code for rcsc::FormationData, rcsc::Formation and rcsc::FormationParser
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

#include "formation_data.h"
#include "formation.h"
#include "formation_parser.h"
#include "formation_static.h"

#include <cppunit/extensions/HelperMacros.h>

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// See https://github.com/helios-base/librcsc/issues/124

#ifndef TEST_DATA_DIR
#error "TEST_DATA_DIR is not defined"
#endif

using rcsc::FormationData;
using rcsc::Formation;
using rcsc::FormationParser;
using rcsc::Vector2D;

namespace {

typedef std::array< int, 11 > Pairs;

/*!
  \brief discard std::cerr while this object is alive (for the expected errors)
*/
class CerrSilencer {
private:
    std::ostringstream M_buf;
    std::streambuf * M_old;

public:
    CerrSilencer()
        : M_buf(),
          M_old( std::cerr.rdbuf( M_buf.rdbuf() ) )
      { }

    ~CerrSilencer()
      {
          std::cerr.rdbuf( M_old );
      }
};

/*!
  \brief create a data whose ball is at (x, y), and the players are placed relative to the ball
*/
FormationData::Data
make_data( const double x,
           const double y )
{
    std::vector< Vector2D > players;
    for ( int i = 1; i <= 11; ++i )
    {
        players.emplace_back( x - 3.0 * i, y + i );
    }
    return FormationData::Data( Vector2D( x, y ), players );
}

/*!
  \brief create the data with the balls at (0, 0), (10, 0), ..., (10 * (n - 1), 0)
*/
void
add_data( FormationData & formation_data,
          const int n )
{
    for ( int i = 0; i < n; ++i )
    {
        CPPUNIT_ASSERT_EQUAL( std::string(), formation_data.addData( make_data( 10.0 * i, 0.0 ) ) );
    }
}

/*!
  \brief the x of the balls in the order of the data
*/
std::vector< double >
ball_xs( const FormationData & formation_data )
{
    std::vector< double > xs;
    for ( const FormationData::Data & d : formation_data.dataCont() )
    {
        xs.push_back( d.ball_.x );
    }
    return xs;
}

/*!
  \brief the index values of the data
*/
std::vector< int >
indexes( const FormationData & formation_data )
{
    std::vector< int > v;
    for ( const FormationData::Data & d : formation_data.dataCont() )
    {
        v.push_back( d.index_ );
    }
    return v;
}

/*!
  \brief check that two data sets are the same (balls, players and index values)
*/
void
assert_same_data( const FormationData & expected,
                  const FormationData & actual )
{
    CPPUNIT_ASSERT_EQUAL( expected.dataCont().size(), actual.dataCont().size() );

    FormationData::DataCont::const_iterator e = expected.dataCont().begin();
    for ( const FormationData::Data & a : actual.dataCont() )
    {
        CPPUNIT_ASSERT_EQUAL( e->index_, a.index_ );
        CPPUNIT_ASSERT( e->ball_.equals( a.ball_ ) );
        CPPUNIT_ASSERT_EQUAL( e->players_.size(), a.players_.size() );
        for ( size_t i = 0; i < a.players_.size(); ++i )
        {
            CPPUNIT_ASSERT( e->players_[i].equals( a.players_[i] ) );
        }
        ++e;
    }
}

std::string
to_string( const Pairs & pairs )
{
    std::ostringstream os;
    for ( const int p : pairs )
    {
        os << ' ' << p;
    }
    return os.str();
}

/*!
  \brief check the position pairs. compared as strings to show the values on failure
*/
void
assert_pairs( const Pairs & expected,
              const Pairs & actual )
{
    CPPUNIT_ASSERT_EQUAL( to_string( expected ), to_string( actual ) );
}

Formation::Ptr
create_formation()
{
    Formation::Ptr f = Formation::create( rcsc::FormationStatic::NAME );
    CPPUNIT_ASSERT( f );
    return f;
}

std::string
test_data_path( const std::string & name )
{
    return std::string( TEST_DATA_DIR ) + "/" + name;
}

}

/*-------------------------------------------------------------------*/
/*!
  \brief copy of FormationData
*/
class FormationDataCopyTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( FormationDataCopyTest );
    CPPUNIT_TEST( testCopyConstructor );
    CPPUNIT_TEST( testCopyAssignment );
    CPPUNIT_TEST( testCopyEmpty );
    CPPUNIT_TEST_SUITE_END();

public:

    // the copy has the same data, and is independent of the original
    void testCopyConstructor()
      {
          FormationData original;
          add_data( original, 3 );

          FormationData copied( original );
          assert_same_data( original, copied );

          CPPUNIT_ASSERT_EQUAL( std::string(), copied.removeData( 0 ) );
          CPPUNIT_ASSERT_EQUAL( size_t( 2 ), copied.dataCont().size() );
          CPPUNIT_ASSERT_EQUAL( size_t( 3 ), original.dataCont().size() );
          CPPUNIT_ASSERT( original.data( 0 )->ball_.equals( Vector2D( 0.0, 0.0 ) ) );

          {
              CerrSilencer silencer; // replaceData() prints a message
              CPPUNIT_ASSERT_EQUAL( std::string(), original.replaceData( 1, make_data( 15.0, 5.0 ) ) );
          }
          CPPUNIT_ASSERT( copied.data( 0 )->ball_.equals( Vector2D( 10.0, 0.0 ) ) );
      }

    // the assigned data replace the old data, and are independent of the original
    void testCopyAssignment()
      {
          FormationData original;
          add_data( original, 3 );

          FormationData assigned;
          CPPUNIT_ASSERT_EQUAL( std::string(), assigned.addData( make_data( -30.0, 20.0 ) ) );

          assigned = original;
          assert_same_data( original, assigned );

          CPPUNIT_ASSERT_EQUAL( std::string(), original.addData( make_data( 40.0, 0.0 ) ) );
          CPPUNIT_ASSERT_EQUAL( size_t( 4 ), original.dataCont().size() );
          CPPUNIT_ASSERT_EQUAL( size_t( 3 ), assigned.dataCont().size() );

          // assign the old state back, as an undo would do
          FormationData snapshot( assigned );
          CPPUNIT_ASSERT_EQUAL( std::string(), assigned.removeData( 2 ) );
          assigned = snapshot;
          assert_same_data( snapshot, assigned );
          CPPUNIT_ASSERT( indexes( assigned ) == std::vector< int >( { 0, 1, 2 } ) );
      }

    void testCopyEmpty()
      {
          FormationData original;
          add_data( original, 2 );

          const FormationData empty;
          original = empty;
          CPPUNIT_ASSERT( original.dataCont().empty() );
          CPPUNIT_ASSERT( original.data( 0 ) == nullptr );
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION( FormationDataCopyTest );

/*-------------------------------------------------------------------*/
/*!
  \brief the range of the index of FormationData
*/
class FormationDataIndexTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( FormationDataIndexTest );
    CPPUNIT_TEST( testData );
    CPPUNIT_TEST( testReplaceData );
    CPPUNIT_TEST( testRemoveData );
    CPPUNIT_TEST( testChangeDataIndex );
    CPPUNIT_TEST_SUITE_END();

public:

    void testData()
      {
          FormationData formation_data;
          CPPUNIT_ASSERT( formation_data.data( 0 ) == nullptr );

          add_data( formation_data, 3 );
          CPPUNIT_ASSERT( formation_data.data( 0 ) != nullptr );
          CPPUNIT_ASSERT( formation_data.data( 2 ) != nullptr );
          CPPUNIT_ASSERT( formation_data.data( 2 )->ball_.equals( Vector2D( 20.0, 0.0 ) ) );
          CPPUNIT_ASSERT( formation_data.data( 3 ) == nullptr ); // == size()
          CPPUNIT_ASSERT( formation_data.data( 4 ) == nullptr );
      }

    void testReplaceData()
      {
          FormationData formation_data;
          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ),
                                formation_data.replaceData( 0, make_data( 0.0, 0.0 ) ) );
          CPPUNIT_ASSERT( formation_data.dataCont().empty() );

          add_data( formation_data, 3 );
          const FormationData before( formation_data );

          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ),
                                formation_data.replaceData( 3, make_data( 50.0, 0.0 ) ) ); // == size()
          assert_same_data( before, formation_data );

          {
              CerrSilencer silencer; // replaceData() prints a message
              CPPUNIT_ASSERT_EQUAL( std::string(), formation_data.replaceData( 2, make_data( 50.0, 0.0 ) ) );
          }
          CPPUNIT_ASSERT_EQUAL( size_t( 3 ), formation_data.dataCont().size() );
          CPPUNIT_ASSERT( formation_data.data( 2 )->ball_.equals( Vector2D( 50.0, 0.0 ) ) );
      }

    void testRemoveData()
      {
          FormationData formation_data;
          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ), formation_data.removeData( 0 ) );

          add_data( formation_data, 3 );
          const FormationData before( formation_data );

          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ), formation_data.removeData( 3 ) ); // == size()
          assert_same_data( before, formation_data );

          CPPUNIT_ASSERT_EQUAL( std::string(), formation_data.removeData( 2 ) );
          CPPUNIT_ASSERT_EQUAL( size_t( 2 ), formation_data.dataCont().size() );
      }

    void testChangeDataIndex()
      {
          FormationData formation_data;
          add_data( formation_data, 3 ); // 0, 10, 20
          const FormationData before( formation_data );

          // the old index must be less than size()
          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ), formation_data.changeDataIndex( 3, 0 ) );
          assert_same_data( before, formation_data );

          // the new index can be size() (moved to the end), but not more
          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ), formation_data.changeDataIndex( 0, 4 ) );
          assert_same_data( before, formation_data );

          CPPUNIT_ASSERT_EQUAL( std::string( "Invalid index" ), formation_data.changeDataIndex( 1, 1 ) );
          assert_same_data( before, formation_data );

          CPPUNIT_ASSERT_EQUAL( std::string(), formation_data.changeDataIndex( 0, 3 ) );
          CPPUNIT_ASSERT( ball_xs( formation_data ) == std::vector< double >( { 10.0, 20.0, 0.0 } ) );

          CPPUNIT_ASSERT_EQUAL( std::string(), formation_data.changeDataIndex( 2, 0 ) );
          CPPUNIT_ASSERT( ball_xs( formation_data ) == std::vector< double >( { 0.0, 10.0, 20.0 } ) );
          CPPUNIT_ASSERT( indexes( formation_data ) == std::vector< int >( { 0, 1, 2 } ) );
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION( FormationDataIndexTest );

/*-------------------------------------------------------------------*/
/*!
  \brief Formation::setPositionPairs()
*/
class FormationPositionPairsTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( FormationPositionPairsTest );
    CPPUNIT_TEST( testSet );
    CPPUNIT_TEST( testSwap );
    CPPUNIT_TEST( testMinusOne );
    CPPUNIT_TEST( testIllegal );
    CPPUNIT_TEST_SUITE_END();

public:

    void testSet()
      {
          Formation::Ptr f = create_formation();
          assert_pairs( { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, f->positionPairs() );

          const Pairs pairs = { 0, 3, 2, 5, 4, 0, 8, 7, 10, 9, 0 };
          CPPUNIT_ASSERT( f->setPositionPairs( pairs ) );
          assert_pairs( pairs, f->positionPairs() );
          CPPUNIT_ASSERT_EQUAL( 3, f->pairedNumber( 2 ) );
          CPPUNIT_ASSERT_EQUAL( 2, f->pairedNumber( 3 ) );

          const Pairs none = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
          CPPUNIT_ASSERT( f->setPositionPairs( none ) );
          assert_pairs( none, f->positionPairs() );
      }

    // 2-3 and 4-5 to 2-4 and 3-5, which setPositionPair() cannot do one by one
    void testSwap()
      {
          Formation::Ptr f = create_formation();
          const Pairs before = { 0, 3, 2, 5, 4, 0, 8, 7, 10, 9, 0 };
          CPPUNIT_ASSERT( f->setPositionPairs( before ) );

          {
              CerrSilencer silencer;
              CPPUNIT_ASSERT( ! f->setPositionPair( 2, 4 ) ); // 4 is already paired with 5
          }
          assert_pairs( before, f->positionPairs() );

          const Pairs after = { 0, 4, 5, 2, 3, 0, 8, 7, 10, 9, 0 };
          CPPUNIT_ASSERT( f->setPositionPairs( after ) );
          assert_pairs( after, f->positionPairs() );

          // and back
          CPPUNIT_ASSERT( f->setPositionPairs( before ) );
          assert_pairs( before, f->positionPairs() );
      }

    // -1 is accepted if no player refers to it
    void testMinusOne()
      {
          Formation::Ptr f = create_formation();

          const Pairs all = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
          CPPUNIT_ASSERT( f->setPositionPairs( all ) );
          assert_pairs( all, f->positionPairs() );

          const Pairs some = { 0, 3, 2, 5, 4, -1, 8, 7, 10, 9, -1 };
          CPPUNIT_ASSERT( f->setPositionPairs( some ) );
          assert_pairs( some, f->positionPairs() );
      }

    // illegal values change nothing
    void testIllegal()
      {
          Formation::Ptr f = create_formation();
          const Pairs before = { 0, 3, 2, 5, 4, 0, 8, 7, 10, 9, 0 };
          CPPUNIT_ASSERT( f->setPositionPairs( before ) );

          const std::vector< Pairs > illegal = {
              { 0, 3, 2, 5, 4, 0, 8, 7, 10, 9, 12 }, // out of range
              { 0, 3, 2, 5, 4, 0, 8, 7, 10, 9, -2 }, // out of range
              { 0, 2, 0, 5, 4, 0, 8, 7, 10, 9, 0 }, // paired with itself
              { 0, 3, 0, 5, 4, 0, 8, 7, 10, 9, 0 }, // one way (3 refers to no player)
              { 0, 3, -1, 5, 4, 0, 8, 7, 10, 9, 0 }, // one way (3 is -1)
              { 0, 3, 4, 3, 0, 0, 8, 7, 10, 9, 0 }, // 2 -> 3, but 3 -> 4
              { 0, 4, 4, 2, 0, 0, 8, 7, 10, 9, 0 }, // 2 and 3 refer to 4
          };

          for ( const Pairs & pairs : illegal )
          {
              CerrSilencer silencer;
              CPPUNIT_ASSERT_MESSAGE( to_string( pairs ), ! f->setPositionPairs( pairs ) );
              assert_pairs( before, f->positionPairs() );
          }
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION( FormationPositionPairsTest );

/*-------------------------------------------------------------------*/
/*!
  \brief the position pairs read by FormationParser
*/
class FormationParserPairsTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( FormationParserPairsTest );
    CPPUNIT_TEST( testReversedJSON );
    CPPUNIT_TEST( testReversedV3 );
    CPPUNIT_TEST( testOneWayJSON );
    CPPUNIT_TEST_SUITE_END();

private:

    // 2 -> 3 and 7 -> 8 refer to the players read later (3 and 8 are -1).
    // 4 -> -1 and 9 -> -1 are referred to by the players read later (5 and 10).
    // 11 -> -1 is not referred to by any player.
    static
    Pairs expected_reversed()
      {
          return { 0, 3, 2, 5, 4, 0, 8, 7, 10, 9, -1 };
      }

    static
    Formation::Ptr parse( const std::string & name )
      {
          CerrSilencer silencer;
          return FormationParser::parse( test_data_path( name ) );
      }

public:

    // the paired players refer to each other, regardless of the order in the file
    void testReversedJSON()
      {
          Formation::Ptr f = parse( "pairs_reversed.conf" );
          CPPUNIT_ASSERT( f );
          assert_pairs( expected_reversed(), f->positionPairs() );

          // the sides in the file are kept
          CPPUNIT_ASSERT_EQUAL( rcsc::RoleType::Right, f->roleType( 2 ).side() );
          CPPUNIT_ASSERT_EQUAL( rcsc::RoleType::Left, f->roleType( 3 ).side() );
      }

    void testReversedV3()
      {
          Formation::Ptr f = parse( "pairs_reversed_v3.conf" );
          CPPUNIT_ASSERT( f );
          assert_pairs( expected_reversed(), f->positionPairs() );

          // the sides are made from the values in the file
          CPPUNIT_ASSERT_EQUAL( rcsc::RoleType::Right, f->roleType( 2 ).side() );
          CPPUNIT_ASSERT_EQUAL( rcsc::RoleType::Left, f->roleType( 3 ).side() );
      }

    // 2 -> 3, but 3 -> 0. The JSON parser did not check the pairs before.
    void testOneWayJSON()
      {
          Formation::Ptr f = parse( "pairs_one_way.conf" );
          CPPUNIT_ASSERT( ! f );
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION( FormationParserPairsTest );

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
