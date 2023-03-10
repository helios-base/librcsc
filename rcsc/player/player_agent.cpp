// -*-c++-*-

/*!
  \file player_agent.cpp
  \brief basic player agent Source File
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

#include "player_agent.h"

#include "body_sensor.h"
#include "visual_sensor.h"
#include "audio_sensor.h"
#include "fullstate_sensor.h"

#include "localization_default.h"

#include "player_command.h"
#include "say_message_builder.h"
#include "soccer_action.h"
#include "soccer_intention.h"

#include <rcsc/common/audio_codec.h>
#include <rcsc/common/audio_memory.h>
#include <rcsc/common/abstract_client.h>
#include <rcsc/common/online_client.h>
#include <rcsc/common/offline_client.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/player_param.h>
#include <rcsc/common/player_type.h>
#include <rcsc/common/server_param.h>
#include <rcsc/param/param_map.h>
#include <rcsc/param/cmd_line_parser.h>
#include <rcsc/param/conf_file_parser.h>
#include <rcsc/math_util.h>
#include <rcsc/game_time.h>
#include <rcsc/game_mode.h>
#include <rcsc/timer.h>
#include <rcsc/version.h>

#include <sstream>
#include <cstdio>
#include <cstring>

//#define PROFILE_SEE

namespace rcsc {

///////////////////////////////////////////////////////////////////////
/*!
  \struct PlayerAgentImpl
  \brief player agent implementation
*/
struct PlayerAgent::Impl {

    //! reference to the agent instance
    PlayerAgent & agent_;

    //! flag to check if (think) message was received or not.
    bool think_received_;

    //! flag to check if server cycle is stopped or not.
    bool server_cycle_stopped_;

    //! last action decision game time
    GameTime last_decision_time_;

    //! current game time
    GameTime current_time_;


    int clang_min_; //!< supported minimal clang version
    int clang_max_; //!< supported maximal clang version

    //! referee info
    GameMode game_mode_;

    // it may be useful to make the abstract sensor class
    // to manage sensor container.

    //! sense_body info
    BodySensor body_;
    //! see info
    VisualSensor visual_;
    //! hear info
    AudioSensor audio_;
    //! fullstate info
    FullstateSensor fullstate_;

    //! clang parser
    CLangParser clang_;

    //! time when sense_body is received
    TimeStamp body_time_stamp_;
    //! time when see is received
    TimeStamp see_time_stamp_;

    //! status of the see messaege arrival timing
    SeeState see_state_;

    //! counter of see message arrival timing
    int see_timings_[11];

    //! pointer to reserved action
    std::shared_ptr< ArmAction > arm_action_;

    //! pointer to reserved action
    std::shared_ptr< NeckAction > neck_action_;

    //! pointer to reserved action
    std::shared_ptr< ViewAction > view_action_;

    //! pointer to reserved action
    std::shared_ptr< FocusAction > focus_action_;

    //! intention queue
    SoccerIntention::Ptr intention_;

    /*!
      \brief initialize all members
    */
    explicit
    Impl( PlayerAgent & agent )
        : agent_( agent ),
          think_received_( false ),
          server_cycle_stopped_( true ),
          last_decision_time_( -1, 0 ),
          current_time_( 0, 0 ),
          clang_min_( 0 ),
          clang_max_( 0 )
      {
          for ( int i = 0; i < 11; ++i )
          {
              see_timings_[i] = 0;
          }
      }


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
      Do NOT call this method by yourself!
    */
    void sendInitCommand();

    /*!
      \brief send client setting commands(synch_see,ear,clang,compression...) to server
     */
    void sendSettingCommands();

    /*!
      \brief send disconnection command message to server
      and set the server status to end.
    */
    void sendByeCommand();

    ////////////////////////////////////

    /*!
      \brief analyze cycle info in server message
      \param msg raw server message
      \param by_sense_body if message type is sense_body, this value becomes true
      \return parsing result status

      parse cycle data from several sensory message
      see, hear, sensebody and fullstate
    */
    bool analyzeCycle( const char * msg,
                       const bool by_sense_body );

    /*!
      \brief analyze see info
      \param msg raw server message
    */
    void analyzeSee( const char * msg );


    /*!
      \brief analyze sense_body info
      \param msg raw server message
    */
    void analyzeSenseBody( const char * msg );

    /*!
      \brief analyze hear info
      \param msg raw server message
    */
    void analyzeHear( const char * msg );

    /*!
      \brief analyze referee message inf hear info
      \param msg raw server message
    */
    void analyzeHearReferee( const char * msg );

    /*!
      \brief analyze player message in hear info
      \param msg raw server message
    */
    void analyzeHearPlayer( const char * msg );

    /*!
      \brief analyze our coach message in hear info
      \param msg raw server message
    */
    void analyzeHearOurCoach( const char * msg );

    /*!
      \brief analyze opponent coach message in hear info
      \param msg raw server message
    */
    void analyzeHearOpponentCoach( const char * msg );

    /*!
      \brief analyze trainer message in hear info
      \param msg raw server message
    */
    void analyzeHearTrainer( const char * msg );

    /*!
      \brief analyze fullstate message
      \param msg raw server message
    */
    void analyzeFullstate( const char * msg );

    /*!
      \brief analyze player_type message
      \param msg raw server message
    */
    void analyzePlayerType( const char * msg );

    /*!
      \brief analyze player_param message
      \param msg raw server message
    */
    void analyzePlayerParam( const char * msg );

    /*!
      \brief analyze server_param message
      \param msg raw server message
    */
    void analyzeServerParam( const char * msg );


    /*!
      \brief analyze init replay message
      \param msg raw server message
    */
    void analyzeInit( const char * msg );

    /*!
      \brief analyze change_player_type message
      \param msg raw server message
    */
    void analyzeChangePlayerType( const char * msg );

    /*!
      \brief analyze score message
      \param msg raw server message
    */
    void analyzeScore( const char * msg );

    /*!
      \brief analyze ok message
      \param msg raw server message
    */
    void analyzeOK( const char * msg );

    /*!
      \brief analyze error message
      \param msg raw server message
    */
    void analyzeError( const char * msg );

    /*!
      \brief analyze warningok message
      \param msg raw server message
    */
    void analyzeWarning( const char * msg );


    /*!
      \brief update current time using analyzed time value
      \param new_time analyzed time value
      \param by_sense_body true if called after sense_body message
    */
    void updateCurrentTime( const long & new_time,
                            const bool by_sense_body );

    /*!
      \brief update server game cycle status.

      This method must be called just after referee message
    */
    void updateServerStatus();

    /*!
      \brief check if now decision timing
      \param msec_from_sense elapsed milli seconds from last sense_body message arrival
      \param timeout_count timeout count since last sensing message.
      \return true if player should send action
    */
    bool isDecisionTiming( const long & msec_from_sense,
                           const int timeout_count ) const;


    /*!
      \brief adjust see message timing.

      This method is called when see info received
      This method is for normal server mode (not synch_mode).
    */
    void adjustSeeSynchNormalMode();

    /*!
      \brief adjust see message timing.

      This method is called only when 'think' message received(in decideAction())
      This method is for synch_mode.

      NOTE: this method depends on rcssserver setting.
      if synch_offset parametor is changed, this method must be modified.
    */
    void adjustSeeSynchSynchMode();


    ////////////////////////////////////

    /*!
      \brief perform reserved body action

      This method is called after doBodyAction()
    */
    void doArmAction();

    /*!
      \brief perform reserved change view action

      This method is called after doBodyAction()
      This method is called before doNeckAction()
    */
    void doViewAction();

    /*!
      \brief perform reserved turn neck action

      This method is called just after doViewAction()
    */
    void doNeckAction();

    /*!
      \brief perform reserved change_focus action

      This method is called after doBodyAction()
      This method is called after doViewAction()
      This method is called after doNeckAction()
    */
    void doFocusAction();

