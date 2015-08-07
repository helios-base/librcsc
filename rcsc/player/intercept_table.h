// -*-c++-*-

/*!
  \file intercept_table.h
  \brief interception info holder Header File
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

#ifndef RCSC_PLAYER_INTERCEPT_TABLE_H
#define RCSC_PLAYER_INTERCEPT_TABLE_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <vector>
#include <map>

namespace rcsc {

class AbstractPlayerObject;
class PlayerObject;
class WorldModel;

/*-------------------------------------------------------------------*/

/*!
  \class InterceptInfo
  \brief interception data
*/
class InterceptInfo {
public:

    /*!
      \enum StaminaType
      \brief stamina type
     */
    enum StaminaType {
        NORMAL = 0, //!< ball gettable without stamina exhaust
        EXHAUST = 100, //!< fastest ball gettable, but recovery may be consumed.
    };

    /*!
      \enum ActionType
      \brief action type
     */
    enum ActionType {
        OMNI_DASH = 0,
        TURN_FORWARD_DASH = 1,
        TURN_BACK_DASH = 2,
        UNKNOWN_TYPE = 100,
    };

private:
    StaminaType M_stamina_type; //!< stamina type
    ActionType M_action_type; //!< action type

    int M_turn_step; //!< estimated turn step
    double M_turn_angle; //!< angle difference between current body angle and dash angle

    int M_dash_step; //!< estimated dash step
    double M_dash_power; //!< first dash power
    double M_dash_dir; //!< first dash direction (relative to body)

    Vector2D M_self_pos; //!< estimated final self position
    double M_ball_dist; //!< estimated final ball distance
    double M_stamina; //!< estimated final stamina value

    double M_value; //!< evaluation value

public:

    /*!
      \brief create invalid info
     */
    InterceptInfo()
        : M_stamina_type( EXHAUST ),
          M_action_type( UNKNOWN_TYPE ),
          M_turn_step( 10000 ),
          M_turn_angle( 0.0 ),
          M_dash_step( 10000 ),
          M_dash_power( 100000.0 ),
          M_dash_dir( 0.0 ),
          M_self_pos( -10000.0, 0.0 ),
          M_ball_dist( 10000000.0 ),
          M_stamina( 0.0 ),
          M_value( 0.0 )
      { }

    /*!
      \brief construct with all variables
    */
    InterceptInfo( const StaminaType stamina_type,
                   const ActionType action_type,
                   const int turn_step,
                   const double turn_angle,
                   const int dash_step,
                   const double dash_power,
                   const double dash_dir,
                   const Vector2D & self_pos,
                   const double ball_dist,
                   const double stamina )
        : M_stamina_type( stamina_type ),
          M_action_type( action_type ),
          M_turn_step( turn_step ),
          M_turn_angle( turn_angle ),
          M_dash_step( dash_step ),
          M_dash_power( dash_power ),
          M_dash_dir( dash_dir ),
          M_self_pos( self_pos ),
          M_ball_dist( ball_dist ),
          M_stamina( stamina ),
          M_value( 0.0 )
      { }

    /*!
      \brief check if this object is legal one or not.
      \return checked result.
     */
    bool isValid() const
      {
          return M_action_type != UNKNOWN_TYPE;
      }

    /*!
      \brief get stamina type id
      \return stamina type id
    */
    StaminaType staminaType() const
      {
          return M_stamina_type;
      }

    /*!
      \brief get interception action type
      \return type id
     */
    ActionType actionType() const
      {
          return M_action_type;
      }

    /*!
      \brief get estimated total turn steps
      \return the number of turn steps
    */
    int turnCycle() const { return M_turn_step; }
    int turnStep() const { return M_turn_step; }

    /*!
      \brief get estimated total dash cycles
      \return the number of dash steps
    */
    int dashCycle() const { return M_dash_step; }
    int dashStep() const { return M_dash_step; }

    /*!
      \brief get esitimated total step to reach
      \return the number of total steps
    */
    int reachCycle() const { return turnStep() + dashStep(); }
    int reachStep() const { return turnStep() + dashStep(); }

    /*!
      \brief get dash power for the first dash
      \return dash power value
    */
    double dashPower() const
      {
          return M_dash_power;
      }

    /*!
      \brief ger dash direction for the first dash
      \return dash direction value (relative to body)
     */
    double dashDir() const
      {
          return M_dash_dir;
      }

    /*!
      \brief get the estimated final self position
      \return final self position
     */
    const Vector2D & selfPos() const
      {
          return M_self_pos;
      }

    /*!
      \brief get the estimated final ball distance
      \return final ball distance
     */
    double ballDist() const
      {
          return M_ball_dist;
      }

    /*!
      \brief get the estimated self stamina value
      \return final self stamina value
     */
    double stamina() const
      {
          return M_stamina;
      }
};

/*-------------------------------------------------------------------*/

/*!
  \class InterceptTable
  \brief interception info holder for all players
*/
class InterceptTable {
private:

    //! reference to the WorldModel instance
    const WorldModel & M_world;

    //! last updated time
    GameTime M_update_time;

    //! ball inertia movement position cache
    std::vector< Vector2D > M_ball_cache;

