// -*-c++-*-

/*!
  \file body_intercept2007.cpp
  \brief ball chasing action including smart planning.
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

#include "body_intercept2007.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>

#include <rcsc/player/intercept_table.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Intercept2007::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  __FILE__": Body_Intercept2007" );

    const WorldModel & wm = agent->world();

    /////////////////////////////////////////////
    if ( doKickableOpponentCheck( agent ) )
    {
        return true;;
    }

    const InterceptTable * table = wm.interceptTable();

    /////////////////////////////////////////////
    if ( table->selfReachCycle() > 100 )
    {
        Vector2D final_point = wm.ball().inertiaFinalPoint();
        agent->debugClient().setTarget( final_point );

        dlog.addText( Logger::INTERCEPT,
                      __FILE__": no solution... Just go to ball end point (%.2f %.2f)",
                      final_point.x, final_point.y );
        agent->debugClient().addMessage( "InterceptNoSolution" );
        Body_GoToPoint( final_point,
                        2.0,
                        ServerParam::i().maxDashPower()
                        ).execute( agent );
        return true;
    }

    /////////////////////////////////////////////
    //InterceptInfo best_intercept = getBestIntercept( wm, table );
    InterceptInfo best_intercept = getBestIntercept_Test( wm, table );

    if ( ! best_intercept.isValid() )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": no intercept candidate" );
        return false;
    }

    dlog.addText( Logger::INTERCEPT,
                  __FILE__": solution size= %d. selected best cycle is %d"
                  " (turn:%d + dash:%d)",
                  table->selfCache().size(),
                  best_intercept.reachCycle(),
                  best_intercept.turnCycle(), best_intercept.dashCycle() );

//     if ( 3 <= best_intercept.reachCycle()
//          && best_intercept.reachCycle() < 6 )
//     {
//         agent->setViewAction( new View_Normal() );
//     }

    Vector2D target_point = wm.ball().inertiaPoint( best_intercept.reachCycle() );
    agent->debugClient().setTarget( target_point );

    if ( best_intercept.dashCycle() == 0 )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": can get the ball only by inertia move. Turn!" );

        Vector2D face_point = M_face_point;
        if ( ! face_point.valid() )
        {
            face_point.assign( 50.5, wm.self().pos().y * 0.75 );
        }

        agent->debugClient().addMessage( "InterceptTurnOnly" );
        Body_TurnToPoint( face_point,
                          best_intercept.reachCycle() ).execute( agent );
        return true;
    }

    /////////////////////////////////////////////
    if ( best_intercept.turnCycle() > 0 )
    {
        Vector2D my_inertia = wm.self().inertiaPoint( best_intercept.reachCycle() );
        AngleDeg target_angle = ( target_point - my_inertia ).th();
        if ( best_intercept.dashPower() < 0.0 )
        {
            // back dash
            target_angle -= 180.0;
        }

        dlog.addText( Logger::INTERCEPT,
                      __FILE__": turn.first.%s target_body_angle = %.1f",
                      ( best_intercept.dashPower() < 0.0 ? "BackMode" : "" ),
                      target_angle.degree() );
        agent->debugClient().addMessage( "InterceptTurn%d(%d/%d)",
                                         best_intercept.reachCycle(),
                                         best_intercept.turnCycle(),
                                         best_intercept.dashCycle() );

        return agent->doTurn( target_angle - wm.self().body() );
    }

    /////////////////////////////////////////////
    dlog.addText( Logger::INTERCEPT,
                  __FILE__": try dash. power=%.1f  target_point=(%.2f, %.2f)",
                  best_intercept.dashPower(),
                  target_point.x, target_point.y );

    if ( doWaitTurn( agent, target_point, best_intercept ) )
    {
        return true;
    }

    if ( M_save_recovery
         && ( wm.self().stamina()
              - ( best_intercept.dashPower() * ( best_intercept.dashPower() > 0.0
                                                 ? 1.0
                                                 : -2.0 )
                  )
              < ServerParam::i().recoverDecThrValue() + 1.0 )
         )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": insufficient stamina" );
        agent->debugClient().addMessage( "InterceptRecover" );
        agent->doTurn( 0.0 );
        return false;
    }

    return doInertiaDash( agent,
                          target_point,
                          best_intercept );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Intercept2007::doKickableOpponentCheck( PlayerAgent * agent )
{
    const WorldModel & wm = agent->world();
    if ( wm.ball().distFromSelf() < 2.0
         && wm.existKickableOpponent() )
    {
        const PlayerObject * opp = wm.opponentsFromBall().front();
        if ( opp )
        {
            dlog.addText( Logger::INTERCEPT,
                          __FILE__": attack to opponent" );
            Vector2D attack_pos = opp->pos() + opp->vel();

            Body_GoToPoint( attack_pos,
                            0.1,
                            ServerParam::i().maxDashPower(),
                            -1.0, // speed
                            1, // cycle
                            true, // save recovery
                            15.0  // dir thr
                            ).execute( agent );
            return true;
        }
    }
    return false;
}

#if 0
/*-------------------------------------------------------------------*/
/*!

*/
InterceptInfo
Body_Intercept2007::getBestIntercept( const WorldModel & wm,
                                      const InterceptTable * table ) const
{
    const std::vector< InterceptInfo > & cache = table->selfCache();

    if ( cache.empty() )
    {
        return InterceptInfo( InterceptInfo::NORMAL,
                              100, 100, 100 );
    }

    const int opp_cycle = table->opponentReachCycle();

    const rcsc::Vector2D goal_pos( 65.0, 0.0 );

    std::size_t best_idx = 1000;
    double min_goal_dist2 = 100000.0;
    double min_dist2 = 40000.0;
    bool found_forward = false;

    const std::size_t max_idx = cache.size();
    for ( std::size_t i = 0; i < max_idx; ++i )
    {
        if ( M_save_recovery
             && cache[i].mode() != InterceptInfo::NORMAL )
        {
            continue;
        }

        const int cycle = cache[i].reachCycle();
        const Vector2D ball_pos = wm.ball().inertiaPoint( cycle );

        if ( ball_pos.absX() > ServerParam::i().pitchHalfLength() - 1.0
             || ball_pos.absY() > ServerParam::i().pitchHalfWidth() - 1.0 )
        {
            continue;
        }

        const Vector2D ball_vel
            = wm.ball().vel()
            * std::pow( ServerParam::i().ballDecay(), cycle );

        bool attacker = false;
        if ( ball_vel.x > 1.0
             && cache[i].dashPower() > 0.0
             && ( ball_pos.x > 40.0
                  || ball_pos.x > wm.offsideLineX() )
             )
        {
            attacker = true;
        }


        int opp_buf = 4;

        if ( attacker )
        {
            opp_buf = 1;
        }

        if ( cycle >= opp_cycle - opp_buf )
        {
            continue;
        }

        if ( attacker )
        {
            if ( best_idx < 100 )
            {
                if ( ball_pos.x > 47.0
                     || std::fabs( ball_pos.y - wm.self().pos().y ) > 10.0
                     )
                {
                    continue;
                }
            }

            double goal_dist2 = ball_pos.dist2( goal_pos );
            if ( goal_dist2 < min_goal_dist2 )
            {
                min_goal_dist2 = goal_dist2;
                min_dist2 = wm.self().pos().dist2( ball_pos );
                best_idx = i;
                found_forward = true;

                dlog.addText( Logger::INTERCEPT,
                              __FILE__": getBestIntercept. update attacker cycle = %d",
                              cycle );
                continue;
            }

            if ( best_idx < 100 )
            {
                continue;
            }
        }

        if ( ball_vel.x > 0.5
             && ball_pos.x > wm.offsideLineX() - 15.0
             && ball_vel.r() > 0.9
             && cycle <= opp_cycle - 5 )
        {
            double goal_dist2 = ball_pos.dist2( goal_pos );
            if ( goal_dist2 < min_goal_dist2 )
            {
                min_goal_dist2 = goal_dist2;
                min_dist2 = wm.self().pos().dist2( ball_pos );
                best_idx = i;
                found_forward = true;

                dlog.addText( Logger::INTERCEPT,
                              __FILE__": getBestIntercept. update attacker"
                              " cycle = %d",
                              cycle );
                continue;
            }
        }

        if ( found_forward )
        {
            continue;
        }

        // can get the ball only by inertia move
        if ( best_idx > 100
             && cache[i].dashCycle() == 0 )
        {
            min_goal_dist2 = ball_pos.dist2( goal_pos );
            min_dist2 = wm.self().pos().dist2( ball_pos );
            best_idx = i;

            dlog.addText( Logger::INTERCEPT,
                          __FILE__": getBestIntercept. update. no dash cycle = %d",
                          cycle );
            //break;
            continue;
        }

        // no turn is recommended
        if ( best_idx > 100
             && ball_vel.x > 0.0
             && cache[i].turnCycle() == 0 )
        {
            min_goal_dist2 = ball_pos.dist2( goal_pos );
            min_dist2 = wm.self().pos().dist2( ball_pos ) - 1.0;
            best_idx = i;

            dlog.addText( Logger::INTERCEPT,
                          __FILE__": getBestIntercept. update. no turn cycle = %d",
                          cycle );
            //break;
            continue;
        }


        // select the nearest pointa
        double d2 = wm.self().pos().dist2( ball_pos );
        if ( d2 < min_dist2 )
        {
            min_goal_dist2 = ball_pos.dist2( goal_pos );
            min_dist2 = d2;
            best_idx = i;

            dlog.addText( Logger::INTERCEPT,
                          __FILE__": getBestIntercept. update. nearest cycle = %d",
                          cycle );
        }
    }

    // found
    if ( best_idx < cache.size() )
    {
        return cache[best_idx];
    }

    // select the first one
    for ( std::size_t i = 0; i < max_idx; ++i )
    {
        if ( M_save_recovery
             && cache[i].mode() != InterceptInfo::NORMAL )
        {
            continue;
        }

        return cache[i];
    }

    return cache[0];
}
#endif

