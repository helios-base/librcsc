// -*-c++-*-

/*!
  \file world_model.h
  \brief world model Header File
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

#ifndef RCSC_PLAYER_WORLD_MODEL_H
#define RCSC_PLAYER_WORLD_MODEL_H

#include <rcsc/player/self_object.h>
#include <rcsc/player/ball_object.h>
#include <rcsc/player/player_object.h>
#include <rcsc/player/view_area.h>
#include <rcsc/player/view_grid_map.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/time/timer.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_mode.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <memory>
#include <string>

namespace rcsc {

class AudioMemory;
class ActionEffector;
class BodySensor;
class FullstateSensor;
class Localization;
class PenaltyKickState;
class PlayerPredicate;
class PlayerType;
class VisualSensor;

/*!
  \class WorldModel
  \brief player's internal field status
*/
class WorldModel {
public:

    enum {
        DIR_CONF_DIVS = 72
    };

    static const double DIST_TOO_FAR; //!< long distance

    static const std::size_t MAX_RECORD; //!< max record size
    static const double DIR_STEP; //!< the angle steps for dir confidence

private:

    double M_client_version;

    std::shared_ptr< Localization > M_localize; //!< localization module
    InterceptTable M_intercept_table; //!< interception info table
    std::shared_ptr< AudioMemory > M_audio_memory; //!< heard deqinfo memory
    PenaltyKickState * M_penalty_kick_state; //!< penalty kick mode status

    //////////////////////////////////////////////////
    std::string M_our_team_name; //!< our teamname
    SideID M_our_side; //!< our side ID

    std::string M_their_team_name; //!< opponent teamname

    //////////////////////////////////////////////////
    // timer & mode

    GameTime M_time; //!< updated time
    GameTime M_sense_body_time; //!< sense_body updated time
    GameTime M_see_time; //!< see updated time
    GameTime M_fullstate_time; //!< fullstate update time
    GameTime M_decision_time; //!< action performed time

    TimeStamp M_see_time_stamp; //! time stamp when see received
    TimeStamp M_decision_time_stamp; //! time stamp when action performed

    GameTime M_last_set_play_start_time; //!< SetPlay started time
    int M_setplay_count; //!< setplay counter

    GameMode M_game_mode; //!< playmode and scores

    GameTime M_training_time; //!< training start/end time for keepaway

    bool M_valid; //!< if this world model is initialized, true.

    //////////////////////////////////////////////////
    // field object instance
    SelfObject M_self; //!< self object
    BallObject M_ball; //!< current ball object
    BallObject M_prev_ball; //!< ball object in the previous cycle
    PlayerObject::List M_teammates; //!< teammmates instance. at least, the side information is observed
    PlayerObject::List M_opponents; //!< opponents instance. at least, the side information is observed
    PlayerObject::List M_unknown_players; //!< unknown players instance

    //////////////////////////////////////////////////
    // object reference (pointers to each object)
    // these containers are updated just before decision making
    PlayerObject::Cont M_teammates_from_self; //!< teammates sorted by distance from self
    PlayerObject::Cont M_opponents_from_self; //!< opponents sorted by distance from ball, include unknown players
    PlayerObject::Cont M_teammates_from_ball; //!< teammates sorted by distance from self
    PlayerObject::Cont M_opponents_from_ball; //!< opponents sorted by distance from ball, include unknown players

    int M_our_goalie_unum; //!< uniform number of teammate goalie
    int M_their_goalie_unum; //!< uniform number of opponent goalie

    AbstractPlayerObject::Cont M_all_players; //!< all players pointers includes self
    AbstractPlayerObject::Cont M_our_players; //!< all teammates pointers includes self
    AbstractPlayerObject::Cont M_their_players; //!< all opponents pointers includes unknown

    AbstractPlayerObject * M_our_player_array[12]; //!< unum known teammates (include self)
    AbstractPlayerObject * M_their_player_array[12]; //!< unum known opponents (exclude unknown player)

    double M_our_recovery[11]; //!< recovery value for each player
    double M_our_stamina_capacity[11]; //!< stamina capacity for each player

    //////////////////////////////////////////////////
    // analyzed result

    double M_offside_line_x; //!< offside line x value
    double M_prev_offside_line_x; //!< offside line x value
    int M_offside_line_count; //!< accuracy count of the offside line

    double M_our_offense_line_x; //!< our offense line x value(consider ball x)
    double M_our_defense_line_x; //!< our defense line x value(consider ball x)(their offside line)
    double M_their_offense_line_x; //!< their offense line x value(consider ball x)
    double M_their_defense_line_x; //!< their defense line x value(consider ball x)
    int M_their_defense_line_count; //!< accuracy count of their defense line x value

    double M_our_offense_player_line_x; //!< our offense player line x value (not consider ball)
    double M_our_defense_player_line_x; //!< our defense player line x value (not consider ball)
    double M_their_offense_player_line_x; //!< their offense player line x value (not consider ball)
    double M_their_defense_player_line_x; //!< their defense player line x value (not consider ball)

