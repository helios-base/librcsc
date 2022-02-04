// -*-c++-*-

/*!
  \file rcss_param_parser.h
  \brief rcssserver parameter message parser Header File
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

#ifndef RCSC_PARAM_RCSS_PARAM_PARSER_H
#define RCSC_PARAM_RCSS_PARAM_PARSER_H

#include <rcsc/param/param_parser.h>

#include <string>
#include <vector>
#include <utility>

namespace rcsc {

/*!
  \class RCSSParamParser
  \brief rcssserver parameter message parser
 */
class RCSSParamParser
    : public ParamParser {
private:
    typedef std::vector< std::pair< std::string, std::string > > StrPairVec;

    //! parameter type name (server_param, player_param ...)
    std::string M_param_name;

    //! container of string pair(parameter name and value)
    StrPairVec M_str_pairs;

    //! not used
    RCSSParamParser() = delete;
public:
    /*!
      \brief construct with original command line arguments
      \param msg raw server message string
     */
    explicit
    RCSSParamParser( const char * msg );

    /*!
      \brief analyze server message string and results are stored
      to parameter map
      \param param_map reference to the parameter container
      \return true if successfully parserd
     */
    bool parse( ParamMap & param_map ) override;

private:

    /*!
      \brief remove escaped quatation character from string.
      \param str string variable to be checked
      \return clened string
     */
    std::string cleanString( std::string str );

    /*!
      \brief lexical analyze and create string pair vector
      \param msg raw server message string

      This method is called only from constructor
     */
    bool init( const char * msg );
};

}

#endif
