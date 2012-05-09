// -*-c++-*-

/*!
  \file rcgversion.cpp
  \brief rcg version printer source File.
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

#include <rcsc/gz.h>
#include <rcsc/rcg/types.h>

#include <iostream>
#include <fstream>
#include <string>

///////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
/*

*/
static
int
get_version( std::istream & is )
{
    int version = rcsc::rcg::REC_OLD_VERSION;

    char header[5];

    is.read( header, 4 ); // read 'U', 'L', 'G', <version>

    if ( is.gcount() != 4 )
    {
        return -1;
    }

    if ( header[0] == 'U'
         && header[1] == 'L'
         && header[2] == 'G' )
    {
        version = static_cast< int >( header[3] );
    }

    if ( version == rcsc::rcg::REC_OLD_VERSION
         || version == rcsc::rcg::REC_VERSION_2
         || version == rcsc::rcg::REC_VERSION_3 )
    {
        return version;
    }

    if ( version == static_cast< int >( '0' ) + rcsc::rcg::REC_VERSION_4 )
    {
        return rcsc::rcg::REC_VERSION_4;
    }

    if ( version == static_cast< int >( '0' ) + rcsc::rcg::REC_VERSION_5 )
    {
        return rcsc::rcg::REC_VERSION_5;
    }

    return -1;
}

/*---------------------------------------------------------------*/
/*

*/
static
void
usage( const char * prog )
{
    std::cerr << "Usage: " << prog <<  " <RcgFile>[.gz]"
              << std::endl;
}


////////////////////////////////////////////////////////////////////////

int
main( int argc, char ** argv )
{
    if ( argc < 2 )
    {
        usage( argv[0] );
        return 1;
    }

    for ( int i = 1; i < argc; ++i )
    {
        if ( argv[i][0] == '-' )
        {
            continue;
        }

        rcsc::gzifstream fin( argv[i] );

        if ( ! fin.is_open() )
        {
            std::cerr << "Failed to open file : " << argv[i]
                      << std::endl;
            continue;
        }

        int ver = get_version( fin );

        fin.close();

        std::cout << "file=" << argv[i] << ", version=" << ver
                  << std::endl;
    }

    return 0;
}
