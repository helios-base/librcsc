// -*-c++-*-

/*!
  \file coach_command.h
  \brief coach command classes Header File
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

#ifndef RCSC_COACH_COMMAND_H
#define RCSC_COACH_COMMAND_H

#include <rcsc/types.h>

#include <string>
#include <vector>
#include <utility>
#include <iostream>

namespace rcsc {

/*!
  \class CoachCommand
  \brief abstract coach command class
 */
class CoachCommand {
public:
    /*!
      \brief online coach command type Id
    */
    enum Type {
        INIT,
        BYE,

        CHECK_BALL,
        LOOK,
        TEAM_NAMES,

        EYE,

        CHANGE_PLAYER_TYPE,
        CHANGE_PLAYER_TYPES,

        CLANG_FREEFORM,

        TEAM_GRAPHIC,
        COMPRESSION,
        DONE,

        ILLEGAL
    };

    /*
    enum CLangType {
    CLANG_META,
    CLANG_FREEFORM,
    CLANG_INFO,
    CLANG_ADVICE,
    CLANG_DEFINE,
        CLANG_ADVISE,
        CLANG_ILLEGAL
    };
    */

protected:
    /*!
      \brief constructor is protected because this is abstract class.
     */
    CoachCommand()
      { }

public:
    /*!
      \brief virtual destructor, but nothing to do
     */
    virtual
    ~CoachCommand()
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
  \class CoachInitCommand
  \brief initial connection command for coach

