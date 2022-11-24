// -*-c++-*-

/*!
  \file self_intercept_v13.h
  \brief self intercept predictor for rcssserver v13+ Header File
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

#ifndef RCSC_PLAYER_SELF_INTERCEPT_V13_H
#define RCSC_PLAYER_SELF_INTERCEPT_V13_H

#include <rcsc/player/intercept_table.h>
#include <rcsc/common/stamina_model.h>
#include <rcsc/geom/vector_2d.h>
#include <vector>

namespace rcsc {

class BallObject;
class SelfObject;
class WorldModel;

/*!
  \class SelfInterceptV13
  \brief self intercept predictor for rcssserver v13+
*/
class SelfInterceptV13 {
public:
    //! max short step
    static const int MAX_SHORT_STEP;
    //! turn margin threshold
    static const double MIN_TURN_THR;
    //! back dasn angle diff threshold
    static const double BACK_DASH_THR_ANGLE;

private:
    //! const reference to the WorldModel instance
    const WorldModel & M_world;

    // noncopyable
    SelfInterceptV13();
    SelfInterceptV13( const SelfInterceptV13 & );
    SelfInterceptV13 & operator=( const SelfInterceptV13 & );
public:
    /*!
      \brief constructor
      \param world const reference to the WorldModel instance
    */
    explicit
    SelfInterceptV13( const WorldModel & world )
        : M_world( world )
      { }

    //////////////////////////////////////////////////////////
    /*!
      \brief predict self interception, and store the resules to self_cache
      \param max_cycle max estimation cycle
      \param self_cache reference to the interception info container
      to store the result
    */
    void predict( const int max_cycle,
                  std::vector< InterceptInfo > & self_cache ) const;


private:
    /////////////////////////////////////////////////////
    // one dash

    /*!
      \brief predict one step action result
      \param self_cache reference to the cahce variable to store the result
    */
    void predictOneStep( std::vector< InterceptInfo > & self_cache ) const;

    /*!
      \brief check if player can get the ball WITHOUT dash
      \param self_cache reference to the cahce variable to store the result
      \return true if player can get the ball without dash
    */
    bool predictNoDash( std::vector< InterceptInfo > & self_cache ) const;

    /*!
      \brief predict one dash result.
      \param self_cache reference to the cahce variable to store the result

      This method assumes that Y difference of ball next position is within
      control area
    */
    void predictOneDash( std::vector< InterceptInfo > & self_cache ) const;


    /*!

    */
    bool predictOneDashAdjust( const AngleDeg & dash_angle,
                               const Vector2D & forward_accel,
                               const Vector2D & back_accel,
                               const double & control_area,
                               InterceptInfo * info ) const;

    /*!
      \brief estimate best dash power to trap the ball at the best point
      \param next_ball_rel ball cooridnates relative to next player pos
      and player's body
      \param max_back_accel_x available player's accel backword
      \param max_back_accel_x available player's accel forward
      \return dash power

      assume that when this method is called, at least player can put
      the ball on his side by one dash accel
    */
    double getOneStepDashPower( const Vector2D & next_ball_rel,
                                const AngleDeg & dash_angle,
                                const double & max_forward_accel_x,
                                const double & max_back_accel_x ) const;

    /////////////////////////////////////////////////////
    // short step prediction
    /*!
      \brief predict short term interception
      \param max_cycle max estimation cycle
      \param save_recovery if true, player keeps his recovery.
      \param self_cache reference to the cahce variable to store the result
    */
    void predictShortStep( const int max_cycle,
                           const bool save_recovery,
                           std::vector< InterceptInfo > & self_cache ) const;
    void predictTurnDashShort( const int cycle,
                               const Vector2D & ball_pos,
                               const double & control_area,
                               const bool save_recovery,
                               const bool back_dash,
                               const double & turn_margin_control_area,
                               std::vector< InterceptInfo > & self_cache ) const;
    int predictTurnCycleShort( const int cycle,
                               const Vector2D & ball_pos,
                               const double & control_area,
                               const bool back_dash,
                               const double & turn_margin_control_area,
                               AngleDeg * result_dash_angle ) const;
    void predictDashCycleShort( const int cycle,
                                const int n_turn,
                                const Vector2D & ball_pos,
                                const AngleDeg & dash_angle,
                                const double & control_area,
                                const bool save_recovery,
                                const bool back_dash,
                                std::vector< InterceptInfo > & self_cache ) const;
    void predictOmniDashShort( const int cycle,
                               const Vector2D & ball_pos,
                               const double & control_area,
                               const bool save_recovery,
                               const bool back_dash,
                               std::vector< InterceptInfo > & self_cache ) const;
    int predictAdjustOmniDash( const int cycle,
                               const Vector2D & ball_pos,
                               const double & control_area,
                               const bool save_recovery,
                               const bool back_dash,
                               const double & dash_rel_dir,
                               Vector2D * my_pos,
                               Vector2D * my_vel,
                               StaminaModel * stamina_model,
                               double * first_dash_power ) const;

