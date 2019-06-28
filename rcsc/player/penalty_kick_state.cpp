// -*-c++-*-

/*!
  \file penalty_kick_state.cpp
  \brief penalty kick state model Source File
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

#include "penalty_kick_state.h"

#include <rcsc/game_mode.h>

#include <set>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PenaltyKickState::PenaltyKickState()
    : M_time( 0, 0 ),
      M_onfield_side( NEUTRAL ),
      M_our_taker_counter( 0 ),
      M_their_taker_counter( 0 ),
      M_our_score( 0 ),
      M_their_score( 0 ),
      M_kick_taker_side( NEUTRAL )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PenaltyKickState::isKickTaker( const SideID side,
                               const int unum ) const
{
    if ( M_kick_taker_side != side )
    {
        return false;
    }

    const int idx = ( ourTakerCounter() - 1 ) % 11;
    const int kick_taker_unum = M_kick_taker_order[idx];

    return kick_taker_unum == unum;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PenaltyKickState::update( const GameMode & game_mode,
                          const SideID ourside,
                          const GameTime & current )
{
    switch ( game_mode.type() ) {
    case GameMode::PenaltySetup_:
        M_time = current;
        M_current_taker_side = game_mode.side();
        if ( game_mode.side() == ourside )
        {
            ++M_our_taker_counter;
        }
        else
        {
            ++M_their_taker_counter;
        }
        break;
    case GameMode::PenaltyReady_:
        M_time = current;
        break;
    case GameMode::PenaltyTaken_:
        M_time = current;
        break;
    case GameMode::PenaltyMiss_:
        break;
    case GameMode::PenaltyScore_:
        if ( game_mode.side() == ourside )
        {
            ++M_our_score;
        }
        else
        {
            ++M_their_score;
        }
        break;
    case GameMode::PenaltyOnfield_:
        // NOT a real playmode
        // used only at first.
        // playmode is changed to PenaltySetup_ immediately.
        M_onfield_side = game_mode.side();
        break;
    case GameMode::PenaltyFoul_:
        // NOT a real playmode
        // playmode is changed to PenaltyMiss_ or PenaltyScore_ immediately.
        break;
    default:
        break;
    }

    //
    // default kick taker assignment
    //

    if ( ourTakerCounter() > 0
         && ( game_mode.type() == GameMode::PenaltySetup_
              || game_mode.type() == GameMode::PenaltyReady_
              || game_mode.type() == GameMode::PenaltyTaken_ )
         )
    {
        M_kick_taker_side = game_mode.side();
    }
    else
    {
        M_kick_taker_side = NEUTRAL;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PenaltyKickState::setKickTakerOrder( const std::vector< int > & unum_set )
{
    if ( unum_set.size() != 11 )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (setKickTakerOrder) illegal unum set size = "
                  << unum_set.size() << std::endl;
        return;
    }

    std::set< int > s;
    for ( int i = 0; i < 11; ++i )
    {
        if ( unum_set[i] < 1 || 11 < unum_set[i] )
        {
            std::cerr << __FILE__ << ' ' << __LINE__
                      << ": (setKickTakerOrder) illegal uniform number "
                      << unum_set[i] << std::endl;
            return;
        }
        s.insert( unum_set[i] );
    }

    if ( s.size() != 11 )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (setKickTakerOrder) exist duplicated uniform number"
                  << std::endl;
        return;
    }

    for ( int i = 0; i < 11; ++i )
    {
        M_kick_taker_order[i] = unum_set[i];
    }
}

}
