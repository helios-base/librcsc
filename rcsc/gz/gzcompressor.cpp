// -*-c++-*-

/*!
  \file gzcompressor.cpp
  \brief gzip compressor/decompressor Source File.
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

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

#include "gzcompressor.h"

#include <cstdlib>

namespace rcsc {

/////////////////////////////////////////////////////////////////////

/*!
  \class GZCompressorImpl
  \brief the implementation of gzip compressor
 */
class GZCompressor::Impl {
private:
#ifdef HAVE_LIBZ
    z_stream M_stream;

    char * M_out_buffer;
    int M_out_size;
    int M_out_avail;
#endif
public:

    /*!
      \brief construct with the compression level
      \param level gzip compression level
     */
    explicit
    Impl( const int level )
#ifdef HAVE_LIBZ
        : M_out_buffer( nullptr )
        , M_out_size( 0 )
        , M_out_avail( 0 )
#endif
      {
#ifdef HAVE_LIBZ
          M_stream.zalloc = Z_NULL;
          M_stream.zfree = Z_NULL;
          M_stream.opaque = nullptr;

          int lv = std::max( Z_BEST_SPEED, level );
          lv = std::min( level, Z_BEST_COMPRESSION );

          deflateInit( &M_stream, lv );
          deflateParams( &M_stream, lv, Z_DEFAULT_STRATEGY );
#endif
      }

    /*!
      \brief cleaned up the memory
     */
    ~Impl()
      {
#ifdef HAVE_LIBZ
          deflateEnd( &M_stream );
          std::free( M_out_buffer );
#endif
      }

    /*!
      \return Z_OK or Z_ERROR
     */
    int setLevel( const int level )
      {
#ifdef HAVE_LIBZ
          int lv = std::max( Z_BEST_SPEED, level );
          lv = std::min( level, Z_BEST_COMPRESSION );

          return deflateParams( &M_stream, lv, Z_DEFAULT_STRATEGY );
#else
          return 0;
#endif
      }

    /*!
      \return the return value of deflate

      Z_OK, Z_STREAM_END, Z_STREAM_ERROR, Z_BUF_ERROR
     */
    int compress( const char * src_buf,
                  const int src_size,
                  std::string & dest )
      {
          dest.clear();
#ifdef HAVE_LIBZ
          // allocate output buffer
          if ( M_out_buffer == nullptr )
          {
              M_out_avail = static_cast< int >( src_size * 1.01 + 12 );
              M_out_buffer = static_cast< char * >( std::malloc( M_out_avail ) );

              if ( M_out_buffer == nullptr )
              {
                  return Z_MEM_ERROR;
              }

              M_stream.next_out = (Bytef*)M_out_buffer;
              M_stream.avail_out = M_out_avail;
          }

          M_stream.next_in = (Bytef*)src_buf;
          M_stream.avail_in = src_size;

          int bytes_out = M_stream.total_out;
          int err = 0;
          for ( ; ; )
          {
              if ( M_stream.avail_out == 0 )
              {
                  int extra = static_cast< int >( M_out_avail * 0.5 );
                  M_out_buffer
                      = static_cast< char * >( std::realloc( M_out_buffer,
                                                             M_out_avail + extra ) );
                  if ( M_out_buffer == nullptr )
                  {
                      err = Z_MEM_ERROR;
                      break;
                  }

                  M_stream.next_out = (Bytef*)( M_out_buffer + M_out_avail );
                  M_stream.avail_out += extra;
                  M_out_avail += extra;
              }

              err = deflate( &M_stream, Z_SYNC_FLUSH ); //Z_NO_FLUSH );

              if ( err != Z_OK )
              {
                  break;
              }
          }

          M_out_size = M_stream.total_out - bytes_out;

          // copy to the destination buffer

          dest.assign( M_out_buffer, M_out_size );

          M_out_size = 0;
          deflateReset( &M_stream );
          M_stream.next_out = (Bytef*)M_out_buffer;
          M_stream.avail_out = M_out_avail;

          return err;
#else
          dest.assign( src_buf, src_size );
          return 0;
#endif
      }
};

