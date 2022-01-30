// -*-c++-*-

/*!
  \file freeform_message.h
  \brief freeform message builder Header File
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

#ifndef RCSC_COMMON_FREEFORM_MESSAGE_H
#define RCSC_COMMON_FREEFORM_MESSAGE_H

#include <memory>
#include <string>
#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class FreeformMessage
  \brief abstract freeform message.
*/
class FreeformMessage {
public:

    //! pointer type
    typedef std::shared_ptr< const FreeformMessage > Ptr;

private:

    const std::string M_type;

    // not used
    FreeformMessage() = delete;
    FreeformMessage( const FreeformMessage & ) = delete;
    FreeformMessage & operator=( const FreeformMessage & ) = delete;

protected:

    /*!
      \brief protected constructer.
    */
    FreeformMessage( const std::string & t )
        : M_type( t )
      { }

public:

    /*!
      \brief virtual destructor.
    */
    virtual
    ~FreeformMessage() = default;

    /*!
      \brief pure virtual method. get the message type strring.
      \return message type string
     */
    const std::string & type() const
      {
          return M_type;
      }

    /*!
      \brief pure virtual method. get the length of this message
      \return the length of encoded message
    */
    virtual
    int length() const = 0;

    /*!
      \brief pure virtual method. append the freeform message to 'to'.
      \param to reference to the result message string.
      \return result of message appending.
    */
    virtual
    bool append( std::string & to ) const = 0;

    /*!
      \brief pure virtual method. output a debug message.
      \param os reference to the output stream.
      \return reference to the output stream.
     */
    virtual
    std::ostream & printDebug( std::ostream & os ) const = 0;

};

}

#endif
