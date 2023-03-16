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

#include <rcsc/player/intercept.h>
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
  \class InterceptTable
  \brief interception info holder for all players
*/
class InterceptTable {
private:

    //! last updated time
    GameTime M_update_time;

    //! predicted min reach step for self without stamina exhaust
    int M_self_step;
    //! predicted min reach step for self with stamina exhaust
    int M_self_exhaust_step;
    //! predicted min reach step for teammate
    int M_teammate_step;
    //! predicted reach step for second fastest teammate
    int M_second_teammate_step;
    //! predicted min reach step for teammate goalie
    int M_our_goalie_step;
    //! predicted min reach step for opponent
    int M_opponent_step;
    //! predicted reach step for second fastest opponent
    int M_second_opponent_step;

    //! const pointer to the fastest ball gettable teammate player object
    const PlayerObject * M_first_teammate;
    //! const pointer to the second fastest ball gettable teammate player object
    const PlayerObject * M_second_teammate;
    //! const pointer to the fastest ball gettable opponent player object
    const PlayerObject * M_first_opponent;
    //! const pointer to the second fastest ball gettable opponent player object
    const PlayerObject * M_second_opponent;

    //! interception info cache for smart interception
    std::vector< Intercept > M_self_results;

    //! all players' intercept step container. key: pointer, value: step value
    std::map< const AbstractPlayerObject *, int > M_player_map;

    // not used
    InterceptTable( const InterceptTable & ) = delete;
    InterceptTable & operator=( const InterceptTable & ) = delete;

public:
    /*!
      \brief init member variables, reserve cache vector memory
    */
    InterceptTable();

    /*!
      \brief destructor. nothing to do
    */
    virtual
    ~InterceptTable()
      { }

    /*!
      \brief update table information
      \param wm const reference to the world model
    */
    void update( const WorldModel & wm );

    /*!
      \brief update teammate intercept information by heard information
      \param wm const reference to the world model
      \param unum uniform number
      \param step interception step
     */
    void hearTeammate( const WorldModel & wm,
                       const int unum,
                       const int step );

    /*!
      \brief update opponent intercept information by heard information
      \param wm const reference to the world model
      \param unum uniform number
      \param step interception step
     */
    void hearOpponent( const WorldModel & wm,
                       const int unum,
                       const int step );

    /*!
      \brief get minimal ball gettable step for self without stamina exhaust
      \return step value to get the ball
    */
    int selfStep() const { return M_self_step; }

    /*!
      \brief get minimal ball gettable step for self with stamina exhaust
      \return step value to get the ball
    */
    int selfExhaustStep() const { return M_self_exhaust_step; }

    /*!
      \brief get minimal ball gettable step for teammate
      \return step value to get the ball
    */
    int teammateStep() const { return M_teammate_step; }

    /*!
      \brief get the ball access step for the second teammate
      \return step value to get the ball
    */
    int secondTeammateStep() const { return M_second_teammate_step; }

    /*!
      \brief get the ball access step for the teammate goalie
      \return step value to get the ball
    */
    int ourGoalieStep() const { return M_our_goalie_step; }

    /*!
      \brief get minimal ball gettable step for opponent
      \return step value to get the ball
    */
    int opponentStep() const { return M_opponent_step; }

    /*!
      \brief get the ball access step for the second opponent
      \return step value to get the ball
    */
    int secondOpponentStep() const { return M_second_opponent_step; }

    /*!
      \brief get the teammate object fastest to the ball
      \return const pointer to the PlayerObject.
      if not exist such a player, return NULL
    */
    const PlayerObject * firstTeammate() const { return M_first_teammate; }

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
    const PlayerObject * firstOpponent() const { return M_first_opponent; }

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
    const std::vector< Intercept > & selfResults() const
    {
        return M_self_results;
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
      \brief predict self interception
      \param wm const reference to the world model
    */
    void predictSelf( const WorldModel & wm );

    /*!
      \predict teammate interception
      \param wm const reference to the world model
    */
    void predictTeammate( const WorldModel & wm );

    /*!
      \predict opponent interception
      \param wm const reference to the world model
    */
    void predictOpponent( const WorldModel & wm );
};

}

#endif
