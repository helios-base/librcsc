// -*-c++-*-

/*!
  \file rcg_parser_bench.cpp
  \brief measure the time to load game logs by the rcg parser (a tool for the development, not installed)
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

/*
  Usage: rcg_parser_bench [-n <Repeat>] [-t <Threads>] <RcgFile>[.gz]...

  For each game log, the following times are measured <Repeat> times (default 5),
  and the best and the median are printed in milliseconds.

    stream : open the file by rcsc::gzifstream, and call rcsc::rcg::Parser::create() and parse(std::istream&).
             This is the way of soccerwindow2 (and of the tools of librcsc before the speedup).
    read   : read the whole file into memory through rcsc::gzifstream (decompression only).
    parse  : call Parser::create() and parse(std::istream&) on the data in memory (std::istringstream).
    file   : read the whole file into memory by rcsc::gzread_file() (decompression only).
    memory : call Parser::create(data,size) and parse(data,size,handler) on the data in memory.
    total  : file + memory. This is the way of the tools of librcsc.

  -t <Threads> gives the upper limit of the number of the threads that parse the lines
  (rcsc::rcg::Parser::setMaxThreads()). 1 means no parallel parsing. The default is automatic.

  The handler stores all the shows in a std::vector, as a viewer does.
  The messages of Parser::create() are printed to stderr; redirect them by "2>/dev/null".

  The path of the loaded librcsc is printed first (Linux only). LD_LIBRARY_PATH is searched before
  the path recorded in the program, so an installed librcsc (e.g. a debug build in ~/local/lib)
  may be measured instead of the built one. Run it with "env -u LD_LIBRARY_PATH" in that case.

  See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.1.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rcsc/rcg.h>
#include <rcsc/gz/gzfstream.h>
#include <rcsc/gz/gzread_file.h>

#if defined(__linux__)
#include <link.h>
#endif

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

/*!
  \brief a handler that stores the shows and counts the other records
*/
class StoreHandler
    : public rcsc::rcg::Handler {
public:
    std::vector< rcsc::rcg::ShowInfoT > shows_;
    size_t n_records_ = 0; //!< the number of the other records

    bool handleEOF() override
      {
          return true;
      }

    bool handleShow( const rcsc::rcg::ShowInfoT & show ) override
      {
          shows_.push_back( show );
          return true;
      }

    bool handleMsg( const int, const int, const std::string & ) override
      {
          ++n_records_;
          return true;
      }

    bool handleDraw( const int, const rcsc::rcg::drawinfo_t & ) override
      {
          ++n_records_;
          return true;
      }

    bool handlePlayMode( const int, const rcsc::PlayMode ) override
      {
          ++n_records_;
          return true;
      }

    bool handleTeam( const int, const rcsc::rcg::TeamT &, const rcsc::rcg::TeamT & ) override
      {
          ++n_records_;
          return true;
      }

    bool handleServerParam( const rcsc::rcg::ServerParamT & ) override
      {
          ++n_records_;
          return true;
      }

    bool handlePlayerParam( const rcsc::rcg::PlayerParamT & ) override
      {
          ++n_records_;
          return true;
      }

    bool handlePlayerType( const rcsc::rcg::PlayerTypeT & ) override
      {
          ++n_records_;
          return true;
      }

    bool handleTeamGraphic( const char, const int, const int,
                            const std::vector< std::string > & ) override
      {
          ++n_records_;
          return true;
      }
};

/*!
  \brief open a game log as the tools of librcsc do
*/
std::unique_ptr< std::istream >
open_file( const std::string & path )
{
#ifdef HAVE_LIBZ
    std::unique_ptr< rcsc::gzifstream > fin( new rcsc::gzifstream( path.c_str() ) );
    if ( ! fin->is_open() )
    {
        return std::unique_ptr< std::istream >();
    }
    return std::unique_ptr< std::istream >( std::move( fin ) );
#else
    std::unique_ptr< std::ifstream > fin( new std::ifstream( path, std::ios_base::in | std::ios_base::binary ) );
    if ( ! fin->is_open() )
    {
        return std::unique_ptr< std::istream >();
    }
    return std::unique_ptr< std::istream >( std::move( fin ) );
#endif
}

