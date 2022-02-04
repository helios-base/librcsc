// -*-c++-*-

/*!
  \file conf_file_parser.h
  \brief config file parser Header File
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

#ifndef RCSC_CONFIG_FILE_PARSER_H
#define RCSC_CONFIG_FILE_PARSER_H

#include <rcsc/param/param_parser.h>

#include <string>

namespace rcsc {

/*!
  \class ConfFileParser
  \brief config file parser
 */
class ConfFileParser
    : public ParamParser {
private:
    //! file path string
    const std::string M_file_path;

    //! delimiter character set
    const std::string M_delimiters;

    //! parameter's realm
    const std::string M_realm;

    //! not used
    ConfFileParser() = delete;
public:
    /*!
      \brief construct with file path and delimiters
      \param file_path path string to the config file to be parsed
      \param delim delimiter character set
      \param realm realm string
     */
    explicit
    ConfFileParser( const std::string & file_path,
                    const std::string & delim = ":=",
                    const std::string & realm = "" );

    /*!
      \brief analyze file content and results are stored to parameter map
      \param param_map reference to the parameter container
      \return true if successfully parserd
     */
    bool parse( ParamMap & param_map ) override;
};

}

#endif
