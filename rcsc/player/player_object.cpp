// -*-c++-*-

/*!
  \file player_object.cpp
  \brief player object class Source File
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

#include "player_object.h"

#include "fullstate_sensor.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_type.h>

// #define DEBUG_PRINT

namespace rcsc {

int PlayerObject::S_pos_count_thr = 30;
int PlayerObject::S_vel_count_thr = 5;
int PlayerObject::S_face_count_thr = 2;

int PlayerObject::S_player_count = 0;

/*-------------------------------------------------------------------*/
/*!

*/
PlayerObject::PlayerObject()
    : AbstractPlayerObject( ++S_player_count ),
      M_ghost_count( 0 ),
      M_tackle_count( 1000 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerObject::PlayerObject( const SideID side,
                            const Localization::PlayerT & p )
    : AbstractPlayerObject( ++S_player_count, side, p ),
      M_ghost_count( 0 ),
      M_tackle_count( 1000 )
{
    M_dist_from_self = p.rpos_.r();

    if ( p.hasVel() )
    {
        M_vel = p.vel_;
        M_vel_count = 0;
    }

    if ( p.hasAngle() )
    {
        M_body = p.body_;
        M_body_count = 0;
        M_face = p.face_;
        M_face_count = 0;
    }

    if ( p.isPointing() )
    {
        M_pointto_angle = p.arm_;
        M_pointto_count = 0;
    }

    M_kicking = p.kicking_;

    if ( p.isTackling() )
    {
        if ( M_tackle_count > ServerParam::i().tackleCycles() ) // no tackling recently
        {
            M_tackle_count = 0;
        }
    }
    else
    {
        M_tackle_count = 1000;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::set_count_thr( const int pos_thr,
                             const int vel_thr,
                             const int face_thr )
{
    S_pos_count_thr = pos_thr;
    S_vel_count_thr = vel_thr;
    S_face_count_thr = face_thr;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::reset_player_count()
{
    S_player_count = 0;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerObject::isTackling() const
{
    return M_tackle_count <= ServerParam::i().tackleCycles() - 2;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerObject::isKickable( const double & buf ) const
{
    if ( ! M_player_type )
    {
        return distFromBall() < ServerParam::i().defaultKickableArea();
    }

    return distFromBall() < M_player_type->kickableArea() - buf;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::update()
{
    M_pos_history.push_front( M_pos );
    if ( M_pos_history.size() > 100 )
    {
        M_pos_history.pop_back();
    }

    if ( velValid() )
    {
        M_pos += M_vel;
        // speed is not decayed in internal update.
    }
    /*
    // wind effect
    //vel += wind_effect( ServerParam::i().windRand(),
    //                          ServerParam::i().windForce(),
    //                          ServerParam::i().windDir());
    */
    M_unum_count = std::min( 1000, M_unum_count + 1 );
    M_pos_count = std::min( 1000, M_pos_count + 1 );
    M_seen_pos_count = std::min( 1000, M_seen_pos_count + 1 );
    M_heard_pos_count = std::min( 1000, M_heard_pos_count + 1 );
    M_vel_count = std::min( 1000, M_vel_count + 1 );
    M_body_count = std::min( 1000, M_body_count + 1 );
    M_face_count = std::min( 1000, M_face_count + 1 );
    M_pointto_count = std::min( 1000, M_pointto_count + 1 );
    M_kicking = false;
    M_tackle_count = std::min( 1000, M_tackle_count + 1 );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateBySee( const SideID side,
                           const Localization::PlayerT & p )
{
    M_side = side;
    M_ghost_count = 0;

    // unum is updated only when unum is seen.
    if ( p.unum_ != Unum_Unknown )
    {
        M_unum = p.unum_;
        M_unum_count = 0;
        if ( ! p.goalie_ )
        {
            // when unum is seen, goalie info is also seen
            M_goalie = false;
        }
    }

    if ( p.goalie_ )
    {
        M_goalie = true;
    }

    const Vector2D last_seen_move = p.pos_ - M_seen_pos;
    const int last_seen_pos_count = M_seen_pos_count;

    if ( p.hasVel() )
    {
        M_vel = p.vel_;
        M_vel_count = 0;
        M_seen_vel = p.vel_;
        M_seen_vel_count = 0;

#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateBySee) unum=%d. pos=(%.2f, %.2f) vel=(%.2f, %.2f)",
                      p.unum_, M_pos.x, M_pos.y, M_vel.x, M_vel.y );
#endif
    }
    else if ( 0 < M_pos_count
              && M_pos_count <= 2
              && p.rpos_.r2() < std::pow( 40.0, 2 ) )
    {
        const double speed_max = ( M_player_type
                                   ? M_player_type->realSpeedMax()
                                   : ServerParam::i().defaultRealSpeedMax() );
        const double decay = ( M_player_type
                               ? M_player_type->playerDecay()
                               : ServerParam::i().defaultPlayerDecay() );

        M_vel = last_seen_move / static_cast< double >( last_seen_pos_count );
        double tmp = M_vel.r();
        if ( tmp > speed_max )
        {
            M_vel *= speed_max / tmp;
        }
        M_vel *= decay;
        M_vel_count = last_seen_pos_count;

        M_seen_vel = M_vel;
        M_seen_vel_count = 0;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::WORLD,
                      __FILE__" (updateBySee) unum=%d. update vel by pos diff."
                      "prev_pos=(%.2f, %.2f) old_pos=(%.2f, %.2f) -> vel=(%.3f, %.3f)",
                      p.unum_,
                      M_pos.x, M_pos.y, p.pos_.x, p.pos_.y, M_vel.x, M_vel.y );
#endif
    }
    else
    {
        M_vel.assign( 0.0, 0.0 );
        M_vel_count = 1000;
    }

    M_pos = p.pos_;
    M_seen_pos = p.pos_;

    M_pos_count = 0;
    M_seen_pos_count = 0;

    if ( p.hasAngle() )
    {
        M_body = p.body_;
        M_face = p.face_;
        M_body_count = M_face_count = 0;
    }
    else if ( last_seen_pos_count <= 2
              && last_seen_move.r2() > std::pow( 0.2, 2 ) ) // Magic Number
    {
        M_body = last_seen_move.th();
        M_body_count = std::max( 0, last_seen_pos_count - 1 );
        M_face = 0.0;
        M_face_count = 1000;
    }
    else if ( velValid()
              && vel().r2() > std::pow( 0.2, 2 ) ) // Magic Number
    {
        M_body = vel().th();
        M_body_count = velCount();
        M_face = 0.0;
        M_face_count = 1000;
    }

    if ( p.isPointing()
         && M_pointto_count >= ServerParam::i().pointToBan() )
    {
        M_pointto_angle = p.arm_;
        M_pointto_count = 0;
    }

    M_kicking = p.isKicking();

    if ( p.isTackling() )
    {
        if ( M_tackle_count > ServerParam::i().tackleCycles() ) // no tackling recently
        {
            M_tackle_count = 0;
        }
    }
    else if ( p.rpos_.r2() > std::pow( ServerParam::i().visibleDistance(), 2 ) )
    {
        M_tackle_count = 1000;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateByFullstate( const FullstateSensor::PlayerT & p,
                                 const Vector2D & self_pos,
                                 const Vector2D & ball_pos )
{
    M_side = p.side_;
    M_unum = p.unum_;
    M_unum_count = 0;
    M_goalie = p.goalie_;

    M_pos = p.pos_;
    M_pos_count = 0;

    M_seen_pos = p.pos_;
    M_seen_pos_count = 0;

    M_vel = p.vel_;
    M_vel_count = 0;
    M_seen_vel = p.vel_;
    M_seen_vel_count = 0;

    M_body = p.body_;
    M_body_count = 0;
    M_face = p.body_ + p.neck_;
    M_face_count = 0;

    M_dist_from_ball = ( M_pos - ball_pos ).r();
    M_angle_from_ball = ( M_pos - ball_pos ).th();

    M_dist_from_self = self_pos.dist( p.pos_ );
    M_angle_from_self = ( p.pos_ - self_pos ).th();

    M_ghost_count = 0;

    M_pointto_angle = M_face + p.pointto_dir_;
    M_pointto_count = 0;

    M_kicking = p.kicked_;

    if ( p.tackle_ )
    {
        if ( M_tackle_count > ServerParam::i().tackleCycles() ) // no tackling recently
        {
            M_tackle_count = 0;
        }
    }
    else
    {
        M_tackle_count = 1000;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateByHear( const SideID heard_side,
                            const int heard_unum,
                            const bool goalie,
                            const Vector2D & heard_pos )
{
    M_heard_pos = heard_pos;
    M_heard_pos_count = 0;

    M_ghost_count = 0;

    if ( heard_side != NEUTRAL )
    {
        M_side = heard_side;
    }

    if ( heard_unum != Unum_Unknown
         && unumCount() > 0 )
    {
        M_unum = heard_unum;
        //M_unum_count = 1;
    }

    if ( goalie )
    {
        M_goalie = true;
    }

    if ( unumCount() > 2 )
    {
        M_unum_count = 2;
    }

    if ( seenPosCount() >= 2
         || ( seenPosCount() > 0
              && distFromSelf() > 20.0 ) )
    {
        M_pos = heard_pos;
        M_pos_count = 1;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateByHear( const SideID heard_side,
                            const int heard_unum,
                            const bool goalie,
                            const Vector2D & heard_pos,
                            const double & heard_body )
{
    updateByHear( heard_side, heard_unum, goalie, heard_pos );

    if ( heard_body != -360.0 )
    {
        if ( bodyCount() >= 2 )
        {
            M_body = heard_body;
            M_body_count = 1;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::updateSelfBallRelated( const Vector2D & self,
                                     const Vector2D & ball )
{
    M_dist_from_ball = ( M_pos - ball ).r();
    M_angle_from_ball = ( M_pos - ball ).th();
    M_dist_from_self = ( M_pos - self ).r();
    M_angle_from_self = ( M_pos - self ).th();
}


/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::setCollisionEffect()
{
    if ( M_vel.isValid() )
    {
        M_vel *= -0.1;
    }

    if ( M_seen_vel.isValid() )
    {
        M_seen_vel *= -0.1;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerObject::forget()
{
    M_pos_count
        = M_seen_pos_count
        = M_heard_pos_count
        = M_vel_count
        = M_seen_vel_count
        = M_face_count
        = M_pointto_count
        = M_tackle_count
        = 1000;
}

}