    const PlayerObject * M_kickable_teammate; //!< kickable teammate
    const PlayerObject * M_kickable_opponent; //!< kickable opponent
    const PlayerObject * M_maybe_kickable_teammate; //!< kickable teammate
    const PlayerObject * M_maybe_kickable_opponent; //!< kickable opponent

    bool M_previous_kickable_teammate; //! flag for kickable teammate in previous cycle
    int M_previous_kickable_teammate_unum; //! uniform number kickable teammate in previous cycle
    bool M_previous_kickable_opponent; //! flag for kickable opponents in previous cycle
    int M_previous_kickable_opponent_unum; //! uniform number kickable opponent in previous cycle

    SideID M_last_kicker_side; //!< estimated last ball kicker player's side
    int M_last_kicker_unum; //!< estimated last ball kicker player's uniform number

    //////////////////////////////////////////////////
    // player type management

    int M_our_player_type[11]; //!< teammate type reference
    int M_their_player_type[11]; //!< opponent type flag

    //////////////////////////////////////////////////
    // card information

    Card M_our_card[11]; //!< our yellow/red card status
    Card M_their_card[11]; //!< their yellow/red card status

    //////////////////////////////////////////////////
    // visual info

    //! array of direction confidence count
    int M_dir_count[DIR_CONF_DIVS];

    //! view area history
    ViewAreaCont M_view_area_cont;

    //! accuracy count grid map
    ViewGridMap M_view_grid_map;

    //////////////////////////////////////////////////

    //! not used
    WorldModel( const WorldModel & ) = delete;
    //! not used
    WorldModel & operator=( const WorldModel & ) = delete;

public:
    /*!
      \brief initialize member variables
    */
    WorldModel();

    /*!
      \brief delete dynamically allocated memory
    */
    ~WorldModel();

    /*!
      \brief initialize by team information
      \param team_name our team name string
      \param our_side our side ID
      \param unum my uniform number
      \param goalie true if I am goalie
      \param client_version the client protocol version
      \return true if successfully initialized, false otherwise

      This method is called just after receive init reply
    */
    bool init( const std::string & team_name,
               const SideID our_side,
               const int unum,
               const bool goalie,
               const double client_version );

    /*!
      \brief get this world mode is valid or not
      \return true if this world model is valid
    */
    bool isValid() const;

    /*!
      \brief set this world mode is valid or not
      \param is_valid value to be set
    */
    void setValid( bool is_valid );

    /*!
      \brief get the client version.
      \return the version numver
     */
    double clientVersion() const
      {
          return M_client_version;
      }

    /*!
      \brief get intercept table
      \return const pointer to the intercept table instance
    */
    const InterceptTable & interceptTable() const
    {
        return M_intercept_table;
    }

    /*!
      \brief get penalty kick state
      \return const pointer to the penalty kick state instance
    */
    const PenaltyKickState * penaltyKickState() const;

    /*!
      \brief get audio memory
      \return const reference to the audio memory instance
     */
    const AudioMemory & audioMemory() const
      {
          return *M_audio_memory;
      }

