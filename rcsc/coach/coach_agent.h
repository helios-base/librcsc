// -*-c++-*-

/*!
  \file coach_agent.h
  \brief basic coach agent Header File
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

#ifndef RCSC_COACH_COACH_AGENT_H
#define RCSC_COACH_COACH_AGENT_H

#include <rcsc/coach/coach_world_model.h>
#include <rcsc/coach/coach_debug_client.h>
#include <rcsc/coach/coach_config.h>
#include <rcsc/common/freeform_message.h>
#include <rcsc/common/soccer_agent.h>
#include <rcsc/common/team_graphic.h>

#include <memory>
#include <string>
#include <set>

namespace rcsc {

class CLangMessage;
class CoachAudioSensor;
class CoachCommand;
class GlobalVisualSensor;
class SayMessageParser;

/*!
  \class CoachAgent
  \brief abstract coach agent class
 */
class CoachAgent
    : public SoccerAgent {
private:

    struct Impl; //!< pimpl idiom

    //! internal implementation object
    std::unique_ptr< Impl > M_impl;

protected:

    //! configuration parameter set
    CoachConfig M_config;

    //! debug client interface
    CoachDebugClient M_debug_client;

    //! internal memory of field status
    CoachWorldModel M_worldmodel;

public:
    /*!
      \brief init member variables
     */
    CoachAgent();
    /*!
      \brief delete dynamic allocated memory
     */
    virtual
    ~CoachAgent();

    /*!
      \brief create a client object (online or offline) according to the command line option.
      \return client object pointer.
     */
    virtual
    std::shared_ptr< AbstractClient > createConsoleClient();

    /*!
      \brief finalize program process
     */
    void finalize();

    /*!
      \brief finalize all things when the process exits
    */
    const
    CoachConfig & config() const
      {
          return M_config;
      }


    /*!
      \brief get debug client interface
      \return reference to the DebugClient object
    */
    CoachDebugClient & debugClient()
      {
          return M_debug_client;
      }

    /*!
      \brief get field status
      \return const reference to the worldmodel instance
     */
    const CoachWorldModel & world() const
      {
          return M_worldmodel;
      }

    /*!
      \brief get visual sensor.
      \return const reference to the visual sensor instance.
     */
    const CoachVisualSensor & visualSensor() const;

    /*!
      \brief get audio sensor
      \return const reference to the audio sensor instance
     */
    const CoachAudioSensor & audioSensor() const;

    /*!
      \brief get team_graphic ok flags
      \return const reference to the flag container
    */
    const std::set< TeamGraphic::Index > & teamGraphicOKSet() const;

    /*!
      \brief send check_ball command
      \return true if command is generated and sent
    */
    bool doCheckBall();

    /*!
      \brief send look command
      \return true if command is generated and sent
    */
    bool doLook();

    /*!
      \brief send team_name command
      \return true if command is generated and sent
    */
    bool doTeamNames();

    //bool doTeamGraphic();

    /*!
      \brief send eye command
      \brief on if true, send (eye on), else (eye off)
      \return true if command is generated and sent
    */
    bool doEye( bool on );

    /*!
      \brief send change_player_type command
      \param unum target player's uniform number
      \param type new player type Id
      \return true if command is generated and sent
    */
    bool doChangePlayerType( const int unum,
                             const int type );

    /*!
      \brief send change_player_types command
      \param types player change pair list
      \return true if command is generated and sent
    */
    bool doChangePlayerTypes( const std::vector< std::pair< int, int > > & types );

    /*
      \brief send freeform message by say command
      \return true if command is generated and sent
    */
    // bool doSayFreeform( const std::string & msg );

    /*!
      \brief add freeform message to the message queue.
      \param message pointer to the dynamically allocated message object
     */
    void addFreeformMessage( FreeformMessage::Ptr message );

    /*!
      \brief remove the registered say message if exist
      \param header message header character
      \return true if removed
     */
    bool removeFreeformMessage( const std::string & type );

    /*!
      \brief set clang message to the queue
      \param msg new clang message object
     */
    void doSendCLang( const CLangMessage * msg );

    /*!
      \brief send team_graphic command
      \return true if command is generated and sent
    */
    bool doTeamGraphic( const int x,
                        const int y,
                        const TeamGraphic & team_graphic );

private:

    /*!
      \brief parse server message.
      \param msg raw server message
     */
    void parse( const char * msg );

    /*!
      \brief main action decision. this method is called from handleMessage()
    */
    void action();

    /*!
      \brief send command string to the rcssserver
      \param com coach command object
      \return true if command is sent
     */
    bool sendCommand( const CoachCommand & com );

protected:

    /*!
      \brief analyze command line options
      \param cmd_parser command lien parser object
      \return only if "help" option is given, false is returned.

      This method is called from SoccerAgent::init( argc, argv )
      SoccerAgent::init(argc,argv) should be called in main().
      Do NOT call this method by yourself!
     */
    virtual
    bool initImpl( CmdLineParser & cmd_parser );

    /*!
      \brief handle start event
      \return status of start procedure.

      This method is called on the top of BasicClient::run() method.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
    */
    virtual
    bool handleStart();

    /*!
      \brief handle start event in offline client mode.
      \return status of start procedure.

      This method is called on the top of BasicClient::run() method.
      The concrete agent must connect to the server and send init command.
      Do NOT call this method by yourself!
    */
    virtual
    bool handleStartOffline();

    /*!
      \brief handle server message event

      Do NOT call this method by yourself!
     */
    virtual
    void handleMessage();


    /*!
      \brief handle offline client log messages in offline client mode.

      Do NOT call this method by yourself!
     */
    virtual
    void handleMessageOffline();

    /*!
      \brief handle timeout event
      \param timeout_count count of timeout without sensory message.
      \param waited_msec elapsed milli seconds sinc last sensory message.

      This method is called when select() timeout occurs
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
      \brief pure virtual method. the decision making procedure implemented by team developper.
      This method is automatically called.
      Do NOT call this method by yourself.
    */
    virtual
    void actionImpl() = 0;


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
      Do NOT call this method by yourself.
     */
    virtual
    void handleInitMessage()
      { }

    /*!
      \brief this method is called just after analyzing server_param message.
      Do NOT call this method by yourself.
     */
    virtual
    void handleServerParam()
      { }

    /*!
      \brief this method is called just after analyzing player_param message.
      Do NOT call this method by yourself.
     */
    virtual
    void handlePlayerParam()
      { }

    /*!
      \brief this method is called just after analyzing player_type message.
      Do NOT call this method by yourself.
     */
    virtual
    void handlePlayerType()
      { }

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

};

}

#endif
