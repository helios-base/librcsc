// -*-c++-*-

/*!
  \file free_message_parser.h
  \brief player's freeform say message parser Header File
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

#ifndef RCSC_COMMON_FREE_MESSAGE_PARSER_H
#define RCSC_COMMON_FREE_MESSAGE_PARSER_H

#include <rcsc/common/say_message_parser.h>
#include <rcsc/common/audio_memory.h>

#include <string>
#include <iostream>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class FreeMessageParser
  \brief player's freeform say message parser
*/
template < std::size_t LEN >
class FreeMessageParser
    : public SayMessageParser {
private:
    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

    // not used
    FreeMessageParser() = delete;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    FreeMessageParser( std::shared_ptr< AudioMemory > memory )
        : M_memory( memory )
      { }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const
      {
          return static_cast< char >( '0' + LEN );
      }

    /*!
      \brief virtual method which analyzes audio messages.
      \param unum sender's uniform number
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int unum,
               const double & /* dir */,
               const char * msg,
               const GameTime & current )
      {
          if ( *msg != header() ) return 0;
          ++msg;
          if ( std::strlen( msg ) < LEN )
          {
              std::cerr << __FILE__ << ':' << __LINE__
                        << " FreeMessageParser: Illegal message length. message="
                        << msg << " must be length " << LEN
                        << std::endl;
              return -1;
          }

          M_memory->setFreeMessage( unum,
                                    std::string( msg, 0, LEN ),
                                    current );
          return 1 + LEN;
      }

};

}

#endif
