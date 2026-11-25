// -*-c++-*-

/*!
  \file line_parser.cpp
  \brief parsing the lines of a game log in memory. Source File.
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

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "line_parser.h"

#include "handler.h"
#include "parser.h"
#include "types.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <system_error>
#include <thread>

// The design follows rcssmonitor (rcss/rcg/line_parser.cpp).
// See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.4 (S1, S10, S13, S19).

namespace rcsc {
namespace rcg {

namespace {

//! the fewest lines for a thread; fewer lines are parsed in one thread
const std::size_t MIN_LINES_PER_THREAD = 1024;

//! the number of the skipped lines reported one by one
const std::size_t MAX_REPORTED_LINES = 10;

/*!
  \class RecordingHandler
  \brief a handler that records what a parser passes to it, to pass it to
  another handler later, in the same order.
*/
class RecordingHandler
    : public Handler {
private:
    std::vector< std::function< bool ( Handler & ) > > M_records;

public:

    /*!
      \brief set the state of the handler that the records are passed to,
      in case the parser refers to it
    */
    explicit
    RecordingHandler( const Handler & target )
      {
          Handler::handleLogVersion( target.logVersion() );
          Handler::handleServerVersion( target.serverVersion() );
          Handler::handleTimestamp( target.timestamp() );
      }

    /*!
      \brief pass the records to the handler. The return values are not used.
    */
    void replay( Handler & handler ) const
      {
          for ( const std::function< bool ( Handler & ) > & record : M_records )
          {
              record( handler );
          }
      }

    void clear()
      {
          M_records.clear();
      }

    bool handleServerVersion( const std::string & version ) override
      {
          Handler::handleServerVersion( version );
          M_records.emplace_back( [version]( Handler & h ) { return h.handleServerVersion( version ); } );
          return true;
      }

    bool handleTimestamp( const std::string & timestamp ) override
      {
          Handler::handleTimestamp( timestamp );
          M_records.emplace_back( [timestamp]( Handler & h ) { return h.handleTimestamp( timestamp ); } );
          return true;
      }

    bool handleEOF() override
      {
          // the parser calls handleEOF() of the target handler after parse_lines()
          return true;
      }

    bool handleShow( const ShowInfoT & show ) override
      {
          M_records.emplace_back( [show]( Handler & h ) { return h.handleShow( show ); } );
          return true;
      }

    bool handleMsg( const int time,
                    const int board,
                    const std::string & msg ) override
      {
          M_records.emplace_back( [time, board, msg]( Handler & h ) { return h.handleMsg( time, board, msg ); } );
          return true;
      }

    bool handleDraw( const int time,
                     const drawinfo_t & draw ) override
      {
          M_records.emplace_back( [time, draw]( Handler & h ) { return h.handleDraw( time, draw ); } );
          return true;
      }

    bool handlePlayMode( const int time,
                         const PlayMode pm ) override
      {
          M_records.emplace_back( [time, pm]( Handler & h ) { return h.handlePlayMode( time, pm ); } );
          return true;
      }

    bool handleTeam( const int time,
                     const TeamT & team_l,
                     const TeamT & team_r ) override
      {
          M_records.emplace_back( [time, team_l, team_r]( Handler & h ) { return h.handleTeam( time, team_l, team_r ); } );
          return true;
      }

    bool handleServerParam( const ServerParamT & param ) override
      {
          // not copy constructible
          std::shared_ptr< ServerParamT > copy = std::make_shared< ServerParamT >();
          copy->copyFrom( param );
          M_records.emplace_back( [copy]( Handler & h ) { return h.handleServerParam( *copy ); } );
          return true;
      }

    bool handlePlayerParam( const PlayerParamT & param ) override
      {
          // not copy constructible
          std::shared_ptr< PlayerParamT > copy = std::make_shared< PlayerParamT >();
          copy->copyFrom( param );
          M_records.emplace_back( [copy]( Handler & h ) { return h.handlePlayerParam( *copy ); } );
          return true;
      }

    bool handlePlayerType( const PlayerTypeT & param ) override
      {
          M_records.emplace_back( [param]( Handler & h ) { return h.handlePlayerType( param ); } );
          return true;
      }

    bool handleTeamGraphic( const char side,
                            const int x,
                            const int y,
                            const std::vector< std::string > & xpm_data ) override
      {
          M_records.emplace_back( [side, x, y, xpm_data]( Handler & h ) { return h.handleTeamGraphic( side, x, y, xpm_data ); } );
          return true;
      }
};

/*!
  \brief parse a line, taking an exception as a failure.
*/
bool
parse_one( const LineParser & parse,
           const LineRef & line,
           Handler & handler )
{
    try
    {
        return parse( line, handler );
    }
    catch ( ... )
    {
        return false;
    }
}

