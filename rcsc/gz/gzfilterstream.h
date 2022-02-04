// -*-c++-*-

/*!
  \file gzfilterstream.h
  \brief gzip filtering stream Header File.
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

#ifndef RCSC_GZ_GZFILTERSTREAM_H
#define RCSC_GZ_GZFILTERSTREAM_H

#include <boost/scoped_ptr.hpp>
#include <iostream>

namespace rcsc {

/////////////////////////////////////////////////////////////////////

/*!
  \class gzfilterstreambuf
  \brief gzip filtering stream buffer class.
*/
class gzfilterstreambuf
    : public std::streambuf {
public:
    /*!
      \brief typical compression level enumeration

      The list of compresion level defined in zlib.h
      - Z_NO_COMPRESSION
      - Z_BEST_SPEED
      - Z_BEST_COMPRESSION
      - Z_DEFAULT_COMPRESSION
    */
    enum CompressionLevel {
        DEFAULT_COMPRESSION = 6,
        NO_COMPRESSION = 0,
        BEST_SPEED = 1,
        BEST_COMPRESSION = 9,
    };

    /*!
      \brief flush type enumeration.
     */
    enum FlushType {
        NO_FLUSH = 0,
        PARTIAL_FLUSH  = 1, // will be removed, use SYNC_FLUSH instead
        SYNC_FLUSH = 2,
        FULL_FLUSH = 3,
        FINISH = 4
    };

private:
    //! filtered stream buffer
    std::streambuf & M_strmbuf;
    //! used for writing to M_strmbuf
    std::ostream * M_output_stream;
    //! used for reading from M_strmbuf
    std::istream * M_input_stream;
    //! size of the following buffers
    std::streamsize M_buf_size;
    //! used to read compressed data from M_strmbuf
    char_type * M_read_buf;
    //! used to buffer uncompressed input to this stream
    char_type * M_input_buf;
    //! used to buffer uncompressed output from this stream
    char_type * M_output_buf;
    //! used to write compressed data to M_strmbuf
    char_type * M_write_buf;

    //! Pimpl ideom.
    struct Impl;

    //! Pimpl ideom.
    std::unique_ptr< Impl > M_impl;
    /*!
      current level of compression/decompression.
      -1 means that data is handled without modification.
    */
    int M_level;

    //! not used
    gzfilterstreambuf( const gzfilterstreambuf & ) = delete;
    //! not used
    gzfilterstreambuf & operator=( const gzfilterstreambuf & ) = delete;

public:

    /*!
      \brief constructor with another stream buffer
      \param strm filtered stream buffer.
      \param level gzip compression level
      \param buf_size allocated buffer size (default: 8192)

      Default constructor creates an internal file buffer using unique_ptr.
      This buffer is deleted automatically.
    */
    explicit
    gzfilterstreambuf( std::streambuf & strm,
                       int level = DEFAULT_COMPRESSION,
                       std::size_t buf_size = 8192 );

    /*!
      \brief destructor

      Destructor flushes buffered data and releases all allocated buffers.
    */
    ~gzfilterstreambuf();

    /*!
      \brief change complession level
      \param level new compression level(0-9)
      \return true if level is validated value, else false.

      Compression strategy is set to Z_DEFAULT_STRATEGY automatically.
    */
    bool setLevel( const int level );

protected:

    /*!
      \brief write data to buffer and/or destination
      \param flush_type zlib flush type parameter. see deflate in zlib.h.
      \return true if successfully written, else false.
    */
    bool writeData( int flush_type = NO_FLUSH );

    /*!
      \brief read data from destination.
      \param dest buffer to record the data.
      \param dest_size reference to the buffer size variable
      \return size of the read data
    */
    int readData( char * dest,
                  int & dest_size );

    /*!
      \brief flush current internal buffer.

      this method is overrided method and called from close(),
      sync() and overflow().
    */
    virtual
    int_type overflow( int_type c );

    /*!
      \brief syncronize data to output devide

      this method is overrided method
     */
    virtual
    int sync();

    /*!
      \brief read data from input device

      this method is overrided method and supposed to read some bytes from
      the input device to internal buffer.
    */
    virtual
    int_type underflow();
};

/////////////////////////////////////////////////////////////////////

/*!
  \class gzfilterstream
  \brief gzip filtering stream class.
 */
