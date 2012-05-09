// -*-c++-*-

/*!
  \file body_kick_collide_with_ball.cpp
  \brief intentional kick action to collide with ball
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

#include "body_hold_ball2008.h"

#include "body_kick_collide_with_ball.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickCollideWithBall::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__":%d: Body_KickCollideWithBall" );

    const WorldModel & wm = agent->world();

    if ( ! wm.self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__":  not kickable" );
        return false;
    }

    Vector2D required_accel = wm.self().vel(); // target relative pos
    required_accel -= wm.ball().rpos(); // required vel
    required_accel -= wm.ball().vel(); // ball next rpos

    double kick_power = required_accel.r() / wm.self().kickRate();
    if ( kick_power > ServerParam::i().maxPower() * 1.1 )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__":   over max power(%f).",
                      kick_power );
        Body_HoldBall2008( false ).execute( agent );
        return false;
    }

    if ( kick_power > ServerParam::i().maxPower() )
    {
        kick_power = ServerParam::i().maxPower();
    }

    agent->doKick( kick_power,
                   required_accel.th() - wm.self().body() );
    return true;
}

}
