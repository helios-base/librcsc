// -*-c++-*-

/*!
  \file intercept_simulator_player.h
  \brief intercept simulator for other players Header File
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

#ifndef RCSC_PLAYER_INTERCEPT_SIMULATOR_PLAYER_H
#define RCSC_PLAYER_INTERCEPT_SIMULATOR_PLAYER_H

#include <rcsc/geom/vector_2d.h>
#include <vector>

namespace rcsc {

class PlayerType;
class BallObject;
class PlayerObject;
class WorldModel;

/*!
  \class InterceptSimulatorPlayer
  \brief intercept simulator for other players
*/
class InterceptSimulatorPlayer {
private:

    /*!
      \struct PlayerData
      \brief player data
    */
    struct PlayerData {
        const PlayerObject & player_; //!< player reference
        const PlayerType & ptype_; //!< player type
        const Vector2D pos_; //!< initial pos
        const Vector2D vel_; //!< initial vel
        const double control_area_; //!< kickable or catchable area
        const int bonus_step_; //!< bonus step (position accuracy count)
        const int penalty_step_; //!< penalty step (tackling)

        PlayerData( const PlayerObject & player,
                    const PlayerType & ptype,
                    const Vector2D & pos,
                    const Vector2D & vel,
                    const double control_area,
                    const int bonus_step,
                    const int penalty_step )
            : player_( player ),
              ptype_( ptype ),
              pos_( pos ),
              vel_( vel ),
              control_area_( control_area ),
              bonus_step_( bonus_step ),
              penalty_step_( penalty_step )
        { }

        Vector2D inertiaPoint( const int step ) const;

    };


    //! predicted ball positions
    std::vector< Vector2D > M_ball_cache;
    //! ball velocity angle
    const AngleDeg M_ball_move_angle;

    // not used
    InterceptSimulatorPlayer() = delete;

public:

    /*!
      \brief construct with all variables.
      \param ball_pos initial ball position
      \param ball_vel initial ball velocity
    */
    InterceptSimulatorPlayer( const Vector2D & ball_pos,
                              const Vector2D & ball_vel );

    /*!
      \brief destructor. nothing to do
    */
    ~InterceptSimulatorPlayer()
    { }

    //////////////////////////////////////////////////////////
    /*!
      \brief get predicted ball gettable cycle
      \param wm const reference to the instance of world model
      \param player const reference to the player object
      \param goalie goalie mode or not
      \param max_cycle max predict cycle. estimation loop is limited to this value.
      \return predicted cycle value
    */
    int simulate( const WorldModel & wm,
                  const PlayerObject & player,
                  const bool goalie ) const;

private:

    /*!
      \brief create predicted ball positions
      \param ball_pos initial ball position
      \param ball_vel initial ball velocity
     */
    void createBallCache( const Vector2D & ball_pos,
                          const Vector2D & ball_vel );

    /*!
      \brief estimate minimum reach step (very rough calculation)
      \param ptype player type
      \param control_area kickable/catchable area
      \param player_pos player's initial position
    */
    int estimateMinStep( const PlayerData & data ) const;


    /*!
      \brief check if player can reach after turn & dash within total_step
      \param player const reference to the player object
      \param control_area player's kickable/catchable area
      \param ball_pos ball position after total_step
      \param total_step total time step
      \param bonus_step bonus time step for the target player
      \param penalty_step penalty time step for the target player
      \return true if player can get the ball
    */
    bool canReachAfterTurnDash( const PlayerData & data,
                                const Vector2D & ball_pos,
                                const int total_step ) const;

    /*!
      \brief predict required cycle to face to the ball position
      \param cycle we consder the status 'cycle' cycles later
      \param player const reference to the player object
      \param player_type player type parameter
      \param control_area player's ball controllable radius
      \param ball_pos ball position 'cycle' cycles later
      \return predicted cycle value
    */
    int predictTurnCycle( const PlayerData & data,
                          const Vector2D & ball_pos,
                          const int total_step ) const;

    /*!
      \brief check if player can reach by n_dash dashes
      \param n_turn the number of tunes to be used
      \param n_dash the number of dashes to be used
      \param player const reference to the player object
      \param player_type player type parameter
      \param control_area player's ball controllable radius
      \param ball_pos ball position 'cycle' cycles later
      \return true if player can get the ball
    */
    bool canReachAfterDash( const PlayerData & data,
                            const Vector2D & ball_pos,
                            const int total_step,
                            const int n_turn ) const;

    /*!
      \brief predict player's reachable cycle to the ball final point
      \param player const reference to the player object
      \param player_type player type parameter
      \return predicted cycle value
    */
    int predictFinal( const PlayerData & data ) const;

};

}

#endif