    /*!
      \brief set new audio memory
      \param memory pointer to the memory instance. This must be
      a dynamically allocated object.
     */
    void setAudioMemory( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief set new localization method
      \param localization pointer to the localization instance.
    */
    void setLocalization( std::shared_ptr< Localization > localization );

    /*!
      \brief set server param. this method have to be called only once just after server_param message received.
     */
    void setServerParam();

    /*!
      \brief set our goalie's uniform number using strategic information or coach message.
      \param unum uniform number
     */
    void setOurGoalieUnum( const int unum );

    /*!
      \brief set their goalie's uniform number using coach message.
      \param unum uniform number
     */
    void setTheirGoalieUnum( const int unum );

    /*!
      \brief set teammate player type & reset card status
      \param unum uniform number of changed teammate
      \param id player type ID
    */
    void setOurPlayerType( const int unum,
                           const int id );

    /*!
      \brief set opponent player type & reset card status
      \param unum uniform number of changed opponent
      \param id player type ID
    */
    void setTheirPlayerType( const int unum,
                             const int id );

    /*!
      \brief set yellow card information
      \param side punished player's side
      \param unum punished player's unum
      \param card card type
     */
    void setCard( const SideID side,
                  const int unum,
                  const Card card );

    /*!
      \brief set current penalty kick taker.
      \param order order number [0,10]
      \param unum kick taker's uniform number
     */
    void setPenaltyKickTakerOrder( const std::vector< int > & unum_set );

    // update stuff
private:
    /*!
      \brief internal update
      \param act action effector
      \param current current game time

      This method updates world status using recorded action effect only
    */
    void update( const ActionEffector & act,
                 const GameTime & current );
public:
    /*!
      \brief update by sense_body.
      \param sense_body analyzed sense_body info
      \param act action effector
      \param current current game time

      This method is called just after sense_body message receive
    */
    void updateAfterSenseBody( const BodySensor & sense_body,
                               const ActionEffector & act,
                               const GameTime & current );

    /*!
      \brief update by see info
      \param see analyzed see info
      \param sense_body analyzed sense_body info
      \param act action effector
      \param current current game time

      This method is called just after see message receive
    */
    void updateAfterSee( const VisualSensor & see,
                         const BodySensor & sense_body,
                         const ActionEffector & act,
                         const GameTime & current );

    /*!
      \brief update by fullstate info
      \param fullstate analyzed fullstate info
      \param act action effector
      \param current current game time

      This method is called just after fullstate message receive
     */
    void updateAfterFullstate( const FullstateSensor & fullstate,
                               const ActionEffector & act,
                               const GameTime & current );

    /*!
      \brief update current playmode
      \param game_mode playmode info
      \param current current game time

      This method is called after heared referee message
    */
    void updateGameMode( const GameMode & game_mode,
                         const GameTime & current );

    /*!
      \brief set training start/end time
      \param t game time
     */
    void setTrainingTime( const GameTime & t )
      {
          M_training_time = t;
      }

    /*!
      \brief update self view move
      \param w new view width
      \param q new view quality
    */
    void setViewMode( const ViewWidth & w,
                      const ViewQuality & q )
      {
          M_self.setViewMode( w, q );
      }

    /*!
      \brief internal update for action decision
      \param act action effector
      \param current current game time

      This method is called just before action decision to update and
      adjust world model.
    */
    void updateJustBeforeDecision( const ActionEffector & act,
                                   const GameTime & current );

    /*!
      \brief update using internal by command effects. This method is called just before command sending.
      \param act ActionEffector object.
    */
    void updateJustAfterDecision( const ActionEffector & act );

    /*!
      \brief set player's side/unum/goalie information
      \param player player object pointer
      \param unum new uniform number
      \param goalie goalie flag
     */
    void updatePlayer( const PlayerObject * player,
                       const SideID side,
                       const int unum,
                       const bool goalie );

private:
    /*!
      \brief self localization
      \param see analyzed see info
      \param sense_body analyzed sense_body info
      \param current current game time
      \return if failed, returns false
    */
    bool localizeSelf( const VisualSensor & see,
                       const BodySensor & sense_body,
                       const ActionEffector & act,
                       const GameTime & current );

    /*!
      \brief ball localization
      \param see analyzed see info
      \param act action effector
      \param current current game time
    */
    void localizeBall( const VisualSensor & see,
                       const ActionEffector & act,
                       const GameTime & current );

    /*!
      \brief estimate ball velocity using position difference
      \param see analyzed see info
      \param act action effector
      \param rpos seen relative pos
      \param rpos_error seen relative pos error
      \param vel reference to the velocity variable
      \param vel_error reference to the velocity error variable
      \param vel_count reference to the velocity count variable
    */
    void estimateBallVelByPosDiff( const VisualSensor & see,
                                   const ActionEffector & act,
                                   const Vector2D & rpos,
                                   const Vector2D & rpos_error,
                                   Vector2D & vel,
                                   Vector2D & vel_error,
                                   int & vel_count );

    /*!
      \brief players localization
      \param see analyzed see info
      \param current current game time
    */
    void localizePlayers( const VisualSensor & see );

    /*!
      \brief check player that has team info
      \param side seen side info
      \param player localized info
      \param old_known_players old team known players
      \param old_unknown_players previous unknown players
      \param new_known_players new team known players
    */
    void checkTeamPlayer( const SideID side,
                          const Localization::PlayerT & player,
                          PlayerObject::List & old_known_players,
                          PlayerObject::List & old_unknown_players,
                          PlayerObject::List & new_known_players );

    /*!
      \brief check player that has no identifier. matching to unknown players
      \param player localized info
      \param old_teammates previous seen teammates
      \param old_opponents previous seen opponents
      \param old_unknown_players previous seen unknown player
      \param new_teammates current seen teammates
      \param new_opponents current seen opponents
      \param new_unknown_players current seen unknown players
    */
    void checkUnknownPlayer( const Localization::PlayerT & player,
                             PlayerObject::List & old_teammates,
                             PlayerObject::List & old_opponent,
                             PlayerObject::List & old_unknown_players,
                             PlayerObject::List & new_teammates,
                             PlayerObject::List & new_opponents,
                             PlayerObject::List & new_unknown_players );

    /*!
      \brief set collision effect with ball
    */
    void updateBallCollision();

    /*!
      \brief set collision effect with players
    */
    void updatePlayersCollision();

    /*!
      \brief check ghost object
      \param varea current view area info
    */
    void checkGhost( const ViewArea & varea );

    /*!
      \brief update seen direction accuracy
      \param varea seen view area info
    */
    void updateDirCount( const ViewArea & varea );

    /*!
      \brief update ball by heard info
    */
    void updateBallByHear( const ActionEffector & act );

    /*!
      \brief update opponent goalie by heard info
    */
    void updateGoalieByHear();

    /*!
      \brief update players by heard info
    */
    void updatePlayerByHear();

    /*!
      \brief update other players' stamina by heard info
    */
    void updatePlayerStaminaByHear();

    /*!
      \brief update player type id of the recognized players
     */
    void updatePlayerType();

    /*!
      \brief update players' card state
     */
    void updatePlayerCard();

    /*!
      \brief estimate unknown players' uniform number
     */
    void estimateUnknownPlayerUnum();

    /*!
      \brief update player info. relation with ball and self.
     */
    void updatePlayerStateCache();

    /*!
      \brief update our/their goalie
     */
    void estimateGoalie();

    /*!
      \brief update our goalie
     */
    void estimateOurGoalie();

    /*!
      \brief update their goalie
     */
    void estimateTheirGoalie();

    /*!
      \brief update maybe kickable teammate
      this methos has to be called after WorldModel::updateInterceptTable()
     */
    void estimateMaybeKickableTeammate();

    /*!
      \brief update kickable players.
      this method has to be called after calling PlayerObject::updateSelfBallRelated().
     */
    void updateKickablePlayers();

    /*!
      \brief update offside line
    */
    void updateOffsideLine();

    /*!
      \brief update our offense line
    */
    void updateOurOffenseLine();

    /*!
      \brief update our defense line
    */
    void updateOurDefenseLine();

    /*!
      \brief update their offense line
    */
    void updateTheirOffenseLine();

    /*!
      \brief update their defense line (offside line)
    */
    void updateTheirDefenseLine();

    /*!
      \brief update forward & defense player lines
     */
    void updatePlayerLines();

    /*!
      \brief estimate last kicker player
     */
    void updateLastKicker();

    /*!
      \brief update intercept table
     */
    void updateInterceptTable();
public:

    /*!
      \brief get our teamname
      \return const reference to the team name string
    */
    const std::string & teamName() const { return M_our_team_name; }
    const std::string & ourTeamName() const { return M_our_team_name; }

    /*!
      \brief get our team side Id
      \return side Id
    */
    SideID ourSide() const { return M_our_side; }

    /*!
      \brief get their teamname
      \return const reference to the team name string
    */
    const std::string & theirTeamName() const { return M_their_team_name; }

    /*!
      \brief get opponent team side Id
      \return side Id
    */
    SideID theirSide() const
      {
          return M_our_side == LEFT ? RIGHT : LEFT;
      }

    /*!
      \brief get last updated time (== current game time)
      \return const reference to the game time object
    */
    const GameTime & time() const { return M_time; }

    /*!
      \brief get last time updated by sense_body
      \return const reference to the game time object
    */
    const GameTime & senseBodyTime() const { return M_sense_body_time; }

    /*!
      \brief get last time updated by see
      \return const reference to the game time object
    */
    const GameTime & seeTime() const { return M_see_time; }

    /*!
       \brief get last time updated by fullstate
       \return const reference to the game time object
     */
    const GameTime & fullstateTime() const { return M_fullstate_time; }

    /*!
      \brief get the time value when the last action performed
     */
    const GameTime & decisionTime() const { return M_decision_time; }

    /*!
      \brief get the time stamp when see received
    */
    const TimeStamp & seeTimeStamp() const { return M_see_time_stamp; }

    /*!
      \brief get the time stamp when action performed
    */
    const TimeStamp & decisionTimeStamp() const { return M_decision_time_stamp; }

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
      \brief get current playmode info
      \return const reference to the GameMode object
    */
    const GameMode & gameMode() const { return M_game_mode; }

    /*!
      \brief get training start/end time
      \return game time object
     */
    const GameTime & trainingTime() const { return M_training_time; }

    /*!
      \brief get self info
      \return const reference to the SelfObject
    */
    const SelfObject & self() const { return M_self; }

    /*!
      \brief get the current ball info
      \return const reference to the BallObject
    */
    const BallObject & ball() const { return M_ball; }

    /*!
      \brief get the previous ball info
      \return const reference to the BallObject
    */
    const BallObject & prevBall() const { return M_prev_ball; }

    /*!
      \brief get teammates. the order is undefined.
      \return const reference to the PlayerObject pointer container
    */
    const PlayerObject::Cont & teammates() const { return M_teammates_from_self; }

    /*!
      \brief get opponents(include unknown players). the order is undefined
      \return const reference to the PlayerObject container
    */
    const PlayerObject::Cont & opponents() const { return M_opponents_from_self; }

    // reference to the sorted players

    /*!
      \brief get teammates sorted by distance from self
      \return const reference to the PlayerObject pointer container
    */
    const PlayerObject::Cont & teammatesFromSelf() const { return M_teammates_from_self; }

    /*!
      \brief get opponents sorted by distance from self (includes unknown players)
      \return const reference to the PlayerObject pointer container
    */
    const PlayerObject::Cont & opponentsFromSelf() const { return M_opponents_from_self; }

    /*!
      \brief get teammates sorted by distance from ball
      \return const reference to the PlayerObject pointer container
    */
    const PlayerObject::Cont & teammatesFromBall() const { return M_teammates_from_ball; }

    /*!
      \brief get opponents sorted by distance from ball (includes unknown players)
      \return const reference to the PlayerObject pointer container
    */
    const PlayerObject::Cont & opponentsFromBall() const { return M_opponents_from_ball; }

    /*!
      \brief get the uniform number of teammate goalie
      \return uniform number value or Unum_Unknown
     */
    int ourGoalieUnum() const { return M_our_goalie_unum; }

    /*!
      \brief get the uniform number of opponent goalie
      \return uniform number value or Unum_Unknown
     */
    int theirGoalieUnum() const { return M_their_goalie_unum; }

    /*!
      \brief get all players includes self.
      \return const rerefence to the AbstractPlayerObject pointer container.
     */
    const AbstractPlayerObject::Cont & allPlayers() const { return M_all_players; }

    /*!
      \brief get all teammate players includes self.
      \return const rerefence to the AbstractPlayerObject pointer container.
     */
    const AbstractPlayerObject::Cont & ourPlayers() const { return M_our_players; }

    /*!
      \brief get all opponent players (includes unknown players)
      \return const rerefence to the AbstractPlayerObject pointer container.
     */
    const AbstractPlayerObject::Cont & theirPlayers() const { return M_their_players; }

    //////////////////////////////////////////////////////////

    /*!
      \brief get a teammate (or self) specified by uniform number
      \param unum wanted player's uniform number
      \return const pointer to the AbstractPlayerObject instance or NULL
    */
    const AbstractPlayerObject * ourPlayer( const int unum ) const
      {
          if ( unum <= 0 || 11 < unum ) return M_our_player_array[0];
          return M_our_player_array[unum];
      }

    /*!
      \brief get an opponent specified by uniform number
      \param unum wanted player's uniform number
      \return const pointer to the AbstractPlayerObject instance or NULL
    */
    const AbstractPlayerObject * theirPlayer( const int unum ) const
      {
          if ( unum <= 0 || 11 < unum ) return M_their_player_array[0];
          return M_their_player_array[unum];
      }

    /*!
      \brief get (heard) recovery value of the specified player
      \return recovery value
     */
    double ourRecovery( const int unum ) const
      {
          if ( unum <= 0 || 11 < unum ) return 0.0;
          return M_our_recovery[ unum - 1 ];
      }

    /*!
      \brief get (heard) stamina capacity value of the specified player
      \return recovery value
     */
    double ourStaminaCapacity( const int unum ) const
      {
          if ( unum <= 0 || 11 < unum ) return 0.0;
          return M_our_stamina_capacity[ unum - 1 ];
      }

private:

    /*!
      \brief get fist PlayerObject in [first, last] that satisfies confidence
      count threshold
      \param first first iterator of PlayerObject pointer container
      \param last last iterator of PlayerObject pointer container
      \param count_thr accuracy count threshold
      \param with_goalie if this value is false, goalie is ignored.
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const PlayerObject * getFirstPlayer( const PlayerObject::Cont & players,
                                         const int count_thr,
                                         const bool with_goalie ) const
      {
          for ( PlayerObject::Cont::const_reference p : players )
          {
              if ( ! with_goalie
                   && p->goalie() )
              {
                  continue;
              }

              if ( ! p->isGhost()
                   && p->posCount() <= count_thr )
              {
                  return p;
              }
          }
          return nullptr;
      }

public:

    /*!
      \brief get teammate nearest to self with confidence count check
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const PlayerObject * getTeammateNearestToSelf( const int count_thr,
                                                   const bool with_goalie = true ) const
      {
          return getFirstPlayer( teammatesFromSelf(),
                                 count_thr,
                                 with_goalie );
      }

    /*!
      \brief get opponent nearest to self with accuracy count check
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const PlayerObject * getOpponentNearestToSelf( const int count_thr,
                                                   const bool with_goalie = true ) const
      {
          return getFirstPlayer( opponentsFromSelf(),
                                 count_thr,
                                 with_goalie );
      }

    /*!
      \brief get the distance from teammate nearest to self wtth accuracy count
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistTeammateNearestToSelf( const int count_thr,
                                         const bool with_goalie = true ) const
      {
          const PlayerObject * p = getTeammateNearestToSelf( count_thr, with_goalie );
          return ( p ? p->distFromSelf() : DIST_TOO_FAR );
      }

    /*!
      \brief get the distance from opponent nearest to self wtth accuracy count
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistOpponentNearestToSelf( const int count_thr,
                                         const bool with_goalie = true) const
      {
          const PlayerObject * p = getOpponentNearestToSelf( count_thr, with_goalie );
          return ( p ? p->distFromSelf() : DIST_TOO_FAR );
      }

    /*!
      \brief get teammate nearest to with with confidence count check
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const PlayerObject * getTeammateNearestToBall( const int count_thr,
                                                   const bool with_goalie = true ) const
      {
          return getFirstPlayer( teammatesFromBall(),
                                 count_thr,
                                 with_goalie );
      }

    /*!
      \brief get opponent nearest to ball with confidence count check
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return if found, const pointer to the PlayerOjbect. else NULL
    */
    const PlayerObject * getOpponentNearestToBall( const int count_thr,
                                                   const bool with_goalie = true ) const
      {
          return getFirstPlayer( opponentsFromBall(),
                                 count_thr,
                                 with_goalie );
      }

    /*!
      \brief get the distance to teammate nearest to ball wtth accuracy count
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistTeammateNearestToBall( const int count_thr,
                                         const bool with_goalie ) const
      {
          const PlayerObject * p = getTeammateNearestToSelf( count_thr, with_goalie );
          return ( p ? p->distFromBall() : DIST_TOO_FAR );
      }

    /*!
      \brief get the distance to opponent nearest to ball wtth accuracy count
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistOpponentNearestToBall( const int count_thr,
                                         const bool with_goalie = true ) const
      {
          const PlayerObject * p = getOpponentNearestToBall( count_thr, with_goalie );
          return ( p ? p->distFromBall() : DIST_TOO_FAR );
      }

    /*!
      \brief get estimated offside line x coordinate
      \return offside line
    */
    double offsideLineX() const { return M_offside_line_x; }

    /*!
      \brief get estimated offside line x coordinate
      \return offside line
    */
    double prevOffsideLineX() const { return M_offside_line_x; }

    /*!
      \brief get the accuracy count for the offside line
      \return accuracy count
     */
    int offsideLineCount() const { return M_offside_line_count; }

    /*!
      \brief our offense line (consider ball pos)
      \return our offense line x
    */
    double ourOffenseLineX() const { return M_our_offense_line_x; }

    /*!
      \brief get our defense line (consider ball pos)(offside line for opponent)
      \return our defense line x
    */
    double ourDefenseLineX() const { return M_our_defense_line_x; }

    /*!
      \brief get their offense line (consider ball pos)
      \return their offense line x
    */
    double theirOffenseLineX() const { return M_their_offense_line_x; }

    /*!
      \brief get their defense line x value (consider ball pos)
      \return their offense line x
    */
    double theirDefenseLineX() const { return M_their_defense_line_x; }

    /*!
      \brief get our offense player line (not consder ball pos)
      \return our offense player line x
    */
    double ourOffensePlayerLineX() const { return M_our_offense_player_line_x; }

    /*!
      \brief get our defense player line (not consder ball pos)
      \return our defense player line x
    */
    double ourDefensePlayerLineX() const { return M_our_defense_player_line_x; }

    /*!
      \brief get their offense player line (not consder ball pos)
      \return our defense player line x
    */
    double theirOffensePlayerLineX() const { return M_their_offense_player_line_x; }

    /*!
      \brief get estimated their offense line x value
      \return their offense line x
    */
    double theirDefensePlayerLineX() const { return M_their_defense_player_line_x; }

    /*!
      \brief get the estimated kickable teammate if exist.
      \return player object pointer
     */
    const PlayerObject * kickableTeammate() const { return M_kickable_teammate; }

    /*!
      \brief get the estimated kickable opponent if exist.
      \return player object pointer
     */
    const PlayerObject * kickableOpponent() const { return M_kickable_opponent; }

    /*!
      \brief get the estimated kickable teammate (maybe kickable).
      \return player object pointer
     */
    const PlayerObject * maybeKickableTeammate() const { return M_maybe_kickable_teammate; }

    /*!
      \brief get the estimated kickable opponent (maybe kickable).
      \return player object pointer
     */
    const PlayerObject * maybeKickableOpponent() const { return M_maybe_kickable_opponent; }


    /*!
      \brief get the estimated last kicker's side
      \return side id
     */
    SideID lastKickerSide() const { return M_last_kicker_side; }

    /*!
      \brief get the estimated last kicker's uniform number
      \return uniform number
     */
    int lastKickerUnum() const { return M_last_kicker_unum; }

    /*!
      \brief get player type Id of teammate
      \param unum target teammate uniform number
      \return player type Id. if unum is illegal, Default Id is returned.
    */
    int ourPlayerTypeId( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "WorldModel::teammateHeteroID. Illegal unum "
                        << unum << std::endl;
              return Hetero_Default;
          }
          return M_our_player_type[ unum - 1 ];
      }

