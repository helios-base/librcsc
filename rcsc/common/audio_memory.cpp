// -*-c++-*-

/*!
  \file audio_memory.cpp
  \brief communication message data holder Source File
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

#include "audio_memory.h"

#include <rcsc/common/logger.h>
#include <rcsc/types.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
AudioMemory::AudioMemory()
    : M_time( -1, 0 ),
      M_ball_time( -1, 0 ),
      M_pass_time( -1, 0 ),
      M_our_intercept_time( -1, 0 ),
      M_opp_intercept_time( -1, 0 ),
      M_goalie_time( -1, 0 ),
      M_player_time( -1, 0 ),
      M_offside_line_time( -1, 0 ),
      M_defense_line_time( -1, 0 ),
      M_wait_request_time( -1, 0 ),
      M_setplay_time( -1, 0 ),
      M_pass_request_time( -1, 0 ),
      M_run_request_time( -1, 0 ),
      M_stamina_time( -1, 0 ),
      M_recovery_time( -1, 0 ),
      M_dribble_time( -1, 0 ),
      M_free_message_time( -1, 0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setBall( const int sender,
                      const Vector2D & pos,
                      const Vector2D & vel,
                      const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard ball: sender=%d "
                  "pos=(%.3f, %.3f) vel=(%.2f, %.2f)",
                  sender,
                  pos.x, pos.y, vel.x, vel.y );

    if ( M_ball_time != current )
    {
        M_ball.clear();
    }

    M_ball.emplace_back( sender, pos, vel );
    M_ball_time = current;

    M_time = current;
}


/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setPass( const int sender,
                      const int receiver,
                      const Vector2D & pos,
                      const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard pass: sender=%d "
                  "receiver=%d, pos=(%.2f, %.2f)",
                  sender,
                  receiver, pos.x, pos.y );

    if ( M_pass_time != current )
    {
        M_pass.clear();
    }

    M_pass.emplace_back( sender, receiver, pos );
    M_pass_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setIntercept( const int sender,
                           const int interceptor,
                           const int cycle,
                           const GameTime & current )
{
    if ( interceptor <= MAX_PLAYER )
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": set heard teammate intercept: sender=%d "
                      "unum=%d cycle=%d",
                      sender,
                      interceptor, cycle );

        if ( M_our_intercept_time != current )
        {
            M_our_intercept.clear();
        }

        // -1 because the heard value was estimated in the previous cycle
        M_our_intercept.emplace_back( sender, interceptor, std::max( 0, cycle - 1 ) );
        M_our_intercept_time = current;
    }
    else
    {
        dlog.addText( Logger::WORLD,
                      __FILE__": set heard opponent intercept: sender=%d "
                      "unum=%d cycle=%d",
                      sender,
                      interceptor, cycle );

        if ( M_opp_intercept_time != current )
        {
            M_opp_intercept.clear();
        }

        // -1 because the heard value was estimated in the previous cycle
        M_opp_intercept.emplace_back( sender, interceptor - MAX_PLAYER, std::max( 0, cycle - 1 ) );
        M_opp_intercept_time = current;
    }

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setOpponentGoalie( const int sender,
                                const Vector2D & pos,
                                const AngleDeg & body,
                                const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard goalie: sender=%d "
                  "pos=(%.2f, %.2f) body=%.1f",
                  sender,
                  pos.x, pos.y, body.degree() );

    if ( M_goalie_time != current )
    {
        M_goalie.clear();
    }

    M_goalie.emplace_back( sender, pos, body );
    M_goalie_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setPlayer( const int sender,
                        const int unum,
                        const Vector2D & pos,
                        const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard player. sender=%d "
                  "unum=%d pos=(%.2f, %.2f) no body",
                  sender,
                  unum, pos.x, pos.y );
    if ( M_player_time != current )
    {
        M_player.clear();
    }

    M_player.emplace_back( sender, unum, pos );
    M_player_time = current;

    M_time = current;

    M_player_record.push_back( std::pair< GameTime, Player >( current, M_player.back() ) );
    while ( M_player_record.size() > 30 )
    {
        M_player_record.pop_front();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setPlayer( const int sender,
                        const int unum,
                        const Vector2D & pos,
                        const double & body,
                        const double & stamina,
                        const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard player. sender=%d "
                  "unum=%d pos=(%.2f, %.2f) body=%.1f",
                  sender,
                  unum, pos.x, pos.y, body );
    if ( M_player_time != current )
    {
        M_player.clear();
    }

    M_player.emplace_back( sender, unum, pos, body, stamina );
    M_player_time = current;

    M_time = current;


    M_player_record.push_back( std::pair< GameTime, Player >( current, M_player.back() ) );
    while ( M_player_record.size() > 30 )
    {
        M_player_record.pop_front();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setOffsideLine( const int sender,
                             const double & offside_line_x,
                             const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard offside line. sender=%d x=%.1f",
                  sender, offside_line_x );

    if ( M_offside_line_time != current )
    {
        M_offside_line.clear();
    }

    M_offside_line.emplace_back( sender, offside_line_x );
    M_offside_line_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setDefenseLine( const int sender,
                             const double & defense_line_x,
                             const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard defense line. sender=%d x=%.1f",
                  sender, defense_line_x );

    if ( M_defense_line_time != current )
    {
        M_defense_line.clear();
    }

    M_defense_line.emplace_back( sender, defense_line_x );
    M_defense_line_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setWaitRequest( const int sender,
                             const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard wait request. sender=%d",
                  sender );

    if ( M_wait_request_time != current )
    {
        M_wait_request.clear();
    }

    M_wait_request.emplace_back( sender );
    M_wait_request_time = current;

    M_time = current;
}



/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setSetplay( const int sender,
                         const int wait_step,
                         const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard setplay. sender=%d wait_step=%d",
                  sender, wait_step );

    if ( M_setplay_time != current )
    {
        M_setplay.clear();
    }

    M_setplay.emplace_back( sender, wait_step );
    M_setplay_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setPassRequest( const int sender,
                             const Vector2D & request_pos,
                             const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard pass request. sender=%d pos=(%.1f %.1f)",
                  sender, request_pos.x, request_pos.y );

    if ( M_pass_request_time != current )
    {
        M_pass_request.clear();
    }

    M_pass_request.emplace_back( sender, request_pos );
    M_pass_request_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setRunRequest( const int sender,
                            const int runner,
                            const Vector2D & request_pos,
                            const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard run request."
                  " sender=%d runner=%d pos=(%.1f %.1f)",
                  sender, runner, request_pos.x, request_pos.y );

    if ( M_run_request_time != current )
    {
        M_run_request.clear();
    }

    M_run_request.emplace_back( sender, runner, request_pos );
    M_run_request_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setStamina( const int sender,
                         const double & rate,
                         const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard stamina. sender=%d rate=%.3f",
                  sender, rate );

    if ( M_stamina_time != current )
    {
        M_stamina.clear();
    }

    M_stamina.emplace_back( sender, rate );
    M_stamina_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setRecovery( const int sender,
                          const double & rate,
                          const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard recovery. sender=%d rate=%.3f",
                  sender, rate );

    if ( M_recovery_time != current )
    {
        M_recovery.clear();
    }

    M_recovery.emplace_back( sender, rate );
    M_recovery_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setStaminaCapacity( const int sender,
                                 const double & rate,
                                 const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard stamina capacity. sender=%d rate=%.3f",
                  sender, rate );

    if ( M_stamina_capacity_time != current )
    {
        M_stamina_capacity.clear();
    }

    M_stamina_capacity.emplace_back( sender, rate );
    M_stamina_capacity_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setDribbleTarget( const int sender,
                               const Vector2D & pos,
                               const int queue_count,
                               const GameTime & current )
{
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard dribble target. sender=%d"
                  " target=(%.1f %.1f) count=%d",
                  sender,
                  pos.x, pos.y,
                  queue_count );

    if ( M_dribble_time != current )
    {
        M_dribble.clear();
    }

    M_dribble.emplace_back( sender, pos, queue_count );
    M_dribble_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
AudioMemory::setFreeMessage( const int sender,
                             const std::string & msg,
                             const GameTime & current )
{
    dlog.addText( Logger::SENSOR,
                  "FreeMessageParser::parse() success! length=%d",
                  msg.length() );
    dlog.addText( Logger::WORLD,
                  __FILE__": set heard free message. sender=%d"
                  " message=[%s]",
                  sender, msg.c_str() );

    if ( M_free_message_time != current )
    {
        M_free_message.clear();
    }

    M_free_message.emplace_back( sender, msg );
    M_free_message_time = current;

    M_time = current;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
AudioMemory::printDebug( std::ostream & os ) const
{
    //
    // TODO: reimplemented using virtual method.
    //

    if ( time() == ballTime() )
    {
        for ( const Ball & b : ball() )
        {
            os << '(' << b.sender_ << " \"Ball\")";
        }
    }

    if ( time() == passTime() )
    {
        for ( const Pass & p : pass() )
        {
            os << '(' << p.sender_ << " \"Pass\")";
        }
    }

    if ( time() == ourInterceptTime() )
    {
        for ( const OurIntercept & i : ourIntercept() )
        {
            os << '(' << i.sender_ << " \"OurIntercept:" << i.interceptor_ << "\")";
        }
    }

    if ( time() == oppInterceptTime() )
    {
        for ( const OppIntercept & i : oppIntercept() )
        {
            os << '(' << i.sender_ << " \"OppIntercept:" << i.interceptor_ << "\")";
        }
    }

    if ( time() == goalieTime() )
    {
        for ( const Goalie & g : goalie() )
        {
            os << '(' << g.sender_ << " \"Goalie\")";
        }
    }

    if ( time() == playerTime() )
    {
        for ( const Player & p : player() )
        {
            os << '(' << p.sender_;
            if ( p.unum_ <= 11 )
            {
                os << " \"T_" << p.unum_;
            }
            else
            {
                os << " \"O_" << p.unum_ - 11;
            }
            os << "\")";
        }
    }

    if ( time() == offsideLineTime() )
    {
        for ( const OffsideLine & v : offsideLine() )
        {
            os << '(' << v.sender_ << " \"OffsideLine\")";
        }
    }

    if ( time() == defenseLineTime() )
    {
        for ( const DefenseLine & v : defenseLine() )
        {
            os << '(' << v.sender_ << " \"DefenseLine\")";
        }
    }

    if ( time() == waitRequestTime() )
    {
        for ( const WaitRequest & v : waitRequest() )
        {
            os << '(' << v.sender_ << " \"Wait\")";
        }
    }

    if ( time() == passRequestTime() )
    {
        for ( const PassRequest & v : passRequest() )
        {
            os << '(' << v.sender_ << " \"PassRequest\")";
        }
    }

    if ( time() == runRequestTime() )
    {
        for ( const RunRequest & v : runRequest() )
        {
            os << '(' << v.sender_ << " \"RunRequest\")";
        }
    }

    if ( time() == staminaTime() )
    {
        for ( const Stamina &v : stamina() )
        {
            os << '(' << v.sender_ << " \"Stamina\")";
        }
    }

    if ( time() == recoveryTime() )
    {
        for ( const Recovery & v : recovery() )
        {
            os << '(' << v.sender_ << " \"Recovery\")";
        }
    }

    if ( time() == dribbleTime() )
    {
        for ( const Dribble & v : dribble() )
        {
            os << '(' << v.sender_ << " \"Dribble\")";
        }
    }

    if ( time() == freeMessageTime() )
    {
        for ( const FreeMessage & v : freeMessage() )
        {
            os << '(' << v.sender_ << " \"FreeMessage\")";
        }
    }

    return os;
}

}
