// -*-c++-*-

/*!
  \file shoot_table2008.cpp
  \brief shoot plan search and holder class
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

#include "shoot_table2008.h"

#include "kick_table.h"

#include <rcsc/player/player_agent.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/math_util.h>
#include <rcsc/timer.h>

// #define DEBUG_PROFILE
// #define DEBUG_PRINT
// #define DEBUG_PRINT_LEVEL_2

// #define DEBUG_PAINT_SUCCESS_COURSE
// #define DEBUG_PAINT_FAILED_COURSE

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
void
ShootTable2008::search( const PlayerAgent * agent )
{
    static GameTime s_time( 0, 0 );

    /////////////////////////////////////////////////////////////////////
    const WorldModel & wm = agent->world();

    if ( s_time == wm.time() )
    {
        return;
    }

    s_time = wm.time();
    M_total_count = 0;
    M_shots.clear();

    static const Vector2D goal_c( ServerParam::i().pitchHalfLength(), 0.0 );

    if ( ! wm.self().isKickable() )
    {
        return;
    }

    if ( wm.self().pos().dist2( goal_c ) > std::pow( 30.0, 2 ) )
    {
        return;
    }

#ifdef DEBUG_PROFILE
    Timer timer;
#endif

    Vector2D goal_l( ServerParam::i().pitchHalfLength(),
                     -ServerParam::i().goalHalfWidth() );
    Vector2D goal_r( ServerParam::i().pitchHalfLength(),
                     ServerParam::i().goalHalfWidth() );

    goal_l.y += std::min( 1.5,
                          0.6 + goal_l.dist( wm.ball().pos() ) * 0.042 );
    goal_r.y -= std::min( 1.5,
                          0.6 + goal_r.dist( wm.ball().pos() ) * 0.042 );

    if ( wm.self().pos().x > ServerParam::i().pitchHalfLength() - 1.0
         && wm.self().pos().absY() < ServerParam::i().goalHalfWidth() )
    {
        goal_l.x = wm.self().pos().x + 1.5;
        goal_r.x = wm.self().pos().x + 1.5;
    }

    const int DIST_DIVS = 25;
    const double dist_step = std::fabs( goal_l.y - goal_r.y ) / ( DIST_DIVS - 1 );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::SHOOT,
                  __FILE__": ===== Shoot search range=(%.1f %.1f)-(%.1f %.1f) dist_step=%.1f =====",
                  goal_l.x, goal_l.y, goal_r.x, goal_r.y, dist_step );
#endif

    const AbstractPlayerObject * goalie = agent->world().getTheirGoalie();

    Vector2D shot_point = goal_l;

    for ( int i = 0;
          i < DIST_DIVS;
          ++i, shot_point.y += dist_step )
    {
        ++M_total_count;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::SHOOT,
                      "%d: ===== shoot target(%.2f %.2f) ===== ",
                      M_total_count,
                      shot_point.x, shot_point.y );
#endif
        calculateShotPoint( wm, shot_point, goalie );
    }

#ifdef DEBUG_PROFILE
    dlog.addText( Logger::SHOOT,
                  __FILE__": PROFILE %d/%d. elapsed=%.3f [ms]",
                  (int)M_shots.size(),
                  DIST_DIVS,
                  timer.elapsedReal() );
#endif

}

/*-------------------------------------------------------------------*/
/*!

 */
