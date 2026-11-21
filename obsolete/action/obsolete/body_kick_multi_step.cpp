// -*-c++-*-

/*!
  \file body_kick_multi_step.cpp
  \brief multiple kick behavior to accelerate the ball to the
  desired speed.
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

#include "body_kick_multi_step.h"

#include "body_kick_one_step.h"
#include "body_kick_two_step.h"
#include "body_stop_ball.h"
#include "body_hold_ball.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickMultiStep::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::KICK,
                  __FILE__": Body_KickMultiStep" );

    if ( ! agent->world().self().isKickable() )
    {
        std::cerr << __FILE__ << ": " << __LINE__
                  << " not ball kickable!"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__":  not kickable" );
        return false;
    }

    //-------------------------------------------
    Vector2D target_rpos = M_target_point - agent->world().self().pos();
    M_first_speed = std::min( M_first_speed, ServerParam::i().ballSpeedMax() );

    Vector2D achieved_vel;

    dlog.addText( Logger::KICK,
                  __FILE__": simulate one kick" );
    if ( Body_KickTwoStep::simulate_one_kick( &achieved_vel,
                                              NULL,
                                              NULL,
                                              target_rpos,
                                              M_first_speed,
                                              Vector2D( 0.0, 0.0 ), // my current pos
                                              agent->world().self().vel(),
                                              agent->world().self().body(),
                                              agent->world().ball().rpos(),
                                              agent->world().ball().vel(),
                                              agent,
                                              false ) ) // not enforce
    {
        Vector2D accel = achieved_vel - agent->world().ball().vel();
        double kick_power = accel.r() / agent->world().self().kickRate();
        AngleDeg kick_dir = accel.th() - agent->world().self().body();

        dlog.addText( Logger::KICK,
                      __FILE__": only one kick"
                      "   result=(%.3f, %.3f)[r=%.3f]  accel=(%.3f, %.3f)  power=%.1f dir=%.1f",
                      achieved_vel.x, achieved_vel.y, achieved_vel.r(),
                      accel.x, accel.y,
                      kick_power, kick_dir.degree() );
        M_ball_result_pos = agent->world().ball().pos() + achieved_vel;
        M_ball_result_vel = achieved_vel * ServerParam::i().ballDecay();;
        M_kick_step = 1;

        return agent->doKick( kick_power, kick_dir );
    }


    dlog.addText( Logger::KICK,
                  __FILE__": simulate two kick" );

    Vector2D next_vel;

    if ( Body_KickTwoStep::simulate_two_kick( &achieved_vel,
                                              &next_vel,
                                              target_rpos,
                                              M_first_speed,
                                              Vector2D( 0.0, 0.0 ), // my current pos
                                              agent->world().self().vel(),
                                              agent->world().self().body(),
                                              agent->world().ball().rpos(),
                                              agent->world().ball().vel(),
                                              agent,
                                              false ) ) // not enforced
    {
        Vector2D accel = next_vel - agent->world().ball().vel();
        double kick_power = accel.r() / agent->world().self().kickRate();
        AngleDeg kick_dir = accel.th() - agent->world().self().body();

        dlog.addText( Logger::KICK,
                      __FILE__": two kick."
                      "  result=(%.3f, %.3f)[r=%.3f]  next_bvel=(%.3f, %.3f)[r=%.3f]"
                      "      accel=(%.3f, %.3f)  power=%.1f dir=%.1f",
                      achieved_vel.x, achieved_vel.y, achieved_vel.r(),
                      next_vel.x, next_vel.y, next_vel.r(),
                      accel.x, accel.y,
                      kick_power, kick_dir.degree() );
        agent->debugClient().addCircle( agent->world().ball().pos() + next_vel,
                                        0.05 );
        M_ball_result_pos = agent->world().ball().pos() + next_vel;
        M_ball_result_vel = next_vel * ServerParam::i().ballDecay();;
        M_kick_step = 2;

        return agent->doKick( kick_power, kick_dir );
    }

    dlog.addText( Logger::KICK,
                  __FILE__": simulate three kick" );

    if ( simulate_three_kick( &achieved_vel,
                              &next_vel,
                              target_rpos,
                              M_first_speed,
                              Vector2D( 0.0, 0.0 ), // my current pos
                              agent->world().self().vel(),
                              agent->world().self().body(),
                              agent->world().ball().rpos(),
                              agent->world().ball().vel(),
                              agent,
                              false ) )
    {
        Vector2D accel = next_vel - agent->world().ball().vel();
        double kick_power = accel.r() / agent->world().self().kickRate();
        AngleDeg kick_dir = accel.th() - agent->world().self().body();

        dlog.addText( Logger::KICK,
                      __FILE__": three kick."
                      "   result=(%.3f, %.3f)[r=%.3f]  next_bvel=(%.3f, %.3f)[r=%.3f]"
                      "   accel=(%.3f, %.3f)  power=%.1f dir=%.1f",
                      achieved_vel.x, achieved_vel.y, achieved_vel.r(),
                      next_vel.x, next_vel.y, next_vel.r(),
                      accel.x, accel.y,
                      kick_power, kick_dir.degree() );
        agent->debugClient().addCircle( agent->world().ball().pos() + next_vel,
                                        0.05 );
        M_ball_result_pos = agent->world().ball().pos() + next_vel;
        M_ball_result_vel = next_vel * ServerParam::i().ballDecay();;
        M_kick_step = 3;

        return agent->doKick( kick_power, kick_dir );
    }


    if ( M_enforce_kick )
    {
        dlog.addText( Logger::KICK,
                      __FILE__": failed but enforce mode. try two step" );
        Body_KickTwoStep kick( M_target_point,
                               M_first_speed,
                               true // enforce
                               );
        bool result = kick.execute( agent );
        M_ball_result_pos = kick.ballResultPos();
        M_ball_result_vel = kick.ballResultVel();
        M_kick_step = kick.kickStep();

        return result;
    }

    dlog.addText( Logger::KICK,
                  __FILE__": failed. hold" );

    return Body_HoldBall( true, // turn to target
                          M_target_point,
                          ( agent->world().gameMode().type() == GameMode::PlayOn
                            ? M_target_point  // keep reverse side
                            : Vector2D::INVALIDATED )
                          ).execute( agent );
}

/*-------------------------------------------------------------------*/
/*!
  static method
  internal simulation like recursive
*/
bool
Body_KickMultiStep::simulate_three_kick( Vector2D * achieved_vel,
                                         Vector2D * next_vel,
                                         const Vector2D & target_rpos,
                                         const double & first_speed,
                                         const Vector2D & my_rpos,
                                         const Vector2D & my_vel,
                                         const AngleDeg & my_body,
                                         const Vector2D & ball_rpos,
                                         const Vector2D & ball_vel,
                                         const PlayerAgent * agent,
                                         const bool enforce )
{
    //dlog.addText( Logger::KICK,
    //"Kick Search: three step +*+*+*+*+*+*+*+*+*+*+*+*+" ));

    Vector2D ball_rel_at_here = ball_rpos - my_rpos;
    const double krate
        = kick_rate( ball_rel_at_here.r(),
                     ( ball_rel_at_here.th() - my_body ).degree(),
                     ServerParam::i().kickPowerRate(),
                     ServerParam::i().ballSize(),
                     agent->world().self().playerType().playerSize(),
                     agent->world().self().playerType().kickableMargin() );

    const double my_kickable = agent->world().self().playerType().kickableArea();

    std::vector< std::pair< Vector2D, Vector2D > > subtargets; // next ball position & vel

    const Vector2D my_next = my_rpos + my_vel;

    //////////////////////////////////////////////////////////////////
    // generate subtargets
    {
        // subtargets is generated in opposite side of target dir.
        const double max_accel2 = std::pow( std::min( ServerParam::i().maxPower() * krate,
                                                      ServerParam::i().ballAccelMax() ),
                                            2 );

        const Vector2D my_next_next
            = my_next + my_vel * agent->world().self().playerType().playerDecay();

        const double subtarget_dist = my_kickable * 0.65;
        const double default_dir_inc =  30.0;
        const double default_add_max = 181.0 - default_dir_inc * 0.5;

        const AngleDeg angle_self_to_target = (target_rpos - my_next_next).th();
        const AngleDeg first_sub_target_angle
            = angle_self_to_target + 90.0 + default_dir_inc * 0.5;

        double add_dir = 0.0;
        while ( add_dir < default_add_max )
        {
            Vector2D sub // rel to current my pos
                = my_next
                + Vector2D::polar2vector(subtarget_dist,
                                         first_sub_target_angle + add_dir);
            Vector2D require_vel = sub - ball_rpos;
            if ( ( require_vel - ball_vel ).r2() < max_accel2
                 && ! Body_KickTwoStep::is_opp_kickable( agent, sub, NULL ) )
            {
                subtargets.push_back
                    ( std::make_pair( sub,
                                      require_vel * ServerParam::i().ballDecay() ) );
            }
            add_dir += default_dir_inc;
        }
    }


    //////////////////////////////////////////////////////////////////
    // subtargets are next ball position

    Vector2D max_achieved_vel( 0.0, 0.0 );
    Vector2D sol_next_ball_vel( 100.0, 100.0 ); // next ball vel to archive the final vel
    bool found = false;
    {
        const Vector2D my_next_vel
            = my_vel * agent->world().self().playerType().playerDecay();

        Vector2D sol_vel; // last reachable vel
        const std::vector< std::pair< Vector2D, Vector2D > >::const_iterator
            target_end = subtargets.end();

        for ( std::vector< std::pair< Vector2D, Vector2D > >::const_iterator
                  it = subtargets.begin();
              it != target_end;
              ++it )
        {
            //Vector2D rel = it->first - my_next;
            //dlog.addText( Logger::KICK,
            //              " three step check subtarget=(%f, %f) r=%f, th=%f",
            //              rel.x, rel.y, rel.r(), rel.th() );
            if ( Body_KickTwoStep::simulate_two_kick( &sol_vel,
                                                      NULL,
                                                      target_rpos,
                                                      first_speed,
                                                      my_next,
                                                      my_next_vel,
                                                      my_body,
                                                      it->first, // next ball
                                                      it->second, // next ball vel
                                                      agent,
                                                      enforce )
                 )
            {
                //dlog.addText( Logger::KICK,
                //              "  --> success  final achieved vel=(%f, %f) r=%f",
                //              sol_vel.x, sol_vel.y, sol_vel.r() );
                Vector2D bvel = it->second / ServerParam::i().ballDecay();
                if ( ( enforce && sol_vel.r2() > max_achieved_vel.r2() )
                     || ( ! enforce // lower speed makes lower noise
                          && sol_vel.r2() >= max_achieved_vel.r2() - 0.0001
                          && sol_next_ball_vel.r2() > bvel.r2() ) )
                {
                    found = true;
                    max_achieved_vel = sol_vel;
                    sol_next_ball_vel = bvel;

                    //dlog.addText( Logger::KICK,
                    //"      --> updated" );
                }
            }
            //else
            //{
            //    dlog.addText( Logger::KICK,
            //                  "  --> failed ");
            //}
        }
    }


    if ( ! found )
    {
        //dlog.addText( Logger::KICK,
        //" three step not found" );
        return false;
    }

    if ( achieved_vel )
    {
        *achieved_vel = max_achieved_vel;
    }
    if ( next_vel )
    {
        *next_vel = sol_next_ball_vel;
    }

    //dlog.addText( Logger::KICK,
    //" three step found" );
    return true;
}

}
