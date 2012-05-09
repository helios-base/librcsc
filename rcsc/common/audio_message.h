// -*-c++-*-

/*!
  \file audio_message.h
  \brief abstract audio message parser Header File
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

#ifndef RCSC_PLAYER_AUDIO_MESSAGE_H
#define RCSC_PLAYER_AUDIO_MESSAGE_H

#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <string>


namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \struct HearMessage
  \brief player message instance
 */
struct HearMessage {

    int unum_; //!< sender's uniform number
    double dir_; //!< sender's direction
    std::string str_; //!< raw message string

    /*
      \brief set default values and reserve string buffer
     */
    HearMessage()
        : unum_( Unum_Unknown ),
          dir_( 0.0 )
      {
          str_.reserve( 10 );
      }

};

// TODO:
//   FreeformMessage
//   CLangMessage

}

#endif
