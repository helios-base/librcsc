// -*-c++-*-

/*!
  \file gzread_file.cpp
  \brief read a whole (gzip) file into memory Source File
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

#include "gzread_file.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#ifdef HAVE_LIBDEFLATE
#include <libdeflate.h>
#endif

// See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.3 (S3, D7) and 4.5 (D4).

namespace rcsc {

namespace {

/*!
  \brief check if the file begins with the magic number of gzip
  \param fin the file opened in binary mode. It is read from the beginning.
*/
bool
is_gzip( std::ifstream & fin )
{
    unsigned char magic[2] = { 0, 0 };
    fin.read( reinterpret_cast< char * >( magic ), 2 );
    return fin.gcount() == 2
        && magic[0] == 0x1f
        && magic[1] == 0x8b;
}

/*!
  \brief read the file as it is
*/
bool
read_raw( std::ifstream & fin,
          std::string * data )
{
    fin.clear();
    fin.seekg( 0, std::ios_base::end );
    const std::streamoff size = fin.tellg();
    fin.seekg( 0, std::ios_base::beg );
    if ( size < 0 )
    {
        return false;
    }

    data->resize( static_cast< std::size_t >( size ) );
    return size == 0
        || static_cast< bool >( fin.read( &( *data )[0], size ) );
}

#ifdef HAVE_LIBDEFLATE
/*!
  \brief decompress gzip data in memory with libdeflate, which is a few times faster than zlib.
  The data may have more than one member, and the bytes that are not gzip after a member
  are ignored, as zlib does.
  \return false if the data are broken (also when they end in the middle)

  The design follows rcssmonitor (src/game_log_file.cpp).
*/
bool
inflate_with_libdeflate( const std::string & compressed,
                         std::string * data )
{
    // the largest ratio of deflate is about 1032:1
    const std::size_t max_size = compressed.size() * 1032 + ( 1u << 20 );

    struct Free {
        void operator()( libdeflate_decompressor * d ) const
          {
              libdeflate_free_decompressor( d );
          }
    };
    std::unique_ptr< libdeflate_decompressor, Free > decompressor( libdeflate_alloc_decompressor() );
    if ( ! decompressor )
    {
        return false;
    }

    const unsigned char * in = reinterpret_cast< const unsigned char * >( compressed.data() );
    std::size_t in_size = compressed.size();
    std::size_t size = 0;

    // the size of the last member (modulo 2^32), from the last 4 bytes
    const std::size_t hint
        = ( in_size >= 4
            ? ( static_cast< std::size_t >( in[in_size - 4] )
                | ( static_cast< std::size_t >( in[in_size - 3] ) << 8 )
                | ( static_cast< std::size_t >( in[in_size - 2] ) << 16 )
                | ( static_cast< std::size_t >( in[in_size - 1] ) << 24 ) )
            : 0 );
    data->resize( std::min( std::max( hint, in_size ), max_size ) );

    while ( in_size >= 2
            && in[0] == 0x1f
            && in[1] == 0x8b )
    {
        std::size_t in_used = 0;
        std::size_t out_size = 0;
        const libdeflate_result result
            = libdeflate_gzip_decompress_ex( decompressor.get(),
                                             in, in_size,
                                             &( *data )[0] + size, data->size() - size,
                                             &in_used, &out_size );
        if ( result == LIBDEFLATE_INSUFFICIENT_SPACE )
        {
            if ( data->size() >= max_size )
            {
                return false;
            }
            data->resize( std::min( data->size() * 2, max_size ) );
            continue;
        }

        if ( result != LIBDEFLATE_SUCCESS )
        {
            return false;
        }

        size += out_size;
        in += in_used;
        in_size -= in_used;
    }

    data->resize( size );
    return true;
}
#endif

