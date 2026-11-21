// -*-c++-*-

/*!
  \file intention_kick2007.h
  \brief queued kick behavior.
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

#ifndef RCSC_ACTION_INTENAION_KICK_H
#define RCSC_ACTION_INTENAION_KICK_H

#include <rcsc/player/soccer_intention.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>

namespace rcsc {

/*!
  \class IntentionKick
  \brief queued kick behavior.
*/
class IntentionKick
    : public SoccerIntention {
private:
    const Vector2D M_target_point; //!< target point
    const double M_first_speed; //!< first ball speed when last kick

    //! player try to finish to kick within this step
    int M_kick_step;

    //! if this is true, player must finish kicks within M_kick_step
    bool M_enforce_kick;

    GameTime M_last_execute_time; //!< last executed time

public:
    /*!
      \brief constructor
      \param target_point kick target point
      \param first_speed first ball speed when last kick
      \param kick_step player try to finish to kick within this step
      \param enforce_kick if this is true, player must finish kicks within M_kick_step
      \param start_time action started time
     */
    IntentionKick( const Vector2D & target_point,
                   const double & first_speed,
                   const int kick_step,
                   const bool enforce_kick,
                   const GameTime & start_time );

    /*!
      \brief check the status.
      \param agent pointer to the agent itself.
      \return true if action queue is finished.
     */
    bool finished( const PlayerAgent * agent );

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
     */
    bool execute( PlayerAgent * agent );


private:

    /*!
      \brief clear the action queue
     */
    void clear()
      {
          M_kick_step = 0;
      }

};

}

#endif
