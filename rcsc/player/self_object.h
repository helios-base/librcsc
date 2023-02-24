// -*-c++-*-

/*!
  \file self_object.h
  \brief self object class Header File
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

#ifndef RCSC_PLAYER_SELF_OBJECT_H
#define RCSC_PLAYER_SELF_OBJECT_H

#include <rcsc/player/abstract_player_object.h>
#include <rcsc/player/view_mode.h>
#include <rcsc/player/fullstate_sensor.h>

#include <rcsc/common/stamina_model.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

namespace rcsc {

class BodySensor;
class ActionEffector;
class BallObject;

//! self object class
class SelfObject
    : public AbstractPlayerObject {
private:
    //! validation count threshold value for M_pos
    static int S_pos_count_thr;
    //! validation count threshold value for M_vel
    static int S_vel_count_thr;
    //! validation count threshold value for M_body and M_face
    static int S_face_count_thr;

    //! latest update time
    GameTime M_time;
    //! time updated by sense_body
    GameTime M_sense_body_time;


    // status on field
    Vector2D M_pos_error; //!< estimated error of global position
    Vector2D M_pos_prev; //!< global position at previous cycle

    Vector2D M_vel_error; //!< estimated error of velocity

    AngleDeg M_neck; //!< relative neck angle
    double M_face_error; //!< estimated error of angle info

    ViewWidth M_view_width; //!< current view width type
    ViewQuality M_view_quality; //!< current view quality type

    double M_focus_dist; //!< the distance to the current focus point
    AngleDeg M_focus_dir; //!< the direction to the current focus point

    StaminaModel M_stamina; //!< current stamina info

    // some action effect
    GameTime M_last_catch_time; //!< last goalie catch time

    int M_tackle_expires; //!< tackle expires. if positive, agent cannot perfome any body action.
    int M_charged_expires; //!< foul charged expires. if positive, agent cannot perfome any body action.

    int M_arm_movable; //!< cycles till arm movable
    int M_arm_expires; //!< cycles till arm stops pointing
    Vector2D M_pointto_rpos; //!< estimated pointing position relative to the neck
    Vector2D M_pointto_pos; //!< estimated pointing position
    GameTime M_last_pointto_time; //!< action performed time

    SideID M_attentionto_side; //!< attentioned player's side
    int M_attentionto_unum; //!< attentioned player's uniform number

    //! last move vector (transformation from previous to current)
    Vector2D M_last_move;
    //! last 3 move vectors
    Vector2D  M_last_moves[4];

    bool M_collision_estimated; //!< collision estimation result
    bool M_collides_with_none; //!< true if agent does not collide
    bool M_collides_with_ball; //!< true if collided with ball
    bool M_collides_with_player; //!< true if collided with player
    bool M_collides_with_post; //!< true if collided with post

    // status values for action
    bool M_kickable; //!< ball kickable flag
    double M_kick_rate; //!< current kick power rate
    double M_catch_probability; //!< estimated catch success probability
    double M_tackle_probability; //!< estimated tackle success probability
    double M_foul_probability; //!< estimated tackle success probability

    //! not used
    SelfObject( const SelfObject & self ) = delete;
    //! not used
    SelfObject & operator=( const SelfObject & self ) = delete;

public:

    /*!
      \brief constructor. just initialize member variables
    */
    SelfObject();
    /*!
      \brief destructor. nothing to do
    */
    ~SelfObject() = default;

    /*!
      \brief set accuracy count threshold values.
      \param pos_thr threshold value for M_pos
      \param vel_thr threshold value for M_vel
      \param face_thr threshold value for M_body and M_face
    */
    static
    void set_count_thr( const int pos_thr,
                        const int vel_thr,
                        const int face_thr );

    /*!
      \brief set uniform number and goalie flag.
      \param side side id returned from server
      \param unum uniform number returned from server
      \param goalie true if agent connected as goalie.

      This method is called just after received init replay message.
    */
    void init( const SideID side,
               const int unum,
               const bool goalie );

   /*!
      \brief check if this player is self or not
      \return true if this player is self
     */
    virtual
    bool isSelf() const
      {
          return true;
      }

    /*!
      \brief update player type id
      \param type new player type id
     */
    virtual
    void setPlayerType( const int type );

    /*!
      \brief get player type parameter
      \return const reference to the player type object
    */
    const PlayerType & playerType() const
      {
          return *M_player_type;
      }

    /*!
      \brief velify global position accuracy
      \return true if position has enough accuracy
    */
    bool posValid() const;

    /*!
      \brief verify velocity accuracy
      \return true if velocity has enough accuracy
    */
    bool velValid() const;

    /*!
      \brief verify angle accuracy
      \return true if angle has enough accuracy
    */
    bool faceValid() const;


    /*!
      \brief get estimated error of agent's global position
      \return const reference to the error vector
    */
    const Vector2D & posError() const
      {
          return M_pos_error;
      }

    /*!
      \brief get global position at previous cycle
      \return const reference to the position object
    */
    const Vector2D & posPrev() const
      {
          return M_pos_prev;
      }

    /*!
      \brief get estimated speed
      \return scalar value of current velocity
    */
    double speed() const
      {
          return M_vel.r();
      }

    /*!
      \brief get estimated error of velocity
      \return const reference to the error vector object
    */
    const Vector2D & velError() const
      {
          return M_vel_error;
      }

    /*!
      \brief get estimated neck angle relative to body angle
      \return const reference to the angle object
    */
    const AngleDeg & neck() const
      {
          return M_neck;
      }

    /*!
      \brief get estimated error about angle
      \return error value about angle
    */
    const double & faceError() const
      {
          return M_face_error;
      }

    /*!
      \brief get current view width
      \return const reference to the view width object
    */
    const ViewWidth & viewWidth() const
      {
          return M_view_width;
      }

    /*!
      \brief get current view quality
      \return const reference to the view quality object
    */
    const ViewQuality & viewQuality() const
      {
          return M_view_quality;
      }

    /*!
      \brief get the focus distance updated by the sense_body message
      \return the distance to the focus point
     */
    double focusDist() const
      {
          return M_focus_dist;
      }

    /*!
      \brief get the focus direction updated by the sense_body message
      \return the value of focus direction
      \return the direction to the focus point, relative to the body angle
     */

    const AngleDeg & focusDir() const
      {
          return M_focus_dir;
      }

    /*!
      \brief get the estimated focus point according to the self localization result
      \return the global coordinates of the estimated focus point
     */
    Vector2D focusPoint() const;

    /*!
      \brief get time when the last catch command is performed
      \return const reference to the time object
    */
    const GameTime & catchTime() const
      {
          return M_last_catch_time;
      }

    /*!
      \brief get tackle expire count
      \return tackle expire count
    */
    int tackleExpires() const
      {
          return M_tackle_expires;
      }

    int chargedExpires() const
      {
          return M_charged_expires;
      }

    /*!
      \brief check if body is freezed by tackle effect
      \return true if body is freezed.
    */
    bool isFrozen() const
      {
          return M_tackle_expires > 0
              || M_charged_expires > 0;
      }

    /*!
      \brief check if player is tackling or not.
      \return checked result.
     */
    bool isTackling() const
      {
          return M_tackle_expires > 0;
      }

    /*!
      \brief get arm movable count. if this value equals 0, agent can perform pointto command.
      \return cycles till arm is movable
    */
    int armMovable() const
      {
          return M_arm_movable;
      }

    /*!
      \brief get am expires count. if this value equals 0, agent is not pointing anywhere.
      \return cycles till arm stops pointing.
     */
    int armExpires() const
      {
          return M_arm_expires;
      }

    /*!
      \brief get estimated pointing point
      \brief const reference to the point object
    */
    const Vector2D & pointtoPos() const
      {
          return M_pointto_pos;
      }

    /*!
      \brief get pointto action performed time
      \brief const reference to the time object
    */
    const GameTime & pointtoTime() const
      {
          return M_last_pointto_time;
      }

    /*!
      \brief get current attentioned player's side
      \return side Id
    */
    SideID attentiontoSide() const
      {
          return M_attentionto_side;
      }

    /*!
      \brief get current attentioned player's uniform number
      \return uniform number
    */
    int attentiontoUnum() const
      {
          return M_attentionto_unum;
      }

    /*!
      \brief get current stamina model.
      \return stamina model object.
     */
    const StaminaModel & staminaModel() const
      {
          return M_stamina;
      }

    /*!
      \brief get current stamina value
      \return raw stamina value from stamina object
    */
    double stamina() const
      {
          return M_stamina.stamina();
      }

    /*!
      \brief get current stamina capacity value
      \return raw stamina capacity value from stamina object
    */
    double staminaCapacity() const
      {
          return M_stamina.capacity();
      }

    /*!
      \brief get current effort value
      \return effort value from stamina object
    */
    double effort() const
      {
          return M_stamina.effort();
      }

    /*!
      \brief get current estimated recovery value
      \return recovery value from stamina object
    */
    double recovery() const
      {
          return M_stamina.recovery();
      }

    /*!
      \brief get estimated collistion status
      \return true collision is estimated
    */
    bool collisionEstimated() const
      {
          return M_collision_estimated;
      }

    /*!
      \brief get a flag whether agent has a certain collistion info
      \return true if agent has a certain collision info
    */
    bool hasSensedCollision() const
      {
          return M_collides_with_none
              || M_collides_with_ball
              || M_collides_with_player
              || M_collides_with_post;
      }

    /*!
      \brief get a certain collistion status with no objects
      \return true if no collision
    */
    bool collidesWithNone() const
      {
          return M_collides_with_none;
      }

    /*!
      \brief get a certain collistion status with ball
      \return true collision occured
    */
    bool collidesWithBall() const
      {
          return M_collides_with_ball;
      }

    /*!
      \brief get a certain collistion status with players
      \return true collision occured
    */
    bool collidesWithPlayer() const
      {
          return M_collides_with_player;
      }

    /*!
      \brief get a certain collistion status with posts
      \return true collision occured
    */
    bool collidesWithPost() const
      {
          return M_collides_with_post;
      }

    /*!
      \brief get last move vector (transformation from previous to current)
      \return const reference to the vector object
    */
    const Vector2D & lastMove() const
      {
          return M_last_move;
      }

    /*!
      \brief get last 3 move vectors
      \return const reference to the vector object
    */
    const Vector2D & lastMove( const int i ) const
      {
          if ( i < 0 || 2 < i ) return M_last_moves[3];
          return M_last_moves[i];
      }

    /*!
      \brief get ball kickable status
      \return true if ball kickable
    */
    bool isKickable() const
      {
          return M_kickable;
      }

    /*!
      \brief get current estimated kick power rate
      \return calculated kick rate value
    */
    double kickRate() const
      {
          return M_kick_rate;
      }

    /*!
      \brief get current dash power rate
      \return current effort * dash power rate
    */
    double dashRate() const
      {
          return effort() * playerType().dashPowerRate();
      }

    /*!
      \brief get estimated catch success probability
      \return probability [0,1]
    */
    double catchProbability() const
      {
          return M_catch_probability;
      }

    /*!
      \brief get estimated tackle success probability
      \return probability [0,1]
    */
    double tackleProbability() const
      {
          return M_tackle_probability;
      }

    /*!
      \brief get estimated foul success probability
      \return probability [0,1]
    */
    double foulProbability() const
      {
          return M_foul_probability;
      }


    ///////////////////////////////////////////////////////////////

    /*!
      \brief update status only using internal info
      \param act internal action info
      \param current current game time
    */
    void update( const ActionEffector & act,
                 const GameTime & current );

