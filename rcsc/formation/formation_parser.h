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

#include <memory>
#include <string>
#include <iosfwd>

namespace rcsc {

/*!
  \class FormationParser
  \brief abstarct formation data parser interface
*/
class FormationParser {
public:

    typedef std::shared_ptr< FormationParser > Ptr; //!< smart pointer type

private:

    FormationParser( FormationParser & ) = delete;
    FormationParser & operator=( FormationParser & ) = delete;

protected:

    /*!
      \brief default constructor
     */
    FormationParser() = default;

public:

    /*!
      \brief virtual default destructor
     */
    virtual
    ~FormationParser() = default;

    /*!
      \brief get the parser name
      \return parser name
     */
    virtual
    std::string name() const = 0;

protected:

    /*!
      \brief parse the input stream
      \param is reference to the input stream to be parsed
      \return formation instance
     */
    virtual
    Formation::Ptr parseImpl( std::istream & is ) = 0;

    /*!
      \brief check the consistency of role names
      \return true if success
     */
    bool checkRoleNames( const Formation::ConstPtr ptr );

    /*!
      \brief complete the position pairs read from the file, and check their consistency
      \param ptr the formation whose roles have been read
      \return true if success

      In the formation files, -1 means that the paired player refers to this player.
      Formation::setPositionPair() also sets the number of the referring player to the referred player,
      but if the referred player is read after that, its -1 overwrites the number.
      Such -1 is replaced with the number of the referring player, so that the paired players
      refer to each other. Then the pairs are checked by Formation::setPositionPairs().
     */
    bool checkPositionPair( const Formation::Ptr ptr );


private:

    /*!
      \brief create formation parser instance according to the header data
      \param filepath the path string of the input file
      \return formation parser instance
     */
     static FormationParser::Ptr create( const std::string & filepath );

public:

    /*!
      \brief parse the given file
      \param filepath the file path to be parsed
      \return formation instance
     */
    static Formation::Ptr parse( const std::string & filepath );

};

}

#endif