    /*!
      \brief get player type of the specified teammate
      \param unum target teammate uniform number
      \return const pointer to the player type object instance
     */
    const PlayerType * ourPlayerType( const int unum ) const;

    /*!
      \brief get player type Id of opponent
      \param unum target teammate uniform number
      \return player type Id. if unum is illegal, Unknown is returned.
    */
    int theirPlayerTypeId( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "WorldModel::opponentHeteroID. Illegal unum "
                        << unum << std::endl;
              return Hetero_Unknown;
          }
          return M_their_player_type[ unum - 1 ];
      }

    /*!
      \brief get player type of the specified opponent
      \param unum target opponent uniform number
      \return const pointer to the player type object instance
     */
    const PlayerType * theirPlayerType( const int unum ) const;

    //
    // card information
    //

    /*!
      \brief get teammate's yellow card status
      \param unum uniform number
      \return yellow card status
     */
    Card ourCard( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "(WorldModel::teammateCard) Illegal unum "
                        << unum << std::endl;
              return NO_CARD;
          }
          return M_our_card[ unum - 1 ];
      }

    /*!
      \brief get opponent's yellow card status
      \param unum uniform number
      \return yellow card status
     */
    Card theirCard( const int unum ) const
      {
          if ( unum < 1 || 11 < unum )
          {
              std::cerr << "(WorldModel::opponentCard) Illegal unum "
                        << unum << std::endl;
              return NO_CARD;
          }
          return M_their_card[ unum - 1 ];
      }

    //
    // analyzed results
    //


    // visual memory info

    /*!
      \brief get direction confidence count
      \param angle target direction
      \return confidence count value
    */
    int dirCount( const AngleDeg & angle ) const
      {
          int idx = static_cast< int >( ( angle.degree() - 0.5 + 180.0 )
                                        / DIR_STEP );
          if ( idx < 0 || DIR_CONF_DIVS - 1 < idx )
          {
              std::cerr << "WorldModel::getDirConf. index over flow"
                        << std::endl;
              idx = 0;
          }
          return M_dir_count[idx];
      }

    /*!
      \brief get max count, sum of count and average count of angle range
      \param angle center of target angle range
      \param width angle range
      \param max_count pointer to variable of max accuracy count
      \param sum_count pointer to variable of sum of accuracy count
      \param ave_count pointer to variable of average accuracy count
      \return steps in the range
    */
    int dirRangeCount( const AngleDeg & angle,
                       const double & width,
                       int * max_count,
                       int * sum_count,
                       int * ave_count ) const;

    /*!
      \brief get view area history container
      \return const refrence to the view area container.
     */
    const ViewAreaCont & viewAreaCont() const { return M_view_area_cont; }

    /*!
      \brief get field grid map that holds observation accuracy count
      \return const reference to the ViewGridMap instance
     */
    const ViewGridMap & viewGridMap() const { return M_view_grid_map; }

    /*!
      \brief get the specific point accuracy count
      \param point global cooridinate value of checked point
      \param dir_thr direction threshold for view area
      \return accuracy count. if player has not seen the point, returnthe big value (e.g. 1000).
     */
    int getPointCount( const Vector2D & point,
                       const double & dir_thr ) const;

    //
    // interfaces to player objects
    //

    /*!
      \brief get the new container of AbstractPlayer matched with the predicate.
      \param predicate predicate object for the player condition matching. This have to be a dynamically allocated object.
      \return container of AbstractPlayer pointer.
     */
    AbstractPlayerObject::Cont getPlayers( const PlayerPredicate * predicate ) const;

    /*!
      \brief get the new container of AbstractPlayer matched with the predicate.
      \param predicate predicate object for the player condition matching.
      \return container of AbstractPlayer pointer.
     */
    AbstractPlayerObject::Cont getPlayers( std::shared_ptr< const PlayerPredicate > predicate ) const;

    /*!
      \brief get the new container of AbstractPlayer matched with the predicate.
      \param result reference to the result variable
      \param predicate predicate object for the player condition matching. This have to be a dynamically allocated object.
     */
    void getPlayers( AbstractPlayerObject::Cont & result,
                     const PlayerPredicate * predicate ) const;

    /*!
      \brief get the new container of AbstractPlayer matched with the predicate.
      \param cont reference to the result variable
      \param predicate predicate object for the player condition matching.
     */
    void getPlayers( AbstractPlayerObject::Cont & result,
                     std::shared_ptr< const PlayerPredicate > predicate ) const;

    /*!
      \brief get the number of players that satisfy an input predicate.
      \param predicate predicate predicate object for the player condition matching. This have to be a dynamically allocated object.
      \return number of players.
     */
    size_t countPlayer( const PlayerPredicate * predicate ) const;

    /*!
      \brief get the number of players that satisfy an input predicate.
      \param predicate predicate predicate object for the player condition matching.
      \return number of players.
     */
    size_t countPlayer( std::shared_ptr< const PlayerPredicate > predicate ) const;

    /*!
      \brief get a goalie teammate (include self)
      \return if found pointer to goalie object, otherwise NULL
    */
    const AbstractPlayerObject * getOurGoalie() const;

    /*!
      \get opponent goalie pointer
      \return if found pointer to goalie object, otherwise NULL
     */
    const AbstractPlayerObject * getTheirGoalie() const;