/*!
  \struct Block
  \brief consecutive lines that a thread parses
*/
struct Block {
    std::size_t first_; //!< the index of the first line
    std::size_t last_; //!< the index after the last line
    RecordingHandler recorder_; //!< what the parser passed
    std::vector< int > failed_; //!< the numbers of the lines that failed

    Block( const std::size_t first,
           const std::size_t last,
           const Handler & target )
        : first_( first ),
          last_( last ),
          recorder_( target )
      { }

    void parse( const std::vector< LineRef > & lines,
                const LineParserFactory & make_parser )
      {
          const LineParser parser = make_parser();
          for ( std::size_t i = first_; i < last_; ++i )
          {
              if ( ! parse_one( parser, lines[i], recorder_ ) )
              {
                  failed_.push_back( lines[i].number_ );
              }
          }
      }
};

}

/*-------------------------------------------------------------------*/
/*!

 */
std::vector< LineRef >
split_lines( const char * data,
             const std::size_t size )
{
    std::vector< LineRef > lines;

    const char * const end = data + size;
    const char * pos = data;
    int number = 0;
    while ( pos < end )
    {
        const char * eol = static_cast< const char * >( std::memchr( pos, '\n', end - pos ) );
        if ( ! eol )
        {
            eol = end;
        }

        lines.push_back( LineRef{ pos, static_cast< std::size_t >( eol - pos ), ++number } );
        pos = ( eol < end ? eol + 1 : end );
    }

    return lines;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
count_threads( const std::size_t n_lines )
{
    const std::size_t limit = static_cast< std::size_t >( std::max( 1, Parser::maxThreads() ) );
    return static_cast< int >( std::max( std::size_t( 1 ),
                                         std::min( limit, n_lines / MIN_LINES_PER_THREAD ) ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
std::vector< int >
parse_lines( const std::vector< LineRef > & lines,
             const LineParserFactory & make_parser,
             Handler & handler )
{
    std::vector< int > failed;

    const std::size_t threads = static_cast< std::size_t >( count_threads( lines.size() ) );

    if ( threads <= 1 )
    {
        // each line is recorded and passed at once,
        // so that the handler is used in the same way as with the threads (S19)
        const LineParser parser = make_parser();
        RecordingHandler recorder( handler );
        for ( const LineRef & line : lines )
        {
            recorder.clear();
            if ( ! parse_one( parser, line, recorder ) )
            {
                failed.push_back( line.number_ );
            }
            recorder.replay( handler );
        }
        return failed;
    }

    std::vector< std::unique_ptr< Block > > blocks;
    blocks.reserve( threads );
    for ( std::size_t i = 0; i < threads; ++i )
    {
        blocks.emplace_back( new Block( lines.size() * i / threads,
                                        lines.size() * ( i + 1 ) / threads,
                                        handler ) );
    }

    std::vector< std::thread > workers;
    workers.reserve( threads );
    std::size_t started = 0;
    try
    {
        for ( ; started < blocks.size(); ++started )
        {
            Block * block = blocks[started].get();
            workers.emplace_back( [&lines, &make_parser, block]()
                                  {
                                      block->parse( lines, make_parser );
                                  } );
        }
    }
    catch ( const std::system_error & e )
    {
        std::cerr << "(rcsc::rcg::parse_lines) failed to start a thread: " << e.what() << std::endl;
    }

    // the blocks without a thread are parsed in this thread
    for ( std::size_t i = started; i < blocks.size(); ++i )
    {
        blocks[i]->parse( lines, make_parser );
    }

    for ( std::thread & worker : workers )
    {
        worker.join();
    }

    for ( const std::unique_ptr< Block > & block : blocks )
    {
        block->recorder_.replay( handler );
        failed.insert( failed.end(), block->failed_.begin(), block->failed_.end() );
    }

    return failed;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
report_skipped( const std::string & who,
                const std::string & what,
                const std::vector< int > & numbers )
{
    for ( std::size_t i = 0; i < numbers.size() && i < MAX_REPORTED_LINES; ++i )
    {
        std::cerr << '(' << who << ") " << what << ' ' << numbers[i]
                  << ": skipped, as it could not be parsed" << std::endl;
    }

    if ( numbers.size() > MAX_REPORTED_LINES )
    {
        std::cerr << '(' << who << ") skipped " << numbers.size() << ' ' << what << "s in total" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
read_stream( std::istream & is,
             std::string * data )
{
    data->clear();

    const std::size_t chunk = 1 << 20;
    std::size_t size = 0;
    while ( is )
    {
        data->resize( size + chunk );
        is.read( &( *data )[size], chunk );
        size += static_cast< std::size_t >( is.gcount() );
    }
    data->resize( size );

    return ! is.bad();
}

}
}