void
ShootTable2008::calculateShotPoint( const WorldModel & wm,
                                    const Vector2D & shot_point,
                                    const AbstractPlayerObject * goalie )
{
    Vector2D shot_rel = shot_point - wm.ball().pos();
    AngleDeg shot_angle = shot_rel.th();

    int goalie_count = 1000;
    if ( goalie )
    {
        goalie_count = goalie->posCount();
    }

    if ( 5 < goalie_count
         && goalie_count < 30
         && wm.dirCount( shot_angle ) > 3 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::SHOOT,
                      "%d: __ xxx goalie_count=%d, low dir accuracy",
                      M_total_count,
                      goalie_count );
#endif
        return;
    }

    double shot_dist = shot_rel.r();

    Vector2D one_step_vel
        = KickTable::calc_max_velocity( shot_angle,
                                        wm.self().kickRate(),
                                        wm.ball().vel() );
    double max_one_step_speed = one_step_vel.r();

    double shot_first_speed
        = ( shot_dist + 5.0 ) * ( 1.0 - ServerParam::i().ballDecay() );
    shot_first_speed = std::max( max_one_step_speed, shot_first_speed );
    shot_first_speed = std::max( 1.5, shot_first_speed );

    // gaussian function, distribution = goal half width
    //double y_rate = std::exp( - std::pow( shot_point.y, 2.0 )
    //                          / ( 2.0 * ServerParam::i().goalHalfWidth() * 3.0 ) );
    double y_dist = std::max( 0.0, shot_point.absY() - 4.0 );
    double y_rate = std::exp( - std::pow( y_dist, 2.0 )
                              / ( 2.0 * ServerParam::i().goalHalfWidth() ) );

    bool over_max = false;
    while ( ! over_max )
    {
        if ( shot_first_speed > ServerParam::i().ballSpeedMax() - 0.001 )
        {
            over_max = true;
            shot_first_speed = ServerParam::i().ballSpeedMax();
        }

        Shot shot( shot_point, shot_first_speed, shot_angle );
        shot.score_ = 0;

        bool one_step = ( shot_first_speed <= max_one_step_speed );
        if ( canScore( wm, one_step, &shot ) )
        {
            shot.score_ += 100;
            if ( one_step )
            {   // one step kick
                shot.score_ += 100;
            }

            double goalie_rate = -1.0;
            if ( shot.goalie_never_reach_ )
            {
                shot.score_ += 100;
            }

            if ( goalie )
            {
                AngleDeg goalie_angle = ( goalie->pos() - wm.ball().pos() ).th();
                double angle_diff = ( shot.angle_ - goalie_angle ).abs();
                goalie_rate = 1.0 - std::exp( - std::pow( angle_diff * 0.1, 2 )
                                              // / ( 2.0 * 90.0 * 0.1 ) );
                                              // / ( 2.0 * 40.0 * 0.1 ) ); // 2009-07
                                              / ( 2.0 * 90.0 * 0.1 ) ); // 2009-12-13
                shot.score_ = static_cast< int >( shot.score_ * goalie_rate );
#ifdef DEBUG_PRINT
                dlog.addText( Logger::SHOOT,
                              "--- apply goalie rate. angle_diff=%.1f rate=%.2f",
                              angle_diff, goalie_rate );
#endif
            }

            shot.score_ = static_cast< int >( shot.score_ * y_rate );

#ifdef DEBUG_PRINT
            dlog.addText( Logger::SHOOT,
                          "%d: ok shoot score=%d pos(%.1f %.1f)"
                          " angle=%.1f speed=%.1f"
                          " y_rate=%.2f g_rate=%.2f"
                          " one_step=%d GK_never_reach=%d",
                          M_total_count,
                          shot.score_,
                          shot_point.x, shot_point.y,
                          shot_angle.degree(),
                          shot_first_speed,
                          y_rate, goalie_rate,
                          ( one_step ? 1 : 0 ),
                          ( shot.goalie_never_reach_ ? 1 : 0 ) );
#endif
#ifdef DEBUG_PAINT_SUCCESS_COURSE
            dlog.addRect( Logger::SHOOT,
                          shot_point.x - 0.1, shot_point.y - 0.1,
                          0.2, 0.2,
                          "#00ff00" );
            char num[8];
            snprintf( num, 8, "%d", M_total_count );
            dlog.addMessage( Logger::SHOOT,
                             shot_point, num, "#ffffff" );
#endif
            M_shots.push_back( shot );
        }
        else
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SHOOT,
                          "%d: xxx shoot failed to(%.1f %.1f)"
                          " angle=%.1f speed=%.1f",
                          M_total_count,
                          shot_point.x, shot_point.y,
                          shot_angle.degree(),
                          shot_first_speed );
#endif
#ifdef DEBUG_PAINT_FAILED_COURSE
            dlog.addRect( Logger::SHOOT,
                          shot_point.x - 0.1, shot_point.y - 0.1,
                          0.2, 0.2,
                          "#ff0000" );
            char num[8];
            snprintf( num, 8, "%d", M_total_count );
            dlog.addMessage( Logger::SHOOT,
                             shot_point, num, "#ffffff" );
