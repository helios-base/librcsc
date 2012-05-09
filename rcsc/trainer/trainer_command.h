// -*-c++-*-

/*!
  \file trainer_command.h
  \brief trainer command classes Header File
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

#ifndef RCSC_TRAINER_COMMAND_H
#define RCSC_TRAINER_COMMAND_H

#include <rcsc/geom/vector_2d.h>
#include <rcsc/types.h>

#include <string>
#include <iostream>

namespace rcsc {

/*!
  \class TrainerCommand
  \brief abstract trainer command class
*/
class TrainerCommand {
public:
    /*!
      \brief trainer command type Id
    */
    enum Type {
        INIT, //!< connection command

        CHECK_BALL,
        LOOK,
        TEAM_NAMES,

        EAR,
        EYE,

        START,
        CHANGE_MODE,
        MOVE,
        RECOVER,
        CHANGE_PLAYER_TYPE,
        SAY,

        COMPRESSION,
        DONE,

        ILLEGAL
    };

protected:
    /*!
      \brief constructor is protected because this is abstract class.
    */
    TrainerCommand()
      { }

public:
    /*!
      \brief virtual destructor, but nothing to do
    */
    virtual
    ~TrainerCommand()
      { }

    /*!
      \brief get command type (pure virtual)
      \return command type Id
    */
    virtual
    Type type() const = 0;

    /*!
      \brief put command string to ostream (pure virtual)
      \param to referent to the output stream
      \return referenct to the output stream
    */
    virtual
    std::ostream & toCommandString( std::ostream & to ) const = 0;

    /*!
      \brief get command name (pure virtual)
      \return command name string
    */
    virtual
    std::string name() const = 0;
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerInitCommand
  \brief trainer's connection command