/////////////////////////////////////////////////////////////////////

/*!
  \class GZDecompressorImpl
  \brief the implementation of gzip decompressor
 */
class GZDecompressor::Impl {
private:
#ifdef HAVE_LIBZ
    z_stream M_stream;

    char * M_out_buffer;
    int M_out_size;
    int M_out_avail;
#endif
public:
    Impl()
#ifdef HAVE_LIBZ
        : M_out_buffer( nullptr )
        , M_out_size( 0 )
        , M_out_avail( 0 )
#endif
      {
#ifdef HAVE_LIBZ
          M_stream.zalloc = Z_NULL;
          M_stream.zfree = Z_NULL;
          M_stream.opaque = nullptr;

          inflateInit( &M_stream );
#endif
      }

    ~Impl()
      {
#ifdef HAVE_LIBZ
          inflateEnd( &M_stream );
          std::free( M_out_buffer );
#endif
      }

    /*!
      \brief decompress the message
      \param src_buf source message
      \param src_size the length of source message
      \param dest reference to the destination variable.
     */
    int decompress( const char * src_buf,
                    const int src_size,
                    std::string & dest )
      {
          dest.clear();
#ifdef HAVE_LIBZ
          // allocate output buffer
          if ( M_out_buffer == nullptr )
          {
              M_out_avail = src_size * 2;
              M_out_buffer
                  = static_cast< char * >( std::malloc( M_out_avail ) );
              if ( M_out_buffer == nullptr )
              {
                  return Z_MEM_ERROR;
              }

              M_stream.next_out = (Bytef*)M_out_buffer;
              M_stream.avail_out = M_out_avail;
          }

          M_stream.next_in = (Bytef*)src_buf;
          M_stream.avail_in = src_size;

          int bytes_out = M_stream.total_out;

          int err;
          for ( ; ; )
          {
              if ( M_stream.avail_out == 0 )
              {
                  int extra = static_cast< int >( M_out_avail * 0.5 );
                  M_out_buffer
                      = static_cast< char * >( std::realloc( M_out_buffer,
                                                             M_out_avail + extra ) );
                  if ( M_out_buffer == nullptr )
                  {
                      err = Z_MEM_ERROR;
                      break;
                  }

                  M_stream.next_out = (Bytef*)( M_out_buffer + M_out_avail );
                  M_stream.avail_out += extra;
                  M_out_avail += extra;
              }

              err = inflate( &M_stream, Z_SYNC_FLUSH ); // Z_NO_FLUSH );

              if ( err != Z_OK )
              {
                  break;
              }
          }

          M_out_size = M_stream.total_out - bytes_out;

          // copy to the destination buffer

          dest.assign( M_out_buffer, M_out_size );

          M_out_size = 0;
          inflateReset( &M_stream );
          M_stream.next_out = (Bytef*)M_out_buffer;
          M_stream.avail_out = M_out_avail;

          return err;
#else
          dest.assign( src_buf, src_size );
          return 0;
#endif
      }
};


/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
GZCompressor::GZCompressor( const int level )
    : M_impl( new Impl( level ) )
{

}


/*-------------------------------------------------------------------*/
/*!

*/
GZCompressor::~GZCompressor()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
GZCompressor::setLevel( int level )
{
    return M_impl->setLevel( level );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
GZCompressor::compress( const char * src_buf,
                        const int src_size,
                        std::string & dest )
{
    return M_impl->compress( src_buf, src_size, dest );
}

/////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
GZDecompressor::GZDecompressor()
    : M_impl( new Impl() )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
GZDecompressor::~GZDecompressor()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
GZDecompressor::decompress( const char * src_buf,
                            const int src_size,
                            std::string & dest )
{
    return M_impl->decompress( src_buf, src_size, dest );
}

}
