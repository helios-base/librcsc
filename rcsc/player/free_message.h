// -*-c++-*-

/*!
  \file free_message.h
  \brief player's freeform say message builder Header File
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

#ifndef RCSC_PLAYER_FREE_MESSAGE_H
#define RCSC_PLAYER_FREE_MESSAGE_H

#include <rcsc/player/say_message_builder.h>
#include <rcsc/common/audio_memory.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include <string>
#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class FreeMessage
  \brief player's free formed say message encoder
*/
template < std::size_t LEN >
class FreeMessage
    : public SayMessage {
private:

    std::string M_message; //!< message without header

public:

    /*!
      \brief construct with raw message string
      \param msg message string
     */
    FreeMessage( const std::string & msg )
      {
          M_message = msg;
      }

    /*!
      \brief pure virtual method. get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return static_cast< char >( '0' + LEN );
      }

    /*!
      \brief pure virtual method. get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return LEN + 1;
      }

    /*!
      \brief append the audio message to be sent
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool toString( std::string & to ) const
      {
          if ( static_cast< int >( to.length() + 1 + LEN )
               > ServerParam::i().playerSayMsgSize() )
          {
              std::cerr << __FILE__ << ':' << __LINE__
                        << " FreeMessage: over the capacity. message="
                        << M_message << ". current size = "
                        << to.length()
                        << std::endl;
              return false;
          }

          if ( M_message.length() != LEN )
          {
              std::cerr << __FILE__ << ':' << __LINE__
                        << " Illegal message length. message="
                        << M_message << " must be length " << LEN
                        << std::endl;
              return false;
          }

          to += header();
          to += M_message;
          return true;
      }

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const
      {
          os << "[Free:" << M_message << ']';
          return os;
      }

};

}

#endif
