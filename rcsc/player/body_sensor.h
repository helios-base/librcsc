// -*-c++-*-

/*!
  \file body_sensor.h
  \brief sense_body sensor Header File
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

#ifndef RCSC_PLAYER_BODY_SENSOR_H
#define RCSC_PLAYER_BODY_SENSOR_H

#include <rcsc/player/view_mode.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <iostream>

namespace rcsc {

/*!
  \class BodySensor
  \brief sense_body info holder
*/
class BodySensor {
public:

    GameTime M_time; //!< updated game time

    ViewQuality M_view_quality; //!< sensed view quality
    ViewWidth M_view_width; //!< sensed view width
    double M_stamina; //!< sensed stamina value
    double M_effort; //!< sensed effort value
    double M_stamina_capacity; //!< sensed stamina capacity
    double M_speed_mag; //!< sensed speed magnitude. this is quantized by 0.01.
    double M_speed_dir_relative; //!< speed dir. this is relative to face angle.
    double M_neck_relative; //!< neck angle. this is relative to body angle

    int M_kick_count; //!< sensed command count
    int M_dash_count; //!< sensed command count
    int M_turn_count; //!< sensed command count
    int M_say_count; //!< sensed command count
    int M_turn_neck_count; //!< sensed command count
    int M_catch_count; //!< sensed command count
    int M_move_count; //!< sensed command count
    int M_change_view_count; //!< sensed command count
    int M_change_focus_count; //!< sensed command count

    /*!
      the number of cycles till the arm is movable.
      if 0, arm is movable now
    */
    int M_arm_movable;
    /*!
      the number of cycles till the arm stops pointing.
      if 0, player is not pointing.
    */
    int M_arm_expires;

    /*!
      the distance of the point the player is pointing to, relative to the
      player's position, accurate to 10cm
    */
    double M_pointto_dist;

    /*!
      the direction of the point where the player is pointing to, relative to the
      player's face angle, accurate to 0.1 deg.
    */
    double M_pointto_dir;

    int M_pointto_count; //!< sensed command count

    SideID M_attentionto_side; //!< attended side
    int M_attentionto_unum; //!< attended player number
    int M_attentionto_count; //!< sensed command count

    /*!
      the number of cycles the current tackle will last for.
      if 0, player is not tackling.
    */
    int M_tackle_expires; //!< tackle expire cycles
    int M_tackle_count; //!< sensed command count1

    bool M_none_collided; //!< true if 'none' collides
    bool M_ball_collided; //!< ball collision info
    bool M_player_collided; //!< player collision info
    bool M_post_collided; //!< post collision info

    int M_charged_expires; //!< foul charged expire cycle
    Card M_card; //!< yellow/red card

    // v18+
    double M_focus_dist; //!< distance to the focus point
    double M_focus_dir; //!< direction to the focus point, relative to the body direction

public:
    /*!
      \brief init member variables
    */
    BodySensor();

    /*!
      \brief analyze server message
      \param msg raw server message
      \param version client version
      \param current current game time
    */
    void parse( const char * msg,
                const double & version,
                const GameTime & current );

private:

    /*!
      \brief analyze arm information in the sense_body message.
      \param msg server message started with (arm
      \param pointer pointer to the next character after parsing
      \return parsing result
     */
    bool parseArm( const char * msg,
                   char ** next );

    /*!
      \brief analyze attentionto(focus) information in the sense_body message.
      \param msg server message started with (focus
      \param pointer pointer to the next character after parsing
      \return parsing result
     */
    bool parseAttentionto( const char * msg,
                           char ** next );

    /*!
      \brief analyze tackle information in the sense_body message.
      \param msg server message started with (tackle
      \param pointer pointer to the next character after parsing
      \return parsing result
     */
    bool parseTackle( const char * msg,
                      char ** next );


    /*!
      \brief analyze collision information contained by sense_body message.
      \param msg server message started with (collision
      \param pointer pointer to the next character after parsing
      \return parsing result
     */
    bool parseCollision( const char * msg,
                         char ** next );

    /*!
      \brief analyze card information
      \param msg server message started with (card
      \param pointer pointer to the next character after parsing
      \return parsing result
     */
    bool parseFoul( const char * msg,
                    char ** next );

    /*!
      \brief analyze focus point information
      \param msg server message started with (focus_point
      \param pointer pointer to the next character after parsing
      \return parsing result
     */
    bool parseFocusPoint( const char * msg,
                          char ** next );

public:

    /*!
      \brief get last updated time
      \return const reference to the game time
    */
    const
    GameTime & time() const
      {
          return M_time;
      }

    /*!
      \brief get analyzed view quality
      \return const reference to the view quality object
    */
    const
    ViewQuality & viewQuality() const
      {
          return M_view_quality;
      }

    /*!
      \brief get analyzed view width
      \return const reference to the view width object
    */
    const
    ViewWidth & viewWidth() const
      {
          return M_view_width;
      }

    /*!
      \brief get analyzed stamina value
      \return stamina value
    */
    const
    double & stamina() const
      {
          return M_stamina;
      }

    /*!
      \brief get analyzed effort
      \return effort value
    */
    const
    double & effort() const
      {
          return M_effort;
      }

