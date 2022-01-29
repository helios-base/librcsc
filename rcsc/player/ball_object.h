// -*-c++-*-

/*!
  \file ball_object.h
  \brief ball object class Header File
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

#ifndef RCSC_PLAYER_BALL_OBJECT_H
#define RCSC_PLAYER_BALL_OBJECT_H

#include <rcsc/common/server_param.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/soccer_math.h>
#include <rcsc/math_util.h>

#include <list>

namespace rcsc {

class GameMode;
class ActionEffector;
class SelfObject;

/*!
  \class BallObject
  \brief observed ball object class

  order of update process.
  update()
  -> updateAll() (updatePos(),updateOnlyVel(), updateOnlyRelativePos())
  -> updateByHeardInfo()
  -> updateByCollision()
  -> updateByGameMode()
  -> updateSelfRelated()
*/
class BallObject {
private:
    //! validation count threshold value for M_pos
    static int S_pos_count_thr;
    //! validation count threshold value for M_rpos
    static int S_rpos_count_thr;
    //! validation count threshold value for M_vel
    static int S_vel_count_thr;

    Vector2D M_pos; //!< estimated global position
    Vector2D M_pos_error; //!< estimated error of global position
    int M_pos_count; //!< cycle count since the last observation

    Vector2D M_rpos; //!< estimated relative  position
    Vector2D M_rpos_error; //!< estimated error fo relative position
    int M_rpos_count; //!< cycle count since the last observation

    Vector2D M_seen_pos; //!< seen global position
    Vector2D M_seen_rpos; //!< seen relative position
    int M_seen_pos_count; //!< cycle count since the last see update

    Vector2D M_heard_pos; //!< heard global position
    int M_heard_pos_count; //!< cycle count since the last hear update

    Vector2D M_vel; //!< estimated velocity
    Vector2D M_vel_error; //!< estimated error of velocity
    int M_vel_count; //!< cycle count since the last observation

    Vector2D M_seen_vel; //!< seen velocity
    int M_seen_vel_count; //!< cycle count since the last see update

    Vector2D M_heard_vel; //!< heard velocity
    int M_heard_vel_count; //!< cycle count since the last hear update

    int M_lost_count; //!< cycle count since the ball lost detection

    int M_ghost_count; //!< ghost flag

    double M_dist_from_self; //!< estimated distance from self
    AngleDeg M_angle_from_self; //!< estimated global angle from self


    std::list< Vector2D > M_pos_history;

    // not used
    BallObject( const BallObject & ball ) = delete;

public:
    /*!
      \brief constructor. initialize member variables
    */
    BallObject();

    /*!
      \brief substitition operator
     */
    BallObject & operator=( const BallObject & ball) = default;

    /*!
      \brief set accuracy count threshold values.
      \param pos_thr threshold value for M_pos
      \param rpos_thr threshold value for M_rpos
      \param vel_thr threshold value for M_vel
    */
    static
    void set_count_thr( const int pos_thr,
                        const int rpos_thr,
                        const int vel_thr );

    /*!
      \brief get estimated global coordinate
      \return const reference to point object
    */
    const Vector2D & pos() const { return M_pos; }

    /*!
      \brief get estimated error of global coordinate
      \return const reference to vector object
    */
    const Vector2D & posError() const { return M_pos_error; }

    /*!
      \brief get global position accuracy count
      \return cycle count from last observation
    */
    int posCount() const { return M_pos_count; }

    /*!
      \brief get estimated position relative from self
      \return const referenct to point object
    */
    const Vector2D & rpos() const { return M_rpos; }

    /*!
      \brief get estimated error of relative coordinate
      \return const reference to vector object
    */
    const Vector2D & rposError() const { return M_rpos_error; }

    /*!
      \brief get relative position accuracy count
      \return cycle count from last observation
    */
    int rposCount() const { return M_rpos_count; }

    /*!
      \brief get the last seen position
      \return const reference to the point object
     */
    const Vector2D & seenPos() const { return M_seen_pos; }

    /*!
      \brief get the number of cycles since last seen
      \return count since last seen
    */
    int seenPosCount() const { return M_seen_pos_count; }

    /*!
      \brief get the last seen relative position
      \return const reference to the variable
     */
    const Vector2D & seenRPos() const { return M_seen_rpos; }

    /*!
      \brief get the last heard position
      \return const reference to the point object
     */
    const Vector2D & heardPos() const { return M_heard_pos; }

    /*!
      \brief get the number of cycles since last observation
      \return count since last observation
    */
    int heardPosCount() const { return M_heard_pos_count; }

    /*!
      \brief get estimated velocity
      \return const referenct to vector object
    */
    const Vector2D & vel() const { return M_vel; }

    /*!
      \brief get estimated error of velocity
      \return const reference to vector object
    */
    const Vector2D & velError() const { return M_vel_error; }

    /*!
      \brief get velocity accuracy count
      \return cycle count from last observation
    */
    int velCount() const { return M_vel_count; }

    /*!
      \brief get the last seen velocity
      \return const reference to the point object
     */
    const Vector2D & seenVel() const { return M_seen_vel; }

    /*!
      \brief get the number of cycles since last velocity seen
      \return count since last velocity seen
    */
    int seenVelCount() const { return M_seen_vel_count; }

    /*!
      \brief get the last seen velocity
      \return const reference to the point object
     */
    const Vector2D & heardVel() const { return M_heard_vel; }

    /*!
      \brief get the number of cycles since last velocity seen
      \return count since last velocity seen
    */
    int heardVelCount() const { return M_heard_vel_count; }

    /*!
      \brief get the number of ghost detection count
     */
    int ghostCount() const { return M_ghost_count; }

