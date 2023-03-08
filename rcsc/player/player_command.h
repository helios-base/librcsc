// -*-c++-*-

/*!
  \file player_command.h
  \brief player command classes Header File
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

#ifndef RCSC_PLAYER_COMMAND_H
#define RCSC_PLAYER_COMMAND_H

#include <rcsc/player/view_mode.h>
#include <rcsc/geom/vector_2d.h>
#include <rcsc/geom/angle_deg.h>

#include <string>
#include <iostream>
#include <cmath>

namespace rcsc {

/*!
  \class PlayerCommand
  \brief abstract player command class
*/
class PlayerCommand {
public:
    /*!
      \brief player client command Id
    */
    enum Type {
        // connection commands
        INIT, //!< server connection command
        RECONNECT, //!< server reconnection command
        BYE, //!< server disconnection command
        // base commands
        MOVE,
        DASH,
        TURN,
        KICK,
        CATCH,
        TACKLE,
        // support commands
        TURN_NECK,
        CHANGE_VIEW,
        CHANGE_FOCUS,
        SAY,
        POINTTO,
        ATTENTIONTO,
        // mode change commands
        CLANG,
        EAR,
        // other commands
        SENSE_BODY,
        SCORE,
        COMPRESSION,
        // synch_mode command
        DONE,

        ILLEGAL
    };

protected:
    /*!
      \brief constructor is protected because this is abstract class.
    */
    PlayerCommand()
      { }

public:
    /*!
      \brief virtual destructor, but nothing to do
    */
    virtual
    ~PlayerCommand()
      { }

    /*!
      \brief get command type (pure virtual)
      \return command type Id
    */
    virtual
    Type type() const = 0;

    /*!
      \brief put command string to ostream (pure virtual)
      \param to reference to the output stream
      \return reference to the output stream
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
  \class PlayerInitCommand
  \brief player's init command

  <pre>
  Format:
  <- (init <teamname>[ (version <ver>)[ (goalie)]])
  Response:
  -> (init <side> <unum> <playmode>)
  </pre>
*/
class PlayerInitCommand
    : public PlayerCommand {
private:
    std::string M_team_name; //!< our team name
    double M_version; //!< client version
    bool M_goalie; //!< true if goalie
public:
    /*!
      \brief construct with init parameters
      \param team_name player's team name
      \param version player's client version (default: 3.0)
      \param goalie if true, player start as goalie
    */
    explicit
    PlayerInitCommand( const std::string & team_name,
                       const double & version = 3.0,
                       const bool goalie = false );

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
      \param to reference to the output stream
      \return reference to the output stream
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
  \class PlayerReconnectCommand
  \brief reconnect command

  <pre>
  Format:
  <- (reconnect <teamname> <unum>)
  Response:
  -> (reconnect <side> <playmode>)
  </pre>
*/
class PlayerReconnectCommand
    : public PlayerCommand {
private:
    std::string M_team_name; //!< team name string
    int M_unum; //!< uniform number to reconnect
public:
    /*!
      \brief construct with reconnect parameters
      \param team_name team name string
      \param unum unuform number to reconnect
    */
    PlayerReconnectCommand( const std::string & team_name,
                            const int unum );

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return RECONNECT;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "reconnect" );
      }
};


//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerByeCommand
  \brief player's disconnect command

  <pre>
  Format:
  <- (bye)
  </pre>
*/
class PlayerByeCommand
    : public PlayerCommand {
private:

public:
    /*!
      \brief constructor. nothingto do
    */
    PlayerByeCommand();

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return BYE;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "bye" );
      }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerBodyCommand
  \brief abstract body command