  <pre>
  Format:
  <- (init (version <ver>))
  Response:
  -> (init ok)
  -> (error illegal_command_for)  : when version is illegal
  </pre>
*/
class TrainerInitCommand
    : public TrainerCommand {
private:
    double M_version; //!< client version

public:
    /*!
      \brief construct with client version
    */
    explicit
    TrainerInitCommand( const double & version )
        : M_version( version )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return INIT;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "init" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerCheckBallCommand
  \brief command to check ball status

  <pre>
  Format:
  <- (check_ball)
  Response:
  -> (ok check_ball <ball_status>)

  ball_status :- "in_field" | "goal_l" | "goal_r" | "out_of_field"
  </pre>
*/
class TrainerCheckBallCommand
    : public TrainerCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    TrainerCheckBallCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CHECK_BALL;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "check_ball" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerLookCommand
  \brief command to check field status

  <pre>
  Format:
  <- (look)
  Response:
  -> (ok look TIME GOAL_INFO BALL_INFO PLAYER_INFO ...)
  </pre>
*/
class TrainerLookCommand
    : public TrainerCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    TrainerLookCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return LOOK;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "look" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerTeamNamesCommand
  \brief command to require name of both teams

  <pre>
  Format:
  <- (team_names)
  Response:
  -> (ok team_names (team l <teamname>) (team r <teamname>))
  </pre>
*/
class TrainerTeamNamesCommand
    : public TrainerCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    TrainerTeamNamesCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return TEAM_NAMES;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "team_names" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerEarCommand
  \brief command to enable/disable trainer's ear

  <pre>
  Format:
  <- (ear <onoff>)
  Response:
  -> (ok ear on) | (ok ear off)
  -> (error illegal_command_form)
  </pre>
*/
class TrainerEarCommand
    : public TrainerCommand {
private:
    bool M_on; //!< switch to specify whether ear is enable
public:
    /*!
      \brief construct with ear mode flag
      \param on switch to specify whether ear is enable
    */
    explicit
    TrainerEarCommand( const bool on )
        : M_on( on )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return EAR;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "ear" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerEyeCommand
  \brief command to enable/dispable trainer's eye

  <pre>
  Format:
  <- (eye <onoff>)
  Response:
  -> (ok eye on) | (ok eye off)
  -> (error illegal_command_form)
  </pre>
*/
class TrainerEyeCommand
    : public TrainerCommand {
private:
    bool M_on; //!< switch to specify whether ear is enable
public:
    /*!
      \brief construct with eye mode flag
      \param on switch to specify whether eye is enable
    */
    explicit
    TrainerEyeCommand( const bool on )
        : M_on( on )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return EYE;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "eye" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerKickOffCommand
  \brief command to start the game

  change the playmode from before_kick_off to kick_off_[lr]

  <pre>
  Format:
  <- (start)
  Response:
  -> (ok start)
  </pre>
*/
class TrainerKickOffCommand
    : public TrainerCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    TrainerKickOffCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return START;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "start" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerChangeModeCommand
  \brief command to change playmode

  <pre>
  Format:
  <- (change_mode <playmode>)
  Response:
  -> (ok change_mode)
  -> (error illegal_mode)
  </pre>
*/
class TrainerChangeModeCommand
    : public TrainerCommand {
private:
    PlayMode M_playmode; //!< playmode type

public:
    /*!
      \brief construct with playmode type
      \param mode playmode type
    */
    explicit
    TrainerChangeModeCommand( const PlayMode mode )
        : M_playmode( mode )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CHANGE_MODE;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "change_mode" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerMoveBallCommand
  \brief command to move ball

  <pre>
  Format:
  <- (move (ball) <x> <y>[ <angle> <vx> <vy>])
  Response:
  -> (ok move)
  -> (error illegal_command_form)
  </pre>
*/
class TrainerMoveBallCommand
    : public TrainerCommand {
private:
    Vector2D M_pos; //!< move target point
    Vector2D M_vel; //!< velocity after move

public:
    /*!
      \brief construct with move target point. no velocity.
      \param x move target x
      \param y move target y
    */
    TrainerMoveBallCommand( const double & x,
                            const double & y );

    /*!
      \brief construct with move target point. no velocity.
      \param pos move target
    */
    explicit
    TrainerMoveBallCommand( const Vector2D & pos );

    /*!
      \brief construct with move target point and ball velocity.
      \param x move target x
      \param y move target y
      \param vx velocity x after move
      \param vy velocity y after move
    */
    TrainerMoveBallCommand( const double & x,
                            const double & y,
                            const double & vx,
                            const double & vy );

    /*!
      \brief construct with move target point. no velocity.
      \param pos move target position
      \param vel velocity after move
    */
    TrainerMoveBallCommand( const Vector2D & pos,
                            const Vector2D & vel );

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return MOVE;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "move" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerMovePlayerCommand
  \brief command to move player

  <pre>
  Format:
  <- (move (player <teamname> <unum>) <x> <y> [<angle> [<vx> <vy>]])
  Response:
  -> (ok move)
  -> (error illegal_command_form)
  </pre>
*/
class TrainerMovePlayerCommand
    : public TrainerCommand {
private:
    std::string M_team_name; //!< target player's team name string
    int M_unum; //!< target player's uniform number
    Vector2D M_pos; //!< move target point
    double M_angle; //!< player's body angle after move. if -360.0, no body angle
    Vector2D M_vel; //!< player's velocity after move

    /*!
      \brief check if target player's uniform number is valid.
      \return true if valid uniform number
    */
    bool check() const;

public:
    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param x target point x
      \param y target point y
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const double & x,
                              const double & y );

    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param pos target point
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const Vector2D & pos );

    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param x target point x
      \param y target point y
      \param angle player's body angle after move
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const double & x,
                              const double & y,
                              const AngleDeg & angle );

    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param pos target point
      \param angle player's body angle after move
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const Vector2D & pos,
                              const AngleDeg & angle );

    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param x target point x
      \param y target point y
      \param angle player's body angle after move
      \param vx player's velocity x after move
      \param vy player's velocity y after move
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const double & x,
                              const double & y,
                              const AngleDeg & angle,
                              const double & vx,
                              const double & vy );

    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param pos target point
      \param angle player's body angle after move
      \param vx player's velocity x after move
      \param vy player's velocity y after move
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const Vector2D & pos,
                              const AngleDeg & angle,
                              const double & vx,
                              const double & vy );

    /*!
      \brief construct with xy value, no angle, no velocity
      \param team_name target player's team name
      \param unum target player's uniform number
      \param pos target point
      \param angle player's body angle after move
      \param vel player's velocity after move
    */
    TrainerMovePlayerCommand( const std::string & team_name,
                              const int unum,
                              const Vector2D & pos,
                              const AngleDeg & angle,
                              const Vector2D & vel );

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return MOVE;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string("move");
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerRecoverCommand
  \brief command to recover players' stamina

  <pre>
  Format:
  <- (recover)
  Response:
  -> (ok recover)
  </pre>
*/
class TrainerRecoverCommand
    : public TrainerCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    TrainerRecoverCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return RECOVER;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "recover" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerChangePlayerTypeCommand
  \brief command to change player type

