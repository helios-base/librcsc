// -*-c++-*-

/*!
  \file intention_with_condition.h
  \brief intention that has condition Heder File
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

#ifndef RCSC_ACTION_INTENTION_WITH_CONDITION_H
#define RCSC_ACTION_INTENTION_WITH_CONDITION_H

#include <rcsc/player/soccer_intention.h>

#include <rcsc/common/soccer_condition.h>
#include <rcsc/player/soccer_action.h>

namespace rcsc {

class PlayerAgent;

/*!
  \class IntentionWithCondition
  \brief intention that has condition to be checked.
 */
class IntentionWithCondition
    : public SoccerIntention {
private:

    //! condition object
    boost::shared_ptr< Condition > M_condition_ptr;

    //! behavior object
    boost::shared_ptr< SoccerBehavior > M_behavior_ptr;

public:
    /*!
      \brief construct with condition & behavior
     */
    IntentionWithCondition( boost::shared_ptr< Condition > condition,
                            boost::shared_ptr< SoccerBehavior > behavior )
        : M_condition_ptr( condition )
        , M_behavior_ptr( behavior )
      { }

    /*!
      \brief check if this intention finishes
      \param agent const pointer to the agent instance
      \return true if intention finishes
     */
    bool finished( const PlayerAgent * agent );

    /*!
      \brief execute action
      \param agent pointer to the agent itself
      \return true if action is performed
     */
    bool execute( PlayerAgent * agent );
};

}

#endif
