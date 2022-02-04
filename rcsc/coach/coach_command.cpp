// -*-c++-*-

/*!
  \file coach_command.cpp
  \brief coach command classes Source File
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

#include "coach_command.h"

#include <rcsc/common/player_param.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CoachInitCommand::CoachInitCommand( const std::string & team_name,
                                    const double & version,
                                    const std::string & coach_name )
    : M_team_name( team_name )
    , M_version( version )
    , M_coach_name( coach_name )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachInitCommand::toCommandString( std::ostream & to ) const
{
    to << "(init " << M_team_name;
    if ( ! M_coach_name.empty() )
    {
        to << " " << M_coach_name;
    }
    return to << " (version " << M_version << "))";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachByeCommand::toCommandString( std::ostream & to ) const
{
    return to << "(bye)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachCheckBallCommand::toCommandString( std::ostream & to ) const
{
    return to << "(check_ball)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachLookCommand::toCommandString( std::ostream & to ) const
{
    return to << "(look)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachTeamNamesCommand::toCommandString( std::ostream & to ) const
{
    return to << "(team_names)";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachEyeCommand::toCommandString( std::ostream & to ) const
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
CoachChangePlayerTypeCommand::toCommandString( std::ostream & to ) const
{
    if ( M_type < 0
         || PlayerParam::i().playerTypes() <= M_type )
    {
        std::cerr << "CoachChangePlayerTypeCommand: illegal type Id"
                  << M_type
                  << std::endl;
        return to;
    }

    return to << "(change_player_type "
              << M_unum << " "
              << M_type << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
CoachChangePlayerTypesCommand::CoachChangePlayerTypesCommand( const int unum,
                                                              const int type )
{
    add( unum, type );
}

/*-------------------------------------------------------------------*/
/*!

*/
CoachChangePlayerTypesCommand::
CoachChangePlayerTypesCommand( const std::vector< std::pair< int, int > > & types )
{
    M_types.reserve( types.size() );

    for ( const std::pair< int, int > & v : types )
    {
        add( v.first, v.second );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CoachChangePlayerTypesCommand::add( const int unum,
                                    const int type )
{
    if ( unum < 1
         || 11 < unum )
    {
        std::cerr << "(CoachChangePlayerTypesCommand::add) Illegal player number "
                  << unum
                  << std::endl;
        return;
    }

    if ( type < 0
         || PlayerParam::i().playerTypes() <= type )
    {
        std::cerr << "(CoachChangePlayerTypesCommand::add) Illegal player type id "
                  << type
                  << std::endl;
        return;
    }

    for ( const std::pair< int, int > & v : M_types )
    {
        if ( v.first == unum )
        {
            std::cerr << "(CoachChangePlayerTypesCommand::add) unum "
                      << unum << " is already registered. overwritten."
                      << std::endl;
            return;
        }
    }

    M_types.push_back( std::pair< int, int >( unum, type ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachChangePlayerTypesCommand::toCommandString( std::ostream & to ) const
{
    if ( M_types.empty() )
    {
        std::cerr << "CoachChangePlayerTypesCommand::toCommandString()  Empty data!"
                  << std::endl;
        return to;
    }

    to << "(change_player_types ";

    for ( const std::pair< int, int > & v : M_types )
    {
        if ( v.first < 1
             || 11 < v.first
             || v.second < 0
             || PlayerParam::i().playerTypes() <= v.second )
        {
            continue;
        }

        to << '(' << v.first << ' ' << v.second << ')';
    }

    return to << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachFreeformCommand::toCommandString( std::ostream & to ) const
{
    if ( M_version < 7.0 )
    {
        to << "(say " << M_message << ")";
    }
    else
    {
        to << "(say (freeform \"" << M_message << "\"))";
    }

    return to;
}

/*-------------------------------------------------------------------*/
/*!

*/
CoachTeamGraphicCommand::
CoachTeamGraphicCommand( const unsigned int x,
                         const unsigned int y,
                         const std::vector< std::string > & xpm_lines )
    : M_x( x )
    , M_y( y )
    , M_xpm_lines( xpm_lines )
{
    if ( M_xpm_lines.size() > 8 )
    {
        std::cerr << "team_graphic: xpm over flow" << std::endl;
        M_xpm_lines.erase( M_xpm_lines.begin() + 8, M_xpm_lines.end() );
    }
    for ( std::string & str : M_xpm_lines )
    {
        if ( str.size() > 8 )
        {
            std::cerr << "team_graphic: xpm line over flow" << std::endl;
            str.erase( str.begin() + 8, str.end() );
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachTeamGraphicCommand::toCommandString( std::ostream & to ) const
{
    to << "(team_graphic (" << M_x << " " << M_y;

    for ( const std::string & str : M_xpm_lines )
    {
        to << " \"" << str << "\"";
    }
    return to << "))";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachCompressionCommand::toCommandString( std::ostream & to ) const
{
    return to << "(compression " << M_level << ")";
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
CoachDoneCommand::toCommandString( std::ostream & to ) const
{
    return to << "(done)";
}

}
