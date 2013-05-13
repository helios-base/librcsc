// -*-c++-*-

/*!
  \file trainer_config.h
  \brief trainer configuration Header File
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

#ifndef RCSC_TRAINER_CONFIG_H
#define RCSC_TRAINER_CONFIG_H

#include <string>
#include <iosfwd>

namespace rcsc {

class ParamMap;
class ParamParser;

/*!
  \class TrainerConfig
  \brief trainer configuration parameters
 */
class TrainerConfig {
private:

    ParamMap * M_param_map; //!< parameter map instalce

    // basic setting

    std::string M_team_name; //!< our team name string
    double      M_version; //!< client version

    //! select timeout interval
    int M_interval_msec;
    //! max server message wait time by second
    int M_server_wait_seconds;

    //! server host name
    std::string M_rcssserver_host;
    //! server port number
    int         M_rcssserver_port;

    //! zlib compression level for the compression command
    int M_compression;

    //! if true trainer will send (eye on) command
    bool M_use_eye;

    //! if true trainer will send (ear on) command
    bool M_use_ear;


    //
    // debug
    //

    //! if false, debug log file is never opened.
    bool M_debug;

    //! log written directory. this is common for all type log
    std::string M_log_dir;

    //
    // offline client settings
    //

    bool M_offline_logging; //!< if true, client write all sensory info to the file.
    std::string M_offline_log_ext; //!< the extension string of offline client log file.

    bool M_offline_client_mode; //!< offline client mode switch.

    //
    // debug logging
    //

    //! the extension string of debug log file
    std::string M_debug_log_ext;

    // debug output switches
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

public:

    /*!
      \brief init variables by default value. create ParamMap instance
     */
    TrainerConfig();

    /*!
      \brief delete ParamMap instance
     */
    ~TrainerConfig();

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

private:
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
      \brief get the interrupt interval for decision making
      \return interval milli-seconds
     */
    int intervalMSec() const { return M_interval_msec; }

    /*!
      \brief get the maximum seconds to wait server response.
      \return the maximum seconds to wait server response.
     */
    int serverWaitSeconds() const { return M_server_wait_seconds; }

    /*!
      \brief get the server host name string
      \return server host name string
     */
    const std::string & host() const { return M_rcssserver_host; }

    /*!
      \brief get the connection port number
      \return connection port number
     */
    int port() const { return M_rcssserver_port; }

    /*!
      \brief get the message compression level
      \return message compression level
     */
    int compression() const { return M_compression; }

    /*!
      \brief get the eye mode status
      \return true if eye mode is on, othewise false
     */
    bool useEye() const { return M_use_eye; }

    /*!
      \brief get the ear mode status
      \return true if ear mode is on, othewise false
     */
    bool useEar() const { return M_use_eye; }

    //
    // debug
    //

    /*!
      \brief get the debug flag
      \return debug flag
     */
    bool debug() const { return M_debug; }

    /*!
      \brief get the debug log directory string.
      \return the debug log directory string.
     */
    const std::string & logDir() const { return M_log_dir; }

    //
    // offline client settings
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
      \brief get the offline client mode switch.
      \return offline client mode switch.
     */
    bool offlineClientMode() const { return M_offline_client_mode; }


    //
    // debug logging
    //

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

};

}

#endif
