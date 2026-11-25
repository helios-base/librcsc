// -*-c++-*-

/*!
  \file line_parser.h
  \brief parsing the lines of a game log in memory. Header File.

  This header file is used only inside librcsc (not installed).
  The design follows rcssmonitor (rcss/rcg/line_parser.h).
  See docs/plan/2026-11-24-02_rcg-parser-speedup.md 4.3 and 4.4.
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

#ifndef RCSC_RCG_LINE_PARSER_H
#define RCSC_RCG_LINE_PARSER_H

#include <cstddef>
#include <istream>
#include <string>
#include <vector>

namespace rcsc {
namespace rcg {

/*!
  \struct LineRef
  \brief a line of the log data in memory, without its line break
*/
struct LineRef {
    const char * begin_; //!< the first character
    std::size_t length_; //!< the number of characters
    int number_; //!< the line number, from 1
};

/*!
  \brief split the data into lines, as std::getline() does: '\n' ends a
  line (a '\r' before it stays in the line), and a '\n' at the end of the
  data does not make an empty line after it.
  \param data the data
  \param size the size of data
  \return the lines, which point into data (not copied)
*/
std::vector< LineRef > split_lines( const char * data,
                                    const std::size_t size );

/*!
  \brief read the rest of the stream into memory
  \param is the input stream
  \param data the destination. Its contents are replaced.
  \return false if the stream has a read error
*/
bool read_stream( std::istream & is,
                  std::string * data );

}
}

#endif
