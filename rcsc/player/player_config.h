// -*-c++-*-

/*!
  \file player_config.h
  \brief player configuration Header File
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

#ifndef RCSC_PLAYER_PLAYER_CONFIG_H
#define RCSC_PLAYER_PLAYER_CONFIG_H

#include <string>
#include <iosfwd>

namespace rcsc {

class ParamMap;
class ParamParser;

/*!
  \class PlayerConfig
  \brief player configuration variable set
*/
class PlayerConfig {
private:

    ParamMap * M_param_map;

    // basic setting

    std::string M_team_name; //!< team name string
    double      M_version; //!< client version
    int         M_reconnect_number; //!< uniform number for reconnection
    bool        M_goalie; //!< goalie flag

    int M_interval_msec; //!< timeout interval
    int M_server_wait_seconds; //!< time to wait server message

    //! msec threshold for action decision timing when see sync
    int M_wait_time_thr_synch_view;
    //! msec threshold for action decision timing when no see sync
    int M_wait_time_thr_nosynch_view;

    //! msec threshold for normal view width when manual see sync
    int M_normal_view_time_thr;

    std::string M_rcssserver_host; //!< host name that rcssserver is running
    int         M_rcssserver_port; //!< rcssserver connection port number

    int M_compression; //!< zlib compression level for the compression command

    int M_clang_min; //!< supported clang min version
    int M_clang_max; //!< supported clang max version

    bool M_use_communication; //!< if true, communiction is used
    bool M_hear_opponent_audio; //!< if true, opponent communication is heared
    int M_audio_shift; //!< shift parameter to encrypt audio message

    bool M_use_fullstate; //!< if true, WorldModel is updated by fullstate.
    bool M_debug_fullstate; //!< if true, the virtual fullstate worldmodel is used.

    bool M_synch_see; //!< if true, synchronous see mode is used.

    // confidence value

    int M_self_pos_count_thr; //!< self position confidence threshold
    int M_self_vel_count_thr; //!< self velocity confidence threshold
    int M_self_face_count_thr; //!< self angle confidence threshold

    int M_ball_pos_count_thr; //!< ball position confidence threshold
    int M_ball_rpos_count_thr; //!< ball relative position confidence threshold
    int M_ball_vel_count_thr; //!< ball velocity confidence threshold

    int M_player_pos_count_thr; //!< player position confidence threshold
    int M_player_vel_count_thr; //!< player velocity confidence threshold
    int M_player_face_count_thr; //!< player angle confidence threshold


    //! specifies player's number independent of uniform number
    int M_player_number;

    //! miscellaneous configuration directory.
    std::string M_config_dir;

    //
    // debug
    //

    bool M_debug; //!< if false, all log files or debug client are never used.

    //! the directory path string where log files are written. this directory path is used by all log file types.
    std::string M_log_dir;

    //
    // debug client settings
    //

    bool M_debug_server_connect; //!< if true, client connects to the debug server.
    bool M_debug_server_logging; //!< if true, debug server messages are written to the debug log file.

    std::string M_debug_server_host; //!< host name or ip address where debug server is running.
    int  M_debug_server_port; //!< debug server port number.

    //
    // offline client settings
    //

    bool M_offline_logging; //!< if true, client write all sensory info to the file.
    std::string M_offline_log_ext; //!< the extension string of offline client log file.

    //! the uniform number for offline client. 1-11 means offline mode, other values mean online mode.
    int M_offline_client_number;

    //
    // debug logging
    //

    int M_debug_start_time; //!< the start time for recording the debug log
    int M_debug_end_time; //!< the end time for recording the debug log

    std::string M_debug_log_ext; //!< the extension string of debug log file

    bool M_debug_system; //!< debug level flag
    bool M_debug_sensor; //!< debug level flag
    bool M_debug_world; //!< debug level flag
    bool M_debug_action; //!< debug level flag
    bool M_debug_intercept; //!< debug level flag
    bool M_debug_kick; //!< debug level flag
    bool M_debug_hold; //!< debug level flag
    bool M_debug_dribble; //!< debug level flag
    bool M_debug_pass; //!< debug level flag
    bool M_debug_cross; //!< debug level flag
    bool M_debug_shoot; //!< debug level flag
    bool M_debug_clear; //!< debug level flag
    bool M_debug_block; //!< debug level flag
    bool M_debug_mark; //!< debug level flag
    bool M_debug_positioning; //!< debug level flag
    bool M_debug_role; //!< debug level flag
    bool M_debug_plan; //!< debug level flag
    bool M_debug_team; //!< debug level flag
    bool M_debug_communication; //!< debug level flag
    bool M_debug_analyzer; //!< debug level flag
    bool M_debug_action_chain; //!< debug level flag

