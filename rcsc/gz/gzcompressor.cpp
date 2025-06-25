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
#include <memory>
#include <algorithm>
#include <stdexcept>

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
    
    // Use smart pointer for automatic memory management
    std::unique_ptr<char[]> M_out_buffer;
    size_t M_out_size;
    size_t M_out_avail;
    bool M_initialized;
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
        , M_initialized( false )
#endif
      {
#ifdef HAVE_LIBZ
          M_stream.zalloc = Z_NULL;
          M_stream.zfree = Z_NULL;
          M_stream.opaque = nullptr;

          int lv = std::clamp(level, Z_BEST_SPEED, Z_BEST_COMPRESSION);

          int result = deflateInit( &M_stream, lv );
          if (result != Z_OK) {
              throw std::runtime_error("Failed to initialize deflate");
          }
          
          result = deflateParams( &M_stream, lv, Z_DEFAULT_STRATEGY );
          if (result != Z_OK) {
              deflateEnd(&M_stream);
              throw std::runtime_error("Failed to set deflate parameters");
          }
          
          M_initialized = true;
#endif
      }

    /*!
      \brief cleaned up the memory
     */
    ~Impl()
      {
#ifdef HAVE_LIBZ
          if (M_initialized) {
              deflateEnd( &M_stream );
          }
          // M_out_buffer will be automatically freed by unique_ptr
#endif
      }

    /*!
      \return Z_OK or Z_ERROR
     */
    int setLevel( const int level )
      {
#ifdef HAVE_LIBZ
          if (!M_initialized) {
              return Z_STREAM_ERROR;
          }
          
          int lv = std::clamp(level, Z_BEST_SPEED, Z_BEST_COMPRESSION);
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
          
          if (!src_buf || src_size <= 0) {
              return Z_STREAM_ERROR;
          }
          
#ifdef HAVE_LIBZ
          if (!M_initialized) {
              return Z_STREAM_ERROR;
          }
          
          // allocate output buffer with better size estimation
          if ( !M_out_buffer )
          {
              // Better size estimation: worst case is ~1% + 12 bytes for small data
              // For larger data, compression ratio is typically better
              M_out_avail = std::max(static_cast<size_t>(src_size * 1.01 + 12), 
                                   static_cast<size_t>(1024));
              
              try {
                  M_out_buffer = std::make_unique<char[]>(M_out_avail);
              } catch (const std::bad_alloc&) {
                  return Z_MEM_ERROR;
              }

              M_stream.next_out = reinterpret_cast<Bytef*>(M_out_buffer.get());
              M_stream.avail_out = static_cast<uInt>(M_out_avail);
          }

          M_stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(src_buf));
          M_stream.avail_in = static_cast<uInt>(src_size);

          const uLong bytes_out = M_stream.total_out;
          int err = Z_OK;
          
          // Process all input data
          while ( M_stream.avail_in > 0 )
          {
              if ( M_stream.avail_out == 0 )
              {
                  // Expand buffer by 50%
                  const size_t extra = M_out_avail / 2;
                  const size_t new_size = M_out_avail + extra;
                  
                  try {
                      auto new_buffer = std::make_unique<char[]>(new_size);
                      std::copy_n(M_out_buffer.get(), M_out_avail, new_buffer.get());
                      M_out_buffer = std::move(new_buffer);
                      M_out_avail = new_size;
                  } catch (const std::bad_alloc&) {
                      err = Z_MEM_ERROR;
                      break;
                  }

                  M_stream.next_out = reinterpret_cast<Bytef*>(M_out_buffer.get() + (M_out_avail - extra));
                  M_stream.avail_out = static_cast<uInt>(extra);
              }

              err = deflate( &M_stream, Z_SYNC_FLUSH );

              if ( err != Z_OK )
              {
                  break;
              }
          }

          M_out_size = M_stream.total_out - bytes_out;

          // copy to the destination buffer
          if (M_out_size > 0) {
              dest.assign( M_out_buffer.get(), M_out_size );
          }

          M_out_size = 0;
          deflateReset( &M_stream );
          M_stream.next_out = reinterpret_cast<Bytef*>(M_out_buffer.get());
          M_stream.avail_out = static_cast<uInt>(M_out_avail);

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

    // Use smart pointer for automatic memory management
    std::unique_ptr<char[]> M_out_buffer;
    size_t M_out_size;
    size_t M_out_avail;
    bool M_initialized;
#endif
public:
    Impl()
#ifdef HAVE_LIBZ
        : M_out_buffer( nullptr )
        , M_out_size( 0 )
        , M_out_avail( 0 )
        , M_initialized( false )
#endif
      {
#ifdef HAVE_LIBZ
          M_stream.zalloc = Z_NULL;
          M_stream.zfree = Z_NULL;
          M_stream.opaque = nullptr;

          int result = inflateInit( &M_stream );
          if (result != Z_OK) {
              throw std::runtime_error("Failed to initialize inflate");
          }
          M_initialized = true;
#endif
      }

    ~Impl()
      {
#ifdef HAVE_LIBZ
          if (M_initialized) {
              inflateEnd( &M_stream );
          }
          // M_out_buffer will be automatically freed by unique_ptr
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
          
          if (!src_buf || src_size <= 0) {
              return Z_STREAM_ERROR;
          }
          
#ifdef HAVE_LIBZ
          if (!M_initialized) {
              return Z_STREAM_ERROR;
          }
          
          // allocate output buffer
          if ( !M_out_buffer )
          {
              // Start with 2x the input size for decompression
              M_out_avail = std::max(static_cast<size_t>(src_size * 2), 
                                   static_cast<size_t>(1024));
              
              try {
                  M_out_buffer = std::make_unique<char[]>(M_out_avail);
              } catch (const std::bad_alloc&) {
                  return Z_MEM_ERROR;
              }

              M_stream.next_out = reinterpret_cast<Bytef*>(M_out_buffer.get());
              M_stream.avail_out = static_cast<uInt>(M_out_avail);
          }

          M_stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(src_buf));
          M_stream.avail_in = static_cast<uInt>(src_size);

          const uLong bytes_out = M_stream.total_out;
          int err = Z_OK;
          
          // Process all input data
          while ( M_stream.avail_in > 0 )
          {
              if ( M_stream.avail_out == 0 )
              {
                  // Expand buffer by 50%
                  const size_t extra = M_out_avail / 2;
                  const size_t new_size = M_out_avail + extra;
                  
                  try {
                      auto new_buffer = std::make_unique<char[]>(new_size);
                      std::copy_n(M_out_buffer.get(), M_out_avail, new_buffer.get());
                      M_out_buffer = std::move(new_buffer);
                      M_out_avail = new_size;
                  } catch (const std::bad_alloc&) {
                      err = Z_MEM_ERROR;
                      break;
                  }

                  M_stream.next_out = reinterpret_cast<Bytef*>(M_out_buffer.get() + (M_out_avail - extra));
                  M_stream.avail_out = static_cast<uInt>(extra);
              }

              err = inflate( &M_stream, Z_SYNC_FLUSH );

              if ( err != Z_OK )
              {
                  break;
              }
          }

          M_out_size = M_stream.total_out - bytes_out;

          // copy to the destination buffer
          if (M_out_size > 0) {
              dest.assign( M_out_buffer.get(), M_out_size );
          }

          M_out_size = 0;
          inflateReset( &M_stream );
          M_stream.next_out = reinterpret_cast<Bytef*>(M_out_buffer.get());
          M_stream.avail_out = static_cast<uInt>(M_out_avail);

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
