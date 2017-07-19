// -*-c++-*-

/*!
  \file ball_object.cpp
  \brief ball object class Source File
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

#include "ball_object.h"

#include "action_effector.h"
#include "self_object.h"
#include "player_command.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_mode.h>

#include <iostream>

// #define DEBUG_PRINT

namespace rcsc {

int BallObject::S_pos_count_thr = 10;
int BallObject::S_rpos_count_thr = 5;
int BallObject::S_vel_count_thr = 10;

/*-------------------------------------------------------------------*/
/*!

 */
BallObject::BallObject()
    : M_pos( 0.0, 0.0 ),
      M_pos_error( 0.0, 0.0 ),
      M_pos_count( 1000 ),
      M_rpos( Vector2D::INVALIDATED ),
      M_rpos_error( 0.0, 0.0 ),
      M_rpos_count( 1000 ),
      M_seen_pos( 0.0, 0.0 ),
      M_seen_rpos( Vector2D::INVALIDATED ),
      M_seen_pos_count( 1000 ),
      M_heard_pos( 0.0, 0.0 ),
      M_heard_pos_count( 1000 ),
      M_vel( 0.0, 0.0 ),
      M_vel_error( 0.0, 0.0 ),
      M_vel_count( 1000 ),
      M_seen_vel( 0.0, 0.0 ),
      M_seen_vel_count( 1000 ),
      M_heard_vel( 0.0, 0.0 ),
      M_heard_vel_count( 1000 ),
      M_lost_count( 0 ),
      M_ghost_count( 0 ),
      M_dist_from_self( 1000.0 ),
      M_angle_from_self( 0.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::set_count_thr( const int pos_thr,
                           const int rpos_thr,
                           const int vel_thr )
{
    S_pos_count_thr = pos_thr;
    S_rpos_count_thr = rpos_thr;
    S_vel_count_thr = vel_thr;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::setGhost()
{
    if ( M_ghost_count > 0 )
    {
        M_pos_count = 1000;
        M_rpos_count = 1000;
        M_lost_count = 0;
        M_ghost_count += 1;

        M_dist_from_self = 1000.0;
    }
    else
    {
        M_ghost_count = 1;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::update( const ActionEffector & act,
                    const GameMode & game_mode )
{
    M_pos_history.push_front( M_pos );
    if ( M_pos_history.size() > 100 )
    {
        M_pos_history.pop_back();
    }

    Vector2D new_vel( 0.0, 0.0 );

    ////////////////////////////////////////////////////////////////////////
    // vel
    if ( velValid() )
    {
        Vector2D accel( 0.0, 0.0 );
        Vector2D accel_err( 0.0, 0.0 );
        double tmp = 0.0;

        new_vel = vel();

        /////////////////////////////////////////////////////////////
        // kicked in last cycle
        // get info from stored action param
        if ( act.lastBodyCommandType() == PlayerCommand::KICK )
        {
            act.getKickInfo( &accel, &accel_err );

            // check max accel
            tmp = accel.r();
            if ( tmp > ServerParam::i().ballAccelMax() )
            {
                accel *= ( ServerParam::i().ballAccelMax() / tmp );
            }

            new_vel += accel;

#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (update) queued_kick_accel=(%.3f, %.3f) new_vel=(%.3f %.3f)",
                          accel.x, accel.y,
                          new_vel.x, new_vel.y );
#endif
        }

        // check max vel
        tmp = new_vel.r();
        if ( tmp > ServerParam::i().ballSpeedMax() )
        {
            new_vel *= ( ServerParam::i().ballSpeedMax() / tmp );
            tmp = ServerParam::i().ballSpeedMax();
        }

        // add move noise.
        // ball speed max is not considerd, therefore value of tmp is not changed.
        M_vel_error.add( tmp * ServerParam::i().ballRand(),
                         tmp * ServerParam::i().ballRand() );
        // add kick noise
        M_vel_error += accel_err;
    }

    ////////////////////////////////////////////////////////////////////////
    // wind effect
    updateWindEffect();

    ////////////////////////////////////////////////////////////////////////

    const GameMode::Type pmode = game_mode.type();

    if ( pmode == GameMode::PlayOn
         || pmode == GameMode::GoalKick_
         || pmode == GameMode::GoalieCatch_
         || pmode == GameMode::PenaltyTaken_ )
    {
        // ball position may change.
        M_pos_count = std::min( 1000, M_pos_count + 1 );
    }
    else
    {
        // if setplay playmode, ball does not move until playmode change to playon.
        // if the agent didin't see the ball in this setplay playmode,
        // the agent has to check the ball first.
        if ( posCount() >= 5
             || ( rposCount() >= 2
                  && distFromSelf() * 1.05 < ServerParam::i().visibleDistance() )
             )
        {
            // NOT seen at last cycle, but internal info means ball visible.
            // !!! IMPORTANT to check the ghost
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (update) SetPlay. but not seen. posCount=%d"
                          " rposCount=%d. distFromSelf=%.3f",
                          posCount(), rposCount(), distFromSelf() );
#endif
            M_pos_count = 1000;
        }
        else
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (update) SetPlay. seen once. posCount=%d"
                          " rposCount=%d distFromSelf=%.3f",
                          posCount(), rposCount(), distFromSelf() );
#endif
            M_pos_count = 1;
        }

        // in SetPlay mode, ball velocity must be Zero.
        new_vel.assign( 0.0, 0.0 );

        M_vel_error.assign( 0.0, 0.0 );
        M_vel_count = 0;
        M_seen_vel.assign( 0.0, 0.0 );
        M_seen_vel_count = 0;
    }

    // update position with velocity
    if ( posValid() )
    {
        M_pos += new_vel;
        M_pos_error += M_vel_error;
    }

    // vel decay
    M_vel = new_vel;
    M_vel *= ServerParam::i().ballDecay();
    M_vel_error *= ServerParam::i().ballDecay();

    // update accuracy counter
    M_rpos_count = std::min( 1000, M_rpos_count + 1 );
    M_seen_pos_count = std::min( 1000, M_seen_pos_count + 1 );
    M_heard_pos_count = std::min( 1000, M_heard_pos_count + 1 );
    M_vel_count = std::min( 1000, M_vel_count + 1 );
    M_seen_vel_count = std::min( 1000, M_seen_vel_count + 1 );
    M_heard_vel_count = std::min( 1000, M_heard_vel_count + 1 );
    M_lost_count = std::min( 1000, M_lost_count + 1 );

    // M_ghost_count = 0;

    // M_rpos is updated using visual info or self info
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateByFullstate( const Vector2D & pos,
                               const Vector2D & vel,
                               const Vector2D & self_pos )
{
    M_pos = pos;
    M_pos_error.assign( 0.0, 0.0 );
    M_pos_count = 0;

    M_rpos = pos - self_pos;
    M_rpos_error.assign( 0.0, 0.0 );
    M_rpos_count = 0;

    M_seen_pos = pos;
    M_seen_rpos = M_rpos;
    M_seen_pos_count = 0;

    M_vel = vel;
    M_vel_error.assign( 0.0, 0.0 );
    M_vel_count = 0;

    M_seen_vel = vel;
    M_seen_vel_count = 0;

    M_lost_count = 0;

    M_ghost_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateWindEffect()
{
    // ball_speed_max is not considerd in rcssserver
    // wind effect
#if 0
    if ( ! ServerParam::i().windNone() ) // use wind
    {
        if ( ! ServerParam::i().useWindRandom() ) // but static initialization
        {
            Vector2D wind_vector( 1,
                                  ServerParam::i().windForce(),
                                  ServerParam::i().windDir() );
            double speed = M_vel.r();

            Vector2D wind_effect( speed * wind_vector.x / (weight * WIND_WEIGHT),
                                  speed * wind_vector.y / (weight * WIND_WEIGHT) );
            M_vel += wind_effect;

            Vector2D wind_error( speed * wind_vector.x * ServerParam::i().windRand()
                                 / (ServerParam::i().playerWeight() * WIND_WEIGHT),
                                 speed * wind_vector.y * ServerParam::i().windRand()
                                 / (ServerParam::i().playerWeight() * WIND_WEIGHT) );
            M_vel_error.add( wind_error, wind_error );
        }
        else
        {
            // it is necessary to estimate wind force & dir

        }
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateByCollision( const Vector2D & pos,
                               const int pos_count,
                               const Vector2D & rpos,
                               const int rpos_count,
                               const Vector2D & vel,
                               const int vel_count )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateByCollision)"
                  " pos(%.2f %.2f)count=%d rpos=(%2.f %.2f)count=%d vel=(%.2f %.2f)count=%d",
                  pos.x, pos.y, pos_count,
                  rpos.x, rpos.y, rpos_count,
                  vel.x, vel.y, vel_count );
#endif
    M_pos = pos;
    M_pos_count = pos_count;
    M_rpos = rpos;
    M_rpos_count = rpos_count;
    M_vel = vel;
    M_vel_count = vel_count;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateByGameMode( const GameMode & mode )
{
    const GameMode::Type type = mode.type();

    if ( type == GameMode::PlayOn
         || type == GameMode::GoalKick_
         //|| type == GameMode::GoalieCatch_
         || type == GameMode::PenaltyTaken_ )
    {
        return;
    }

    M_vel.assign( 0.0, 0.0 );
    M_vel_error.assign( 0.0, 0.0 );
    M_vel_count = 0;
    M_seen_vel.assign( 0.0, 0.0 );
    M_seen_vel_count = 0;

    if ( type == GameMode::GoalieCatch_ )
    {
        // reset only the velocity
        return;
    }

    if ( type == GameMode::CornerKick_ )
    {
        if ( posCount() <= 1
             && M_rpos.r2() > std::pow( 3.0, 2 ) )
        {
            M_pos.x = ( M_pos.x > 0.0
                        ? +ServerParam::i().pitchHalfLength() - ServerParam::i().cornerKickMargin()
                        : -ServerParam::i().pitchHalfLength() + ServerParam::i().cornerKickMargin() );
            M_pos.y = ( M_pos.y > 0.0
                        ? +ServerParam::i().pitchHalfWidth() - ServerParam::i().cornerKickMargin()
                        : -ServerParam::i().pitchHalfWidth() + ServerParam::i().cornerKickMargin() );
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateByGameMode) corner_kick. set to corner." );
#endif
        }
        return;
    }

    if ( type == GameMode::KickIn_ )
    {
        if ( posCount() <= 1
             && M_rpos.r2() > std::pow( 3.0, 2 ) )
        {
            M_pos.y = ( M_pos.y > 0.0
                        ? +ServerParam::i().pitchHalfWidth()
                        : -ServerParam::i().pitchHalfWidth() );
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateByGameMode) kick_in. set on the side line." );
#endif
        }
        return;
    }

    if ( type == GameMode::BeforeKickOff
         || type == GameMode::KickOff_ )
    {
        M_pos.assign( 0.0, 0.0 );
        M_pos_count = 0;
        M_seen_pos.assign( 0.0, 0.0 );
        M_lost_count = 0;

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByGameMode) before_kick_off. set to center." );
#endif
        return;
    }

}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateOnlyRelativePos( const Vector2D & rpos,
                                   const Vector2D & rpos_err )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateOnlyRelativePos)"
                  " rpos=(%2.f %.2f) error=(%f %f)",
                  rpos.x, rpos.y,
                  rpos_err.x, rpos_err.y );