/*-------------------------------------------------------------------*/
/*!

*/
InterceptInfo
Body_Intercept2007::getBestIntercept_Test( const WorldModel & wm,
                                           const InterceptTable * table ) const
{
    const std::vector< InterceptInfo > & cache = table->selfCache();

    if ( cache.empty() )
    {
        return InterceptInfo();
    }

    const rcsc::Vector2D goal_pos( 65.0, 0.0 );
    const double speed_max
        = wm.self().playerType().realSpeedMax();

    const int opp_cycle = table->opponentReachCycle();


    const InterceptInfo * attacker_best = nullptr;
    double attacker_score = 0.0;

    const InterceptInfo * forward_best = nullptr;
    double forward_score = 0.0;

    const InterceptInfo * noturn_best = nullptr;
    double noturn_score = 10000.0;

    const InterceptInfo * nearest_best = nullptr;
    double nearest_score = 10000.0;



    //std::vector< double > scores( cache.size() 0.0 );

    const std::size_t MAX = cache.size();
    for ( std::size_t i = 0; i < MAX; ++i )
    {
        if ( M_save_recovery
             && cache[i].mode() != InterceptInfo::NORMAL )
        {
            //scores[i] = -1000.0;
            continue;
        }

        const int cycle = cache[i].reachCycle();
        const Vector2D ball_pos = wm.ball().inertiaPoint( cycle );

        if ( ball_pos.absX() > ServerParam::i().pitchHalfLength() - 1.0
             || ball_pos.absY() > ServerParam::i().pitchHalfWidth() - 1.0 )
        {
            //scores[i] = -1000.0;
            continue;
        }

       const Vector2D ball_vel
            = wm.ball().vel()
            * std::pow( ServerParam::i().ballDecay(), cycle );


        bool attacker = false;
        if ( ball_vel.x > speed_max
             && cache[i].dashPower() >= 0.0
             && ball_pos.x < 47.0
             && std::fabs( ball_pos.y - wm.self().pos().y ) < 10.0
             && ( ball_pos.x > 40.0
                  || ball_pos.x > wm.offsideLineX() )
             )
        {
            attacker = true;
        }

        const int opp_buf = ( attacker ? 1 : 5 );

        if ( cycle >= opp_cycle - opp_buf )
        {
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "___ %d failed: cycle=%d pos=(%.1f %.1f) turn=%d dash=%d  opp_cycle=%d",
                          i, cycle,
                          ball_pos.x, ball_pos.y,
                          cache[i].turnCycle(), cache[i].dashCycle(),
                          opp_cycle );
#endif
            continue;
        }

        // attacker type

        if ( attacker )
        {
            double goal_dist = 100.0 - std::min( 100.0, ball_pos.dist( goal_pos ) );
            double x_diff = 47.0 - ball_pos.x;

            double score
                = ( goal_dist / 100.0 )
                * std::exp( - ( x_diff * x_diff ) / ( 2.0 * 100.0 ) );
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "___ %d attacker cycle=%d pos=(%.1f %.1f) turn=%d dash=%d score=%f",
                          i, cycle,
                          ball_pos.x, ball_pos.y,
                          cache[i].turnCycle(), cache[i].dashCycle(),
                          score );
#endif
            if ( score > attacker_score )
            {
                attacker_best = &cache[i];
                attacker_score = score;
            }

            continue;
        }

        // no turn type

        if ( cache[i].turnCycle() == 0 )
        {
            //double score = ball_pos.x;
            double score = wm.self().pos().dist2( ball_pos );
            //if ( ball_vel.x > 0.0 )
            //{
            //    score *= std::exp( - std::pow( ball_vel.r() - 1.0, 2.0 )
            //                       / ( 2.0 * 1.0 ) );
            //}
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "___ %d noturn cycle=%d pos=(%.1f %.1f) turn=%d dash=%d score=%f",
                          i, cycle,
                          ball_pos.x, ball_pos.y,
                          cache[i].turnCycle(), cache[i].dashCycle(),
                          score );
