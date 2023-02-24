// -*-c++-*-

/*!
  \file player_agent.h
  \brief basic player agent Header File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA, Hiroki SHIMORA

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

#ifndef RCSC_PLAYER_AGENT_H
#define RCSC_PLAYER_AGENT_H

#include <rcsc/player/world_model.h>
#include <rcsc/player/action_effector.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/player_config.h>
#include <rcsc/player/see_state.h>
#include <rcsc/common/soccer_agent.h>
#include <rcsc/timer.h>
#include <rcsc/types.h>

#include <memory>
#include <vector>

namespace rcsc {

class AudioSensor;
class ArmAction;
class BodySensor;
class FullstateSensor;
class FreeformMessageParser;
class SayMessage;
class SayMessageParser;
class SeeState;
class SoccerIntention;
class NeckAction;
class ViewAction;
class FocusAction;
class VisualSensor;

/*!
  \class PlayerAgent
  \brief basic player agent class
*/
class PlayerAgent
    : public SoccerAgent {
private:

    struct Impl; //!< pimpl idiom

    //! internal implementation object
    std::unique_ptr< Impl > M_impl;

protected:

    //! configuration parameter set
    PlayerConfig M_config;

    //! debug client interface
    DebugClient M_debug_client;

    ///////////////////////////////

    //! mental memory of world status
    WorldModel M_worldmodel;

    //! mental memory of fullstate world status
    WorldModel M_fullstate_worldmodel;

    //! action info manager
    ActionEffector M_effector;

public:
    /*!
      \brief create internal modules
    */
    PlayerAgent();

    /*!
      \brief virtual destructor
    */
    virtual
    ~PlayerAgent();

    /*!
      \brief create a client object (online or offline) according to the command line option.
      \return client object pointer.
     */
    virtual
    std::shared_ptr< AbstractClient > createConsoleClient();

    /*!
      \brief finalize all things when the process exits
    */
    void finalize();

    /*!
      \brief get configuration set
      \return const reference to the configuration class object
    */
    const PlayerConfig & config() const
      {
          return M_config;
      }

    /*!
      \brief get debug client interface
      \return reference to the DebugClient object
    */
    DebugClient & debugClient()
      {
          return M_debug_client;
      }

    /*!
      \brief get worldmodel
      \return const reference to world model instance
    */
    const WorldModel & world() const
      {
          return M_worldmodel;
      }

    /*!
      \brief get fullstate worldmodel
      \return const reference to fullstate world model instance
    */
    const WorldModel & fullstateWorld() const
      {
          return M_fullstate_worldmodel;
      }

    /*!
      \brief get action effector
      \return reference to action effector
    */
    const ActionEffector & effector() const
      {
          return M_effector;
      }

    /*!
      \brief get body sensor
      \return const reference to the body sensor instance
     */
    const BodySensor & bodySensor() const;

    /*!
      \brief get visual sensor
      \return const reference to the visual sensor instance
     */
    const VisualSensor & visualSensor() const;

    /*!
      \brief get audio sensor
      \return const reference to the audio sensor instance
     */
    const AudioSensor & audioSensor() const;

    /*!
      \brief get fullstate sensor
      \return const reference to the fullstate sensor instance
     */
    const FullstateSensor & fullstateSensor() const;

    /*!
      \brief get see state
      \return const reference to the see state instance
     */
    const SeeState & seeState() const;

    /*!
      \brief get time stamp when sense_body message is received
      \return const reference to the time stamp object
    */
    const TimeStamp & bodyTimeStamp() const;

    /*!
      \brief get time stamp of see message when see message is received
      \return const reference to the time stamp object
    */
    const TimeStamp & seeTimeStamp() const;

    /*!
      \brief register kick command
      \param power command argument: kick power
      \param rel_dir command argument kick direction relative to body angle
      \return true if successfully registered.
    */
    bool doKick( const double & power,
                 const AngleDeg & rel_dir );

    /*!
      \brief register dash command
      \param power command argument: dash power
      \param rel_dir command argument: dash direction relative to body (or reverse body) angle
      \return true if successfully registered.
    */
    bool doDash( const double & power,
                 const AngleDeg & rel_dir = 0.0 );

    /*!
      \brief register turn command
      \param moment command argument: turn moment
      \return true if successfully registered.
    */
    bool doTurn( const AngleDeg & moment );

    /*!
      \brief register catch command. catch direction is automatically calculated.
      \return true if successfully registered.
    */
    bool doCatch();

    /*!
      \brief register move command
      \param x move target x
      \param y move target y
      \return true if successfully registered.
    */
    bool doMove( const double & x,
                 const double & y );

    /*!
      \brief register tackle command
      \param power_or_dir tackle power or direction
      \param foul foul mode switch
      \return true if successfully registered.
    */
    bool doTackle( const double & power_or_dir,
                   const bool foul = false );

    /*!
      \brief register turn_neck command.
      \param moment turn neck moment
      \return true if successfully registered.
    */
    bool doTurnNeck( const AngleDeg & moment );

    /*!
      \brief register change_view command.
      \param width new view width
      \return true if successfully registered.

      ViewQuality should not be changed by user
    */
    bool doChangeView( const ViewWidth & width );

    /*!
      \brief register change_focus command
      \param moment_dist distance added to the current focus point
      \param moment_dir direction added to the current focus point
     */
    bool doChangeFocus( const double moment_dist,
                        const AngleDeg & moment_dir );

    /*
      brief register say command.
      param msg message string
      return true if successfully registered.
    */
    //bool doSay( const std::string & msg );

    /*!
      \brief register pointto command.
      \param x target point x
      \param y target point y
      \return true if successfully registered.
    */
    bool doPointto( const double & x,
                    const double & y );

    /*!
      \brief register pointto command. turn off mode
      \return true if successfully registered.
    */
    bool doPointtoOff();

    /*!
      \brief register attentionto command by off mode
      \param side target player's side
      \param unum target player's uniform number
      \return true if successfully registered.
    */
    bool doAttentionto( SideID side,
                        const int unum );

    /*!
      \brief register attentionto command. turn off mode
      \return true if successfully registered.
    */
    bool doAttentiontoOff();


    /*!
      \brief reserve pointto action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setArmAction( ArmAction * act );

    /*!
      \brief reserve turn neck action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setNeckAction( NeckAction * act );

    /*!
      \brief reserve change view action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setViewAction( ViewAction * act );

    /*!
      \brief reserve change_focus action
      \param act pointer to the action. must be a dynamically allocated object.
    */
    void setFocusAction( FocusAction * act );


    /*!
      \brief add say message to the action effector
      \param message pointer to the dynamically allocated object.
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
      \brief set intention object
      \param intention pointer to the dynamically allocated object.
    */
    void setIntention( SoccerIntention * intention );

    /*!
      \brief execute queued intention if exist.
      \retval true if action is executed
      \retval false if queue is empty, or action is failed.
    */
    bool doIntention();

private:

    /*!
      \brief parse server message.
      \param msg raw server message
    */
    void parse( const char * msg );

    /*!
      \brief main action decision.
    */
    void action();

protected:

    /*!
      \brief analyze command line options
      \param cmd_parser command line parser object
      \return false only if "help" option is given, othewise true.

      This method is called from SoccerAgent::init( argc, argv )
      SoccerAgent::init(argc,argv) should be called in main().
    */
    virtual
    bool initImpl( CmdLineParser & cmd_parser );

    /*!
      \brief handle start event
      \return status of start procedure.

      This method is called when client starts to run.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
     */
    virtual
    bool handleStart();

    /*!
      \brief handle start event in offline client mode.
      \return status of start procedure.

      This method is called at the top of AbstractClient::run() method.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
     */
    virtual
    bool handleStartOffline();

    /*!
      \brief handle server message event

      This method is called from AbstractClient::run() method.
      Do NOT call this method by yourself!
    */
    virtual
    void handleMessage();

    /*!
      \brief handle offline client log message in offline client mode.

      This method is called when offline client log message is read.
     */
    virtual
    void handleMessageOffline();

    /*!
      \brief handle timeout event
      \param timeout_count count of timeout without sensory message.
      \param waited_msec elapsed milliseconds since last sensory message.
      This method is called from AbstractClient::run() method.
      Do NOT call this method by yourself!
    */
    virtual
    void handleTimeout( const int timeout_count,
                        const int waited_msec );

    /*!
      \brief handle exit event
    */
    virtual
    void handleExit();

    //
    //
    //

    /*!
      \brief pure virtual method. register body action to ActionEffector.

      This method is used to set player's body action.
      Do *not* call this method by yourself because this method is called automatically.
    */
    virtual
    void actionImpl() = 0;

    /*!
      \brief virtual method. register say action to ActionEffector

      This method is called just after turn_neck action.
      Do *not* call this method by yourself because this method is called automatically.
    */
    virtual
    void communicationImpl()
      { }


    /*!
      \brief This method is called at the top of action().
      Do *not* call this method by yourself.
    */
    virtual
    void handleActionStart()
      { }

    /*!
      \brief This method is called at the end of action() but before the debug output.
      Do *not* call this method by yourself.
    */
    virtual
    void handleActionEnd()
      { }

    /*!
      \brief this method is called just after analyzing init message.
      Do *not* call this method by yourself.
     */
    virtual
    void handleInitMessage()
      { }

    /*!
      \brief this method is called just after analyzing server_param message.
      Do *not* call this method by yourself.
     */
    virtual
    void handleServerParam()
      { }

    /*!
      \brief this method is called just after analyzing player_param message.
      Do *not* call this method by yourself.
     */
    virtual
    void handlePlayerParam()
      { }

    /*!
      \brief this method is called just after analyzing player_type message.
      Do *not* call this method by yourself.
     */
    virtual
    void handlePlayerType()
      { }

    /*!
      \brief this method is called just after analyzing online coach's say message.
      Do *not* call this method by yourself.
     */
    virtual
    void handleOnlineCoachAudio()
      { }

    //
    //
    //

    /*!
      \brief register new say message parser object
      \param parser pointer to the dynamically allocated parser object.
     */
    void addSayMessageParser( SayMessageParser * parser );

    /*!
      \brief remove registered parser object
      \param header say message header character
     */
    void removeSayMessageParser( const char header );

    /*!
      \brief set new freeform message parser
      \param parser pointer to the dynamically allocated parser object.
     */
    void addFreeformMessageParser( FreeformMessageParser * parser );

    /*!
      \brief remove registered parser object
      \param type freeform message type string
     */
    void removeFreeformMessageParser( const std::string & type );


};

}

#endif
