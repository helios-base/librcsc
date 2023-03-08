// -*-c++-*-

/*!
  \file action_effector.h
  \brief player's action effecto manager Header File
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

#ifndef RCSC_PLAYER_ACTION_EFFECTOR_H
#define RCSC_PLAYER_ACTION_EFFECTOR_H

#include <rcsc/player/view_mode.h>
#include <rcsc/player/player_command.h>
#include <rcsc/player/say_message_builder.h>

#include <rcsc/geom/vector_2d.h>
#include <rcsc/game_time.h>
#include <rcsc/types.h>

#include <iostream>
#include <string>
#include <vector>

namespace rcsc {

class BodySensor;
class PlayerAgent;

/*!
  \class ActionEffector
  \brief manages action effect, command counter
*/
class ActionEffector {
private:
    //! const reference to the PlayerAgent instance
    const PlayerAgent & M_agent;

    //! pointer of body action for dynamic allocation
    PlayerBodyCommand * M_command_body;

    //! pointer of turn_neck for dynamic allocation
    PlayerTurnNeckCommand * M_command_turn_neck;
    //! pointer of change_view for dynamic allocation
    PlayerChangeViewCommand * M_command_change_view;
    //! pointer of change_focus for dynamic allocation
    PlayerChangeFocusCommand * M_command_change_focus;
    //! pointer of say for dynamic allocation
    PlayerSayCommand * M_command_say;
    //! pointer of pointto for dynamic allocation
    PlayerPointtoCommand * M_command_pointto;
    //! pointer to attentionto for dynamic allocation
    PlayerAttentiontoCommand * M_command_attentionto;


    //! command counter
    int M_command_counter[PlayerCommand::ILLEGAL + 1];

    //! last time when command is composed. used only to check action count
    GameTime M_last_action_time;

    //! last body command type
    PlayerCommand::Type M_last_body_command_type[2];

    //! checker of turn_neck. true if turn_neck was done at last
    bool M_done_turn_neck;

    // stored last action effect variables

    // kick effect
    Vector2D M_kick_accel;       //!< estimated last kick accel
    Vector2D M_kick_accel_error; //!< estimated last kick accel error

    // turn effect
    double M_turn_actual; //!< estimated last 'real' turn moment
    double M_turn_error;  //!< estimated turn moment error

    // dash effect
    Vector2D M_dash_accel; //!< estimated last dash accel
    //Vector2D M_dash_accel_error;
    double M_dash_power; //!< last dash power to update stamina
    double M_dash_dir; //!< last dash direction relative to body (or reverse body)

    // move effect
    Vector2D M_move_pos; //!< last move coordinates

    // catch effect
    GameTime M_catch_time; //!< last catch time

    // tackle effect
    double M_tackle_power; //!< last tackle power
    double M_tackle_dir;   //!< last tackle dir == body dir
    bool M_tackle_foul;    //!< last tackle foul switch

    // turn_neck effect
    double M_turn_neck_moment; //!< last turn_neck moment

    // say effect
    std::string M_say_message; //!< last said message string
    std::vector< SayMessage::Ptr > M_say_message_cont;

    // pointto effect
    Vector2D M_pointto_pos;  //!< last pointto coordinates


    // not used
    ActionEffector() = delete;
    // nocopyable
    ActionEffector( const ActionEffector & ) = delete;
    ActionEffector operator=( const ActionEffector & ) = delete;
public:
    /*!
      \brief init member variables
      \param agent const reference to the PlayerAgent instance
    */
    explicit
    ActionEffector( const PlayerAgent & agent );

    /*!
      \brief destruct dynamic allocated command objects
    */
    ~ActionEffector();

    /*!
      \brief reset all command effect. called just before action decision
    */
    void reset();

    /*!
      \brief increment command count
      \param type target command type
    */
    void incCommandCount( const PlayerCommand::Type type );