private:

    /*!
      \brief get player pointer nearest to point (excludes self)
      \param point considered point
      \param players target players
      \param count_thr confidence count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
     */
    const PlayerObject * getPlayerNearestTo( const Vector2D & point,
                                             const PlayerObject::Cont & players,
                                             const int count_thr,
                                             double * dist_to_point ) const;

    /*!
      \brief get the distance from input point to the nearest player
      \param players target players
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistPlayerNearestTo( const Vector2D & point,
                                   const PlayerObject::Cont & players,
                                   const int count_thr ) const
      {
          double d = DIST_TOO_FAR;
          const PlayerObject * p = getPlayerNearestTo( point, players, count_thr, &d );
          return ( p ? d : DIST_TOO_FAR );
      }

public:

    /*!
      \brief get the distance from input point to the nearest teammate
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistTeammateNearestTo( const Vector2D & point,
                                     const int count_thr ) const
      {
          double d = DIST_TOO_FAR;
          const PlayerObject * p = getPlayerNearestTo( point, teammatesFromSelf(), count_thr, &d );
          return ( p ? d : DIST_TOO_FAR );
      }

    /*!
      \brief get the distance from input point to the nearest opponent
      \param count_thr accuracy count threshold
      \param with_goalie include goalie if true
      \return distance to the matched opponent. if not found, a big value is returned.
     */
    double getDistOpponentNearestTo( const Vector2D & point,
                                     const int count_thr ) const
      {
          double d = DIST_TOO_FAR;
          const PlayerObject * p = getPlayerNearestTo( point, opponentsFromSelf(), count_thr, &d );
          return ( p ? d : DIST_TOO_FAR );
      }

    /*!
      \brief get teammate pointer nearest to point
      \param point considered point
      \param count_thr confidence count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
    */
    const PlayerObject * getTeammateNearestTo( const Vector2D & point,
                                               const int count_thr,
                                               double * dist_to_point ) const
      {
          return getPlayerNearestTo( point, teammatesFromSelf(), count_thr, dist_to_point );
      }

    /*!
      \brief get teammate pointer nearest to the specified player
      \param p pointer to the player
      \param count_thr accuracy count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
     */
    const PlayerObject * getTeammateNearestTo( const PlayerObject * p,
                                               const int count_thr,
                                               double * dist_to_point ) const
      {
          if ( ! p ) return nullptr;
          return getTeammateNearestTo( p->pos(), count_thr, dist_to_point );
      }

    /*!
      \brief get opponent pointer nearest to point
      \param point considered point
      \param count_thr accuracy count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found pointer to player object, othewise NULL
    */
    const PlayerObject * getOpponentNearestTo( const Vector2D & point,
                                               const int count_thr,
                                               double * dist_to_point ) const
      {
          return getPlayerNearestTo( point, opponentsFromSelf(), count_thr, dist_to_point );
      }

    /*!
      \brief get teammate pointer nearest to the specified player
      \param p pointer to the player
      \param count_thr accuracy count threshold
      \param dist_to_point variable pointer to store the distance
      from retuned player to point
      \return if found, pointer to player object, othewise NULL
     */
    const PlayerObject * getOpponentNearestTo( const PlayerObject * p,
                                         const int count_thr,
                                         double * dist_to_point ) const
      {
          if ( ! p ) return nullptr;
          return getOpponentNearestTo( p->pos(), count_thr, dist_to_point );
      }