  <pre>
  Format:
  <- (init <teamname> (version <ver>))
  Response:
  -> (init l ok) | (init r ok)  : version 6.0 or later
  -> (init ok)
  -> (error no_such_team_or_already_have_coach)
  </pre>
*/
class CoachInitCommand
    : public CoachCommand {
private:
    std::string M_team_name; //!< coach's team name
    double M_version; //!< coach client version
    std::string M_coach_name; //!< the name of coach
public:
    /*!
      \brief construct with init parameters
      \param team_name coach's team name string
      \param version coach's client version
      \param coach_name the name string of coach
     */
    CoachInitCommand( const std::string & team_name,
                      const double & version,
                      const std::string & coach_name = "" );

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
  \class CoachByeCommand
  \brief command to disconnect

  <pre>
  Format:
  <- (bye)
  </pre>
*/
class CoachByeCommand
    : public CoachCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
     */
    CoachByeCommand()
      { }

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
/*!
  \class CoachCheckBallCommand
  \brief command to require ball status

  <pre>
  Format:
  <- (check_ball)
  Response:
  -> (ok check_ball <ball_info>)

  ball_info :- "in_field" | "goal_l" | "goal_r" | "out_of_field"
  </pre>
*/
class CoachCheckBallCommand
    : public CoachCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
     */
    CoachCheckBallCommand()
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
          return std::string( "check_ball" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class CoachLookCommand
  \brief commad to require field status

  <pre>
  Format:
  <- (look)
  Response:
  (ok look TIME GOAL_INFO BALL_INFO PLAYER_INFO ...)
  </pre>
*/
class CoachLookCommand
    : public CoachCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
     */
    CoachLookCommand()
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
          return std::string( "look" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class CoachTeamNamesCommand
  \brief command to require team names

  <pre>
  Format:
  <- (team_names)
  Response;
  -> (ok team_names (team l TEAM_NAME) (team r TEAM_NAME))
  </pre>
*/
class CoachTeamNamesCommand
    : public CoachCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
     */
    CoachTeamNamesCommand()
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
          return std::string( "team_names" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class CoachEyeCommand
  \brief command to enable/disable coach's eye

  <pre>
  Format:
  <- (eye on) | (eye off)
  Response:
  -> (ok eye on) | (ok eye off)
  </pre>
*/
class CoachEyeCommand
    : public CoachCommand {
private:
    bool M_on; //!< if true, coach's eye is enabled
public:
    /*!
      \brief construct with eye mode switch
      \param on if true, coach's eye is enabled.
     */
    explicit
    CoachEyeCommand( bool on )
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
          return std::string( "eye" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class CoachChangePlayerTypeCommand
  \brief command to change player type

  <pre>
  Format:
  <- (change_player_type <unum> <typeid>)
  Response:
  -> (ok change_player_type <unum> <typeid>)
  -> (warning cannot_sub_while_playon)
  -> (warning no_team_found)
  -> (warning no_subs_left)
  -> (error out_of_range_player_type)
  -> (warning no_such_player)
  -> (warning cannot_change_goalie)
  -> (warning max_of_that_type_on_field)
  </pre>
*/
class CoachChangePlayerTypeCommand
    : public CoachCommand {
private:
    int M_unum; //!< target player's uniform number
    int M_type; //!< changed player type id
public:
    /*!
      \brief construct with target number and type id
      \param unum target player's uniform number
      \param type player type id
     */
    CoachChangePlayerTypeCommand( const int unum,
                                  const int type )
        : M_unum( unum )
        , M_type( type )
      { }

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
          return std::string( "change_player_type" );
      }
};


//////////////////////////////////////////////////////////////////////
/*!
  \class CoachChangePlayerTypesCommand
  \brief command to change player type

  <pre>
  Format:
  <- (change_player_types [(<unum> <typeid>)]+)
  Response:
  -> (ok change_player_type <unum> <typeid>)
  -> (warning cannot_sub_while_playon)
  -> (warning no_team_found)
  -> (warning no_subs_left)
  -> (error out_of_range_player_type)
  -> (warning no_such_player)
  -> (warning cannot_change_goalie)
  -> (warning max_of_that_type_on_field)
  </pre>
*/
class CoachChangePlayerTypesCommand
    : public CoachCommand {
private:
    //! the set of player number & type id
    std::vector< std::pair< int, int > > M_types;

public:
    /*!
      \brief construct with one target player and type id
      \param unum target player's uniform number
      \param type player type id
     */
    CoachChangePlayerTypesCommand( const int unum,
                                   const int type );

    /*!
      \brief construct with one target player and type id
      \param types container of the pair of unum and player type id
     */
    CoachChangePlayerTypesCommand( const std::vector< std::pair< int, int > >  & types );

    /*!
      \brief add new change_player_type pair
      \param unum target player's uniform number
      \param type player type id
     */
    void add( const int unum,
              const int type );

    /*!
      \brief get command type
      \return command type Id
     */
    Type type() const
      {
          return CHANGE_PLAYER_TYPES;
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
          return std::string( "change_player_types" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class CoachFreeformCommand
  \brief freeform message command

  Old version:
  <pre>
  Format:
  <- (say <msg>)
  Response:
  -> (ok say)
  -> (error could_not_parse_say)
  -> (error said_too_many_****_messages)
  -> (warning cannot_say_while_playon) [only v7-]
  </pre>

  New version:
  <pre>
  Format:
  <- (say (freeform "<msg>"))
  </pre>
*/
class CoachFreeformCommand
    : public CoachCommand {
private:
    //! client version
    const double M_version;
    //! clang message string. ***const reference***
    const std::string & M_message;

    // not used
    CoachFreeformCommand() = delete;
    CoachFreeformCommand( const CoachCommand & ) = delete;
    CoachFreeformCommand & operator=( const CoachCommand & ) = delete;
public:
    /*!
      \brief construct with message string.
      \param clang_msg reference to the clang message instance
     */
    CoachFreeformCommand( const double & version,
                          const std::string & message )
        : CoachCommand(),
          M_version( version ),
          M_message( message )
      { }

    /*!
      \brief get command type
      \return command type Id
     */
    Type type() const
      {
          return CLANG_FREEFORM;
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
          return std::string( "freeform" );
      }
};

//////////////////////////////////////////////////////////////////////
/*!
  \class CoachTeamGraphicCommand
  \brief command to send a tile of xpm team graphic

  <pre>
  Format:
  <- (team_graphic (<x> <y> "<xpmline>" ... "<xpmline>"))
  Response:
  -> (ok team_graphic <x> <y>)
  -> (warning only_before_kick_off)
  -> (warning invalid_tile_location)
  -> (warning invalid_tile_size)
  -> (error illegal_command_form)
  </pre>
*/
class CoachTeamGraphicCommand
    : public CoachCommand {
private:
    unsigned int M_x; //!< x coordinate of this tile in xpm image
    unsigned int M_y; //!< y coordinate of this tile in xpm image
    std::vector< std::string > M_xpm_lines; //!< a tile of xpm

public:
    /*!
      \brief construct with xpm string and its coordinate.
      \param x x coordinate of this tile in xpm image
      \param y y coordinate of this tile in xpm image
      \param xpm_lines a tile of xpm
     */
    CoachTeamGraphicCommand( const unsigned int x,
                             const unsigned int y,
                             const std::vector< std::string > & xpm_lines );

    /*!
      \brief get command type
      \return command type Id
     */
    Type type() const
      {
          return TEAM_GRAPHIC;
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
          return std::string( "team_graphic" );
      }
};


//////////////////////////////////////////////////////////////////////
/*!
  \class CoachCompressionCommand
  \brief command to set message compression level

  <pre>
  Format:
  <- (compression <level>)
  Response:
  -> (ok compression <level>)
  -> (warning compression_unsupported)
  </pre>
*/
class CoachCompressionCommand
    : public CoachCommand {
private:
    int M_level; //!< gzip compression level
public:
    /*!
      \brief construct with compression level
     */
    explicit
    CoachCompressionCommand( const int level )
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
  \class CoachDoneCommand
  \brief command to send done command for (think) message in synch_mode

  <pre>
  Format:
  <- (done)
  </pre>
*/
class CoachDoneCommand
    : public CoachCommand {
private:

public:
    /*!
      \brief constructor. nothing to do
     */
    CoachDoneCommand()
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
      \brief get command name
      \return command name string
     */
    std::string name() const
      {
          return std::string( "done" );
      }
};

}

/*

Online Coach Initialization Command
  See rcssserver/src/netif.C : void Stadium::parseOnlineCoachInit
      rcssserver/src/field.C : OnlineCoach* Stadium::newCoach

if illegal command
  -> "(error illegal_command_form)"
----------
  Coach's default version is 5.0

"(init TEAM_NAME (version VERSION))"
  if TEAM_NAME and VERSION is illegal
    -> "(error no_such_team_or_already_have_coach)"
  else
    if VERSION >= 6.0
      -> "(init l ok)" or "(init r ok)"
    else
      -> "(init ok)"

  receive parameter info
  receive changed players info
  receive each player's clang version

    if VERSION >= 7.0
      -> "(server_param ...)" "(player_param ...)" "(player_type ...)"x7
      -> "(change_player_type UNUM ID)" "(change_player_type UNUM)" "(change_player_type TEAM_NAME UNUM ID)"
    if VERSION >= 8.0
      -> "(clang (ver PLAYER_SHORT_NAME MIN MAX))"
----------

----------

----------
Online Coach Command List
  See rcssserver/src/netif.C : void Coach::parse_command(const char *command)l

if use illegal character for command string
  -> "(error illegal_command_form)"
else if illegal command
  -> "(error unknown_command)"

----------
"(check_ball)" : check ball positional state
  -> (ok check_ball BALL_POS_INFO)
  BALL_POS_INFO :- "in_field" | "goal_l" | "goal_r" | "out_of_field"
----------
"(look)" : get all movable objects' positional information
  -> "(ok look TIME GOAL_INFO BALL_INFO PLAYER_INFO ...)"
----------
"(team_names)" : get team name
  -> "(ok team_names (team l TEAM_NAME) (team r TEAM_NAME))"
----------
"(say MESSAGE)" : say advice message (if version > 7.0, it must be CLang.)
  if version >= 7.0
  {
    if MESSAGE cannot be parsed
      -> "(error could_not_parse_say)"
    else
    {
      if MESSAGE is MetaType
        if MetaType has no left
          -> "(error said_too_many_meta_messages)"
        else
          -> "(ok say)"
      else if MESSAGE is FreeFormType
        if FreeFormType is enable now
          if FreeFormType has no left
            -> "(error said_too_many_freeform_messages)"
          else
            -> "(ok say)"
        else
          -> "(error cannot_say_freeform_while_playon)"
      else if MESSAGE is InfoType
        if InfoType has no left
          -> "(error said_too_many_info_messages)"
        else
          -> "(ok say)"
      else if MESSAGE is AdviceType
        if AdviceType has no left
          -> "(error said_too_many_advice_messages)"
        else
          -> "(ok say)"
      else if MESSAGE is DefineType
        if DefineType has no left
          -> "(error said_too_many_define_messages)"
        else
          -> "(ok say)"
      else if MESSAGE is DeleteType
        if DeleteType has no left
          -> "(error said_too_many_del_messages)"
        else
          -> "(ok say)"
      else if MESSAGE is RuleType
        if RuleType has no left
          -> "(error said_too_many_rule_messages)"
        else
          -> "(ok say)"
    }
  }
  else // version < 7.0, CLang is not supported
  {
    if playmode is PlayOn
      -> "(warning cannot_say_while_playon)"
    else if coach's say has no left
      -> "(error said_too_many_messages)"
    else
      if MESSAGE uses illegal charcter or is NULL string
        -> "(error illegal_command_form)"
      else
        -> "(ok say)"
  }
----------
"(bye)" : close connection
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
"(change_player_type UNUM PLAYER_TYPE_ID)" : change hetero player type
  if playmode is PlayOn
    -> "(warning cannot_sub_while_playon)"
  else if coach's side is Unknown
    -> "(warning no_team_found)"
  else if team_subs_count == subsMax
    -> "(warning no_subs_left)"
  else if UNUM or PLAYER_TYPE_ID has illegal form (eg. NULL string)
    -> "(error illegal_command_form)"
  else if PLAYER_TYPE_ID is out of range (ID<0 or PlayerTypes<ID)
    -> "(error out_of_range_player_type)"
  else if UNUM is not match
    -> "(warning no_such_player)"
  else if player is goalie && PLAYER_TYPE_ID != 0
    -> "(warning cannot_change_goalie)"
  else if ID != 0 && ID's player count == ptMax() && player's id != ID
    -> "(warning max_of_that_type_on_field)"
  else
    -> "(ok change_player_type UNUM PLAYER_TYPE_ID)\n
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
"(team_graphic (X Y "XPMLINE" ... "XPMLINE"))" : change team graphic on rcssmonitor
  if playmode is not BeforeKickOff
    -> "warning only_before_kick_off"
  else if X or Y is illegal (eg. NULL string)
    -> "(error illegal_command_form)"
  else if (X >= 32 || Y = 8)
    -> "(warning invalid_tile_location)"
  else if (xpm's width != 8 || xpm's height != 8)
    -> "(warning invalid_tile_size)"
  else
    -> "(ok team_graphic X Y)"
----------
*/


#endif