*/
class PlayerBodyCommand
    : public PlayerCommand {
protected:
    /*!
      \brief default constructor is protected because this is abstract class
    */
    PlayerBodyCommand()
      { }

public:
    /*!
      \brief virtual destructor. nothing to do.
    */
    virtual
    ~PlayerBodyCommand()
      { }

    /*!
      \brief get command type (pure virtual)
      \return command type Id
    */
    virtual
    Type type() const = 0;

    /*!
      \brief put command string to ostream (pure virtual)
      \param to reference to the output stream
      \return reference to the output stream
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
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerMoveCommand
  \brief player's move command

  <pre>
  Format:
  <- (move <x> <y>)
  <- (move (<x> <y>))
  </pre>
*/
class PlayerMoveCommand
    : public PlayerBodyCommand {
private:
    double M_x; //!< moved point coordinate X
    double M_y; //!< moved point coordinate Y
public:
    /*!
      \brief construct with move target point
      \param x move target x
      \param y move target y
    */
    PlayerMoveCommand( const double & x,
                       const double & y )
        : M_x( x )
        , M_y( y )
      { }

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
      \param to reference to the output stream
      \return reference to the output stream
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

    /*!
      \brief get move command parameter
      \return move position
     */
    Vector2D movePos() const
      {
          return Vector2D( M_x, M_y );
      }

};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerDashCommand
  \brief player's dash command

  <pre>
  Format:
  <- (dash <power>)
     (dash <power> <dir>)
  </pre>
*/
class PlayerDashCommand
    : public PlayerBodyCommand {
private:
    double M_power; //!< dash power
    double M_dir; //!< dash direction
public:
    /*!
      \brief construct with dash power
      \param power dash power
      \param dir dash direction
    */
    explicit
    PlayerDashCommand( const double & power,
                       const double & dir = 0.0 )
        : M_power( power )
        , M_dir( dir )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return DASH;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "dash" );
      }

    /*!
      \brief get dash command parameter
      \return dash power
     */
    double dashPower() const
      {
          return M_power;
      }

    /*!
      \brief get dash command parameter
      \return dash direction
     */
    double dashDir() const
      {
          return M_dir;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerTurnCommand
  \brief turn command

  <pre>
  Format:
  <- (turn <moment>)
  </pre>
*/
class PlayerTurnCommand
    : public PlayerBodyCommand {
private:
    double M_moment; //!< moment
public:
    /*!
      \brief construct with turn moment
      \param moment turn moment
    */
    explicit
    PlayerTurnCommand( const double & moment )
        : M_moment( moment )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return TURN;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "turn" );
      }

    /*!
      \brief get turn command parameter
      \return turn moment
     */
    double turnMoment() const
      {
          return M_moment;
      }
};


//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerKickCommand
  \brief kick command

  <pre>
  Format:
  <- (kick <power> <dir>)
  </pre>
*/
class PlayerKickCommand
    : public PlayerBodyCommand {
private:
    double M_power; //!< kick power
    double M_dir; //!< relative to body angle
public:
    /*!
      \brief construct with kick parameters
      \param power kick power
      \param dir kick accel dir relative to body
    */
    PlayerKickCommand( const double & power,
                       const double & dir )
        : M_power( power )
        , M_dir( dir )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return KICK;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "kick" );
      }

    /*!
      \brief get kick command parameter
      \return kick power
     */
    double kickPower() const
      {
          return M_power;
      }

    /*!
      \brief get kick command parameter
      \return kick direction
     */
    double kickDir() const
      {
          return M_dir;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerCatchCommand
  \brief goalie's catch command

  <pre>
  Format:
  <- (catch <dir>)
  </pre>
*/
class PlayerCatchCommand
    : public PlayerBodyCommand {
private:
    double M_dir; //!< catch direction relative to body angle
public:
    /*!
      \brief construct with catch direction
      \param rel_dir catch direction relative to body angle
    */
    explicit
    PlayerCatchCommand( const double & rel_dir )
        : M_dir( rel_dir )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CATCH;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "catch" );
      }

    /*!
      \brief get catch command parameter
      \return catch direction
     */
    double catchDir() const
      {
          return M_dir;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerTackleCommand
  \brief player's tackle command

  <pre>
  Format:
  <- (tackle <power_or_dir>)
  <- (tackle <power_or_dir> <foul>)
  </pre>
*/
class PlayerTackleCommand
    : public PlayerBodyCommand {
private:
    double M_power_or_dir; //!< tackle power or direction
    bool M_foul; //!< intentional foul switch
public:
    /*!
      \brief construct with tackle power
      \param power_or_dir tackle power or direction
    */
    explicit
    PlayerTackleCommand( const double & power_or_dir )
        : M_power_or_dir( power_or_dir ),
          M_foul( false )
      { }

    PlayerTackleCommand( const double & power_or_dir,
                         const bool foul )
        : M_power_or_dir( power_or_dir ),
          M_foul( foul )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return TACKLE;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "tackle" );
      }

    /*!
      \brief get tackle command parameter
      \return tackle power
     */
    double tacklePowerOrDir() const
      {
          return M_power_or_dir;
      }

    bool tackleFoul() const
      {
          return M_foul;
      }
};

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerSupportCommand
  \brief abstract support command
