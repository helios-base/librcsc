// -*-c++-*-

/*!
  \file intercept_simulator.cpp
  \brief abstract intercept simulator manager Source File
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

#include "intercept_simulator_manager.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
void
InterceptSimulator::createBallCache( const WorldModel & wm,
                                   std::vector< Vector2D > & ball_cache )
{
    constexpr int MAX_STEP = 50;

    const ServerParam & SP = ServerParam::i();
    const double max_x = ( SP.keepawayMode()
                           ? SP.keepawayLength() * 0.5
                           : SP.pitchHalfLength() + 5.0 );
    const double max_y = ( SP.keepawayMode()
                           ? SP.keepawayWidth() * 0.5
                           : SP.pitchHalfWidth() + 5.0 );
    const double bdecay = SP.ballDecay();

    Vector2D bpos = wm.ball().pos();
    Vector2D bvel = ( wm.kickableOpponent()
                      //|| wm.kickableTeammate()
                      )
        ? Vector2D( 0.0, 0.0 )
        : wm.ball().vel();
    double bspeed = bvel.r();

    ball_cache.clear();
    ball_cache.reserve( MAX_STEP );

    for ( int i = 0; i < MAX_STEP; ++i )
    {
        ball_cache.push_back( bpos );

        if ( bspeed < 0.005 && i >= 10 )
        {
            break;
        }

        bpos += bvel;
        bvel *= bdecay;
        bspeed *= bdecay;

        if ( max_x < bpos.absX()
             || max_y < bpos.absY() )
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
InterceptSimulator::simulateSelf( const SelfObject & self,
                                  const std::vector< Vector2D > & ball_cache,
                                  std::vector< Intercept > & self_results )
{
    if ( self.isKickable() )
    {
        return;
    }

    const int max_step = static_cast< int >( M_ball_cache.size() );

    std::shared_ptr< InterceptSimulatorSelf > sim = createSelfSimulator();

    sim->simulate( wm, max_step, self_results );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
WorldModel::updatePlayerIntercept( const WorldModel & wm )
{

    std::shared_ptr< InterceptSimulatorPlayer > sim = createPlayerSimulator();


    for ( PlayarObject & o : M_opponents )
    {

    }

    for ( PlayarObject & t : M_teammates )
    {

    }

    if ( player.posCount() >= 10 )
    {
        return 1000;
    }

    if ( player.isKickable() )
    {
        return 0;
    }

    std::shared_ptr< InterceptSimulatorPlayer > sim = createPlayerSimulator();

    if ( ! sim )
    {
        return 1000;
    }

    return sim->simulate( wm, ball_cache, false );
}


/*-------------------------------------------------------------------*/
/*!

*/
int
InterceptSimulator::simulateGoalie( const WorldModel & wm,
                                    const PlayerObject & player,
                                    const std::vector< Vector2D > & ball_cache )
{
    if ( ! player.goalie() )
    {
        return 1000;
    }

    if ( player.posCount() >= 10 )
    {
        return 1000;
    }

    if ( player.isKickable() )
    {
        return 0;
    }

    std::shared_ptr< InterceptSimulatorPlayer > sim = createPlayerSimulator();

    if ( ! sim )
    {
        return 1000;
    }

    return sim->simulate( wm, ball_cache, true );
}


}
