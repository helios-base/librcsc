// -*-c++-*-

/*!
  \file clang_message.h
  \brief abstract clang message class Header File
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

#ifndef RCSC_CLANG_MESSAGE_H
#define RCSC_CLANG_MESSAGE_H

#include <rcsc/clang/types.h>

#include <memory>
#include <iosfwd>

namespace rcsc {

/*!
  \class CLangMessage
  \brief abstract clang message
 */
class CLangMessage {
public:

    //! smart pointer type
    typedef std::shared_ptr< CLangMessage > Ptr;

    //! const smart pointer type
    typedef std::shared_ptr< const CLangMessage > ConstPtr;

private:

    // not used
    CLangMessage( const CLangMessage & ) = delete;
    CLangMessage & operator=( const CLangMessage & ) = delete;

protected:

    /*!
      \brief protected constructor
     */
    CLangMessage()
      { }

public:

    /*!
      \brief virtual destructor
     */
    virtual
    ~CLangMessage()
      { }

    /*!
      \brief get message type id.
      \return message type id
     */
    virtual
    CLangType type() const = 0;

    /*!
      \brief get message type name.
      \return literal characters.
     */
    virtual
    const char * typeName() const = 0;

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const = 0;

};

}

/*!
  \brief stream operator
  \param os reference to the output stream
  \param msg clang message object
  \return reference to the output stream
 */
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::CLangMessage & msg )
{
    return msg.print( os );
}

#endif
