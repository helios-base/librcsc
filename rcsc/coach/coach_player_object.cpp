// -*-c++-*-

/*!
  \file coach_player_object.cpp
  \brief player object class for coach/trainer Source file.
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

#include "coach_player_object.h"

#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/rcg/types.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CoachPlayerObject::CoachPlayerObject()
    : M_side( NEUTRAL ),
      M_unum( Unum_Unknown ),
      M_goalie( false ),
      M_type( Hetero_Unknown ),
      M_player_type( nullptr ),
      M_pos( Vector2D::INVALIDATED ),
      M_vel( 0.0, 0.0 ),
      M_body( 0.0 ),
      M_face( 0.0 ),
      M_stamina(),
      M_pointto_cycle( 0 ),
      M_pointto_angle( 0.0 ),
      M_kicking( false ),
      M_tackle_cycle( 0 ),
      M_charged_cycle( 0 ),
      M_card( NO_CARD ),
      M_ball_reach_step( 1000 )
{
    M_stamina.init( PlayerTypeSet::i().defaultType() );
}

/*-------------------------------------------------------------------*/
/*!

*/
CoachPlayerObject *
CoachPlayerObject::clone() const
{
    CoachPlayerObject * p = new CoachPlayerObject();
    *p = *this;
    return p;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachPlayerObject::setPlayerType( const int type )
{
    if ( M_type == type
         && M_player_type )
    {
        return;
    }

    dlog.addText( Logger::WORLD,
                  __FILE__":(setPlayerType) player %c %d, change_player_type %d -> %d",
                  side_char( M_side ), M_unum,
                  M_type, type );

    if ( M_type != Hetero_Unknown )
    {
        changePlayerType( type );
    }
    else
    {
        M_type = type;
        M_player_type = PlayerTypeSet::i().get( type );

        if ( M_player_type )
        {
            M_stamina.setEffort( M_player_type->effortMax() );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachPlayerObject::changePlayerType( const int type )
{
    M_type = type;
    M_player_type = PlayerTypeSet::i().get( type );
    M_card = NO_CARD;

    if ( M_player_type )
    {
        M_stamina.init( *M_player_type );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachPlayerObject::update( const CoachPlayerObject & p )
{
    M_side = p.side();
    M_unum = p.unum();
    M_goalie = p.goalie();

    // *** Do NOT set player type here! ***
    // M_type = p.type();
    // M_player_type = p.playerTypePtr();

    M_pos = p.pos();
    M_vel = p.vel();

    M_body = p.body();
    M_face = p.face();

    if ( p.isPointing() )
    {
        ++M_pointto_cycle;
        M_pointto_angle = p.pointtoAngle();
    }
    else
    {
        M_pointto_cycle = 0;
    }

    M_kicking = p.isKicking();

    if ( p.isTackling() )
    {
        ++M_tackle_cycle;
        if ( M_tackle_cycle > ServerParam::i().tackleCycles() )
        {
            M_tackle_cycle = 1;
        }
    }
    else
    {
        M_tackle_cycle = 0;
    }

    if ( p.isCharged() )
    {
        ++M_charged_cycle;
        if ( M_charged_cycle > ServerParam::i().foulCycles() )
        {
            M_charged_cycle = 1;
        }
    }
    else
    {
        M_charged_cycle = 0;
    }

    M_card = p.card();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachPlayerObject::update( const rcg::PlayerT & p )
{
    if ( ! p.isAlive() )
    {
        return;
    }

    M_side = p.side();
    M_unum = p.unum_;
    M_goalie = p.isGoalie();

    // setPlayerType() must be called before updating stamina information
    setPlayerType( p.type_ );

    M_pos.assign( p.x_, p.y_ );
    if ( p.hasVelocity() )
    {
        M_vel.assign( p.vx_, p.vy_ );
    }

    M_body = p.body_;
    if ( p.hasNeck() )
    {
        M_face = p.body_ + p.neck_;
    }
    else
    {
        M_face = M_body;
    }

    if ( p.hasStamina() )
    {
        M_stamina.setValues( p.stamina_,
                             p.effort_,
                             p.recovery_,
                             p.stamina_capacity_ );
    }

    if ( p.isPointing() )
    {
        ++M_pointto_cycle;
        M_pointto_angle = ( Vector2D( p.point_x_, p.point_y_ ) - M_pos ).th();
    }
    else
    {
        M_pointto_cycle = 0;
        M_pointto_angle = 0.0;
    }

    M_kicking = p.isKicking();

    if ( p.isTackling() )
    {
        ++M_tackle_cycle;
        if ( M_tackle_cycle > ServerParam::i().tackleCycles() )
        {
            M_tackle_cycle = 1;
        }
    }
    else
    {
        M_tackle_cycle = 0;
    }

    if ( p.isFoulCharged() )
    {
        ++M_charged_cycle;
        if ( M_charged_cycle > ServerParam::i().foulCycles() )
        {
            M_charged_cycle = 1;
        }
    }
    else
    {
        M_charged_cycle = 0;
    }

    if ( p.hasYellowCard() )
    {
        M_card = YELLOW;
    }
    else if ( p.hasRedCard() )
    {
        M_card = RED;
    }

}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachPlayerObject::recoverStamina()
{
    const double effort = ( M_player_type
                            ? M_player_type->effortMax()
                            : ServerParam::i().defaultEffortMax() );

    M_stamina.setValues( ServerParam::i().staminaMax(),
                         effort,
                         ServerParam::i().recoverInit(),
                         ServerParam::i().staminaCapacity() );

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachPlayerObject::print( std::ostream & os ) const
{
    os << "Player (" << ( M_side == LEFT ? "l " : "r " )
       << M_unum << ( M_goalie ? " g) " : ") " )
       << pos() << ' ' << vel() << ' '
       << body() << ' ' << face();

    if ( isPointing() )
    {
        os << " arm:cycle=" << pointtoCycle()
           << "dir=" << pointtoAngle();
    }

    if ( isKicking() )
    {
        os << " kicking";
    }

    if ( isTackling() )
    {
        os << " tackle=" << tackleCycle();
    }

    return os;
}

}
