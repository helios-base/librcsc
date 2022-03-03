// -*-c++-*-

/*!
  \file formation_parser_v2.h
  \brief v2 formation parser Header File.
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

#ifndef RCSC_FORMATION_FORMATION_PARSER_V2_H
#define RCSC_FORMATION_FORMATION_PARSER_V2_H

#include <rcsc/formation/formation_parser.h>

namespace rcsc {

/*!
  \class FormationParserV2
  \brief v2 formation parser interface
*/
class FormationParserV2
    : public FormationParser {

public:

    /*!
      \brief default constructor
     */
    FormationParserV2();

    /*!
      \brief virtual default destructor
     */
    ~FormationParserV2() override
    { }

    /*!
      \brief parse the input stream
      \param is reference to the input stream to be parsed
      \return formation instance
     */
    FormationData::Ptr parse( std::istream & is ) override;

private:

    bool parseHeader( std::istream & is );
    bool parseRoles( std::istream & is,
                     FormationData::Ptr result );
    bool parseData( std::istream & is,
                    FormationData::Ptr result );
    bool parseDataHeader( std::istream & is,
                          int * data_size );
    bool parseOneData( std::istream & is,
                       const int index,
                       FormationData::Ptr result );

};

}

#endif