#ifdef HAVE_LIBZ
/*!
  \brief get the size of the decompressed data from the last 4 bytes of a gzip file
  (the size of the last member, modulo 2^32), to reserve the buffer.
  \return 0 if it is not available

  The value is limited by the maximum compression ratio of deflate (about 1032),
  because the last 4 bytes of a broken file may have any value.
*/
std::size_t
gzip_size_hint( std::ifstream & fin )
{
    fin.clear();
    unsigned char isize[4];
    if ( ! fin.seekg( -4, std::ios_base::end ) )
    {
        return 0;
    }
    const std::streamoff file_size = static_cast< std::streamoff >( fin.tellg() ) + 4;
    if ( ! fin.read( reinterpret_cast< char * >( isize ), 4 ) )
    {
        return 0;
    }

    const std::size_t hint = static_cast< std::size_t >( isize[0] )
        | ( static_cast< std::size_t >( isize[1] ) << 8 )
        | ( static_cast< std::size_t >( isize[2] ) << 16 )
        | ( static_cast< std::size_t >( isize[3] ) << 24 );
    const std::size_t max_size = static_cast< std::size_t >( file_size ) * 1032;
    return std::min( hint, max_size );
}

/*!
  \brief decompress a gzip file with zlib, by large chunks
*/
bool
read_gzip( const std::string & path,
           const std::size_t size_hint,
           std::string * data )
{
    gzFile file = gzopen( path.c_str(), "rb" );
    if ( ! file )
    {
        std::cerr << "(rcsc::gzread_file) failed to open " << path << std::endl;
        return false;
    }

    const unsigned int chunk = 1u << 20;
    gzbuffer( file, chunk );
    data->clear();
    data->reserve( size_hint + chunk );

    std::size_t size = 0;
    while ( true )
    {
        data->resize( size + chunk );
        const int n = gzread( file, &( *data )[size], chunk );
        if ( n < 0 )
        {
            int errnum = 0;
            std::cerr << "(rcsc::gzread_file) " << path << ": " << gzerror( file, &errnum ) << std::endl;
            gzclose( file );
            data->clear();
            return false;
        }

        size += static_cast< std::size_t >( n );
        if ( static_cast< unsigned int >( n ) < chunk )
        {
            break;
        }
    }
    data->resize( size );

    // Z_BUF_ERROR: the file ends in the middle of a gzip stream
    int errnum = Z_OK;
    gzerror( file, &errnum );
    if ( errnum == Z_BUF_ERROR )
    {
        std::cerr << "(rcsc::gzread_file) " << path << ": unexpected end of file."
                  << " The data are read up to there." << std::endl;
    }

    gzclose( file );
    return true;
}
#endif

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
gzread_file( const std::string & path,
             std::string * data )
{
    if ( ! data )
    {
        return false;
    }

    std::ifstream fin( path, std::ios_base::in | std::ios_base::binary );
    if ( ! fin )
    {
        std::cerr << "(rcsc::gzread_file) failed to open " << path << std::endl;
        return false;
    }

    if ( ! is_gzip( fin ) )
    {
        if ( ! read_raw( fin, data ) )
        {
            std::cerr << "(rcsc::gzread_file) failed to read " << path << std::endl;
            data->clear();
            return false;
        }
        return true;
    }

#ifdef HAVE_LIBDEFLATE
    {
        std::string compressed;
        if ( ! read_raw( fin, &compressed ) )
        {
            std::cerr << "(rcsc::gzread_file) failed to read " << path << std::endl;
            data->clear();
            return false;
        }

        if ( inflate_with_libdeflate( compressed, data ) )
        {
            return true;
        }

        // the data are broken (e.g. the file ends in the middle).
        // zlib reads the data up to where they are broken.
        data->clear();
    }
#endif

#ifdef HAVE_LIBZ
    const std::size_t size_hint = gzip_size_hint( fin );
    fin.close();
    return read_gzip( path, size_hint, data );
#elif defined(HAVE_LIBDEFLATE)
    std::cerr << "(rcsc::gzread_file) " << path << ": broken gzip data" << std::endl;
    return false;
#else
    std::cerr << "(rcsc::gzread_file) " << path << ": gzip is not supported (librcsc is built without zlib and libdeflate)" << std::endl;
    data->clear();
    return false;
#endif
}

}