    /////////////////////////////////////////////////////
    // long step prediction

    /*!
      \brief predict long term interception
      \param max_cycle max estimation cycle
      \param save_recovery if true, player keeps his recovery.
      \param self_cache reference to the cahce variable to store the result
    */
    void predictLongStep( const int max_cycle,
                          const bool save_recovery,
                          std::vector< InterceptInfo > & self_cache ) const;

    /*!
      \brief predict reachable cycle to the ball final point
      \param max_cycle max estimation cycle (cycles till ball stops)
      \param self_cache reference to the cahce variable to store the result
    */
    void predictFinal( const int max_cycle,
                       std::vector< InterceptInfo > & self_cache ) const;

    /*!
      \brief check if player can get the ball after cycle
      \param cycle we consder the status 'cycle' cycles later
      \param ball_pos future ball position
      \param control_area player's ball controllable radius
      \param save_recovery if true, player keeps his recovery.
      \paran n_turn variable pointer to store the result turn cycles
      \param back_dash variable pointer to store the back dash mode or not
      \param result_recovery pointer to the variable to store the recovery value after dashes
      \return true if player can get the ball
    */
    bool canReachAfterTurnDash( const int cycle,
                                const Vector2D & ball_pos,
                                const double & control_area,
                                const bool save_recovery,
                                int * n_turn,
                                bool * back_dash,
                                double * result_recovery,
                                std::vector< InterceptInfo > & self_cache ) const;

    /*!
      \brief predict required cycle to face to the ball position
      \param cycle we consder the status 'cycle' cycles later
      \param ball_pos future ball position
      \param control_area player's ball controllable radius
      \param dash_angle variable pointer to store the result dash angle
      \param back_dash variable pointer to store the back dash mode or not
      \return predicted cycle value
    */
    int predictTurnCycle( const int cycle,
                          const Vector2D & ball_pos,
                          const double & control_area,
                          AngleDeg * dash_angle,
                          bool * back_dash ) const;

    /*!
      \brief check if player can chase the ball by back dash or not
      \param cycle we consder the status 'cycle' cycles later
      \param target_dist distance to dash
      \param angle_diff angle difference from body angle to target point angle
      \return true if player has back dash chance
    */
    bool canBackDashChase( const int cycle,
                           const double & target_dist,
                           const double & angle_diff ) const;

    /*!
      \brief check if player can get the ball
      \param n_turn the number of tunes to be used
      \param n_dash the number of dashes to be used
      \param ball_pos future ball position
      \param control_area player's ball controllable radius
      \param save_recovery if true, player keeps his recovery.
      \param dash_angle dash accel direction
      \param back_dash if true, player try dash backword
      \param result_recovery pointer to the variable to store the recovery value after dashes
      \return true if player can get the ball
    */
    bool canReachAfterDash( const int n_turn,
                            const int n_dash,
                            const Vector2D & ball_pos,
                            const double & control_area,
                            const bool save_recovery,
                            const AngleDeg & dash_angle,
                            const bool back_dash,
                            double * result_recovery,
                            std::vector< InterceptInfo > & self_cache ) const;

    //
    //
    //

    void predictTurnDashLong( const int cycle,
                              const Vector2D & ball_pos,
                              const double & control_area,
                              const bool save_recovery,
                              const bool back_dash,
                              std::vector< InterceptInfo > & self_cache ) const;

    int predictTurnCycleLong( const int cycle,
                              const Vector2D & ball_pos,
                              const double & control_area,
                              const bool back_dash,
                              AngleDeg * result_dash_angle ) const;

    void predictDashCycleLong( const int cycle,
                               const int n_turn,
                               const Vector2D & ball_pos,
                               const AngleDeg & dash_angle,
                               const double & control_area,
                               const bool save_recovery,
                               const bool back_dash,
                               std::vector< InterceptInfo > & self_cache ) const;
};

}

#endif