    /*!
      \brief get analized stamina capacity value
      \return stamina capacity value
     */
    const
    double & staminaCapacity() const
      {
          return M_stamina_capacity;
      }

    /*!
      \brief get analyzed speed value
      \return scalar value of velocity
    */
    const
    double & speedMag() const
      {
          return M_speed_mag;
      }

    /*!
      \brief get analyzed velocity direction relative to player's face direction
      \return velocity direction
    */
    const
    double & speedDir() const
      {
          return M_speed_dir_relative;
      }

    /*!
      \brief get analyzed neck angle
      \return neck angle value
    */
    const
    double & neckDir() const
      {
          return M_neck_relative;
      }

    /*!
      \brief get analyzed kick count
      \return count of performed kick command
    */
    int kickCount() const
      {
          return M_kick_count;
      }

    /*!
      \brief get analyzed dash count
      \return count of performed kick command
    */
    int dashCount() const
      {
          return M_dash_count;
      }

    /*!
      \brief get analyzed turn count
      \return count of performed turn command
    */
    int turnCount() const
      {
          return M_turn_count;
      }

    /*!
      \brief get analyzed say count
      \return count of performed say command
    */
    int sayCount() const
      {
          return M_say_count;
      }

    /*!
      \brief get analyzed turn_neck count
      \return count of performed turn_neck command
    */
    int turnNeckCount() const
      {
          return M_turn_neck_count;
      }

    /*!
      \brief get analyzed catch count
      \return count of performed catch command
    */
    int catchCount() const
      {
          return M_catch_count;
      }

    /*!
      \brief get analyzed move count
      \return count of performed move command
    */
    int moveCount() const
      {
          return M_move_count;
      }

    /*!
      \brief get analyzed change_view count
      \return count of performed change_view command
    */
    int changeViewCount() const
      {
          return M_change_view_count;
      }

    /*!
      \brief get analyzed change_focus count
      \return count of performed change_focus command
    */
    int changeFocusCount() const
      {
          return M_change_focus_count;
      }

    /*!
      \brief get analyzed cycles till the arm is movable
      \return cycles till the arm is movable
    */
    int armMovable() const
      {
          return M_arm_movable;
      }

    /*!
      \brief get analyzed cycles till the arm stops pointing
      \return cycles till the arm is movable
    */
    int armExpires() const
      {
          return M_arm_expires;
      }

    /*!
      \brief get analyzed distance to the point that player is pointing
      \return distance value
    */
    const
    double & pointtoDist() const
      {
          return M_pointto_dist;
      }

    /*!
      \brief get analyzed direction relative to player's face
      \return direction value
    */
    const
    double & pointtoDir() const
      {
          return M_pointto_dir;
      }

    /*!
      \brief get analyzed pointto count
      \return count of performed pointto command
    */
    int pointtoCount() const
      {
          return M_pointto_count;
      }

    /*!
      \brief get analyzed attended player's side
      \return side Id
    */
    SideID attentiontoSide() const
      {
          return M_attentionto_side;
      }

    /*!
      \brief get analyzed attended player's uniform number
      \return uniform number
    */
    int attentiontoUnum() const
      {
          return M_attentionto_unum;
      }

    /*!
      \brief get analyzed attentionto count
      \return count of performed attentionto command
    */
    int attentiontoCount() const
      {
          return M_attentionto_count;
      }

    /*!
      \brief get analyzed cycles the current tackle will last for
      \return cycles till tackle is exired
    */
    int tackleExpires() const
      {
          return M_tackle_expires;
      }

    /*!
      \brief get analyzed tackle count
      \return count of performed tackle command
    */
    int tackleCount() const
      {
          return M_tackle_count;
      }

    /*!
      \brief get the information wheter the agent receive does not collide.
      \return true if this body sensor has 'none' collision information.
     */
    bool noneCollided() const
      {
          return M_none_collided;
      }

    /*!
      \brief get the information wheter the agent collides with ball
      \return true if the agent collides with ball.
     */
    bool ballCollided() const
      {
          return M_ball_collided;
      }

    /*!
      \brief get the information wheter the agent collides with player
      \return true if the agent collides with player.
     */
    bool playerCollided() const
      {
          return M_player_collided;
      }

    /*!
      \brief get the information wheter the agent collides with posts
      \return true if the agent collides with posts.
     */
    bool postCollided() const
      {
          return M_post_collided;
      }

    /*!
      \brief get expire cycle of foul charge
      \return expire cycle of foul charge
     */
    int chargedExpires() const
      {
          return M_charged_expires;
      }

    /*!
      \brief get the yellow/red card status
      \return card type
     */
    Card card() const
      {
          return M_card;
      }

    /*!
      \brief get the focus distance value
      \return the distance to the focus point
     */
    double focusDist() const
      {
          return M_focus_dist;
      }

    /*!
      \brief get the focus direction value
      \return the direction to the focus point, relative to the body angle
     */
    double focusDir() const
      {
          return M_focus_dir;
      }

    /*!
      \brief put data to output stream
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & print( std::ostream & os ) const;
};

}

#endif
