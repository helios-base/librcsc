// -*-c++-*-

/*!
  \file gzfilterstream.cpp
  \brief gzip filtering stream Source File.
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

#include "gzfilterstream.h"

#ifdef HAVE_LIBZ
#include <zlib.h>
#endif

namespace rcsc {

/*!
  \brief implementation of gzfilterstreambuf
 */
struct gzfilterstreambuf::Impl {
#ifdef HAVE_LIBZ
    z_stream * comp_stream_; //!< compressin buffer
    z_stream * decomp_stream_; //!< decompression buffer
#endif

    /*!
      \brief default constructo
     */
    Impl()
#ifdef HAVE_LIBZ
        : comp_stream_( nullptr ),
          decomp_stream_( nullptr )
#endif
      { }
};

/*-------------------------------------------------------------------*/
/*!

*/
gzfilterstreambuf::gzfilterstreambuf( std::streambuf & strm,
                                      int level,
                                      std::size_t bufsize )
    : std::streambuf(),
      M_strmbuf( strm ),
      M_output_stream( nullptr ),
      M_input_stream( nullptr ),
      M_buf_size( bufsize ),
      M_read_buf( nullptr ),
      M_input_buf( nullptr ),
      M_output_buf( nullptr ),
      M_write_buf( nullptr ),
      M_impl( new Impl() )
#ifdef HAVE_LIBZ
    , M_level( level )
#else
    , M_level( NO_COMPRESSION )
#endif
{

}

