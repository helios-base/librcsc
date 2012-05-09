// -*-c++-*-

/*!
  \file coach_audio_sensor.cpp
  \brief audio message analyzer for coach Source File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

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

#include "coach_audio_sensor.h"

#include <rcsc/common/say_message_parser.h>
#include <rcsc/common/logger.h>

#include <cstdio>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
CoachAudioSensor::CoachAudioSensor()
    : M_teammate_message_time( -1, 0 ),
      M_opponent_message_time( -1, 0 ),
      M_trainer_message_time( -1, 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachAudioSensor::setTeamName( const std::string & team_name )
{
    M_team_name = "\"";
    M_team_name += team_name;
    M_team_name += "\"";
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachAudioSensor::addParser( SayMessageParser::Ptr parser )
{
    if ( ! parser )
    {
        std::cerr << __FILE__ << ":" << __LINE__ << ' '
                  << " ***ERROR*** CoachAudioSensor::addParser()"
                  << " NULL parser object."
                  << std::endl;
        return;
    }

    if ( M_say_message_parsers.find( parser->header() )
         != M_say_message_parsers.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__ <<  ' '
                  << " ***ERROR*** CoachAudioSensor::addParser()"
                  << " parser for [" << parser->header()
                  << "] is already registered."
                  << std::endl;
        return;
    }

    M_say_message_parsers.insert( ParserMap::value_type( parser->header(),
                                                         parser ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachAudioSensor::removeParser( const char header )
{
    ParserMap::iterator it = M_say_message_parsers.find( header );
    if ( it == M_say_message_parsers.end() )
    {
        std::cerr << " ***WARNING*** CoachAudioSensor::removeParser()"
                  << " header [" << header
                  << "] is not registered."
                  << std::endl;
        return;
    }

    M_say_message_parsers.erase( it );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachAudioSensor::parsePlayerMessage( const char * msg,
                                      const GameTime & current )
{
    // v7-: (hear <time> (player "<teamname>" <unum>[ goalie]) "<message>")
    // v7+: (hear <time> (p "<teamname>" <unum>[ goalie]) "<message>")


    if ( M_team_name.empty() )
    {
        // no team name
        return;
    }

    long cycle = 0;
    char player[8];
    char teamname[32];
    int unum = 0;
    int n_read = 0;

    // complete message
    if ( std::sscanf( msg, " ( hear %ld ( %7s %31s %d %n ",
                      &cycle, player, teamname, &unum, &n_read ) != 4
         || player[0] != 'p'
         || n_read == 0 )
    {
        std::cerr << current
                  << " ***ERROR*** CoachAudioSensor::parsePlayerMessage()"
                  << " heard unsupported message. [" << msg << "]"
                  << std::endl;
        return;
    }
    msg += n_read;

    if ( unum < 1 || 11 < unum )
    {
        // unsupported uniform number range
        std::cerr << current
                  << " ***ERROR*** CoachAudioSensor::parsePlayerMessage."
                  << " Illegal player number = " << unum
                  << std::endl;
        return;
    }

    while ( *msg != ' ' ) ++msg; // skip "goalie)"
    while ( *msg == ' ' ) ++msg;

    char end_char = ')';
    if ( *msg == '\"' )
    {
        end_char = '\"';
        ++msg;
    }

    HearMessage message;
    message.unum_ = unum;
    message.dir_ = 0.0;
    message.str_ = msg;

    std::string::size_type pos = message.str_.rfind( end_char );
    if ( pos == std::string::npos )
    {
        std::cerr << current
                  << " ***ERROR*** CoachAudioSensor::parsePlayerMessage."
                  << " Illegal quated message. [" << msg << ']'
                  << std::endl;
        return;
    }
    message.str_.erase( pos );

    if ( message.str_.empty() )
    {
        // empty message;
        return;
    }

    if ( M_team_name.compare( teamname ) == 0 )
    {
        if ( M_teammate_message_time != current )
        {
            M_teammate_message_time = current;
            M_teammate_messages.clear();
            dlog.addText( Logger::SENSOR,
                          "CoachAudioSensor: new cycle teammate audio [%ld,%ld]",
                          current.cycle(), current.stopped() );
        }

        M_teammate_messages.push_back( message );

        //     std::cerr << "coach " << current
        //               << " hear sender=" << unum
        //               << " msg=[" << message.str_
        //               << "]" << std::endl;

        parseTeammateMessage( message );
    }
    else
    {
        // opponent audio
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachAudioSensor::parseTrainerMessage( const char * msg,
                                       const GameTime & current )
{

    // (hear <time> referee <msg>) : v7-
    // (hear <time> coach "<msg>") : v7+
    // (hear <time> coach <clang>) : v7+

    int n_read = 0;
    long cycle;
    char sender[32];

    if ( std::sscanf( msg, " ( hear %ld %31s %n ",
                      &cycle, sender, &n_read ) != 2 )
    {
        std::cerr << current
                  << " ***ERRORR*** failed to parse trainer message. ["
                  << msg << ']'
                  << std::endl;
        return;
    }
    msg += n_read;

    while ( *msg != '\0' && *msg == ' ' ) ++msg;

    char end_char = ')';
    if ( *msg == '\"' )
    {
        end_char = '\"';
        ++msg;
    }

    M_trainer_message_time = current;
    M_trainer_message.erase();
    M_trainer_message.assign( msg );

    // remove quotation or parenthesis
    std::string::size_type pos = M_trainer_message.rfind( end_char );
    if ( pos == std::string::npos )
    {
        std::cerr << current
                  << " ***ERROR*** CoachAudioSensor::parsePlayerMessage."
                  << " Illegal quated message. [" << msg << ']'
                  << std::endl;
        return;
    }

    M_trainer_message.erase( pos );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
CoachAudioSensor::parseTeammateMessage( const HearMessage & message )
{
    if ( message.str_.empty() )
    {
        return;
    }

    const ParserMap::iterator end = M_say_message_parsers.end();

    const char * msg = message.str_.c_str();

    int n_parsed = 0;
    while ( *msg != '\0' )
    {
        int len = 0;

        ParserMap::iterator it = M_say_message_parsers.find( *msg );

        if ( it == end )
        {
            dlog.addText( Logger::SENSOR,
                          "CoachAudioSensor: unsupported message [%s] in [%s]",
                          msg, message.str_.c_str() );
            return;
        }

        len = it->second->parse( message.unum_, message.dir_, msg,
                                 M_teammate_message_time );

        if ( len < 0 )
        {
            return;
        }

        ++n_parsed;
        msg += len;
    }

}

}