*/
class PlayerSupportCommand
    : public PlayerCommand {
protected:
    /*!
      \brief constructor is protected because this is abstract class.
    */
    PlayerSupportCommand()
      { }

public:
    /*!
      \brief virtual destructor, but nothing to do
    */
    virtual
    ~PlayerSupportCommand()
      { }

    /*!
      \brief get command type (pure virtual)
      \return command type Id
    */
    virtual
    Type type() const = 0;

    /*!
      \brief put command string to ostream (pure virtual)
      \param to reference to the output stream
      \return reference to the output stream
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
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerTurnNeckCommand
  \brief player's turn neck command

  <pre>
  Format:
  <- (turn_neck <moment>)
  </pre>
*/
class PlayerTurnNeckCommand
    : public PlayerSupportCommand {
private:
    double M_moment; //!< moment to turn neck
public:
    /*!
      \brief construct with moment to turn neck
      \param moment turn neck moment
    */
    explicit
    PlayerTurnNeckCommand( const double & moment )
        : M_moment( moment )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return TURN_NECK;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "turn_neck" );
      }

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    const
    double & moment() const
      {
          return M_moment;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerChangeViewCommand
  \brief player's change view command

  <pre>
  Format:
  <- (change_view <width> <quality>)

  <width> :- narrow | normal | wide
  <quality> :- high | low
  </pre>
*/
class PlayerChangeViewCommand
    : public PlayerSupportCommand {
private:
    ViewWidth M_width; //!< view width
    ViewQuality M_quality; //!< view quality
public:
    /*!
      \brief construct with view mode objects
      \param w view width object
      \param q view quality object
    */
    PlayerChangeViewCommand( const ViewWidth & w,
                             const ViewQuality & q )
        : M_width( w )
        , M_quality( q )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CHANGE_VIEW;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "change_view" );
      }

    /*!
      \brief get view width of this command
      \brief ViewWidth of this command
    */
    const
    ViewWidth & width() const
      {
          return M_width;
      }

    /*!
      \brief get view quality of this command
      \brief ViewQuality of this command
    */
    const
    ViewQuality & quality() const
      {
          return M_quality;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerChangeFocusCommand
  \brief player's change focus command

  <pre>
  Format:
  <- (change_focus <moment_dist> <moment_dir>)
  </pre>
*/
class PlayerChangeFocusCommand
    : public PlayerSupportCommand {
private:
    double M_moment_dist; //!< this value is added to the current focus distance, and the result is limited to [0.0, 40.0].
    double M_moment_dir; //!< this value is added to the current focus direction, and the result is limited to visible angle.

    PlayerChangeFocusCommand() = delete; // not used
public:

    /*!
      \brief construct with command parameters
      \param moment_dist moment for the distance
      \param moment_dir moment for the direction
     */
    PlayerChangeFocusCommand( const double moment_dist,
                              const double moment_dir )
        : M_moment_dist( moment_dist ),
          M_moment_dir( moment_dir )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CHANGE_FOCUS;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get thencommand name
      \return command name string
    */
    std::string name() const
      {
          return std::string( "change_focus" );
      }

    /*!
      \brief get the command parameter
      \return the value of moment distance
    */
    double momentDist() const
      {
          return M_moment_dist;
      }

    /*!
      \brief get the command parameter
      \return the value of moment direction
    */
    double momentDir() const
      {
          return M_moment_dir;
      }

};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerSayCommand
  \brief player's say command

  <pre>
  Format:
  <- (say "<msg>")   : version 8.0 or later. double quatation is recommended.
  <- (say <msg>)
  </pre>
*/
class PlayerSayCommand
    : public PlayerSupportCommand {
private:
    std::string M_message; //!< message body
    double M_version; //!< client version
public:

    /*!
      \brief construct with client version, no message
      \param version player's client version
     */
    explicit
    PlayerSayCommand( const double & version )
        : M_version( version )
      { }

    /*!
      \brief construct with cstring say message
      \param msg cstring message
      \param version player's client version
    */
    PlayerSayCommand( const char * msg,
                      const double & version )
        : M_message( msg )
        , M_version( version )
      { }
    /*!
      \brief construct with say message string
      \param msg message string
      \param version player's client version
    */
    PlayerSayCommand( const std::string & msg,
                      const double & version )
        : M_message( msg )
        , M_version( version )
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
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "say" );
      }

    /*!
      \brief assign new message
      \param msg new string
     */
    void assign( const std::string & msg )
      {
          M_message = msg;
      }

    /*!
      \brief append new string
      \param msg appended string
     */
    void append( const std::string & msg )
      {
          M_message += msg;
      }

    /*!
      \brief get command parameter
      \return say message
     */
    const
    std::string & message() const
      {
          return M_message;
      }

};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerPointtoCommand
  \brief player's turn neck command

  <pre>
  Format:
  <- (pointto <dist> <dir>)
  <- (pointto off)
  </pre>