    /*!
      \brief compare internal command count with sensed command count
      \param sense analyzed sense_body info

      This method is called when sense_body received.
      This method MUST be called before WorldModel update
    */
    void checkCommandCount( const BodySensor & sense );

    /*!
      \brief make command string and update last action time
      \param to reference to the output stream
      \return reference to the output stream

      After command string composition, all command objects are deleted.
    */
    std::ostream & makeCommand( std::ostream & to );

    /*!
      \brief delete all command objects and say messages.
     */
    void clearAllCommands();

    ///////////////////////////////////////////////////////////////
    /*!
      \brief get const pointer to the player's body command object
      \return const pointer to the body command object
    */
    const PlayerBodyCommand * bodyCommand() const
      {
          return M_command_body;
      }

    /*!
      \brief get const pointer to the player's turn_neck command object
      \return const pointer to the turn_neck command object
    */
    const PlayerTurnNeckCommand * turnNeckCommand() const
      {
          return M_command_turn_neck;
      }

    /*!
      \brief get const pointer to the player's change_view command object
      \return const pointer to the change_view command object
    */
    const PlayerChangeViewCommand * changeViewCommand() const
      {
          return M_command_change_view;
      }

    /*!
      \brief get const pointer to the player's say command object
      \return const pointer to the say command object
    */
    const PlayerSayCommand * sayCommand() const
      {
          return M_command_say;
      }

    /*!
      \brief get const pointer to the player's pointto command object
      \return const pointer to the pointto command object
    */
    const PlayerPointtoCommand * pointtoCommand() const
      {
          return M_command_pointto;
      }

    /*!
      \brief get const pointer to the player's attentionto command object
      \return const pointer to the attentionto command object
    */
    const PlayerAttentiontoCommand * attentiontoCommand() const
      {
          return M_command_attentionto;
      }

    ///////////////////////////////////////////////////////////////
    // register base command

    /*!
      \brief create kick command and its effect with kick parameter
      \param power kick power for command argument
      \param rel_dir kick direction relative to body angle for command argument

      power is normalized by server parameter
    */
    void setKick( const double & power,
                  const AngleDeg & rel_dir );

    /*!
      \brief create dash command and its effect with dash parameter
      \param power dash power for command argument

      power is normalized by server parameter
      useless dash power is reduceed automatically.
    */
    void setDash( const double & power );

    /*!
      \brief create dash command and its effect with dash parameter
      \param power dash power for command argument
      \param rel_dir dash direction for command argument

      power is normalized by server parameter
      useless dash power is reduceed automatically.
    */
    void setDash( const double & power,
                  const AngleDeg & rel_dir );

    /*!
      \brief create turn command and its effect with turn parameter
      \param moment turn moment that player wants to perform

      command argument, moment is calculated using current player's speed
      and its inertia moment parameter.
      command argument moment is normalized by server parameter.
    */
    void setTurn( const AngleDeg & moment );

    /*!
      \brief create catch command.

      command argumment, catch direction is calculated in this method.
      catch direction is calculated using diagonal line direction
    */
    void setCatch();

    /*!
      \brief create move command and its effect with move parameter
      \param x move target point x
      \param y move target point y
    */
    void setMove( const double & x,
                  const double & y );

    /*!
      \brief create tackle command and its effect with tackle parameter
      \param power_or_dir tackle power or directin for command argument
      \param foul intentional foul switch

      power is normalized by server parameter
    */
    void setTackle( const double & power_or_dir,
                    const bool foul );

    ///////////////////////////////////////////////////////////////
    // register support command

    /*!
      \brief create turn_neck command and its effect with turn_neck parameter
      \param moment turn_neck moment that player wants to perform

      command argument moment is normalized by server parameter.
    */
    void setTurnNeck( const AngleDeg & moment );

    /*!
      \brief create change_view command
      \param width view width that player wants.

      ViewQuality should not be changed by user
    */
    void setChangeView( const ViewWidth & width );