/*!
  \brief the result of one way of loading
*/
struct Timing {
    std::string name_;
    std::string description_;
    std::vector< double > msecs_;

    Timing( const std::string & name,
            const std::string & description )
        : name_( name ),
          description_( description )
      { }

    void print( std::ostream & os ) const
      {
          std::vector< double > sorted = msecs_;
          std::sort( sorted.begin(), sorted.end() );
          os << "  " << std::left << std::setw( 7 ) << name_ << std::right
             << std::fixed << std::setprecision( 1 )
             << " best " << std::setw( 8 ) << sorted.front() << " ms"
             << ", median " << std::setw( 8 ) << sorted[sorted.size() / 2] << " ms"
             << "  (" << description_ << ")"
             << std::defaultfloat << '\n';
      }
};

/*!
  \brief measure the time of func in milliseconds
*/
double
measure( const std::function< bool() > & func,
         bool * result )
{
    const auto start = std::chrono::steady_clock::now();
    *result = func();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration< double, std::milli >( end - start ).count();
}

/*!
  \brief parse a stream by the parser chosen by Parser::create()
*/
bool
parse_stream( std::istream & is,
              StoreHandler & handler )
{
    rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( is );
    return parser
        && parser->parse( is, handler );
}

bool
bench( const std::string & path,
       const int repeat )
{
    size_t n_shows = 0;
    size_t n_records = 0;
    size_t n_bytes = 0;

    Timing stream( "stream", "gzifstream + parse(std::istream&)" );
    Timing read( "read", "gzifstream into memory" );
    Timing parse( "parse", "parse(std::istream&) on the data in memory" );
    Timing file( "file", "gzread_file()" );
    Timing memory( "memory", "parse(data,size) on the data in memory" );
    Timing total( "total", "gzread_file() + parse(data,size)" );

    // parse the data in memory
    auto parse_memory = []( const std::string & data,
                            StoreHandler & handler )
        {
            rcsc::rcg::Parser::Ptr parser = rcsc::rcg::Parser::create( data.data(), data.size() );
            return parser
                && parser->parse( data.data(), data.size(), handler );
        };
    // check that the same data are read
    auto same_data = [&]( const StoreHandler & handler )
        {
            return handler.shows_.size() == n_shows
                && handler.n_records_ == n_records;
        };

    for ( int i = 0; i < repeat; ++i )
    {
        bool result = false;

        // stream
        {
            StoreHandler handler;
            stream.msecs_.push_back( measure( [&]()
                                              {
                                                  std::unique_ptr< std::istream > fin = open_file( path );
                                                  return fin && parse_stream( *fin, handler );
                                              },
                                              &result ) );
            if ( ! result )
            {
                std::cerr << "Failed to parse " << path << std::endl;
                return false;
            }
            n_shows = handler.shows_.size();
            n_records = handler.n_records_;
        }

        // read
        std::string data;
        read.msecs_.push_back( measure( [&]()
                                        {
                                            std::unique_ptr< std::istream > fin = open_file( path );
                                            if ( ! fin ) return false;
                                            std::ostringstream os;
                                            os << fin->rdbuf();
                                            data = os.str();
                                            return true;
                                        },
                                        &result ) );
        if ( ! result )
        {
            std::cerr << "Failed to read " << path << std::endl;
            return false;
        }
        n_bytes = data.size();

        // parse
        {
            std::istringstream is( data );
            StoreHandler handler;
            parse.msecs_.push_back( measure( [&]()
                                             {
                                                 return parse_stream( is, handler );
                                             },
                                             &result ) );
            if ( ! result
                 || ! same_data( handler ) )
            {
                std::cerr << "Failed to parse the data in memory " << path << std::endl;
                return false;
            }
        }

        // file
        std::string file_data;
        file.msecs_.push_back( measure( [&]()
                                        {
                                            return rcsc::gzread_file( path, &file_data );
                                        },
                                        &result ) );
        if ( ! result
             || file_data != data )
        {
            std::cerr << "Failed to read by gzread_file() " << path << std::endl;
            return false;
        }

        // memory
        {
            StoreHandler handler;
            memory.msecs_.push_back( measure( [&]()
                                              {
                                                  return parse_memory( file_data, handler );
                                              },
                                              &result ) );
            if ( ! result
                 || ! same_data( handler ) )
            {
                std::cerr << "Failed to parse the data in memory by parse(data,size) " << path << std::endl;
                return false;
            }
        }

        // total
        {
            StoreHandler handler;
            total.msecs_.push_back( measure( [&]()
                                             {
                                                 std::string d;
                                                 return rcsc::gzread_file( path, &d )
                                                     && parse_memory( d, handler );
                                             },
                                             &result ) );
            if ( ! result
                 || ! same_data( handler ) )
            {
                std::cerr << "Failed to read and parse " << path << std::endl;
                return false;
            }
        }
    }

    std::cout << path << ": " << n_bytes << " bytes, " << n_shows << " shows, "
              << n_records << " other records" << '\n';
    stream.print( std::cout );
    read.print( std::cout );
    parse.print( std::cout );
    file.print( std::cout );
    memory.print( std::cout );
    total.print( std::cout );
    std::cout << std::flush;
    return true;
}