*/
class PlayerPointtoCommand
    : public PlayerSupportCommand {
private:
    bool M_on; //!< on/off switch
    double M_dist; //!< target distance
    double M_dir; //! target angle relative to body angle
public:
    /*!
      \brief construct off type pointto command
    */
    PlayerPointtoCommand()
        : M_on( false )
        , M_dist( 0.0 )
        , M_dir( 0.0 )
      { }

    /*!
      \brief construct on type pointto command with point target
      \param dist point target distance
      \param rel_dir point target angle relative to body angle
    */
    PlayerPointtoCommand( const double & dist,
                          const double & rel_dir )
        : M_on( true )
        , M_dist( dist )
        , M_dir( rel_dir )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return POINTTO;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "pointto" );
      }


    /*!
      \brief get pointto command switch
      \return true if pointo is on
     */
    bool pointtoOn() const
      {
          return M_on;
      }

    /*!
      \brief get pointto command parameter
      \return pointto distance
     */
    const
    double & pointtoDist() const
      {
          return M_dist;
      }

    /*!
      \brief get pointto command parameter
      \return pointto dir
     */
    const
    double & pointtoDir() const
      {
          return M_dist;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerAttentiontoCommand
  \brief player's attentionto command

  <pre>
  Format:
  <- (attentionto <side> <unum>)
  <- (attentionto off)

  side :- our | opp | l | left | r | right | TEAMNAME
  </pre>
*/
class PlayerAttentiontoCommand
    : public PlayerSupportCommand {
public:
    /*!
      \brief target player's side type
    */
    enum SideType {
        OUR,
        OPP,
        NONE,
    };

private:
    SideType M_side; //!< our/opp switch
    int M_number; //!< target player's uniform number
public:
    /*!
      \brief construct off type attentionto command
    */
    PlayerAttentiontoCommand()
        : M_side( NONE )
        , M_number( 0 )
      { }

    /*!
      \brief construct on type attentionto command
      \param side target player's side
      \param unum target player's uniform number
    */
    PlayerAttentiontoCommand( const SideType side,
                              const int unum )
        : M_side( side )
        , M_number( unum )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return ATTENTIONTO;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "attentionto" );
      }

    /*!
      \brief get on/off flag
      \return true if this command is on type
    */
    bool isOn() const
      {
          return M_side != NONE;
      }

    /*!
      \brief get target player's side type
      \return target player's side type
    */
    SideType side() const
      {
          return M_side;
      }

    /*!
      \brief get target player's uniform number
      \return target player's uniform number
    */
    int number() const
      {
          return M_number;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerCLangCommand
  \brief command to specify the player's supported clang version

  <pre>
  Format:
  <- (clang (ver <min> <max>))
  Response:
  -> (ok clang (ver <min> <max>))
  </pre>
*/
class PlayerCLangCommand
    : public PlayerSupportCommand {
private:
    int M_min; //!< minimal supported clang version
    int M_max; //!< maximal supported clang version
public:
    /*!
      \brief construct with version parameters
      \param min_version minimal supported clang version
      \param max_version maximal supported clang version
    */
    PlayerCLangCommand( const int min_version,
                        const int max_version )
        : M_min( min_version )
        , M_max( max_version )
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return CLANG;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "clang" );
      }

    /*!
      \brief get clang command parameter
      \return clang min version
     */
    int minVer() const
      {
          return M_min;
      }

    /*!
      \brief get clang command parameter
      \return clang max version
     */
    int maxVer() const
      {
          return M_max;
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerEarCommand
  \brief command to specify player's ear mode

  <pre>
  Format:
  <- (ear (<onoff> <side> <mode>)) : change only team (our or opp)
  <- (ear (<onoff> <mode>))        : change all players (our & opp)
  <- (ear (<onoff> <side>))        : change team (our or opp) & turn on/off all (complete & partial)
  <- (ear (<onoff>))               : change all players (our & opp) & turn on/off all (complete & partial)

  <onoff> :- on | off
  <side> :- our | opp | left | l | right | r | TEAMNAME
  <mode> :- partial | p | complete | c

  Response:
  -> (error no team with name <teanmane>) : case illegal team name

  if you do NOT want to hear any opponent message,
  you should to send next command
  <- (ear (off opp complete))  : partial mode was set to off by default.
  or,
  <- (ear (off opp))
  </pre>
*/
class PlayerEarCommand
    : public PlayerSupportCommand {
public:
    /*!
      \brief on/off types
    */
    enum OnOffType {
        ON,
        OFF,
    };
    /*!
      \brief side types
    */
    enum SideType {
        OUR,
        OPP,
    };
    /*!
      \brief ear mode types that specifies enabled hear message type
    */
    enum ModeType {
        COMPLETE,
        PARTIAL,
        ALL,
    };

private:
    OnOffType M_onoff; //!< on/off switch
    SideType M_side; //!< our/opp switch
    ModeType M_mode; //!< ear mode type
public:
    /*!
      \brief constcust command for all type message
      \param onoff on/off switch
      \param side side type
    */
    PlayerEarCommand( const OnOffType onoff,
                      const SideType side )
        : M_onoff( onoff )
        , M_side( side )
        , M_mode( ALL )
      { }

    /*!
      \brief constcust command for specified type message
      \param onoff on/off switch
      \param side side type
      \param mode ear mode type
    */
    PlayerEarCommand( const OnOffType onoff,
                      const SideType side,
                      const ModeType mode )
        : M_onoff( onoff )
        , M_side( side )
        , M_mode( mode )
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
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "ear" );
      }


    /*!
      \brief get ear command parameter
      \return on/off type
    */
    OnOffType onOff() const
      {
          return M_onoff;
      }

    /*!
      \brief get ear command parameter
      \return side type
     */
    SideType side() const
      {
          return M_side;
      }

    /*!
      \brief get ear command parameter
      \return mode type
     */
    ModeType mode() const
      {
          return M_mode;
      }

};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerSenseBodyCommand
  \brief command to require sense_body

  <pre>
  Format:
  <- (sense_body)
  Response:
  -> (sense_body ...)   : nomral sense_body message
  </pre>
