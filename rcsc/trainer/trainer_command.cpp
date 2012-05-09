// -*-c++-*-

/*!
  \file trainer_command.cpp
  \brief trainer command classes Source File
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

#include "trainer_command.h"

#include <rcsc/common/player_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerInitCommand::toCommandString( std::ostream & to ) const
{
    return to << "(init (version " << M_version << "))";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerCheckBallCommand::toCommandString( std::ostream & to ) const
{
    return to << "(check_ball)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerLookCommand::toCommandString( std::ostream & to ) const
{
    return to << "(look)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerTeamNamesCommand::toCommandString( std::ostream & to ) const
{
    return to << "(team_names)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerEarCommand::toCommandString( std::ostream & to ) const
{
    if ( M_on )
    {
        return to << "(ear on)";
    }
    else
    {
        return to << "(ear off)";
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerEyeCommand::toCommandString( std::ostream & to ) const
{
    if ( M_on )
    {
        return to << "(eye on)";
    }
    else
    {
        return to << "(eye off)";
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerKickOffCommand::toCommandString( std::ostream & to ) const
{
    return to << "(start)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerChangeModeCommand::toCommandString( std::ostream & to ) const
{
    static const char * pmodes[] = PLAYMODE_STRINGS;
    return to << "(change_mode " << pmodes[M_playmode] << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMoveBallCommand::TrainerMoveBallCommand( const double & x,
                                                const double & y )
    : M_pos( x, y )
    , M_vel( Vector2D::INVALIDATED )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMoveBallCommand::TrainerMoveBallCommand( const Vector2D & pos )
    : M_pos( pos )
    , M_vel( Vector2D::INVALIDATED )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMoveBallCommand::TrainerMoveBallCommand( const double & x,
                                                const double & y,
                                                const double & vx,
                                                const double & vy )
    : M_pos( x, y )
    , M_vel( vx, vy )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMoveBallCommand::TrainerMoveBallCommand( const Vector2D & pos,
                                                const Vector2D & vel )
    : M_pos( pos )
    , M_vel( vel )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerMoveBallCommand::toCommandString( std::ostream & to ) const
{
    to << "(move (ball) " << M_pos.x << " " << M_pos.y;

    if ( ! M_vel.isValid() )
    {
        to << ")";
    }
    else
    {
        to << " 0 " << M_vel.x << " " << M_vel.y << ")";
    }
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const double & x,
                                                    const double & y )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( x, y )
    , M_angle( -360.0 )
    , M_vel( Vector2D::INVALIDATED )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const Vector2D & pos )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( pos )
    , M_angle( -360.0 )
    , M_vel( Vector2D::INVALIDATED )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const double & x,
                                                    const double & y,
                                                    const AngleDeg & angle )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( x, y )
    , M_angle( angle.degree() )
    , M_vel( Vector2D::INVALIDATED )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const Vector2D & pos,
                                                    const AngleDeg & angle )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( pos )
    , M_angle( angle.degree() )
    , M_vel( Vector2D::INVALIDATED )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const double & x,
                                                    const double & y,
                                                    const AngleDeg & angle,
                                                    const double & vx,
                                                    const double & vy )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( x, y )
    , M_angle( angle.degree() )
    , M_vel( vx, vy )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const Vector2D & pos,
                                                    const AngleDeg & angle,
                                                    const double & vx,
                                                    const double & vy )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( pos )
    , M_angle( angle.degree() )
    , M_vel( vx, vy )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerMovePlayerCommand::TrainerMovePlayerCommand( const std::string & team_name,
                                                    const int unum,
                                                    const Vector2D & pos,
                                                    const AngleDeg & angle,
                                                    const Vector2D & vel )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_pos( pos )
    , M_angle( angle.degree() )
    , M_vel( vel )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerMovePlayerCommand::check() const
{
    if ( M_unum < 1 || 11 < M_unum )
    {
        std::cerr << "TranierMovePlayerCommand: illegal uniform number"
                  << std::endl;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerMovePlayerCommand::toCommandString( std::ostream & to ) const
{
    if ( ! check() )
    {
        return to;
    }

    to << "(move (player "
       << M_team_name << " "
       << M_unum << ") "
       << M_pos.x << " " << M_pos.y;

    if ( M_angle == -360.0 )
    {
        return to << ")";
    }

    to << " " << M_angle;

    if ( M_vel.isValid() )
    {
        to << " " << M_vel.x << " " << M_vel.y;
    }

    return to << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerRecoverCommand::toCommandString( std::ostream & to ) const
{
    return to << "(recover)";
}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerChangePlayerTypeCommand::
TrainerChangePlayerTypeCommand( const std::string & team_name,
                                const int unum,
                                const int type )
    : M_team_name( team_name )
    , M_unum( unum )
    , M_type( type )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerChangePlayerTypeCommand::toCommandString( std::ostream & to ) const
{
    if ( M_type < Hetero_Unknown
         || PlayerParam::i().playerTypes() <= M_type )
    {
        std::cerr << "TranierChangePlayerTypeCommand: illegal type Id"
                  << M_type
                  << std::endl;
        return to;
    }

    to << "(change_player_type "
       << M_team_name << " "
       << M_unum << " "
       << M_type << ")";
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerSayCommand::toCommandString( std::ostream & to ) const
{
    return to << "(say " << M_message << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerCompressionCommand::toCommandString( std::ostream & to ) const
{
    return to << "(compression " << M_level << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TrainerDoneCommand::toCommandString( std::ostream & to ) const
{
    return to << "(done)";
}

}
