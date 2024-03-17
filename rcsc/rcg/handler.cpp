// -*-c++-*-

/*!
  \file handler.cpp
  \brief rcg data handler Source File.
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

#include "handler.h"

#include "util.h"


#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <iostream>

namespace rcsc {
namespace rcg {

/*-------------------------------------------------------------------*/
/*!

*/
Handler::Handler()
    : M_log_version( 0 ),
      M_read_time( 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
Handler::~Handler()
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleDispInfo( const dispinfo_t & dinfo )
{
    handleLogVersion( REC_VERSION_2 );

    switch ( ntohs( dinfo.mode ) ) {
    case SHOW_MODE:
        return handleShowInfo( dinfo.body.show );
        break;
    case MSG_MODE:
        return handleMsg( M_read_time,
                          dinfo.body.msg.board,
                          std::string( dinfo.body.msg.message ) );
        break;
    case DRAW_MODE:
        return handleDrawInfo( dinfo.body.draw );
        break;
    default:
        std::cerr << __FILE__ << ':' << __LINE__
                  << " detect unsupported mode ["
                  << static_cast< int >( ntohs( dinfo.mode ) ) << ']'
                  << std::endl;
        break;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleDispInfo2( const dispinfo_t2 & dinfo2 )
{
    handleLogVersion( REC_VERSION_3 );

    switch ( ntohs( dinfo2.mode ) ) {
    case SHOW_MODE:
        return handleShowInfo2( dinfo2.body.show );
        break;
    case MSG_MODE:
        return handleMsg( M_read_time,
                          dinfo2.body.msg.board,
                          std::string( dinfo2.body.msg.message ) );
        break;
    case PT_MODE:
        return handlePlayerType( dinfo2.body.ptinfo );
        break;
    case PARAM_MODE:
        return handleServerParam( dinfo2.body.sparams );
        break;
    case PPARAM_MODE:
        return handlePlayerParam( dinfo2.body.pparams );
        break;
    default:
        std::cerr << __FILE__ << ':' << __LINE__
                  << " detect unsupported mode ["
                  << static_cast< int >( ntohs( dinfo2.mode ) ) << ']'
                  << std::endl;
        break;
    }
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleShowInfo( const showinfo_t & info )
{
    ShowInfoT show;
    convert( info, show );

    M_read_time = static_cast< int >( show.time_ );

    return ( handlePlayMode( info.pmode )
             && handleTeamInfo( info.team[0], info.team[1] )
             && handleShow( show ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleShowInfo2( const showinfo_t2 & info )
{
    ShowInfoT show;
    convert( info, show );
    M_read_time = static_cast< int >( show.time_ );

    return ( handlePlayMode( info.pmode )
             && handleTeamInfo( info.team[0], info.team[1] )
             && handleShow( show ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleShortShowInfo2( const short_showinfo_t2 & info )
{
    ShowInfoT show;
    convert( info, show );

    M_read_time = static_cast< int >( show.time_ );

    return handleShow( show );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleMsgInfo( const Int16 board,
                        const std::string & msg )
{
    return handleMsg( M_read_time, board, msg );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleDrawInfo( const drawinfo_t & draw )
{
    return handleDraw( M_read_time, draw );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handlePlayMode( char playmode )
{
    return handlePlayMode( M_read_time, static_cast< PlayMode >( playmode ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleTeamInfo( const team_t & team_left,
                         const team_t & team_right )
{
    TeamT l, r;
    convert( team_left, l );
    convert( team_right, r );

    return handleTeam( M_read_time, l, r );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handlePlayerType( const player_type_t & type )
{
    return handlePlayerType( to_string( type ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handleServerParam( const server_params_t & param )
{
    return handleServerParam( to_string( param ) );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
Handler::handlePlayerParam( const player_params_t & param )
{
    return handlePlayerParam( to_string( param ) );
}

/*-------------------------------------------------------------------*/
bool
Handler::handlePlayMode( const int time,
                         const std::string & playmode )
{
    static const std::unordered_map< std::string, PlayMode > s_map = {
        { "before_kick_off", PM_BeforeKickOff },
        { "time_over", PM_TimeOver },
        { "play_on", PM_PlayOn },
        { "kick_off_l", PM_KickOff_Left },
        { "kick_off_r", PM_KickOff_Right },
        { "kick_in_l", PM_KickIn_Left },
        { "kick_in_r", PM_KickIn_Right },
        { "free_kick_l", PM_FreeKick_Left },
        { "free_kick_r", PM_FreeKick_Right },
        { "corner_kick_l", PM_CornerKick_Left },
        { "corner_kick_r", PM_CornerKick_Right },
        { "goal_kick_l", PM_GoalKick_Left },
        { "goal_kick_r", PM_GoalKick_Right },
        { "goal_l", PM_AfterGoal_Left },
        { "goal_r", PM_AfterGoal_Right },
        { "drop_ball", PM_Drop_Ball },
        { "offside_l", PM_OffSide_Left },
        { "offside_r", PM_OffSide_Right },
        { "penalty_kick_l", PM_PK_Left },
        { "penalty_kick_r", PM_PK_Right },
        { "first_half_over", PM_FirstHalfOver },
        { "pause", PM_Pause },
        { "human_judge", PM_Human },
        { "foul_charge_l", PM_Foul_Charge_Left },
        { "foul_charge_r", PM_Foul_Charge_Right },
        { "foul_push_l", PM_Foul_Push_Left },
        { "foul_push_l", PM_Foul_Push_Right },
        { "foul_multiple_attack_l", PM_Foul_MultipleAttacker_Left },
        { "foul_multiple_attack_r", PM_Foul_MultipleAttacker_Right },
        { "foul_ballout_l", PM_Foul_BallOut_Left },
        { "foul_ballout_r", PM_Foul_BallOut_Right },
        { "back_pass_l", PM_Back_Pass_Left },
        { "back_pass_r", PM_Back_Pass_Right },
        { "free_kick_fault_l", PM_Free_Kick_Fault_Left },
        { "free_kick_fault_r", PM_Free_Kick_Fault_Right },
        { "catch_fault_l", PM_CatchFault_Left },
        { "catch_fault_r", PM_CatchFault_Right },
        { "indirect_free_kick_l", PM_IndFreeKick_Left },
        { "indirect_free_kick_r", PM_IndFreeKick_Right },
        { "penalty_setup_l", PM_PenaltySetup_Left },
        { "penalty_setup_r", PM_PenaltySetup_Right },
        { "penalty_ready_l", PM_PenaltyReady_Left },
        { "penalty_ready_r", PM_PenaltyReady_Right },
        { "penalty_taken_l", PM_PenaltyTaken_Left },
        { "penalty_taken_r", PM_PenaltyTaken_Right },
        { "penalty_miss_l", PM_PenaltyMiss_Left },
        { "penalty_miss_r", PM_PenaltyMiss_Right },
        { "penalty_score_l", PM_PenaltyScore_Left },
        { "penalty_score_r", PM_PenaltyScore_Right },
        { "illegal_defense_l", PM_Illegal_Defense_Left },
        { "illegal_defense_r", PM_Illegal_Defense_Right },
    };

    std::unordered_map< std::string, PlayMode >::const_iterator it = s_map.find( playmode );
    if ( it == s_map.end() )
    {
        return false;
    }

    return handlePlayMode( time, it->second );
}

} // end namespace
} // end namespace
