// -*-c++-*-

/*!
  \file coach_player_object.h
  \brief player object class for coach/trainer Header file.
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

#ifndef RCSC_COACH_PLAYER_OBJECT_H
#define RCSC_COACH_PLAYER_OBJECT_H

#include <rcsc/common/stamina_model.h>
#include <rcsc/common/player_type.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <vector>
#include <iostream>

namespace rcsc {

namespace rcg {
struct PlayerT;
}

/*-------------------------------------------------------------------*/

/*!
  \class CoachPlayerObject
  \brief player information for coath/trainer
*/
class CoachPlayerObject {
public:

    //! container type of const CoachPlayerObject*
    typedef std::vector< const CoachPlayerObject * > Cont;

private:

    SideID M_side; //!< LEFT or RIGHT
    int M_unum; //!< uniform number
    bool M_goalie; //!< goalie or not
    int M_type; //!< plaeyr type id
    const PlayerType * M_player_type; //!< const point to the player type instance

    Vector2D M_pos; //!< global position
    Vector2D M_vel; //!< velocity

    AngleDeg M_body; //!< body angle
    AngleDeg M_face; //!< global neck angle

    StaminaModel M_stamina; //!< (heared or estimated) stamina value

    int M_pointto_cycle; //< if player is pointing, this value in incremented
    AngleDeg M_pointto_angle; //!< player's global arm angle

    bool M_kicking; //!< true if player performed the kick.
    int M_tackle_cycle; //!< if player is tackling, this value is incremented
    int M_charged_cycle; //!< if player is charged, this value is incremented

    Card M_card; //!< player's card status

    //

    int M_ball_reach_step; //!< estimated ball interception step

public:

    /*!
      \brief inititialize all value with 0 or invalid values
    */
    CoachPlayerObject();

    /*!
      \brief create clone object
      \return new object pointer
     */
    CoachPlayerObject * clone() const;

    /*!
      \brief check if this player is valid or not.
      \return checked result.
     */
    bool isValid() const
      {
          return M_side != NEUTRAL;
      }

    /*!
      \brief get side info
      \return side ID
     */
    SideID side() const
      {
          return M_side;
      }

    /*!
      \brief get player's uniform number
      \return uniform number
     */
    int unum() const
      {
          return M_unum;
      }

    /*!
      \brief check if this player is goalie
      \return if goalie, true
     */
    bool goalie() const
      {
          return M_goalie;
      }

    /*!
      \brief get player type id
      \return player type id
     */
    int type() const
      {
          return M_type;
      }

    /*!
      \brief get the pointer to player type object
      \return const pointer
     */
    const PlayerType * playerTypePtr() const
      {
          return M_player_type;
      }

    /*!
      \brief get player's position
      \return const reference to the position variable
     */
    const Vector2D & pos() const
      {
          return M_pos;
      }

    /*!
      \brief get player's velocity
      \return const reference to the velocity variable
     */
    const Vector2D & vel() const
      {
          return M_vel;
      }

    /*!
      \brief get player's body angle
      \return const reference to the body angle variable
     */
    const AngleDeg & body() const
      {
          return M_body;
      }

    /*!
      \brief get player's global face angle
      \return const reference to the face angle variable
     */
    const AngleDeg & face() const
      {
          return M_face;
      }

    /*!
      \brief get player's stamina model data
      \return stamina model data
     */
    const StaminaModel & staminaModel() const
      {
          return M_stamina;
      }

    /*!
      \brief get player's stamina value
      \return stamina value
     */
    double stamina() const
      {
          return M_stamina.stamina();
      }

    /*!
      \brief get player's effort value
      \return effort value
     */
    double effort() const
      {
          return M_stamina.effort();
      }

    /*!
      \brief get player's recovery value
      \return recovery value
     */
    double recovery() const
      {
          return M_stamina.recovery();
      }

    /*!
      \brief get player's stamina capacity value
      \return stamina capacity value
     */
    double staminaCapacity() const
      {
          return M_stamina.capacity();
      }

    /*!
      \brief get player's pointto status
      \return if the player is now pointing,
      return positive value as the continuous period.
      ir no pointing action, return 0.
     */
    int pointtoCycle() const
      {
          return M_pointto_cycle;
      }

    /*!
      \brief get player's global arm angle
      \return global pointing angle
     */
    const AngleDeg & pointtoAngle() const
      {
          return M_pointto_angle;
      }

    /*!
      \brief check if player is pointing or not
      \return true if player is pointing
     */
    bool isPointing() const
      {
          return ( M_pointto_cycle > 0 );
      }

