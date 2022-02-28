// -*-c++-*-

/*!
  \file param_parser.h
  \brief abstract parameter parser Header File
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

#ifndef RCSC_PARAM_PARAM_PARSER_H
#define RCSC_PARAM_PARAM_PARSER_H

namespace rcsc {

class ParamMap;

/*!
  \class ParamParser
  \brief abstract parameter paraser class
 */
class ParamParser {
private:

    // non copyable
    ParamParser( const ParamParser & ) = delete;
    ParamParser & operator=( const ParamParser & ) = delete;

protected:

    /*!
      \brief protected access
     */
    ParamParser() = default;

public:

    /*!
      \brief virtual destructor
     */
    virtual
    ~ParamParser()
      { }

    /*!
      \brief interface method
      \param param_map reference to the parameter map object
      \return true if successfully parsed.
     */
    virtual
    bool parse( ParamMap & param_map ) = 0;

};

}

#endif
