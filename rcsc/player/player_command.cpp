// -*-c++-*-

/*!
  \file player_command.cpp
  \brief player command classes Source File
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

#include "player_command.h"

#include "see_state.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
PlayerInitCommand::PlayerInitCommand( const std::string & team_name,
                                      const double & version,
                                      const bool goalie )
    : M_team_name( team_name ),
      M_version( version ),
      M_goalie( goalie )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerInitCommand::toCommandString( std::ostream & to ) const
{
    if ( M_goalie && M_version >= 4.0 )
    {
        // "(init TEAMNAME (version VER) (goalie))"
        to <<  "(init " << M_team_name << " (version " << M_version << ") (goalie))";
    }
    else if ( M_version >= 4.0 )
    { // "(init TEAMNAME (version VER))"
        to <<  "(init " << M_team_name << " (version " << M_version << "))";
    }
    else
    {
        to << "(init " << M_team_name << ")";
    }
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerReconnectCommand::PlayerReconnectCommand( const std::string & team_name,
                                                const int unum )
    : M_team_name( team_name ),
      M_unum( unum )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerReconnectCommand::toCommandString( std::ostream & to ) const
{
    return to << "(reconnect " << M_team_name << " " << M_unum << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerByeCommand::PlayerByeCommand()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerByeCommand::toCommandString( std::ostream & to ) const
{
    return to << "(bye)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerMoveCommand::toCommandString( std::ostream & to ) const
{
    return to << "(move " << M_x << " " << M_y << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerDashCommand::toCommandString( std::ostream & to ) const
{
    to << "(dash " << M_power;
    if ( M_dir != 0.0 )
    {
        to << ' ' << M_dir;
    }
    return to << ')';
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerTurnCommand::toCommandString( std::ostream & to ) const
{
    return to << "(turn " << M_moment << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerKickCommand::toCommandString( std::ostream & to ) const
{
    return to << "(kick " << M_power << " " << M_dir << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerCatchCommand::toCommandString( std::ostream & to ) const
{
    return to << "(catch " << M_dir << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerTackleCommand::toCommandString( std::ostream & to ) const
{
    to << "(tackle " << M_power_or_dir;
    if ( M_foul )
    {
        to << " on";
    }
    to << ')';

    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerTurnNeckCommand::toCommandString( std::ostream & to ) const
{
    return to << "(turn_neck " << M_moment << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerChangeViewCommand::toCommandString( std::ostream & to ) const
{
    to << "(change_view " << M_width.str();

    if ( ! SeeState::synch_see_mode() )
    {
        to << " " << M_quality.str() << ')';
    }
    else
    {
        to << ')';
    }

    return to;
}


/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerChangeFocusCommand::toCommandString( std::ostream & to ) const
{
    return to << "(change_focus " << M_moment_dist << ' ' << M_moment_dir << ')';
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerSayCommand::toCommandString( std::ostream & to ) const
{
    if ( ! M_message.empty() )
    {
        if ( M_version >= 8.0 )
        {
            to << "(say \"" << M_message << "\")";
        }
        else
        {
            to << "(say " << M_message << ")";
        }
    }
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerPointtoCommand::toCommandString( std::ostream & to ) const
{
    if ( M_on )
    {
        to << "(pointto " << M_dist << " " << M_dir << ")";
    }
    else
    {
        to << "(pointto off)";
    }
    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerAttentiontoCommand::toCommandString( std::ostream & to ) const
{
    if ( M_side != NONE )
    {
        to << "(attentionto ";
        if ( M_side == OUR ) to << "our " << M_number << ")";
        else                 to << "opp " << M_number << ")";
    }
    else
    {
        to << "(attentionto off)";
    }
    return to;
}


/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerCLangCommand::toCommandString( std::ostream & to ) const
{
    return to << "(clang (ver " << M_min << " " << M_max << "))";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerEarCommand::toCommandString( std::ostream & to ) const
{
    to << "(ear (";

    // set on/off
    if ( M_onoff == ON )
    {
        to << "on";
    }
    else
    {
        to << "off";
    }

    // set side
    if ( M_side == OUR )
    {
        to << " our";
    }
    else
    {
        to << " opp";
    }

    // set mode
    switch ( M_mode ) {
    case COMPLETE:
        to << " complete";
        break;
    case PARTIAL:
        to << " partial";
        break;
    default:
        break;
    }

    return to << "))";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerSenseBodyCommand::toCommandString( std::ostream & to ) const
{
    return to << "(sense_body)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerScoreCommand::toCommandString( std::ostream & to ) const
{
    return to << "(score)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerCompressionCommand::toCommandString( std::ostream & to ) const
{
    return to << "(compression " << M_level << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PlayerDoneCommand::toCommandString( std::ostream & to ) const
{
    return to << "(done)";
}

}