/*-------------------------------------------------------------------*/
/*!

*/
gzfilterstreambuf::~gzfilterstreambuf()
{
#ifdef HAVE_LIBZ
    writeData( Z_FINISH );
#else
    writeData();
#endif

#ifdef HAVE_LIBZ
    if ( M_impl->comp_stream_ )
    {
        deflateEnd( M_impl->comp_stream_ );
    }

    if ( M_impl->decomp_stream_ )
    {
        deflateEnd( M_impl->decomp_stream_ );
    }
#endif

    delete M_output_stream;
    M_output_stream = nullptr;

    delete M_input_stream;
    M_input_stream = nullptr;

    delete [] M_read_buf;
    M_read_buf = nullptr;

    delete [] M_input_buf;
    M_input_buf = nullptr;

    delete [] M_output_buf;
    M_output_buf = nullptr;

    delete [] M_write_buf;
    M_write_buf = nullptr;

#ifdef HAVE_LIBZ
    delete M_impl->comp_stream_;
    M_impl->comp_stream_ = nullptr;

    delete M_impl->decomp_stream_;
    M_impl->decomp_stream_ = nullptr;
#endif

    this->setg( nullptr, nullptr, nullptr );
    this->setp( nullptr, nullptr );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
gzfilterstreambuf::setLevel( const int level )
{
    bool ret = false;
#ifdef HAVE_LIBZ
    if ( level == DEFAULT_COMPRESSION
         || ( NO_COMPRESSION <= level
              && level <= BEST_COMPRESSION )
         )
    {
        //std::cerr << "set compression level = " << level << std::endl;
        // make sure there is room for the deflate to flush
        this->sync();
        deflateParams( M_impl->comp_stream_,
                       level, Z_DEFAULT_STRATEGY );
        // write data flushed by deflateParams
        this->sync();
        M_level = level;
        ret = true;
    }
    // Without zlib, this class cannot handle any compression level.
#endif
    return ret;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
gzfilterstreambuf::writeData( int flush_type )
{
    // size of data to write
    int size = ( pptr() - pbase() ) * sizeof( char_type );
    if ( size == 0 )
    {
        return true;
    }

    if ( M_output_stream == nullptr )
    {
        //std::cerr << "create stream" << std::endl;
        M_output_stream = new std::ostream( &M_strmbuf );
    }

#ifdef HAVE_LIBZ
    if ( M_level <= NO_COMPRESSION
         || BEST_COMPRESSION < M_level )
    {
        M_output_stream->write( M_output_buf, size );
    }
    else
    {
        if ( M_impl->comp_stream_ == nullptr )
        {
            M_impl->comp_stream_ = new z_stream;
            M_impl->comp_stream_->zalloc = Z_NULL;
            M_impl->comp_stream_->zfree = Z_NULL;
            M_impl->comp_stream_->opaque = nullptr;
            M_impl->comp_stream_->avail_in = 0;
            M_impl->comp_stream_->next_in = 0;
            M_impl->comp_stream_->next_out = 0;
            M_impl->comp_stream_->avail_out = 0;

            if ( deflateInit( M_impl->comp_stream_, M_level ) != Z_OK )
            {
                return false;
            }

            if ( M_write_buf == nullptr )
            {
                //std::cerr << "create write buf" << std::endl;
                M_write_buf = new char[ M_buf_size ];
            }

            M_impl->comp_stream_->next_out = (Bytef*)M_write_buf;
            M_impl->comp_stream_->avail_out = M_buf_size;
        }

        M_impl->comp_stream_->next_in = (Bytef*)M_output_buf;
        M_impl->comp_stream_->avail_in = size;

        do
        {
            int bytes_out = - M_impl->comp_stream_->total_out;
            int err = deflate( M_impl->comp_stream_, flush_type );

            if ( err != Z_OK && err != Z_STREAM_END )
            {
                //std::cerr << "error deflating ["
                //          << M_impl->comp_stream_->msg << "]"
                //          << std::endl;
                return false;
            }

            bytes_out += M_impl->comp_stream_->total_out;
            //std::cerr << "compress loop size = " << bytes_out << std::endl;

            M_output_stream->write( M_write_buf, bytes_out );
            M_impl->comp_stream_->next_out = (Bytef*)M_write_buf;
            M_impl->comp_stream_->avail_out = M_buf_size;
        }
        while ( M_impl->comp_stream_->avail_in != 0 );
        // we want to keep writing until all the data has been
        // consumed by the compression stream

        deflateReset( M_impl->comp_stream_ );
    }
#else
    M_output_stream->write( M_output_buf, size );
#endif

    if ( flush_type != NO_FLUSH ) // == Z_NO_FLUSH
    {
        // flush the underlying stream if a flush has been used
        M_output_stream->flush();
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
gzfilterstreambuf::readData( char * dest,
                             int & dest_size )
{
    //std::cerr << "readData" << std::endl;
    if ( M_input_stream == nullptr )
    {
        M_input_stream = new std::istream( &M_strmbuf );
    }

    int ret = 0;

    if ( ! M_input_stream->good() )
    {
        M_input_stream->setstate( std::istream::failbit );
        return ret;
    }

    // get number of characters available in input buffer
    int readable_bytes = M_strmbuf.in_avail();

    if ( readable_bytes < 0 )
    {
        // no more readable data
        M_input_stream->setstate( std::istream::eofbit );
    }
    else if ( readable_bytes == 0 )
    {
        // read new data from stream
        ret = M_input_stream->read( dest, 1 ).gcount();
        // set new size
        readable_bytes = M_strmbuf.in_avail();
        if ( readable_bytes > dest_size - 1 )
        {
            // readable size is larger than buffer size.
            // adjust readable data size.
            readable_bytes = dest_size - 1;
        }
        ret += M_input_stream->read( dest + 1, readable_bytes ).gcount();
        dest_size = readable_bytes + 1;
    }
    else
    {
        if ( readable_bytes > dest_size )
        {
            // readable size is larger than buffer size.
            // adjust readable data size.
            readable_bytes = dest_size;
        }
        ret = M_input_stream->read( dest, readable_bytes ).gcount();
        dest_size = readable_bytes;
    }

    return ret;
}

/*-------------------------------------------------------------------*/
/*!

*/
gzfilterstreambuf::int_type
gzfilterstreambuf::overflow( int_type c )
{
    // if the buffer was not already allocated nor set by user,
    // do it just now
    if ( pptr() == nullptr )
    {
        M_output_buf = new char_type[ M_buf_size ];
    }
    else
    {
        if ( ! writeData() )
        {
            return traits_type::eof();
        }
    }

    // reset putting buffer pointer
    this->setp( M_output_buf, M_output_buf + M_buf_size );

    if ( c != traits_type::eof() )
    {
        sputc( c );
    }

    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
gzfilterstreambuf::sync()
{
    if ( pptr() != nullptr )
    {
        // just flush the put area
        if ( ! writeData( SYNC_FLUSH ) ) // == Z_SYNC_FLUSH
        {
            return -1;
        }

        // reset putting buffer pointer
        this->setp( M_output_buf, M_output_buf + M_buf_size );
    }

    return 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
gzfilterstreambuf::int_type
gzfilterstreambuf::underflow()
{
    static int s_remained = 0; // number of bytes remaining in M_output_buffer
    static char_type s_remained_char; // remained character in M_output_buffer

    //std::cerr << "underflow" << std::endl;

    // if the buffer was not already allocated nor set by user,
    // do it just now
    if ( gptr() == nullptr )
    {
        M_input_buf = new char_type[ M_buf_size ];
        this->setg( M_input_buf, M_input_buf, M_input_buf );
    }

    int readn = 0;
#ifdef HAVE_LIBZ
    if ( M_level <= NO_COMPRESSION
         || BEST_COMPRESSION < M_level )
    {
#endif
        if ( s_remained != 0 )
        {
            M_input_buf[0] = s_remained_char;
        }

        readn = M_buf_size * sizeof( char_type ) - s_remained;
        readData( M_input_buf + s_remained, readn );
        int totalbytes = readn + s_remained;
        this->setg( M_input_buf, M_input_buf,
                    M_input_buf + totalbytes / sizeof( char_type ) );

        s_remained = totalbytes % sizeof( char_type );

        if ( s_remained != 0 )
        {
            s_remained_char = M_input_buf[ totalbytes / sizeof(char_type) ];
        }
#ifdef HAVE_LIBZ
    }
    else
    {
        if ( M_read_buf == nullptr )
        {
            M_read_buf = new char_type[ M_buf_size ];
        }

        if ( s_remained != 0 )
        {
            M_input_buf[ 0 ] = s_remained_char;
        }

        if ( M_impl->decomp_stream_ == nullptr )
        {
            M_impl->decomp_stream_ = new z_stream;
            M_impl->decomp_stream_->zalloc = Z_NULL;
            M_impl->decomp_stream_->zfree = Z_NULL;
            M_impl->decomp_stream_->opaque = nullptr;
            M_impl->decomp_stream_->avail_in = 0;
            M_impl->decomp_stream_->next_in = 0;
            M_impl->decomp_stream_->avail_out = 0;
            M_impl->decomp_stream_->next_out = 0;

            if ( inflateInit( M_impl->decomp_stream_ ) != Z_OK )
            {
                return false;
            }
        }

        M_impl->decomp_stream_->next_out = (Bytef*)( M_input_buf
                                                     + s_remained );
        M_impl->decomp_stream_->avail_out = ( M_buf_size
                                              * sizeof(char_type)
                                              - s_remained );
        do
        {
            if ( M_impl->decomp_stream_->avail_in == 0 )
            {
                int bytes_read = M_buf_size;
                readData( M_read_buf, bytes_read );

                M_impl->decomp_stream_->next_in = (Bytef*)M_read_buf;
                M_impl->decomp_stream_->avail_in = bytes_read;
            }

            readn -= M_impl->decomp_stream_->total_out;

            if ( inflate( M_impl->decomp_stream_, Z_NO_FLUSH ) != Z_OK )
            {
                //cerr << decomp_stream_->msg << endl;
                return traits_type::eof();
            }
            readn += M_impl->decomp_stream_->total_out;

        }
        while( readn == 0 );

        int totalbytes = readn + s_remained;
        this->setg( M_input_buf, M_input_buf,
                    M_input_buf + totalbytes / sizeof( char_type ) );

        s_remained = totalbytes % sizeof( char_type );

        if ( s_remained != 0 )
        {
            s_remained_char = M_input_buf[ totalbytes / sizeof( char_type ) ];
        }

        inflateReset( M_impl->decomp_stream_ );
    }
#endif
    // if (readn == 0 && remained_ != 0)
    // error - there is not enough bytes for completing
    // the last character before the end of the stream
    // - this can mean error on the remote end
    if ( readn == 0 )
    {
        return traits_type::eof();
    }

    return sgetc();
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
gzfilterstream::gzfilterstream( std::streambuf & strmbuf,
                                int level,
                                std::size_t buf_size )
    : std::iostream( nullptr ),
      M_filter_buf( strmbuf, level, buf_size )
{
    // std::basic_ios::init( basic_streambuf<charT,traits>* sb );
    this->init( &M_filter_buf );
}

/*-------------------------------------------------------------------*/
/*!

*/
gzfilterstream::gzfilterstream( std::iostream & strm,
                                int level,
                                std::size_t buf_size )
    : std::iostream( nullptr ),
      M_filter_buf( *(strm.rdbuf()), level, buf_size )

{
    // std::basic_ios::init( basic_streambuf<charT,traits>* sb );
    this->init( &M_filter_buf );
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
gzifilterstream::gzifilterstream( std::streambuf & src,
                                  int level,
                                  std::size_t buf_size )
    : std::istream( nullptr ),
      M_filter_buf( src, level, buf_size )
{
    // std::basic_ios::init( basic_streambuf<charT,traits>* sb );
    this->init( &M_filter_buf );
}

/*-------------------------------------------------------------------*/
/*!

*/
gzifilterstream::gzifilterstream( std::istream & src,
                                  int level,
                                  std::size_t buf_size )
    : std::istream( nullptr ),
      M_filter_buf( *(src.rdbuf()), level, buf_size )
{
    // std::basic_ios::init( basic_streambuf<charT,traits>* sb );
    this->init( &M_filter_buf );
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
gzofilterstream::gzofilterstream( std::streambuf & dest,
                                  int level,
                                  std::size_t buf_size )
    : std::ostream( nullptr ),
      M_filter_buf( dest, level, buf_size )
{
    // std::basic_ios::init( basic_streambuf<charT,traits>* sb );
    this->init( &M_filter_buf );
}

/*-------------------------------------------------------------------*/
/*!

*/
gzofilterstream::gzofilterstream( std::ostream & dest,
                                  int level,
                                  std::size_t buf_size )
    : std::ostream( nullptr ),
      M_filter_buf( *(dest.rdbuf()), level, buf_size )

{
    // std::basic_ios::init( basic_streambuf<charT,traits>* sb );
    this->init( &M_filter_buf );
}

} // end namespace
