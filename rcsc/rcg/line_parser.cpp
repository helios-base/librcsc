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

#include <cstring>

namespace rcsc {
namespace rcg {

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