    bool M_debug_training; //!< debug level flag

public:

    /*!
      \brief init variables by default value. create ParamMap instance
     */
    PlayerConfig();

    /*!
      \brief delete ParamMap instance
     */
    ~PlayerConfig();

    /*!
      \brief set parameter values using param parser instance
      \param parser param parser instance
     */
    void parse( ParamParser & parser );

    /*!
      \brief print help message to the output stream
      \param os output stream
      \return output stream
     */
    std::ostream & printHelp( std::ostream & os ) const;

protected:
    /*!
      \brief set default value
    */
    void setDefaultParam();

    /*!
      \brief set parameter entries
     */
    void createParamMap();

public:

    // basic settings

    /*!
      \brief get the team name string
      \return team name string
     */
    const std::string & teamName() const { return M_team_name; }

    /*!
      \brief get the client version
      \return client version
     */
    double version() const { return M_version; }

    /*!
      \brief get the uniform numver for a reconnect command
      \return the uniform numver for a reconnect command
     */
    int reconnectNumber() const { return M_reconnect_number; }

    /*!
      \brief get the goalie flag
      \return goalie flag
     */
    bool goalie() const { return M_goalie; }

    /*!
      \brief get the interrupt interval for decision making
      \return interrupt interval in milli-seconds
     */
    int intervalMSec() const { return M_interval_msec; }

    /*!
      \brief get the maximum time to wait server response
      \return wait time in seconds.
     */
    int serverWaitSeconds() const { return M_server_wait_seconds; }

    /*!
      \brief get the maximum time to wait see message for synch view mode
      \return wait time in milli-seconds
     */
    int waitTimeThrSynchView() const { return M_wait_time_thr_synch_view; }

    /*!
      \brief get the maximum time to wait see message for asynch view mode
      \return wait time in milli-seconds
     */
    int waitTimeThrNoSynchView() const { return M_wait_time_thr_nosynch_view; }

    /*!
      \brief get the threshold time to change to normal view width for old timer synch view mode
      \return the threshold time to change to normal view width
     */
    int normalViewTimeThr() const { return M_normal_view_time_thr; }

    /*!
      \brief get the server host name string
      \return server host name string
     */
    const std::string & host() const { return M_rcssserver_host; }

    /*!
      \brief get the server port number
      \return server port number
     */
    int port() const { return M_rcssserver_port; }

    /*!
      \brief get the server message compression level
      \return server message compression level
     */
    int compression() const { return M_compression; }

    /*!
      \brief get the minimum clang version to be sent
      \return minimum clang version to be sent
     */
    int clangMin() const { return M_clang_min; }

    /*!
      \brief get the maximum clang version to be sent
      \return maximum clang version to be sent
     */
    int clangMax() const { return M_clang_max; }

    /*!
      \brief get the communication mode flag
      \return communication mode flag
     */
    bool useCommunication() const { return M_use_communication; }

    /*!
      \brief get the auditory sensor mode flag for opponent
      \return auditory sensor mode flag for opponent
     */
    bool hearOpponentAudio() const { return M_hear_opponent_audio; }

    /*
      \brief shift value to encrypt audio encoder
      \return shift value
     */
    int audioShift() const { return M_audio_shift; }

    /*!
      \brief get the fullstate mode flag that determines whether the world model is updated by fullstate information
      \return fullstate mode flag
     */
    bool useFullstate() const { return M_use_fullstate; }

    /*!
      \brief get the switch variable thate determines whether a fullstate world model for debugging is used or not.
      \return debugging fullstate world model switch
     */
    bool debugFullstate() const { return M_debug_fullstate; }

    /*!
      \brief get the synch_see mode flag
      \return synch_see mode flag
     */
    bool synchSee() const { return M_synch_see; }

    // confidence value

    /*!
      \brief get the self position accuracy count threshold
      \return self position accuracy count threshold
     */
    int selfPosCountThr() const { return M_self_pos_count_thr; }

    /*!
      \brief get the self velocity accuracy count threshold
      \return self velocity accuracy count threshold
     */
    int selfVelCountThr() const { return M_self_vel_count_thr; }

    /*!
      \brief get the self face angle accuracy count threshold
      \return self face angle accuracy count threshold
     */
    int selfFaceCountThr() const { return M_self_face_count_thr; }