    /*!
      \brief output debug messages to disk/server.
     */
    void printDebug();

};

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::updateCurrentTime( const long & new_time,
                                      const bool by_sense_body )
{
    // called after sense (see, hear, sense_body, fullstate...)

    GameTime old_time = current_time_;

    // server cycle stopped (BeforeKickOff, Offside, FoulCharge, FoulPush, BackPass, FreeKickFault)
    if ( server_cycle_stopped_ )
    {
        if ( new_time != current_time_.cycle() )
        {
            current_time_.assign( new_time, 0 ) ;

            dlog.addText( Logger::LEVEL_ANY,
                          "CYCLE %ld-0 --------------------"
                          " return from cycle stop",
                          new_time );
            if ( new_time - 1 != old_time.cycle() )
            {
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " Stop Mode: previous server time is incorrect?? "
                          << old_time << " -> " << new_time << std::endl;
                dlog.addText( Logger::SYSTEM,
                              __FILE__" (updateCurrentTime) stop mode: previous server time is incorrect??"
                              "  (%ld, %ld) -> %ld",
                              old_time.cycle(), old_time.stopped(),
                              new_time );
            }
        }
        else
        {
            // if sense type is sense_body, it can be updated very safety.
            if ( by_sense_body )
            {
                current_time_.assign( current_time_.cycle(),
                                      current_time_.stopped() + 1 );

                dlog.addText( Logger::LEVEL_ANY,
                              "CYCLE %ld-%ld --------------------"
                              " stopped time was updated by sense_body",
                              current_time_.cycle(), current_time_.stopped() );
                if ( last_decision_time_ != old_time )
                {
                    if ( old_time.stopped() == 0 )
                    {
                        // just after changed to stop mode.
                        //   e.g. after goal playmode
                        // no error message
                    }
                    else
                    {
                        dlog.addText( Logger::SYSTEM,
                                      __FILE__" (updateCurrentTime) missed last action(1)..." );
                        std::cout << agent_.config().teamName() << ' '
                                  << agent_.world().self().unum() << ": "
                                  << current_time_
                                  << " missed last action?(1) last decision="
                                  << last_decision_time_
                                  << std::endl;
                    }
                }
            }
        }
    }
    // normal case
    else
    {
        current_time_.assign( new_time, 0 );

        if ( old_time.cycle() != new_time )
        {
            dlog.addText( Logger::LEVEL_ANY,
                          "CYCLE %ld-0  -------------------------------------------------",
                          new_time );
            if ( new_time - 1 != old_time.cycle() )
            {
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " skipped server time?? "
                          << old_time << " -> " << new_time << std::endl;
                dlog.addText( Logger::SYSTEM,
                              __FILE__" (updateCurrentTime) skipped server time?? (%ld, %ld) -> %ld",
                              old_time.cycle(), old_time.stopped(),
                              new_time );
            }

            if ( last_decision_time_.stopped() == 0
                 && last_decision_time_.cycle() != new_time - 1 )
            {
                dlog.addText( Logger::SYSTEM,
                              __FILE__" (updateCurrentTime) missed last action(2)..." );
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " missed last action?(2) last decision="
                          << last_decision_time_
                          << std::endl;
            }
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::updateServerStatus()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  This method must be called just after referee message

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

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::Impl::isDecisionTiming( const long & msec_from_sense,
                                     const int timeout_count ) const
{
    // if server is synch mode, 'think' message is sent to all clients
    // so it is not necessary to check decision timing.
    if ( ServerParam::i().synchMode() )
    {
        return false;
    }

    // sense_body is not received yet
    if ( msec_from_sense < 0 )
    {
        return false;
    }

    // already done in current cycle
    if ( last_decision_time_ == current_time_ )
    {
        return false;
    }

    // not initialized
    if ( agent_.world().self().unum() == Unum_Unknown )
    {
        return false;
    }

    // got current see info
    if ( agent_.world().seeTime() == current_time_ )
    {
        return true;
    }

    const int wait_thr = ( see_state_.isSynch()
                           ? agent_.config().waitTimeThrSynchView()
                           : agent_.config().waitTimeThrNoSynchView() );

    // already done in sense_body received cycle.
    // When referee message is sent before sense_body,
    // it may cause confusion to players.
    if ( last_decision_time_ == agent_.world().senseBodyTime()
         && timeout_count <= 2
         //&& msec_from_sense < ( ( ServerParam::i().simulatorStep() + wait_thr )
         //                       * ServerParam::i().slowDownFactor() )
         )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (isDecisionTiming) last_decision_time=(%ld,%ld) is same as last_sense_body=(%ld,%ld)",
                      last_decision_time_.cycle(),
                      last_decision_time_.stopped(),
                      agent_.world().senseBodyTime().cycle(),
                      agent_.world().senseBodyTime().stopped() );
        return false;
    }

    // synch_see mode, and big see_offset
    if ( SeeState::synch_see_mode()
         && ServerParam::i().synchSeeOffset() > wait_thr
         && msec_from_sense >= 0 )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (isDicisionTiming) [true] synch_see mode. offset(%d) > threshold(%d)",
                      ServerParam::i().synchSeeOffset(), wait_thr );
        return true;
    }

    // no see info during the current cycle.
    if ( see_state_.isSynch()
         && see_state_.cyclesTillNextSee() > 0 )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (isDicisionTiming) estimated cycles till next see ----- %d",
                      see_state_.cyclesTillNextSee() );
        return true;
    }

    // over the wait threshold
    if ( msec_from_sense >= wait_thr * ServerParam::i().slowDownFactor() )
    {
        if ( see_state_.isSynch() )
        {
            if ( ! SeeState::synch_see_mode()
                 || msec_from_sense > ServerParam::i().synchSeeOffset() * ServerParam::i().slowDownFactor() )
            {
                std::cout << agent_.config().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << current_time_
                          << " over offset - " << msec_from_sense
                          << "   server response delayed??"
                          << std::endl;
            }
        }
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (isDecisionTiming) over offset - %ld",
                      msec_from_sense );
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

 */
PlayerAgent::PlayerAgent()
    : SoccerAgent(),
      M_impl( new PlayerAgent::Impl( *this ) ),
      M_debug_client(),
      M_worldmodel(),
      M_fullstate_worldmodel(),
      M_effector( *this )
{
    // std::cerr << "construct player" << std::endl;

    M_fullstate_worldmodel.setValid( false );
}

/*-------------------------------------------------------------------*/
/*!

 */
