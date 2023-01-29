// -*-c++-*-

/*!
  \file see_state.h
  \brief see synch state manager Header File
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

#ifndef RCSC_PLAYER_SEE_STATE_H
#define RCSC_PLAYER_SEE_STATE_H

#include <rcsc/player/view_mode.h>
#include <rcsc/game_time.h>

namespace rcsc {

/*!
  \class SeeState
  \brief see timing manager for see synchronization
*/
class SeeState {
public:
    /*!
      \brief see synchronization type
     */
    enum SynchType {
        SYNCH_NO, //! no sync.
        SYNCH_EVERY, //! Normal - Narrow - Narrow
        SYNCH_NARROW, //! only Narrow
        SYNCH_NORMAL, //! only Normal
        SYNCH_WIDE, //! only Wide
        SYNCH_SYNC, //!< rcssserver sync view mode
    };

    /*!
      \brief see arrival timing type Id
     */
    enum Timing {
        TIME_0_00 = 0, //! synch0 0ms from sense_body
        TIME_37_5 = 375,
        TIME_75_0 = 750,
        TIME_12_5 = 125,
        TIME_50_0 = 500, //! synch1 50ms from sense_body
        TIME_87_5 = 875,
        TIME_22_5 = 225, //! synch2 about 30ms from sense_body
        TIME_62_5 = 625,
        TIME_SYNC = 999,
        TIME_NOSYNCH = 1000, //! no synchronization
    };

private:
    enum {
        HISTORY_SIZE = 3
    };

    static bool S_synch_see_mode;

    double M_protocol_version;

    GameTime M_current_time; //!< update when new cycle detected
    GameTime M_last_see_time; //!< last see arrival game time

    SynchType M_synch_type; //!< see synchronization type

    Timing M_last_timing; //!< last see timing from sensebody
    int M_current_see_count; //!< previous cycle's see count
    int M_see_count_history[HISTORY_SIZE]; //!< last 3 see counts

    int M_cycles_till_next_see; //!< count till next see arrival time

    ViewWidth M_view_width; //!< current view width
    ViewQuality M_view_quality; //!< current view quality

    //! not used
    SeeState( const SeeState & ) = delete;
    //! not used
    SeeState & operator=( const SeeState & ) = delete;
public:
    /*!
      \brief init member variables
     */
    SeeState();

    /*!
       \brief set a synchronous see timer mode.
     */
    void setSynchSeeMode()
      {
          S_synch_see_mode = true;
      }

    /*!
      \brief set the protocl version number
      \param ver the version number
     */
    void setProtocolVersion( const double ver )
      {
          M_protocol_version = ver;
          if ( ver >= 18.0 )
          {
              S_synch_see_mode = true;
          }
      }

    /*!
      \brief static method. get the status of synch_see
      \return the status of synch_see
     */
    static
    bool synch_see_mode()
      {
          return S_synch_see_mode;
      }

    /*!
      \brief get last see timing
      \return timing type Id
     */
    Timing lastTiming() const
      {
          return M_last_timing;
      }

private:

    /*!
      \brief get next see arrival timing while see synchronization
      \param vw current view width
      \param vq current view quality
      \return timing type Id
     */
    Timing getNextTiming( const ViewWidth & vw,
                          const ViewQuality & vq ) const;

public:
    ///////////////////////////////////////////////////////
    // used when server messages are received

    /*!
      \brief update time & view mode
      \param sense_time game time when sense_body message arrived
      \param vw view width contained by sense_body message
      \param vq view quality contained by sense_body message

      This methos is called when see arrived.
     */
    void updateBySenseBody( const GameTime & sense_time,
                            const ViewWidth & vw,
                            const ViewQuality & vq );

    /*!
      \brief update see counter
      \param see_time game time when see message arrived
      \param vw view width that received see message
      \param vq view quality that received see message

      This methos is called when see arrived.
     */
    void updateBySee( const GameTime & see_time,
                      const ViewWidth & vw,
                      const ViewQuality & vq );

    /*!
      \brief update game time

      This method is called when sense_body arrived or new time is detected.
      All counters are rotated.
     */
    void setNewCycle( const GameTime & new_time );

    /*!
      \brief set new timing

      This method is called only when synchronization is detected
     */
    void setLastSeeTiming( const Timing last_timing );

    /*!
      \brief check if see synchronization is keeped or not
      \return true if keeped synchronization
     */
    bool isSynch() const;

    /*!
      \brief check if current status is synchronization chance
      \return true if synchronization chance

      This method is used for normal server mode.
      assumed that this method is used while (narrow low) mode
      This method must be called just after (see) message arrival.

      === NOTE ===
      This method depends on rcssserver's configuration
      - If send_step or any other step parameter are changed,
      we must NOT use this method.
    */
    bool isSynchedSeeCountNormalMode() const;

    /*!
      \brief check if current status is synchronization chance
      \return true if synchronization chance

      This method is used for server synch_mode
      assume that this method is used while (narrow low) mode

      === NOTE ===
      This method depends on rcssserver's configuration
      - This method must be called when agent received (think) message.
      - If synch_offset parametor is changed from '60', implementation of
      this must be changed.
    */
    bool isSynchedSeeCountSynchMode() const;

    ///////////////////////////////////////////////////////
    // used when decision making

    /*!
      \brief check if player can change view width to 'next_width'
      with see synchronization
      \param next_width required new view width
      \param current current game time
      \return true if player can change to next_width
    */
    bool canSendChangeView( const ViewWidth & next_width,
                            const GameTime & current ) const;

    /*!
      \brief get cycles to get next see info
      \return cycles to get next see info
    */
    int cyclesTillNextSee() const;

    /*!
      \brief set new view mode, and update estimated next see arrival timing
      and estimated cycles till next see
      \param new_width new view width
      \param new_quality new view quality

      This method MUST be called just after sending change_view command and
      only when current cycle see message is received
    */
    void setViewMode( const ViewWidth & new_width,
                      const ViewQuality & new_quality );
};

}

#endif