#endif
            if ( score < noturn_score )
            {
                noturn_best = &cache[i];
                noturn_score = score;
            }

            continue;

        }

        // forward type

        if ( ball_vel.x > 0.5
             && ball_pos.x > wm.offsideLineX() - 15.0
             && ball_vel.r() > speed_max * 0.98
             && cycle <= opp_cycle - 5 )
        {
            double score
                = ( 100.0 * 100.0 )
                - std::min( 100.0 * 100.0, ball_pos.dist2( goal_pos ) );
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "___ %d forward cycle=%d pos=(%.1f %.1f) turn=%d dash=%d score=%f",
                          i, cycle,
                          ball_pos.x, ball_pos.y,
                          cache[i].turnCycle(), cache[i].dashCycle(),
                          score );
#endif
            if ( score > forward_score )
            {
                forward_best = &cache[i];
                forward_score = score;
            }

            continue;
        }

        // other: select nearest one

        {
            double d = wm.self().pos().dist2( ball_pos );
#ifdef DEBUG
            dlog.addText( Logger::INTERCEPT,
                          "___ %d other cycle=%d pos=(%.1f %.1f) turn=%d dash=%d dist2=%.2f",
                          i, cycle,
                          ball_pos.x, ball_pos.y,
                          cache[i].turnCycle(), cache[i].dashCycle(),
                          d );
#endif
            if ( d < nearest_score )
            {
                nearest_best = &cache[i];
                nearest_score = d;
            }
        }

    }

    if ( attacker_best )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<--- attacker best: cycle=%d(t=%d,d=%d) score=%f",
                      attacker_best->reachCycle(),
                      attacker_best->turnCycle(), attacker_best->dashCycle(),
                      attacker_score );

        return *attacker_best;
    }

    if ( noturn_best && forward_best )
    {
        //const Vector2D forward_ball_pos = wm.ball().inertiaPoint( forward_best->reachCycle() );
        //const Vector2D forward_ball_vel
        //    = wm.ball().vel()
        //    * std::pow( ServerParam::i().ballDecay(), forward_best->reachCycle() );

        if ( forward_best->reachCycle() >= 5 )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<--- forward best(1): cycle=%d(t=%d,d=%d) score=%f",
                          forward_best->reachCycle(),
                          forward_best->turnCycle(), forward_best->dashCycle(),
                          forward_score );
        }

        const Vector2D noturn_ball_vel
            = wm.ball().vel()
            * std::pow( ServerParam::i().ballDecay(), noturn_best->reachCycle() );
        const double noturn_ball_speed = noturn_ball_vel.r();
        if ( noturn_ball_vel.x > 0.3
             && ( noturn_ball_speed > speed_max
                  || noturn_best->reachCycle() <= forward_best->reachCycle() + 3 )
             )
        {
            dlog.addText( Logger::INTERCEPT,
                              "<--- noturn best(1): cycle=%d(t=%d,d=%d) score=%f",
                          noturn_best->reachCycle(),
                          noturn_best->turnCycle(), noturn_best->dashCycle(),
                          noturn_score );
            return *noturn_best;
        }
    }

    if ( forward_best )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<--- forward best(2): cycle=%d(t=%d,d=%d) score=%f",
                      forward_best->reachCycle(),
                      forward_best->turnCycle(), forward_best->dashCycle(),
                      forward_score );

        return *forward_best;
    }

    if ( noturn_best && nearest_best )
    {
        const Vector2D noturn_ball_pos = wm.ball().inertiaPoint( noturn_best->reachCycle() );
        const Vector2D nearest_ball_pos = wm.ball().inertiaPoint( nearest_best->reachCycle() );

        if ( wm.self().pos().dist2( noturn_ball_pos )
             < wm.self().pos().dist2( nearest_ball_pos ) )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<--- noturn best(2): cycle=%d(t=%d,d=%d) score=%f",
                          noturn_best->reachCycle(),
                          noturn_best->turnCycle(), noturn_best->dashCycle(),
                          noturn_score );

            return *noturn_best;
        }

        if ( nearest_best->reachCycle() <= noturn_best->reachCycle() + 2 )
        {
            const Vector2D nearest_ball_vel
                = wm.ball().vel()
                * std::pow( ServerParam::i().ballDecay(), nearest_best->reachCycle() );
            const double nearest_ball_speed = nearest_ball_vel.r();
            if ( nearest_ball_speed < 0.7 )
            {
                dlog.addText( Logger::INTERCEPT,
                              "<--- nearest best(2): cycle=%d(t=%d,d=%d) score=%f",
                              nearest_best->reachCycle(),
                              nearest_best->turnCycle(), nearest_best->dashCycle(),
                              nearest_score );
                return *nearest_best;
            }

            const Vector2D noturn_ball_vel
                = wm.ball().vel()
                * std::pow( ServerParam::i().ballDecay(), noturn_best->reachCycle() );

            if ( noturn_ball_vel.x < 0.5
                 && noturn_ball_vel.r2() > 1.0 * 1.0
                 && noturn_ball_pos.x > nearest_ball_pos.x )
            {
                dlog.addText( Logger::INTERCEPT,
                              "<--- nearest best(3): cycle=%d(t=%d,d=%d) score=%f",
                              nearest_best->reachCycle(),
                              nearest_best->turnCycle(), nearest_best->dashCycle(),
                              nearest_score );
                return *nearest_best;
            }

            if ( nearest_ball_speed > 0.7
                 && wm.self().pos().dist( nearest_ball_pos ) < wm.self().playerType().kickableArea() )
            {
                dlog.addText( Logger::INTERCEPT,
                              "<--- nearest best(4): cycle=%d(t=%d,d=%d) score=%f",
                              nearest_best->reachCycle(),
                              nearest_best->turnCycle(), nearest_best->dashCycle(),
                              nearest_score );
                return *nearest_best;
            }
        }

        dlog.addText( Logger::INTERCEPT,
                          "<--- noturn best(3): cycle=%d(t=%d,d=%d) score=%f",
                      noturn_best->reachCycle(),
                      noturn_best->turnCycle(), noturn_best->dashCycle(),
                      noturn_score );

        return *noturn_best;
    }

    if ( noturn_best )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<--- noturn best only: cycle=%d(t=%d,d=%d) score=%f",
                      noturn_best->reachCycle(),
                      noturn_best->turnCycle(), noturn_best->dashCycle(),
                      noturn_score );

        return *noturn_best;
    }

    if ( nearest_best )
    {
        dlog.addText( Logger::INTERCEPT,
                      "<--- nearest best only: cycle=%d(t=%d,d=%d) score=%f",
                      nearest_best->reachCycle(),
                      nearest_best->turnCycle(), nearest_best->dashCycle(),
                      nearest_score );

        return *nearest_best;
    }



    if ( wm.self().pos().x > 40.0
         && wm.ball().vel().r() > 1.8
         && wm.ball().vel().th().abs() < 100.0 )
    {
        const InterceptInfo * chance_best = nullptr;
        for ( std::size_t i = 0; i < MAX; ++i )
        {
            if ( cache[i].reachCycle() <= cache[0].reachCycle() + 3
                 && cache[i].reachCycle() <= opp_cycle - 2 )
            {
                chance_best = &cache[i];
            }
        }

        if ( chance_best )
        {
            dlog.addText( Logger::INTERCEPT,
                          "<--- chance best only: cycle=%d(t=%d,d=%d)",
                          chance_best->reachCycle(),
                          chance_best->turnCycle(), chance_best->dashCycle() );
            return *chance_best;
        }
    }

    return cache[0];

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Intercept2007::doWaitTurn( PlayerAgent * agent,
                                const Vector2D & target_point,
                                const InterceptInfo & info )
{
    const WorldModel & wm = agent->world();

    {
        const PlayerObject * opp = wm.getOpponentNearestToSelf( 5 );
        if ( opp && opp->distFromSelf() < 3.0 )
        {
            dlog.addText( Logger::INTERCEPT,
                          __FILE__": doWaitTurn. exist near opponent  cancel" );
            return false;
        }
    }

    const Vector2D my_inertia = wm.self().inertiaPoint( info.reachCycle() );
    Vector2D inertia_rel = target_point - my_inertia;
    inertia_rel.rotate( - wm.self().body() );

    const double inertia_dist = inertia_rel.r();

    const double ball_travel
        = inertia_n_step_distance( wm.ball().vel().r(),
                                   info.reachCycle(),
                                   ServerParam::i().ballDecay() );
    const double ball_noise = ball_travel * ServerParam::i().ballRand();

    if ( info.reachCycle() == 1 )
    {
        if ( inertia_dist < wm.self().playerType().kickableArea() - 0.15 - ball_noise
             && inertia_dist > ( wm.self().playerType().playerSize()
                                 + ServerParam::i().ballSize()
                                 + 0.15 )
             )
        {
            double next_kick_rate
                = wm.self().playerType().kickRate( inertia_dist,
                                                   inertia_rel.th().abs() );
            double next_ball_speed
                = wm.ball().vel().r()
                * ServerParam::i().ballDecay();

            // at least, player can stop the ball
            if ( next_ball_speed < next_kick_rate * ServerParam::i().maxPower() )
            {
                Vector2D face_point = M_face_point;
                if ( ! face_point.valid() )
                {
                    face_point.assign( 50.5, wm.self().pos().y * 0.9 );
                }
                Body_TurnToPoint( face_point ).execute( agent );
                dlog.addText( Logger::INTERCEPT,
                              __FILE__": doWaitTurn. 1 step inertia_ball_dist=%.2f",
                              inertia_dist  );
                agent->debugClient().addMessage( "WaitTurn1" );
                return true;
            }
        }

        return false;
    }


    double extra_buf = 0.1 * bound( 0, info.reachCycle() - 1, 4 );
    {
        double angle_diff = ( wm.ball().vel().th() - wm.self().body() ).abs();
        if ( angle_diff < 10.0
             || 170.0 < angle_diff )
        {
            extra_buf = 0.0;
        }
    }

    Vector2D face_point = M_face_point;
    if ( ! face_point.valid() )
    {
        face_point.assign( 50.5, wm.self().pos().y * 0.9 );
    }

    Vector2D face_rel = face_point - my_inertia;
    AngleDeg face_angle = face_rel.th();

    //if ( inertia_rel.absY() > wm.self().kickableArea() - ball_noise - 0.2 )
    Vector2D faced_rel = target_point - my_inertia;
    faced_rel.rotate( face_angle );
    if ( faced_rel.absY() > wm.self().playerType().kickableArea() - ball_noise - 0.2 )
    {
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": doWaitTurn. inertia_y_diff %.2f  ball_noise=%.2f",
                      faced_rel.y, ball_noise );
        return false;
    }

    double dist_buf = ( wm.self().playerType().kickableArea()
                        - 0.3
                        + extra_buf );

    dlog.addText( Logger::INTERCEPT,
                  __FILE__": doWaitTurn. inertia_ball_dist=%.2f buf=%.2f extra=%.2f",
                  inertia_dist,
                  dist_buf, extra_buf );

    if ( inertia_dist > dist_buf )
    {
        return false;
    }


    double turn_margin = std::max( AngleDeg::asin_deg( 1.0 / face_rel.r() ),
                                   15.0 );

    if ( ( face_angle - wm.self().body() ).abs() < turn_margin )
    {
        // already facing
        return false;
    }


    Body_TurnToPoint( face_point ).execute( agent );
    agent->debugClient().addMessage( "WaitTurn%d", info.reachCycle() );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Body_Intercept2007::doInertiaDash( PlayerAgent * agent,
                                   const Vector2D & target_point,
                                   const InterceptInfo & info )
{
    const WorldModel & wm = agent->world();

    if ( info.reachCycle() == 1 )
    {
        agent->debugClient().addMessage( "Intercept1Dash%.0f",
                                         info.dashPower() );
        agent->doDash( info.dashPower() );
        return true;
    }

    Vector2D target_rel = target_point - wm.self().pos();
    target_rel.rotate( - wm.self().body() );

    AngleDeg accel_angle = wm.self().body();
    if ( info.dashPower() < 0.0 ) accel_angle += 180.0;

    if ( ! wm.self().goalie()
         && wm.self().body().abs() < 50.0
         && wm.ball().vel().x > - 0.2 )
    {
        double buf = 0.0;
#if 0
        if ( target_rel.x < 0.0 )
        {
            buf = 0.0;
        }
        else if ( target_rel.x < 0.3 )
        {
            if ( info.reachCycle() >= 3 ) buf = 0.4;
        }
        else if ( target_rel.absY() < 0.5 )
        {
            if ( info.reachCycle() >= 3 ) buf = 0.5;
            if ( info.reachCycle() == 2 ) buf = 0.5;
        }
        else
        {
            if ( info.reachCycle() >= 4 ) buf = 0.45;
            else if ( info.reachCycle() == 3 ) buf = 0.4;
            else if ( info.reachCycle() == 2 ) buf = 0.35;
        }
#else
        if ( info.reachCycle() >= 8 )
        {
            buf = 0.0;
        }
        else if ( target_rel.absY() > wm.self().playerType().kickableArea() - 0.25 ) //0.75 )
        {
            buf = 0.0;
        }
        else if ( target_rel.x < 0.0 )
        {
            if ( info.reachCycle() >= 3 ) buf = 0.8;
        }
        else if ( target_rel.x < 0.3 )
        {
            if ( info.reachCycle() >= 3 ) buf = 0.8;
        }
        else if ( target_rel.absY() < 0.5 )
        {
            if ( info.reachCycle() >= 3 ) buf = 0.9;
            if ( info.reachCycle() == 2 ) buf = std::min( target_rel.x, 0.9 );
        }
        else
        {
            if ( info.reachCycle() >= 4 ) buf = 0.7;
            else if ( info.reachCycle() == 3 ) buf = 0.7;
            else if ( info.reachCycle() == 2 ) buf = std::min( target_rel.x, 0.7 );
        }
#endif

        target_rel.x -= buf;

        dlog.addText( Logger::INTERCEPT,
                      __FILE__": doInertiaDash. slightly back to wait. buf=%.3f",
                      buf );
    }

    double used_power = info.dashPower();

    if ( target_rel.absX() < 1.5 )
    {
        double first_speed
            = calc_first_term_geom_series( target_rel.x,
                                           wm.self().playerType().playerDecay(),
                                           info.reachCycle() );

        first_speed = min_max( - wm.self().playerType().playerSpeedMax(),
                               first_speed,
                               wm.self().playerType().playerSpeedMax() );
        Vector2D rel_vel = wm.self().vel().rotatedVector( - wm.self().body() );
        double required_accel = first_speed - rel_vel.x;
        used_power = required_accel / wm.self().dashRate();
        //if ( info.dashPower() < 0.0 ) used_power = -used_power;

        used_power = ServerParam::i().normalizePower( used_power );
        used_power = wm.self().getSafetyDashPower( used_power );

        agent->debugClient().addMessage( "InterceptInertiaDash%d:%.0f",
                                         info.reachCycle(), used_power );
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": doInertiaDash. x_diff=%.2f first_speed=%.2f"
                      " accel=%.2f power=%.1f",
                      target_rel.x, first_speed, required_accel, used_power );

    }
    else
    {
        agent->debugClient().addMessage( "InterceptDash%.0f:%d",
                                         used_power, info.reachCycle() );
        dlog.addText( Logger::INTERCEPT,
                      __FILE__": doInertiaDash. normal dash. x_diff=%.2f ",
                      target_rel.x );
    }


    if ( info.reachCycle() >= 4
         && ( target_rel.absX() < 0.5
              || std::fabs( used_power ) < 5.0 )
         )
    {
        agent->debugClient().addMessage( "LookBall" );

        Vector2D my_inertia = wm.self().inertiaPoint( info.reachCycle() );
        Vector2D face_point = M_face_point;
        if ( ! M_face_point.valid() )
        {
            face_point.assign( 50.5, wm.self().pos().y * 0.75 );
        }
        AngleDeg face_angle = ( face_point - my_inertia ).th();

        Vector2D ball_next = wm.ball().pos() + wm.ball().vel();
        AngleDeg ball_angle = ( ball_next - my_inertia ).th();

        if ( ( ball_angle - face_angle ).abs()
             > ( ServerParam::i().maxNeckAngle()
                 + ServerParam::i().visibleAngle() * 0.5
                 - 10.0 )
             )
        {
            face_point.x = my_inertia.x;
            if ( ball_next.y > my_inertia.y + 1.0 ) face_point.y = 50.0;
            else if ( ball_next.y < my_inertia.y - 1.0 ) face_point.y = -50.0;
            else  face_point = ball_next;
            dlog.addText( Logger::INTERCEPT,
                          __FILE__": doInertiaDash. check ball with turn."
                          " face to (%.1f %.1f)",
                          face_point.x, face_point.y );
        }
        else
        {
            dlog.addText( Logger::INTERCEPT,
                          __FILE__": doInertiaDash. can check ball without turn"
                          " face to (%.1f %.1f)",
                          face_point.x, face_point.y );
        }
        Body_TurnToPoint( face_point ).execute( agent );
        return true;
    }

    agent->doDash( used_power );
    return true;
}

}