    /*!
      \brief get the ball position accuracy count threshold
      \return ball position accuracy count threshold
     */
    int ballPosCountThr() const { return M_ball_pos_count_thr; }

    /*!
      \brief get the ball relative position accuracy count threshold
      \return ball relative position accuracy count threshold
     */
    int ballRPosCountThr() const { return M_ball_rpos_count_thr; }

    /*!
      \brief get the ball velocity accuracy count threshold
      \return ball velocity accuracy count threshold
     */
    int ballVelCountThr() const { return M_ball_vel_count_thr; }

    /*!
      \brief get the other player position accuracy count threshold
      \return other player position accuracy count threshold
     */
    int playerPosCountThr() const { return M_player_pos_count_thr; }

    /*!
      \brief get the other player position accuracy count threshold
      \return other player velocity accuracy count threshold
     */
    int playerVelCountThr() const { return M_player_vel_count_thr; }

    /*!
      \brief get the other player face angle accuracy count threshold
      \return other player face angle accuracy count threshold
     */
    int playerFaceCountThr() const { return M_player_face_count_thr; }

    /*!
      \brief get the player number (not a uniform number)
      \return player number
     */
    int playerNumber() const { return M_player_number; }

    /*!
      \brief set a player number (not a uniform number)
      \param num player number
     */
    void setPlayerNumber( const int num ) { M_player_number = num; }

    /*!
      \brief get the configulation directory path
      \return configulation directory path
     */
    const std::string & configDir() const { return M_config_dir; }

    //
    // debug
    //

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debug() const { return M_debug; }

    /*!
      \brief get the debug log directory path string.
      \return the debug log directory string.
     */
    const std::string & logDir() const { return M_log_dir; }

    //
    // debug server settings
    //

    /*!
      \brief get the debug flag whether debug information is sent to a debug server
      \return debug flag whether debug information is sent to a debug server
     */
    bool debugServerConnect() const { return M_debug_server_connect; }

    /*!
      \brief get the debug flag whether debug information is written to files
      \return debug flag whether debug information is written to files
     */
    bool debugServerLogging() const { return M_debug_server_logging; }

    /*!
      \brief get the debug server host name string
      \return debug server host name string
     */
    const std::string & debugServerHost() const { return M_debug_server_host; }

    /*!
      \brief get the debug server port number
      \return debug server port number
     */
    int debugServerPort() const { return M_debug_server_port; }

    //
    // offline client
    //

    /*!
      \brief get the switch for offline logging. this value is ignored if client runs in offline clinet mode.
      \return switch value for offline logging.
     */
    bool offlineLogging() const { return M_offline_logging; }

    /*!
      \brief get the offline client log file extention string.
      \return the offline client log file extention string.
     */
    const std::string & offlineLogExt() const { return M_offline_log_ext; }

    /*!
      \brief get the uniform number for offline client mode.
      1-11 means offline mode, other values mean online mode.
      \return the uniform number for offline client mode.
     */
    int offlineClientNumber() const { return M_offline_client_number; }

    //
    // debug logging
    //

    /*!
      \brief get the start time for receoding the debug log
      \return the start time for receoding the debug log
     */
    int debugStartTime() const { return M_debug_start_time; }

    /*!
      \brief get the end time for receoding the debug log
      \return the end time for receoding the debug log
     */
    int debugEndTime() const { return M_debug_end_time; }

    /*!
      \brief get the debug log file extention string.
      \return the debug log file extention string.
     */
    const std::string & debugLogExt() const { return M_debug_log_ext; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugSystem() const { return M_debug_system; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugSensor() const { return M_debug_sensor; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugWorld() const { return M_debug_world; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugAction() const { return M_debug_action; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugIntercept() const { return M_debug_intercept; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugKick() const { return M_debug_kick; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugHold() const { return M_debug_hold; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugDribble() const { return M_debug_dribble; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugPass() const { return M_debug_pass; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugCross() const { return M_debug_cross; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugShoot() const { return M_debug_shoot; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugClear() const { return M_debug_clear; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugBlock() const { return M_debug_block; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugMark() const { return M_debug_mark; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugPositioning() const { return M_debug_positioning; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugRole() const { return M_debug_role; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugPlan() const { return M_debug_plan; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugTeam() const { return M_debug_team; }


    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugCommunication() const { return M_debug_communication; }


    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugAnalyzer() const { return M_debug_analyzer; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugActionChain() const { return M_debug_action_chain; }

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debugTraining() const { return M_debug_training; }
};

}

#endif
