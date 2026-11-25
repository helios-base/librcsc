// -*-c++-*-

/*!
  \file gzread_file.h
  \brief read a whole (gzip) file into memory Header File
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

#ifndef RCSC_GZ_GZREAD_FILE_H
#define RCSC_GZ_GZREAD_FILE_H

#include <string>

namespace rcsc {

/*!
  \brief read a whole file into memory. A gzip file is decompressed, and any other file is read as it is.
  \param path the path of the file
  \param data the destination. Its contents are replaced by the contents of the file.
  \retval true if the file is read
  \retval false if the file cannot be opened or read, or if it is a gzip file and librcsc is built without zlib
  (and libdeflate)

  A gzip file is decompressed with libdeflate if librcsc is built with it (a few times faster than zlib),
  and otherwise with zlib.

  As zlib's gzread() does, a gzip file that has more than one member is read to the end,
  and the bytes that are not gzip after a member are ignored.
  A gzip file that ends in the middle (e.g. the program that wrote it stopped abnormally)
  is read up to there, with a warning.

  This is faster than reading the file through rcsc::gzifstream.
  For example, the whole game log is read by this function, and then parsed by
  rcsc::rcg::Parser::create( const char *, std::size_t ) and
  rcsc::rcg::Parser::parse( const char *, std::size_t, Handler & ).
 */
bool gzread_file( const std::string & path,
                  std::string * data );

}

#endif