    /*!
      \brief create change_focus command
      \param moment_dist distance added to the current focus point
      \param moment_dir direction added to the current focus point
    */
    void setChangeFocus( const double moment_dist,
                         const AngleDeg & moment_dir );

    /*!
      \brief add new say message
      \param message pointer to the dynamically allocated say message object.
     */
    void addSayMessage( SayMessage * message );

    /*!
      \brief remove the registered say message if exist
      \param header message header character
      \return true if removed
     */
    bool removeSayMessage( const char header );

    /*!
      \brief remove all registered say messages
    */
    void clearSayMessage();

    /*!
      \brief create pointto command and its effect with pointto parameter
      \param x target point x
      \param y target point y
    */
    void setPointto( const double & x,
                     const double & y );

    /*!
      \brief create pointto command by off mode
    */
    void setPointtoOff();

    /*!
      \brief create attentionto command
      \param side target player's side
      \param unum target player's uniform number
    */
    void setAttentionto( const SideID side,
                         const int unum );

    /*!
      \brief create attentionto command by off mode
    */
    void setAttentiontoOff();

    ///////////////////////////////////////////////////////////////

    // accessor method
    // for action effect estimation

    /*!
      \brief get last command composition time
      \return const reference to the game time
    */
    const GameTime & lastActionTime() const
      {
          return M_last_action_time;
      }

    /*!
      \brief get last perfomed command type to update SelfObject
      \return command type Id
    */
    PlayerCommand::Type lastBodyCommandType() const
      {
          return M_last_body_command_type[0];
      }

    /*!
      \brief get last perfomed command type to update SelfObject
      \return command type Id
    */
    PlayerCommand::Type lastBodyCommandType( int i ) const
      {
          return ( 0 <= i && i < 2
                   ? M_last_body_command_type[i]
                   : M_last_body_command_type[0] );
      }

    /*!
      \brief check if turn_neck is performed to update SelfObject
      \return true if turn_neck is performed in previous cycle
    */
    bool doneTurnNeck() const
      {
          return M_done_turn_neck;
      }

    //////////////////////////////////////////////////////////////

    // stored action effect getter method

    //////////////////////////////////////////
    /*!
      \brief get estimated kick action effect
      \param accel variable pointer to store the estimated ball accel
      \param accel_err variable pointer to store the estimated ball accel error
    */
    void getKickInfo( Vector2D * accel,
                      Vector2D * accel_err ) const
      {
          if ( accel ) *accel = M_kick_accel;
          if ( accel_err ) *accel_err = M_kick_accel_error;
      }

    //////////////////////////////////////////
    /*!
      \brief get estimated turn action effect
      \param moment variable pointer to store the estimated actual turned moment
      \param err variable pointer to store the estimated turned moment error
    */
    void getTurnInfo( double * moment,
                      double * err ) const
      {
          if ( moment ) *moment = M_turn_actual;
          if ( err ) *err = M_turn_error;
      }

    //////////////////////////////////////////
    /*!
      \brief get estimated dash action effect
      \param accel variable pointer to store the estimated dash accel
      \param power variable pointer to store the used dash power
    */
    void getDashInfo( Vector2D * accel,
                      /*Vector2D * acc_err,*/
                      double * power ) const
      {
          if ( accel ) *accel = M_dash_accel;
          //if ( acc_err ) *acc_err = M_dash_accel_error;
          if ( power ) *power = M_dash_power;
      }

    //////////////////////////////////////////

    /*!
      \brief get move action effect
      \return moved position
    */
    const Vector2D & getMovePos() const
      {
          return M_move_pos;
      }

    //////////////////////////////////////////

    /*!
      \brief get last time catch action is performed
      \return game time object
    */
    const GameTime & getCatchTime() const
      {
          return M_catch_time;
      }