    /*!
     \brief check if player performed a kick.
     \return true if player performed a kick
     */
    bool isKicking() const
      {
          return M_kicking;
      }

    /*!
      \brief get player's tackle status
      \return if the player is now tackling,
      return positive value as the continuous period.
      ir no tackle action, return 0.
     */
    int tackleCycle() const
      {
          return M_tackle_cycle;
      }

    /*!
      \brief check if player is tackling or not
      \return true if player is tackling
     */
    bool isTackling() const
      {
          return ( M_tackle_cycle > 0 );
      }

    /*!
      \brief get player's charged status
      \return the accumulated number of charged cycle
     */
    int chargedCycle() const
      {
          return M_charged_cycle;
      }

    /*!
      \brief check if player is charged or not
      \return true if player is charged
     */
    bool isCharged() const
      {
          return ( M_charged_cycle > 0 );
      }

    /*!
      \brief get current card status.
      \return card type
     */
    Card card() const
      {
          return M_card;
      }

    /*!
      \brief get the estimated interception step. negative value means this player never intercept the ball.
      \return estimated interception step.
     */
    int ballReachStep() const
      {
          return M_ball_reach_step;
      }

    //////////////////////////////////////////////////////////////

    /*!
      \brief set team data with seen information
      \param side side ID
      \param unum uniform number
      \param goalie goalie flag
     */
    void setTeam( const SideID side,
                  const int unum,
                  const bool goalie )
      {
          M_side = side;
          M_unum = unum;
          M_goalie = goalie;
      }

    /*!
      \brief set player type id
      \param type player type id
     */
    void setPlayerType( const int type );


    /*!
      \brief change player type (recover stamina and reset card)
      \param type player type id
     */
    void changePlayerType( const int type );

    /*!
      \brief set position with seen information
      \param x seen x
      \param y seen y
     */
    void setPos( const double x,
                 const double y )
      {
          M_pos.assign( x, y );
      }

    /*!
      \brief set velocity with seen information
      \param vx seen velocity x
      \param vy seen velocity y
     */
    void setVel( const double vx,
                 const double vy )
      {
          M_vel.assign( vx, vy );
      }

    /*!
      \brief set body and neck angle with seen information
      \param b seen body angle
      \param n seen neck angle relative to body
     */
    void setAngle( const double b,
                   const double n )
      {
          M_body = b;
          M_face = b + n;
      }

    /*!
      \brief set player's stamina information
      \param s new stamina value
     */
    void setStamina( const double s )
      {
          M_stamina.setStamina( s );
      }

    /*!
      \brief set player's effort information
      \param e new effort value
     */
    void setEffort( const double e )
      {
          M_stamina.setEffort( e );
      }

    /*!
      \brief set player's recovery information
      \param r new recovery value
     */
    void setRecovery( const double r )
      {
          M_stamina.setRecovery( r );
      }

    /*!
      \brief set player's stamina capacity information
      \param c new capacity value
     */
    void setStaminaCapacity( const double c )
      {
          M_stamina.setCapacity( c );
      }

    /*!
      \brief set arm status with seen information
      \param angle pointing global angle
     */
    void setArm( const double angle )
      {
          M_pointto_cycle = 1;
          M_pointto_angle = angle;
      }

    /*!
     \brief set kick state
     \param on true if player performed kick.
    */
    void setKicking( const bool on )
      {
          M_kicking = on;
      }

    /*!
      \brief set tackle status
     */
    void setTackle()
      {
          M_tackle_cycle = 1;
      }

    void setCharged()
      {
          M_charged_cycle = 1;
      }

    /*!
      \brief set card status
     */
    void setCard( const Card card )
      {
          M_card = card;
      }


    void setBallReachStep( const int step )
      {
          M_ball_reach_step = step;
      }

    /*!
      \brief update with seen data
      \param p seen data
     */
    void update( const CoachPlayerObject & p );

    /*!
      \brief update using game log data
      \param p player data in game log format
     */
    void update( const rcg::PlayerT & p );

    /*!
      \brief reset stamina to the initial value
     */
    void recoverStamina();

    /*!
      \brief reverse positional value
     */
    void reverseSide()
      {
          M_pos.reverse();
          M_vel.reverse();
          M_body += 180.0;
          M_face += 180.0;
          if ( isPointing() ) M_pointto_angle += 180.0;
      }

    /*!
      \brief put data to the output stream
      \param os reference cto the output stream
      \return os reference cto the output stream
     */
    std::ostream & print( std::ostream & os ) const;

};

}

#endif