  <pre>
  Format:
  <- (change_player_type <teamname> <unum> <typeid>)
  Response:
  -> (ok change_player_type <teamname> <unum> <typeid>)
  -> (warning no_team_found)
  -> (error out_of_range_player_type)
  -> (warning no_such_player)
  </pre>
*/
class TrainerChangePlayerTypeCommand
    : public TrainerCommand {
private:
    std::string M_team_name; //!< target player's team name
    int M_unum; //!< target player's uniform number
    int M_type; //!< player type Id
public:
    /*!
      \brief construct with target player and type id
      \param team_name target player's team name
      \param unum target player's uniform number
      \param type player type Id
    */
    TrainerChangePlayerTypeCommand( const std::string & team_name,
                                    const int unum,
                                    const int type );

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CHANGE_PLAYER_TYPE;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "change_player_type" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerSayCommand
  \brief trainer's say command

  <pre>
  Format:
  <- (say <message>)
  Response:
  -> (ok say)
  </pre>
*/
class TrainerSayCommand
    : public TrainerCommand {
private:
    std::string M_message; //!< message string
public:
    /*!
      \brief construct with message string
      \param msg message to be sent
    */
    explicit
    TrainerSayCommand( const std::string & msg )
        : M_message( msg )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return SAY;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "say" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerCompressionCommand
  \brief command to set message compression level

  <pre>
  Format:
  <- (compression <level>)
  Response:
  -> (ok compression <level>)
  -> (warning compression_unsupported)
  </pre>
*/
class TrainerCompressionCommand
    : public TrainerCommand {
private:
    int M_level; //!< gzip compression level

public:
    /*!
      \brief construct with compression level
    */
    explicit
    TrainerCompressionCommand( const int level )
        : M_level( level )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return COMPRESSION;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "compression" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class TrainerDoneCommand
  \brief command to send done command for (think) message in synch_mode

  <pre>
  Format:
  <- (done)
  </pre>
*/
class TrainerDoneCommand
    : public TrainerCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    TrainerDoneCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return DONE;
      }

    /*!
      \brief put command string to ostream
      \param to referent to the output stream
      \return referenct to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "done" );
      }
};

}

#endif

/*

----------
Offline Coach Initialization Command
See rcssserver/src/netif.C : void Coach::parse_init

"(init (version VERSION))"
if VERSION is illegal
-> "(error illegal_command_form)"
else
-> "(init ok)"

receive parameter info
receive changed players info
----------

----------

----------
Offline Coach Command List
See rcssserver/src/netif.C : void Coach::parse_command(const char *command)l


if use irregal character for command string
-> "(error illegal_command_form)"
else if irregal command
-> "(error unknown_command)"

----------
"(start)" : start game (change playmode before_kickoff to kick_off_?)
-> return "(ok start)"
----------
"(change_mode NEW_PLAYMODE)" : change playmode
if NEW_PLAYMODE string is NULL
-> "(error illegal_command_form)"
else NEW_PLAYMODE is not valid
-> "(error illegal_mode)"
else
-> "(ok change_mode)"
----------
"(move OBJ_NAME X Y ANG VX VY)" : move pos, set ang, vel
"(move OBJ_NAME X Y ANG)" : move pos, set ang
"(move OBJ_NAME X Y)" : move pos
if (X,Y,ANG,VX,VY is not a number)
-> "(error illegal_command_form)"
else if (arg != 2 || arg != 3 || arg != 5)
-> "(error illegal_command_form)"
else
-> "(ok move)"
----------
"(look)" : get all movable objects' positional information
-> "(ok look TIME GOAL_INFO BALL_INFO PLAYER_INFO ...)"
----------
"(team_names)" : get team name
-> "(ok team_names (team l TEAM_NAME) (team r TEAM_NAME))"
----------
"(recover)" : recover all player's stamina, effort and recover
-> "(ok recover)"
----------
"(check_ball)" : check ball positional state
-> (ok check_ball BALL_POS_INFO)
BALL_POS_INFO :- "in_field" | "goal_l" | "goal_r" | "out_of_field"
----------
"(say MESSAGE)"
if MESSAGE is NULL string
-> "(error illegal_command_form)"
else
-> "(ok say)"
MESSAGE is [-0-9a-zA-Z ()./+*?<>_]
----------
"(ear ONOFF)" : turn on/off to get audio info
if ONOFF is NULL
-> "(error illegal_command_form)"
else if ONOFF_MODE == "on"
-> "(ok ear on)"
else if ONOFF_MODE == "off"
-> "(ok ear off)"
else
-> "(error illegal_command_form)"
----------
"(eye ONOFF)" : turn on/off to get visual info
if ONOFF is NULL
-> "(error illegal_command_form)"
else if ONOFF_MODE == "on"
-> "(ok eye on)"
else if ONOFF_MODE == "off"
-> "(ok eye off)"
else
-> "(error illegal_command_form)"
----------
"(change_player_type TEAM_NAME UNUM PLAYER_TYPE_ID)" : change hetero player type
if TEAM_NAME is not match
-> "(warning no_team_found)"
else if PLAYER_TYPE_ID is out of range (ID<0 or PlayerTypes<ID)
-> "(error out_of_range_player_type)"
else if UNUM is not match
-> "(warning no_such_player)"
else
-> "(ok change_player_type TEAM_NAME UNUM PLAYER_TYPE_ID)"
----------
"(done)" : thinking end nortification for sync mode
----------
"(compression LEVEL)" : set zlib compression level
if LEVEL is NULL string
-> "(error illegal_command_form)"
else if not HAVE_LIBZ
-> "(warning compression_unsupported)"
else if (LEVEL < 0 || 9 < LEVEL)
-> "(error illegal_command_form)"
else
-> "(ok compression LEVEL)"
----------
*/
