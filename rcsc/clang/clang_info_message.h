// -*-c++-*-

/*!
  \file clang_info_message.h
  \brief info message class Header File
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

#ifndef RCSC_CLANG_INFO_MESSAGE_H
#define RCSC_CLANG_INFO_MESSAGE_H

#include <rcsc/clang/clang_message.h>
#include <rcsc/clang/clang_token.h>

namespace rcsc {

/*!
  \class CLangInfoMessage
  \brief info message
 */
class CLangInfoMessage
    : public CLangMessage {
private:

    //! token contaner
    CLangToken::Cont M_tokens;

public:

    /*!
      \brief construct with created time
      \brief t message created time
     */
    CLangInfoMessage()
      { }

    /*!
      \brief get message type id.
      \return message type id
     */
    virtual
    CLangType type() const
      {
          return CLANG_INFO;
      }

    /*!
      \brief get message type name.
      \return literal characters.
     */
    virtual
    const char * typeName() const
      {
          return "info";
      }

    /*!
      \brief get token container.
      \return token container.
     */
    const CLangToken::Cont & tokens() const
      {
          return M_tokens;
      }

    /*!
      \brief add new token.
      \param tok new token object.
     */
    void addToken( CLangToken * tok )
      {
          M_tokens.push_back( CLangToken::ConstPtr( tok ) );
      }

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;

};

}

#endif
