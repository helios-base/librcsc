// -*-c++-*-

/*!
  \file gzcompressor.h
  \brief gzip compressor/decompressor Header File.
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

#ifndef RCSC_GZ_GZCOMPRESSOR_H
#define RCSC_GZ_GZCOMPRESSOR_H

#include <memory>
#include <string>

namespace rcsc {

/*!
  \class GZCompressor
  \brief compress message string
 */
class GZCompressor {
private:
    //! pimpl
    struct Impl;

    //! implementation object
    std::unique_ptr< Impl > M_impl;

public:

    /*!
      \brief construct with compression level
      \param level zlib compression level. [1,9]
     */
    explicit
    GZCompressor( const int level = 6 );

    /*!
      \brief destruct implementation
     */
    ~GZCompressor();

    /*!
      \brief set zlib compression level
      \param level zlib compression level. [1,9]
      \return result status of deflateParams
     */
    int setLevel( const int level );

    /*!
      \brief compress the src_buf and copy output buffer to std::string
      \param src_buf pointer to the source buffer
      \param src_size size of source buffer
      \param dest destination string variable
      \return status of compression
     */
    int compress( const char * src_buf,
                  const int src_size,
                  std::string & dest );

};


/////////////////////////////////////////////////////////////////////

/*!
  \class GZDecompressor
  \brief decompress message string
 */
class GZDecompressor {
private:
    //! pimpl
    struct Impl;

    //! implementation object
    std::unique_ptr< Impl > M_impl;

public:

    /*!
      \brief construct implementation
     */
    GZDecompressor();

    /*!
      \brief destruct implementation
     */
    ~GZDecompressor();

    /*!
      \brief decompress the src_buf and copy output buffer to std::string
      \param src_buf source buffer
      \param src_size size of source buffer
      \param dest destination string variable
      \return status of decompression
     */
    int decompress( const char * src_buf,
                    const int src_size,
                    std::string & dest );

};

}

#endif
