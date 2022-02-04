// -*-c++-*-

/*!
  \file clang_manager.h
  \brief coach language status manager Header File
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

#ifndef RCSC_COACH_CLANG_MANAGER_H
#define RCSC_COACH_CLANG_MANAGER_H

#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <string>
#include <utility>

/*!
  \class CLangManager
  \brief manages coach language status
*/
class CLangManager {
private:
    // not used
    CLangManager( const CLangManager & ) = delete;
    // not used
    CLangManager & operator=( const CLangManager & ) = delete;

    static const int DEFAULT_CLANG_MIN_VER; //!< default minimal clang version
    static const int DEFAULT_CLANG_MAX_VER; //!< default maximal clang version

    // team's acceptable clang version
    int M_clang_min_ver; //!< minimal version in teammate
    int M_clang_max_ver; //!< maximal version in teammate

    //! array of players' supported min & max clang version pair
    std::pair< int, int > M_teammate_clang_ver[11];

    // last clang sent time
    GameTime M_last_define_time; //!< define message sent time
    GameTime M_last_meta_time; //!< meta message sent time
    GameTime M_last_advice_time; //!< advice message sent time
    GameTime M_last_info_time; //!< info message sent time
    GameTime M_last_delete_time; //!< delete message sent time
    GameTime M_last_rule_time; //!< rule message sent time

    //! count of freeform message sent
    int M_freeform_count;

public:
    /*!
      \brief init member variables
     */
    CLangManager();

    // setter

    /*!
      \brief parse player's acceptable clang version and update acceptable version
      \param msg server raw message
     */
    void parseCLangVer( const char * msg );

    /*!
      \brief set time clang define message is sent.
      \param t sent time
     */
    void updateDefineTime( const GameTime & t )
      {
          M_last_define_time = t;
      }

    /*!
      \brief set time clang meta message is sent.
      \param t sent time
     */
    void updateMetaTime( const GameTime & t )
      {
          M_last_meta_time = t;
      }

    /*!
      \brief set time clang advice message is sent.
      \param t sent time
     */
    void updateAdviceTime( const GameTime & t )
      {
          M_last_advice_time = t;
      }

    /*!
      \brief set time clang info message is sent.
      \param t sent time
     */
    void updateInfoTime( const GameTime & t )
      {
          M_last_advice_time = t;
      }

    /*!
      \brief set time clang delete message is sent.
      \param t sent time
     */
    void updateDeleteTime( const GameTime & t )
      {
          M_last_delete_time = t;
      }

    /*!
      \brief set time clang rule message is sent.
      \param t sent time
     */
    void updateRuleTime( const GameTime & t )
      {
          M_last_rule_time = t;
      }

    /*!
      \brief increment freeform message send count
     */
    void incFreeFormCount()
      {
          ++M_freeform_count;
      }

    /*!
      \brief reset freeform message send count to 0
     */
    void resetFreeFormCount()
      {
          M_freeform_count = 0;
      }

    // getter

    /*!
      \brief get minimal acceptable clang version
      \return version number
     */
    int minVersion() const
      {
          return M_clang_min_ver;
      }

    /*!
      \brief get maximal acceptable clang version
      \return version number
     */
    int maxVersion() const
      {
          return M_clang_min_ver;
      }

    /*!
      \brief get last define message send time
      \return const reference to the game time object
     */
    const
    GameTime & lastDefineTime() const
      {
          return M_last_define_time;
      }

    /*!
      \brief get last meta message send time
      \return const reference to the game time object
     */
    const
    GameTime & lastMetaTime() const
      {
          return M_last_meta_time;
      }

    /*!
      \brief get last advice message send time
      \return const reference to the game time object
     */
    const
    GameTime & lastAdviceTime() const
      {
          return M_last_advice_time;
      }

    /*!
      \brief get last info message send time
      \return const reference to the game time object
     */
    const
    GameTime & lastInfoTime() const
      {
          return M_last_info_time;
      }

    /*!
      \brief get freeform message send count
      \return counted number
     */
    int freeFormCount() const
      {
          return M_freeform_count;
      }
};

#endif
