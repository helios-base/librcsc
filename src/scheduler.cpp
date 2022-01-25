// -*-c++-*-

/*!
  \file scheduler.cpp
  \brief league scheduler program source File.
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
#include "config.h"
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <set>
#include <utility>
#include <string>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>

///////////////////////////////////////////////////////////

class TeamNameLoader {
private:
    std::vector< std::string > M_teams;
public:
    TeamNameLoader()
      { }

    explicit
    TeamNameLoader( std::istream & is );

    const
    std::vector< std::string > & teams() const
      {
          return M_teams;
      }

    std::ostream & print( std::ostream & os ) const;
};

///////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
/*!

*/
TeamNameLoader::TeamNameLoader( std::istream & is )
{
    const std::string valid_chars
        ( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-/" );

    std::string buf;
    int n_line = 0;

    std::set< std::string > read_teams;

    while ( std::getline( is, buf ) )
    {
        ++n_line;

        std::replace( buf.begin(), buf.end(), '\t', ' ' );
        buf.erase( std::remove( buf.begin(), buf.end(), ' ' ),
                   buf.end() );

        if ( buf.empty()
             || buf[0] == '#' )
        {
            continue;
        }

        if ( buf.find_first_not_of( valid_chars ) != std::string::npos )
        {
            std::cerr << "Found an illegal character at line " << n_line
                      << " [" << buf << "]"
                      << std::endl;
            continue;
        }

        if ( read_teams.find( buf ) != read_teams.end() )
        {
            std::cerr << "Found a exsiting team name at line " << n_line
                      << " [" << buf << "]"
                      << std::endl;
            continue;
        }
        read_teams.insert( buf );

        M_teams.push_back( buf );
    }
}

/*---------------------------------------------------------------*/
/*!

*/
std::ostream &
TeamNameLoader::print( std::ostream & os ) const
{
    for ( const auto & s : M_teams )
    {
        os << s << std::endl;
    }
    return os;
}


///////////////////////////////////////////////////////////

class Scheduler {
public:
    typedef std::pair< int, int > MatchId;
    typedef std::vector< MatchId > MatchCont;

private:
    int M_total_teams;
    MatchCont M_match_list;

public:
    Scheduler()
        : M_total_teams( 0 )
      { }

    Scheduler( const int total_teams,
               const int num_parallel )
      {
          create( total_teams, num_parallel );
      }

    void create( const int total_teams,
                 const int num_parallel );

    const
    MatchCont & matchList() const
      {
          return M_match_list;
      }

    std::ostream & print( std::ostream & os ) const;
};

/*---------------------------------------------------------------*/
/*!

*/
void
Scheduler::create( const int total_teams,
                   const int num_parallel )
{
    if ( total_teams < 2 )
    {
        return;
    }

    M_total_teams = total_teams;
    M_match_list.clear();

    //
    // create single line schedule
    //
    const int total_matches = total_teams * (total_teams - 1) / 2;
    for ( int match = 1; match <= total_matches; ++match )
    {
        int k[2];
        k[0] = 1;
        k[1] = total_teams;

        int i = 0;
        int down = 0;

        int m = 1;
        while ( m < match )
        {
            if ( i + k[down] < total_teams - 1 )
            {
                ++i;
            }
            else
            {
                i = 0;
                if ( down == 0 )
                {
                    down = 1;
                    --k[down];
                } else {
                    down = 0;
                    ++k[down];
                }
            }
            ++m;
        }

        int left = i;
        int right = k[down] + i;

        M_match_list.push_back( std::make_pair( left, right ) );
    }

    //
    // create parallel line schedule
    //
    if ( num_parallel > 1
         && num_parallel * 2 <= total_teams )
    {
        std::size_t num_para = static_cast< std::size_t >( num_parallel );

        MatchCont new_list;
        new_list.reserve( M_match_list.size() );

        std::list< std::pair< int, int > > match_cache; // read matches
        std::list< std::pair< int, int > > matches; // resistered matches
        std::set< int > team_set;

        //int count = 0;
        for ( const auto & m : M_match_list )
        {
            //std::cerr << ++count << ": read match: "
            //          << it->first << " vs " << it->second
            //          << std::endl;
            match_cache.push_back( m );

            std::list< std::pair< int, int > >::iterator c = match_cache.begin();
            while ( c != match_cache.end() )
            {
                if ( team_set.find( c->first ) != team_set.end()
                     || team_set.find( c->second ) != team_set.end() )
                {
                    ++c;
                    continue;
                }

                team_set.insert( c->first );
                team_set.insert( c->second );

                matches.push_back( *c );

                c = match_cache.erase( c );
            }

            //std::cerr << "team set size = " << team_set.size() << std::endl;
            //std::cerr << "matches size = " << matches.size() << std::endl;

            if ( matches.size() >= num_para )
            {
                std::list< std::pair< int, int > >::iterator m = matches.begin();
                for ( std::size_t i = 0; i < num_para; ++i )
                {
                    //std::cout << "register " << i << ": " << m->first
                    //          << " vs " << m->second << '\n';
                    new_list.push_back( *m );
                    team_set.erase( m->first );
                    team_set.erase( m->second );
                    m = matches.erase( m );
                }
            }
        }

        //std::cerr << "match cache size = " << match_cache.size() << std::endl;
        //std::cerr << "matches size = " << matches.size() << std::endl;
        //std::cerr << "new list size = " << new_list.size() << std::endl;

        if ( M_match_list.size() != new_list.size() )
        {
            std::cerr << "*** Failed to create the paralleled matches.*** \n"
                      << "  original = " << M_match_list.size()
                      << "  new size = " << new_list.size()
                      << "  match cache size = " << match_cache.size()
                      << "  remained size = " << matches.size()
                      << std::endl;
        }

        M_match_list = new_list;

        for ( const auto & m : matches )
        {
            M_match_list.push_back( m );
        }

        for ( const auto & c : match_cache )
        {
            M_match_list.push_back( c );
        }
    }
}

/*---------------------------------------------------------------*/
/*!

*/
std::ostream &
Scheduler::print( std::ostream & os ) const
{
    //int count = 0;
    for ( const auto & m : M_match_list )
    {
        // os << ++count << ": " << it->first << " vs " << it->second << std::endl;
        os << m.first << " vs " << m.second << std::endl;
    }
    return os;
}

///////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
/*
  Usage:
  $ rclmscheduler < TeamListedFile
*/
void
usage( const char * prog )
{
    std::cerr << "Usage: " << prog << " < TeamListedFile\n"
              << "Available options:\n"
              << "    --help [ -h ]\n"
              << "        print this message.\n"
              << "    --para <Value> : (DefaultValue=\"1\")\n"
              << "        specify the number of the parallel line schedule."
              << std::endl;
}


///////////////////////////////////////////////////////////

int
main( int argc, char** argv )
{
    int num_para = 1;

    for ( int i = 1; i < argc; ++i )
    {
        if ( ! std::strcmp( argv[i], "--help" )
             || ! std::strcmp( argv[i], "-h" ) )
        {
            usage( argv[0] );
            return 0;
        }
        else if ( ! std::strcmp( argv[i], "--para" ) )
        {
            ++i;
            if ( i >= argc )
            {
                usage( argv[0] );
                return 1;
            }

            long num = std::strtol( argv[i], NULL, 10 );
            if ( num == LONG_MAX
                 || num == LONG_MIN
                 || num < 1 )
            {
                usage( argv[0] );
                return 1;
            }
            num_para = static_cast< int >( num );
        }
    }

    TeamNameLoader loader( std::cin );
    //loader.print( std::cout );
    //std::cout << " -------------------------------- " << std::endl;

    if( loader.teams().empty() )
    {
        std::cerr << "No teams!" << std::endl;
        usage( argv[0] );
        return 1;
    }

    Scheduler scheduler( loader.teams().size(),
                         num_para );

    //scheduler.print( std::cout );
    //std::cout << " -------------------------------- " << std::endl;

    std::cerr << "The number of teams : " << loader.teams().size() << std::endl;
    std::cerr << "The number of matches : " << scheduler.matchList().size() << std::endl;

    const int total_teams = static_cast< int >( loader.teams().size() );
    const bool para = ( num_para > 1 && num_para * 2 <= total_teams );

    int count = 0;
    for ( const auto & m : scheduler.matchList() )
    {
        if ( para )
        {
            if ( count % num_para == 0 )
            {
                std::cout << "----- phase "
                          << ( count / num_para ) + 1
                          << " -----\n";
            }
            std::cout << ( count % num_para ) << ": ";
        }

        ++count;

        std::cout
            //<< ++count << ": "
            << loader.teams().at( m.first )
            << " vs "
            << loader.teams().at( m.second )
            << '\n';
    }

    std::cout << std::flush;

    return 0;
}