private:

    /*!
      \brief template utility. check if player exist in the specified region.
      \param region template parameter. region to be checked
      \param players target players
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
     */
    template < typename REGION >
    bool existPlayerIn( const REGION & region,
                        const PlayerObject::Cont & players,
                        const int count_thr,
                        const bool with_goalie ) const
      {
          for ( PlayerObject::Cont::const_reference p : players )
          {
              if ( p->posCount() > count_thr
                   || p->isGhost() )
              {
                  continue;
              }
              if ( p->goalie() && ! with_goalie )
              {
                  continue;
              }
              if ( region.contains( p->pos() ) )
              {
                  return true;
              }
          }
          return false;
      }

public:

    /*!
      \brief template utility. check if teammate exist in the specified region.
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
     */
    template < typename REGION >
    bool existTeammateIn( const REGION & region,
                          const int count_thr,
                          const bool with_goalie ) const
      {
          return existPlayerIn( region, teammatesFromSelf(), count_thr, with_goalie );
      }

    /*!
      \brief template utility. check if opponent exist in the specified region.
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return true if some opponent exist
     */
    template < typename REGION >
    bool existOpponentIn( const REGION & region,
                          const int count_thr,
                          const bool with_goalie ) const
      {
          return existPlayerIn( region, opponentsFromSelf(), count_thr, with_goalie );
      }

