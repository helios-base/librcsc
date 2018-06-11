// -*-c++-*-

/*!
  \file coach_config.h
  \brief coach configuration Header File
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

#ifndef RCSC_COACH_CONFIG_H
#define RCSC_COACH_CONFIG_H

#include <string>
#include <iosfwd>

namespace rcsc {

class ParamMap;
class ParamParser;

/*!
  \class CoachConfig
  \brief coach configuration parameters
 */
class CoachConfig {
private:

    ParamMap * M_param_map; //!< parameter map instance


    // basic setting

    std::string M_team_name; //!< our team name string
    std::string M_coach_name; //!< coach name string
    double      M_version; //!< client version

    //! if true coach should send its name by init command
    bool M_use_coach_name;

    //! select timeout interval
    int M_interval_msec;
    //! max server message wait time by second
    int M_server_wait_seconds;

    //! server host name
    std::string M_rcssserver_host;
    //! server port number
    int M_rcssserver_port;

    //! zlib compression level for the compression command
    int M_compression;

    //! if true, coach should send (eye on) command
    bool M_use_eye;

    //! if true, coach will analyze say message
    bool M_hear_say;

    //! shift parameter to encrypt audio message
    int M_audio_shift;

    //! if true, coach will try to analyze opponent team players' player type
    bool M_analyze_player_type;

    //! if true, coach send advise
    bool M_use_advise;
    //! if true, coach send freeform message
    bool M_use_freeform;

    //! use hetero player
    bool M_use_hetero;

    //! if true, coach may send the team logo by team_graphic command.
    bool M_use_team_graphic;

    //! path to the team graphic xpm file
    std::string M_team_graphic_file;

    //! maximum number of team_graphic command per cycle
    int M_max_team_graphic_per_cycle;

    //
    // debug
    //

    bool M_debug; //!< if false, log file or debug client are never opened

    //! log written directory. this is common for all type log
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
    CoachConfig();

    /*!
      \brief delete ParamMap instance
     */
    ~CoachConfig();

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
      \brief create parameter map
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
      \brief get the coach name string
      \return coach name string
     */
    const std::string & coachName() const { return M_coach_name; }

    /*!
      \brief get the flag whether coach name is used or not
      \return flag whether coach name is used or not
     */
    bool useCoachName() const { return M_use_coach_name; }

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
      \brief get the server host name
      \return server host name
     */
    const std::string & host() const { return M_rcssserver_host; }

    /*!
      \brief get the server port number
      \return server port number
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
    bool hearSay() const { return M_hear_say; }

    /*
      \brief shift value to encrypt audio encoder
      \return shift value
     */
    int audioShift() const { return M_audio_shift; }

    /*!
      \brief get the player type analyzer mode
      \return true if coach tries to analyze opponent player types.
     */
    bool analyzePlayerType() const { return M_analyze_player_type; }

    /*!
      \brief get the advise mode
      \return true if coach tries to advise to players.
     */
    bool useAdvise() const { return M_use_advise; }

    /*!
      \brief get the freeform message mode
      \return true if coach uses freeform message.
     */
    bool useFreeform() const { return M_use_freeform; }

    /*!
      \brief get the change_player_type mode
      \return true if coach tries to change player types.
     */
    bool useHetero() const { return M_use_hetero; }

    /*!
      \brief get the team_graphic mode
      \return true if coach tries to send team graphics to server.
     */
    bool useTeamGraphic() const { return M_use_team_graphic; }

    /*!
      \brief get the file path to the team graphic file.
      \return file path string
     */
    const std::string & teamGraphicFile() const { return M_team_graphic_file; }

    /*!
      \brief get the maximum number of team_graphic message per one cycle.
      \return the maximum number of team_graphic message per one cycle.
     */
    int maxTeamGraphicPerCycle() const { return M_max_team_graphic_per_cycle; }

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
