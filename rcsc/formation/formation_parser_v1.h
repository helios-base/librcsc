// -*-c++-*-

/*!
  \file formation_parser_v1.h
  \brief v1 formation parser Header File.
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

#ifndef RCSC_FORMATION_FORMATION_PARSER_V1_H
#define RCSC_FORMATION_FORMATION_PARSER_V1_H

#include <rcsc/formation/formation_parser.h>

namespace rcsc {

/*!
  \class FormationParserV1
  \brief v1 formation parser interface
*/
class FormationParserV1
    : public FormationParser {

public:

    /*!
      \brief default constructor
     */
    FormationParserV1() = default;

    /*!
      \brief virtual default destructor
     */
    ~FormationParserV1() override
    { }

    /*!
      \brief get the parser name
      \return parser name
     */
    virtual
    std::string name() const override
    {
        return "v1";
    }

    /*!
      \brief parse the input stream
      \param is reference to the input stream to be parsed
      \return formation instance
     */
    Formation::Ptr parse( std::istream & is ) override;

private:
    std::string parseHeader( std::istream & is );
    bool parseRoles( std::istream & is,
                     Formation::Ptr result );
    bool parseData( std::istream & is,
                    Formation::Ptr result );

};

}

#endif
