// -*-c++-*-

/*!
  \file say_message_parser.cpp
  \brief player's say message parser Source File
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

#include "say_message_parser.h"

#include "audio_codec.h"
#include "audio_memory.h"

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/game_time.h>

#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
BallMessageParser::BallMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}


/*-------------------------------------------------------------------*/
/*!

*/
int
BallMessageParser::parse( const int sender ,
                          const double & ,
                          const char * msg,
                          const GameTime & current )
{
    // format:
    //    "b<pos_vel:5>"
    // the length of message == 6

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "***ERROR*** BallMessageParser::parse()"
                  << " Illegal ball message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallMessageParser: Illegal ball info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    Vector2D ball_pos;
    Vector2D ball_vel;

    if ( ! AudioCodec::i().decodeStr5ToPosVel( std::string( msg, slength() - 1 ),
                                               &ball_pos, &ball_vel ) )
    {
        std::cerr << "***ERROR*** BallMessageParser::parse()"
                  << " Failed to decode ball [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallMessageParser: Failed to decode Ball Info [%s]",
                      msg );
        return -1;
    }

    dlog.addText( Logger::SENSOR,
                  "BallMessageParser::parse() success! pos(%.1f %.1f) vel(%.1f %.1f)",
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y );

    M_memory->setBall( sender, ball_pos, ball_vel, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
PassMessageParser::PassMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
PassMessageParser::parse( const int sender,
                          const double & ,
                          const char * msg,
                          const GameTime & current )
{
    // format:
    //    "p<unum_pos:4><pos_vel:5>"
    // the length of message == 10

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "PassMessageParser::parse()"
                  << " Illegal pass pass message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessageParser Failed to decode Pass Info [%s]",
                      msg );
        return -1;
    }

    ++msg;

    int receiver_number = 0;
    Vector2D receive_pos;

    if ( ! AudioCodec::i().decodeStr4ToUnumPos( std::string( msg, 4 ),
                                                &receiver_number,
                                                &receive_pos ) )
    {
        std::cerr << "PassMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessageParser: Failed to decode Pass Info [%s]",
                      msg );
        return -1;
    }
    msg += 4;

    Vector2D ball_pos;
    Vector2D ball_vel;

    if ( ! AudioCodec::i().decodeStr5ToPosVel( std::string( msg, 5 ),
                                               &ball_pos, &ball_vel ) )
    {
        std::cerr << "***ERROR*** PassMessageParser::parse()"
                  << " Failed to decode ball [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessageParser: Failed to decode Ball Info [%s]",
                      msg );
        return -1;
    }
    msg += 5;

    dlog.addText( Logger::SENSOR,
                  "PassMessageParser::parse() success! receiver %d"
                  " recv_pos(%.1f %.1f)"
                  " bpos(%.3f %.3f) bvel(%.3f %.3f)",
                  receiver_number,
                  receive_pos.x, receive_pos.y,
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y );

    M_memory->setPass( sender, receiver_number, receive_pos, current );
    M_memory->setBall( sender, ball_pos, ball_vel, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
InterceptMessageParser::InterceptMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
InterceptMessageParser::parse( const int sender,
                               const double & ,
                               const char * msg,
                               const GameTime & current )
{
    // format:
    //    "i<unum:1><cycle:1>"
    // the length of message == 3

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "InterceptMessageParser::parse()"
                  << " Illegal message = [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "AudioSensor: Failed to decode intercept info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    AudioCodec::CharToIntCont::const_iterator unum_it = AudioCodec::i().charToIntMap().find( *msg );
    if ( unum_it == AudioCodec::i().charToIntMap().end()
         || unum_it->second <= 0
         || MAX_PLAYER*2 < unum_it->second )
    {
        std::cerr << "InterceptMessageParser::parse() "
                  << " Illegal player number. message = [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "InterceptMessageParser: Failed to decode intercept info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    AudioCodec::CharToIntCont::const_iterator cycle = AudioCodec::i().charToIntMap().find( *msg );
    if ( cycle == AudioCodec::i().charToIntMap().end() )
    {
        std::cerr << "InterceptMessageParser::parse() "
                  << " Illegal cycle. message = [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "InterceptMessageParser: Failed to decode intercept info [%s]",
                      msg );
        return -1;
    }

    dlog.addText( Logger::SENSOR,
                  "InterceptMessageParser: success! number=%d cycle=%d",
                  unum_it->second, cycle->second );

    M_memory->setIntercept( sender, unum_it->second, cycle->second, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
GoalieMessageParser::GoalieMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
GoalieMessageParser::parse( const int sender,
                            const double & ,
                            const char * msg,
                            const GameTime & current )
{
    // format:
    //    "g<pos_body:4>"
    // the length of message == 5

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "GoalieMessageParser::parse()."
                  << " Illegal message [" << msg
                  << "] len = " << std::strlen( msg )
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "GoalieMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }

    Vector2D goalie_pos;
    AngleDeg goalie_body;

    goalie_body = static_cast< double >( ival % 360 - 180 );
    ival /= 360;

    goalie_pos.y = ( ival % 400 ) * 0.1 - 20.0;
    ival /= 400;

    goalie_pos.x = ( ival % 160 ) * 0.1 + ( 53.0 - 16.0 );

    dlog.addText( Logger::SENSOR,
                  "GoalieMessageParser: success! goalie pos = (%.2f %.2f) body = %.1f",
                  goalie_pos.x, goalie_pos.y, goalie_body.degree() );

    M_memory->setOpponentGoalie( sender, goalie_pos, goalie_body, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
GoalieAndPlayerMessageParser::GoalieAndPlayerMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
GoalieAndPlayerMessageParser::parse( const int sender,
                                   const double & ,
                                   const char * msg,
                                   const GameTime & current )
{
    // format:
    //    "g<pos_body:4,unm_pos:3>"
    // the length of message == 8

    // ( 22 * 105/0.63 * 68/0.63 ) * ( 16.0/0.1 * 40.0/0.1 * 360 )
    // -> (22 * 168 * 109) * (160 * 400 * 360) = 9281986560000

    // 74^6 = 164206490176
    //        9281986560000
    // 74^7 = 12151280273024

    // ( 22 * 105/0.55 * 68/0.55 ) * ( 16.0/0.1 * 40.0/0.1 * 360 )
    // -> (22 * 192 * 125) * (160 * 400 * 360) = 12165120000000

    // ==========
    // ( 22 * 105/0.555 * 68/0.555 ) * ( 16.0/0.1 * 40.0/0.1 * 360 )
    // -> (22 * 191 * 124) * (160 * 400 * 360) = 12004945920000
    // ==========

    // ( 22 * 105/0.56 * 68/0.55 ) * ( 16.0/0.1 * 40.0/0.1 * 360 )
    // -> (22 * 189 * 123) * (160 * 400 * 360) = 11783439360000


    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "Goalie1PlayerMessageParser::parse()."
                  << " Illegal message [" << msg
                  << "] len = " << std::strlen( msg )
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "Goalie1PlayerMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "Goalie1PlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "Goalie1PlayerMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }

    Vector2D goalie_pos;
    AngleDeg goalie_body;
    int player_number = Unum_Unknown;
    Vector2D player_pos;

    // 124 > 68/0.555 + 1
    player_pos.y = ( ival % 124 ) * 0.555 - 34.0;
    ival /= 124;

    // 191 > 105/0.555 + 1
    player_pos.x = ( ival % 191 ) * 0.555 - 52.5;
    ival /= 191;

    // 22
    player_number = ( ival % 22 ) + 1;
    ival /= 22;


    goalie_body = static_cast< double >( ival % 360 - 180 );
    ival /= 360;

    goalie_pos.y = ( ival % 400 ) * 0.1 - 20.0;
    ival /= 400;

    goalie_pos.x = ( ival % 160 ) * 0.1 + ( 53.0 - 16.0 );
    // ival /= 160;


    dlog.addText( Logger::SENSOR,
                  "GoalieAndPlayerMessageParser: success! goalie pos=(%.2f %.2f) body=%.1f",
                  goalie_pos.x, goalie_pos.y, goalie_body.degree() );
    dlog.addText( Logger::SENSOR,
                  "____ player number=%d pos=(%.2f %.2f)",
                  player_number, player_pos.x, player_pos.y );

    M_memory->setOpponentGoalie( sender, goalie_pos, goalie_body, current );
    M_memory->setPlayer( sender, player_number, player_pos, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
OffsideLineMessageParser::OffsideLineMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
OffsideLineMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "o<x_rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "OffsideLineMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessageParser: Failed to decode Offside Line Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );

    if ( rate == AudioCodec::ERROR_VALUE )
    {
        std::cerr << "OffsideLineMessageParser::parse()"
                  << " Failed to read offside line"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessageParser: Failed to decode Offside Line Info [%s]",
                      msg );
        return -1;
    }

    double offside_line_x = 10.0 + ( 52.0 - 10.0 ) * rate;

    dlog.addText( Logger::SENSOR,
                  "OffsideLineMessageParser: success! x=%.1f rate=%.3f",
                  offside_line_x, rate );

    M_memory->setOffsideLine( sender, offside_line_x, current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
DefenseLineMessageParser::DefenseLineMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
DefenseLineMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "d<x_rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "DefenseLineMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessageParser: Failed to decode Defense Line Info [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );

    if ( rate == AudioCodec::ERROR_VALUE )
    {
        std::cerr << "DefenseLineMessageParser::parser()"
                  << " Failed to read offside line [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessageParser: Failed to decode Defense Line Info [%s]",
                      msg );
        return -1;
    }

    double defense_line_x = 52.0 + ( -10.0 + 52.0 ) * rate;

    dlog.addText( Logger::SENSOR,
                  "DefenseLineMessageParser::parse() success! x=%.1f rate=%.3f",
                  defense_line_x, rate );

    M_memory->setDefenseLine( sender, defense_line_x, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
WaitRequestMessageParser::WaitRequestMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
WaitRequestMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    if ( *msg != sheader() )
    {
        return 0;
    }

    M_memory->setWaitRequest( sender, current );
    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
SetplayMessageParser::SetplayMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
SetplayMessageParser::parse( const int sender,
                             const double & ,
                             const char * msg,
                             const GameTime & current )
{
    // format:
    //    "F<wait:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "(SetplayMessageParser::parse) illegal message [" << msg
                  << ']' << std::endl;
        dlog.addText( Logger::SENSOR,
                      "SetplayMessageParser: illegal message length [%s]", msg );
        return -1;
    }
    ++msg;

    AudioCodec::CharToIntCont::const_iterator it = AudioCodec::i().charToIntMap().find( *msg );
    if ( it == AudioCodec::i().charToIntMap().end()
         || it->second <= 0 )
    {
        std::cerr << "(SetplayMessageParser::parse) illegal value [" << msg
                  << ']' << std::endl;
        dlog.addText( Logger::SENSOR,
                      "SetplayMessageParser: Failed to decode [%s]", msg );
        return -1;
    }

    M_memory->setSetplay( sender, it->second, current );
    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
PassRequestMessageParser::PassRequestMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
PassRequestMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "h<pos:3>"
    // the length of message == 4

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "PassRequestMessageParser::parse()"
                  << " Illegal pass request message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    Vector2D pos;

    if ( ! AudioCodec::i().decodeStr3ToPos( std::string( msg, slength() - 1 ),
                                            &pos ) )
    {
        std::cerr << "PassRequestMessage::parse()"
                  << " Failed to decode pass request potiiton. ["
                  << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "AudioSensor: Failed to decode hey pass potiiton" );
        return -1;
    }

    dlog.addText( Logger::SENSOR,
                  "PassRequestMessageParser: success! "
                  "sender = %d  request pos = (%.2f %.2f)",
                  sender, pos.x, pos.y );

    M_memory->setPassRequest( sender, pos, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
StaminaMessageParser::StaminaMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
StaminaMessageParser::parse( const int sender,
                             const double & ,
                             const char * msg,
                             const GameTime & current )
{
    // format:
    //    "s<rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "StaminaMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaMessageParser: Failed to decode Stamina Rate [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );
    if ( rate < 0.0 || 1.00001 < rate )
    {
        std::cerr << "StaminaMessageParser::parser()"
                  << " Failed to read stamina rate [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaMessageParser: Failed to decode Stamina Rate [%s]",
                      msg );
        return -1;
    }

    double stamina = ServerParam::i().staminaMax() * rate;

    dlog.addText( Logger::SENSOR,
                  "StaminaMessageParser::parse() success! rate=%f stamina=%.1f",
                  rate, stamina );

    M_memory->setStamina( sender, rate, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
RecoveryMessageParser::RecoveryMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
RecoveryMessageParser::parse( const int sender,
                              const double & ,
                              const char * msg,
                              const GameTime & current )
{
    // format:
    //    "r<rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "RecoveryMessageParser::parse()"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessageParser: Failed to decode Recovery Rate [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );
    if ( rate == AudioCodec::ERROR_VALUE )
    {
        std::cerr << "RecoveryMessageParser::parser()"
                  << " Failed to read recovery rate [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessageParser: Failed to decode Recovery Rate [%s]",
                      msg );
        return -1;
    }

    double recovery
        = rate * ( ServerParam::i().recoverInit() - ServerParam::i().recoverMin() )
        + ServerParam::i().recoverMin();

    dlog.addText( Logger::SENSOR,
                  "RecoverMessageParser::parse() success! rate=%f recovery=%.3f",
                  rate, recovery );

    M_memory->setRecovery( sender, rate, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
StaminaCapacityMessageParser::StaminaCapacityMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
StaminaCapacityMessageParser::parse( const int sender,
                                     const double & ,
                                     const char * msg,
                                     const GameTime & current )
{
    // format:
    //    "c<rate:1>"
    // the length of message == 2

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( static_cast< int >( std::strlen( msg ) ) < slength() )
    {
        std::cerr << "(StaminaCapacityMessageParser::parse)"
                  << " Illegal message [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaCapacityMessageParser: Failed to decode Stamina Rate [%s]",
                      msg );
        return -1;
    }
    ++msg;

    double rate = AudioCodec::i().decodeCharToPercentage( *msg );
    if ( rate < 0.0 || 1.00001 < rate )
    {
        std::cerr << "(StaminaCapacityMessageParser::parse)"
                  << " Failed to read stamina rate [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaCapacityMessageParser: Failed to decode stamina canapcity rate [%s]",
                      msg );
        return -1;
    }

    double value = ServerParam::i().staminaCapacity() * rate;

    dlog.addText( Logger::SENSOR,
                  "(StaminaCapacityMessageParser::parse) success! rate=%f value=%.1f",
                  rate, value );

    M_memory->setStaminaCapacity( sender, rate, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
DribbleMessageParser::DribbleMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
DribbleMessageParser::parse( const int sender,
                                   const double & ,
                                   const char * msg,
                                   const GameTime & current )
{
    // format:
    //    "D<count_pos:3>"
    // the length of message == 4

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "DribbleMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;

    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "DribbleMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DribbleMessageParser: Failed to decode Dribble Info [%s]",
                      msg );
        return -1;
    }

    Vector2D pos;
    int count;

    count = static_cast< int >( ival % 10 ) + 1;
    ival /= 10;

    std::int64_t div = static_cast< std::int64_t >( std::ceil( 68.0 / 0.5 ) );
    pos.y = ( ival % div ) * 0.5 - 34.0;
    ival /= div;

    // div = static_cast< std::int64_t >( std::ceil( 105.0 / 0.5 ) );
    // pos.x = ( ival % div ) * 0.5 - 52.5;
    pos.x = ival * 0.5 - 52.5;


    dlog.addText( Logger::SENSOR,
                  "DribbleMessageParser: success! "
                  "sender = %d  target_pos=(%.2f %.2f) count=%d",
                  sender, pos.x, pos.y, count );

    M_memory->setDribbleTarget( sender, pos, count, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
BallGoalieMessageParser::BallGoalieMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
BallGoalieMessageParser::parse( const int sender,
                                const double & ,
                                const char * msg,
                                const GameTime & current )
{
    // format:
    //    "G<bpos_bvel_gpos_gbody:9>"
    // the length of message == 10

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "BallGoalieMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "BallGoalieMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessageParser: Failed to decode Goalie Info [%s]",
                      msg );
        return -1;
    }

    // 74^9                       = 66540410775079424
    // 1050*680*60*60*160*400*360 = 59222016000000000
    // 1050*680*63*63*160*400*360 = 65292272640000000

    const double max_speed = ServerParam::i().ballSpeedMax();
    const double prec_speed = max_speed * 2.0 / 63.0;

    Vector2D ball_pos;
    Vector2D ball_vel;
    Vector2D goalie_pos;
    AngleDeg goalie_body;

    goalie_body = static_cast< double >( ival % 360 - 180 );
    ival /= 360;

    goalie_pos.y = ( ival % 400 ) * 0.1 - 20.0;
    ival /= 400;

    goalie_pos.x = ( ival % 160 ) * 0.1 + ( 52.5 - 16.0 );
    ival /= 160;

    ball_vel.y = ( ival % 63 ) * prec_speed - max_speed;
    ival /= 63;

    ball_vel.x = ( ival % 63 ) * prec_speed - max_speed;
    ival /= 63;

    ball_pos.y = ( ival % 680 ) * 0.1 - 34.0;
    ival /= 680;

    ball_pos.x = ( ival % 1050 ) * 0.1 - 52.5;
    //ival /= 1050;

    dlog.addText( Logger::SENSOR,
                  "BallGoalieMessageParser: success! "
                  "sender = %d  bpos(%.1f %.1f) bvel(%.1f %.1f)"
                  " gpos(%.1f %.1f) gbody %.1f",
                  sender,
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y,
                  goalie_pos.x, goalie_pos.y,
                  goalie_body.degree() );

    M_memory->setBall( sender, ball_pos, ball_vel, current );
    M_memory->setOpponentGoalie( sender, goalie_pos, goalie_body, current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
OnePlayerMessageParser::OnePlayerMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
OnePlayerMessageParser::parse( const int sender,
                               const double & ,
                               const char * msg,
                               const GameTime & current )
{
    // format:
    //    "P<unum_pos:3>"
    // the length of message == 4

    // ( 22 * 105/0.63 * 68/0.63 ) = 395767.195767196 < 74^3(=405224)
    //  -> 22 * 168 * 109 = 402864

    //               74^3 = 405224

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "OnePlayerMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "OnePlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = Unum_Unknown;
    Vector2D player_pos;

    // 109 > 68/0.63 + 1
    player_pos.y = ( ival % 109 ) * 0.63 - 34.0;
    ival /= 109;

    // 168 > 105/0.63 + 1
    player_pos.x = ( ival % 168 ) * 0.63 - 52.5;
    ival /= 168;

    // 22
    player_unum = ( ival % 22 ) + 1;
    ival /= 22;

    dlog.addText( Logger::SENSOR,
                  "OnePlayerMessageParser: success! "
                  "unum = %d  pos(%.1f %.1f)",
                  player_unum,
                  player_pos.x, player_pos.y );

    M_memory->setPlayer( sender,
                         player_unum,
                         player_pos,
                         current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
TwoPlayerMessageParser::TwoPlayerMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
TwoPlayerMessageParser::parse( const int sender,
                               const double & ,
                               const char * msg,
                               const GameTime & current )
{
    // format:
    //    "Q<unum_pos:3,unum_pos3>"
    // the length of message == 7

    // ( 22 * 105/0.63 * 68/0.63 ) = 395767.195767196 < 74^3(=405224)
    //  -> 22 * 168 * 109 = 402864
    //     (22 * 168 * 109)^2 = 162299402496

    //                   74^6 = 164206490176

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "TwoPlayerMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "TwoPlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "TwoPlayerMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player1_unum = Unum_Unknown;
    Vector2D player1_pos;
    int player2_unum = Unum_Unknown;
    Vector2D player2_pos;

    // 109 > 68/0.63
    player2_pos.y = ( ival % 109 ) * 0.63 - 34.0;
    ival /= 109;

    // 168 > 105/0.63
    player2_pos.x = ( ival % 168 ) * 0.63 - 52.5;
    ival /= 168;

    // 22
    player2_unum = ( ival % 22 ) + 1;
    ival /= 22;


    // 109 > 68/0.63
    player1_pos.y = ( ival % 109 ) * 0.63 - 34.0;
    ival /= 109;

    // 168 > 105/0.63
    player1_pos.x = ( ival % 168 ) * 0.63 - 52.5;
    ival /= 168;

    // 22
    player1_unum = ( ival % 22 ) + 1;
    ival /= 22;

    dlog.addText( Logger::SENSOR,
                  "TwoPlayerMessageParser: success! "
                  "(unum=%d (%.2f %.2f)), (unum=%d (%.2f %.2f)) ",
                  player1_unum,
                  player1_pos.x, player1_pos.y,
                  player2_unum,
                  player2_pos.x, player2_pos.y );

    M_memory->setPlayer( sender,
                         player1_unum,
                         player1_pos,
                         current );
    M_memory->setPlayer( sender,
                         player2_unum,
                         player2_pos,
                         current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
ThreePlayerMessageParser::ThreePlayerMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
ThreePlayerMessageParser::parse( const int sender,
                                 const double & ,
                                 const char * msg,
                                 const GameTime & current )
{
    // format:
    //    "R<unum_pos:3,unum_pos:3,unm_pos:3>"
    // the length of message == 10

    // ( 22 * 105/0.63 * 68/0.63 ) = 395767.195767196 < 74^3(=405224)
    //  -> 22 * 168 * 109 = 402864
    //     (22 * 168 * 109)^3 = 65384586487148544

    //                   74^9 = 66540410775079424

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "ThreePlayerMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "ThreePlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "ThreePlayerMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player1_unum = Unum_Unknown;
    Vector2D player1_pos;
    int player2_unum = Unum_Unknown;
    Vector2D player2_pos;
    int player3_unum = Unum_Unknown;
    Vector2D player3_pos;

    // 109 > 68/0.63
    player3_pos.y = ( ival % 109 ) * 0.63 - 34.0;
    ival /= 109;

    // 168 > 105/0.63
    player3_pos.x = ( ival % 168 ) * 0.63 - 52.5;
    ival /= 168;

    // 22
    player3_unum = ( ival % 22 ) + 1;
    ival /= 22;

    // 109 > 68/0.63
    player2_pos.y = ( ival % 109 ) * 0.63 - 34.0;
    ival /= 109;

    // 168 > 105/0.63
    player2_pos.x = ( ival % 168 ) * 0.63 - 52.5;
    ival /= 168;

    // 22
    player2_unum = ( ival % 22 ) + 1;
    ival /= 22;


    // 109 > 68/0.63
    player1_pos.y = ( ival % 109 ) * 0.63 - 34.0;
    ival /= 109;

    // 168 > 105/0.63
    player1_pos.x = ( ival % 168 ) * 0.63 - 52.5;
    ival /= 168;

    // 22
    player1_unum = ( ival % 22 ) + 1;
    ival /= 22;

    dlog.addText( Logger::SENSOR,
                  "ThreePlayerMessageParser: success! "
                  "(unum=%d (%.2f %.2f)), (unum=%d (%.2f %.2f)), (unum=%d (%.2f %.2f)) ",
                  player1_unum,
                  player1_pos.x, player1_pos.y,
                  player2_unum,
                  player2_pos.x, player2_pos.y,
                  player3_unum,
                  player3_pos.x, player3_pos.y );

    M_memory->setPlayer( sender,
                         player1_unum,
                         player1_pos,
                         current );
    M_memory->setPlayer( sender,
                         player2_unum,
                         player2_pos,
                         current );
    M_memory->setPlayer( sender,
                         player3_unum,
                         player3_pos,
                         current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
SelfMessageParser::SelfMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
SelfMessageParser::parse( const int sender,
                          const double & ,
                          const char * msg,
                          const GameTime & current )
{
    // format:
    //    "S<pos_body_stamina:4>"
    // the length of message == 5

    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "SelfMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "SelfMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "SelfMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = sender;
    Vector2D player_pos;
    AngleDeg player_body;
    double stamina = -1.0;

    // 11
    stamina = ServerParam::i().staminaMax() * static_cast< double >( ( ival % 11 ) ) / 10.0;
    ival /= 11;

    // 60=360/6
    player_body = ( ival % 60 ) * 6.0 - 180.0;
    ival /= 60;

    // 171 > 68/0.4
    player_pos.y = ( ival % 171 ) * 0.4 - 34.0;
    ival /= 171;

    // 264 > 105/0.4=262.5
    player_pos.x = ( ival % 264 ) * 0.4 - 52.5;
    //ival /= 264;

    dlog.addText( Logger::SENSOR,
                  "SelfMessageParser: success! "
                  "unum = %d  pos(%.1f %.1f) body=%.1f stamina=%f",
                  player_unum,
                  player_pos.x, player_pos.y,
                  player_body.degree(),
                  stamina );

    M_memory->setPlayer( sender,
                         player_unum,
                         player_pos,
                         player_body.degree(),
                         stamina,
                         current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
TeammateMessageParser::TeammateMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
TeammateMessageParser::parse( const int sender,
                              const double & ,
                              const char * msg,
                              const GameTime & current )
{
    // format:
    //    "T<unum_pos_body:4>"
    // the length of message == 5

    //11 * 105/0.7 * 68/0.7 * 360/2
    // -> 11 * 151 * 98 * 180
    // =29300040 < 4 characters(74^4=29986576)


    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "TeammateMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "TeammateMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "TeammateMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = Unum_Unknown;
    Vector2D player_pos;
    AngleDeg player_body;

    // 180=360/2
    player_body = static_cast< double >( ( ival % 180 ) * 2 - 180 );
    ival /= 180;

    // 98=68/0.7=97.14
    player_pos.y = ( ival % 98 ) * 0.7 - 34.0;
    ival /= 98;

    // 151>105/0.7=150
    player_pos.x = ( ival % 151 ) * 0.7 - 52.5;
    ival /= 151;

    player_unum = ( ival % 11 ) + 1;
    // ival /= 11

    dlog.addText( Logger::SENSOR,
                  "TeammateMessageParser: success! "
                  "unum = %d  pos(%.1f %.1f) body %.1f",
                  player_unum,
                  player_pos.x, player_pos.y,
                  player_body.degree() );

    M_memory->setPlayer( sender,
                         player_unum,
                         player_pos,
                         player_body.degree(),
                         -1.0, // unknown stamina
                         current );

    return slength();
}


/*-------------------------------------------------------------------*/
/*!

*/
OpponentMessageParser::OpponentMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
OpponentMessageParser::parse( const int sender,
                              const double & ,
                              const char * msg,
                              const GameTime & current )
{
    // format:
    //    "O<unum_pos_body:4>"
    // the length of message == 5

    //11 * 105/0.7 * 68/0.7 * 360/2
    // -> 11 * 151 * 98 * 180
    // =29300040 < 4 characters(74^4=29986576)


    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "OpponentMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, slength() - 1 ),
                                             &ival ) )
    {
        std::cerr << "OpponentMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OpponentMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = Unum_Unknown;
    Vector2D player_pos;
    AngleDeg player_body;

    // 180=360/2
    player_body = static_cast< double >( ( ival % 180 ) * 2 - 180 );
    ival /= 180;

    // 98=68/0.7=97.14
    player_pos.y = ( ival % 98 ) * 0.7 - 34.0;
    ival /= 98;

    // 151>105/0.7=150
    player_pos.x = ( ival % 151 ) * 0.7 - 52.5;
    ival /= 151;

    player_unum = ( ival % 11 ) + 1;
    // ival /= 11

    dlog.addText( Logger::SENSOR,
                  "OpponentMessageParser: success! "
                  "unum = %d  pos(%.1f %.1f) body %.1f",
                  player_unum,
                  player_pos.x, player_pos.y,
                  player_body.degree() );

    M_memory->setPlayer( sender,
                         player_unum + 11,
                         player_pos,
                         player_body.degree(),
                         -1.0, // unknown stamina
                         current );

    return slength();
}

/*-------------------------------------------------------------------*/
/*!

*/
BallPlayerMessageParser::BallPlayerMessageParser( std::shared_ptr< AudioMemory > memory )
    : M_memory( memory )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
int
BallPlayerMessageParser::parse( const int sender,
                                const double & ,
                                const char * msg,
                                const GameTime & current )
{
    // format:
    //    "P<bpos_bvel_unum_pos_body:9>"
    // the length of message == 10


    if ( *msg != sheader() )
    {
        return 0;
    }

    if ( (int)std::strlen( msg ) < slength() )
    {
        std::cerr << "OnePlayerMessageParser::parse()"
                  << " Illegal message ["
                  << msg << "] len = " << std::strlen( msg )
                  << std::endl;
        return -1;
    }
    ++msg;

    Vector2D ball_pos;
    Vector2D ball_vel;

    if ( ! AudioCodec::i().decodeStr5ToPosVel( std::string( msg, 5 ),
                                               &ball_pos, &ball_vel ) )
    {
        std::cerr << "***ERROR*** BallPlayerMessageParser::parse()"
                  << " Failed to decode ball [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessageParser: Failed to decode Ball Info [%s]",
                      msg );
        return -1;
    }
    msg += 5;

    std::int64_t ival = 0;
    if ( ! AudioCodec::i().decodeStrToInt64( std::string( msg, 4 ),
                                             &ival ) )
    {
        std::cerr << "BallPlayerMessageParser::parse()"
                  << " Failed to parse [" << msg << "]"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessageParser: Failed to decode Player Info [%s]",
                      msg );
        return -1;
    }

    int player_unum = Unum_Unknown;
    Vector2D player_pos;
    AngleDeg player_body;

    // 180=360/2
    player_body = static_cast< double >( ( ival % 180 ) * 2 - 180 );
    ival /= 180;

    // 69 > 68/1.0
    player_pos.y = ( ival % 69 ) * 1.0 - 34.0;
    ival /= 69;

    // 106 > 105/1.0
    player_pos.x = ( ival % 106 ) * 1.0 - 52.5;
    ival /= 106;

    player_unum = ( ival % 22 ) + 1;
    // ival /= 22

    dlog.addText( Logger::SENSOR,
                  "BallPlayerMessageParser: success! "
                  " bpos(%.1f %.1f) bvel(%.1f %.1f)"
                  " unum=%d  pos(%.1f %.1f) body %.1f",
                  ball_pos.x, ball_pos.y,
                  ball_vel.x, ball_vel.y,
                  player_unum,
                  player_pos.x, player_pos.y,
                  player_body.degree() );

    M_memory->setBall( sender, ball_pos, ball_vel, current );
    M_memory->setPlayer( sender,
                         player_unum,
                         player_pos,
                         player_body.degree(),
                         -1.0, // unknown stamina
                         current );

    return slength();
}

} // end namespace rcsc