#endif
    M_rpos = rpos;
    M_rpos_error = rpos_err;
    M_rpos_count = 0;

    M_seen_rpos = rpos;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateOnlyVel( const Vector2D & vel,
                           const Vector2D & vel_err,
                           const int vel_count )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateOnlyVel)"
                  " vel=(%2.f %.2f) error=(%f %f) count=%d",
                  vel.x, vel.y,
                  vel_err.x, vel_err.y,
                  vel_count );
#endif
    M_vel = vel;
    M_vel_error = vel_err;
    M_vel_count = vel_count;

    M_seen_vel = vel;
    M_seen_vel_count = vel_count;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::setPlayerKickable()
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (setPlayerKickable) velocity is set to Zero." );
#endif
    M_vel_error += vel();
    M_vel_count += 1;

    M_vel.assign( 0.0, 0.0 );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updatePos( const Vector2D & pos,
                       const Vector2D & pos_err,
                       const int pos_count,
                       const Vector2D & rpos,
                       const Vector2D & rpos_err )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updatePos)"
                  " pos(%.1f %.1f) count=%d",
                  pos.x, pos.y, pos_count );
#endif

    M_pos = pos;
    M_pos_error = pos_err;
    M_pos_count = pos_count;
    M_seen_pos = pos;
    M_seen_pos_count = 0;

    updateOnlyRelativePos( rpos, rpos_err );

    M_lost_count = 0;
    M_ghost_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateAll( const Vector2D & pos,
                       const Vector2D & pos_err,
                       const int pos_count,
                       const Vector2D & rpos,
                       const Vector2D & rpos_err,
                       const Vector2D & vel,
                       const Vector2D & vel_err,
                       const int vel_count )
{
#ifdef DEBUG_PRINT
    dlog.addText( Logger::WORLD,
                  __FILE__" (updateAll)" );
#endif
    updatePos( pos, pos_err, pos_count, rpos, rpos_err );
    updateOnlyVel( vel, vel_err, vel_count );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateByHear( const ActionEffector & act,
                          const double & sender_to_ball_dist,
                          const Vector2D & heard_pos,
                          const Vector2D & heard_vel,
                          const bool pass )
{
    // double heard_speed = 0.0;
    if ( heard_vel.isValid() )
    {
        // heard_speed = heard_vel.r();
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByHear)"
                      " heard_pos=(%.2f, %.2f)"
                      " heard_vel=(%.2f, %.2f)",
                      heard_pos.x, heard_pos.y,
                      heard_vel.x, heard_vel.y );
#endif
    }
