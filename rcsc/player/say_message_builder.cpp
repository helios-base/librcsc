// -*-c++-*-

/*!
  \file say_message_builder.cpp
  \brief player's say message builder Source File
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

#include "say_message_builder.h"

#include <rcsc/common/audio_codec.h>
#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>
#include <rcsc/math_util.h>

#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
bool
BallMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "BallMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodePosVelToStr5( M_ball_pos,
                                               M_ball_vel,
                                               msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallMessage. "
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallMessage. error!"
                      " pos=(%f %f) vel=(%f %f)",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "BallMessage. success!"
                  " pos=(%f %f) vel=(%f %f) -> [%s]",
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
BallMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Ball "
    //        << '(' << round( M_ball_pos.x, 0.1 ) << ',' << round( M_ball_pos.y, 0.1 ) << ")("
    //        << round( M_ball_vel.x, 0.01 ) << ',' << round( M_ball_vel.y, 0.01 ) << ")]";
    os << "[Ball]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PassMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "PassMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeUnumPosToStr4( M_receiver_unum,
                                                M_receive_point,
                                                msg ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassMessage.  receiver"
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "PassMessage. error! receiver=%d pos=(%f %f)",
                      M_receiver_unum,
                      M_receive_point.x, M_receive_point.y );
        return false;
    }

    if ( ! AudioCodec::i().encodePosVelToStr5( M_ball_pos,
                                               M_ball_vel,
                                               msg ) )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassMessage. ball info"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessage. error!"
                      " ball_pos=(%f %f) vel=(%f %f)",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y );
        return false;
    }

    if ( (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassMessage. length"
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassMessage. error!"
                      " illegal message length = %d [%s] ",
                      msg.length(), msg.c_str() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "PassMessage. success!"
                  " receiver=%d recv_pos=(%f %f)"
                  " bpos(%f %f) bvel(%f %f) -> [%s]",
                  M_receiver_unum,
                  M_receive_point.x, M_receive_point.y,
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PassMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Pass "
    //        << M_receiver_unum << " ("
    //        << round( M_receive_point.x, 0.1 ) << ',' << round( M_receive_point.y, 0.1 )
    //        << ")]";
    os << "[Pass:" << M_receiver_unum << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
GoalieMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    // 74^4 = 29986576
    // 16 * 40 * 360 / 74^4 < prec^2, prec > 0.087655223

    if ( M_goalie_pos.x < 53.0 - 16.0
         || 52.9 < M_goalie_pos.x
         || M_goalie_pos.absY() > 19.9 )
    {
//         std::cerr << __FILE__ << ":" << __LINE__
//                   << " ***ERROR*** GoalieMessage. over the position range. "
//                   << M_goalie_pos
//                   << std::endl;
        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. over the position range : (%f %f)",
                      M_goalie_pos.x, M_goalie_pos.y );
        return false;
    }

    std::int64_t ival = 0;

    double x = min_max( 53.0 - 16.0, M_goalie_pos.x, 52.9 ) - ( 53.0 - 16.0 );
    double y = min_max( -19.9, M_goalie_pos.y, 19.9 ) + 20.0;
    double body = M_goalie_body.degree() + 180.0;

    // ival *= 160
    ival += static_cast< std::int64_t >( bound( 0.0, rint( x / 0.1 ), 159.0 ) );

    ival *= 400;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( y / 0.1 ), 399.0 ) );

    ival *= 360;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( body ), 359.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** GoalieMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "GoalieMessage. error! unum=%d pos=(%f %f) body=%f",
                      M_goalie_unum, M_goalie_pos.x, M_goalie_pos.y,
                      M_goalie_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "GoalieMessage. success! unum=%d pos=(%f %f) x=%f y=%f -> [%s]",
                  M_goalie_unum, M_goalie_pos.x, M_goalie_pos.y,
                  x, y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
GoalieMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Goalie "
    //        << M_goalie_unum << " ("
    //        << round( M_goalie_pos.x, 0.1 ) << ',' << round( M_goalie_pos.y, 0.1 )
    //        << ")]";
    os << "[Goalie]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
GoalieAndPlayerMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "GoalieAndPlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_goalie_pos.x < 53.0 - 16.0
         || 52.9 < M_goalie_pos.x
         || M_goalie_pos.absY() > 19.9 )
    {
        dlog.addText( Logger::SENSOR,
                      "GoalieAndPlayerMessage. over the position range : (%f %f)",
                      M_goalie_pos.x, M_goalie_pos.y );
        return false;
    }

    if ( M_player_number < 1 || 23 < M_player_number )
    {
        dlog.addText( Logger::SENSOR,
                      "GoalieAndPlayerMessage. illegal player number %d",
                      M_player_number );
    }

    std::int64_t ival = 0;

    double goalie_x = bound( 53.0 - 16.0, M_goalie_pos.x, 52.9 ) - ( 53.0 - 16.0 );
    double goalie_y = bound( -19.9, M_goalie_pos.y, 19.9 ) + 20.0;
    double goalie_body = M_goalie_body.degree() + 180.0;

    // ival *= 160
    ival += static_cast< std::int64_t >( bound( 0.0, rint( goalie_x / 0.1 ), 159.0 ) );

    ival *= 400;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( goalie_y / 0.1 ), 399.0 ) );

    ival *= 360;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( goalie_body ), 359.0 ) );

    double player_x = bound( -52.49, M_player_pos.x, 52.49 ) + 52.5;
    double player_y = bound( -33.99, M_player_pos.y, 33.99 ) + 34.0;

    ival *= 22;
    ival += M_player_number - 1;

    ival *= 191;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( player_x / 0.555 ), 190.0 ) );

    ival *= 124;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( player_y / 0.555 ), 123.0 ) );


    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** GoalieAndPlayerMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "GoalieAndPlayerMessage. error! goalie unum=%d (%.2f %.2f) body=%f"
                      " player=%d (%.2f %.2f)",
                      M_goalie_unum, M_goalie_pos.x, M_goalie_pos.y,
                      M_goalie_body.degree(),
                      M_player_number, M_player_pos.x, M_player_pos.y );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "GoalieAndPlayerMessage. success! goalie=%d (%.2f %.2f) x=%f y=%f"
                  " player num=%d (%.2f %.2f) -> [%s]",
                  M_goalie_unum, M_goalie_pos.x, M_goalie_pos.y,
                  goalie_x, goalie_y,
                  M_player_number, player_x, player_y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
GoalieAndPlayerMessage::printDebug( std::ostream & os ) const
{
    os << "[Goalie:"
       << ( M_player_number > 11 ? "O_" : "T_" )
       << ( M_player_number > 11 ? M_player_number - 11 : M_player_number )
       << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OffsideLineMessage::appendTo( std::string & to ) const
{
    if ( M_offside_line_x < 10.0 )
    {
        return false;
    }

    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double x = min_max( 10.0, M_offside_line_x, 52.0 ) - 10.0;
    double rate = x / ( 52.0 - 10.0 );

    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OffsideLineMessage. value = "
                  << M_offside_line_x
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OffsideLineMessage. error! real_x=%f, rate=%f",
                      M_offside_line_x, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "OffsideLineMessage. success! x=%f rate=%f [%c]",
                  M_offside_line_x, rate, ch );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
OffsideLineMessage::printDebug( std::ostream & os ) const
{
    os << "[OffsideLine:" << round( M_offside_line_x, 0.1 ) << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DefenseLineMessage::appendTo( std::string & to ) const
{
    if ( M_defense_line_x > -10.0 )
    {
        return false;
    }

    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double x = min_max( -52.0, M_defense_line_x, -10.0 ) + 52.0;
    double rate = x / ( -10.0 + 52.0 );

    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** DefenseLineMessage. value = " << M_defense_line_x
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DefenseLineMessage. error! x=%f, rate=%f",
                      M_defense_line_x, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "DefenseLineMessage. success! x=%f rate=%f -> [%c]",
                  M_defense_line_x, rate, ch );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
DefenseLineMessage::printDebug( std::ostream & os ) const
{
    os << "[DefenseLine:" << round( M_defense_line_x, 0.1 ) << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
WaitRequestMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "WaitRequestMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "WaitRequestMessage. success! [w]" );

    to += header();

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
WaitRequestMessage::printDebug( std::ostream & os ) const
{
    os << "[Wait]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SetplayMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "SetplayMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    char ch;
    try
    {
        ch = AudioCodec::i().intToCharMap().at( M_wait_step );
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** SetplayMessage. cannot encode wait_step = " << M_wait_step
                  << std::endl;
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "SetplayMessage. success! step=%d -> [F%c]", M_wait_step, ch );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SetplayMessage::printDebug( std::ostream & os ) const
{
    os << "[Setplay:" << M_wait_step << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
InterceptMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "InterceptMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    try
    {
        int unum = ( M_our ? M_unum : M_unum + MAX_PLAYER );

        char unum_ch = AudioCodec::i().intToCharMap().at( unum );
        char cycle_ch = AudioCodec::i().intToCharMap().at( M_cycle );

        to += header();
        to += unum_ch;
        to += cycle_ch;

        dlog.addText( Logger::SENSOR,
                      "InterceptMessage. success! %s unum = %d, cycle = %d -> [%c%c]",
                      M_our ? "our" : "opp",
                      M_unum, M_cycle, unum_ch, cycle_ch );

    }
    catch ( ... )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** InterceptMessage."
                  << " Exception caught! Failed to encode cycle = "
                  << M_cycle
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "InterceptMessage. error! unum = %d, cycle = %d",
                      M_unum, M_cycle );
        return false;
    }


    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
InterceptMessage::printDebug( std::ostream & os ) const
{
    os << "[Intercept "
       << ( M_our ? M_unum : -M_unum ) << ':' << M_cycle << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PassRequestMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "PassRequestMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodePosToStr3( M_target_point, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** PassRequestMessage. "
                  << "Failed to encode a pass request message. dash_target="
                  << M_target_point
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "PassRequestMessage. error!. dash_target=(%f %f)",
                      M_target_point.x, M_target_point.y );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "PassRequestMessage. success!. dash_target=(%f %f) -> [%s]",
                  M_target_point.x, M_target_point.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
PassRequestMessage::printDebug( std::ostream & os ) const
{
    os << "[HeyPass "
       << '(' << round( M_target_point.x, 0.1 ) << ',' << round( M_target_point.y, 0.1 ) << ")]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
StaminaMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "StaminaMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double rate = M_stamina / ServerParam::i().staminaMax();
    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** Say_Stamina. value = " << M_stamina
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaMessage. error! value= %f",
                      M_stamina );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "StaminaMessage. success! value= %f",
                  M_stamina );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
StaminaMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Stamina:" << round( M_stamina, 0.1 ) << ']';
    os << "[Stamina]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
RecoveryMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double rate
        = ( M_recovery - ServerParam::i().recoverMin() )
        / ( ServerParam::i().recoverInit() - ServerParam::i().recoverMin() );

    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** RecoveryMessage. value = " << M_recovery
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "RecoveryMessage: error!. value = %f. rate = %f",
                      M_recovery, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "RecoveryMessage: success!. value = %f. rate = %f",
                  M_recovery, rate );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
RecoveryMessage::printDebug( std::ostream & os ) const
{
    //os << "[Recovery " << M_recovery << ']';
    os << "[Recovery]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
StaminaCapacityMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "StaminaCapacityMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    double rate = M_stamina_capacity / ServerParam::i().staminaCapacity();
    char ch = AudioCodec::i().encodePercentageToChar( rate );

    if ( ch == '\0' )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** StaminaCapacityMessage. value = " << M_stamina_capacity
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "StaminaCapacityMessage: error!. value = %f. rate = %f",
                      M_stamina_capacity, rate );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "RecoveryMessage: success!. capacity = %f. rate = %f",
                  M_stamina_capacity, rate );

    to += header();
    to += ch;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
StaminaCapacityMessage::printDebug( std::ostream & os ) const
{
    os << "[StaminaCapacity]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
DribbleMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "DribbleMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    // 74^3 = 405224
    // 10 * 105.0/prec * 68.0/prec < 74^3
    // 10 * 105.0 * 68.0 / 74^3 < prec^2
    // prec > 0.419760459

    std::int64_t ival = 0;

    double x = min_max( -52.5, M_target_point.x, 52.5 ) + 52.5;
    double y = min_max( -34.0, M_target_point.y, 34.0 ) + 34.0;
    std::int64_t count = min_max( 1, M_queue_count, 10 );

    ival += static_cast< std::int64_t >( rint( x / 0.5 ) );

    ival *= static_cast< std::int64_t >( std::ceil( 68.0 / 0.5 ) );
    ival += static_cast< std::int64_t >( rint( y / 0.5 ) );

    ival *= 10;
    ival += count - 1;

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** DribbleMessage. target=" << M_target_point
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "DribbleMessage. error!. pos=(%f %f) count=%d,"
                      " message_length=%d",
                      M_target_point.x, M_target_point.y,
                      M_queue_count, msg.length() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "DribbleMessage. success!. pos=(%f %f) count=%d -> [%s]",
                  M_target_point.x, M_target_point.y,
                  M_queue_count,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
DribbleMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Dribble "
    //        << M_queue_count << ' '
    //        << '(' << round( M_target_point.x, 0.1 ) << ',' << round( M_target_point.y, 0.1 ) << ")]";
    os << "[Dribble:" << M_queue_count << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
BallGoalieMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_goalie_pos.x < 52.5 - 16.0
         || M_goalie_pos.absY() > 20.0 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallGoalieMessage. over the position range. "
                  << M_goalie_pos
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessage. over the position range : (%f %f)",
                      M_goalie_pos.x, M_goalie_pos.y );
        return false;
    }

    // 74^9                       = 66540410775079424
    // 1050*680*60*60*160*400*360 = 59222016000000000
    // 1050*680*63*63*160*400*360 = 65292272640000000

    const double max_speed = ServerParam::i().ballSpeedMax() * ServerParam::i().ballDecay();
    const double prec = max_speed * 2.0 / 63.0;

    std::int64_t ival = 0;
    double dval = 0.0;

    dval = min_max( -52.5, M_ball_pos.x, 52.5 ) + 52.5;
    // ival *= 1050
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.1 ), 1049.0 ) );

    dval = min_max( -34.0, M_ball_pos.y, 34.0 ) + 34.0;
    ival *= 680;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.1 ), 679.0 ) );

    if ( M_ball_vel.isValid() )
    {
        dval = min_max( -max_speed, M_ball_vel.x, max_speed ) + max_speed;
        ival *= 63;
        ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / prec ), 62.0 ) );

        dval = min_max( -max_speed, M_ball_vel.y, max_speed ) + max_speed;
        ival *= 63;
        ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / prec ), 62.0 ) );
    }
    else
    {
        ival *= 63;
        ival *= 63;
    }

    dval = min_max( 52.5 - 16.0, M_goalie_pos.x, 52.5 ) - ( 52.5 - 16.0 );
    ival *= 160;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.1 ), 159.0 ) );

    dval = min_max( -20.0, M_goalie_pos.y, 20.0 ) + 20.0;
    ival *= 400;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.1 ), 399.0 ) );

    dval = M_goalie_body.degree() + 180.0;
    ival *= 360;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval ), 359.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallGoalieMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "BallGoalieMessage. error!"
                      " bpos(%f %f) bvel(%f %f)"
                      " gpos=(%f %f) gbody=%f",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y,
                      M_goalie_pos.x, M_goalie_pos.y,
                      M_goalie_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "BallGoalieMessage. success!. bpos=(%f %f) bvel(%f %f)"
                  " gpos(%f %f) gbody %f -> [%s]",
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  M_goalie_pos.x, M_goalie_pos.y,
                  M_goalie_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
BallGoalieMessage::printDebug( std::ostream & os ) const
{
    //     os << "[BallGoalie "
    //        << "B(" << round( M_ball_pos.x, 0.1 ) << ',' << round( M_ball_pos.y, 0.1 ) << ')'
    //        << '(' <<  round( M_ball_vel.x, 0.01 ) << ',' << round( M_ball_vel.y, 0.01 ) << ')'
    //        << " G(" <<  round( M_goalie_pos.x, 0.1 ) << ',' << round( M_goalie_pos.y, 0.1 ) << ')'
    //        << ']';
    os << "[BallGoalie]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OnePlayerMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_unum < 1 || 22 < M_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OnePlayerMessage. illegal unum = "
                  << M_unum
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. illegal unum = %d",
                      M_unum );
        return false;
    }

    std::int64_t ival = 0;
    double player_x = min_max( -52.49, M_player_pos.x, 52.49 ) + 52.5;
    double player_y = min_max( -33.99, M_player_pos.y, 33.99 ) + 34.0;

    // ival *= 22;
    ival += M_unum - 1;

    ival *= 168;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( player_x / 0.63 ), 167.0 ) );

    ival *= 109;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( player_y / 0.63 ), 108.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OnePlayerMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "OnePlayerMessage. error!"
                      " unum=%d pos=(%f %f)",
                      M_unum,
                      M_player_pos.x, M_player_pos.y );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "OnePlayerMessage. success!. unum = %d pos=(%f %f) -> [%s]",
                  M_unum,
                  M_player_pos.x, M_player_pos.y,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
OnePlayerMessage::printDebug( std::ostream & os ) const
{
    //     os << "[1Player "
    //        << ( M_unum > 11 ? 11 - M_unum : M_unum ) << ' '
    //        << '(' << round( M_player_pos.x, 0.1 ) << ',' << round( M_player_pos.y, 0.1 ) << ')'
    //        << ']';
    os << "[1Player:"
       << ( M_unum > 11 ? "O_" : "T_" )  << ( M_unum > 11 ? M_unum - 11 : M_unum )
       << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
TwoPlayerMessage::TwoPlayerMessage( const int player0_unum,
                                    const Vector2D & player0_pos,
                                    const int player1_unum,
                                    const Vector2D & player1_pos )
{
    M_player_unum[0] = player0_unum;
    M_player_pos[0] = player0_pos;

    M_player_unum[1] = player1_unum;
    M_player_pos[1] = player1_pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TwoPlayerMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "TwoPlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::int64_t ival = 0;
    double dval = 0.0;

    for ( int i = 0; i < 2; ++i )
    {
        if ( M_player_unum[i] < 1 || 22 < M_player_unum[i] )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " ***ERROR*** TwoPlayerMessage. illegal unum = "
                      << M_player_unum[i]
                      << std::endl;
            dlog.addText( Logger::SENSOR,
                          "TwoPlayerMessage. illegal unum = %d",
                          M_player_unum[i] );
            return false;
        }

        ival *= 22;
        ival += M_player_unum[i] - 1;

        dval = min_max( -52.49, M_player_pos[i].x, 52.49 ) + 52.5;
        ival *= 168;
        ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.63 ), 167.0 ) );

        dval = min_max( -33.99, M_player_pos[i].y, 33.99 ) + 34.0;
        ival *= 109;
        ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.63 ), 108.0 ) );
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** TwoPlayerMessage. "
                  << std::endl;

        for ( int i = 0; i < 2; ++i )
        {
            dlog.addText( Logger::SENSOR,
                          "TwoPlayerMessage. error! unum=%d pos=(%f %f)",
                          M_player_unum[i],
                          M_player_pos[i].x, M_player_pos[i].y );
        }
        return false;
    }

    if ( dlog.isEnabled( Logger::SENSOR ) )
    {
        for ( int i = 0; i < 2; ++i )
        {
            dlog.addText( Logger::SENSOR,
                          "TwoPlayerMessage. success!. unum=%d pos=(%f %f)",
                          M_player_unum[i],
                          M_player_pos[i].x, M_player_pos[i].y );
        }

        dlog.addText( Logger::SENSOR,
                      "--> [%s]", msg.c_str() );
    }

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TwoPlayerMessage::printDebug( std::ostream & os ) const
{
    //     os << "[2Player "
    //        << '(' << ( M_player_unum[0] > 11 ? 11 - M_player_unum[0] : M_player_unum[0] ) << ' '
    //        << '(' << round( M_player_pos[0].x, 0.1 ) << ',' << round( M_player_pos[0].y, 0.1 ) << "))"
    //        << '(' << ( M_player_unum[1] > 11 ? 11 - M_player_unum[1] : M_player_unum[1] ) << ' '
    //        << '(' << round( M_player_pos[1].x, 0.1 ) << ',' << round( M_player_pos[1].y, 0.1 ) << "))"
    //        << ']';
    os << "[2Player:"
       << ( M_player_unum[0] > 11 ? "O_" : "T_" )
       << ( M_player_unum[0] > 11 ? M_player_unum[0] - 11 : M_player_unum[0] ) << '|'
       << ( M_player_unum[1] > 11 ? "O_" : "T_" )
       << ( M_player_unum[1] > 11 ? M_player_unum[1] - 11 : M_player_unum[1] ) << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
ThreePlayerMessage::ThreePlayerMessage( const int player0_unum,
                                        const Vector2D & player0_pos,
                                        const int player1_unum,
                                        const Vector2D & player1_pos,
                                        const int player2_unum,
                                        const Vector2D & player2_pos )
{
    M_player_unum[0] = player0_unum;
    M_player_pos[0] = player0_pos;

    M_player_unum[1] = player1_unum;
    M_player_pos[1] = player1_pos;

    M_player_unum[2] = player2_unum;
    M_player_pos[2] = player2_pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ThreePlayerMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "ThreePlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    std::int64_t ival = 0;
    double dval = 0.0;

    for ( int i = 0; i < 3; ++i )
    {
        if ( M_player_unum[i] < 1 || 22 < M_player_unum[i] )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " ***ERROR*** ThreePlayerMessage. illegal unum = "
                      << M_player_unum[i]
                      << std::endl;
            dlog.addText( Logger::SENSOR,
                          "ThreePlayerMessage. illegal unum = %d",
                          M_player_unum[i] );
            return false;
        }

        ival *= 22;
        ival += M_player_unum[i] - 1;

        dval = min_max( -52.49, M_player_pos[i].x, 52.49 ) + 52.5;
        ival *= 168;
        ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.63 ), 167.0 ) );

        dval = min_max( -33.99, M_player_pos[i].y, 33.99 ) + 34.0;
        ival *= 109;
        ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.63 ), 108.0 ) );
    }

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** ThreePlayerMessage. "
                  << std::endl;

        for ( int i = 0; i < 3; ++i )
        {
            dlog.addText( Logger::SENSOR,
                          "ThreePlayerMessage. error! unum=%d pos=(%.2f %.2f)",
                          M_player_unum[i],
                          M_player_pos[i].x, M_player_pos[i].y );
        }
        return false;
    }

    if ( dlog.isEnabled( Logger::SENSOR ) )
    {
        for ( int i = 0; i < 3; ++i )
        {
            dlog.addText( Logger::SENSOR,
                          "ThreePlayerMessage. success!. unum=%d pos=(%.2f %.2f)",
                          M_player_unum[i],
                          M_player_pos[i].x, M_player_pos[i].y );
        }

        dlog.addText( Logger::SENSOR,
                      "--> [%s]", msg.c_str() );
    }

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ThreePlayerMessage::printDebug( std::ostream & os ) const
{
    //     os << "[3Player "
    //        << '(' << ( M_player_unum[0] > 11 ? 11 - M_player_unum[0] : M_player_unum[0] ) << ' '
    //        << '(' << round( M_player_pos[0].x, 0.1 ) << ',' << round( M_player_pos[0].y, 0.1 ) << "))"
    //        << '(' << ( M_player_unum[1] > 11 ? 11 - M_player_unum[1] : M_player_unum[1] ) << ' '
    //        << '(' << round( M_player_pos[1].x, 0.1 ) << ',' << round( M_player_pos[1].y, 0.1 ) << "))"
    //        << '(' << ( M_player_unum[2] > 11 ? 11 - M_player_unum[2] : M_player_unum[2] ) << ' '
    //        << '(' << round( M_player_pos[2].x, 0.1 ) << ',' << round( M_player_pos[2].y, 0.1 ) << "))"
    //        << ']';
    os << "[3Player:"
       << ( M_player_unum[0] > 11 ? "O_" : "T_" )
       << ( M_player_unum[0] > 11 ? M_player_unum[0] - 11 : M_player_unum[0] ) << '|'
       << ( M_player_unum[1] > 11 ? "O_" : "T_" )
       << ( M_player_unum[1] > 11 ? M_player_unum[1] - 11 : M_player_unum[1] ) << '|'
       << ( M_player_unum[2] > 11 ? "O_" : "T_" )
       << ( M_player_unum[2] > 11 ? M_player_unum[2] - 11 : M_player_unum[2] ) << ']';

    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
SelfMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "SelfMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    // 74^4 = 29986576
    // 264 * 171 * 60 * 11 = 29795040


    std::int64_t ival = 0;
    double dval = 0.0;

    dval = min_max( -52.5, M_self_pos.x, 52.5 ) + 52.5;
    //ival *= 264; // 105.0/0.4=262.5
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.4 ), 263.0 ) );

    dval = min_max( -34.0, M_self_pos.y, 34.0 ) + 34.0;
    ival *= 171; // = 68/0.4
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.4 ), 170.0 ) );

    dval = M_self_body.degree() + 180.0;
    ival *= 60; // = 360/6
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 6.0 ), 59.0 ) );

    dval = min_max( 0.0, M_self_stamina / ServerParam::i().staminaMax(), 1.0 );
    ival *= 11;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval * 10.0 ), 10.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** SelfMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "SelfMessage. error!"
                      " pos=(%f %f) body=%f stamina=%f",
                      M_self_pos.x, M_self_pos.y,
                      M_self_body.degree(),
                      M_self_stamina );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "SelfMessage. success!."
                  " pos=(%f %f)"
                  " body=%f stamina_rate=%f-> [%s]",
                  M_self_pos.x, M_self_pos.y,
                  M_self_body.degree(),
                  M_self_stamina,
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
SelfMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Self "
    //        << '(' << round( M_self_pos.x, 0.1 ) << ',' << round( M_self_pos.y, 0.1 ) << ')'
    //        << ']';
    os << "[Self]";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
TeammateMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "TeammateMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_unum < 1 || 11 < M_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** TeammateMessage. illegal unum = "
                  << M_unum
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "TeammateMessage. illegal unum = %d",
                      M_unum );
        return false;
    }

    std::int64_t ival = 0;
    double dval = 0.0;

    ival += M_unum - 1;

    dval = bound( -52.49, M_player_pos.x, 52.49 ) + 52.5;
    ival *= 151;
    ival += static_cast< std::int64_t >( rint( dval / 0.7 ) );

    dval = bound( -33.99, M_player_pos.y, 33.99 ) + 34.0;
    ival *= 98;
    ival += static_cast< std::int64_t >( rint( dval / 0.7 ) );

    dval = bound( 0.0, M_player_body.degree() + 180.0, 358.9 );
    ival *= 180; // = 360/2
    ival += static_cast< std::int64_t >( rint( dval / 2.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** TeammateMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "TeammateMessage. error!"
                      " unum=%d pos=(%f %f) body=%f",
                      M_unum,
                      M_player_pos.x, M_player_pos.y,
                      M_player_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "TeammateMessage. success!. unum = %d pos=(%f %f)"
                  " body=%f -> [%s]",
                  M_unum,
                  M_player_pos.x, M_player_pos.y,
                  M_player_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
TeammateMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Teammate "
    //        << M_unum << ' '
    //        << '(' << round( M_player_pos.x, 0.1 ) << ',' << round( M_player_pos.y, 0.1 ) << ')'
    //        << ']';
    os << "[Teammate:" << M_unum << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OpponentMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "OpponentMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_unum < 1 || 11 < M_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OpponentMessage. illegal unum = "
                  << M_unum
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "OpponentMessage. illegal unum = %d",
                      M_unum );
        return false;
    }

    std::int64_t ival = 0;
    double dval = 0.0;

    ival += M_unum - 1;

    dval = bound( -52.49, M_player_pos.x, 52.49 ) + 52.5;
    ival *= 151;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.7 ), 150.0 ) );

    dval = bound( -33.99, M_player_pos.y, 33.99 ) + 34.0;
    ival *= 98;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 0.7 ), 99.0 ) );

    dval = bound( 0.0, M_player_body.degree() + 180.0, 358.9 );
    ival *= 180; // = 360/2
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 2.0 ), 179.0 ) );

    std::string msg;
    msg.reserve( slength() - 1 );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, slength() - 1, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** OpponentMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "OpponentMessage. error!"
                      " unum=%d pos=(%f %f) body=%f",
                      M_unum,
                      M_player_pos.x, M_player_pos.y,
                      M_player_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "OpponentMessage. success!. unum = %d pos=(%f %f)"
                  " body=%f -> [%s]",
                  M_unum,
                  M_player_pos.x, M_player_pos.y,
                  M_player_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
OpponentMessage::printDebug( std::ostream & os ) const
{
    //     os << "[Opponent "
    //        << M_unum << ' '
    //        << '(' << round( M_player_pos.x, 0.1 ) << ',' << round( M_player_pos.y, 0.1 ) << ')'
    //        << ']';
    os << "[Opponent:" << M_unum << ']';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
BallPlayerMessage::appendTo( std::string & to ) const
{
    if ( (int)to.length() + slength() > ServerParam::i().playerSayMsgSize() )
    {
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessage. over the message size : buf = %d, this = %d",
                      to.length(), slength() );
        return false;
    }

    if ( M_unum < 1 || 22 < M_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallPlayerMessage. illegal unum = "
                  << M_unum
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessage. illegal unum = %d",
                      M_unum );
        return false;
    }

    std::string msg;
    msg.reserve( slength() - 1 );


    //
    // ball info (5 characters)
    //
    if ( ! AudioCodec::i().encodePosVelToStr5( M_ball_pos,
                                               M_ball_vel,
                                               msg )
         || msg.length() != 5 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallPlayerMessage. "
                  << std::endl;
        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessage. ball encode error!"
                      " pos=(%f %f) vel=(%f %f)",
                      M_ball_pos.x, M_ball_pos.y,
                      M_ball_vel.x, M_ball_vel.y );
        return false;
    }


    //
    // player info (4 characters)
    //   74^4 = 29986576
    //   22 * 106 * 69 * 180 = 28963400
    //

    std::int64_t ival = 0;
    double dval = 0.0;

    ival += M_unum - 1;

    dval = bound( -52.49, M_player_pos.x, 52.49 ) + 52.5;
    ival *= 106;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval ), 105.0 ) );

    dval = bound( -33.99, M_player_pos.y, 33.99 ) + 34.0;
    ival *= 69;
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval ), 68.0 ) );

    dval = bound( 0.0, M_player_body.degree() + 180.0, 359.0 );
    ival *= 180; // = 360/2
    ival += static_cast< std::int64_t >( bound( 0.0, rint( dval / 2.0 ), 179.0 ) );

    if ( ! AudioCodec::i().encodeInt64ToStr( ival, 4, msg )
         || (int)msg.length() != slength() - 1 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " ***ERROR*** BallPlayerMessage. "
                  << std::endl;

        dlog.addText( Logger::SENSOR,
                      "BallPlayerMessage. player encode error!"
                      " unum=%d pos=(%f %f) body=%f",
                      M_unum,
                      M_player_pos.x, M_player_pos.y,
                      M_player_body.degree() );
        return false;
    }

    dlog.addText( Logger::SENSOR,
                  "BallPlayerMessage. success!."
                  " bpos(%f %f) bvel(%f %f)"
                  " unum=%d ppos(%f %f) pbody=%f"
                  " -> [%s]",
                  M_ball_pos.x, M_ball_pos.y,
                  M_ball_vel.x, M_ball_vel.y,
                  M_unum,
                  M_player_pos.x, M_player_pos.y,
                  M_player_body.degree(),
                  msg.c_str() );

    to += header();
    to += msg;

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
BallPlayerMessage::printDebug( std::ostream & os ) const
{
    //     os << "[BallPlayer "
    //        << "B(" << round( M_ball_pos.x, 0.1 ) << ',' << round( M_ball_pos.y, 0.1 ) << ')'
    //        << " (" << M_unum << ' '
    //        << '(' << round( M_player_pos.x, 0.1 ) << ',' << round( M_player_pos.y, 0.1 ) << "))"
    //        << ']';
    os << "[BallPlayer:"
       << ( M_unum > 11 ? "O_" : "T_" )
       << ( M_unum > 11 ? M_unum - 11 : M_unum ) << ']';
    return os;
}

}
