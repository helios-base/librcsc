// -*-c++-*-

/*!
  \file player_change_state.cpp
  \brief player change state holder class Source File
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

#include "player_change_state.h"

#include <rcsc/param/server_param.h>
#include <rcsc/param/player_param.h>
#include <rcsc/game_mode.h>
#include <rcsc/game_time.h>

#include <cstdio>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PlayerChangeState::PlayerChangeState()
    : M_change_count( 0 )
{
    for ( int i = 0; i < 11; i++ )
    {
        M_teammate_types[i] = 0;
        M_opponent_changed[i] = false;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerChangeState::parse( const char * msg,
                          const GameMode & game_mode,
                          const GameTime & current )
{

    // teammate: "(change_player_type <unum> <type>)\n"
    //           "(ok change_player_type <unum> <type>)\n"
    // opponent: "(change_player_type <unum>)\n"

    int unum = -1, type = -1;

    if ( std::sscanf( msg, "(ok change_player_type %d %d)",
                      &unum, &type ) == 2
         )
    {
        change( unum, type );

        if ( current.cycle() % ServerParam::i().halfTime() != 0
             || game_mode.type() != GameMode::BeforeKickOff )
        {
            // increment change count
            M_change_count++;
        }
        std::unordered_map< int, int >::iterator it = M_teammate_type_count.find( type );
        if ( it != M_teammate_type_count.end() )
        {
            it->second += 1;
        }
        else
        {
            M_teammate_type_count.insert( std::make_pair( type, 1 ) );
        }
    }
    else if ( std::sscanf( msg, "(change_player_type %d %d)",
                           &unum, &type ) == 2
              )
    {
        change( unum, type );
    }
    else if ( std::sscanf( msg, "(change_player_type %d)",
                           &unum ) == 1
              )
    {
        change( unum, -1 );
    }
    else
    {
        std::cerr << "Coach: parse error : change_player_type." << std::endl;
    }

}

/*-------------------------------------------------------------------*/
/*!
  set changed player type id
  it type is negative, opponent type is set
*/
void
PlayerChangeState::change( const int unum,
                           const int type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << "PlayerChangeState::change. invalid unum "
                  << unum << std::endl;
        return;
    }

    if ( PlayerParam::i().playerTypes() <= type )
    {
        std::cerr << "PlayerChangeState::substitute. invalid type "
                  << type << std::endl;
        return;
    }

    if ( type == -1 )
    {
        M_opponent_changed[unum - 1] = true;
    }
    else
    {
        M_teammate_types[unum - 1] = type;
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerChangeState::canChange( const int unum,
                              const int type,
                              const GameMode & game_mode,
                              const GameTime & current ) const
{
    if ( game_mode.type() == GameMode::PlayOn )
    {
        return false;
    }

    if ( current.cycle() >= ( ServerParam::i().halfTime()
                              * ( ServerParam::i().nrNormalHalfs()
                                  + ServerParam::i().nrExtraHalfs() ) ) )
    {
        return false;
    }

    if ( M_change_count < PlayerParam::i().subsMax()
         || ( game_mode.type() == GameMode::BeforeKickOff
              && current.cycle() % ServerParam::i().halfTime() == 0 )
         )
    {
        // if game is half time mode, coach can always change player type.
    }
    else
    {
        return false;
    }

    if ( type != 0 )
    {
        std::unordered_map< int, int >::const_iterator it = M_teammate_type_count.find(type);
        if ( it == M_teammate_type_count.end() )
        {
            return true;
        }

        if ( it->second == PlayerParam::i().ptMax()
             && type != getTeammateType(unum) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
int
PlayerChangeState::getTeammateType( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << "Coach::PlayerChangeState::getTeammatePlayerType. invalid unum "
                  << unum << std::endl;
        return -1;
    }

    return M_teammate_types[unum - 1];
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerChangeState::isOpponentChanged( const int unum ) const
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << "Coach::PlayerChangeState::isOpponentChanged. invalid unum "
                  << unum << std::endl;
        return false;
    }

    return M_opponent_changed[unum - 1];
}

}