PlayerAgent::~PlayerAgent()
{
    //cerr << "delete PlayerAgent" << endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::shared_ptr< AbstractClient >
PlayerAgent::createConsoleClient()
{
    std::shared_ptr< AbstractClient > ptr;

    if ( 1 <= config().offlineClientNumber()
         && config().offlineClientNumber() <= 11 )
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
BodySensor &
PlayerAgent::bodySensor() const
{
    return M_impl->body_;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
VisualSensor &
PlayerAgent::visualSensor() const
{
    return M_impl->visual_;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
AudioSensor &
PlayerAgent::audioSensor() const
{
    return M_impl->audio_;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
FullstateSensor &
PlayerAgent::fullstateSensor() const
{
    return M_impl->fullstate_;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
SeeState &
PlayerAgent::seeState() const
{
    return M_impl->see_state_;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
TimeStamp &
PlayerAgent::bodyTimeStamp() const
{
    return M_impl->body_time_stamp_;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
TimeStamp &
PlayerAgent::seeTimeStamp() const
{
    return M_impl->see_time_stamp_;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::initImpl( CmdLineParser & cmd_parser )
{
    bool help = false;
    std::string player_config_file;

    ParamMap system_param_map( "System options" );
    system_param_map.add()
        ( "help", "", BoolSwitch( &help ), "print help message.")
        ( "player-config", "", &player_config_file, "specifies player config file." );

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
    if ( ! player_config_file.empty() )
    {
        ConfFileParser conf_parser( player_config_file.c_str() );
        M_config.parse( conf_parser );
    }

    // parse command line
    M_config.parse( cmd_parser );

    if ( config().version() < 8.0
         || MAX_PROTOCOL_VERSION < config().version() )
    {
        std::cerr << "(PlayerAgent::initImpl) Unsupported client version: " << config().version()
                  << std::endl;
        return false;
    }

    M_impl->setDebugFlags();

    SelfObject::set_count_thr( config().selfPosCountThr(),
                               config().selfVelCountThr(),
                               config().selfFaceCountThr() );

    BallObject::set_count_thr( config().ballPosCountThr(),
                               config().ballRPosCountThr(),
                               config().ballVelCountThr() );

    PlayerObject::set_count_thr( config().playerPosCountThr(),
                                 config().playerVelCountThr(),
                                 config().playerFaceCountThr() );

    AudioCodec::instance().createMap( config().audioShift() );


    M_worldmodel.setLocalization( std::shared_ptr< Localization >( new LocalizationDefault() ) );
    M_fullstate_worldmodel.setLocalization( std::shared_ptr< Localization >( new LocalizationDefault() ) );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::handleStart()
{
    if ( ! M_client )
    {
        return false;
    }

    if ( config().host().empty() )
    {
        std::cerr << config().teamName()
                  << ": ***ERROR*** server host name is empty" << std::endl;
        M_client->setServerAlive( false );
        return false;
    }

    if ( ! M_client->connectTo( config().host().c_str(),
                                config().port() ) )
    {
        std::cerr << config().teamName()
                  << ": ***ERROR*** Failed to connect to ["
                  << config().host()
                  << "]"
                  << std::endl;
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
bool
PlayerAgent::handleStartOffline()
{
    if ( ! M_client )
    {
        return false;

    }

    if ( ! M_impl->openOfflineLog() )
    {
        return false;
    }

    M_client->setServerAlive( true );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::handleMessage()
{
    if ( ! M_client )
    {
        std::cerr << __FILE__ << " (handleMessage) Client is not registered."
                  << std::endl;
        return;
    }

    int counter = 0;
    GameTime start_time = M_impl->current_time_;

    // receive and analyze message
    while ( M_client->receiveMessage() > 0 )
    {
        ++counter;
        parse( M_client->message() );
    }

    // game cycle is changed while several message parsing
    if ( M_impl->current_time_.cycle() > start_time.cycle() + 1
         && start_time.stopped() == 0
         && M_impl->current_time_.stopped() == 0 )
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << "parser used several steps -- Missed an action?"
                  << "  sensory counts= " << counter
                  << "  start_time= " << start_time
                  << "  end_time= " << M_impl->current_time_
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (handleMessage) parser used several steps -- action missed! sensed %d"
                      " start=(%ld, %ld) end=(%ld, %ld)",
                      counter,
                      start_time.cycle(), start_time.stopped(),
                      M_impl->current_time_.cycle(),
                      M_impl->current_time_.stopped() );
    }

    if ( M_impl->think_received_ )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (handleMessage) Got think message: decide action" );
        action();
        M_impl->think_received_ = false;
    }
    else if ( ! ServerParam::i().synchMode() )
    {
        if ( M_impl->last_decision_time_ != M_impl->current_time_
             && world().seeTime() == M_impl->current_time_
             )
        {
            // player got a current cycle visual information
            // decide action imeddiately
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (handleMessage) Got see info: decide action" );
            action();
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::handleMessageOffline()
{
    if ( ! M_client )
    {
        std::cerr << __FILE__ << " (handleMessageOffline) Client is not registered."
                  << std::endl;
        return;
    }

    if ( M_client->receiveMessage() > 0 )
    {
        parse( M_client->message() );
    }

    if ( M_impl->think_received_ )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (handleMessageOffline) Got think message: decide action" );
#if 0
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " action" << std::endl;
#endif
        action();
        M_impl->think_received_ = false;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::handleTimeout( const int timeout_count,
                            const int waited_msec )
{
    if ( ! M_client )
    {
        std::cerr << __FILE__ << " (handleTimeout) Client is not registered."
                  << std::endl;
        return;
    }

    TimeStamp cur_time;
    cur_time.setNow();

    std::int64_t msec_from_sense = -1;
    /*
      std::cerr << "cur_sec = " << cur_time.sec()
      << "  cur_usec = " << cur_time.usec()
      << "   sense_sec=" << M_impl->body_time_stamp_.sec()
      << "  sense_usec=" << M_impl->body_time_stamp_.usec()
      << std::endl;
    */
    if ( M_impl->body_time_stamp_.isValid() )
    {
        msec_from_sense = cur_time.elapsedSince( M_impl->body_time_stamp_ );
    }

    dlog.addText( Logger::SYSTEM,
                  "----- Timeout. msec from sense_body = [%ld] ms."
                  " Timeout count = %d",
                  msec_from_sense / ServerParam::i().slowDownFactor(),
                  timeout_count );

    // estimate server down
    if ( waited_msec > config().serverWaitSeconds() * 1000 )
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << "waited "
                  << waited_msec / 1000
                  << " seconds. server down??" << std::endl;
        M_client->setServerAlive( false );
        return;
    }

    // check alarm count etc...
    if ( M_impl->isDecisionTiming( msec_from_sense, timeout_count ) )
    {
        // start decision
        dlog.addText( Logger::SYSTEM,
                      "----- TIMEOUT DECISION !! [%ld]ms from sense_body",
                      msec_from_sense / ServerParam::i().slowDownFactor() );
        action();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::handleExit()
{
    finalize();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::addSayMessageParser( SayMessageParser * parser )
{
    if ( ! parser )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": NULL SayMessageParser." << std::endl;
        return;
    }

    SayMessageParser::Ptr ptr( parser );
    M_impl->audio_.addSayMessageParser( ptr );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::removeSayMessageParser( const char header )
{
    M_impl->audio_.removeSayMessageParser( header );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::addFreeformMessageParser( FreeformMessageParser * parser )
{
    if ( ! parser )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": NULL FreeformMessageParser." << std::endl;
        return;
    }

    FreeformMessageParser::Ptr ptr( parser );
    M_impl->audio_.addFreeformMessageParser( ptr );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::removeFreeformMessageParser( const std::string & type )
{
    M_impl->audio_.removeFreeformMessageParser( type );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::finalize()
{
    if ( M_client->isServerAlive() )
    {
        M_impl->sendByeCommand();
    }
#ifdef PROFILE_SEE
    std::cout << config().teamName() << ' '
              << world().self().unum() << ": "
              << "profile see arrival timing\n";
    std::printf( "    10    20    30    40    50    60    70    80    90   100  over\n" );
    for ( int i = 0; i < 11; ++i )
    {
        std::printf( "%6d", M_impl->see_timings_[i] );
    }
    std::printf( "\n" );
#endif
    std::cout << config().teamName() << ' '
              << world().self().unum() << ": "
              << "finished."
              << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::initDebug()
{
    if ( agent_.config().offlineClientNumber() < 1
         || 11 < agent_.config().offlineClientNumber() ) // == online mode
    {
        if ( agent_.config().debugServerConnect() )
        {
            agent_.M_debug_client.connect( agent_.config().debugServerHost(),
                                           agent_.config().debugServerPort() );
        }

        if ( agent_.config().offlineLogging() )
        {
            openOfflineLog();
        }
    }

    if ( agent_.config().debug() )
    {
        openDebugLog();
    }

    if ( agent_.config().debugServerLogging() )
    {
        agent_.M_debug_client.open( agent_.config().logDir(),
                                    agent_.config().teamName(),
                                    agent_.world().self().unum() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::Impl::openOfflineLog()
{
    std::ostringstream filepath;

    if ( ! agent_.config().logDir().empty() )
    {
        filepath << agent_.config().logDir();
        if ( *(agent_.config().logDir().rbegin()) != '/' )
        {
            filepath << '/';
        }
    }

    filepath << agent_.config().teamName() << '-';

    if ( 1 <= agent_.config().offlineClientNumber()
         && agent_.config().offlineClientNumber() <= 11 )
    {
        filepath << agent_.config().offlineClientNumber();
    }
    else
    {
        filepath << agent_.world().self().unum();
    }

    filepath << agent_.config().offlineLogExt();

    if ( ! agent_.M_client->openOfflineLog( filepath.str() ) )
    {
        std::cerr << "Failed to open the offline client log file ["
                  << filepath.str()
                  << "]" << std::endl;
        agent_.M_client->setServerAlive( false );
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::Impl::openDebugLog()
{
    std::ostringstream filepath;

    if ( ! agent_.config().logDir().empty() )
    {
        filepath << agent_.config().logDir();
        if ( *(agent_.config().logDir().rbegin()) != '/' )
        {
            filepath << '/';
        }
    }

    filepath << agent_.config().teamName() << '-' << agent_.world().self().unum()
             << agent_.config().debugLogExt();

    dlog.open( filepath.str() );

    if ( ! dlog.isOpen() )
    {
        std::cerr << agent_.config().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << " Failed to open the debug log file [" << filepath.str() << "]"
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
PlayerAgent::Impl::sendInitCommand()
{
    std::ostringstream ostr;

    // make command string

    if ( agent_.config().reconnectNumber() < 1
         || 11 < agent_.config().reconnectNumber() )
    {
        // normal case
        PlayerInitCommand com( agent_.config().teamName(),
                               agent_.config().version(),
                               agent_.config().goalie() );
        com.toCommandString( ostr );
    }
    else
    {
        std::cout << agent_.config().teamName()
                  << ": reconnect. number = "
                  << agent_.config().reconnectNumber() << std::endl;
        // reconnect
        PlayerReconnectCommand com( agent_.config().teamName(),
                                    agent_.config().reconnectNumber() );
        com.toCommandString( ostr );

    }

    // send to server
    if ( agent_.M_client->sendMessage( ostr.str().c_str() ) <= 0 )
    {
        std::cout << agent_.config().teamName()
                  << ": Failed to init ...\nExit ..." << std::endl;
        agent_.M_client->setServerAlive( false );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::sendSettingCommands()
{
    std::ostringstream ostr;

    // set synch see mode
    if ( agent_.config().version() < 18.0
         && agent_.config().synchSee() )
    {
        ostr << "(synch_see)";
    }

    // turn off opponent all audio message

    if ( ! agent_.config().hearOpponentAudio() )
    {
        // off, opp
        PlayerEarCommand opp_ear_com( PlayerEarCommand::OFF,
                                      PlayerEarCommand::OPP );
        opp_ear_com.toCommandString( ostr );
    }

    if ( ! agent_.config().useCommunication() )
    {
        // off, our
        PlayerEarCommand our_ear_com( PlayerEarCommand::OFF,
                                      PlayerEarCommand::OUR );
        our_ear_com.toCommandString( ostr );
    }

    // set clang version

    if ( agent_.config().clangMin() != clang_min_
         || agent_.config().clangMax() != clang_max_ )
    {
        PlayerCLangCommand com( agent_.config().clangMin(),
                                agent_.config().clangMax() );
        com.toCommandString( ostr );
    }

    // set compression level
    if ( 0 < agent_.config().compression()
         && agent_.config().compression() <= 9 )
    {
        PlayerCompressionCommand com( agent_.config().compression() );
        com.toCommandString( ostr );
    }

    if ( ! ostr.str().empty() )
    {
        dlog.addText( Logger::SYSTEM,
                      "---- send[%s]",
                      ostr.str().c_str() );
        agent_.M_client->sendMessage( ostr.str().c_str() );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::sendByeCommand()
{
    PlayerByeCommand com;
    std::ostringstream ostr;

    com.toCommandString( ostr );
    agent_.M_client->sendMessage( ostr.str().c_str() );

    agent_.M_client->setServerAlive( false );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::setDebugFlags()
{
    const PlayerConfig & c = agent_.config();

    if ( ! c.debug() )
    {
        return;
    }

    dlog.setTimeRange( c.debugStartTime(), c.debugEndTime() );

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

    dlog.setLogFlag( &current_time_, Logger::TRAINING, c.debugTraining() );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::parse( const char * msg )
{

    if ( ! std::strncmp( msg, "(see ", 5 ) )
    {
        M_impl->analyzeSee( msg );
    }
    else if ( ! std::strncmp( msg, "(sense_body ", 12 ) )
    {
        M_impl->analyzeSenseBody( msg );
    }
    else if ( ! std::strncmp( msg, "(hear ", 6 ) )
    {
        M_impl->analyzeHear( msg );
    }
    else if ( ! std::strncmp( msg, "(think)", 7 ) )
    {
        M_impl->think_received_ = true;
    }
    else if ( ! std::strncmp( msg, "(fullstate ", 11 ) )
    {
        M_impl->analyzeFullstate( msg );
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
    else if ( ! std::strncmp(msg, "(server_param ", 14) )
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
        M_impl->analyzeScore( msg );
    }
    else if ( ! std::strncmp( msg, "(init ", 6 )
              || ! std::strncmp( msg, "(reconnect ", 11 ) )
    {
        M_impl->analyzeInit( msg );
    }
    else
    {
        std::cout << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Received unsupported message : ["
                  << msg << "]" << std::endl;
    };
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::Impl::analyzeCycle( const char * msg,
                                 bool by_sense_body )
{
    char id[16];
    long cycle = 0;
    if ( std::sscanf( msg, "(%15s %ld ",
                      id, &cycle ) != 2 )
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << "time parse error in ["
                  << msg
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      __FILE__" (analyzeCycle) Cycle parse error [%s]", msg );
        return false;
    }

    updateCurrentTime( cycle, by_sense_body );
    return true;
}

/*-------------------------------------------------------------------*/
/*!
  (see <TIME> <ObjInfo> <ObjInfo> <ObjInfo> ...
*/
void
PlayerAgent::Impl::analyzeSee( const char * msg )
{
    std::int64_t msec_from_sense = -1;

    see_time_stamp_.setNow();
    if ( body_time_stamp_.isValid() )
    {
        msec_from_sense = see_time_stamp_.elapsedSince( body_time_stamp_ );
#ifdef PROFILE_SEE
        if ( see_state_.isSynch() )
        {
            int index = (int)(msec_from_sense / ServerParam::i().slowDownFactor()) / 10;
            if ( index > 10 )
            {
                index = 10;
            }
            see_timings_[index] += 1;
        }
#endif
    }

    // parse cycle info
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    dlog.addText( Logger::SENSOR,
                  "===receive see --- [%ld]ms from sense_body",
                  msec_from_sense );

    // parse see info
    visual_.parse( msg,
                   agent_.config().teamName(),
                   agent_.config().version(),
                   current_time_ );
    //visual_.toString( std::cout );

    // update see timing status
    see_state_.updateBySee( current_time_,
                            agent_.world().self().viewWidth(),
                            agent_.world().self().viewQuality() );
    // debug purpose
    if ( visual_.time() != body_.time() )
    {
        std::cout << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " got see without sense_body." << std::endl;
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (analyzeSee) (%ld, %ld) without sense_body",
                      visual_.time().cycle(),
                      visual_.time().stopped() );
    }

    // update world model
    if ( visual_.time() == current_time_
         && agent_.world().seeTime() != current_time_ )
    {
        // update seen objects
        agent_.M_worldmodel.updateAfterSee( visual_,
                                            body_,
                                            agent_.effector(),
                                            current_time_ );
    }

    // adjust see synch
    if ( ! ServerParam::i().synchMode()
         && ! see_state_.isSynch() )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (analyzeSee) SEE received. but see timing is not synched. try to adjust" );
        adjustSeeSynchNormalMode();
    }
}

/*-------------------------------------------------------------------*/
/*!
  (sense_body <Time> <BodyInfo> <BodyInfo> ...
*/
void
PlayerAgent::Impl::analyzeSenseBody( const char * msg )
{
    body_time_stamp_.setNow();

    // parse cycle info
    if ( ! analyzeCycle( msg, true ) )
    {
        return;
    }

    // analyze process
    dlog.addText( Logger::SENSOR,
                  "===receive sense_body" );

    // parse body info
    body_.parse( msg,
                 agent_.config().version(),
                 current_time_ );
    // update see sync information
    //    M_impl->see_state_.setNewCycle( M_impl->current_time_ );
    see_state_.updateBySenseBody( current_time_,
                                  body_.viewWidth(),
                                  body_.viewQuality() );

    //----------------------------------------------
    // update process
    // check command counter
    agent_.M_effector.checkCommandCount( body_ );
    // pure internal update
    agent_.M_worldmodel.updateAfterSenseBody( body_,
                                              agent_.effector(),
                                              current_time_ );
}

/*-------------------------------------------------------------------*/
/*!
  Referee:
  -> (hear <TIME> referee <PLAYMODE>)
  Trainer:
  -> v7-: (hear <TIME> referee <MSG>)
  -> v7+: (hear <TIME> coach <MSG>)
  Coach::
  -> v7-: (hear <TIME> online_coach_{left|right} <MSG>) // no double quatation
  Self or other Player:
  -> v7-:
  ---> self:  (hear <TIME> self <MSG>)
  ---> other: (hear <TIME> <DIR> <MSG>)
  -> v7:
  ---> self:   (hear <TIME> self "<MSG>")
  ---> other:  (hear <TIME> <DIR> "<MSG>")
  -> v8+:
  ---> self:   (hear <TIME> self "<MSG>")
  ---> teammate complete: (hear <TIME> <DIR> our <UNUM> "<MSG>")
  ---> teammate partial:  (hear <TIME> our <UNUM>)
  ---> opponent complete: (hear <TIME> <DIR> opp "<MSG>")
  ---> opponent partial:  (hear <TIME> opp)
*/
void
PlayerAgent::Impl::analyzeHear( const char * msg )
{
    // parse cycle info
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }
    // parse sender info
    long cycle;
    char sender[128];

    if ( std::sscanf( msg, "(hear %ld %127s ",
                      &cycle, sender ) != 2 )
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " ***ERRORR*** failed to parse audio sender. ["
                  << msg << ']'
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      __FILE__" (analyzeHear) Hear parse error [%s]", msg );
        return;
    }


    // check sender name

    if ( ! std::strncmp( sender, "self", 4 ) )
    {
        // nothing to do
    }
    else if ( sender[0] == '-' || std::isdigit( sender[0] ) )
    {
        // complete audio from other player
        // sender string means the direction to the sender player.
        analyzeHearPlayer( msg );
    }
    else if ( ! std::strncmp( sender, "our", 3 )
              || ! std::strncmp( sender, "opp", 3 ) )
    {
        // partial audio from other player
        // nothing to do
    }
    else if ( ! std::strncmp( sender, "referee", 7 ) )
    {
        analyzeHearReferee( msg );
    }
    else if ( ! std::strncmp( sender, "online_coach_left", 17 ) )
    {
        if ( agent_.world().ourSide() == LEFT ) analyzeHearOurCoach( msg );
        if ( agent_.world().ourSide() == RIGHT ) analyzeHearOpponentCoach( msg );
    }
    else if ( ! std::strncmp( sender, "online_coach_right", 18 ) )
    {
        if ( agent_.world().ourSide() == RIGHT ) analyzeHearOurCoach( msg );
        if ( agent_.world().ourSide() == LEFT ) analyzeHearOpponentCoach( msg );
    }
    else if ( ! std::strncmp( sender, "coach", 5 ) )
    {
        analyzeHearTrainer( msg );
    }
    else
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " Received unsupported hear message [" << msg << "]"
                  << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeHearReferee( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive referee [%s]",
                  msg );
    long cycle;
    char mode[512]; // playmode or trainer's message

    if ( std::sscanf( msg, "(hear %ld referee %511[^)]", &cycle, mode ) != 2 )
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " playmode scan error. " << msg
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
                std::cerr << agent_.world().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << agent_.world().time()
                          << " could not parse the yellow card message [" << msg << ']'
                          << std::endl;
            }

            agent_.M_worldmodel.setCard( ( side == 'l'
                                           ? LEFT
                                           : side == 'r'
                                           ? RIGHT
                                           : NEUTRAL ),
                                         unum,
                                         YELLOW );
        }
        else if ( ! std::strncmp( mode, "red_card", std::strlen( "red_card" ) ) )
        {
            char side = '?';
            int unum = Unum_Unknown;
            if ( std::sscanf( mode, "red_card_%c_%d", &side, &unum ) != 2 )
            {
                std::cerr << agent_.world().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << agent_.world().time()
                          << " could not parse the red card message [" << msg << ']'
                          << std::endl;
            }

            agent_.M_worldmodel.setCard( ( side == 'l'
                                           ? LEFT
                                           : side == 'r'
                                           ? RIGHT
                                           : NEUTRAL ),
                                         unum,
                                         RED );
        }
        else if ( ! std::strncmp( mode, "training", std::strlen( "training" ) ) )
        {
            // end keepaway (or some training) episode

            // reset intention queue
            intention_.reset();

            agent_.M_worldmodel.setTrainingTime( current_time_ );
            if ( agent_.M_fullstate_worldmodel.isValid() )
            {
                agent_.M_fullstate_worldmodel.setTrainingTime( current_time_ );
            }
        }
        else
        {
            if ( agent_.config().version() < 7.0 )
            {
                std::cerr << agent_.world().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << agent_.world().time()
                          << " Referee messaga is analyzed as trainer message"
                          << std::endl;
                analyzeHearTrainer( msg );
            }
            else
            {
                std::cerr << agent_.world().teamName() << ' '
                          << agent_.world().self().unum() << ": "
                          << agent_.world().time()
                          << " Unknown playmode string. [" << mode << ']'
                          << std::endl;
                game_mode_.update( "play_on", current_time_ );
            }
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
    if ( agent_.M_fullstate_worldmodel.isValid() )
    {
        agent_.M_fullstate_worldmodel.updateGameMode( game_mode_, current_time_ );
    }

    //
    // if playmode change to NOT play_on mode, reset current intention queue
    //
    if ( game_mode_.type() != GameMode::PlayOn
         && game_mode_.type() != GameMode::PenaltyTaken_ )
    {
        intention_.reset();
    }

}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeHearPlayer( const char * msg )
{
    //----------------------------------------------
    // analyze process
    dlog.addText( Logger::SENSOR,
                  "===receive hear [%s]",
                  msg );

    if ( ! agent_.config().useCommunication() )
    {
        return;
    }

    // parse message
    audio_.parsePlayerMessage( msg, current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeHearOurCoach( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive say message from our coach" );

    audio_.parseCoachMessage( msg, current_time_ );

    agent_.handleOnlineCoachAudio();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeHearOpponentCoach( const char * )
{
    dlog.addText( Logger::SENSOR,
                  "===receive say message from opponent coach" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeHearTrainer( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive trainer audio" );

    audio_.parseTrainerMessage( msg, current_time_ );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeFullstate( const char * msg )
{
    if ( ! analyzeCycle( msg, false ) )
    {
        return;
    }

    dlog.addText( Logger::SENSOR,
                  "===receive fullstate" );

    fullstate_.parse( msg,
                      agent_.world().ourSide(),
                      agent_.config().version(),
                      current_time_ );

    if ( agent_.config().debugFullstate() )
    {
        agent_.M_fullstate_worldmodel.updateAfterFullstate( fullstate_,
                                                            agent_.effector(),
                                                            current_time_ );
        agent_.M_fullstate_worldmodel.setValid( true );
    }

    if ( agent_.config().useFullstate() )
    {
        agent_.M_worldmodel.updateAfterFullstate( fullstate_,
                                                  agent_.effector(),
                                                  current_time_ );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzePlayerType( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive player_type" );
    PlayerType player_type( msg, agent_.config().version() );
    PlayerTypeSet::instance().insert( player_type );

    agent_.handlePlayerType();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzePlayerParam( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive player_param" );
    PlayerParam::instance().parse( msg, agent_.config().version() );

    agent_.handlePlayerParam();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeServerParam( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive server_param" );
    //std::cout << msg << std::endl;
    ServerParam::instance().parse( msg, agent_.config().version() );
    PlayerTypeSet::instance().resetDefaultType();

    agent_.M_worldmodel.setServerParam();

    if ( agent_.config().debugFullstate() )
    {
        agent_.M_fullstate_worldmodel.setServerParam();
    }

    // update alarm interval
    if ( ! ServerParam::i().synchMode()
         && ServerParam::i().slowDownFactor() > 1 )
    {
        long interval = ( agent_.config().intervalMSec()
                          * ServerParam::i().slowDownFactor() );
        /*
          std::cout << "slow_down_factor is changed. new simst="
          << ServerParam::i().simulatorStep()
          << "  new interval=" << interval
          << std::endl;
        */
        agent_.M_client->setIntervalMSec( interval );
    }

    agent_.handleServerParam();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeInit( const char * msg )
{
    char mode[128];
    char side_char;
    int  unum;

    bool reconnect = false;

    // init message
    if ( ! std::strncmp( msg, "(init ", std::strlen( "(init " ) ) )
    {
        // get SIDE, UNUM and PLAYMODE
        std::sscanf( msg, "(init %c %d %127[^)]", &side_char, &unum, mode );
        std::cerr << agent_.config().teamName() << ": "
                  << "init ok.  unum: " << unum << " side: " << side_char
                  << std::endl;
    }
    // reconnect message
    else if ( ! std::strncmp( msg, "(reconnect ", std::strlen( "(reconnect " ) ) )
    {
        // get the SIDE and PLAYMODE
        reconnect = true;
        std::sscanf( msg, "(reconnect %c %127[^)]", &side_char, mode );
        unum = agent_.config().reconnectNumber();
        std::cerr << agent_.config().teamName()
                  << ": reconnected as number:"
                  << agent_.config().reconnectNumber()
                  << "  side: " << side_char << std::endl;
    }
    else
    {
        std::cout << agent_.config().teamName()
                  << ": failed to get an init message: " << msg << std::endl;
        agent_.M_client->setServerAlive( false );
        return;
    }

    if ( reconnect
         && ( agent_.config().reconnectNumber() < 1
              || 11 < agent_.config().reconnectNumber() ) )
    {
        std::cerr << agent_.config().teamName()
                  << ": parsed reconnect, but reconect number is not specified??"
                  << std::endl;
        agent_.M_client->setServerAlive( false );
        return;
    }

    SideID side = ( side_char == 'l'
                    ? LEFT
                    : RIGHT );

    //
    // member initialization concerned with team side & uniform number
    //

    if ( ! game_mode_.update( mode, current_time_ ) )
    {
        std::cerr << agent_.config().teamName() << ' '
                  << " Failed to parse init replay message."
                  << " Unknown playmode string. [" << mode << ']'
                  << std::endl;
        game_mode_.update( "play_on", current_time_ );
    }

    updateServerStatus();

    if ( agent_.config().playerNumber() == 0 )
    {
        agent_.M_config.setPlayerNumber( unum );
    }

    if ( ! agent_.M_worldmodel.init( agent_.config().teamName(),
                                     side, unum,
                                     agent_.config().goalie(),
                                     agent_.config().version() ) )
    {
        agent_.M_client->setServerAlive( false );
        return;
    }

    if ( agent_.config().debugFullstate()
         && ! agent_.M_fullstate_worldmodel.init( agent_.config().teamName(),
                                                  side, unum,
                                                  agent_.config().goalie(),
                                                  agent_.config().version() ) )
    {
        agent_.M_client->setServerAlive( false );
        return;
    }

    //
    // initialize debug resources
    //

    initDebug();

    //
    // send client setting commands
    //

    sendSettingCommands();

    //
    //
    //
    see_state_.setProtocolVersion( agent_.config().version() );
    if ( agent_.config().version() >= 18.0 )
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " (v18+) force synch see mode."
                  << std::endl;
    }

    //
    // call init message event handler
    //
    agent_.handleInitMessage();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeChangePlayerType( const char * msg )
{
    // teammate: "(change_player_type <unum> <type>)\n"
    // opponent: "(change_player_type <unum>)\n"

    int unum = Unum_Unknown;
    int type = Hetero_Unknown;

    dlog.addText( Logger::SENSOR,
                  __FILE__" (analyzeChangePlayerType) [%s]",
                  msg );

    if ( std::sscanf( msg, " ( change_player_type %d %d ) ",
                      &unum, &type ) == 2 )
    {
        agent_.M_worldmodel.setOurPlayerType( unum, type );
        if ( agent_.config().debugFullstate()
             && agent_.M_fullstate_worldmodel.isValid() )
        {
            agent_.M_fullstate_worldmodel.setOurPlayerType( unum, type );
        }
    }
    else if ( std::sscanf( msg, " ( change_player_type %d ) ",
                           &unum ) == 1 )
    {
        agent_.M_worldmodel.setTheirPlayerType( unum, Hetero_Unknown );
        if ( agent_.config().debugFullstate()
             && agent_.M_fullstate_worldmodel.isValid() )
        {
            agent_.M_fullstate_worldmodel.setTheirPlayerType( unum, Hetero_Unknown );
        }
    }
    else
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " ***ERROR*** Failed to analyze change_player_type"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      __FILE__" (analyzeChangePlayerType) error change_player_type" );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeOK( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive ok [%s]",
                  msg );

    if ( ! std::strncmp( msg,
                         "(ok synch_see)",
                         std::strlen( "(ok synch_see)" ) ) )
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << " set synch see mode."
                  << std::endl;
        see_state_.setSynchSeeMode();
        see_state_.setViewMode( agent_.world().self().viewWidth(),
                                agent_.world().self().viewQuality() );
        return;
    }
    if ( ! std::strncmp( msg,
                         "(ok compression ",
                         std::strlen( "(ok compression " ) ) )
    {
        int level = 0;
        if ( std::sscanf( msg, "(ok compression %d)", &level ) == 1 )
        {
            std::cerr << agent_.world().teamName() << ' '
                      << agent_.world().self().unum() << ": "
                      << agent_.world().time()
                      << " set compression level " << level
                      << std::endl;
            agent_.M_client->setCompressionLevel( level );
            return;
        }
    }
    else if ( ! std::strncmp( msg,
                              "(ok clang ",
                              std::strlen( "(ok clang " ) ) )
    {
        // (ok clang (ver 7 8))
        int vermin, vermax;
        if ( std::sscanf( msg, "(ok clang (ver %d %d))", &vermin, &vermax ) == 2 )
        {
            //std::cerr << config().teamName() << ' '
            //          << world().self().unum() << ": "
            //          << M_impl->current_time_
            //          << "set clang version " << vermin << " " << vermax
            //          << std::endl;
            clang_min_ = vermin;
            clang_max_ = vermax;
            return;
        }
    }

    std::cerr << agent_.config().teamName() << ' '
              << agent_.world().self().unum() << ": "
              << current_time_
              << " recv unsupported or illegal ok message [" << msg << "]" << std::endl;
    dlog.addText( Logger::SENSOR,
                  __FILE__" (analyzeOk) unsupported ok" );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeScore( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive score [%s]",
                  msg );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeError( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive error [%s]",
                  msg );
    std::cerr << agent_.world().teamName() << ' '
              << agent_.world().self().unum() << ": "
              << agent_.world().time()
              << " recv error message [" << msg << "]" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::analyzeWarning( const char * msg )
{
    dlog.addText( Logger::SENSOR,
                  "===receive warning [%s]",
                  msg );
    std::cerr << agent_.world().teamName() << ' '
              << agent_.world().self().unum() << ": "
              << agent_.world().time()
              << "recv warning message [" << msg << "]" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::action()
{
    Timer timer;
    dlog.addText( Logger::SYSTEM,
                  __FILE__" (action) start" );

    if ( config().offlineLogging()
         && ! ServerParam::i().synchMode() )
    {
        M_client->printOfflineThink();
    }

    // check see synchronization
    if ( M_impl->see_state_.isSynch()
         && M_impl->see_state_.cyclesTillNextSee() == 0
         && world().seeTime() != M_impl->current_time_ )
    {
        if ( SeeState::synch_see_mode()
             && ServerParam::i().synchSeeOffset() > ServerParam::i().synchOffset() )
        {
            // no problem?
        }
        else
        {
            dlog.addText( Logger::SYSTEM,
                          __FILE__" (action) missed see synch. action without see" );
            std::cout << world().teamName() << ' '
                      << world().self().unum() << ": "
                      << world().time()
                      << " missed see synch. action without see" << std::endl;

            // set synch timing to illegal.
            M_impl->see_state_.setLastSeeTiming( SeeState::TIME_NOSYNCH );
        }
    }

    // ------------------------------------------------------------------------
    // last update
    // update positining matrix, offside line, defense line, etc.
    M_worldmodel.updateJustBeforeDecision( effector(),
                                           M_impl->current_time_ );
    if ( config().debugFullstate()
         && M_fullstate_worldmodel.isValid() )
    {
        M_fullstate_worldmodel.updateJustBeforeDecision( effector(),
                                                         M_impl->current_time_ );
    }

    // reset last action effect
    M_effector.reset();

    //
    // handle action start event
    //
    handleActionStart();

    // ------------------------------------------------------------------------
    // decide action

    if ( ServerParam::i().synchMode()
         && ! M_impl->see_state_.isSynch() )
    {
        M_impl->adjustSeeSynchSynchMode();
    }

    actionImpl(); // this is pure virtual method
    M_impl->doArmAction();
    M_impl->doViewAction();
    M_impl->doNeckAction();
    M_impl->doFocusAction();
    communicationImpl();

    // ------------------------------------------------------------------------
    // set command effect. these must be called before command composing.
    // set self view mode, pointto and attentionto info.
    M_worldmodel.updateJustAfterDecision( effector() );
    if ( effector().changeViewCommand() )
    {
        // set cycles till next see, update estimated next see arrival timing
        M_impl->see_state_.setViewMode( effector().changeViewCommand()->width(),
                                        effector().changeViewCommand()->quality() );
    }

    // ------------------------------------------------------------------------
    // compose command string, and send it to the rcssserver
    {
        std::ostringstream ostr;
        M_effector.makeCommand( ostr );
        const std::string str = ostr.str();
        if ( str.length() > 0 )
        {
            dlog.addText( Logger::SYSTEM,
                          "---- send[%s]",
                          str.c_str() );
            M_client->sendMessage( str.c_str() );
        }
    }

    // ------------------------------------------------------------------------
    // update last decision time
    M_impl->last_decision_time_ = M_impl->current_time_;
    double elapsed = timer.elapsedReal();

    dlog.addText( Logger::SYSTEM,
                  __FILE__" (action) elapsed %lf [ms]", elapsed );
    M_debug_client.addMessage( "%.0fms", elapsed );

    //
    // handle action end event
    //
    handleActionEnd();

    //
    // debugger output
    //
    M_impl->printDebug();

    // delete all command objects and say messages
    M_effector.clearAllCommands();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::doArmAction()
{
    if ( arm_action_
         && agent_.world().self().armMovable() == 0 )
    {
        arm_action_->execute( &agent_ );
        arm_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::doViewAction()
{
    if ( ! see_state_.isSynch()
         && agent_.world().gameMode().type() != GameMode::PlayOn )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (doViewAction) *no sync and no play_on* agent need to synchronize see message." );
        return;
    }

    if ( view_action_ )
    {
        view_action_->execute( &agent_ );
        view_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::doFocusAction()
{
    if ( focus_action_ )
    {
        focus_action_->execute( &agent_ );
        focus_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::doNeckAction()
{
    if ( neck_action_ )
    {
        neck_action_->execute( &agent_ );
        neck_action_.reset();
    }
    else
    {
        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << "  WARNING. no turn_neck." << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::adjustSeeSynchNormalMode()
{
    if ( see_state_.isSynch() )
    {
        return;
    }

    // now, see & sense_body must be received simultaneously
    // --> synch chance
    if ( see_state_.isSynchedSeeCountNormalMode() )
    {
        dlog.addText( Logger::SYSTEM,
                      __FILE__" (adjustSeeSynchNormalMode) see count is synch case" );
        // set synchronized see timing
        see_state_.setLastSeeTiming( SeeState::TIME_0_00 );

        std::ostringstream ostr;

        PlayerChangeViewCommand com( ViewWidth::NORMAL, ViewQuality::HIGH );
        com.toCommandString( ostr );

        agent_.M_client->sendMessage( ostr.str().c_str() );
        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] see sync",
                      ostr.str().c_str() );

        agent_.M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        agent_.M_worldmodel.setViewMode( com.width(), com.quality() );
        see_state_.setViewMode( com.width(), com.quality() );

        std::cout << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << "  see synch." << std::endl;
        return;
    }

    // Now, no synchronization

    // playmode is play_on
    // --> stop adjustment trial.
    if ( agent_.world().gameMode().type() == GameMode::PlayOn )
    {
        if ( agent_.world().self().viewQuality().type() == ViewQuality::LOW )
        {
            std::ostringstream ostr;

            PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::HIGH );
            com.toCommandString( ostr );

            agent_.M_client->sendMessage( ostr.str().c_str() );
            dlog.addText( Logger::SYSTEM,
                          "---- send[%s] no sync. change to high",
                          ostr.str().c_str() );

            agent_.M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
            agent_.M_worldmodel.setViewMode( com.width(), com.quality() );
            see_state_.setViewMode( com.width(), com.quality() );
        }
        return;
    }

    // playmode is not play_on
    // --> try adjustment. view mode is changed to (Narrow, Low)
    if ( agent_.world().self().viewWidth().type() != ViewWidth::NARROW
         || agent_.world().self().viewQuality().type() != ViewQuality::LOW )
    {
        std::ostringstream ostr;

        PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::LOW );
        com.toCommandString( ostr );

        agent_.M_client->sendMessage( ostr.str().c_str() );
        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] prepare see sync",
                      ostr.str().c_str() );

        agent_.M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        agent_.M_worldmodel.setViewMode( com.width(), com.quality() );
        see_state_.setViewMode( com.width(), com.quality() );

        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << "  prepare see synch" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::adjustSeeSynchSynchMode()
{
    if ( see_state_.isSynch() )
    {
        return;
    }

    // last see timing is just 50ms from sense_body
    // --> synch chance
    if ( see_state_.isSynchedSeeCountSynchMode() )
    {
        // set synchronized see timing
        see_state_.setLastSeeTiming( SeeState::TIME_50_0 );

        std::ostringstream ostr;

        PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::HIGH );
        com.toCommandString( ostr );

        agent_.M_client->sendMessage( ostr.str().c_str() );
        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] synch_mode. see synch",
                      ostr.str().c_str() );

        agent_.M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        agent_.M_worldmodel.setViewMode( com.width(), com.quality() );
        see_state_.setViewMode( com.width(), com.quality() );

        std::cout << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << "  see synch." << std::endl;
        return;
    }

    // Now, no synchronization

    // playmode is play_on
    // --> stop adjustment trial.
    if ( agent_.world().gameMode().type() == GameMode::PlayOn )
    {
        if ( agent_.world().self().viewQuality().type() == ViewQuality::LOW )
        {
            std::ostringstream ostr;

            PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::HIGH );
            com.toCommandString( ostr );

            agent_.M_client->sendMessage( ostr.str().c_str() );
            dlog.addText( Logger::SYSTEM,
                          "---- send[%s] synch_mode. no sync. change to high",
                          ostr.str().c_str() );

            agent_.M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
            agent_.M_worldmodel.setViewMode( com.width(), com.quality() );
            see_state_.setViewMode( com.width(), com.quality() );
        }
        return;
    }

    // playmode is not play_on
    // --> try adjustment. view mode is changed to (Narrow, Low)
    if ( agent_.world().self().viewWidth() != ViewWidth::NARROW
         && agent_.world().self().viewQuality() != ViewQuality::LOW )
    {
        std::ostringstream ostr;

        PlayerChangeViewCommand com( ViewWidth::NARROW, ViewQuality::LOW );
        com.toCommandString( ostr );

        agent_.M_client->sendMessage( ostr.str().c_str() );
        dlog.addText( Logger::SYSTEM,
                      "---- send[%s] synch_mode. prepare see sync",
                      ostr.str().c_str() );

        agent_.M_effector.incCommandCount( PlayerCommand::CHANGE_VIEW );
        agent_.M_worldmodel.setViewMode( com.width(), com.quality() );
        see_state_.setViewMode( com.width(), com.quality() );

        std::cerr << agent_.world().teamName() << ' '
                  << agent_.world().self().unum() << ": "
                  << agent_.world().time()
                  << "  prepare see synch" << std::endl;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::Impl::printDebug()
{
    if ( agent_.config().debugServerConnect()
         || agent_.config().debugServerLogging() )
    {
        // compose worldmodel & some additional debug messages
        // send to debug server or write to disk
        agent_.M_debug_client.writeAll( agent_.world(), agent_.effector() );
    }
    else
    {
        agent_.M_debug_client.clear();
    }

    dlog.flush();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doKick( const double & power,
                     const AngleDeg & rel_dir )
{
    if ( ! world().self().isKickable() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__" (doKick) but not kickable" );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " doKick(). but not kickable" << std::endl;
        return false;
    }
    if ( world().self().isFrozen() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__" (doKick) but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setKick( power, rel_dir );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doTurn( const AngleDeg & moment )
{
    if ( world().self().isFrozen() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doTurn. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setTurn( moment );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doDash( const double & power,
                     const AngleDeg & rel_dir )
{
    if ( world().self().isFrozen() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doDash. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    M_effector.setDash( power, rel_dir );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doMove( const double & x,
                     const double & y )
{
    if ( world().self().isFrozen() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doMove. but in tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    // check if I am movable
    if ( ! ( world().gameMode().type() == GameMode::BeforeKickOff
             || world().gameMode().type() == GameMode::AfterGoal_
             || ( world().self().goalie()
                  && world().gameMode().type() == GameMode::GoalieCatch_
                  && world().gameMode().side() == world().ourSide() )
             )
         )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Can move only in before kickoff mode (or after goalie catch)"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doMove. cannot move to (%.1f %.1f)",
                      x, y );
        return false;
    }

    M_effector.setMove( x, y );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doCatch()
{
    if ( world().self().isFrozen() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doCatch. refused. tackle expire period  %d",
                      world().self().tackleExpires() );
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        return false;
    }

    if ( ! world().self().goalie() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Only goalies can catch" << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doCatch. only goalie can catch" );
        return false;
    }

    if ( world().gameMode().type() != GameMode::PlayOn
         && world().gameMode().type() != GameMode::PenaltyTaken_ )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " not play_on mode, cannot catch"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doCatch. playmode is not play_on" );
        return false;
    }

    if ( ! world().ball().rposValid() )
    {
        std::cerr << world().teamName() << ": "
                  << world().self().unum() << ' '
                  << world().time()
                  << " doCatch: ball is unknown." << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": Effector::setCatch. ball is unknown. rpos conf count = %d",
                      world().ball().rposCount() );
        return false;
    }

    M_effector.setCatch();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doTackle( const double & power_or_dir,
                       const bool foul )
{
    if ( world().self().isFrozen() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " Now Tackle expire period" << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doTackle. refused. tackle expire period  %d",
                      world().self().tackleExpires() );
        return false;
    }

    //M_effector.setTackle( power_or_dir, true );
    M_effector.setTackle( power_or_dir, foul );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doTurnNeck( const AngleDeg & moment )
{
    M_effector.setTurnNeck( moment );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doChangeView( const ViewWidth & width )
{
    if ( M_impl->see_state_.isSynch() )
    {
        if ( ! M_impl->see_state_.canSendChangeView( width, world().time() ) )
        {
            dlog.addText( Logger::ACTION,
                          __FILE__" (doChangeView) width(%d) will break see synch... ",
                          width.type() );
            return false;
        }
    }
    else
    {
        if ( world().gameMode().type() != GameMode::PlayOn )
        {
            dlog.addText( Logger::ACTION,
                          __FILE__" (doChangeView) no synch. not play on."
                          " should try to adjust. " );
            return false;
        }
    }

    if ( width == M_effector.queuedNextViewWidth() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__" (doChangeView) already same view mode %d",
                      width.type() );
        return false;
    }

    M_effector.setChangeView( width );
    return true;
}

/*-------------------------------------------------------------------*/
bool
PlayerAgent::doChangeFocus( const double moment_dist,
                            const AngleDeg & moment_dir )
{

    // check the range of distance
    double aligned_moment_dist = moment_dist;
    if ( world().self().focusDist() + aligned_moment_dist < 0.0 )
    {
        if ( world().self().focusDist() + aligned_moment_dist < -1.0e-5 )
        {
            std::cerr << world().teamName() << ' ' << world().self().unum() << ": " << world().time()
                      << " (doChangeFocus) under min dist. " << world().self().focusDist() + moment_dist << std::endl;
            dlog.addText( Logger::ACTION,
                          __FILE__" (doChangeFocus) under min dist %f command=%f",
                          world().self().focusDist() + moment_dist, moment_dist );
        }
        aligned_moment_dist = -world().self().focusDist();
    }
    else if ( world().self().focusDist() + aligned_moment_dist > 40.0 )
    {
        if ( world().self().focusDist() + aligned_moment_dist > 40.0 + 1.0e-5 )
        {
            std::cerr << world().teamName() << ' ' << world().self().unum() << ": " << world().time()
                      << " (doChangeFocus) over dist. " << world().self().focusDist() + moment_dist << std::endl;
            dlog.addText( Logger::ACTION,
                          __FILE__" (doChangeFocus) over max dist %f command=%f",
                          world().self().focusDist() + moment_dist, moment_dist );
        }
        aligned_moment_dist = 40.0 - world().self().focusDist();
    }

    // check the range of visible angle
    const ViewWidth next_width = M_effector.queuedNextViewWidth();
    const double next_half_angle = next_width.width() * 0.5;

    AngleDeg aligned_moment_dir = moment_dir;
    if ( world().self().focusDir().degree() + aligned_moment_dir.degree() < -next_half_angle )
    {
        if ( world().self().focusDir().degree() + aligned_moment_dir.degree() < -next_half_angle - 1.0e-5 )
        {
            std::cerr << world().teamName() << ' ' << world().self().unum() << ": " << world().time()
                      << " (doChangeFocus) under min angle. " << world().self().focusDir().degree() + moment_dir.degree()
                      << " < " << -next_half_angle
                      << std::endl;
            dlog.addText( Logger::ACTION,
                          __FILE__" (doChangeFocus) under min angle %f < %f. command=%f",
                          world().self().focusDir().degree() + moment_dir.degree(), -next_half_angle, moment_dir.degree() );
        }
        aligned_moment_dir = -next_half_angle - world().self().focusDir().degree();
    }
    else if ( world().self().focusDir().degree() + aligned_moment_dir.degree() > next_half_angle )
    {
        if ( world().self().focusDir().degree() + aligned_moment_dir.degree() > next_half_angle + 1.0e-5 )
        {
            std::cerr << world().teamName() << ' ' << world().self().unum() << ": " << world().time()
                      << " (doChangeFocus) over max angle " << world().self().focusDir().degree() + moment_dir.degree()
                      << " > next_half=" << next_half_angle
                      << std::endl;
            dlog.addText( Logger::ACTION,
                          __FILE__" (doChangeFocus) over max angle %f > %f. command=%f",
                          world().self().focusDir().degree() + moment_dir.degree(), next_half_angle, moment_dir.degree() );
        }
        aligned_moment_dir = next_half_angle - world().self().focusDir().degree();
    }

    M_effector.setChangeFocus( aligned_moment_dist, aligned_moment_dir );
    return true;
}


/*-------------------------------------------------------------------*/
/*!

 */
// bool
// PlayerAgent::doSay( const std::string & msg )
// {
//     if ( ! config().useCommunication() )
//     {
//         dlog.addText( Logger::ACTION,
//                       "agent->doSay. communication is not allowed" );
//         return false;
//     }

//     // check message length
//     if ( static_cast< int >( msg.length() )
//          > ServerParam::i().playerSayMsgSize() )
//     {
//         std::cerr << world().teamName() << ' '
//                   << world().self().unum() << ": "
//                   << world().time()
//                   << "  say message too long [" << msg << "]="
//                   << msg.length() << std::endl;
//         dlog.addText( Logger::ACTION,
//                       "agent->doSay. message too long. length= %d",
//                       msg.length() );
//         return false;
//     }

//     M_effector.setSay( msg, config().version() );
//     return true;
// }

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doPointto( const double & x,
                        const double & y )
{
    if ( world().self().armMovable() > 0 )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doPointto. now pointing and cannot move arm." );
        return false;
    }

    if ( ! world().self().posValid() )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << " doPointto : invalid localization" << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": Effector::setPointto. invalid self localization..." );
        return false;
    }

    M_effector.setPointto( x, y );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doPointtoOff()
{
    if ( world().self().armMovable() > 0 )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doPointtoOff. now pointing and cannot move arm." );
        return false;
    }

    M_effector.setPointtoOff();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doAttentionto( SideID side,
                            const int unum )
{
    if ( side == NEUTRAL )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  attentionto. invalid side " << side << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doAttentionto. Invalid side %d",
                      side );
        return false;
    }

    if ( unum == Unum_Unknown )
    {
        return false;
    }

    if ( unum < 1 || 11 < unum )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  attentionto. invalid unum " << unum << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doAttentionto. Invalid unum %d",
                      unum );
        return false;
    }

    if ( world().ourSide() == side
         && world().self().unum() == unum )
    {
        std::cerr << world().teamName() << ' '
                  << world().self().unum() << ": "
                  << world().time()
                  << "  attentionto. try to attention to itself " << std::endl;
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->doAttentionto. try to attention to self." );
        return false;
    }

    // if ( world().self().attentiontoUnum() == unum
    //      && world().self().attentiontoSide() == side )
    // {
    //     dlog.addText( Logger::ACTION,
    //                   __FILE__": agent->doAttentionto. already attended." );
    //     return false;
    // }

    M_effector.setAttentionto( side, unum );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doAttentiontoOff()
{
    M_effector.setAttentiontoOff();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::setArmAction( ArmAction * act )
{
    if ( act )
    {
        M_impl->arm_action_ = std::shared_ptr< ArmAction >( act );
    }
    else
    {
        M_impl->arm_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::setNeckAction( NeckAction * act )
{
    if ( act )
    {
        if ( M_impl->neck_action_ )
        {
            dlog.addText( Logger::ACTION,
                          __FILE__": (setNeckAction) overwrite exsiting neck action." );
        }
        M_impl->neck_action_ = std::shared_ptr< NeckAction >( act );
    }
    else
    {
        M_impl->neck_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::setViewAction( ViewAction * act )
{
    if ( act )
    {
        M_impl->view_action_ = std::shared_ptr< ViewAction >( act );
    }
    else
    {
        M_impl->view_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::setFocusAction( FocusAction * act )
{
    if ( act )
    {
        M_impl->focus_action_ = std::shared_ptr< FocusAction >( act );
    }
    else
    {
        M_impl->focus_action_.reset();
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::addSayMessage( SayMessage * message )
{
    if ( ! config().useCommunication() )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__": agent->addSayMessage. communication is not allowed" );
        return;
    }

    M_effector.addSayMessage( message );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::removeSayMessage( const char header )
{
    return M_effector.removeSayMessage( header );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::clearSayMessage()
{
    M_effector.clearSayMessage();
}

/*-------------------------------------------------------------------*/
/*!

 */
void
PlayerAgent::setIntention( SoccerIntention * intention )
{
    M_impl->intention_ = std::shared_ptr< SoccerIntention >( intention );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
PlayerAgent::doIntention()
{
    if ( M_impl->intention_ )
    {
        if ( M_impl->intention_->finished( this ) )
        {
            M_impl->intention_.reset();
            return false;
        }

        return M_impl->intention_->execute( this );
    }

    return false;
}

}
