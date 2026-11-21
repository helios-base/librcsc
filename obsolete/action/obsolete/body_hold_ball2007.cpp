// -*-c++-*-

/*!
  \file body_hold_ball2007.cpp
  \brief stay there and keep the ball from opponent players.
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

#include "body_hold_ball2007.h"

#include "basic_actions.h"
#include "body_kick_to_relative.h"
#include "body_stop_ball.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/geom/line_2d.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_HoldBall2007::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": Body_HoldBall2007" );

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

    if ( ! wm.ball().velValid() )
    {
        return Body_StopBall().execute( agent );
    }

    const double front_keep_dist
        = agent->world().self().playerType().playerSize()
        + ServerParam::i().ballSize()
        + 0.15;

    if ( avoidOpponent( agent, front_keep_dist ) )
    {
        return true;
    }

    if ( M_kick_target_point.isValid() )
    {
        if ( keepReverse( agent ) )
        {
            return true;
        }
    }

    if ( turnToPoint( agent ) )
    {
        return true;
    }

    return keepFront( agent, front_keep_dist );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_HoldBall2007::avoidOpponent( PlayerAgent * agent,
                                  const double & front_keep_dist )
{
    const WorldModel & wm = agent->world();

    ////////////////////////////////////////////////////////////////////
    // check opponent
    const PlayerObject * nearest_opp = wm.getOpponentNearestToSelf( 3 );

    if ( ! nearest_opp
         || nearest_opp->distFromBall() > 5.0 )
    {
        return false;
    }

    const double safety_dist_front
        = ( nearest_opp->goalie()
            ? ServerParam::i().catchAreaLength()
            : ServerParam::i().defaultKickableArea() )
        + ServerParam::i().defaultPlayerSpeedMax()
        + 0.1;

    const double safety_dist_ball
        = ( nearest_opp->goalie()
            ? ServerParam::i().catchAreaLength()
            : ServerParam::i().tackleDist() )
        + ServerParam::i().defaultPlayerSpeedMax()
        + 0.2;

    const Vector2D my_next = wm.self().pos() + wm.self().vel();

    const Vector2D my_front
        = my_next
        + Vector2D::polar2vector( front_keep_dist, wm.self().body() );

    double opp_to_self_dist = nearest_opp->pos().dist( my_next );
    double opp_to_front_dist = nearest_opp->pos().dist( my_front );
    double opp_to_ball_dist = nearest_opp->pos().dist( wm.ball().pos()
                                                       + wm.ball().vel() );

    ////////////////////////////////////////////////////////////////////

    if ( opp_to_ball_dist > safety_dist_ball )
    {
        if ( turnToPoint( agent ) )
        {
            agent->debugClient().addMessage( "Hold:avTurn1" );
            return true;
        }
    }

    if ( opp_to_front_dist > safety_dist_front )
    {
        if ( keepFront( agent, front_keep_dist ) )
        {
            agent->debugClient().addMessage( "Hold:avFront1" );
            return true;
        }
    }

    // exist dangerous opponent

    ////////////////////////////////////////////////////////////////////
    // avoid opponent move line
    if ( avoidOpponentLine( agent, nearest_opp ) )
    {
        return true;
    }

    ////////////////////////////////////////////////////////////////////
    // calc keep angle
    const AngleDeg opp_angle = ( nearest_opp->pos() - my_next ).th();

    double keep_side = ( opp_angle.isLeftOf( wm.ball().angleFromSelf() )
                         ? 1.0 : -1.0 );

    double added_angle
        = AngleDeg::asin_deg( (ServerParam::i().defaultKickableArea()
                               - ServerParam::i().defaultPlayerSize() * 3.0)
                              / ServerParam::i().defaultKickableArea() );
    added_angle = 180.0 - std::floor( std::fabs( added_angle ) );

    AngleDeg keep_angle = opp_angle + (added_angle * keep_side);

    double dir_diff = ( keep_angle - wm.ball().angleFromSelf() ).abs();
    if ( dir_diff < 2.0 )
    {
        // ball is alredy there. flip keep side.
        keep_angle = opp_angle + (added_angle * -keep_side);
        dir_diff = ( keep_angle - wm.ball().angleFromSelf() ).abs();
    }

    ////////////////////////////////////////////////////////////////////
    // calc keep dist
    const double my_kickable = wm.self().playerType().kickableArea();
    double keep_dist = my_kickable - 0.2;

    if ( dir_diff < 10.0 )
    {
        if  ( opp_to_self_dist < my_kickable
              && wm.ball().distFromSelf() > my_kickable - 0.2 )
        {
            keep_dist = my_kickable + 0.1;
            dlog.addText( Logger::ACTION,
                          __FILE__": avoidOpponent() slightly kick out."
                          " keep_dist=%.2f keep_angle=%.0f",
                          keep_dist, keep_angle.degree() );
        }
        else
        {
            keep_dist
                = wm.self().playerType().playerSize()
                + wm.self().playerType().kickableMargin()
                - 0.06;
            dlog.addText( Logger::ACTION,
                          __FILE__": avoidOpponent() at kickable edge."
                          " keep_dist=%.2f keep_angle=%.0f",
                          keep_dist, keep_angle.degree() );
        }
    }
    else
    {
        keep_dist = wm.self().playerType().kickableArea() - 0.0001;

        Vector2D tmp_pos = wm.self().vel() + Vector2D::polar2vector( keep_dist,
                                                                     keep_angle );
        double ball_travel = ( wm.ball().rpos() - tmp_pos ).r();
        AngleDeg ball_angle = wm.ball().angleFromSelf() - wm.self().body();
        double ball_dist = wm.ball().distFromSelf()
            - wm.self().playerType().playerSize()
            - ServerParam::i().ballSize();
        double pos_rand
            = 0.5
            * 0.25 * ( ball_angle.abs() / 180.0
                       + ball_dist / wm.self().playerType().kickableMargin() );
        double speed_rand
            = 0.5 * 0.5 * wm.ball().vel().r() / ( ServerParam::i().ballSpeedMax()
                                                  * ServerParam::i().ballDecay() );
        Vector2D accel = ( tmp_pos - wm.ball().rpos() - wm.ball().vel() );
        double kick_power = accel.r() / wm.self().kickRate();
        double kick_rand
            = wm.self().playerType().kickRand()
            * ( kick_power / ServerParam::i().maxPower() )
            * ( pos_rand + speed_rand );
        dlog.addText( Logger::ACTION,
                      __FILE__": avoidOpponent() pos_rand=%.3f speed_rand=%.3f kick_rand=%.3f",
                      pos_rand, speed_rand, kick_rand );
        keep_dist
            = wm.self().playerType().kickableArea()
            - wm.self().vel().r() * ServerParam::i().playerRand()
            - ball_travel * ServerParam::i().ballRand()
            - kick_rand
            - 0.2;
        dlog.addText( Logger::ACTION,
                      __FILE__": avoidOpponent() set buffer. keep_dist=%.2f. keep_angle=%.0f"
                      " ball_travel= %.2f",
                      keep_dist, keep_angle.degree(), ball_travel );
    }

    ////////////////////////////////////////////////////////////////////
    // execute action
    Vector2D keep_pos
        = wm.self().pos()
        + wm.self().vel()
        + Vector2D::polar2vector( keep_dist, keep_angle );

    agent->debugClient().addMessage( "Hold:%.1f", keep_angle.degree() );
    agent->debugClient().addCircle( keep_pos, 0.05 );

    Body_KickToRelative( keep_dist,
                         keep_angle - wm.self().body(),
                         false // not stop the ball
                         ).execute( agent );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_HoldBall2007::avoidOpponentLine( PlayerAgent * agent,
                                      const PlayerObject * opponent )
{
    if ( opponent->distFromSelf() > agent->world().self().playerType().playerSize() * 2.0 + 0.2
         && ( opponent->bodyCount() == 0
              || ( opponent->velCount() <= 1 && opponent->vel().r() > 0.2 ) )
         )
    {
        const WorldModel & wm = agent->world();

        Line2D opp_line( opponent->pos() + opponent->vel(),
                         ( opponent->bodyCount() == 0
                           ? opponent->body()
                           : opponent->vel().th() ) );
        Vector2D my_next = wm.self().pos() + wm.self().vel();
        Vector2D proj_pos = opp_line.projection( my_next );
        AngleDeg keep_angle = ( my_next - proj_pos ).th();
        double keep_dist = wm.self().playerType().kickableArea() - 0.0001;
        Vector2D keep_pos = my_next + Vector2D::polar2vector( keep_dist,
                                                              keep_angle );
        double ball_travel = wm.ball().pos().dist( keep_pos );
        AngleDeg ball_angle = wm.ball().angleFromSelf() - wm.self().body();
        double ball_dist = wm.ball().distFromSelf()
            - wm.self().playerType().playerSize()
            - ServerParam::i().ballSize();
        double pos_rand
            = 0.5
            * 0.25 * ( ball_angle.abs() / 180.0
                       + ball_dist / wm.self().playerType().kickableMargin() );
        double speed_rand
            = 0.5
            * 0.5 * wm.ball().vel().r() / ( ServerParam::i().ballSpeedMax()
                                            * ServerParam::i().ballDecay() );
        Vector2D accel = ( keep_pos - wm.ball().pos() - wm.ball().vel() );
        double kick_power = accel.r() / wm.self().kickRate();
        double kick_rand
            = wm.self().playerType().kickRand()
            * ( kick_power / ServerParam::i().maxPower() )
            * ( pos_rand + speed_rand );

        dlog.addText( Logger::ACTION,
                      __FILE__": avoidOpponentLine() pos_rand=%.3f speed_rand=%.3f kick_rand=%.3f",
                      pos_rand, speed_rand, kick_rand );

        keep_dist
            = wm.self().playerType().kickableArea()
            - wm.self().vel().r() * ServerParam::i().playerRand()
            - ball_travel * ServerParam::i().ballRand()
            - kick_rand
            - 0.2;
        keep_pos = my_next
            + Vector2D::polar2vector( keep_dist,
                                      keep_angle );

        dlog.addText( Logger::ACTION,
                      __FILE__": avoid opponent line. keep_pos=(%.2f %.2f) dist=%.2f angle=%.1f",
                      keep_pos.x, keep_pos.y,
                      keep_dist,
                      keep_angle.degree() );
        if ( opponent->pos().dist( keep_pos )
             > ServerParam::i().defaultKickableArea() + 0.2 )
        {
            agent->debugClient().addMessage( "HoldAvoidOppBody%.0f",
                                             keep_angle.degree() );
            agent->debugClient().addCircle( keep_pos, 0.05 );
            dlog.addText( Logger::ACTION,
                          __FILE__": done avoid opponent line. " );

            Body_KickToRelative( keep_dist,
                                 keep_angle - wm.self().body(),
                                 false // not need to stop the ball
                                 ).execute( agent );
            return true;
        }
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_HoldBall2007::keepFront( PlayerAgent * agent,
                              const double & front_keep_dist )
{
    dlog.addText( Logger::ACTION,
                  __FILE__": keep at front dist=%.2f",
                  front_keep_dist );
    agent->debugClient().addMessage( "HoldFront" );

    return Body_KickToRelative( front_keep_dist,
                                0.0, // just front of body -> relative angle == 0
                                true // stop ball
                                ).execute( agent );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_HoldBall2007::keepReverse( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();

    Vector2D my_inertia = wm.self().inertiaFinalPoint();
    if ( ! my_inertia.isValid() )
    {
        my_inertia = wm.self().pos();
    }

    AngleDeg keep_angle = ( my_inertia - M_kick_target_point ).th();

    double keep_dist
        = wm.self().playerType().playerSize()
        + wm.self().playerType().kickableMargin() * 0.6
        + ServerParam::i().ballSize();
    keep_dist = std::min( wm.self().playerType().kickableArea() - 0.3,
                          keep_dist );

    Vector2D keep_pos
        = wm.self().pos()
        + wm.self().vel()
        + Vector2D::polar2vector( keep_dist, keep_angle );

    dlog.addText( Logger::ACTION,
                  __FILE__": keep reverse side. dist=%.2f angle=%.0f",
                  keep_dist, keep_angle.degree() );
    agent->debugClient().addMessage( "HoldReverse" );
    agent->debugClient().addCircle( keep_pos, 0.05 );

    return Body_KickToRelative( keep_dist,
                                keep_angle - wm.self().body(),
                                true // stop ball
                                ).execute( agent );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Body_HoldBall2007::turnToPoint( PlayerAgent * agent )
{
    const int cycle = 100;
    const WorldModel & wm = agent->world();

    Vector2D face_point( ServerParam::i().pitchHalfLength() - 5.0,
                         0.0 );
    if ( M_do_turn )
    {
        face_point = M_turn_target_point;

        dlog.addText( Logger::ACTION,
                      "%s:%d: turnToPoint. face target=(%.1f, %.1f)"
                      ,__FILE__, __LINE__,
                      face_point.x, face_point.y );
    }

    Vector2D my_point = wm.self().inertiaPoint( cycle );
    AngleDeg target_angle = ( face_point - my_point ).th();

    // ball kickabel at next cycle
    // turn to target dir
    if ( ( wm.self().body() - target_angle ).abs() > 5.0 )
    {
        Vector2D next_ball_rel = wm.ball().rpos();
        next_ball_rel += wm.ball().vel();
        next_ball_rel -= wm.self().vel();
        double next_ball_dist = next_ball_rel.r();

        if ( next_ball_dist < ( wm.self().playerType().kickableArea()
                                - wm.self().vel().r() * ServerParam::i().playerRand()
                                - wm.ball().vel().r() * ServerParam::i().ballRand()
                                - 0.15 )
             )
        {
            dlog.addText( Logger::ACTION,
                          "%s:%d: turnToPoint. next_ball_dist=%.2f  turn-to=(%.1f, %.1f)"
                          ,__FILE__, __LINE__,
                          next_ball_dist, face_point.x, face_point.y );
            agent->debugClient().addMessage( "Hold:Turn" );
            Body_TurnToPoint( face_point, cycle ).execute( agent );
            return true;
        }
    }

    return false;
}

}
