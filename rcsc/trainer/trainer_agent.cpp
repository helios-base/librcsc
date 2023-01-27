// -*-c++-*-

/*!
  \file trainer_agent.cpp
  \brief basic trainer agent Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "trainer_agent.h"

#include "trainer_command.h"

#include <rcsc/coach/coach_visual_sensor.h>

#include <rcsc/common/abstract_client.h>
#include <rcsc/common/online_client.h>
#include <rcsc/common/offline_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/audio_memory.h>

#include <rcsc/param/param_map.h>
#include <rcsc/param/conf_file_parser.h>
#include <rcsc/param/cmd_line_parser.h>
#include <rcsc/version.h>

#include <sstream>
#include <cstdio>
#include <cstring>

namespace rcsc {

///////////////////////////////////////////////////////////////////////
/*!
  \struct Impl
  \brief trainer agent implementation
*/
struct TrainerAgent::Impl {

    //! reference to the agent instance
    TrainerAgent & agent_;

    //! flag to check if (think) message was received or not.
    bool think_received_;

    //! flag to check if server cycle is stopped or not.
    bool server_cycle_stopped_;

    //! last action decision game time
    GameTime last_decision_time_;

    //! current game time
    GameTime current_time_;

    //! referee info
    GameMode game_mode_;

    //! visual sensor data
    CoachVisualSensor visual_;

    /*!
      \brief initialize all members
    */
    Impl( TrainerAgent & agent )
        : agent_( agent ),
          think_received_( false ),
          server_cycle_stopped_( true ),
          last_decision_time_( -1, 0 ),
          current_time_( 0, 0 )
      { }

    /*!
      \brief initialize debug resources
     */
    void initDebug();

    /*!
      \brief open offline client log file.
    */
    bool openOfflineLog();

    /*!
      \brief open debug log file.
    */
    bool openDebugLog();

    /*!
      \brief set debug output flags to logger
     */
    void setDebugFlags();

    /*!
      \brief send init or reconnect command to server

      init commad is sent in AbstractClient's run() method
      Do not call this method yourself!
    */
    void sendInitCommand();

    /*!
      \brief send client setting commands(compression...) to server
     */
    void sendSettingCommands();

    /*!
      \brief send disconnection command message to server
      and set the server status to end.
    */
    void sendByeCommand();


    /*!
      \brief analyze init replay message
      \param msg raw server message
    */
    void analyzeInit( const char * msg );

    /*!
      \brief analyze cycle info in server message
      \param msg raw server message
      \param by_see_global if message type is see_global, this value becomes true
    */
    bool analyzeCycle( const char * msg,
                       const bool by_see_global );

    /*!
      \brief analyze see_global message
      \param msg raw server message
    */
    void analyzeSeeGlobal( const char * msg );

    /*!
      \brief analyze hear message
      \param msg raw server message
    */
    void analyzeHear( const char * msg );

    /*!
      \brief analyze referee message
      \param msg raw server message
    */
    void analyzeHearReferee( const char * msg );

    /*!
      \brief analyze audio message from player
      \param msg raw server message
    */
    void analyzeHearPlayer( const char * msg );

    /*!
      \brief analyze change_player_type message
      \param msg raw server message
    */
    void analyzeChangePlayerType( const char * msg );

    /*!
      \brief analyze player_type parameter message
      \param msg raw server message
    */
    void analyzePlayerType( const char * msg );

    /*!
      \brief analyze player_param parameter message
      \param msg raw server message
    */
    void analyzePlayerParam( const char * msg );

    /*!
      \brief analyze server_param parameter message
      \param msg raw server message
    */
    void analyzeServerParam( const char * msg );

    /*!
      \brief analyze ok message
      \param msg raw server message
    */
    void analyzeOK( const char * msg );

    /*!
      \brief analyze ok teamnames message
      \param msg raw server message
     */
    void analyzeTeamNames( const char * msg );

    /*!
      \brief analyze error message
      \param msg raw server message
    */
    void analyzeError( const char * msg );