#ifdef DEBUG_PRINT
    else
    {
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByHear)"
                      " heard_pos=(%.2f, %.2f) no vel",
                      heard_pos.x, heard_pos.y );
    }
#endif

    M_heard_pos = heard_pos;
    M_heard_pos_count = 0;
    M_heard_vel = heard_vel;
    M_heard_vel_count = 0;

    if ( act.lastBodyCommandType() == PlayerCommand::KICK )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByHear) last command is kick." );
#endif
        return;
    }

    const double dist_diff = heard_pos.dist( pos() );

    if ( pass
         && heard_vel.isValid()
         && seenVelCount() > 0 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByHear) update by pass." );
#endif
        if ( seenPosCount() > 0 )
        {
            M_pos = heard_pos;
            M_pos_count = 1;
        }
        M_vel = heard_vel;
        M_vel_count = 1;
        return;
    }

    if ( M_ghost_count > 0 )
    {
        if ( ( M_ghost_count == 1
               && posCount() == 1
               && dist_diff < 3.0 )
             || M_ghost_count > 1 )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateByHear) ghost detected." );
#endif
            M_pos = heard_pos;
            M_pos_count = 1;
            if ( heard_vel.isValid() )
            {
                M_vel = heard_vel;
                M_vel_count = 1;
            }
            return;
        }
    }

    if ( posCount() >= 5
         // || ( posCount() >= 1
         //      && velCount() >= 2
         //      && heard_vel.isValid()
         //      && ( dist_diff > sender_to_ball_dist * 0.05 + 1.0
         //           || sender_to_ball_dist < M_dist_from_self * 0.95 ) ) )
         || ( posCount() >= 2
              && ( dist_diff > sender_to_ball_dist * 0.05 + 1.0
                   || sender_to_ball_dist < M_dist_from_self * 0.95 ) ) )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByHear) big difference from last internal state." );
