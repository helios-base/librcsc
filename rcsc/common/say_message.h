// -*-c++-*-

/*!
  \file say_message.h
  \brief abstract player's say message Header File
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

#ifndef RCSC_COMMON_SAY_MESSAGE_H
#define RCSC_COMMON_SAY_MESSAGE_H

#include <memory>
#include <string>
#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class SayMessage
  \brief abstract player's say message
*/
class SayMessage {
public:

    //! definition of the smart pointer type.
    typedef std::shared_ptr< const SayMessage > Ptr;

private:

    // not used
    SayMessage( const SayMessage & ) = delete;
    SayMessage & operator=( const SayMessage & ) = delete;

protected:

    /*!
      \brief protected constructer
    */
    SayMessage() = default;

public:

    /*!
      \brief virtual destruct. do nothing.
    */
    virtual
    ~SayMessage() = default;

    /*!
      \brief pure virtual method. get the header character of this message
      \return header character of this message
     */
    virtual
    char header() const = 0;

    /*!
      \brief pure virtual method. get the length of this message
      \return the length of encoded message
    */
    virtual
    int length() const = 0;

    /*!
      \brief append an audio message to the string
      \param to reference to the message string object
      \return result status of encoding
    */
    virtual
    bool appendTo( std::string & to ) const = 0;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & printDebug( std::ostream & os ) const = 0;

};


}

#endif
