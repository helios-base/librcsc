// -*-c++-*-

/*!
  \file intention_with_condition.cpp
  \brief intention that has condition Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "intention_with_condition.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/soccer_action.h>

#include <rcsc/common/logger.h>
#include <rcsc/game_time.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
IntentionWithCondition::finished( const PlayerAgent * agent )
{
    static GameTime s_last_updated_time( -1, 0 );

    if ( s_last_updated_time.cycle() + 1 != agent->world().time().cycle() )
    {
        dlog.addText( Logger::DRIBBLE,
                      "%s:%d: finished(). last execute time is not match"
                      ,__FILE__, __LINE__ );
        return true;
    }

    if ( ! M_condition_ptr )
    {
        std::cerr << agent->config().teamName()
                  << ' ' << agent->world().self().unum()
                  << " IntentionWithCondition. no condition object"
                  << std::endl;
        dlog.addText( Logger::KICK,
                      "%s:%d: no condtion object"
                      ,__FILE__, __LINE__ );
        return false;
    }

    if ( ! M_behavior_ptr )
    {
        std::cerr << agent->config().teamName()
                  << ' ' << agent->world().self().unum()
                  << " IntentionWithCondition. no behavior object"
                  << std::endl;
        dlog.addText( Logger::KICK,
                      "%s:%d: no behavior object"
                      ,__FILE__, __LINE__ );
        return false;
    }

    if ( (*M_condition_ptr)( agent ) )
    {
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
IntentionWithCondition::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  "%s:%d: execute."
                  ,__FILE__, __LINE__ );

    if ( ! M_behavior_ptr )
    {
        std::cerr << agent->config().teamName()
                  << ' ' << agent->world().self().unum()
                  << " IntentionWithCondition. no behavior. "
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      "%s:%d: error. no behavior "
                      ,__FILE__, __LINE__ );
    }

    return M_behavior_ptr->execute( agent );
}

}