#endif
        }

        shot_first_speed += 0.5;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ShootTable2008::canScore( const WorldModel & wm,
                          const bool one_step_kick,
                          Shot * shot )
{
    static const double opp_x_thr = ServerParam::i().theirPenaltyAreaLineX() - 5.0;
    static const double opp_y_thr = ServerParam::i().penaltyAreaHalfWidth();

    // estimate required ball travel step
    const double ball_reach_step
        = calc_length_geom_series( shot->speed_,
                                   wm.ball().pos().dist( shot->point_ ),
                                   ServerParam::i().ballDecay() );

    if ( ball_reach_step < 1.0 )
    {
        shot->score_ += 100;
        return true;
    }

    const int ball_reach_step_i = static_cast< int >( std::ceil( ball_reach_step ) );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::SHOOT,
                  "%d: (canScore) target=(%.2f %.2f) vel=(%.2f %.2f) r=%.3f angle=%.1f"
                  " ball_reach_step=%.3f",
                  M_total_count,
                  shot->point_.x, shot->point_.y,
                  shot->vel_.x, shot->vel_.y,
                  shot->speed_,
                  shot->angle_.degree(),
                  ball_reach_step );
#endif

    // estimate opponent interception

    for ( PlayerObject::Cont::const_iterator it = wm.opponentsFromSelf().begin(),
              end = wm.opponentsFromSelf().end();
          it != end;
          ++it )
    {
        // outside of penalty
        if ( (*it)->pos().x < opp_x_thr ) continue;
        if ( (*it)->pos().absY() > opp_y_thr ) continue;
        if ( (*it)->isTackling() ) continue;

        // behind of shoot course
        if ( ( shot->angle_ - (*it)->angleFromSelf() ).abs() > 90.0 )
        {
            continue;
        }

        if ( (*it)->goalie() )
        {
            if ( maybeGoalieCatch( wm, *it, shot ) )
            {
                return false;
            }
        }
        else
        {
            if ( (*it)->posCount() > 10
                 || ( (*it)->isGhost() && (*it)->posCount() > 5 ) )
            {
                continue;
            }

#ifdef DEBUG_PRINT
            dlog.addText( Logger::SHOOT,
                          "%d: __ opp %d(%.1f %.1f)",
                          M_total_count,
                          (*it)->unum(), (*it)->pos().x, (*it)->pos().y );
#endif
            int cycle = predictOpponentReachStep( wm,
                                                  shot->point_,
                                                  *it,
                                                  wm.ball().pos(),
                                                  shot->vel_,
                                                  one_step_kick,
                                                  ball_reach_step_i );
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SHOOT,
                          "%d: ____ reach step=%d, ball_step=%d",
                          M_total_count,
                          cycle,
                          ball_reach_step_i );
#endif
            if ( cycle == 1
                 || cycle < ball_reach_step_i - 1 )
            {
#ifdef DEBUG_PRINT
                dlog.addText( Logger::SHOOT,
                              "%d: xxx opp %d(%.1f %.1f) will get the ball",
                              M_total_count,
                              (*it)->unum(),
                              (*it)->pos().x, (*it)->pos().y );
#endif
                return false;
            }
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ShootTable2008::maybeGoalieCatch( const WorldModel & wm,
                                  const AbstractPlayerObject * goalie,
                                  Shot * shot )
{
    static const
        Rect2D penalty_area( Vector2D( ServerParam::i().theirPenaltyAreaLineX(), // left
                                       -ServerParam::i().penaltyAreaHalfWidth() ), // top
                             Size2D( ServerParam::i().penaltyAreaLength(), // length
                                     ServerParam::i().penaltyAreaWidth() ) ); // width
    static const double catchable_area = ServerParam::i().catchableArea();

    const ServerParam & param = ServerParam::i();

    const double dash_accel_mag = ( param.maxDashPower()
                                    * param.defaultDashPowerRate()
                                    * param.defaultEffortMax() );
    const double seen_dist_noise = goalie->distFromSelf() * 0.05;

    int min_cycle = 1;
    {
        Line2D shot_line( wm.ball().pos(), shot->point_ );
        double goalie_line_dist = shot_line.dist( goalie->pos() );
        goalie_line_dist -= catchable_area;
        goalie_line_dist -= seen_dist_noise;
        min_cycle = static_cast< int >
            ( std::ceil( goalie_line_dist / param.defaultRealSpeedMax() ) ) ;
        min_cycle -= std::min( 5, goalie->posCount() );
        min_cycle = std::max( 1, min_cycle );
    }
    Vector2D ball_pos = inertia_n_step_point( wm.ball().pos(),
                                              shot->vel_,
                                              min_cycle,
                                              param.ballDecay() );
    Vector2D ball_vel = ( shot->vel_
                          * std::pow( param.ballDecay(), min_cycle ) );


    int cycle = min_cycle;
    while ( ball_pos.x < param.pitchHalfLength() + 0.085
            && cycle <= 50 )
    {
        // estimate the required turn angle
        Vector2D goalie_pos = goalie->inertiaPoint( cycle );
        Vector2D ball_relative = ball_pos - goalie_pos;
        double ball_dist = ball_relative.r() - seen_dist_noise;

        if ( ball_dist < catchable_area )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SHOOT,
                          "%d: xxx goalie can catch the ball. cycle=%d ball_pos(%.1f %.1f)"
                          " ball_dist=%.3f",
                          M_total_count,
                          cycle,
                          ball_pos.x, ball_pos.y,
                          ball_dist );
#endif
            return true;
        }

        //if ( ball_dist < catchable_area + 1.75 ) // 2009-07-02: 1.0 -> 1.75
        if ( ball_dist < catchable_area + 1.2 ) // 2009-12-13
        {
            shot->goalie_never_reach_ = false;
        }

        AngleDeg ball_angle = ball_relative.th();
        AngleDeg goalie_body = ( goalie->bodyCount() <= 5
                                 ? goalie->body()
                                 : ball_angle );

        int n_turn = 0;
        double angle_diff = ( ball_angle - goalie_body ).abs();
        if ( angle_diff > 90.0 )
        {
            angle_diff = 180.0 - angle_diff; // back dash
            goalie_body -= 180.0;
        }

        double turn_margin
            = std::max( AngleDeg::asin_deg( catchable_area / ball_dist ),
                        15.0 );

#ifdef DEBUG_PRINT
        dlog.addText( Logger::SHOOT,
                      "%d: __ goalie. cycle=%d ball_pos(%.1f %.1f) g_body=%.0f"
                      " b_angle=%.1f angle_diff=%.1f turn_margin=%.1f",
                      M_total_count,
                      cycle,
                      ball_pos.x, ball_pos.y,
                      goalie_body.degree(),
                      ball_angle.degree(),
                      angle_diff, turn_margin );
#endif

        Vector2D goalie_vel = goalie->vel();

        while ( angle_diff > turn_margin )
        {
            double max_turn
                = effective_turn( 180.0,
                                  goalie_vel.r(),
                                  param.defaultInertiaMoment() );
            angle_diff -= max_turn;
            goalie_vel *= param.defaultPlayerDecay();
            ++n_turn;
        }

        // simulate dash
        goalie_pos = goalie->inertiaPoint( n_turn );

        const Vector2D dash_accel = Vector2D::polar2vector( dash_accel_mag,
                                                            ball_angle );
        const int max_dash = ( cycle - 1 - n_turn
                               + bound( 0, goalie->posCount() - 1, 5 ) );
        double goalie_travel = 0.0;
        for ( int i = 0; i < max_dash; ++i )
        {
            goalie_vel += dash_accel;
            goalie_pos += goalie_vel;
            goalie_travel += goalie_vel.r();
            goalie_vel *= param.defaultPlayerDecay();

            double d = goalie_pos.dist( ball_pos ) - seen_dist_noise;
#ifdef DEBUG_PRINT_LEVEL_2
            dlog.addText( Logger::SHOOT,
                          "%d: __ goalie. cycle=%d turn=%d dash=%d ball(%.1f %.1f) angle=%.0f"
                          " goalie pos(%.1f %.1f) travel=%.1f dist=%.1f turn=%d dash=%d",
                          M_total_count,
                          cycle, n_turn, i + 1,
                          ball_pos.x, ball_pos.y,
                          ball_angle.degree(),
                          goalie_pos.x, goalie_pos.y,
                          goalie_travel,
                          d,
                          n_turn, i + 1 );
#endif
            if ( d < catchable_area + 1.0 + ( goalie_travel * 0.04 ) )
            {
                shot->goalie_never_reach_ = false;
            }
        }

        // check distance
        if ( goalie->pos().dist( goalie_pos ) * 1.05
             > goalie->pos().dist( ball_pos )
             - seen_dist_noise
             - catchable_area )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::SHOOT,
                          "%d: xx goalie can reach. move_dist=%.3f ball_dist=%.3f"
                          " (raw_dist=%.3f seen_noise=%.3f catch_area=%.3f",
                          M_total_count,
                          goalie->pos().dist( goalie_pos ) * 1.05,
                          goalie->pos().dist( ball_pos ) - seen_dist_noise - catchable_area,
                          goalie->pos().dist( ball_pos ), seen_dist_noise, catchable_area );
