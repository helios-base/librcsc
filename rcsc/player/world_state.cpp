// -*-c++-*-

/*!
  \file world_state.cpp
  \brief world state class Source File
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

#include "world_state.h"

#include "world_model.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
WorldState::WorldState( const WorldModel & wm )
    : M_time( wm.time() ),
      M_game_mode( wm.gameMode() ),
      M_self( wm.self() ),
      M_ball( wm.ball() ),
      M_teammates( wm.teammateObjects().begin(), wm.teammateObjects().end() ),
      M_opponents( wm.opponentObjects().begin(), wm.opponentObjects().end() ),
      M_unknown_players( wm.unknownPlayerObjects().begin(), wm.unknownPlayerObjects().end() ),
      M_kickable_teammate( nullptr ),
      M_kickable_opponent( nullptr ),
      M_maybe_kickable_teammate( nullptr ),
      M_maybe_kickable_opponent( nullptr ),
      M_offside_line_x( wm.offsideLineX() ),
      M_our_offense_line_x( wm.ourOffenseLineX() ),
      M_our_defense_line_x( wm.ourDefenseLineX() ),
      M_their_offense_line_x( wm.theirOffenseLineX() ),
      M_their_defense_line_x( wm.theirDefenseLineX() ),
      M_our_offense_player_line_x( wm.ourOffensePlayerLineX() ),
      M_our_defense_player_line_x( wm.ourDefensePlayerLineX() ),
      M_their_offense_player_line_x( wm.theirOffensePlayerLineX() ),
      M_their_defense_player_line_x( wm.theirDefensePlayerLineX() )
{
    M_our_players.reserve( 1 + M_teammates.size() );
    M_their_players.reserve( M_opponents.size() + M_unknown_players.size() );

    std::fill( M_our_players_array.begin(), M_our_players_array.end(), nullptr );
    std::fill( M_their_players_array.begin(), M_their_players_array.end(), nullptr );

    //
    // set our players
    //
    M_our_players.push_back( &M_self );
    M_our_players_array[self().unum()] = &M_self;
    for ( const PlayerObject & t : M_teammates )
    {
        M_our_players.push_back( &t );
        if ( t.unum() != Unum_Unknown )
        {
            M_our_players_array[t.unum()] = &t;
        }
    }

    //
    // set their players
    //
    for ( const PlayerObject & o : M_opponents )
    {
        M_their_players.push_back( &o );
        if ( o.unum() != Unum_Unknown )
        {
            M_their_players_array[o.unum()] = &o;
        }
    }

    for ( const PlayerObject & u : M_unknown_players )
    {
        M_their_players.push_back( &u );
    }

    //
    // update kickable player
    //

    if ( wm.kickableTeammate() )
    {
        std::vector< const AbstractPlayerObject * >::iterator it
            = std::find_if( M_our_players.begin(), M_our_players.end(),
                            [&]( const AbstractPlayerObject * p )
                            {
                                return p->id() == wm.kickableTeammate()->id();
                            } );
        if ( it != M_our_players.end() )
        {
            M_kickable_teammate = *it;
        }
    }

    if ( wm.maybeKickableTeammate() )
    {
         std::vector< const AbstractPlayerObject * >::iterator it
            = std::find_if( M_our_players.begin(), M_our_players.end(),
                            [&]( const AbstractPlayerObject * p )
                            {
                                return p->id() == wm.maybeKickableTeammate()->id();
                            } );
        if ( it != M_our_players.end() )
        {
            M_maybe_kickable_teammate = *it;
        }
    }


    if ( wm.kickableOpponent() )
    {
        std::vector< const AbstractPlayerObject * >::iterator it
            = std::find_if( M_their_players.begin(), M_their_players.end(),
                            [&]( const AbstractPlayerObject * p )
                            {
                                return p->id() == wm.kickableOpponent()->id();
                            } );
        if ( it != M_their_players.end() )
        {
            M_kickable_opponent = *it;
        }
    }

    if ( wm.maybeKickableOpponent() )
    {
         std::vector< const AbstractPlayerObject * >::iterator it
            = std::find_if( M_their_players.begin(), M_their_players.end(),
                            [&]( const AbstractPlayerObject * p )
                            {
                                return p->id() == wm.maybeKickableOpponent()->id();
                            } );
        if ( it != M_their_players.end() )
        {
            M_maybe_kickable_opponent = *it;
        }
    }
}

}