    /*!
      \brief analyze warning message
      \param msg raw server message
    */
    void analyzeWarning( const char * msg );

    /*!
      \brief update current time using analyzed time value
      \param new_time analyzed time value
      \param by_see_global true if called after see_global message
    */
    void updateCurrentTime( const long new_time,
                            const bool by_see_global );

    /*!
      \brief update server game cycle status.

      This method must be called just after referee message
    */
    void updateServerStatus();
};

/*-------------------------------------------------------------------*/
/*!

 */
void
TrainerAgent::Impl::initDebug()
{
    if ( ! agent_.config().offlineClientMode() )
    {
        if ( agent_.config().offlineLogging() )
        {
            openOfflineLog();
        }
    }

    if ( agent_.config().debug() )
    {
        openDebugLog();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::Impl::openOfflineLog()
{
    std::string filepath = agent_.config().logDir();

    if ( ! filepath.empty() )
    {
        if ( *filepath.rbegin() != '/' )
        {
            filepath += '/';
        }
    }

    filepath += agent_.config().teamName();
    filepath += "-trainer";
    filepath += agent_.config().offlineLogExt();

    if ( ! agent_.M_client->openOfflineLog( filepath ) )
    {
        std::cerr << agent_.config().teamName() << " trainer: "
                  << "Failed to open the offline client log file ["
                  << filepath << "]" << std::endl;
        agent_.M_client->setServerAlive( false );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::Impl::openDebugLog()
{
    std::string filepath = agent_.config().logDir();

    if ( ! filepath.empty() )
    {
        if ( *filepath.rbegin() != '/' )
        {
            filepath += '/';
        }
    }

    filepath += agent_.config().teamName();
    filepath += "-trainer";
    filepath += agent_.config().debugLogExt();

    dlog.open( filepath );

    if ( ! dlog.isOpen() )
    {
        std::cerr << agent_.config().teamName() << " trainer: "
                  << " Failed to open the debug log file [" << filepath << "]"
                  << std::endl;
        agent_.M_client->setServerAlive( false );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::setDebugFlags()
{
    const TrainerConfig & c = agent_.config();

    if ( ! c.debug() )
    {
        return;
    }

    dlog.setLogFlag( &current_time_, Logger::SYSTEM, c.debugSystem() );
    dlog.setLogFlag( &current_time_, Logger::SENSOR, c.debugSensor() );
    dlog.setLogFlag( &current_time_, Logger::WORLD, c.debugWorld() );
    dlog.setLogFlag( &current_time_, Logger::ACTION, c.debugAction() );
    dlog.setLogFlag( &current_time_, Logger::INTERCEPT, c.debugIntercept() );
    dlog.setLogFlag( &current_time_, Logger::KICK, c.debugKick() );
    dlog.setLogFlag( &current_time_, Logger::HOLD, c.debugHold() );
    dlog.setLogFlag( &current_time_, Logger::DRIBBLE, c.debugDribble() );
    dlog.setLogFlag( &current_time_, Logger::PASS, c.debugPass() );
    dlog.setLogFlag( &current_time_, Logger::CROSS, c.debugCross() );
    dlog.setLogFlag( &current_time_, Logger::SHOOT, c.debugShoot() );
    dlog.setLogFlag( &current_time_, Logger::CLEAR, c.debugClear() );
    dlog.setLogFlag( &current_time_, Logger::BLOCK, c.debugBlock() );
    dlog.setLogFlag( &current_time_, Logger::MARK, c.debugMark() );
    dlog.setLogFlag( &current_time_, Logger::POSITIONING, c.debugPositioning() );
    dlog.setLogFlag( &current_time_, Logger::ROLE, c.debugRole() );
    dlog.setLogFlag( &current_time_, Logger::PLAN, c.debugPlan() );
    dlog.setLogFlag( &current_time_, Logger::TEAM, c.debugTeam() );
    dlog.setLogFlag( &current_time_, Logger::COMMUNICATION, c.debugCommunication() );
    dlog.setLogFlag( &current_time_, Logger::ANALYZER, c.debugAnalyzer() );
    dlog.setLogFlag( &current_time_, Logger::ACTION_CHAIN, c.debugActionChain() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::updateCurrentTime( const long new_time,
                                       const bool by_see_global )
{
    if ( server_cycle_stopped_ )
    {
        if ( new_time != current_time_.cycle() )
        {
            if ( new_time - 1 != current_time_.cycle() )
            {
                std::cerr << "trainer: server cycle stopped mode:"
                          << " previous server time is incorrect?? "
                          << current_time_ << " -> " << new_time
                          << std::endl;
            }

            current_time_.assign( new_time, 0 );
        }
        else
        {
            if ( by_see_global )
            {
                current_time_.assign( current_time_.cycle(),
                                      current_time_.stopped() + 1 );
            }
        }
    }
    // normal case
    else
    {
        current_time_.assign( new_time, 0 );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::updateServerStatus()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  called just after referee.parse()

    // if current mode is stopped mode,
    // stopped flag is cleared.
    if ( server_cycle_stopped_ )
    {
        server_cycle_stopped_ = false;
    }

    if ( game_mode_.isServerCycleStoppedMode() )
    {
        server_cycle_stopped_ = true;
    }
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
TrainerAgent::TrainerAgent()
    : SoccerAgent(),
      M_impl( new Impl( *this ) ),
      M_config()
{
    M_worldmodel.init( M_config.teamName(), NEUTRAL, 999 );

    std::shared_ptr< AudioMemory > audio_memory( new AudioMemory );

    M_worldmodel.setAudioMemory( audio_memory );
}

/*-------------------------------------------------------------------*/
/*!

*/
TrainerAgent::~TrainerAgent()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
std::shared_ptr< AbstractClient >
TrainerAgent::createConsoleClient()
{
    std::shared_ptr< AbstractClient > ptr;

    if ( config().offlineClientMode() )
    {
        ptr = std::shared_ptr< AbstractClient >( new OfflineClient() );
    }
    else
    {
        ptr = std::shared_ptr< AbstractClient >( new OnlineClient() );
    }

    return ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
CoachVisualSensor &
TrainerAgent::visualSensor() const
{
    return M_impl->visual_;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::initImpl( CmdLineParser & cmd_parser )
{
    bool help = false;
    std::string trainer_config_file;

    ParamMap system_param_map( "System options" );
    system_param_map.add()
        ( "help" , "", BoolSwitch( &help ), "print help message.")
        ( "trainer-config", "", &trainer_config_file, "specifies trainer config file." );

    // analyze command line for system options.
    cmd_parser.parse( system_param_map );
    if ( help )
    {
        std::cout << copyright() << std::endl;
        system_param_map.printHelp( std::cout );
        config().printHelp( std::cout );
        return false;
    }

    // parse config file
    if ( ! trainer_config_file.empty() )
    {
        ConfFileParser conf_parser( trainer_config_file.c_str() );
        M_config.parse( conf_parser );
    }

    // parse command line
    M_config.parse( cmd_parser );

    if ( config().version() < 1.0
         || MAX_PROTOCOL_VERSION < config().version() )
    {
        std::cerr << "(TrainerAgent::initImpl) Unsupported client version: " << config().version()
                  << std::endl;
        return false;
    }

    M_impl->setDebugFlags();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::handleStart()
{
    if ( ! M_client )
    {
        return false;
    }

    if ( config().host().empty() )
    {
        std::cerr << "trainer: ***ERROR*** server host name is empty"
                  << std::endl;
        M_client->setServerAlive( false );
        return false;
    }

    // just create a connection. init command is automaticcaly sent
    // by AbstractClient's run() method.
    if ( ! M_client->connectTo( config().host().c_str(),
                                config().port() ) )
    {
        std::cerr << "trainer: ***ERROR*** failed to connect." << std::endl;
        M_client->setServerAlive( false );
        return false;
    }

    M_client->setIntervalMSec( config().intervalMSec() );

    M_impl->sendInitCommand();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::handleMessage()
{
    if ( ! M_client )
    {
        std::cerr << "TrainerAgent::handleMessage(). Client is not registered."
                  << std::endl;
        return;
    }

    // start parse process

    while ( M_client->receiveMessage() > 0 )
    {
        parse( M_client->message() );
    }

    if ( M_impl->think_received_ )
    {
        action();
    }
    else if ( ! ServerParam::i().synchMode() )
    {
        if ( M_impl->last_decision_time_ != M_impl->current_time_
             && M_impl->visual_.time() == M_impl->current_time_ )
        {
            action();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::handleTimeout( const int /*timeout_count*/,
                             const int waited_msec )
{
    if ( ! M_client )
    {
        std::cerr << "TrainerAgent::handleTimeout(). Client is not registered."
                  << std::endl;
        return;
    }

    if ( waited_msec > config().serverWaitSeconds() * 1000 )
    {
        M_client->setServerAlive( false );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::handleExit()
{
    finalize();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::finalize()
{
    if ( M_client->isServerAlive() )
    {
        M_impl->sendByeCommand();
    }
    std::cerr << "trainer: finished."<< std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::parse( const char * msg )
{

    if ( ! std::strncmp( msg, "(see_global ", strlen("(see_global ")) )
    {
        M_impl->analyzeSeeGlobal( msg );
    }
    else if ( ! std::strncmp( msg, "(hear ", 6 ) )
    {
        M_impl->analyzeHear( msg );
    }
    else if ( ! std::strncmp( msg, "(think)", 7 ) )
    {
        M_impl->think_received_ = true;
    }
    else if ( ! std::strncmp( msg, "(change_player_type ", 20 ) )
    {
        M_impl->analyzeChangePlayerType( msg );
    }
    else if ( ! std::strncmp( msg, "(player_type ", 13 ) )  // hetero param
    {
        M_impl->analyzePlayerType( msg );
    }
    else if ( ! std::strncmp( msg, "(player_param ", 14 ) ) // tradeoff param
    {
        M_impl->analyzePlayerParam( msg );
    }
    else if ( ! std::strncmp( msg, "(server_param ", 14 ) )
    {
        M_impl->analyzeServerParam( msg );
    }
    else if ( ! std::strncmp( msg, "(ok ", 4 ) )
    {
        M_impl->analyzeOK( msg );
    }
    else if ( ! std::strncmp( msg, "(error ", 7 ) )
    {
        M_impl->analyzeError( msg );
    }
    else if ( ! std::strncmp( msg, "(warning ", 9 ) )
    {
        M_impl->analyzeWarning( msg );
    }
    else if ( ! std::strncmp( msg, "(score ", 7 ) )
    {
        std::cerr << "trainer: "
                  << M_impl->current_time_
                  << " recv score " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(init ", 6 )
              || ! std::strncmp( msg, "(reconnect ", 11 ) )
    {
        M_impl->analyzeInit( msg );
    }
    else
    {
        std::cerr << "trainer:"
                  << M_impl->current_time_
                  << " received unsupported Message : ["
                  << msg << "]" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeInit( const char * msg )
{
    if ( std::strncmp( msg, "(init ok)", std::strlen( "(init ok)" ) ) )
    {
        std::cerr << "trainer: Failed to init trainer.. init reply message=["
                  << msg << ']'
                  << std::endl;
        agent_.M_client->setServerAlive( false );
        return;
    }

    sendSettingCommands();

    //
    // call init message event handler
    //
    agent_.handleInitMessage();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::Impl::analyzeCycle( const char * msg,
                                  const bool by_see_global )
{
    char buf[128];
    long cycle = 0;

    if ( std::sscanf( msg, "(%s %ld ",
                      buf, &cycle ) != 2
         && std::sscanf( msg, "(hear (%127[^()]) %ld ",
                         buf, &cycle ) != 2
         && std::sscanf( msg, "(hear %127s %ld ",
                         buf, &cycle ) != 2 )
    {
        std::cerr << "trainer: time parse error msg=["
                  << msg << "]"
                  << std::endl;
        return false;
    }

    updateCurrentTime( cycle, by_see_global );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeSeeGlobal( const char * msg )
{
    if ( ! analyzeCycle( msg, true ) )
    {
        return;
    }

    visual_.parse( msg,
                   agent_.config().version(),
                   current_time_ );

    // update world model
    if ( visual_.time() == current_time_
         && agent_.world().time() != current_time_ )
    {
        agent_.M_worldmodel.updateAfterSeeGlobal( visual_,
                                                  current_time_ );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeHear( const char * msg )
{
    //std::cerr << "Trainer hear " << msg << std::endl;
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    long cycle;
    char sender[128];

    if ( std::sscanf( msg, "(hear %ld (%127[^()]) ",
                      &cycle, sender ) != 2
         && std::sscanf( msg, "(hear %ld %s ",
                         &cycle, sender ) != 2
         && std::sscanf( msg, "(hear (%127[^()]) %ld ",
                         sender, &cycle ) != 2
         && std::sscanf( msg, "(hear %127s %ld ",
                         sender, &cycle ) != 2 )
    {
        std::cerr << "trainer: "
                  << current_time_
                  << " *** ERROR *** failed to parse hear sender. ["
                  << msg << std::endl;
        return;
    }

    // check sender name
    if ( ! std::strcmp( sender, "referee" ) )
    {
        analyzeHearReferee( msg );
    }
    else
    {
        // player message
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeHearReferee( const char * msg )
{
    long cycle;
    char mode[512];

    if ( std::sscanf( msg, "(hear %ld referee %511[^)]", &cycle, mode ) != 2
         && std::sscanf( msg, "(hear referee %ld %511[^)]", &cycle, mode ) != 2 )
    {
        std::cerr << "trainer: "
                  << current_time_
                  << " playmode scan error. "<< msg
                  << std::endl;
        return;
    }

    if ( ! game_mode_.update( mode, current_time_ ) )
    {
        if ( ! std::strncmp( mode, "yellow_card", std::strlen( "yellow_card" ) ) )
        {
            char side = '?';
            int unum = Unum_Unknown;
            if ( std::sscanf( mode, "yellow_card_%c_%d", &side, &unum ) != 2 )
            {
                std::cerr << agent_.config().teamName()
                          << " coach: "
                          << agent_.world().time()
                          << " could not parse the yellow card message [" << msg << ']'
                          << std::endl;
            }

            agent_.M_worldmodel.setCard( ( side == 'l' ? LEFT : side == 'r' ? RIGHT : NEUTRAL ),
                                         unum,
                                         YELLOW );
        }
        else if ( ! std::strncmp( mode, "red_card", std::strlen( "red_card" ) ) )
        {
            char side = '?';
            int unum = Unum_Unknown;
            if ( std::sscanf( mode, "red_card_%c_%d", &side, &unum ) != 2 )
            {
                std::cerr << agent_.config().teamName()
                          << " coach: "
                          << agent_.world().time()
                          << " could not parse the red card message [" << msg << ']'
                          << std::endl;
            }

            agent_.M_worldmodel.setCard( ( side == 'l' ? LEFT : side == 'r' ? RIGHT : NEUTRAL ),
                                         unum,
                                         RED );
        }
        else if ( ! std::strncmp( mode, "training", std::strlen( "training" ) ) )
        {
            // end keepaway (or some training) episode
            agent_.M_worldmodel.setTrainingTime( current_time_ );
            return;
        }
        else
        {
            std::cerr << "trainer: "
                      << agent_.world().time()
                      << " Unknown playmode string." << mode
                      << std::endl;
        }

        return;
    }

    updateServerStatus();

    if ( game_mode_.isGameEndMode() )
    {
        sendByeCommand();
        return;
    }

    agent_.M_worldmodel.updateGameMode( game_mode_, current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeHearPlayer( const char * )
{
    //std::cerr << "trainer: "
    //          << current_time_
    //          << " recv player message [" << msg << "]"
    //          << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeChangePlayerType( const char * msg )
{
    // "(ok change_player_type <teamname> <unum> <type>)"

    char teamname[32];
    int unum = -1, type = -1;

    std::memset( teamname, 0, 32 );

    if ( std::sscanf( msg, " ( ok change_player_type %31[^ ] %d %d )",
                      teamname, &unum, &type ) != 3
         || unum < 0
         || type < 0 )
    {
        std::cerr << "trainer: "
                  << current_time_
                  << " ***ERROR*** parse error. "
                  << msg
                  << std::endl;
        return;
    }

    if ( agent_.world().teamNameLeft() == teamname )
    {
        agent_.M_worldmodel.changePlayerType( LEFT, unum, type );
    }
    else if ( agent_.world().teamNameRight() == teamname )
    {
        agent_.M_worldmodel.changePlayerType( RIGHT, unum, type );
    }
    else if ( agent_.world().teamNameLeft().empty()
              && std::strlen( teamname ) != 0 )
    {
        agent_.M_worldmodel.setTeamName( LEFT, teamname );
        agent_.M_worldmodel.changePlayerType( LEFT, unum, type );
    }
    else if ( agent_.world().teamNameRight().empty()
              && std::strlen( teamname ) != 0 )
    {
        agent_.M_worldmodel.setTeamName( RIGHT, teamname );
        agent_.M_worldmodel.changePlayerType( RIGHT, unum, type );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzePlayerType( const char * msg )
{
    PlayerType player_type( msg, agent_.config().version() );
    PlayerTypeSet::instance().insert( player_type );

    agent_.handlePlayerType();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzePlayerParam( const char * msg )
{
    PlayerParam::instance().parse( msg, agent_.config().version() );

    agent_.handlePlayerParam();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeServerParam( const char * msg )
{
    ServerParam::instance().parse( msg, agent_.config().version() );
    PlayerTypeSet::instance().resetDefaultType();

    // update alarm interval
    if ( ! ServerParam::i().synchMode()
         && ServerParam::i().slowDownFactor() > 1 )
    {
        long interval = ( ServerParam::i().simulatorStep()
                          * ServerParam::i().slowDownFactor() );
        agent_.M_client->setIntervalMSec( interval );
    }

    agent_.handleServerParam();
}

/*-------------------------------------------------------------------*/
/*!
  (ok
*/
void
TrainerAgent::Impl::analyzeOK( const char * msg )
{
    if ( ! std::strncmp( msg, "(ok look ",
                         std::strlen( "(ok look " ) ) )
    {
        std::cout << "trainer: "
                  << current_time_
                  << " recv (ok look ..." << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok check_ball ",
                              std::strlen( "(ok check_ball " ) ) )
    {
        std::cout << "trainer: "
                  << current_time_
                  << " recv (ok check_ball ..." << std::endl;;
    }
    else if ( ! std::strncmp( msg, "(ok compression ",
                              std::strlen( "(ok compression " ) ) )
    {
        int level = 0;
        if ( std::sscanf( msg, "(ok compression %d)", &level ) == 1 )
        {
            std::cerr << "trainer: "
                      << current_time_
                      << " set compression level " << level << std::endl;
            agent_.M_client->setCompressionLevel( level );
        }
    }
    else if ( ! std::strncmp( msg, "(ok eye ", std::strlen( "(ok eye " ) ) )
    {
        std::cout << "trainer: "
                  << current_time_
                  << " recv " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok ear ", std::strlen( "(ok ear " ) ) )
    {
        std::cout << "trainer: "
                  << current_time_
                  << " recv " << msg << std::endl;
    }
    else if ( ! std::strncmp( msg, "(ok team_names ",
                              std::strlen( "(ok team_names " ) ) )
    {
        std::cout << "trainer: "
                  << current_time_
                  << " recv " << msg << std::endl;
        analyzeTeamNames( msg );
    }
    else
    {
        std::cout << "trainer: "
                  << current_time_
                  << " recv " << msg << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeTeamNames( const char * msg )
{
    // "(ok team_names (team l <name>)[ (team r <name>)])"

    char left[32], right[32];

    int n = std::sscanf( msg,
                         "(ok team_names (team l %31[^)]) (team r %31[^)]))",
                         left, right );
    if ( n == 2 )
    {
        agent_.M_worldmodel.setTeamName( LEFT, left );
        agent_.M_worldmodel.setTeamName( RIGHT, right );
    }
    else if ( n == 1 )
    {
        agent_.M_worldmodel.setTeamName( LEFT, left );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeError( const char * msg )
{
    std::cerr << "trainer: "
              << current_time_
              << " recv " << msg  << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::analyzeWarning( const char * msg )
{
    std::cerr << "trainer: "
              << current_time_
              << " recv " << msg << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::sendCommand( const TrainerCommand & com )
{
    std::ostringstream os;
    com.toCommandString( os );

    std::string str = os.str();
    if ( str.length() == 0 )
    {
        return false;
    }

    if ( M_client->sendMessage( str.c_str() ) > 0 )
    {
        if ( str != "(done)" )
        {
            std::cout << "OK send command [" << str << "]" << std::endl;
        }
        return true;
    }

    std::cout << "failed to send command [" << str << "]" << std::endl;
    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::sendInitCommand()
{
    if ( ! agent_.M_client->isServerAlive() )
    {
        std::cerr << "trainer: server is not alive" << std::endl;
        return;
    }

    // make command string
    TrainerInitCommand com( agent_.config().version() );
    if ( ! agent_.sendCommand( com ) )
    {
        std::cerr << "trainer: Failed to init...\nExit." << std::endl;
        agent_.M_client->setServerAlive( false );
        return;
    }

    std::cerr << "trainer: send init" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::sendSettingCommands()
{
    if ( agent_.config().useEye() )
    {
        agent_.doEye( true );
    }

    if ( agent_.config().useEar() )
    {
        agent_.doEar( true );
    }

    if ( 0 < agent_.config().compression()
         && agent_.config().compression() <= 9 )
    {
        TrainerCompressionCommand com( agent_.config().compression() );
        agent_.sendCommand( com );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::Impl::sendByeCommand()
{
    // trainer has no bye type command
    agent_.M_client->setServerAlive( false );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doCheckBall()
{
    TrainerCheckBallCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doLook()
{
    TrainerLookCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doTeamNames()
{
    TrainerTeamNamesCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doEye( bool on )
{
    TrainerEyeCommand com( on );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doEar( bool on )
{
    TrainerEarCommand com( on );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doKickOff()
{
    TrainerKickOffCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doMoveBall( const Vector2D & pos,
                          const Vector2D & vel )
{
    TrainerMoveBallCommand com( pos, vel );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doMovePlayer( const std::string & teamname,
                            const int unum,
                            const Vector2D & pos )
{
    TrainerMovePlayerCommand com( teamname, unum, pos );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doMovePlayer( const std::string & teamname,
                            const int unum,
                            const Vector2D & pos,
                            const AngleDeg & angle )
{
    TrainerMovePlayerCommand com( teamname, unum, pos, angle );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doRecover()
{
    TrainerRecoverCommand com;
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doChangeMode( const PlayMode mode )
{
    TrainerChangeModeCommand com( mode );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doChangePlayerType( const std::string & teamname,
                                  const int unum,
                                  const int type )
{
    TrainerChangePlayerTypeCommand com( teamname, unum, type );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TrainerAgent::doSay( const std::string & msg )
{
    TrainerSayCommand com( msg );
    return sendCommand( com );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
TrainerAgent::action()
{
    if ( M_impl->last_decision_time_ != M_impl->current_time_ )
    {
        actionImpl();
        M_impl->last_decision_time_ = M_impl->current_time_;
    }

    if ( M_impl->think_received_ )
    {
        TrainerDoneCommand com;
        sendCommand( com );
        M_impl->think_received_ = true;
    }
}

}