#endif
            return true;
        }

        // update ball position & velocity
        ++cycle;
        ball_pos += ball_vel;
        ball_vel *= param.ballDecay();
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
int
ShootTable2008::predictOpponentReachStep( const WorldModel &,
                                          const Vector2D & target_point,
                                          const AbstractPlayerObject * opponent,
                                          const Vector2D & first_ball_pos,
                                          const Vector2D & first_ball_vel,
                                          const bool one_step_kick,
                                          const int max_step )
{
    const ServerParam & param = ServerParam::i();
    const PlayerType * player_type = opponent->playerTypePtr();
    const double control_area = player_type->kickableArea();

    int min_cycle = 1;
    {
        Line2D shot_line( first_ball_pos, target_point );
        double line_dist = shot_line.dist( opponent->pos() );
        line_dist -= control_area;
        min_cycle = static_cast< int >
            ( std::ceil( line_dist / player_type->realSpeedMax() ) ) ;
        min_cycle -= std::min( 5, opponent->posCount() );
        min_cycle = std::max( 1, min_cycle );
    }

    Vector2D ball_pos = inertia_n_step_point( first_ball_pos,
                                              first_ball_vel,
                                              min_cycle,
                                              param.ballDecay() );
    Vector2D ball_vel = first_ball_vel * std::pow( param.ballDecay(), min_cycle );

    int cycle = min_cycle;

    while ( cycle <= max_step )
    {
        Vector2D opp_pos = opponent->inertiaPoint( cycle );
        Vector2D opp_to_ball = ball_pos - opp_pos;
        double opp_to_ball_dist = opp_to_ball.r();

        int n_turn = 0;
        if ( opponent->bodyCount() <= 1
             || opponent->velCount() <= 1 )
        {
            double angle_diff =  ( opponent->bodyCount() <= 1
                                  ? ( opp_to_ball.th() - opponent->body() ).abs()
                                  : ( opp_to_ball.th() - opponent->vel().th() ).abs() );

            double turn_margin = 180.0;
            if ( control_area < opp_to_ball_dist )
            {
                turn_margin = AngleDeg::asin_deg( control_area / opp_to_ball_dist );
            }
            turn_margin = std::max( turn_margin, 12.0 );

            double opp_speed = opponent->vel().r();
#ifdef DEBUG_PRINT_LEVEL_2
            dlog.addText( Logger::SHOOT,
                          "%d: (opponent) speed=%.3f turn_marign=%.1f angle_diff=%.1f",
                          M_total_count,
                          opp_speed,
                          turn_margin, angle_diff );
#endif
            while ( angle_diff > turn_margin )
            {
                angle_diff -= player_type->effectiveTurn( param.maxMoment(), opp_speed );
                opp_speed *= player_type->playerDecay();
                ++n_turn;
            }
        }

        opp_to_ball_dist -= control_area;
        opp_to_ball_dist -= opponent->distFromSelf() * 0.03;

        if ( opp_to_ball_dist < 0.0 )
        {
#ifdef DEBUG_PRINT_LEVEL_2
            dlog.addText( Logger::SHOOT,
                          "%d: xxx (opponent) reachable without dash. ball_dist=%.3f",
                          M_total_count,
                          opp_to_ball_dist + control_area + opponent->distFromSelf() * 0.03 );
#endif
            return cycle;
        }

        int n_step = player_type->cyclesToReachDistance( opp_to_ball_dist );
        n_step += n_turn;
        //n_step -= bound( 0, opponent->posCount() - 1, 2 );
        n_step -= bound( 0, opponent->posCount(), 2 );

        if ( n_step < cycle - ( one_step_kick ? 1 : 0 ) )
        {
#ifdef DEBUG_PRINT_LEVEL_2
            dlog.addText( Logger::SHOOT,
                          "%d: xxx (opponent) reachable with dash. cycle=%d turn=%d dash=%d n_buf=%d",
                          M_total_count,
                          cycle,
                          n_turn,
                          n_step - n_turn + bound( 0, opponent->posCount() - 1, 2 ),
                          bound( 0, opponent->posCount() - 1, 2 ) );
#endif
            return cycle;
        }

        // update ball position & velocity
        ++cycle;
        ball_pos += ball_vel;
        ball_vel *= param.ballDecay();
    }

    return cycle;
}

}
