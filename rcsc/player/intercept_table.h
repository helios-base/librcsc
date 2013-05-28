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
  \brief interception data that contains required action count
*/
class InterceptInfo {
public:
    /*!
      \enum Mode
      \brief stamina consumption mode
     */
    enum Mode {
        NORMAL = 0, //!< ball gettable without stamina exhaust
        EXHAUST = 100, //!< fastest ball gettable, but recovery may be consumed.
    };

private:
    bool M_valid;
    Mode M_mode; //!< interception mode, NORMAL or EXHAUST
    int M_turn_cycle; //!< estimated turn cycles
    int M_dash_cycle; //!< estimated dash cycles
    double M_dash_power; //!< dash power
    double M_dash_dir; //!< first dash direction (relative to body)
    Vector2D M_self_pos; //!< final self position
    double M_ball_dist; //!< final squared ball distance
    double M_stamina; //!< final stamina value

public:

    /*!
      \brief create invalid info
     */
    InterceptInfo()
        : M_valid( false ),
          M_mode( EXHAUST ),
          M_turn_cycle( 10000 ),
          M_dash_cycle( 10000 ),
          M_dash_power( 100000.0 ),
          M_dash_dir( 0.0 ),
          M_self_pos( -10000.0, 0.0 ),
          M_ball_dist( 10000000.0 ),
          M_stamina( 0.0 )
      { }

    /*!
      \brief construct with all variables
    */
    InterceptInfo( const Mode mode,
                   const int turn_cycle,
                   const int dash_cycle,
                   const double dash_power,
                   const double dash_dir,
                   const Vector2D & self_pos,
                   const double ball_dist,
                   const double stamina )
        : M_valid( true ),
          M_mode( mode ),
          M_turn_cycle( turn_cycle ),
          M_dash_cycle( dash_cycle ),
          M_dash_power( dash_power ),
          M_dash_dir( dash_dir ),
          M_self_pos( self_pos ),
          M_ball_dist( ball_dist ),
          M_stamina( stamina )
      { }

    /*!
      \brief check if this object is legal one or not.
      \return checked result.
     */
    bool isValid() const
      {
          return M_valid;
      }

    /*!
      \brief set valie property.
      \param valid new value.
     */
    void setValid( const bool valid )
      {
          M_valid = valid;
      }

    /*!
      \brief get interception mode Id
      \return mode Id
    */
    Mode mode() const
      {
          return M_mode;
      }

    /*!
      \brief get estimated total turn cycles
      \return the number of turn cycles
    */
    int turnCycle() const
      {
          return M_turn_cycle;
      }

    /*!
      \brief get estimated total dash cycles
      \return the number of dash cycles
    */
    int dashCycle() const
      {
          return M_dash_cycle;
      }

    /*!
      \brief get esitimated total cycle to reach
      \return the number of total cycles
    */
    int reachCycle() const
      {
          return turnCycle() + dashCycle();
      }

    /*!
      \brief get dash power to be used
      \return dash power to be used
    */
    double dashPower() const
      {
          return M_dash_power;
      }

    /*!
      \brief ger first dash direction
      \return first dash direction
     */
    double dashDir() const
      {
          return M_dash_dir;
      }

    /*!
      \brief get the final self position
      \return final self position
     */
    const Vector2D & selfPos() const
      {
          return M_self_pos;
      }

    /*!
      \brief get the final squared ball distance
      \return final squared ball distance
     */
    double ballDist() const
      {
          return M_ball_dist;
      }

    /*!
      \brief get the final self stamina value
      \return final self stamina value
     */
    double stamina() const
      {
          return M_stamina;
      }

    /*!
      \class Cmp
      \brief interception info compare function object
    */
    struct Cmp {
        /*!
          \brief operator function
          \param lhs left hand side variable
          \param rhs right hand side variable
          \return compared result
        */
        bool operator()( const InterceptInfo & lhs,
                         const InterceptInfo & rhs ) const
          {
              return ( lhs.reachCycle() < rhs.reachCycle()
                       ? true
                       : lhs.reachCycle() == rhs.reachCycle()
                       ? lhs.turnCycle() < rhs.turnCycle()
                       : false );
          }
    };
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

    //! predicted min reach cycle for self without stamina exhaust
    int M_self_reach_cycle;
    //! predicted min reach cycle for self with stamina exhaust
    int M_self_exhaust_reach_cycle;
    //! predicted min reach cycle for teammate
    int M_teammate_reach_cycle;
    //! predicted reach cycle for second fastest teammate
    int M_second_teammate_reach_cycle;
    //! predicted min reach cycle for teammate goalie
    int M_goalie_reach_cycle;
    //! predicted min reach cycle for opponent
    int M_opponent_reach_cycle;
    //! predicted reach cycle for second fastest opponent
    int M_second_opponent_reach_cycle;

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
      \param cycle interception cycle
     */
    void hearTeammate( const int unum,
                       const int cycle );

    /*!
      \brief set opponent intercept info mainly by heard info
      \param unum uniform number
      \param cycle interception cycle
     */
    void hearOpponent( const int unum,
                       const int cycle );

    /*!
      \brief get minimal ball gettable cycle for self without stamina exhaust
      \return cycle value to get the ball
    */
    int selfReachCycle() const { return M_self_reach_cycle; }
    int selfReachStep() const { return M_self_reach_cycle; }

    /*!
      \brief get minimal ball gettable cycle for self with stamina exhaust
      \return cycle value to get the ball
    */
    int selfExhaustReachCycle() const { return M_self_exhaust_reach_cycle; }
    int selfExhaustReachStep() const { return M_self_exhaust_reach_cycle; }

    /*!
      \brief get minimal ball gettable cycle for teammate
      \return cycle value to get the ball
    */
    int teammateReachCycle() const { return M_teammate_reach_cycle; }
    int teammateReachStep() const { return M_teammate_reach_cycle; }

    /*!
      \brief get the ball access cycle for the second teammate
      \return cycle value to get the ball
    */
    int secondTeammateReachCycle() const { return M_second_teammate_reach_cycle; }
    int secondTeammateReachStep() const { return M_second_teammate_reach_cycle; }

    /*!
      \brief get the ball access cycle for the teammate goalie
      \return cycle value to get the ball
    */
    int goalieReachCycle() const { return M_goalie_reach_cycle; }
    int goalieReachStep() const { return M_goalie_reach_cycle; }

    /*!
      \brief get minimal ball gettable cycle for opponent
      \return cycle value to get the ball
    */
    int opponentReachCycle() const { return M_opponent_reach_cycle; }
    int opponentReachStep() const { return M_opponent_reach_cycle; }

    /*!
      \brief get the ball access cycle for the second opponent
      \return cycle value to get the ball
    */
    int secondOpponentReachCycle() const { return M_second_opponent_reach_cycle; }
    int secondOpponentReachStep() const { return M_second_opponent_reach_cycle; }

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