#if defined(__linux__)
int
find_librcsc( struct dl_phdr_info * info,
              size_t,
              void * data )
{
    if ( info->dlpi_name
         && std::strstr( info->dlpi_name, "librcsc" ) )
    {
        *static_cast< std::string * >( data ) = info->dlpi_name;
        return 1;
    }
    return 0;
}
#endif

/*!
  \brief print the path of the loaded librcsc
*/
void
print_librcsc_path()
{
#if defined(__linux__)
    std::string path;
    dl_iterate_phdr( &find_librcsc, &path );
    std::cout << "librcsc: " << ( path.empty() ? std::string( "(not found)" ) : path ) << '\n';
#endif
}

void
usage( const char * prog )
{
    std::cerr << "Usage: " << prog << " [-n <Repeat>] [-t <Threads>] <RcgFile>[.gz]...\n"
              << "  measure the time to load game logs by the rcg parser.\n"
              << "  -n <Repeat> : the number of the measurements (default: 5)\n"
              << "  -t <Threads> : the upper limit of the number of the threads (default: automatic)\n";
}

}

int
main( int argc, char ** argv )
{
    int repeat = 5;
    std::vector< std::string > files;

    for ( int i = 1; i < argc; ++i )
    {
        if ( ! std::strcmp( argv[i], "-h" )
             || ! std::strcmp( argv[i], "--help" ) )
        {
            usage( argv[0] );
            return 0;
        }
        else if ( ! std::strcmp( argv[i], "-n" ) )
        {
            if ( i + 1 >= argc
                 || ( repeat = std::atoi( argv[i + 1] ) ) <= 0 )
            {
                usage( argv[0] );
                return 1;
            }
            ++i;
        }
        else if ( ! std::strcmp( argv[i], "-t" ) )
        {
            int threads = 0;
            if ( i + 1 >= argc
                 || ( threads = std::atoi( argv[i + 1] ) ) <= 0 )
            {
                usage( argv[0] );
                return 1;
            }
            rcsc::rcg::Parser::setMaxThreads( threads );
            ++i;
        }
        else
        {
            files.push_back( argv[i] );
        }
    }

    if ( files.empty() )
    {
        usage( argv[0] );
        return 1;
    }

    print_librcsc_path();
    std::cout << "threads: at most " << rcsc::rcg::Parser::maxThreads() << '\n';

    for ( const std::string & path : files )
    {
        if ( ! bench( path, repeat ) )
        {
            return 1;
        }
    }

    return 0;
}
