// -*-c++-*-

/*!
  \file formation_parser_csv.h
  \brief csv formation parser Header File.
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

#ifndef RCSC_FORMATION_FORMATION_PARSER_CSV_H
#define RCSC_FORMATION_FORMATION_PARSER_CSV_H

#include <rcsc/formation/formation_parser.h>

namespace rcsc {

/*!
  \class FormationParserCSV
  \brief v1 formation parser interface
*/
class FormationParserCSV
    : public FormationParser {

public:

    /*!
      \brief default constructor
     */
    FormationParserCSV() = default;

    /*!
      \brief virtual default destructor
     */
    ~FormationParserCSV() override
    { }

    /*!
      \brief get the parser name
      \return parser name
     */
    virtual
    std::string name() const override
    {
        return "csv";
    }

protected:

    /*!
      \brief parse the input stream
      \param is reference to the input stream to be parsed
      \return formation instance
     */
    Formation::Ptr parseImpl( std::istream & is ) override;

private:
    std::string parseMethodName( std::istream & is );
    bool parseRoleNumbers( std::istream & is );
    bool parseRoleNames( std::istream & is,
                         Formation::Ptr result );
    bool parseRoleTypes( std::istream & is,
                         Formation::Ptr result );
    bool parsePositionPairs( std::istream & is,
                             Formation::Ptr result );
    bool parseMarkerFlags( std::istream & is );
    bool parseSetplayMarkerFlags( std::istream & is );
    bool parseData( std::istream & is,
                    Formation::Ptr result );

    bool parseStaticPositions( std::istream & is,
                               Formation::Ptr result );
};

}

#endif
