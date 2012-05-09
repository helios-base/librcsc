// -*-c++-*-

/*!
  \file parser_v5.h
  \brief rcg v5 parser Header File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

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

#ifndef RCSC_RCG_PARSER_V5_H
#define RCSC_RCG_PARSER_V5_H

#include <rcsc/rcg/parser_v4.h>

namespace rcsc {
namespace rcg {

/*!
  \class ParserV5
  \brief rcg v5 parser class
 */
class ParserV5
    : public ParserV4 {
public:

    /*!
      \brief get supported rcg version
      \return version number
     */
    int version() const
      {
          return REC_VERSION_5;
      }

    /*!
      \brief parse input stream
      \param is reference to the imput stream (usually ifstream/gzifstream).
      \param handler reference to the rcg data handler.
      \retval true, if successfuly parsed.
      \retval false, if incorrect format is detected.
    */
    virtual
    bool parse( std::istream & is,
                Handler & handler ) const;

private:

    /*!
      \brief parse SHOW_MODE inof, actually short_showinfo_t2
      \param n_line the number of total read line
      \param line the data string
      \param handler reference to the data handler object
      \retval true if successfully parsed.
      \retval false if failed to parse.
    */
    virtual
    bool parseShow( const int n_line,
                    const std::string & line,
                    Handler & handler ) const;

};

} // end of namespace
} // end of namespace

#endif
