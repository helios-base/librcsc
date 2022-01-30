// -*-c++-*-

/*!
  \file freeform_message_parser.h
  \brief coach's freeform message parser Header File
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

#ifndef RCSC_COMMON_FREEFORM_MESSAGE_PARSER_H
#define RCSC_COMMON_FREEFORM_MESSAGE_PARSER_H

#include <memory>
#include <string>

namespace rcsc {

/*!
  \class FreeformMessageParser
  \brief abstract freeform message parser
 */
class FreeformMessageParser {
public:

    //! pointer type
    typedef std::shared_ptr< FreeformMessageParser > Ptr;

private:

    const std::string M_type;

    // not used
    FreeformMessageParser() = delete;
    FreeformMessageParser( const FreeformMessageParser & ) = delete;
    FreeformMessageParser & operator=( const FreeformMessageParser & ) = delete;

protected:

    /*!
      \brief protected constructor.
     */
    FreeformMessageParser( const std::string & t )
        : M_type( t )
      { }

public:

    /*!
      \brief virtual destructor.
     */
    virtual
    ~FreeformMessageParser() = default;

    /*!
      \brief get the message type string.
      \return the message type string.
     */
    const std::string & type() const
      {
          return M_type;
      }

    /*!
      \brief analyzes freeform message.
      \retval the size of read bytes if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    virtual
    int parse( const char * msg ) = 0;

};

}

#endif
