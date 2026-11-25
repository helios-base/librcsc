// -*-c++-*-

/*!
  \file test_gzread_file.cpp
  \brief test code for rcsc::gzread_file()
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

#include "gzread_file.h"
#include "gzfstream.h"

#include <cppunit/extensions/HelperMacros.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>

// See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.3 and 6.

namespace {

/*!
  \brief a temporary file, removed by the destructor
*/
class TempFile {
private:
    std::string M_path;
public:
    explicit
    TempFile( const std::string & name )
        : M_path( ( std::filesystem::temp_directory_path()
                    / ( "rcsc_test_gzread_file_" + std::to_string( ::getpid() ) + "_" + name ) ).string() )
      { }

    ~TempFile()
      {
          std::error_code ec;
          std::filesystem::remove( M_path, ec );
      }

    const std::string & path() const
      {
          return M_path;
      }
};

void
write_raw( const std::string & path,
           const std::string & data )
{
    std::ofstream fout( path, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc );
    fout.write( data.data(), data.size() );
}

/*!
  \brief text of about size bytes, like the lines of a game log
*/
std::string
make_text( const size_t size,
           const int seed )
{
    std::string text;
    text.reserve( size + 100 );
    for ( int i = 0; text.size() < size; ++i )
    {
        text += "(show " + std::to_string( i ) + " ((b) " + std::to_string( ( i * 7919 + seed ) % 10007 )
            + " " + std::to_string( ( i * 104729 + seed ) % 1009 ) + "))\n";
    }
    return text;
}

#ifdef HAVE_LIBZ
std::string
read_raw( const std::string & path )
{
    std::ifstream fin( path, std::ios_base::in | std::ios_base::binary );
    std::ostringstream os;
    os << fin.rdbuf();
    return os.str();
}

/*!
  \brief compress data into gzip bytes (one member) by rcsc::gzofstream
*/
std::string
gzip( const std::string & data,
      const std::string & name )
{
    TempFile file( name + ".gz" );
    {
        rcsc::gzofstream fout( file.path().c_str() );
        fout.write( data.data(), data.size() );
        fout.close();
    }
    return read_raw( file.path() );
}
#endif

}

/*-------------------------------------------------------------------*/
class GZReadFileTest
    : public CppUnit::TestFixture {

    CPPUNIT_TEST_SUITE( GZReadFileTest );
    CPPUNIT_TEST( testPlainFile );
    CPPUNIT_TEST( testEmptyFile );
    CPPUNIT_TEST( testMissingFile );
#ifdef HAVE_LIBZ
    CPPUNIT_TEST( testGzipLargerThanChunk );
    CPPUNIT_TEST( testGzipMembers );
    CPPUNIT_TEST( testGzipTruncated );
    CPPUNIT_TEST( testGzipTrailingBytes );
#endif
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override { }
    void tearDown() override { }

protected:
    void testPlainFile();
    void testEmptyFile();
    void testMissingFile();
#ifdef HAVE_LIBZ
    void testGzipLargerThanChunk();
    void testGzipMembers();
    void testGzipTruncated();
    void testGzipTrailingBytes();
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION( GZReadFileTest );

/*-------------------------------------------------------------------*/
/*!
  \brief a file that is not gzip is read as it is (including "\r\n" and '\0')
*/
void
GZReadFileTest::testPlainFile()
{
    TempFile file( "plain.txt" );
    std::string text = "ULG5\r\n(show 1)\n";
    text += '\0';
    text += make_text( 3 * 1024 * 1024, 1 );
    write_raw( file.path(), text );

    std::string data = "old contents";
    CPPUNIT_ASSERT( rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT_EQUAL( text.size(), data.size() );
    CPPUNIT_ASSERT( text == data );
}

/*-------------------------------------------------------------------*/
void
GZReadFileTest::testEmptyFile()
{
    TempFile file( "empty.txt" );
    write_raw( file.path(), std::string() );

    std::string data = "old contents";
    CPPUNIT_ASSERT( rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT( data.empty() );
}

/*-------------------------------------------------------------------*/
void
GZReadFileTest::testMissingFile()
{
    TempFile file( "missing.rcg" );

    std::string data;
    CPPUNIT_ASSERT( ! rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT( ! rcsc::gzread_file( file.path(), nullptr ) );
}

#ifdef HAVE_LIBZ

/*-------------------------------------------------------------------*/
/*!
  \brief a gzip file larger than the chunk of reading (1 MB)
*/
void
GZReadFileTest::testGzipLargerThanChunk()
{
    const std::string text = make_text( 5 * 1024 * 1024 + 123, 2 );
    TempFile file( "large.rcg.gz" );
    write_raw( file.path(), gzip( text, "large" ) );

    std::string data;
    CPPUNIT_ASSERT( rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT_EQUAL( text.size(), data.size() );
    CPPUNIT_ASSERT( text == data );
}

/*-------------------------------------------------------------------*/
/*!
  \brief concatenated gzip members are read to the end. The small last member makes the size hint
  (the last 4 bytes) too small, so the buffer must grow.
*/
void
GZReadFileTest::testGzipMembers()
{
    const std::string text1 = make_text( 2 * 1024 * 1024, 3 );
    const std::string text2 = "(msg 6000 1 \"(result)\")\n";
    TempFile file( "members.rcg.gz" );
    write_raw( file.path(), gzip( text1, "member1" ) + gzip( text2, "member2" ) );

    std::string data;
    CPPUNIT_ASSERT( rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT_EQUAL( text1.size() + text2.size(), data.size() );
    CPPUNIT_ASSERT( text1 + text2 == data );
}

/*-------------------------------------------------------------------*/
/*!
  \brief a gzip file that ends in the middle is read up to there
*/
void
GZReadFileTest::testGzipTruncated()
{
    const std::string text = make_text( 2 * 1024 * 1024, 4 );
    const std::string compressed = gzip( text, "full" );
    TempFile file( "truncated.rcg.gz" );
    write_raw( file.path(), compressed.substr( 0, compressed.size() / 2 ) );

    std::string data;
    CPPUNIT_ASSERT( rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT( ! data.empty() );
    CPPUNIT_ASSERT( data.size() < text.size() );
    CPPUNIT_ASSERT( text.compare( 0, data.size(), data ) == 0 );
}

/*-------------------------------------------------------------------*/
/*!
  \brief the bytes that are not gzip after a member are ignored, as zlib does
*/
void
GZReadFileTest::testGzipTrailingBytes()
{
    const std::string text = make_text( 100 * 1024, 5 );
    TempFile file( "trailing.rcg.gz" );
    write_raw( file.path(), gzip( text, "trailing" ) + "not gzip\n" );

    std::string data;
    CPPUNIT_ASSERT( rcsc::gzread_file( file.path(), &data ) );
    CPPUNIT_ASSERT_EQUAL( text.size(), data.size() );
    CPPUNIT_ASSERT( text == data );
}

#endif

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
