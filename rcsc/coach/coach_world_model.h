// -*-c++-*-

/*!
  \file coach_world_model.h
  \brief coach/trainer world model class Header File
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

#ifndef RCSC_COACH_COACH_WORLD_MODEL_H
#define RCSC_COACH_COACH_WORLD_MODEL_H

#include <rcsc/coach/coach_world_state.h>
#include <rcsc/coach/coach_ball_object.h>
#include <rcsc/coach/coach_player_object.h>
#include <rcsc/coach/player_type_analyzer.h>
#include <rcsc/clang/types.h>
#include <rcsc/game_mode.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <iostream>
#include <string>
#include <vector>
#include <list>

namespace rcsc {

class AudioMemory;
class CoachVisualSensor;

/*!
  \class CoachWorldModel
  \brief world world for coach
 */
class CoachWorldModel {
private:

    //! client protocol version
    int M_client_version;

    //
    // game info
    //

    //! last updated time
    GameTime M_time;
    //! last see_global(look) received time
    GameTime M_see_time;

    //! team side id. if agent is a trainer, the team side becomes NEUTRAL.
    SideID M_our_side;

    std::string M_our_team_name; //!< our team name
    std::string M_their_team_name; //!< opponent teamname string

    GameTime M_last_set_play_start_time; //!< SetPlay started time
    int M_setplay_count; //!< setplay counter

    //! last updated playmode
    GameMode M_game_mode;

    GameTime M_training_time; //!< training start/end time for keepaway

    //! heard info memory
    std::shared_ptr< AudioMemory > M_audio_memory;

    //
    // objects
    //

    CoachWorldState::Ptr M_current_state; //!< current world state. always exist instance.
    CoachWorldState::Ptr M_previous_state; //!< previous world state.

    CoachWorldState::List M_state_list; //!< the record of world state.
    CoachWorldState::Map M_state_map; //!< the map of world state;

    SideID M_last_kicker_side; //!< last ball kicker's team side
    int M_last_kicker_unum; //!< last ball kicker's uniform number

    GameTime M_pass_time; //!< heard pass time
    int M_passer_unum; //!< last heard passer's uniform nuber
    int M_receiver_unum; //!< last heard receiver's uniform nuber
    Vector2D M_pass_start_pos; //!< last heard pass first position
    Vector2D M_pass_receive_pos; //!< last heard pass receive position

    //
    // player type management
    //

    //! player type analyzer instance
    PlayerTypeAnalyzer M_player_type_analyzer;

    //! available player type set for player substitution by online coach
    std::vector< int > M_available_player_type_id;

    int M_our_substitute_count; //!< the number of our player substitution
    int M_their_substitute_count; //!< the number of their player substitution

    int M_our_player_type_id[11]; //!< left team player type id
    int M_their_player_type_id[11]; //!< right team player type id

    //! counter of each used player type for our team
    std::vector< int > M_our_player_type_used_count;
    //! counter of each used player type for their team
    std::vector< int > M_their_player_type_used_count;

    //
    // card information
    //

    Card M_our_card[11]; //!< our team players' card status
    Card M_their_card[11]; //!< their team players' card status

    //
    // coach language management
    //

    long M_last_playon_start; //!< last play_on start cycle

    long M_clang_capacity_update_time; //!< the last clang update time
    int M_clang_capacity[CLANG_MAX_TYPE]; // the number of available message for each message type

    int M_freeform_allowed_count; //!< the number of allowd freeform message
    int M_freeform_send_count; //!< the number of send out freeform message

    // not used
    CoachWorldModel( const CoachWorldModel & ) = delete;
    CoachWorldModel & operator=( const CoachWorldModel & ) = delete;

public:
    /*!
      \brief init member variables.
     */
    CoachWorldModel();

    /*!
      \brief init by team name, team side, and client version
      \param team_name our team name
      \param side side character
      \param client_version client protocol version
     */
    void init( const std::string & team_name,
               const SideID side,
               const int client_version );

