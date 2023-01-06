// -*-c++-*-

/*!
  \file formation_parser_v3.h
  \brief v3 formation parser Header File.
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

#ifndef RCSC_FORMATION_FORMATION_PARSER_V3_H
#define RCSC_FORMATION_FORMATION_PARSER_V3_H

#include <rcsc/formation/formation_parser.h>
#include <rcsc/formation/formation_data.h>

namespace rcsc {

/*!
  \class FormationParserV3
  \brief v3 formation parser interface
*/
class FormationParserV3
    : public FormationParser {

public:

    /*!
      \brief default constructor
     */
    FormationParserV3() = default;

    /*!
      \brief virtual default destructor
     */
    ~FormationParserV3() override
    { }

    /*!
      \brief get the parser name
      \return parser name
     */
    virtual
    std::string name() const override
    {
        return "v3";
    }

protected:

    /*!
      \brief parse the input stream
      \param is reference to the input stream to be parsed
      \return formation instance
     */
    Formation::Ptr parseImpl( std::istream & is ) override;

private:

    std::string parseHeader( std::istream & is );
    bool parseRoles( std::istream & is,
                     Formation::Ptr result );
    bool parseBeginRolesTag( std::istream & is );
    bool parseEndRolesTag( std::istream & is );
    bool parseData( std::istream & is,
                    FormationData * formation_data );
    bool parseDataHeader( std::istream & is,
                          int * data_size );
    bool parseOneData( std::istream & is,
                       const int index,
                       FormationData * formation_data );
    bool parseEnd( std::istream & is );

};

}

#endif
