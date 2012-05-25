// -*-c++-*-

/*!
  \file say_message_builder.h
  \brief player's say message builder Header File
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

#ifndef RCSC_PLAYER_SAY_MESSAGE_BUILDER_H
#define RCSC_PLAYER_SAY_MESSAGE_BUILDER_H

#include <rcsc/common/say_message.h>
#include <rcsc/common/say_message_parser.h>
#include <rcsc/geom/vector_2d.h>

#include <string>
#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!
  \class BallMessage
  \brief ball info message encoder

  format:
  "b<pos_vel:5>"
  the length of message == 6
*/
class BallMessage
    : public SayMessage {
private:

    Vector2D M_ball_pos; //!< ball position to be encoded
    Vector2D M_ball_vel; //!< ball velocity to be encoded

public:

    /*!
      \brief construct with raw information
      \param ball_pos ball position to be encoded
      \param ball_vel ball velocity to be encoded
    */
    BallMessage( const Vector2D & ball_pos,
                 const Vector2D & ball_vel )
        : M_ball_pos( ball_pos ),
          M_ball_vel( ball_vel )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return BallMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return BallMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class PassMessage
  \brief pass info message encoder

  format:
  "p<unum_pos:4><pos_vel:5>"
  the length of message == 10
*/
class PassMessage
    : public SayMessage {
private:

    int M_receiver_unum; //!< pass receiver's uniform number
    Vector2D M_receive_point; //!< desired pass receive point

    Vector2D M_ball_pos; //!< ball first pos
    Vector2D M_ball_vel; //!< ball first vel

public:

    /*!
      \brief construct with raw information
      \param receiver_unum pass receiver's uniform number
      \param receive_point desired pass receive point
      \param ball_pos next ball position
      \param ball_vel next ball velocity
    */
    PassMessage( const int receiver_unum,
                 const Vector2D & receive_point,
                 const Vector2D & ball_pos,
                 const Vector2D & ball_vel )
        : M_receiver_unum( receiver_unum ),
          M_receive_point( receive_point ),
          M_ball_pos( ball_pos ),
          M_ball_vel( ball_vel )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return PassMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return PassMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class InterceptMessage
  \brief intercept info message encoder

  format:
  "i<unum:1><cycle:1>"
  the length of message == 3
*/
class InterceptMessage
    : public SayMessage {
private:

    bool M_our; //!< flag of interceptor's side
    int M_unum; //!< interceptor's uniform number
    int M_cycle; //!< interception cycle

public:

    /*!
      \brief construct with raw information
      \param our flag of intereptor side
      \param unum interceptor's uniform number
      \param cycle interception cycle
    */
    InterceptMessage( const bool our,
                      const int unum,
                      const int cycle )
        : M_our( our ),
          M_unum( unum ),
          M_cycle( cycle )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return InterceptMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return InterceptMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class GoalieMessage
  \brief goalie info message encoder

  format:
  "g<pos_body:4>"
  the length of message == 5
*/
class GoalieMessage
    : public SayMessage {
private:

    int M_goalie_unum; //!< goalie's uniform number
    Vector2D M_goalie_pos; //!< goalie's position
    AngleDeg M_goalie_body; //!< goalie's body angle

public:

    /*!
      \brief construct with raw information
      \param goalie_unum goalie's uniform number
      \param goalie_pos goalie's global position
      \param goalie_body goalie's body direction
    */
    GoalieMessage( const int goalie_unum,
                   const Vector2D & goalie_pos,
                   const AngleDeg & goalie_body )
        : M_goalie_unum( goalie_unum ),
          M_goalie_pos( goalie_pos ),
          M_goalie_body( goalie_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return GoalieMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return GoalieMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class GoalieAndPlayerMessage
  \brief goalie & 1 field player message encoder

  format:
  "e<pos_body:4><num_pos:3>"
  the length of message == 8
*/
class GoalieAndPlayerMessage
    : public SayMessage {
private:

    int M_goalie_unum; //!< goalie's uniform number
    Vector2D M_goalie_pos; //!< goalie's position
    AngleDeg M_goalie_body; //!< goalie's body angle

    int M_player_number; //!< teammate unum [1-11]. or opponent unum [12-23]
    Vector2D M_player_pos;

public:

    /*!
      \brief construct with raw information
      \param goalie_unum goalie's uniform number
      \param goalie_pos goalie's global position
      \param goalie_body goalie's body direction
      \param unum player's number
      \param player_pos player'ss global position
    */
    GoalieAndPlayerMessage( const int goalie_unum,
                            const Vector2D & goalie_pos,
                            const AngleDeg & goalie_body,
                            const int player_number,
                            const Vector2D & player_pos )
        : M_goalie_unum( goalie_unum ),
          M_goalie_pos( goalie_pos ),
          M_goalie_body( goalie_body ),
          M_player_number( player_number ),
          M_player_pos( player_pos )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return GoalieAndPlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return GoalieAndPlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class OffsideLineMessage
  \brief offside line info message encoder

  format:
  "o<x_rate:1>"
  the length of message == 2
*/
class OffsideLineMessage
    : public SayMessage {
private:

    double M_offside_line_x; //!< raw offside line x coordinate value

public:

    /*!
      \brief construct with raw information
      \param offside_line_x raw offside line x coordinate value
    */
    explicit
    OffsideLineMessage( const double & offside_line_x )
        : M_offside_line_x( offside_line_x )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return OffsideLineMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return OffsideLineMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class DefenseLineMessage
  \brief defense line info message encoder

  format:
  "d<x_rate:1>"
  the length of message == 2
*/
class DefenseLineMessage
    : public SayMessage {
private:

    double M_defense_line_x; //!< raw defense line x coordinate value

public:

    /*!
      \brief construct with raw information
      \param defense_line_x raw defense line x coordinate value
    */
    explicit
    DefenseLineMessage( const double & defense_line_x )
        : M_defense_line_x( defense_line_x )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return DefenseLineMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return DefenseLineMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class WaitRequestMessage
  \brief wait request message encoder

  format:
  "w"
  the length of message == 1
*/
class WaitRequestMessage
    : public SayMessage {
private:

public:

    /*!
      \brief construct with raw information
    */
    WaitRequestMessage()
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return WaitRequestMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return WaitRequestMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class SetplayMessage
  \brief setplay message encoder

  format:
  "F<wait:1>"
  the length of message == 2
*/
class SetplayMessage
    : public SayMessage {
private:

    int M_wait_step;

public:

    /*!
      \brief construct with raw information
    */
    explicit
    SetplayMessage( const int wait_step )
        : M_wait_step( wait_step )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return SetplayMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return SetplayMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class PassRequestMessage
  \brief pass request info message encoder

  format:
  "h<pos:3>"
  the length of message == 4
*/
class PassRequestMessage
    : public SayMessage {
private:

    Vector2D M_target_point; //!< dash target point

public:

    /*!
      \brief construct with raw information
      \param target_point dash target point
    */
    explicit
    PassRequestMessage( const Vector2D & target_point )
        : M_target_point( target_point )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return PassRequestMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return PassRequestMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class StaminaMessage
  \brief stamina info message encoder

  format:
  "s<rate:1>"
  the length of message == 2
*/
class StaminaMessage
    : public SayMessage {
private:

    double M_stamina; //!< raw stamina value

public:

    /*!
      \brief construct with raw information
      \param stamina raw stamina value
    */
    explicit
    StaminaMessage( const double & stamina )
        : M_stamina( stamina )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return StaminaMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return StaminaMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class RecoveryMessage
  \brief recovery info message encoder

  format:
  "r<rate:1>"
  the length of message == 2
*/
class RecoveryMessage
    : public SayMessage {
private:

    double M_recovery; //!< raw recovery value

public:

    /*!
      \brief construct with raw information
      \param recovery raw recovery value
    */
    explicit
    RecoveryMessage( const double & recovery )
        : M_recovery( recovery )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return RecoveryMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return RecoveryMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class StaminaCapacityMessage
  \brief stamina info message encoder

  format:
  "c<rate:1>"
  the length of message == 2
*/
class StaminaCapacityMessage
    : public SayMessage {
private:

    double M_stamina_capacity; //!< raw value

public:

    /*!
      \brief construct with raw information
      \param value raw stamina capacity value
    */
    explicit
    StaminaCapacityMessage( const double value )
        : M_stamina_capacity( value )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return StaminaCapacityMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return StaminaCapacityMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class DribbleMessage
  \brief dribble info message encoder

  format:
  "D<count_pos:3>"
  the length of message == 4
*/
class DribbleMessage
    : public SayMessage {
private:

    Vector2D M_target_point; //!< dribble target point
    int M_queue_count; //!< dribble queue count

public:

    /*!
      \brief construct with raw information
      \param target_point dribble target point
      \param queue_count dribble action queue count
    */
    DribbleMessage( const Vector2D & target_point,
                    const int queue_count )
        : M_target_point( target_point ),
          M_queue_count( queue_count )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return DribbleMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return DribbleMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class BallGoalieMessage
  \brief ball & goalie info message encoder

  format:
  "G<bpos_bvel_gpos_gbody:9>"
  the length of message == 5
*/
class BallGoalieMessage
    : public SayMessage {
private:

    Vector2D M_ball_pos; //!< ball position
    Vector2D M_ball_vel; //!< ball vel
    Vector2D M_goalie_pos; //!< goalie's position
    AngleDeg M_goalie_body; //!< goalie's body angle

public:

    /*!
      \brief construct with raw information
      \param ball_pos ball position
      \param ball_vel ball velocity
      \param goalie_pos goalie's global position
      \param goalie_body goalie's body angle
    */
    BallGoalieMessage( const Vector2D & ball_pos,
                       const Vector2D & ball_vel,
                       const Vector2D & goalie_pos,
                       const AngleDeg & goalie_body )
        : M_ball_pos( ball_pos ),
          M_ball_vel( ball_vel ),
          M_goalie_pos( goalie_pos ),
          M_goalie_body( goalie_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return BallGoalieMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return BallGoalieMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class OnePlayerMessage
  \brief one player info message encoder

  format:
  "P<unum_pos:3>"
  the length of message == 4
*/
class OnePlayerMessage
    : public SayMessage {
private:

    int M_unum; //!< player's unum [1-22]. if opponent, unum > 11
    Vector2D M_player_pos; //!< player's position

public:

    /*!
      \brief construct with raw information
      \param unum player's unum
      \param player_pos player's global position
    */
    OnePlayerMessage( const int unum,
                      const Vector2D & player_pos )
        : M_unum( unum ),
          M_player_pos( player_pos )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return OnePlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return OnePlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class TwoPlayerMessage
  \brief 2 players info message encoder

  format:
  "Q<unum_pos:3,unum_pos:3>"
  the length of message == 7
*/
class TwoPlayerMessage
    : public SayMessage {
private:

    int M_player_unum[2]; //!< player's unum [1-22]. if opponent, unum > 11
    Vector2D M_player_pos[2]; //!< player's position

    // not usd
    TwoPlayerMessage();
public:

    /*!
      \brief construct with raw information
      \param player0_unum player's unum, if opponent, += 11
      \param player0_pos goalie's global position
      \param player1_unum player's unum, if opponent, += 11
      \param player1_pos goalie's global position
    */
    TwoPlayerMessage( const int player0_unum,
                      const Vector2D & player0_pos,
                      const int player1_unum,
                      const Vector2D & player1_pos );

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return TwoPlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return TwoPlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class ThreePlayerMessage
  \brief 3 players info message encoder

  format:
  "Q<unum_pos:3,unum_pos:3,unum_pos:3>"
  the length of message == 10
*/
class ThreePlayerMessage
    : public SayMessage {
private:

    int M_player_unum[3]; //!< player's unum [1-22]. if opponent, unum > 11
    Vector2D M_player_pos[3]; //!< player's position

public:

    /*!
      \brief construct with raw information
      \param player0_unum player's unum, if opponent, += 11
      \param player0_pos goalie's global position
      \param player1_unum player's unum, if opponent, += 11
      \param player1_pos goalie's global position
      \param player2_unum player's unum, if opponent, += 11
      \param player2_pos goalie's global position
    */
    ThreePlayerMessage( const int player0_unum,
                        const Vector2D & player0_pos,
                        const int player1_unum,
                        const Vector2D & player1_pos,
                        const int player2_unum,
                        const Vector2D & player2_pos );

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return ThreePlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return ThreePlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class SelfMessage
  \brief self info message encoder

  format:
  "S<pos_body_stamina:4>"
  the length of message == 5
*/
class SelfMessage
    : public SayMessage {
private:

    Vector2D M_self_pos; //!< self position
    AngleDeg M_self_body; //!< self body angle
    double M_self_stamina; //!< self stamina rate [0,1]

public:

    /*!
      \brief construct with raw information
      \param self_pos self global position
      \param self_body self body angle
      \param self_stamina self stamina value
    */
    SelfMessage( const Vector2D & self_pos,
                 const AngleDeg & self_body,
                 const double & self_stamina )
        : M_self_pos( self_pos ),
          M_self_body( self_body ),
          M_self_stamina( self_stamina )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return SelfMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return SelfMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class TeammateMessage
  \brief teammate info message encoder

  format:
  "T<unum_pos_body:4>"
  the length of message == 5
*/
class TeammateMessage
    : public SayMessage {
private:

    int M_unum; //!< player's unum [1-11]
    Vector2D M_player_pos; //!< player's position
    AngleDeg M_player_body; //!< player's body angle

public:

    /*!
      \brief construct with raw information
      \param unum player's unum
      \param player_pos goalie's global position
      \param player_body goalie's body angle
    */
    TeammateMessage( const int unum,
                     const Vector2D & player_pos,
                     const AngleDeg & player_body )
        : M_unum( unum ),
          M_player_pos( player_pos ),
          M_player_body( player_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return TeammateMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return TeammateMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class OpponentMessage
  \brief opponent info message encoder

  format:
  "O<unum_pos_body:4>"
  the length of message == 5
*/
class OpponentMessage
    : public SayMessage {
private:

    int M_unum; //!< player's unum [1-11]
    Vector2D M_player_pos; //!< player's position
    AngleDeg M_player_body; //!< player's body angle

public:

    /*!
      \brief construct with raw information
      \param unum player's unum
      \param player_pos goalie's global position
      \param player_body goalie's body angle
    */
    OpponentMessage( const int unum,
                     const Vector2D & player_pos,
                     const AngleDeg & player_body )
        : M_unum( unum ),
          M_player_pos( player_pos ),
          M_player_body( player_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return OpponentMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return OpponentMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class BallPlayerMessage
  \brief ball & player info message encoder

  format:
  "B<bpos_bvel_unum_ppos_pbody:9>"
  the length of message == 10
*/
class BallPlayerMessage
    : public SayMessage {
private:

    Vector2D M_ball_pos; //!< ball position
    Vector2D M_ball_vel; //!< ball velocity

    int M_unum; //!< player's unum [1-22]. if opponent, unum > 11
    Vector2D M_player_pos; //!< player's position
    AngleDeg M_player_body; //!< player's body angle

public:

    /*!
      \brief construct with raw information
      \param ball_pos ball position
      \param ball_vel ball velocity
      \param unum player's unum
      \param player_pos goalie's global position
      \param player_body goalie's body angle
    */
    BallPlayerMessage( const Vector2D & ball_pos,
                       const Vector2D & ball_vel,
                       const int unum,
                       const Vector2D & player_pos,
                       const AngleDeg & player_body )
        : M_ball_pos( ball_pos ),
          M_ball_vel( ball_vel ),
          M_unum( unum ),
          M_player_pos( player_pos ),
          M_player_body( player_body )
      { }

    /*!
      \brief get the header character of this message
      \return header character of this message
     */
    char header() const
      {
          return BallPlayerMessageParser::sheader();
      }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength()
      {
          return BallPlayerMessageParser::slength();
      }

    /*!
      \brief get the length of this message
      \return the length of encoded message
    */
    int length() const
      {
          return slength();
      }

    /*!
      \brief append this info to the audio message
      \param to reference to the message string instance
      \return result status of encoding
    */
    bool appendTo( std::string & to ) const;

    /*!
      \brief append the debug message
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printDebug( std::ostream & os ) const;

};

}

#endif
