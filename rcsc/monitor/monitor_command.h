// -*-c++-*-

/*!
  \file monitor_command.h
  \brief monitor command classes Header File
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

#ifndef RCSC_MONITOR_COMMAND_H
#define RCSC_MONITOR_COMMAND_H

#include <rcsc/types.h>

#include <iostream>
#include <string>

namespace rcsc {

/*!
  \class MonitorCommand
  \brief abstract monitor command class
 */
class MonitorCommand {
public:
    /*!
      \brief monitor client command Id
     */
    enum Type {
        INIT,
        BYE,

        START, // kickoff
        FOUL, // drop_ball & free_kick_[lr]

        PLAYER, // move player
        DISCARD, // discard player

        CARD, // yellow/red card

        COMPRESSION,

        ILLEGAL
    };

protected:
    /*!
      \brief protected default constructor because this is abstract class.
     */
    MonitorCommand()
      { }

public:
    /*!
      \brief virtual destructor. nothing to do
     */
    virtual
    ~MonitorCommand()
      { }

    /*!
      \brief pure virtual method. get command type id.
      \return monitor command type.
     */
    virtual
    Type type() const = 0;

    /*!
      \brief pure virtual method. make command string.
      \param to reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & toCommandString( std::ostream & to ) const = 0;

    /*!
      \brief pure virtual method. get command name string
      \return command name string
     */
    virtual
    std::string name() const = 0;
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorInitCommand
  \brief initial connection command
 */
class MonitorInitCommand
    : public MonitorCommand {
private:
    //! monitor protocol version
    int M_version;

public:
    /*!
      \brief construct with protocol version number
      \param version protocol version. (default:0)
     */
    explicit
    MonitorInitCommand( const int version = 0 );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return INIT;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "init" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorByeCommand
  \brief disconnection command
 */
class MonitorByeCommand
    : public MonitorCommand {
private:

public:
    /*!
      \brief default constructor does nothing
     */
    MonitorByeCommand()
      { }

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return BYE;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispbye" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorKickOffCommand
  \brief game start command
 */
class MonitorKickOffCommand
    : public MonitorCommand {
private:

public:
    /*!
      \brief default constructor does nothing
     */
    MonitorKickOffCommand()
      { }

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return START;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispstart" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorDropBallCommand
  \brief drop ball at the specified point on the pitch.

  This command is one of FOUL command
 */
class MonitorDropBallCommand
    : public MonitorCommand {
private:
    double M_x; //!< drop point x
    double M_y; //!< drop point y

    MonitorDropBallCommand() = delete; // not used
public:
    /*!
      \brief construct with real coordinate
      \param x x-coordinate on the field
      \param y y-coordinate on the field
     */
    MonitorDropBallCommand( const double & x,
                            const double & y );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return FOUL;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream

      command argument coordinates are rounded by rcg's SHOWINFO_SCALE
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispfoul" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorFreeKickCommand
  \brief freekick awarded at the specified point on the pitch.

  This command is one of FOUL command
 */
class MonitorFreeKickCommand
    : public MonitorCommand {
private:
    double M_x; //!< free kick point y
    double M_y; //!< free kick point y
    SideID M_side; //!< free kick awarded side. NEUTRAL means drop ball.

    MonitorFreeKickCommand() = delete; // not used
public:
    /*!
      \brief construct with real coordinate
      \param x x-coordinate on the field
      \param y y-coordinate on the field
      \param side free kick awarded side
     */
    MonitorFreeKickCommand( const double & x,
                            const double & y,
                            const SideID side );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return FOUL;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream

      command argument coordinates are rounded by rcg's SHOWINFO_SCALE
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispfoul" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorMovePlayerCommand
  \brief command to move player on the pitch
 */
class MonitorMovePlayerCommand
    : public MonitorCommand {
private:
    SideID M_side; //!< moved player's side
    int M_unum; //!< moved player's uniform number
    double M_x; //!< move target point x
    double M_y; //!< move target point y
    double M_angle; //!< player's body angle after move. *degree*

    MonitorMovePlayerCommand() = delete; // not used
public:
    /*!
      \brief construct with all needed info
      \param side moved player's side
      \param unum moved player's uniform number
      \param x move target point x
      \param y move target point y
      \param angle player's body angle after move.
     */
    MonitorMovePlayerCommand( const SideID side,
                              const int unum,
                              const double & x,
                              const double & y,
                              const double & angle );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return PLAYER;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispplayer" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorDiscardPlayerCommand
  \brief command to disscard a player from the server
 */
class MonitorDiscardPlayerCommand
    : public MonitorCommand {
private:
    SideID M_side; //!< target player's side id
    int M_unum; //!< target player's uniform number

    MonitorDiscardPlayerCommand() = delete; // not used
public:
    /*!
      \brief construct with all info
      \param side target player's side id
      \param unum target player's uniform number
     */
    MonitorDiscardPlayerCommand( const SideID side,
                                 const int unum );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return DISCARD;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispdiscard" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorCardCommand
  \brief command to give yellow/red card to a player
 */
class MonitorCardCommand
    : public MonitorCommand {
private:
    SideID M_side; //!< target player's side id
    int M_unum; //!< target player's uniform number
    Card M_card; //!< card type

    MonitorCardCommand() = delete; // not used
public:
    /*!
      \brief construct with all info
      \param side target player's side id
      \param unum target player's uniform number
     */
    MonitorCardCommand( const SideID side,
                        const int unum,
                        const Card card );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return CARD;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "dispcard" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class MonitorCompressionCommand
  \brief set compression level for sserver message
 */
class MonitorCompressionCommand
    : public MonitorCommand {
private:
    //! gzip compression level
    int M_level;

    MonitorCompressionCommand() = delete; // not used
public:
    /*!
      \brief construct with compression level
      \param level gzip compression level [0-9]
     */
    explicit
    MonitorCompressionCommand( const int level );

    /*!
      \brief get command type id
      \return command type
     */
    Type type() const
      {
          return COMPRESSION;
      }

    /*!
      \brief put command string to the output stream
      \param to referenct to the toutput stream
      \return referenct to the toutput stream
     */
    std::ostream & toCommandString( std::ostream & to ) const;

    /*!
      \brief get command name string
      \return command name string
     */
    std::string name() const
      {
          return std::string( "compression" );
      }
};

}

#endif