    //////////////////////////////////////////
    /*!
      \brief get tackle action effect
      \param power variable pointer to store the used tackle power
      \param dir variable pointer to store the tackle direction
    */
    void getTackleInfo( double * power,
                        double * dir,
                        bool * foul ) const
      {
          if ( power ) *power = M_tackle_power;
          if ( dir ) *dir = M_tackle_dir;
          if ( foul ) *foul = M_tackle_foul;
      }

    /*!
      \brief get the last tackle foul switch.
      \return the value of the last tackle foul switch.
     */
    bool tackleFoul() const
      {
          return M_tackle_foul;
      }

    //////////////////////////////////////////
    /*!
      \brief get turn_neck action effect
      \return performed turn_neck moment
    */
    double getTurnNeckMoment() const
      {
          return M_turn_neck_moment;
      }

    //////////////////////////////////////////
    /*!
      \brief get say action effect
      \return say message string
    */
    const std::string & getSayMessage() const
      {
          return M_say_message;
      }

    /*!
      \brief get the total length of current reserved messages
      \return the total length of current reserved messages
     */
    int getSayMessageLength() const;

    /*!
      \brief get the reserved say messages
      \return const reference to the say message builder container
     */
    const std::vector< SayMessage::Ptr > & sayMessageCont() const
      {
          return M_say_message_cont;
      }

    //////////////////////////////////////////
    /*!
      \brief get pointto action effect
      \return estimated pointed position
    */
    const Vector2D & getPointtoPos() const
      {
          return M_pointto_pos;
      }

    /////////////////////////////////////////
    // get queued action info
    /*!
      \brief get estimated next body angle using queued action effect
      \return estimated body angle
    */
    AngleDeg queuedNextSelfBody() const;

    /*!
      \brief get estimated next body angle using queued action effect
      \return estimated body angle
    */
    AngleDeg queuedNextMyBody() const
      {
          return queuedNextSelfBody();
      }

    /*!
      \brief get estimated next position using queued action effect
      \return estimated global position
    */
    Vector2D queuedNextSelfPos() const;

    /*!
      \brief get estimated next position using queued action effect
      \return estimated global position
    */
    Vector2D queuedNextMyPos() const
      {
          return queuedNextSelfPos();
      }

    /*!
      \brief check if ball will be kickable in the next cycle
      \return true if ball will be kickable
     */
    bool queuedNextBallKickable() const;

    /*!
      \brief get estimated next ball position using queued action effect
      \return estimated ball position
    */
    Vector2D queuedNextBallPos() const;

    /*!
      \brief get estimated next ball velocity using queued action effect
      \return estimated ball velocity
    */
    Vector2D queuedNextBallVel() const;

    /*!
      \brief get estimated next angle of target point relative to estimated
      body angle using queued action effect
      \param target target point
      \return angle relative to estimated body angle
    */
    AngleDeg queuedNextAngleFromBody( const Vector2D & target ) const;

    /*!
      \brief get next view width using queued action effect
      \return queued view width
    */
    ViewWidth queuedNextViewWidth() const;

    /*!
      \brief get the next focus distance estimated by the queued acction effect
      \return queued focus distance
     */
    //double queuedNextFocusDist() const;

    /*!
      \brief get the next focus direction estimated by the queued acction effect
      \return queued focus direction
     */
    //AngleDeg queuedNextFocusDir() const;

    /*!
      \brief check if the target point can see only by turn_neck with the buffer
      \param point target point
      \param angle_buf angle buffer for the half view width
      \return true if player can see the point only by turn_neck
     */
    bool queuedNextCanSeeWithTurnNeck( const Vector2D & point,
                                       const double & angle_buf ) const;

    /*!
      \brief get cycles till next see message arrival using queued action effect
      \return cycle value till next see message
     */
    int queuedNextSeeCycles() const;

private:

    /*!
      \brief create say command object using the registered say message objects
     */
    void makeSayCommand();

};

}

#endif
