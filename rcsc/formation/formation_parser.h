// -*-c++-*-

/*!
  \file formation_parser.h
  \brief abstract formation parser Header File.
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

#ifndef RCSC_FORMATION_FORMATION_PARSER_H
#define RCSC_FORMATION_FORMATION_PARSER_H

#include <rcsc/formation/formation.h>

#include <string>
#include <iosfwd>

namespace rcsc {

/*!
  \class FormationParser
  \brief abstarct formation parser interface
*/
class FormationParser {
private:

    FormationParser( FormationParser & ) = delete;
    FormationParser & operator=( FormationParser & ) = delete;

protected:

public:

    /*!
      \brief default constructor
     */
    FormationParser() = default;

    /*!
      \brief virtual default destructor
     */
    virtual
    ~FormationParser() = default;

    /*!
      \brief parse the given file
      \param filepath the file path to be parsed
      \return formation instance
     */
    Formation::Ptr parse( const std::string & filepath );

    /*!
      \brief parse the input stream
      \param is reference to the input stream to be parsed
      \return formation instance
     */
    virtual
    Formation::Ptr parse( std::istream & is ) = 0;


};

}

#endif
