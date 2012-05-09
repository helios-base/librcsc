// -*-c++-*-

/*!
  \file audio_sensor.cpp
  \brief audio message analyzer Source File
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

#include "audio_sensor.h"

#include <rcsc/common/say_message_parser.h>
#include <rcsc/common/logger.h>
#include <rcsc/math_util.h>

#include <cstdio>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
AudioSensor::AudioSensor()
    : M_teammate_message_time( -1, 0 ),
      M_opponent_message_time( -1, 0 ),
      M_freeform_message_time( -1, 0 ),
      M_trainer_message_time( -1, 0 ),
      M_clang_time( -1, 0 )
{
    M_freeform_message.reserve( 256 );
    M_clang_message.reserve( 8192 );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::addSayMessageParser( SayMessageParser::Ptr parser )
{
    if ( ! parser )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** (addParser) NULL parser object."
                  << std::endl;
        return;
    }

    if ( M_say_message_parsers.find( parser->header() ) != M_say_message_parsers.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** AudioSensor::addSayMessageParser()"
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
AudioSensor::removeSayMessageParser( const char header )
{
    ParserMap::iterator it = M_say_message_parsers.find( header );
    if ( it == M_say_message_parsers.end() )
    {
        std::cerr << "***WARNING*** (removeParser)"
                  << " header=[" << header << "] has not been registered."
                  << std::endl;
        return;
    }

    M_say_message_parsers.erase( it );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::addFreeformMessageParser( FreeformMessageParser::Ptr parser )
{
    if ( ! parser )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** (addFreeformParser) NULL parser object."
                  << std::endl;
        return;
    }

    if ( M_freeform_parsers.find( parser->type() ) != M_freeform_parsers.end() )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** AudioSensor::addSayMessageParser()"
                  << " parser for [" << parser->type()
                  << "] is already registered."
                  << std::endl;
        return;
    }

    M_freeform_parsers.insert( FreeformParserMap::value_type( parser->type(), parser ) );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::removeFreeformMessageParser( const std::string & type )
{
    FreeformParserMap::iterator it = M_freeform_parsers.find( type );
    if ( it == M_freeform_parsers.end() )
    {
        std::cerr << "***WARNING*** (removeFreeformParser)"
                  << " type=[" << type << "] has not been registered."
                  << std::endl;
        return;
    }

    M_freeform_parsers.erase( it );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::parsePlayerMessage( const char * msg,
                                 const GameTime & current )
{
    /*
      players' communication audio format
      // from other
      v7-: (hear <TIME> <DIR> <MSG>)
      v7:  (hear <TIME> <DIR> "<MSG>")
      v8+: (hear <TIME> <DIR> our <UNUM> "<MSG>")
      (hear <TIME> <DIR> opp "<MSG>")
      (hear <TIME> our)
      (hear <TIME> opp)
    */

    long cycle = 0;
    double dir = 0.0;
    int unum = 0;
    char sender[8];
    int n_read = 0;

    // v8+ complete message
    if ( std::sscanf( msg, " (hear %ld %lf %7[^ ] %d %n ",
                      &cycle, &dir, sender, &unum, &n_read ) != 4 )
    {
        std::cerr << "***ERROR*** AudioSensor::parsePlayerMessage()"
                  << " heard unsupported message. [" << msg << "]"
                  << std::endl;
        return;
    }
    msg += n_read;

    while ( *msg == ' ' ) ++msg;

    char end_char = ')';
    if ( *msg == '\"' )
    {
        end_char = '\"';
        ++msg;
    }

    HearMessage message;
    message.unum_ = unum;
    message.dir_ = dir;
    message.str_ = msg;

    std::string::size_type pos = message.str_.rfind( end_char );
    if ( pos == std::string::npos )
    {
        std::cerr << "***ERROR*** AudioSensor::parsePlayerMessage."
                  << " Illegal message. [" << msg << ']'
                  << std::endl;
        return;
    }
    message.str_.erase( pos );

    if ( message.str_.empty() )
    {
        // empty message
        return;
    }

    if ( ! std::strncmp( sender, "our", 3 ) )
    {
        if ( M_teammate_message_time != current )
        {
            dlog.addText( Logger::SENSOR,
                          __FILE__" (parsePlayerMessage) clear old data" );
            M_teammate_message_time = current;
            M_teammate_messages.clear();
        }

        M_teammate_messages.push_back( message );

        parseTeammateMessage( message );
    }
    else if ( ! std::strncmp( sender, "opp", 3 ) )
    {
        if ( M_opponent_message_time != current )
        {
            M_opponent_message_time = current;
            M_opponent_messages.clear();
        }

        M_opponent_messages.push_back( message );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::parseCoachMessage( const char * msg,
                                const GameTime & current )
{
    // (hear <time> online_coach_{left,right} <msg>) : v7-
    // (hear <time> online_coach_{left,right} (freeform "<msg>")) : v7+
    // (hear <time> online_coach_{left,right} <clang>)   : v7+

    // skip "(hear "
    while ( *msg != ' ' && *msg != '\0' ) ++msg;
    while ( *msg == ' ' ) ++msg;

    // skip "<time> "
    while ( *msg != ' ' && *msg != '\0' ) ++msg;
    while ( *msg == ' ' ) ++msg;

    // skip sender
    while ( *msg != ' ' && *msg != '\0' ) ++msg;
    while ( *msg == ' ' ) ++msg;

    if ( *msg != '(' )
    {
        //
        // old type freeform message
        //
        buildFreeformMessage( msg );
        if ( parseFreeformMessage() )
        {
            M_freeform_message_time = current;
        }
        return;
    }

    //
    // clang message
    //

    char message_type[32];
    int n_read = 0;

    if ( std::sscanf( msg, " ( %31[^ ] %n ",
                      message_type, &n_read ) != 1 )
    {
        std::cerr << "***ERROR*** failed to parse clang message type. ["
                  << msg << ']' << std::endl;
        return;
    }

    if ( ! std::strcmp( message_type, "freeform" ) )
    {
        msg += n_read; // skip "(freeform "

        buildFreeformMessage( msg );
        if ( parseFreeformMessage() )
        {
            M_freeform_message_time = current;
        }
    }
    else
    {
        buildCLangMessage( msg ); // remove last parenthesis
        if ( parseCLangMessage() )
        {
            // std::cerr << current << " OK parsed clang ["
            //           << *M_clang_parser.message() << ']' << std::endl;
            M_clang_time = current;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::parseTrainerMessage( const char * msg,
                                  const GameTime & current )
{

    // (hear <time> referee <msg>) : v7-
    // (hear <time> coach "<msg>") : v7+
    // (hear <time> coach <clang>) : v7+

    int n_read = 0;
    long cycle;
    char sender[32];

    if ( std::sscanf( msg, "(hear %ld %31s %n ",
                      &cycle, sender, &n_read ) != 2 )
    {
        std::cerr << "***ERRORR*** failed to parse trainer message. ["
                  << msg << ']'
                  << std::endl;
        return;
    }
    msg += n_read;

    while ( *msg == ' ' ) ++msg;

    if ( *msg == '\"'
         || *msg != '(' )
    {
        const char end_char = ( *msg == '\"' ? '\"' : ')' );

        if ( *msg == '\"' )
        {
            ++msg;
        }

        M_trainer_message.erase();
        M_trainer_message = msg;

        // remove quotation or parenthesis
        std::string::size_type pos = M_trainer_message.rfind( end_char );
        if ( pos == std::string::npos )
        {
            std::cerr << "***ERROR*** (AudioSensor::parsePlayerMessage)"
                      << " Illegal quated message. [" << msg << ']'
                      << std::endl;
            return;
        }

        M_trainer_message.erase( pos );

        //
        // TODO: parse message
        //

        M_trainer_message_time = current;
    }
    else
    {
        buildCLangMessage( msg ); // remove last parenthesis
        if ( parseCLangMessage() )
        {
            M_clang_time = current;
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::parseTeammateMessage( const HearMessage & message )
{
    if ( message.str_.empty() )
    {
        return;
    }

    const ParserMap::iterator end = M_say_message_parsers.end();

    const char * msg = message.str_.c_str();

    while ( *msg != '\0' )
    {
        const char tag = *msg;

        ParserMap::iterator it = M_say_message_parsers.find( tag );

        if ( it == end )
        {
            dlog.addText( Logger::SENSOR,
                          __FILE__" (parseTeammateMessage) unsupported message [%s] in [%s]",
                          msg, message.str_.c_str() );
            return;
        }

        int len = it->second->parse( message.unum_, message.dir_, msg,
                                     M_teammate_message_time );

        if ( len < 0 )
        {
            return;
        }

        msg += len;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::buildFreeformMessage( const char * msg )
{
    while ( *msg == ' ' ) ++msg;

    bool quated = false;
    if ( *msg == '\"' )
    {
        quated = true;
        ++msg;
    }

    M_freeform_message = msg;

    // remove quotation or parenthesis
    if ( quated )
    {
        std::string::size_type pos = M_freeform_message.find_last_of( '\"' );
        if ( pos == std::string::npos )
        {
            std::cerr << "***ERROR*** (AudioSensor::buildFreeformMessage)"
                      << " no last double quotation [" << msg << ']'
                      << std::endl;
            M_freeform_message.clear();
            return;
        }
        M_freeform_message.erase( pos );
    }
    else
    {
        std::string::size_type pos = M_freeform_message.find_last_not_of( ')' );
        if ( pos == std::string::npos
             || pos == M_freeform_message.length() - 1 )
        {
            std::cerr << "***ERROR*** (AudioSensor::buildFreeformMessage)"
                      << " no last parenthesis [" << msg << ']'
                      << std::endl;
            M_freeform_message.clear();
            return;
        }
        M_freeform_message.erase( pos + 1 );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AudioSensor::parseFreeformMessage()
{
    if ( M_freeform_message.empty() )
    {
        return false;
    }

    const FreeformParserMap::iterator end = M_freeform_parsers.end();

    const char * msg = M_freeform_message.c_str();

    char tag[16];

    while ( *msg != '\0' )
    {
        int n_read = 0;
        if ( std::sscanf( msg, " ( %15s %n ",  tag, &n_read ) != 1 )
        {
            dlog.addText( Logger::SENSOR,
                          __FILE__" (parseFreeformMessage) illegal message [%s] in [%s]",
                          msg, M_freeform_message.c_str() );
            return false;
        }

        FreeformParserMap::iterator it = M_freeform_parsers.find( tag );

        if ( it == end )
        {
            dlog.addText( Logger::SENSOR,
                          __FILE__" (parseFreeformMessage) unsupported message [%s] in [%s]",
                          tag, M_freeform_message.c_str() );
            return false;
        }

        int len = it->second->parse( msg );

        if ( len < 0 )
        {
            dlog.addText( Logger::SENSOR,
                          __FILE__" (parseFreeformMessage) failed. tag=[%s] msg=[%s]",
                          tag, M_freeform_message.c_str() );
            return false;
        }

        msg += len;

        while ( *msg == ' ' ) ++msg;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
AudioSensor::buildCLangMessage( const char * msg )
{
    while ( *msg == ' ' ) ++msg;

    M_clang_message = msg;

    std::string::size_type pos = M_clang_message.find_last_of( ')' );
    if ( pos == std::string::npos )
    {
        std::cerr << "***ERROR*** (AudioSensor::buildCLangMessage)"
                  << " No last parenthesis. [" << msg << ']'
                  << std::endl;
        M_clang_message.clear();
        return;
    }

    M_clang_message.erase( pos );

    dlog.addText( Logger::SENSOR,
                  __FILE__" (buildCLangMessage) msg=[%s]",
                  M_clang_message.c_str() );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
AudioSensor::parseCLangMessage()
{
    if ( M_clang_parser.parse( M_clang_message )
         && M_clang_parser.message() )
    {
        dlog.addText( Logger::SENSOR,
                      __FILE__": (parseCLangMessage) ok message type = %s",
                      M_clang_parser.message()->typeName() );
        return true;
    }

    dlog.addText( Logger::SENSOR,
                  __FILE__": (parseCLangMessage) failed to parse clang [%s]",
                  M_clang_message.c_str() );
    std::cerr << __FILE__ << ": ***ERROR*** (parseCLangMessage) failed."
              << std::endl;
    M_clang_parser.clear();
    return false;
}

}