    //! predicted min reach step for self without stamina exhaust
    int M_self_reach_step;
    //! predicted min reach step for self with stamina exhaust
    int M_self_exhaust_reach_step;
    //! predicted min reach step for teammate
    int M_teammate_reach_step;
    //! predicted reach step for second fastest teammate
    int M_second_teammate_reach_step;
    //! predicted min reach step for teammate goalie
    int M_goalie_reach_step;
    //! predicted min reach step for opponent
    int M_opponent_reach_step;
    //! predicted reach step for second fastest opponent
    int M_second_opponent_reach_step;

    //! const pointer to the fastest ball gettable teammate player object
    const PlayerObject * M_fastest_teammate;
    //! const pointer to the second fastest ball gettable teammate player object
    const PlayerObject * M_second_teammate;
    //! const pointer to the fastest ball gettable opponent player object
    const PlayerObject * M_fastest_opponent;
    //! const pointer to the second fastest ball gettable opponent player object
    const PlayerObject * M_second_opponent;

    //! interception info cache for smart interception
    std::vector< InterceptInfo > M_self_cache;

    //! all players' intercept step container. key: pointer, value: step value
    std::map< const AbstractPlayerObject *, int > M_player_map;

    // not used
    InterceptTable();
    InterceptTable( const InterceptTable & );
    InterceptTable & operator=( const InterceptTable & );

public:
    /*!
      \brief init member variables, reserve cache vector memory
    */
    explicit
    InterceptTable( const WorldModel & world );

    /*!
      \brief destructor. nothing to do
    */
    virtual
    ~InterceptTable()
      { }

    /*!
      \brief recreate all interception info
    */
    void update();

    /*!
      \brief set teammate intercept info mainly by heard info
      \param unum uniform number
      \param step interception step
     */
    void hearTeammate( const int unum,
                       const int step );

    /*!
      \brief set opponent intercept info mainly by heard info
      \param unum uniform number
      \param step interception step
     */
    void hearOpponent( const int unum,
                       const int step );

    /*!
      \brief get minimal ball gettable step for self without stamina exhaust
      \return step value to get the ball
    */
    int selfReachCycle() const { return M_self_reach_step; }
    int selfReachStep() const { return M_self_reach_step; }

    /*!
      \brief get minimal ball gettable step for self with stamina exhaust
      \return step value to get the ball
    */
    int selfExhaustReachCycle() const { return M_self_exhaust_reach_step; }
    int selfExhaustReachStep() const { return M_self_exhaust_reach_step; }

    /*!
      \brief get minimal ball gettable step for teammate
      \return step value to get the ball
    */
    int teammateReachCycle() const { return M_teammate_reach_step; }
    int teammateReachStep() const { return M_teammate_reach_step; }

    /*!
      \brief get the ball access step for the second teammate
      \return step value to get the ball
    */
    int secondTeammateReachCycle() const { return M_second_teammate_reach_step; }
    int secondTeammateReachStep() const { return M_second_teammate_reach_step; }

    /*!
      \brief get the ball access step for the teammate goalie
      \return step value to get the ball
    */
    int goalieReachCycle() const { return M_goalie_reach_step; }
    int goalieReachStep() const { return M_goalie_reach_step; }

    /*!
      \brief get minimal ball gettable step for opponent
      \return step value to get the ball
    */
    int opponentReachCycle() const { return M_opponent_reach_step; }
    int opponentReachStep() const { return M_opponent_reach_step; }

    /*!
      \brief get the ball access step for the second opponent
      \return step value to get the ball
    */
    int secondOpponentReachCycle() const { return M_second_opponent_reach_step; }
    int secondOpponentReachStep() const { return M_second_opponent_reach_step; }

    /*!
      \brief get the teammate object fastest to the ball
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const PlayerObject * fastestTeammate() const { return M_fastest_teammate; }
    const PlayerObject * firstTeammate() const { return M_fastest_teammate; }

    /*!
      \brief get the teammate object second fastest to the ball
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const PlayerObject * secondTeammate() const { return M_second_teammate; }

    /*!
      \brief get the oppnent object fastest to the ball
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const PlayerObject * fastestOpponent() const { return M_fastest_opponent; }
    const PlayerObject * firstOpponent() const { return M_fastest_opponent; }

    /*!
      \brief get the oppnent object second fastest to the ball
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const PlayerObject * secondOpponent() const { return M_second_opponent; }

    /*!
      \brief get self interception cache container
      \return const reference to the interception info container
    */
    const std::vector< InterceptInfo > & selfCache() const
      {
          return M_self_cache;
      }

    /*!
      \brief get all players' intercept step container.
      \return map container. key: pointer, value: step value
     */
    const std::map< const AbstractPlayerObject *, int > & playerMap() const
      {
          return M_player_map;
      }

private:
    /*!
      \brief clear all cached data
    */
    void clear();

    /*!
      \brief create cache of future ball status
    */
    void createBallCache();

    /*!
      \brief predict self interception
    */
    void predictSelf();

    /*!
      \predict teammate interception
    */
    void predictTeammate();

    /*!
      \predict opponent interception
    */
    void predictOpponent();
};

}

#endif