    /*!
      \brief get count since ball lost
      \return cycle count since last observation
    */
    int lostCount() const { return M_lost_count; }

    /*!
      \brief velify global position accuracy
      \return true if position has enough accuracy
    */
    bool posValid() const
      {
        return M_pos_count < S_pos_count_thr;
      }

    /*!
      \brief velify relative position accuracy
      \return true if relative position has enough accuracy
    */
    bool rposValid() const
      {
          return M_rpos_count < S_rpos_count_thr;
      }

    /*!
      \brief verify velocity accuracy
      \return true if velocity has enough accuracy
    */
    bool velValid() const
      {
          return M_vel_count < S_vel_count_thr;
      }

    /*!
      \brief get estimated distance from self
      \return distance value
    */
    double distFromSelf() const { return M_dist_from_self; }

    /*!
      \brief get estimated global angle from self
      \return const reference to angle object
    */
    const AngleDeg & angleFromSelf() const { return M_angle_from_self; }

    /*!
      \brief get the history of estimated position.
      \return position list. the front element is the position at the previous cycle.
     */
    const std::list< Vector2D > & posHistory() const
      {
          return M_pos_history;
      }

    /*!
      \brief clear all confidence values
    */
    void setGhost();

    /*!
      \brief update by intenal memory
      \param act const reference to the action effector
    */
    void update( const ActionEffector & act,
                 const GameMode & game_mode );

    /*!
      \brief update status with fullstate info
      \param pos no error position in fullstate info
      \param vel no error velocity in fullstate info
      \param self_pos global self position
     */
    void updateByFullstate( const Vector2D & pos,
                            const Vector2D & vel,
                            const Vector2D & self_pos );
private:
    /*!
      \brief apply wind effect

      This method is called only from update()
    */
    void updateWindEffect();

public:
    /*!
      \brief apply collision effect
      \param pos new global position
      \param pos_count accuracy counter
      \param rpos new relative position
      \param rpos_count accuracy counter
      \param vel new velocity
      \param vel_count accuracy counter

      Thie method is called when collision estimatation
    */
    void updateByCollision( const Vector2D & pos,
                            const int pos_count,
                            const Vector2D & rpos,
                            const int rpos_count,
                            const Vector2D & vel,
                            const int vel_count );

    /*!
      \brief update positional data based on the current game mode
      \param mode current game mode
     */
    void updateByGameMode( const GameMode & mode );

    /*!
      \brief update relative position using see info.
      \param rpos observed relative position
      \param rpos_err estimated error of relative position
    */
    void updateOnlyRelativePos( const Vector2D & rpos,
                                const Vector2D & rpos_err );

    /*!
      \brief update velocity using see info
      \param vel estimated velocity
      \param vel_err estimated error of velocity
      \param vel_count new accuracy value of the velocity
    */
    void updateOnlyVel( const Vector2D & vel,
                        const Vector2D & vel_err,
                        const int vel_count );

    /*!
      \brief update by other player's kickable effect
     */
    void setPlayerKickable();

    /*!
      \brief update position by see info (not include velocity)
      \param pos estimated global position
      \param pos_err estimated error of global position
      \param pos_count new accuracy value of global position.
      usually same as the self position accuracy.
      \param rpos estimated relative position
      \param rpos_err estimated error of relative position
    */
    void updatePos( const Vector2D & pos,
                    const Vector2D & pos_err,
                    const int pos_count,
                    const Vector2D & rpos,
                    const Vector2D & rpos_err );

    /*!
      \brief update all status by see info
      \param pos estimated global position
      \param pos_err estimated error of global position
      \param pos_count new accuracy value of global position.
      usually same as the self position accuracy.
      \param rpos estimated relative position
      \param rpos_err estimated error of relative position
      \param vel estimated velocity
      \param vel_err estimated error of velocity
      \param vel_count new accuracy value of the velocity
    */
    void updateAll( const Vector2D & pos,
                    const Vector2D & pos_err,
                    const int pos_count,
                    const Vector2D & rpos,
                    const Vector2D & rpos_err,
                    const Vector2D & vel,
                    const Vector2D & vel_err,
                    const int vel_count );

    /*!
      \brief update ball status using heared info
      \param act const referenct to the ActionEffector instance
      \param sender_to_ball_dist distance from message sender to ball
      \param heard_pos heard position
      \param heard_vel heard velocity
      \param pass with pass message

      This method is called just before decision.
    */
    void updateByHear( const ActionEffector & act,
                       const double & sender_to_ball_dist,
                       const Vector2D & heard_pos,
                       const Vector2D & heard_vel,
                       const bool pass );

    /*!
      \brief update self related info
      \param self const reference to the self object
      \param prev const reference to the previous ball object

      This method is called just before decision.
    */
    void updateSelfRelated( const SelfObject & self,
                            const BallObject & prev );

    //
    //
    //

    // inertia movement calculators

    /*!
      \brief estimate movement travel vector
      \param cycle this method estimates ball travel after this steps
      \return estimated travel vector
    */
    Vector2D inertiaTravel( const int cycle ) const
      {
          return inertia_n_step_travel( vel(),
                                        cycle,
                                        ServerParam::i().ballDecay() );
      }

    /*!
      \brief estimate reach point
      \param cycle this method estimates ball point after this steps
      \return estimated point vector
    */
    Vector2D inertiaPoint( const int cycle ) const
      {
          return inertia_n_step_point( pos(),
                                       vel(),
                                       cycle,
                                       ServerParam::i().ballDecay() );
      }

    /*!
      \brief estimate reach point
      \return estimated point
    */
    Vector2D inertiaFinalPoint() const
      {
          return inertia_final_point( pos(),
                                      vel(),
                                      ServerParam::i().ballDecay() );
      }

};

}

#endif
