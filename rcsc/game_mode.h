// -*-c++-*-

/*!
  \file game_mode.h
  \brief playmode wrapper Header File
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

#ifndef RCSC_GAME_MODE_H
#define RCSC_GAME_MODE_H

#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <string>
#include <utility>

namespace rcsc {

/*!
  \class GameMode
  \brief playmode(referee info) wrapper class

  This class can be updated by server playmode string using update() method.
  This class can also handle current game score.
*/
class GameMode {
public:

    /*!
      \brief playmode type Id
    */
    enum Type {
        BeforeKickOff,
        TimeOver,
        PlayOn,
        KickOff_,    // Left | Right
        KickIn_,     // Left | Right
        FreeKick_,   // Left | Right
        CornerKick_, // Left | Right
        GoalKick_,   // Left | Right
        AfterGoal_,  // Left | Right
        //Drop_Ball,   // Left | Right
        OffSide_,    // Left | Right
        PenaltyKick_,         // Left | Right
        FirstHalfOver,
        Pause,
        Human,
        FoulCharge_, // Left | Right
        FoulPush_,   // Left | Right
        FoulMultipleAttacker_, // Left | Right
        FoulBallOut_,    // Left | Right
        BackPass_,       // Left | Right
        FreeKickFault_, // Left | Right

        CatchFault_, // Left | Right
        IndFreeKick_, // Left | Right

        PenaltySetup_, // Left | Right
        PenaltyReady_, // Left | Right
        PenaltyTaken_, // Left | Right
        PenaltyMiss_, // Left | Right
        PenaltyScore_, // Left | Right

        IllegalDefense_, // Left | Right

        // not a real playmode
        PenaltyOnfield_, // next real playmode is PenaltySetup_
        PenaltyFoul_,    // next real playmode is PenaltyMiss_ or PenaltyScore_
        //PenaltyWinner_,  // next real playmode is TimeOver
        //PenaltyDraw,     // next real playmode is TimeOver

        GoalieCatch_, // Left | Right
        ExtendHalf,
        MODE_MAX
    };


    /*
    // normal playmode string

    "before_kick_off"
    "time_over"
    "play_on"
    "kick_off_l"
    "kick_off_r"
    "kick_in_l"
    "kick_in_r"
    "free_kick_l"
    "free_kick_r"
    "corner_kick_l"
    "corner_kick_r"
    "goal_kick_l"
    "goal_kick_r"
    "goal_l"
    "goal_r"
    "drop_ball"
    "offside_l"
    "offside_r"
    "penalty_kick_l"
    "penalty_kick_r"
    "first_half_over"
    "pause"
    "human_judge"
    "foul_charge_l"
    "foul_charge_r"
    "foul_push_l"
    "foul_push_r"
    "foul_multiple_attack_l"
    "foul_multiple_attack_r"
    "foul_ballout_l"
    "foul_ballout_r"
    "back_pass_l"
    "back_pass_r"
    "free_kick_fault_l"
    "free_kick_fault_r"
    "catch_fault_l",
    "catch_fault_r",
    "indirect_free_kick_l",
    "indirect_free_kick_r",
    "penalty_setup_l",
    "penalty_setup_r",
    "penalty_ready_l",
    "penalty_ready_r",
    "penalty_taken_l",
    "penalty_taken_r",
    "penalty_miss_l",
    "penalty_miss_r",
    "penalty_score_l",
    "penalty_score_r"

    // extend playmode

    "goal_l_<SCORE>"
    "goal_r_<SCORE>"
    "half_time"
    "time_extended"
    "time_up_without_a_team"
    "time_up"
    "foul_l" // set by monitor only
    "foul_r" // set by monitor only
    "goalie_catch_ball_l"
    "goalie_catch_ball_r"

    "penalty_onfield_{l|r}"
    "penalty_foul_{l|r}"
    "penalty_winner_{l|r}"
    "penalty_draw"

    // other notification

    "yellow_card_SIDE_UNUM"
    "red_card_SIDE_UNUM"
    */

    typedef std::pair< Type, SideID > Pair; //!< alias of the pair of playmode type and side type

private:

    //! last update time
    GameTime M_time;

    //! playmode type Id
    Type M_type;
    //! side info in playmode
    SideID M_side;

    //! left team's score
    int M_score_left;
    //! rigth team's score
    int M_score_right;
public:
    /*!
      \brief init member variables
    */
    GameMode();

    /*!
      \brief init member variables with arguments
    */
    GameMode( Type type,
              SideID side,
              const GameTime & time,
              int score_left,
              int score_right );

    /*!
      \brief analyze playmode string and update internal status
      \param mode_str playmode string sent from server
      \param current current game time
      \retval true successfully updated.
      \retval false detected illegal playmode string
    */
    bool update( const std::string & mode_str,
                 const GameTime & current );

    /*!
      \brief set scores directly.
      \param score_l left team score
      \param score_r right team score
     */
    void setScore( const int score_l,
                   const int score_r );

private:
    /*!
      \brief analyze playmode string. if mode is goal_?_?, score is updated
      \param mode_str playmode string
      \return the pair of playmode type and playmode side
    */
    Pair parse( const std::string & mode_str );

public:

    /*!
      \brief get last update time
      \return const reference to the GameTime object
     */
    const GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get current playmode type
      \return client side playmode type Id
    */
    Type type() const
      {
          return M_type;
      }

    /*!
      \brief get current  playmode side
      \return playmode side Id
    */
    SideID side() const
      {
          return M_side;
      }

    /*!
      \brief get current left score
      \return left score value
    */
    int scoreLeft() const
      {
          return M_score_left;
      }

    /*!
      \brief get current right score
      \return right score value
    */
    int scoreRight() const
      {
          return M_score_right;
      }

    /*!
      \brief check if current playmode is one of server cycle stopped type.
      \return true if cycle is stopped, false other wise
    */
    bool isServerCycleStoppedMode() const;

    /*!
      \brief check if current playmode is game end type.
      \return true if game is end
    */
    bool isGameEndMode() const;

    /*!
      \brief check if current playmode is one of penalty kick mode.
      \return true if game is penalty kick mode
    */
    bool isPenaltyKickMode() const;

    /*!
      \brief check if current mode is team's setplay
      \param team_side judged team side id
      \return true if current playmode is team's setplay mode
    */
    bool isTeamsSetPlay( const SideID team_side ) const;

    /*!
      \brief check if current mode is our setplay
      \param our_side clinet's team side
      \return true if current playmode is our setplay mode
    */
    bool isOurSetPlay( const SideID our_side ) const
      {
          return isTeamsSetPlay( our_side );
      }

    /*!
      \brief check if current mode is their setplay
      \param our_side clinet's team side
      \return true if current playmode is their setplay mode
    */
    bool isTheirSetPlay( const SideID our_side ) const
      {
          return isTeamsSetPlay( our_side == LEFT
                                 ? RIGHT
                                 : our_side == RIGHT
                                 ? LEFT
                                 : NEUTRAL );
      }

    /*!
      \brief get rcssserver playmode
      \return rcssserver playmode Id
    */
    PlayMode getServerPlayMode() const;

    /*!
      \brief get playmode string
      \return const char pointer
     */
    const char * toCString() const;

    /*!
      \brief print current playmode string to stream
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & print( std::ostream & os ) const;
};

}

#endif