class gzfilterstream
    : public std::iostream {
private:
    //! buffer for this steram
    gzfilterstreambuf M_filter_buf;

    //! not used
    gzfilterstream( const gzfilterstream & ) = delete;
    //! not used
    gzfilterstream & operator=( const gzfilterstream & ) = delete;

public:
    /*!
      \brief constructor with another stream buffer
      \param strmbuf another stream buffer to be filtered
      \param level gzip compression level
      \param buf_size allocated size of internal buffer
    */
    explicit
    gzfilterstream( std::streambuf & strmbuf,
                    int level = gzfilterstreambuf::DEFAULT_COMPRESSION,
                    std::size_t buf_size = 8192 );

    /*!
      \brief constructor with another stream
      \param strm another stream to be filtered
      \param level gzip compression level
      \param buf_size allocated size of internal buffer
     */
    explicit
    gzfilterstream( std::iostream & strm,
                    int level = gzfilterstreambuf::DEFAULT_COMPRESSION,
                    std::size_t buf_size = 8192 );

    /*!
      \brief change complession level
      \param level new compression level(0-9)
      \return true if level is validated value, else false.

      Compression strategy is set to Z_DEFAULT_STRATEGY automatically.
    */
    bool setLevel( const int level )
      {
          return M_filter_buf.setLevel( level );
      }

};

/////////////////////////////////////////////////////////////////////

/*!
  \class gzifilterstream
  \brief gzip filtering input stream class.
*/
class gzifilterstream
    : public std::istream {
private:
    //! buffer for this steram
    gzfilterstreambuf M_filter_buf;

    //! not used
    gzifilterstream( const gzifilterstream & ) = delete;
    //! not used
    gzifilterstream & operator=( const gzifilterstream & ) = delete;

public:
    /*!
      \brief constructor with another stream buffer
      \param src another stream buffer to be filtered
      \param level gzip compression level
      \param buf_size allocated size of internal buffer
     */
    explicit
    gzifilterstream( std::streambuf & src,
                     int level = gzfilterstreambuf::DEFAULT_COMPRESSION,
                     std::size_t buf_size = 8192 );

    /*!
      \brief constructor with another stream
      \param src another stream to be filtered
      \param level gzip compression level
      \param buf_size allocated size of internal buffer
    */
    explicit
    gzifilterstream( std::istream & src,
                     int level = gzfilterstreambuf::DEFAULT_COMPRESSION,
                     std::size_t buf_size = 8192 );

    /*!
      \brief change complession level
      \param level new compression level(0-9)
      \return true if level is validated value, else false.

      Compression strategy is set to Z_DEFAULT_STRATEGY automatically.
    */
    bool setLevel( const int level )
      {
          return M_filter_buf.setLevel( level );
      }

};

/////////////////////////////////////////////////////////////////////

/*!
  \class gzofilterstream
  \brief gzip filtering output stream class.
*/
class gzofilterstream
    : public std::ostream {
private:
    //! buffer for this steram
    gzfilterstreambuf M_filter_buf;

    //! not used.
    gzofilterstream( const gzofilterstream & ) = delete;
    //! not used.
    gzofilterstream & operator=( const gzofilterstream & ) = delete;
public:

    /*!
      \brief constructor with another stream buffer
      \param dest another stream buffer to be filtered
      \param level gzip compression level
      \param buf_size allocated size of internal buffer
     */
    explicit
    gzofilterstream( std::streambuf & dest,
                     int level = gzfilterstreambuf::DEFAULT_COMPRESSION,
                     std::size_t buf_size = 8192 );

    /*!
      \brief constructor with another stream
      \param dest another stream to be filtered
      \param level gzip compression level
      \param buf_size allocated size of internal buffer
    */
    explicit
    gzofilterstream( std::ostream & dest,
                     int level = gzfilterstreambuf::DEFAULT_COMPRESSION,
                     std::size_t buf_size = 8192 );

    /*!
      \brief change complession level
      \param level new compression level(0-9)
      \return true if level is validated value, else false.

      Compression strategy is set to Z_DEFAULT_STRATEGY automatically.
    */
    bool setLevel( const int level )
      {
          return M_filter_buf.setLevel( level );
      }

};

} // end namespace

#endif
