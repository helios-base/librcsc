// -*-c++-*-

/*!
  \file body_kick_two_step.cpp
  \brief two step kick behavior to accelerate the ball to the desired
  speed.
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

#include "body_kick_two_step.h"

#include "body_kick_one_step.h"
#include "body_hold_ball.h"
#include "body_kick_to_relative.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/circle_2d.h>
#include <rcsc/geom/ray_2d.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

namespace rcsc {

const double Body_KickTwoStep::DEFAULT_MIN_DIST2 = 10000.0;

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickTwoStep::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::KICK,
                  __FILE__": Body_KickTwoStep" );

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

    //---------------------------------------------------

    Vector2D target_rpos = M_target_point - wm.self().pos();
    M_first_speed = std::min( M_first_speed, ServerParam::i().ballSpeedMax() );

    Vector2D achieved_vel;

    if ( simulate_one_kick( &achieved_vel,
                            NULL,
                            NULL,
                            target_rpos,
                            M_first_speed,
                            Vector2D( 0.0, 0.0 ), // my current pos
                            wm.self().vel(),
                            wm.self().body(),
                            wm.ball().rpos(),
                            wm.ball().vel(),
                            agent,
                            false ) ) // not enforce
    {
        Vector2D accel = achieved_vel - wm.ball().vel();
        double kick_power = accel.r() / wm.self().kickRate();
        AngleDeg kick_dir = accel.th() - wm.self().body();

        dlog.addText( Logger::KICK,
                      __FILE__": only one step. result=(%.3f, %.3f) r=%.3f"
                      " accel=(%.3f, %.3f) power=%.1f dir=%.1f",
                      achieved_vel.x, achieved_vel.y, achieved_vel.r(),
                      accel.x, accel.y,
                      kick_power, kick_dir.degree() );
        M_ball_result_pos = wm.ball().pos() + achieved_vel;
        M_ball_result_vel = achieved_vel * ServerParam::i().ballDecay();;
        M_kick_step = 1;

        return agent->doKick( kick_power, kick_dir );
    }


    Vector2D next_vel;
    if ( simulate_two_kick( &achieved_vel,
                            &next_vel,
                            target_rpos,
                            M_first_speed,
                            Vector2D( 0.0, 0.0 ), // my current pos
                            wm.self().vel(),
                            wm.self().body(),
                            wm.ball().rpos(),
                            wm.ball().vel(),
                            agent,
                            M_enforce_kick ) )
    {
        M_kick_step = 2;

        if ( M_enforce_kick
             && next_vel.r2() < square( M_first_speed ) )
        {
            Vector2D one_kick_max_vel
                = Body_KickOneStep::get_max_possible_vel( ( target_rpos - wm.ball().rpos() ).th(),
                                                          wm.self().kickRate(),
                                                          wm.ball().vel() );
            if ( one_kick_max_vel.r2() > next_vel.r2() )
            {
                next_vel = one_kick_max_vel;
                M_kick_step = 1;
            }

            if ( next_vel.r() < M_first_speed * 0.8 )
            {
                dlog.addText( Logger::KICK,
                              __FILE__": failed enforce kick. hold ball" );
                M_kick_step = 0;

                return Body_HoldBall( true, // turn to target
                                      M_target_point,
                                      ( wm.gameMode().type() == GameMode::PlayOn
                                        ? M_target_point
                                        : Vector2D::INVALIDATED )
                                      ).execute( agent );
            }
        }

        Vector2D accel = next_vel - wm.ball().vel();
        double kick_power = accel.r() / wm.self().kickRate();
        AngleDeg kick_dir = accel.th() - wm.self().body();

        dlog.addText( Logger::KICK,
                      __FILE__": two step. result=(%.3f, %.3f)r=%.3f"
                      " next_vel=(%.3f, %.3f)r=%.3f"
                      " accel=(%.3f, %.3f) power=%.1f dir=%.1f",
                      achieved_vel.x, achieved_vel.y, achieved_vel.r(),
                      next_vel.x, next_vel.y, next_vel.r(),
                      accel.x, accel.y,
                      kick_power, kick_dir.degree() );
        agent->debugClient().addCircle( wm.ball().pos() + next_vel,
                                        0.05 );
        M_ball_result_pos = wm.ball().pos() + next_vel;
        M_ball_result_vel = next_vel * ServerParam::i().ballDecay();

        return agent->doKick( kick_power, kick_dir );
    }

    dlog.addText( Logger::KICK,
                  __FILE__": failed. hold ball" );

    return Body_HoldBall( true, // turn to target
                          M_target_point,
                          ( wm.gameMode().type() == GameMode::PlayOn
                            ? M_target_point  // keep reverse side
                            : Vector2D::INVALIDATED )
                          ).execute( agent );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickTwoStep::is_opp_kickable( const PlayerAgent * agent,
                                   const Vector2D & rel_pos,
                                   double * min_dist2 )
{
    // use default value
    static const double KICKABLE2
        = square( ServerParam::i().defaultKickableArea() + 0.17 );

    const PlayerPtrCont::const_iterator end = agent->world().opponentsFromSelf().end();
    for ( PlayerPtrCont::const_iterator it = agent->world().opponentsFromSelf().begin();
          it != end;
          ++it )
    {
        if ( (*it)->distFromSelf() > 6.0 )
        {
            break;
        }

        if ( (*it)->posCount() >= 2 )
        {
            continue;
        }

        double d2 = rel_pos.dist2( (*it)->rpos() + (*it)->vel() );
        if ( min_dist2 && d2 < *min_dist2 )
        {
            *min_dist2 = d2;
        }

        if ( d2 < KICKABLE2 )
        {
#ifdef DEBUG
            dlog.addText( Logger::KICK,
                          __FILE__": is_opp_kickable() detect kickable opp. rpos(%.2f %.2f)",
                          rel_pos.x, rel_pos.y );
#endif
            return true;
        }
        // check opponent goalie
        else if ( (*it)->goalie()
                  && (*it)->pos().x > ServerParam::i().theirPenaltyAreaLineX() + 1.0
                  && ( (*it)->pos().absY()
                       < ServerParam::i().penaltyAreaHalfWidth() - 1.0 )
                  && ( rel_pos.dist( (*it)->rpos() + (*it)->vel() )
                       < ServerParam::i().catchAreaLength() )
                  )
        {
            return true;
        }
    }

#ifdef DEBUG
    dlog.addText( Logger::KICK,
                  __FILE__": is_opp_kickable() No kickable opp. for rpos(%.2f %.2f)",
                  rel_pos.x, rel_pos.y );
#endif
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickTwoStep::simulate_one_kick( Vector2D * achieved_vel,
                                     double * kick_power,
                                     double * opp_dist2,
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
    //"Kick Search: one step @~@~@~@~@~@~@~@~@~@~@~@~@~@" );

    Vector2D ball_rel_at_here = ball_rpos - my_rpos;
    const double krate
        = kick_rate( ball_rel_at_here.r(),
                     ( ball_rel_at_here.th() - my_body ).degree(),
                     ServerParam::i().kickPowerRate(),
                     ServerParam::i().ballSize(),
                     agent->world().self().playerType().playerSize(),
                     agent->world().self().playerType().kickableMargin() );
    const double max_accel = std::min( ServerParam::i().maxPower() * krate,
                                       ServerParam::i().ballAccelMax() );

    Vector2D required_vel
        = Vector2D::polar2vector( first_speed, ( target_rpos - ball_rpos ).th() );
    double required_accel = ( required_vel - ball_vel ).r();
    double required_power = required_accel / krate;

    // can NOT reach the target vel
    if ( required_accel > max_accel )
    {
        if ( ! enforce )
        {
            return false;
        }

        dlog.addText( Logger::KICK,
                      "%s:%d: simulate_one_kick. never reach. try enforce kick"
                      ,__FILE__, __LINE__ );
        required_vel
            = Body_KickOneStep::get_max_possible_vel( (target_rpos - ball_rpos).th(),
                                                      krate,
                                                      ball_vel );
    }

    // check collision & opponents
    double tmp_opp_dist2 = DEFAULT_MIN_DIST2;
    Vector2D next_ball_rpos = ball_rpos + required_vel;
    if ( next_ball_rpos.dist2( my_rpos + my_vel )
         < square( agent->world().self().playerType().playerSize()
                   + ServerParam::i().ballSize()
                   + 0.1 )
         || is_opp_kickable( agent, next_ball_rpos, &tmp_opp_dist2 ) )
    {
        return false;
    }


    if ( achieved_vel )
    {
        *achieved_vel = required_vel;
    }

    if ( kick_power )
    {
        *kick_power = required_power;
    }

    if ( opp_dist2 )
    {
        *opp_dist2 = tmp_opp_dist2;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_KickTwoStep::simulate_two_kick( Vector2D * achieved_vel,
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
    //" Kick Search: two step !_!_!_!_!_!_!_!_!_!_!_!_!" );

    Vector2D ball_rel_at_here = ball_rpos - my_rpos;
    const double krate
        = kick_rate( ball_rel_at_here.r(),
                     ( ball_rel_at_here.th() - my_body ).degree(),
                     ServerParam::i().kickPowerRate(),
                     ServerParam::i().ballSize(),
                     agent->world().self().playerType().playerSize(),
                     agent->world().self().playerType().kickableMargin() );
    const double max_accel = std::min( ServerParam::i().maxPower() * krate,
                                       ServerParam::i().ballAccelMax() );

    const Vector2D my_next = my_rpos + my_vel;
    const double my_kickable = agent->world().self().playerType().kickableArea();

    // next ball position & vel
    //std::vector< std::pair< Vector2D, Vector2D > > subtargets;
    std::vector< SubTarget > subtargets;

    //////////////////////////////////////////////////////////////////
    // first element is next kickable edge
    {
        const Ray2D desired_ray( ball_rpos, ( target_rpos - ball_rpos ).th() );
        const Circle2D next_kickable_circle( my_next, my_kickable - 0.1 );

        // get intersection next kickable circle & desired ray
        // solutions are next ball pos, relative to current my pos
        Vector2D sol1, sol2;
        int num = next_kickable_circle.intersection( desired_ray, &sol1, &sol2 );
        Vector2D required_vel( Vector2D::INVALIDATED );

        if ( num == 1 )
        {
            // if ball point is not within next kicable area.
            // it is very dangerous to kick to the kickable edge.
            if ( next_kickable_circle.contains( ball_rpos ) )
            {
                required_vel = sol1 - ball_rpos;
            }
        }
        else if ( num == 2 )
        {
            // current ball point is NOT within next kicable area.
            Vector2D v1 = sol1 - ball_rpos;
            Vector2D v2 = sol2 - ball_rpos;
            // set bigger one
            required_vel = ( v1.r2() > v2.r2() ? v1 : v2 );
        }

        if ( required_vel.valid() )
        {
            double d = required_vel.r();
            double ball_noise = d * ServerParam::i().ballRand() * 1.412;
            double self_noise = my_vel.r() * ServerParam::i().playerRand();
            d = std::max( d - ball_noise - self_noise - 0.15, 0.0 );
            required_vel.setLength( d );
        }

        double opp_dist2 = DEFAULT_MIN_DIST2;
        if ( required_vel.valid()
             && ! is_opp_kickable( agent, ball_rpos + required_vel, &opp_dist2 )
             && ( required_vel - ball_vel ).r() < max_accel )
        {
            double d = required_vel.r();
            if ( d > ServerParam::i().ballSpeedMax() )
            {
                required_vel *= ServerParam::i().ballSpeedMax() / d;
            }

            // add element
            subtargets.emplace_back( ball_rpos + required_vel,
                                     required_vel * ServerParam::i().ballDecay(),
                                     opp_dist2 );
        }
    }

    //////////////////////////////////////////////////////////////////
    // generate other subtargets
    {
        const double subtarget_dist = std::max( my_kickable * 0.7,
                                                my_kickable - 0.35 );
        const double default_dir_inc = 30.0;

        const AngleDeg angle_self_to_target = ( target_rpos - my_next ).th();
        const double ball_target_dir_diff
            = ( angle_self_to_target - ( ball_rpos - my_next ).th() ).abs();

        // sub-targets should be more forward than ball
        double inc = ball_target_dir_diff / 5.0;
        inc = std::max( inc, default_dir_inc );
        inc = std::min( inc, ball_target_dir_diff );
        for ( double d = -ball_target_dir_diff;
              d <= ball_target_dir_diff + 1.0;
              d += inc )
        {

            Vector2D sub // rel to current my pos
                = my_next
                + Vector2D::polar2vector( subtarget_dist,
                                          angle_self_to_target + d );

            double opp_dist2 = DEFAULT_MIN_DIST2;
            Vector2D require_vel = sub - ball_rpos;
            if ( ! is_opp_kickable( agent, sub, &opp_dist2 )
                 && (require_vel - ball_vel).r() < max_accel )
            {
                subtargets.emplace_back( sub,
                                         require_vel * ServerParam::i().ballDecay(),
                                         opp_dist2 );
            }
        }
    }

    //////////////////////////////////////////////////////////////////
    // subtargets are next ball position
    Vector2D max_achieved_vel( 0.0, 0.0 );
    Vector2D sol_next_ball_vel( 100.0, 100.0 );
    bool found = false;
    {
        double min_kick_power = ServerParam::i().maxPower() + 0.1;
        double min_opp_dist2 = 0.0;

        const Vector2D my_next_vel
            = my_vel * agent->world().self().playerType().playerDecay();

        const std::vector< SubTarget >::const_iterator end = subtargets.end();
        for ( std::vector< SubTarget >::const_iterator it = subtargets.begin();
              it != end;
              ++it )
        {
            //Vector2D tmp_rel = it->first - my_next;
            //dlog.addText( Logger::KICK,
            //              " check subtarget(%f, %f) r=%f th=%f",
            //              tmp_rel.x, tmp_rel.y,
            //              tmp_rel.r(), tmp_rel.th().degree() );
            Vector2D sol_vel;
            double kick_power = 1000000.0;
            double opp_dist2 = DEFAULT_MIN_DIST2;
            if ( simulate_one_kick( &sol_vel,
                                    &kick_power,
                                    &opp_dist2,
                                    target_rpos,
                                    first_speed,
                                    my_next,
                                    my_next_vel,
                                    my_body,
                                    it->ball_pos_, // next ball pos
                                    it->ball_vel_, // next ball vel
                                    agent,
                                    enforce )
                 )
            {
                bool update = true;
                if ( enforce
                     && sol_vel.r2() < max_achieved_vel.r2() )
                {
                    update = false;
                }

                if ( update )
                {
                    if ( it->opp_dist2_ != DEFAULT_MIN_DIST2
                         || min_opp_dist2 != 0.0 )
                    {
                        if ( it->opp_dist2_ == min_opp_dist2 )
                        {
                            if ( kick_power > min_kick_power )
                            {
                                update = false;
                            }
                        }
                        else if ( it->opp_dist2_ < min_opp_dist2 )
                        {
                            update = false;
                        }
                    }
                    else
                    {
                        if ( kick_power > min_kick_power )
                        {
                            update = false;
                        }
                    }
                }

                if ( update )
                {
                    found = true;
                    max_achieved_vel = sol_vel;
                    min_kick_power = kick_power;
                    min_opp_dist2 = it->opp_dist2_;
                    sol_next_ball_vel = it->ball_vel_ / ServerParam::i().ballDecay();
                    //dlog.addText( Logger::KICK,
                    //"      --> updated" );
                }
            }
            //else
            //{
            //    dlog.addText( Logger::KICK,
            //                  "  --> failed " );
            //}
        }

    }


    if ( ! found )
    {
        //dlog.addText( Logger::KICK,
        //" two step not found" );
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

    //TRACE(dlog.addText( Logger::KICK,
    //" two step found" ));
    return true;
}

}