*/
class PlayerSenseBodyCommand
    : public PlayerSupportCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    PlayerSenseBodyCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return SENSE_BODY;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "sense_body" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerScoreCommand
  \brief command to require current game score

  <pre>
  Format:
  <- (score)
  Response:
  -> v5: (score <TIME> <LEFT_POINT> <RIGHT_POINT>)
  -> v8: (score <TIME> <OUR_POINT> <OPP_POINT>)
  </pre>
*/
class PlayerScoreCommand
    : public PlayerSupportCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    PlayerScoreCommand()
      { }

    /*!
      \brief get command type
      \return command type Id
    */
    Type type() const
      {
          return SCORE;
      }

    /*!
      \brief put command string to ostream
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "score" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerCompressionCommand
  \brief command to set message compression level

  <pre>
  Format:
  <- (compression <level>)
  Response:
  -> (ok compression <LEVEL>)
  -> (warning compression_unsupported)
  </pre>
*/
class PlayerCompressionCommand
    : public PlayerSupportCommand {
private:
    int M_level; //!< gzip compresson level
public:
    /*!
      \brief construct with compression level
      \param level gzip compression level
    */
    explicit
    PlayerCompressionCommand( const int level )
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
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "compression" );
      }

    /*!
      \brief get compression command parameter
      \return compression level
     */
    int level() const
      {
          return M_level;
      }
};