#endif
        M_pos = heard_pos;
        M_pos_count = 1;
        if ( heard_vel.isValid() )
        {
            M_vel = heard_vel;
            M_vel_count = 1;
        }
        return;
    }

#if 1
    // 2017-07-18
    if ( posCount() > 0
         && distFromSelf() > ServerParam::i().visibleDistance() // NOTE: previous cycle info
         && sender_to_ball_dist < ServerParam::i().visibleDistance() - 1.0 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateByHear) teammate near to the ball." );
#endif
        M_pos = heard_pos;
        M_pos_count = 1;
        if ( heard_vel.isValid() )
        {
            M_vel = heard_vel;
            M_vel_count = 1;
        }
        return;
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

 */
void
BallObject::updateSelfRelated( const SelfObject & self,
                               const BallObject & prev )
{
    // seen
    if ( rposCount() == 0 )
    {
        // M_rpos is already updated
        M_dist_from_self = rpos().r();
        M_angle_from_self = rpos().th();
    }
    // not seen
    else
    {
        // update rpos
        if ( prev.rpos().isValid()
             && self.lastMove().isValid() )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateSelfRelated) update rpos using self move(%.2f %.2f)",
                          self.lastMove().x, self.lastMove().y );
#endif
            M_rpos
                = prev.rpos()
                + ( vel() / ServerParam::i().ballDecay() )
                - self.lastMove();
            M_rpos_error += velError();
            M_rpos_error += ( self.velError() / self.playerType().playerDecay() );
        }
        // it is not necessary to consider other case.

        // update dist & angle

        // at least, rpos is valid
        if ( rpos().isValid()
             && posCount() > rposCount() )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateSelfRelated) set pos by rpos(%.2f %.2f)",
                          rpos().x, rpos().y );
#endif
            M_pos = self.pos() + this->rpos();
            M_pos_error = self.posError() + this->rposError();
            M_dist_from_self = rpos().r();
            M_angle_from_self = rpos().th();
        }
        else if ( posValid()
                  && self.posValid() )
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateSelfRelated) set rpos by global pos" );
#endif
            M_rpos = pos() - self.pos();
            M_rpos_error = posError() + self.posError();
            M_dist_from_self = rpos().r();
            M_angle_from_self = rpos().th();
        }
        else
        {
#ifdef DEBUG_PRINT
            dlog.addText( Logger::WORLD,
                          __FILE__" (updateSelfRelated) failed" );
#endif
            M_dist_from_self = 1000.0;
            M_angle_from_self = 0.0;
        }
    }
}

}
