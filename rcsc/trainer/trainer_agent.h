// -*-c++-*-

/*!
  \file trainer_agent.h
  \brief basic trainer agent Header File
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

#ifndef RCSC_TRAINER_TRAINER_AGENT_H
#define RCSC_TRAINER_TRAINER_AGENT_H

#include <rcsc/trainer/trainer_config.h>
#include <rcsc/coach/coach_world_model.h>
#include <rcsc/common/soccer_agent.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <memory>
#include <string>

namespace rcsc {

class TrainerCommand;
class CoachVisualSensor;

/*!
  \class TrainerAgent
  \brief basic trainer agent class
*/
class TrainerAgent
    : public SoccerAgent {
private:

    struct Impl; //!< pimpl idiom

    //! internal implementation object
    std::unique_ptr< Impl > M_impl;

protected:

    //! configuration parameter set
    TrainerConfig M_config;

    //! internal memory of field status
    CoachWorldModel M_worldmodel;

public:
    /*!
      \brief initialize member variables
    */
    TrainerAgent();

    /*!
      \brief delete dynamic allocated memory
    */
    virtual
    ~TrainerAgent();

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
    const TrainerConfig & config() const
      {
          return M_config;
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
      \brief get the analyzed visual info
      \return const reference to the visual sensor instance
     */
    const CoachVisualSensor & visualSensor() const;

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

    /*!
      \brief send eye command
      \brief on if true, send (eye on), else (eye off)
      \return true if command is generated and sent
    */
    bool doEye( bool on );

    /*!
      \brief send ear command
      \brief on if true, send (ear on), else (ear off)
      \return true if command is generated and sent
    */
    bool doEar( bool on );

    /*!
      \brief send start command to kickoff the game
      \return true if command is generated and sent
    */
    bool doKickOff();

    /*!
      \brief send ball move command
      \param pos new position
      \param vel new velocity
      \return true if command is generated and sent
    */
    bool doMoveBall( const Vector2D & pos,
                     const Vector2D & vel );

    /*!
      \brief send player move command
      \param teamname target player's team name
      \param unum target player's uniform number
      \param pos new position
      \return true if command is generated and sent
    */
    bool doMovePlayer( const std::string & teamname,
                       const int unum,
                       const Vector2D & pos );

    /*!
      \brief send player move command
      \param teamname target player's team name
      \param unum target player's uniform number
      \param pos new position
      \param angle new body angle
      \return true if command is generated and sent
    */
    bool doMovePlayer( const std::string & teamname,
                       const int unum,
                       const Vector2D & pos,
                       const AngleDeg & angle );

    /*!
      \brief send recover command
      \return true if command is generated and sent
    */
    bool doRecover();

    /*!
      \brief send playmode change command
      \param mode new playmode Id
      \return true if command is generated and sent
    */
    bool doChangeMode( const PlayMode mode );

    /*!
      \brief send change_player_type command
      \param teamname target player's team name
      \param unum target player's uniform number
      \param type new player type Id
      \return true if command is generated and sent
    */
    bool doChangePlayerType( const std::string & teamname,
                             const int unum,
                             const int type );

    /*!
      \brief send say command (coach language)
      \return true if command is generated and sent
    */
    bool doSay( const std::string & msg );

private:

    /*!
      \brief launch each parsing method depending on the message type
      \param msg raw server message
    */
    void parse( const char * msg );


    /*!
      \brief main action decision. this method is called from handleMessage()
    */
    void action();

    /*!
      \brief send command string to the rcssserver
      \param com trainer command object
      \return true if command is sent
    */
    bool sendCommand( const TrainerCommand & com );

protected:
    /*!
      \brief analyze command line options
      \param cmd_parser command line parser object
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
      \brief handle server message event

      Do NOT call this method by yourself!
    */
    virtual
    void handleMessage();

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

    /*!
      \brief pure virtual method. register decision.

      This method is used to set trainer's action.
      This method is called from action().
      So, do *NOT* call this method by yourself.
    */
    virtual
    void actionImpl() = 0;


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

};

}

#endif
