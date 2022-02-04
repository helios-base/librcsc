// -*-c++-*-

/*!
  \file test_rundom_convex_hull.cpp
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

#include <random>
#include <fstream>

#include <sys/time.h> // struct timeval, gettimeofday()

void
print_elapsed( const timeval & start,
               const timeval & end )
{
    long sec_diff = end.tv_sec - start.tv_sec;
    long usec_diff = end.tv_usec - start.tv_usec;

    while ( usec_diff >= (1000 * 1000) )
    {
        usec_diff -= 1000 * 1000;
        ++sec_diff;
    }

    while ( usec_diff < 0 && sec_diff > 0 )
    {
        usec_diff += (1000 * 1000);
        --sec_diff;
    }

    std::cout << "  elapsed "
              << sec_diff * 1000.0 + usec_diff * 0.001
              << " [ms]" << std::endl;
}

int
main()
{
    std::random_devince seed_gen;
    std::mt19937 engine( seed_gen() );

    //const double min_x = -10.0;
    //const double max_x = +10.0;
    //const double min_y = -10.0;
    //const double max_y = +10.0;
    //std::uniform_real_distribution<> > x_dst( min_x, max_x );
    //std::uniform_real_distribution<> > y_dst( min_y, max_y );

    std::normal_distribution<> > x_dst( 0.0, 5.0 );
    std::normal_distribution<> > y_dst( 0.0, 5.0 );

    rcsc::ConvexHull c0;
    rcsc::ConvexHull c1;

    for ( int i = 0; i < 1000; ++i )
    {
        rcsc::Vector2D p( x_dst( engine ), y_dst( engine ) );
        c0.addPoint( p );
        c1.addPoint( p );
    }

    {
        timeval start;
        if ( ::gettimeofday( &start, NULL ) == -1 )
        {
            std::perror( "gettimeofday" );
        }

        c0.compute( rcsc::ConvexHull::WrappingMethod );

        timeval end;
        if ( ::gettimeofday( &end, NULL ) == -1 )
        {
            std::perror( "gettimeofday" );
        }

        std::cout << "WrappingMethod\n"
                  << "  points = " << c0.inputPoints().size() << '\n'
                  << "  vertices = " << c0.vertices().size() << '\n'
                  << "  edges = " << c0.edges().size() << '\n';

        print_elapsed( start, end );
    }


    {
        timeval start;
        if ( ::gettimeofday( &start, NULL ) == -1 )
        {
            std::perror( "gettimeofday" );
        }

        c1.compute( rcsc::ConvexHull::GrahamScan );

        timeval end;
        if ( ::gettimeofday( &end, NULL ) == -1 )
        {
            std::perror( "gettimeofday" );
        }

        std::cout << "GrahamScan\n"
                  << "  points = " << c1.inputPoints().size() << '\n'
                  << "  vertices = " << c1.vertices().size() << '\n'
                  << "  edges = " << c1.edges().size() << '\n';

        print_elapsed( start, end );
    }


    std::ofstream point_file( "points.dat" );
    std::ofstream edge_file( "edges.dat" );

    c1.printInputPoints( point_file );
    c1.printEdges( edge_file );

    /*
      > gnuplot
      > plot "points.dat" w p, "edges.dat" w l
     */

    return 0;
}