    /*!
      \brief set new audio memory
      \param memory pointer to the memory instance. This must be
      a dynamically allocated object.
     */
    void setAudioMemory( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get audio memory
      \return co
     */
    const AudioMemory & audioMemory() const
      {
          return *M_audio_memory;
      }

    /*!
      \brief init variables using received server_param information.
      This method have to be called just after server_param message.
     */
    void setServerParam();

    /*!
      \brief init variables using received player_param information.
      This method have to be called just after player_param message.
     */
    void setPlayerParam();


    /*!
      \brief set team name
      \param side team side
      \param name team name string
     */
    void setTeamName( const SideID side,
                      const std::string & name );

    /*!
      \brief this method have to be called just after change_player_type message is received.
      \param side player's side
      \param unum player's uniform number
      \param type new player type id
     */
    void changePlayerType( const SideID side,
                           const int unum,
                           const int type );

    /*!
      \brief set player's card status
      \param side punished player's side
      \param unum punished player's unum
      \param card card type
     */
    void setCard( const SideID side,
                  const int unum,
                  const Card card );

    /*!
      \brief update playmode using heard referee info
      \param game_mode analyzed referee info
      \param current current game time
     */
    void updateGameMode( const GameMode & game_mode,
                         const GameTime & current );

    /*!
      \brief update status using analyzed visual info
      \param see_global analyzed visual info
      \param current current game time
     */
    void updateAfterSeeGlobal( const CoachVisualSensor & see_global,
                               const GameTime & current );

    /*!
      \brief update status using audio info
      \param current current game time
     */
    void updateJustBeforeDecision( const GameTime & current );


    /*!
      \brief update all information by using display data
      \param disp display data
     */
    void updateAll( const rcg::DispInfoT & disp );

private:

    /*!
      \brief update clang capacity.
     */
    void updateCLangCapacity();

    /*!
      \brief update team names using see information
      \param see_global analyzed visual information
     */
    void updateTeamNames( const CoachVisualSensor & see_global );

    /*!
      \brief analyze and update heterogeneous player type using visual information
     */
    void updatePlayerType();

    /*!
      \brief estimate last ball kicker.
     */
    void updateLastKicker();

    /*!
      \brief set last passer by heard pass information
     */
    void updateLastPasser();

    //
    //
    //

    /*!
      \brief update game mode by using display data
      \param disp display data
     */
    void updateGameMode( const rcg::DispInfoT & disp );

    /*!
      \brief update positional information by using display data
      \param disp display data
     */
    void updateState( const rcg::DispInfoT & disp,
                      const GameTime & current );

    /*!
      \brief update team names using display data
      \param disp display data
     */
    void updateTeamNames( const rcg::DispInfoT & disp );

    /*!
      \brief update heterogeneous player types
      \param disp display data
     */
    void updatePlayerType( const rcg::DispInfoT & disp );

    /*!
      \brief update card status
      \param disp display data
     */
    void updateCard( const rcg::DispInfoT & disp );

public:

    /*!
      \brief set training start/end time
      \param t game time
     */
    void setTrainingTime( const GameTime & t )
      {
          M_training_time = t;
      }

    /*!
      \brief update clang capacity.
      \param type clang message type
     */
    void decCLangCapacity( const CLangType type );

    /*!
      \brief increment freeform send count
     */
    void incFreeformSendCount();

    /*!
      \brief get a client protocol version.
      \return client protocol version number.
     */
    int clientVersion() const
      {
          return M_client_version;
      }

    /*!
      \brief get our team side
      \return side Id
     */
    SideID ourSide() const
      {
          return M_our_side;
      }

    /*!
      \brief get opponent team side
      \return side Id
     */
    SideID theirSide() const
      {
          return ( M_our_side == RIGHT ? LEFT : RIGHT );
      }

    /*!
      \brief get our team name
      \return team name string
     */
    const std::string & ourTeamName() const
      {
          return M_our_team_name;
      }

    /*!
      \brief get opponent team name
      \return team name string
     */
    const std::string & theirTeamName() const
      {
          return M_their_team_name;
      }

    /*!
      \brief get left team name
      \return team name string
     */
    const std::string & teamNameLeft() const
      {
          return ( M_our_side == RIGHT
                   ? M_their_team_name
                   : M_our_team_name );
      }

    /*!
      \brief get right team name
      \return team name string
     */
    const std::string & teamNameRight() const
      {
          return ( M_our_side == RIGHT
                   ? M_our_team_name
                   : M_their_team_name );
      }

    /*!
      \brief get last updated time
      \return const reference to the game time object
     */
    const GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get last see global time
      \return const reference to the game time object
     */
    const GameTime & seeTime() const
      {
          return M_see_time;
      }

    /*!
      \brief get last setplay type playmode start time
      \return const reference to the game time object
    */
    const GameTime & lastSetPlayStartTime() const { return M_last_set_play_start_time; }

    /*!
      \brief get cycle count that setplay type playmode is keeped
      \return counted long integer
    */
    int getSetPlayCount() const { return M_setplay_count; }

    /*!
      \brief get latest playmode info
      \return const reference to the GameMode object
     */
    const GameMode & gameMode() const
      {
          return M_game_mode;
      }

    /*!
      \brief get training start/end time
      \return game time object
     */
    const GameTime & trainingTime() const
      {
          return M_training_time;
      }

    /*!
      \brief get the current state.
      \return const reference to the current state.
     */
    const CoachWorldState & currentState() const
      {
          return *M_current_state;
      }

    /*!
      \brief get the current state pointer.
      \return const reference to the current state.
     */
    CoachWorldState::ConstPtr currentStatePtr() const
      {
          return M_current_state;
      }

    /*!
      \brief get the state list.
      \return the world state list.
     */
    const CoachWorldState::List & stateList() const
      {
          return M_state_list;
      }

    /*!
      \brief get the state map. key: GameTime, value: CoachWorldState::ConstPtr
      \return the world state list.
     */
    const CoachWorldState::Map & stateMap() const
      {
          return M_state_map;
      }

    /*!
      \brief get the state pointer at the specified game time
      \param time nomal game time. the stoppage time is assued as 0.
      \return const pointer. if not found, NULL is returned.
     */
    CoachWorldState::ConstPtr getState( const int time ) const
      {
          if ( time < 0 )
          {
              return M_current_state;
          }

          CoachWorldState::Map::const_iterator it = M_state_map.find( GameTime( time, 0 ) );
          if ( it == M_state_map.end() )
          {
              return CoachWorldState::ConstPtr();
          }
          return it->second;
      }

    /*!
      \brief get the state pointer at the specified game time
      \param time game time
      \return const pointer. if not found, NULL is returned.
     */
    CoachWorldState::ConstPtr getState( const GameTime & time ) const
      {
          if ( time.cycle() < 0 )
          {
              return M_current_state;
          }

          CoachWorldState::Map::const_iterator it = M_state_map.find( time );
          if ( it == M_state_map.end() )
          {
              return CoachWorldState::ConstPtr();
          }
          return it->second;
      }

    /*!
      \brief get the current ball data
      \return const reference to the ball data
     */
    const CoachBallObject & ball() const
      {
          return M_current_state->ball();
      }

    /*!
      \brief get the current ball position status
      \return ball status type
     */
    BallStatus getBallStatus() const
      {
          return M_current_state->getBallStatus();
      }

    /*!
      \brief get all players
      \return const reference to the data container
     */
    const CoachPlayerObject::Cont & allPlayers() const
      {
          return M_current_state->allPlayers();
      }

    /*!
      \brief get left players' pointer
      \return const reference to the data container
     */
    const CoachPlayerObject::Cont & playersLeft() const
      {
          return ( M_our_side == RIGHT
                   ? M_current_state->opponents()
                   : M_current_state->teammates() );
      }

    /*!
      \brief get right players' pointer
      \return const reference to the data container
     */
    const CoachPlayerObject::Cont & playersRight() const
      {
          return ( M_our_side == RIGHT
                   ? M_current_state->teammates()
                   : M_current_state->opponents() );
      }

    /*!
      \brief get teammate players' pointer
      \return const reference to the data container
     */
    const CoachPlayerObject::Cont & teammates() const
      {
          return M_current_state->teammates();
      }

    /*!
      \brief get opponent players' pointer
      \return const reference to the data container
     */
    const CoachPlayerObject::Cont & opponents() const
      {
          return M_current_state->opponents();
      }

    /*!
      \brief get the specified teammate.
      \param unum input uniform number.
      \return pointer to the object. if not found, NULL is returned.
     */
    const CoachPlayerObject * teammate( const int unum ) const
      {
          return M_current_state->teammate( unum );
      }

    /*!
      \brief get the specified teammate.
      \param unum input uniform number.
      \return pointer to the object. if not found, NULL is returned.
     */
    const CoachPlayerObject * opponent( const int unum ) const
      {
          return M_current_state->opponent( unum );
      }

    /*!
      \brief get the estimated last ball kicker's team side
      \return side id
     */
    SideID lastKickerSide() const
      {
          return M_last_kicker_side;
      }

    /*!
      \brief get the estimated last ball kicker's uniform number
      \return uniform number
     */
    int lastKickerUnum() const
      {
          return M_last_kicker_unum;
      }

    /*!
      \brief get the last heard pass time
      \return game time
     */
    const GameTime & passTime() const
      {
          return M_pass_time;
      }

    /*!
      \brief get the last heard passer's uniform nuber
      \return uniform number
    */
    int passerUnum() const
      {
          return M_passer_unum;
      }

    /*!
      \brief get the last heard receiver's uniform number
      \return uniform number
     */
    int receiverUnum() const
      {
          return M_receiver_unum;
      }

    /*!
      \brief get the last heard pass start position
      \return pass first position
     */
    const Vector2D & passStartPos() const
      {
          return M_pass_start_pos;
      }

    /*!
      \brief get the last heard pass receive position
      \return pass receive position
     */
    const Vector2D & passReceivePos() const
      {
          return M_pass_receive_pos;
      }

    //
    // player type information
    //

    /*!
      \brief get the current available player type id set for player substitution.
      \return array of player type id. id may be duplicated.
     */
    const std::vector< int > & availablePlayerTypeId() const
      {
          return M_available_player_type_id;
      }

    /*!
      \brief check if online coach can substitute the specified teammate
      \param unum target player's uniform number
      \param type new player type
      \return checked result
     */
    bool canSubstitute( const int unum,
                        const int type ) const;

    /*!
      \brief get the number of change_player_type message after kickoff
      \return the number of change_player_type message after kickoff
     */
    int substituteCount( const SideID side ) const
      {
          return ( M_our_side == side
                   || ( M_our_side == NEUTRAL
                        && side == LEFT )
                   ? M_our_substitute_count
                   : M_their_substitute_count );
      }

    /*!
      \brief get the number of our change_player_type message after kickoff
      \return the number of our change_player_type message after kickoff
     */
    int ourSubstituteCount() const
      {
          return M_our_substitute_count;
      }

    /*!
      \brief get the number of their change_player_type message after kickoff
      \return the number of their change_player_type message after kickoff
     */
    int theirSubstituteCount() const
      {
          return M_their_substitute_count;
      }


    /*!
      \brief get the player type Id
      \param side target player's side
      \param unum target player's uniform number
      \return player type id
     */
    int playerTypeId( const SideID side,
                      const int unum ) const;

    /*!
      \brief get teammate's player type Id
      \param unum target player's uniform number
      \return player type id
     */
    int ourPlayerTypeId( const int unum ) const
      {
          return playerTypeId( ourSide(), unum );
      }

    /*!
      \brief get opponent's player type Id
      \param unum target player's uniform number
      \return player type id
     */
    int theirPlayerTypeId( const int unum ) const
      {
          return playerTypeId( theirSide(), unum );
      }

    /*!
      \brief get container of each player type's used count
      \param side team side
      \return container of each player type's used count
     */
    const std::vector< int > & playerTypeUsedCount( const SideID side ) const
      {
          return ( ourSide() == side
                   || ( ourSide() == NEUTRAL
                        && side == LEFT )
                   ? M_our_player_type_used_count
                   : M_their_player_type_used_count );
      }

    /*!
      \brief get our team's container of each player type's used count
      \return container of each player type's used count
     */
    const std::vector< int > & ourPlayerTypeUsedCount() const
      {
          return M_our_player_type_used_count;
      }

    /*!
      \brief get their team's container of each player type's used count
      \return container of each player type's used count
     */
    const std::vector< int > & theirPlayerTypeUsedCount() const
      {
          return M_their_player_type_used_count;
      }


    //
    // card information
    //

    /*!
      \brief get yellow card status
      \param side team side
      \param unum uniform number
      \return yellow card status
     */
    bool isYellowCarded( const SideID side,
                         const int unum ) const;


    /*!
      \brief get red card status
      \param side team side
      \param unum uniform number
      \return red card status
     */
    bool isRedCarded( const SideID side,
                      const int unum ) const;

    /*!
      \brief get teammate's yellow card status
      \param unum uniform number
      \return yellow card status
     */
    bool isTeammateYellowCarded( const int unum ) const
      {
          return isYellowCarded( ourSide(), unum );
      }

    /*!
      \brief get teammate's red card status
      \param unum uniform number
      \return red card status
     */
    bool isTeammateRedCarded( const int unum ) const
      {
          return isRedCarded( ourSide(), unum );
      }

    /*!
      \brief get opponent's yellow card status
      \param unum uniform number
      \return yellow card status
     */
    bool isOpponentYellowarded( const int unum ) const
      {
          return isYellowCarded( theirSide(), unum );
      }


    /*!
      \brief get opponent's red card status
      \param unum uniform number
      \return red card status
     */
    bool isOpponentRedCarded( const int unum ) const
      {
          return isRedCarded( theirSide(), unum );
      }


    //
    // coach language info
    //

    /*!
      \brief get the playon period
      \return cycle value
     */
    long lastPlayOnStart() const
      {
          return M_last_playon_start;
      }


    /*!
      \brief check if the clang message can be sent now.
      \param type clang message type.
      \return checked result.
     */
    bool canSendCLang( const CLangType type ) const;


    /*!
      \brief get the allowed freeform message count
      \return the number of the allowd message count
     */
    int freeformAllowedCount() const
      {
          return M_freeform_allowed_count;
      }

    /*!
      \brief get the freeform message send count
      \return the number of the message send count
     */
    int freeformSendCount() const
      {
          return M_freeform_send_count;
      }

    /*!
      \brief check if coach can sent the freeform message now.
      \return the check result
     */
    bool canSendFreeform() const;

    //
    // analyzed result
    //

    /*!
      \brief get the offside line for left team
      \brief the offside line x-coordinate value
     */
    double offsideLineXForLeft() const
      {
          return ( M_our_side == RIGHT
                   ? M_current_state->theirOffsideLineX()
                   : M_current_state->ourOffsideLineX() );
      }

    /*!
      \brief get the offside line for right team
      \brief the offside line x-coordinate value
     */
    double offsideLineXForRight() const
      {
          return ( M_our_side == RIGHT
                   ? M_current_state->ourOffsideLineX()
                   : M_current_state->theirOffsideLineX() );
      }

    /*!
      \brief get the offside line for our offense situation
      \brief the offside line x-coordinate value
     */
    double ourOffsideLineX() const
      {
          return M_current_state->ourOffsideLineX();
      }

    /*!
      \brief get the offside line for their offense situation
      \brief the offside line x-coordinate value
     */
    double theirOffsideLineX() const
      {
          return M_current_state->theirOffsideLineX();
      }


    /*!
      \brief check if kickable player exists or not
      \return true if kickable plaeyr exists
     */
    bool existKickablePlayer() const;

    /*!
      \brief get player pointer nearest to the specified point
      \param point target point
      \return const pointer to the player object
     */
    const CoachPlayerObject * getPlayerNearestTo( const Vector2D & point ) const;

    /*!
      \brief put all data to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;

};

}

#endif