private:

    /*!
      \brief count the number of teammate exist in the specified region
      \param region template parameter. region to be checked
      \param players target players
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return total count of teammate existed in the region
     */
    template < typename REGION >
    size_t countPlayersIn( const REGION & region,
                           const PlayerObject::Cont & players,
                           const int count_thr,
                           const bool with_goalie ) const
      {
          size_t count = 0;
          for ( PlayerObject::Cont::const_reference p : players )
          {
              if ( p->posCount() > count_thr
                   || p->isGhost()
                   || ( p->goalie() && ! with_goalie )
                   )
              {
                  continue;
              }
              if ( region.contains( p->pos() ) )
              {
                  ++count;
              }
          }
          return count;
      }

public:

    /*!
      \brief count the number of teammate exist in the specified region
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return total count of teammate existed in the region
     */
    template < typename REGION >
    size_t countTeammatesIn( const REGION & region,
                             const int count_thr,
                             const bool with_goalie ) const
      {
          return countPlayersIn( region, teammatesFromSelf(), count_thr, with_goalie );
      }

    /*!
      \brief count the number of opponent exist in the specified region
      \param region template parameter. region to be checked
      \param count_thr confdence count threshold for players
      \param with_goalie if true, goalie player is cheked.
      \return total count of opponent existed in the region
     */
    template < typename REGION >
    size_t countOpponentsIn( const REGION & region,
                             const int count_thr,
                             const bool with_goalie ) const
      {
          return countPlayersIn( region, opponentsFromSelf(), count_thr, with_goalie );
      }

};

}

#endif
