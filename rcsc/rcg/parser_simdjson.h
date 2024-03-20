// -*-c++-*-

/*!
  \file parser_simdjson.h
  \brief rcg v6 (json) parser Header File.
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

#ifndef RCSC_RCG_PARSER_SIMDJSON_H
#define RCSC_RCG_PARSER_SIMDJSON_H

#include <rcsc/rcg/parser.h>
#include <rcsc/rcg/types.h>

#include <string>

namespace rcsc {
namespace rcg {


/*!
  \class ParserSimdJSON
  \brief JSON rcg parser class
 */
class ParserSimdJSON
    : public Parser {
public:

    /*!
      \brief create the Impl instance
    */
    ParserSimdJSON();

    /*!
      \brief get supported rcg version
      \return version number
     */
    virtual
    int version() const override
    {
        return REC_VERSION_JSON;
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
                Handler & handler ) const override;

    /*!
      \brief assume to parse one monitor packet.
      \param input the data string
      \param handler reference to the rcg data handler.
      \retval true, if successfuly parsed.
      \retval false, if incorrect format is detected.

      First, check the type of data mode.
      Second, call each data item parsing method.
    */
    bool parseData( const std::string & input,
                    Handler & handler ) const;


};

}
}

#endif
