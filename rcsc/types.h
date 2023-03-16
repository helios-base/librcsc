// -*-c++-*-

/*!
  \file rcsc/types.h
  \brief the type definition set for the RCSSServer2D
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

#ifndef RCSC_TYPES_H
#define RCSC_TYPES_H

namespace rcsc {

//! max player number in one team
constexpr int MAX_PLAYER = 11;

//! uniform number that represents the unknown player
constexpr int Unum_Unknown = -1;

/*!
  \enum SideID
  \brief side type definition
*/
enum SideID {
    LEFT = 1,
    NEUTRAL = 0,
    RIGHT = -1,
};

/*!
  \breif get side information character
  \return 'l', 'r' or 'n'
 */
inline
char
side_char( SideID s )
{
    return ( s == LEFT ? 'l' : s == RIGHT ? 'r' : 'n' );
}

/*!
  \breif get side information string
  \return "left", "right" or "neutral"
 */
inline
const char *
side_str( SideID s )
{
    return ( s == LEFT ? "left" : s == RIGHT ? "right" : "neutral" );
}

/*!
  \enum MarkerID
  \brief marker type defintiion
 */
enum MarkerID {
    Goal_L, Goal_R, // 1

    Flag_C,
    Flag_CT, Flag_CB,
    Flag_LT, Flag_LB,
    Flag_RT,  Flag_RB, // 8

    Flag_PLT, Flag_PLC, Flag_PLB,
    Flag_PRT, Flag_PRC, Flag_PRB, // 14

    Flag_GLT, Flag_GLB,
    Flag_GRT, Flag_GRB, // 18

    Flag_TL50, Flag_TL40, Flag_TL30, Flag_TL20, Flag_TL10, // 23
    Flag_T0,
    Flag_TR10, Flag_TR20, Flag_TR30, Flag_TR40, Flag_TR50, // 29

    Flag_BL50, Flag_BL40, Flag_BL30, Flag_BL20, Flag_BL10,
    Flag_B0,
    Flag_BR10, Flag_BR20, Flag_BR30, Flag_BR40, Flag_BR50, // 40

    Flag_LT30, Flag_LT20, Flag_LT10 , // 43
    Flag_L0,
    Flag_LB10, Flag_LB20, Flag_LB30, // 47

    Flag_RT30, Flag_RT20, Flag_RT10, // 50
    Flag_R0,
    Flag_RB10, Flag_RB20, Flag_RB30, // 54

    Marker_Unknown = 55
};

/*!
  \enum LineID
  \brief line type definition
 */
enum LineID {
    Line_Left, Line_Right, Line_Top, Line_Bottom,
    Line_Unknown
};

//! Id of the unknown player type
constexpr int Hetero_Unknown = -1;
//! Id of the default player type
constexpr int Hetero_Default = 0;

/*!
  \enum PlayMode
  \brief playmode types defined in rcssserver/src/types.h
 */
enum PlayMode {
    PM_Null,
    PM_BeforeKickOff,
    PM_TimeOver,
    PM_PlayOn,
    PM_KickOff_Left,
    PM_KickOff_Right,
    PM_KickIn_Left,
    PM_KickIn_Right,
    PM_FreeKick_Left,
    PM_FreeKick_Right,
    PM_CornerKick_Left,
    PM_CornerKick_Right,
    PM_GoalKick_Left,
    PM_GoalKick_Right,
    PM_AfterGoal_Left,
    PM_AfterGoal_Right, // - sserver-2.94
    PM_Drop_Ball, // - sserver-3.29
    PM_OffSide_Left,
    PM_OffSide_Right, // untill sserver-5.27
    // [I.Noda:00/05/13] added for 3D viewer/commentator/small league
    PM_PK_Left,
    PM_PK_Right,
    PM_FirstHalfOver,
    PM_Pause,
    PM_Human,
    PM_Foul_Charge_Left,
    PM_Foul_Charge_Right,
    PM_Foul_Push_Left,
    PM_Foul_Push_Right,
    PM_Foul_MultipleAttacker_Left,
    PM_Foul_MultipleAttacker_Right,
    PM_Foul_BallOut_Left,
    PM_Foul_BallOut_Right, // until sserver-7.11
    PM_Back_Pass_Left, // after rcssserver-8.05-rel
    PM_Back_Pass_Right,
    PM_Free_Kick_Fault_Left,
    PM_Free_Kick_Fault_Right,
    PM_CatchFault_Left,
    PM_CatchFault_Right,
    PM_IndFreeKick_Left, // after rcssserver-9.2.0
    PM_IndFreeKick_Right,
    PM_PenaltySetup_Left,  // after rcssserver-9.3.0
    PM_PenaltySetup_Right,
    PM_PenaltyReady_Left,
    PM_PenaltyReady_Right,
    PM_PenaltyTaken_Left,
    PM_PenaltyTaken_Right,
    PM_PenaltyMiss_Left,
    PM_PenaltyMiss_Right,
    PM_PenaltyScore_Left,
    PM_PenaltyScore_Right,
    PM_Illegal_Defense_Left, // after rcssserver-16.0.0
    PM_Illegal_Defense_Right,
    PM_MAX
};

//! playmode string table defined in rcssserver.
#define PLAYMODE_STRINGS {"",\
			"before_kick_off",\
			"time_over",\
			"play_on",\
			"kick_off_l",\
			"kick_off_r",\
			"kick_in_l",\
			"kick_in_r",\
			"free_kick_l",\
			"free_kick_r",\
			"corner_kick_l",\
			"corner_kick_r",\
			"goal_kick_l",\
			"goal_kick_r",\
			"goal_l",\
			"goal_r",\
			"drop_ball",\
			"offside_l",\
			"offside_r",\
			"penalty_kick_l",\
			"penalty_kick_r",\
			"first_half_over",\
			"pause",\
			"human_judge",\
			"foul_charge_l",\
			"foul_charge_r",\
			"foul_push_l",\
			"foul_push_r",\
			"foul_multiple_attack_l",\
			"foul_multiple_attack_r",\
			"foul_ballout_l",\
			"foul_ballout_r",\
      "back_pass_l", \
      "back_pass_r", \
      "free_kick_fault_l", \
      "free_kick_fault_r", \
      "catch_fault_l", \
      "catch_fault_r", \
      "indirect_free_kick_l", \
      "indirect_free_kick_r",\
      "penalty_setup_l", \
      "penalty_setup_r",\
      "penalty_ready_l",\
      "penalty_ready_r", \
      "penalty_taken_l", \
      "penalty_taken_r", \
      "penalty_miss_l", \
      "penalty_miss_r", \
      "penalty_score_l", \
      "penalty_score_r", \
      "illegal_defense_l", \
      "illegal_defense_r", \
      "", \
      "" \
}

// available characters in player's say or coach's freeform message
// [-0-9a-zA-Z ().+*/?<>_]
// 74 characters

//! character set that player can say.
#define SAY_CHARACTERS \
"0123456789"\
"abcdefghijklmnopqrstuvwxyz"\
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"\
" ().+*/?<>_-"


/*!
  \enum BallStatus
  \brief ball position status for coach/trainer
 */
enum BallStatus {
    Ball_Null,
    Ball_InField,
    Ball_GoalL,
    Ball_GoalR,
    Ball_OutOfField,
    Ball_MAX
} ;

//! ball status string table for trainer.
#define BALL_STATUS_STRINGS { "",\
    "in_field",\
    "goal_l",\
    "goal_r",\
    "out_of_field",\
}

/*!
  \enum Card
  \brief card type
 */
enum Card {
    YELLOW,
    RED,
    NO_CARD,
};

} // end of namespace

#endif