//////////////////////////////////////////////////////////////////////
/*!
  \class PlayerDoneCommand
  \brief command to send done command for (think) message in synch_mode

  <pre>
  Format:
  <- (done)
  </pre>
*/
class PlayerDoneCommand
    : public PlayerSupportCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
    */
    PlayerDoneCommand()
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
      \param to reference to the output stream
      \return reference to the output stream
    */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command paramter
      \return turn neck moment of this command
    */
    std::string name() const
      {
          return std::string( "done" );
      }
};

}

/*

init,
"(init <teamname>)"
"(init <teamname> (version <ver>))"
"(init <teamname> (version <ver>) (goalie))"

bye,
"(bye)"



move,
"(move <x> <y>)"
"(move (<x> <y>))"

dash,
"(dash <power>)"

turn,
"(turn <moment>)"

kick,
"(kick <power> <dir>)"

catch,
"(catch <dir>)"

tackle,
"(tackle <power>)"



turn_neck,
"(turn_neck <moment>)"

change_view,
"(change_view <width> <qual>)"
<width> :- narrow | normal | wide
<qual> :- high | low

say,
"(say <message>)"
"(say "<message>")" <--- in server 8+, to use the double quatation is recommended

pointto,
"(pointto <dist> <dir>)"
"(pointto off)"

attentionto,
"(attentionto <side> <number>)"
"(attentionto off)"
side :- our | opp | l | left | r | right | TEAMNAME
<--- server check the string in this order

clang,
"(clang (ver <min> <max>))"

ear,
"(ear <flag> <side> <mode>)"
"(ear <flag> <mode>)"
"(ear <flag> <side>)"
"(ear <flag>)"
<flag> :- on | off
<side> :- our | opp | left | l | right | r | TEAMNAME
<mode> :- partial | p | complete | c


sense_body,
"(sense_body)"

score,
"(score)"

compression,
"(compression <level>)"
<level> :- [0-9]

done,
"(done)"



*/

#endif