private:
    /*!
      \brief apply wind effect

      This method is called from update()
    */
    void updateWindEffect();

public:
    /*!
      \brief update status using sense_body info
      \param sense sense_body info
      \param act internal action info
      \param current current game time
    */
    void updateAfterSenseBody( const BodySensor & sense,
                               const ActionEffector & act,
                               const GameTime & current );

    /*!
      \brief update status using fullstate info
      \param my_state my state in fullstate info
      \param act internal action info
      \param current current game time
     */
    void updateAfterFullstate( const FullstateSensor::PlayerT & my_state,
                               const ActionEffector & act,
                               const GameTime & current );

    /*!
      \brief update status using see info
      \param pos estimated global position
      \param pos_err estimated error of position
      \param face estimated global neck angle
      \param face_err estimated error of facing angle
      \param current current game time
    */
    void updatePosBySee( const Vector2D & pos,
                         const Vector2D & pos_err,
                         const double & face,
                         const double & face_err,
                         const GameTime & current );

    /*!
      \brief apply collision effect
      \param pos new global position
      \param pos_error estimated maximum position error
     */
    void updateByCollision( const Vector2D & pos,
                            const Vector2D & pos_error );

    /*!
      \brief update status using see info. but only angle.
      \param face estimated global neck angle
      \param face_err estimated error of facing angle
      \param current current game time
    */
    void updateAngleBySee( const double & face,
                           const double & face_err,
                           const GameTime & current );

    /*!
      \brief adjust velocity direction using observed facing angle and sensed speed
      \param sense sense_body info
      \param current current game time

      *** Call before ball update with see info. ***
      This method is called just after self localization in WorldModel
    */
    void updateVelDirAfterSee( const BodySensor & sense,
                               const GameTime & current );

    /*!
      \brief update ball related status
      \param ball ball object

      This method is called just before action decision
    */
    void updateBallInfo( const BallObject & ball );

    /*!
      \brief update ball kickable state
      \param ball ball object
      \param self_reach_cycle estimated ball interception step.
      \param teammate_reach_cycle estimated ball interception step.
      \param opponent_reach_cycle estimated ball interception step.

      This method is called just before action decision
    */
    void updateKickableState( const BallObject & ball,
                              const int self_reach_cycle,
                              const int teammate_reach_cycle,
                              const int opponent_reach_cycle );

    /////////////////////////////////////////////////////////////
    // used for direct update

    /*!
      \brief set view mode
      \param w view width object
      \param q view quality object

      This method is called just after send command.
    */
    void setViewMode( const ViewWidth & w,
                      const ViewQuality & q )
      {
          M_view_width = w;
          M_view_quality = q;
      }

    /*!
      \brief set pointto effect
      \param point pointing point
      \param done_time action performed time

      This method is called just after send command.
    */
    void setPointto( const Vector2D & point,
                     const GameTime & done_time )
      {
          M_pointto_pos = point;
          M_last_pointto_time = done_time;
          if ( pos().isValid() )
          {
              M_pointto_angle = ( point - pos() ).th();
              M_pointto_count = 0;
          }
      }

    /*!
      \brief set attentionto effect
      \param side attentioned player's side
      \param unum attentioned player's uniform number

      This method is called just after send command.
    */
    void setAttentionto( const SideID side,
                         const int unum )
      {
          M_attentionto_side = side;
          M_attentionto_unum = unum;
      }

    ///////////////////////////////////////////////////////////////
    // utilities

    /*!
      \brief get dash power to save recovery decay.
      \param dash_power evaluated dash power
      \return filtered dash power
    */
    double getSafetyDashPower( const double & dash_power ) const
      {
          return staminaModel().getSafetyDashPower( playerType(), dash_power );
      }

};

}

#endif
