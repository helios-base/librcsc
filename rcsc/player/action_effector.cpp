// -*-c++-*-

/*!
  \file action_effector.cpp
  \brief Effector Source File
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

#include "action_effector.h"

#include "player_agent.h"
#include "world_model.h"
#include "body_sensor.h"
#include "player_command.h"
#include "say_message_builder.h"
#include "see_state.h"

#include <rcsc/math_util.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
ActionEffector::ActionEffector( const PlayerAgent & agent )
    : M_agent( agent ),
      M_command_body( nullptr ),
      M_command_turn_neck( nullptr ),
      M_command_change_view( nullptr ),
      M_command_change_focus( nullptr ),
      M_command_say( nullptr ),
      M_command_pointto( nullptr ),
      M_command_attentionto( nullptr ),
      M_last_action_time( 0, 0 ),
      M_done_turn_neck( false ),
      M_kick_accel( 0.0, 0.0 ),
      M_kick_accel_error( 0.0, 0.0 ),
      M_turn_actual( 0.0 ),
      M_turn_error( 0.0 ),
      M_dash_accel( 0.0, 0.0 ),
      // M_dash_accel_error(0.0, 0.0),
      M_dash_power( 0.0 ),
      M_move_pos( 0.0, 0.0 ),
      M_catch_time( 0, 0 ),
      M_tackle_power( 0.0 ),
      M_tackle_dir( 0.0 ),
      M_tackle_foul( false ),
      M_turn_neck_moment( 0.0 ),
      M_say_message( "" ),
      M_pointto_pos( 0.0, 0.0 )
{
    for ( int i = 0; i < 2; ++i )
    {
        M_last_body_command_type[i] = PlayerCommand::ILLEGAL;
    }

    for ( int i = PlayerCommand::INIT;
          i <= PlayerCommand::ILLEGAL;
          ++i )
    {
        M_command_counter[i] = 0;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
ActionEffector::~ActionEffector()
{
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }

    if ( M_command_turn_neck )
    {
        delete M_command_turn_neck;
        M_command_turn_neck = nullptr;
    }

    if ( M_command_change_view )
    {
        delete M_command_change_view;
        M_command_change_view = nullptr;
    }

    if ( M_command_change_focus )
    {
        delete M_command_change_focus;
        M_command_change_focus = nullptr;
    }

    if ( M_command_say )
    {
        delete M_command_say;
        M_command_say = nullptr;
    }

    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = nullptr;
    }

    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = nullptr;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::reset()
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  CAUTION: this function must be called after WorldModel::update()
    //             and before action decision.
    //  Do NOT update change_viwe info for adjustment of see arrival timing

    for ( int i = 0; i < 2; ++i )
    {
        M_last_body_command_type[i] = PlayerCommand::ILLEGAL;
    }

    M_done_turn_neck = false;
    M_say_message.erase();

    // it is not necesarry to reset these value,
    // because value is selected by last command type specifier in updator function.

    //M_kick_accel.assign(0.0, 0.0);
    //M_kick_accel_error.assign(0.0, 0.0);
    //M_turn_actual =  M_turn_error = 0.0;
    //M_dash_accel.assign(0.0, 0.0);
    //M_dash_accel_error.assign(0.0, 0.0);
    //M_dash_power = 0.0;
    //M_move_pos.assign(0.0, 0.0);
    //M_catch_time
    //M_tackle_power = 0.0;
    //M_tackle_dir = 0.0;
    //M_turnneck_moment = 0.0;
    //M_pointto_pos.assign(0.0, 0.0);
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::incCommandCount( const PlayerCommand::Type type )
{
    if ( type < PlayerCommand::INIT
         || PlayerCommand::ILLEGAL <= type )
    {
        std::cerr << "ActionEffector::incCommandCount()"
                  << "  illegal command type ID "
                  << type
                  << std::endl;
        return;
    }

    M_command_counter[type] += 1;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::checkCommandCount( const BodySensor & sense )
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //  CAUTION: this function must be called after catching sense_body.

    // if action count does not match the internal model,
    //  reset that action effect.

    if ( sense.kickCount() != M_command_counter[PlayerCommand::KICK] )
    {
        if ( sense.chargedExpires() == 0 )
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost kick? at " << M_last_action_time
                      << " sense=" << sense.kickCount()
                      << " internal=" << M_command_counter[PlayerCommand::KICK]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost kick? sense= %d internal= %d",
                          sense.kickCount(),
                          M_command_counter[PlayerCommand::KICK] );
        }
        else
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost kick by foul at " << M_last_action_time
                      << " sense=" << sense.kickCount()
                      << " internal=" << M_command_counter[PlayerCommand::KICK]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost kick by foul sense= %d internal= %d",
                          sense.kickCount(),
                          M_command_counter[PlayerCommand::KICK] );
        }
        M_last_body_command_type[0] = PlayerCommand::ILLEGAL;
        M_kick_accel.assign( 0.0, 0.0 );
        M_kick_accel_error.assign( 0.0, 0.0 );
        M_command_counter[PlayerCommand::KICK] = sense.kickCount();
    }

    if ( sense.turnCount() != M_command_counter[PlayerCommand::TURN] )
    {
        if ( sense.chargedExpires() == 0 )
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost turn? at " << M_last_action_time
                      << " sense=" << sense.turnCount()
                      << " internal=" << M_command_counter[PlayerCommand::TURN]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost turn? sense= %d internal= %d",
                          sense.turnCount(),
                          M_command_counter[PlayerCommand::TURN] );
        }
        M_last_body_command_type[0] = PlayerCommand::ILLEGAL;
        M_turn_actual = 0.0;
        M_turn_error = 0.0;
        M_command_counter[PlayerCommand::TURN] = sense.turnCount();
    }

    if ( sense.dashCount() != M_command_counter[PlayerCommand::DASH] )
    {
        if ( sense.chargedExpires() == 0 )
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost dash? at " << M_last_action_time
                      << " sense=" << sense.dashCount()
                      << " internal=" << M_command_counter[PlayerCommand::DASH]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost dash? sense= %d internal= %d",
                          sense.dashCount(),
                          M_command_counter[PlayerCommand::DASH] );
        }
        M_last_body_command_type[0] = PlayerCommand::ILLEGAL;
        M_dash_accel.assign( 0.0, 0.0 );
        //M_dash_accel_error.assign( 0.0, 0.0 );
        M_dash_power = 0.0;
        M_command_counter[PlayerCommand::DASH] = sense.dashCount();
    }

    if ( sense.moveCount() != M_command_counter[PlayerCommand::MOVE] )
    {
        if ( sense.chargedExpires() == 0 )
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost move? at " << M_last_action_time
                      << " sense=" << sense.moveCount()
                      << " internal=" << M_command_counter[PlayerCommand::MOVE]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost move? sense= %d internal= %d",
                          sense.moveCount(),
                          M_command_counter[PlayerCommand::MOVE] );
        }
        M_last_body_command_type[0] = PlayerCommand::ILLEGAL;
        M_move_pos.invalidate();
        M_command_counter[PlayerCommand::MOVE] = sense.moveCount();
    }

    if ( sense.catchCount() != M_command_counter[PlayerCommand::CATCH] )
    {
        if ( sense.chargedExpires() == 0 )
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost catch? at " << M_last_action_time
                      << " sense=" << sense.catchCount()
                      << " internal=" << M_command_counter[PlayerCommand::CATCH]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost catch? sense= %d internal= %d",
                          sense.catchCount(),
                          M_command_counter[PlayerCommand::CATCH] );
        }
        M_last_body_command_type[0] = PlayerCommand::ILLEGAL;
        //M_catch_time.assign( 0, 0 ); // Do *NOT* reset the time
        M_command_counter[PlayerCommand::CATCH] = sense.catchCount();
    }

    if ( sense.tackleCount() != M_command_counter[PlayerCommand::TACKLE] )
    {
        if ( sense.chargedExpires() == 0 )
        {
            std::cout << M_agent.config().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " lost tackle? at " << M_last_action_time
                      << " sense=" << sense.tackleCount()
                      << " internal=" << M_command_counter[PlayerCommand::TACKLE]
                      << std::endl;
            dlog.addText( Logger::SYSTEM,
                          __FILE__": lost tackle? sense= %d internal= %d",
                          sense.tackleCount(),
                          M_command_counter[PlayerCommand::TACKLE] );
        }
        M_last_body_command_type[0] = PlayerCommand::ILLEGAL;
        M_tackle_power = 0.0;
        M_tackle_dir = 0.0;
        M_tackle_foul = false;
        M_command_counter[PlayerCommand::TACKLE] = sense.tackleCount();
    }

    if ( sense.turnNeckCount() != M_command_counter[PlayerCommand::TURN_NECK] )
    {
        std::cout << M_agent.config().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " lost turn_neck? at " << M_last_action_time
                  << " sense=" << sense.turnNeckCount()
                  << " internal=" << M_command_counter[PlayerCommand::TURN_NECK]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                       __FILE__": lost turn_neck? sense= %d internal= %d",
                      sense.turnNeckCount(),
                      M_command_counter[PlayerCommand::TURN_NECK] );
        M_done_turn_neck = false;
        M_turn_neck_moment = 0.0;
        M_command_counter[PlayerCommand::TURN_NECK] = sense.turnNeckCount();
    }

    if ( sense.changeViewCount() != M_command_counter[PlayerCommand::CHANGE_VIEW] )
    {
        std::cout << M_agent.config().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " lost change_view? at " << M_last_action_time
                  << " sense=" << sense.changeViewCount()
                  << " internal=" << M_command_counter[PlayerCommand::CHANGE_VIEW]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                       __FILE__": lost change_view? sense= %d internal= %d",
                      sense.changeViewCount(),
                      M_command_counter[PlayerCommand::CHANGE_VIEW] );
        M_command_counter[PlayerCommand::CHANGE_VIEW] = sense.changeViewCount();
    }

    if ( sense.changeFocusCount() != M_command_counter[PlayerCommand::CHANGE_FOCUS] )
    {
        std::cout << M_agent.config().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " lost change_focus? at " << M_last_action_time
                  << " sense=" << sense.changeViewCount()
                  << " internal=" << M_command_counter[PlayerCommand::CHANGE_FOCUS]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                       __FILE__": lost change_focus? sense= %d internal= %d",
                      sense.changeFocusCount(),
                      M_command_counter[PlayerCommand::CHANGE_FOCUS] );
        M_command_counter[PlayerCommand::CHANGE_FOCUS] = sense.changeFocusCount();
    }

    if ( sense.sayCount() != M_command_counter[PlayerCommand::SAY] )
    {
        std::cout << M_agent.config().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " lost say? at " << M_last_action_time
                  << " sense=" << sense.sayCount()
                  << " internal=" << M_command_counter[PlayerCommand::SAY]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                       __FILE__": lost say? sense= %d internal= %d",
                      sense.sayCount(),
                      M_command_counter[PlayerCommand::SAY] );
        M_command_counter[PlayerCommand::SAY] = sense.sayCount();
    }

    if ( sense.pointtoCount() != M_command_counter[PlayerCommand::POINTTO] )
    {
        std::cout << M_agent.config().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " lost pointto? at " << M_last_action_time
                  << " sense=" << sense.pointtoCount()
                  << " internal=" << M_command_counter[PlayerCommand::POINTTO]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                       __FILE__": lost pointto? sense= %d internal= %d",
                      sense.pointtoCount(),
                      M_command_counter[PlayerCommand::POINTTO] );
        M_command_counter[PlayerCommand::POINTTO] = sense.pointtoCount();
    }

    if ( sense.attentiontoCount() != M_command_counter[PlayerCommand::ATTENTIONTO] )
    {
        std::cout << M_agent.config().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " lost attentionto? at " << M_last_action_time
                  << " sense=" << sense.attentiontoCount()
                  << " internal=" << M_command_counter[PlayerCommand::ATTENTIONTO]
                  << std::endl;
        dlog.addText( Logger::SYSTEM,
                       __FILE__": lost attentionto? sense= %d internal= %d",
                      sense.attentiontoCount(),
                      M_command_counter[PlayerCommand::ATTENTIONTO] );
        M_command_counter[PlayerCommand::ATTENTIONTO] = sense.attentiontoCount();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
ActionEffector::makeCommand( std::ostream & to )
{
    M_last_body_command_type[1] = M_last_body_command_type[0];

    M_last_action_time = M_agent.world().time();

    if ( M_command_body )
    {
        M_last_body_command_type[0] = M_command_body->type();
        if ( M_last_body_command_type[0] == PlayerCommand::CATCH )
        {
            M_catch_time = M_agent.world().time();
        }
        M_command_body->toCommandString( to );
        incCommandCount( M_command_body->type() );
        delete M_command_body;
        M_command_body = nullptr;
    }
    else
    {
        if ( ! M_agent.world().self().isFrozen() )
        {
            dlog.addText( Logger::SYSTEM,
                           __FILE__": WARNING. no body command." );
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum()<< ": "
                      << M_agent.world().time()
                      << "  WARNING. no body command." << std::endl;
            // register dummy command
            PlayerTurnCommand turn( 0 );
            turn.toCommandString( to );
            incCommandCount( PlayerCommand::TURN );
        }
    }

    if ( M_command_turn_neck )
    {
        M_done_turn_neck = true;
        M_command_turn_neck->toCommandString( to );
        incCommandCount( PlayerCommand::TURN_NECK );
        delete M_command_turn_neck;
        M_command_turn_neck = nullptr;
    }

    if ( M_command_change_view )
    {
        M_command_change_view->toCommandString( to );
        incCommandCount( PlayerCommand::CHANGE_VIEW );
        delete M_command_change_view;
        M_command_change_view = nullptr;
    }

    if ( M_command_change_focus )
    {
        M_command_change_focus->toCommandString( to );
        incCommandCount( PlayerCommand::CHANGE_FOCUS );
        delete M_command_change_focus;
        M_command_change_focus = nullptr;
    }

    if ( M_command_pointto )
    {
        M_command_pointto->toCommandString( to );
        incCommandCount( PlayerCommand::POINTTO );
        delete M_command_pointto;
        M_command_pointto = nullptr;
    }

    if ( M_command_attentionto )
    {
        M_command_attentionto->toCommandString( to );
        incCommandCount( PlayerCommand::ATTENTIONTO );
        delete M_command_attentionto;
        M_command_attentionto = nullptr;
    }

    if ( ServerParam::i().synchMode() )
    {
        PlayerDoneCommand done_com;
        done_com.toCommandString( to );
    }

    makeSayCommand();
    if ( M_command_say )
    {
        M_command_say->toCommandString( to );
        incCommandCount( PlayerCommand::SAY );
    }

    return to;
}


/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::clearAllCommands()
{
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }

    if ( M_command_turn_neck )
    {
        delete M_command_turn_neck;
        M_command_turn_neck = nullptr;
    }

    if ( M_command_change_view )
    {
        delete M_command_change_view;
        M_command_change_view = nullptr;
    }

    if ( M_command_change_focus )
    {
        delete M_command_change_focus;
        M_command_change_focus = nullptr;
    }

    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = nullptr;
    }

    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = nullptr;
    }

    if ( M_command_say )
    {
        delete M_command_say;
        M_command_say = nullptr;
    }

    M_say_message_cont.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setKick( const double & power,
                         const AngleDeg & rel_dir )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setKick) register kick. power= %.1f, rel_dir= %.1f",
                  power, rel_dir.degree() );

    double command_power = power;
    if ( command_power > ServerParam::i().maxPower() + 0.01 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " kick power is over max. com=" << command_power
                  << " > sparam=" << ServerParam::i().maxPower() << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__": (setKick) power over. %.10f",
                      command_power);
        command_power = ServerParam::i().maxPower();
    }

    if ( command_power < 0.0 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " negative kick power " << command_power
                  << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__": (setKick) negative power. %.1f",
                      command_power );
        command_power = 0.0;
    }

    command_power = rint( command_power * 1000.0 ) * 0.001;

    dlog.addText( Logger::ACTION,
                   __FILE__": (setKick) Power=%.1f  Dir=%.1f  KickRate=%.4f  Accel=%.2f",
                  command_power, rel_dir.degree(), M_agent.world().self().kickRate(),
                  M_agent.world().self().kickRate() * command_power );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }
    M_command_body = new PlayerKickCommand( command_power, rel_dir.degree() );

    // set estimated action effect
    M_kick_accel.setPolar( command_power * M_agent.world().self().kickRate(),
                           M_agent.world().self().body() + rel_dir );

    // rcssserver/src/player.cc
    //  add noise to kick
    //  Value maxrnd = kick_rand * power / ServerParam::instance().maxp;
    //  PVector kick_noise(drand(-maxrnd,maxrnd),drand(-maxrnd,maxrnd));
    //  ball->push(kick_noise);

    // double mincos, maxcos, minsin, maxsin;
    // get_con_min_max(body_dir, body_err, &mincos, &maxcos);
    // get_sin_min_max(body_dir, body_err, &minsin, &maxsin);
    // M_kick_accel_error.assign((maxcos - mincos) * 0.5, (maxsin - minsin) * 0.5);
    // M_kick_accel_error *= M_kick_accel.r();
    // M_kick_accel_error.add(maxrnd, maxrnd);

    double maxrnd = ( M_agent.world().self().playerType().kickRand()
                      * command_power
                      / ServerParam::i().maxPower() );
    M_kick_accel_error.assign( maxrnd, maxrnd );

    dlog.addText( Logger::SYSTEM,
                   __FILE__": (setKick) accel=(%f, %f) err=(%f, %f)",
                  M_kick_accel.x, M_kick_accel.y,
                  M_kick_accel_error.x, M_kick_accel_error.y );
}


namespace {
/*-------------------------------------------------------------------*/
/*!
  \brief conserve dash power
  this method is used only from thie file.
*/
double
conserve_dash_power( const WorldModel & world,
                     double power,
                     double rel_dir )
{
    const ServerParam & param = ServerParam::i();

    power = param.normalizeDashPower( power );
    rel_dir = param.discretizeDashAngle( param.normalizeDashAngle( rel_dir ) );

    const bool back_dash = ( power < 0.0 ? true : false );
    const double required_stamina = ( back_dash
                                      ? power * -2.0
                                      : power );
    if ( required_stamina < 0.0 )
    {
        std::cerr << world.teamName() << ' '
                  << world.self().unum() << ": "
                  << world.time()
                  << " (conserve_dash_power) dash power should be positive now"
                  << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__" (conserve_dash_power) dash power should be positive now" );
    }

    const double available_stamina
        = world.self().stamina()
        + world.self().playerType().extraStamina();

    // insufficient stamina
    if ( available_stamina < required_stamina )
    {
        dlog.addText( Logger::ACTION,
                       __FILE__" (conserve_dash_power) no stamina. power = %.1f. stamina = %.1f",
                      power, available_stamina );
        power = available_stamina;
        if ( back_dash )
        {
            power *= -0.5;
        }
    }

    double dir_rate = param.dashDirRate( rel_dir );
    double accel_mag = std::fabs( power * dir_rate * world.self().dashRate() );

    if ( back_dash )
    {
        rel_dir += 180.0;
    }

    AngleDeg accel_angle = world.self().body() + rel_dir;

    // if player can keep max speed without max power,
    // conserve dash power.
    world.self().playerType().normalizeAccel( world.self().vel(),
                                              accel_angle,
                                              &accel_mag );

    power = accel_mag / world.self().dashRate() / dir_rate;

    if ( back_dash )
    {
        power *= -1.0;
    }
    dlog.addText( Logger::ACTION,
                   __FILE__" (conserve_dash_power) conserved power = %.1f",
                  power );

    power = param.normalizeDashPower( power );

    return power;
}

}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setDash( const double & power )
{
    setDash( power, 0.0 );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setDash( const double & power,
                         const AngleDeg & rel_dir )
{
    const ServerParam & param = ServerParam::i();

    dlog.addText( Logger::ACTION,
                   __FILE__" (setDash) register dash. power=%.1f, dir=%.1f",
                  power, rel_dir.degree() );

    //
    // normalize command argument: power
    //

    double command_power = power;

    if ( command_power < param.minDashPower() - 0.01
         || param.maxDashPower() + 0.01 < command_power )
    {
        dlog.addText( Logger::ACTION,
                       __FILE__" (setDash) over dash power range" );
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " over dash power range: " << command_power
                  << std::endl;
        command_power = param.normalizeDashPower( command_power );
    }

    //
    // normalize command argument: direction
    //

    double command_dir = rel_dir.degree();

    if ( command_dir < param.minDashAngle() - 0.01
         || param.maxDashAngle() + 0.01 < command_dir )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__" (setDash) over dash angle range %.1f",
                      rel_dir.degree() );
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " over dash angle range: "
                  << command_dir << std::endl;
        command_dir = param.normalizeDashAngle( command_dir );
    }

    command_dir = param.discretizeDashAngle( command_dir );

    //
    // conserve dash power
    //

    command_power = conserve_dash_power( M_agent.world(), command_power, command_dir );
    command_power = rint( command_power * 1000.0 ) * 0.001;

    //
    // create command object
    //
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }
    M_command_body = new PlayerDashCommand( command_power, command_dir );

    //
    // set estimated command effect: accel magnitude
    //
    double dir_rate = ServerParam::i().dashDirRate( command_dir );
    double accel_mag = std::fabs( command_power * dir_rate * M_agent.world().self().dashRate() );
    accel_mag = std::min( accel_mag, param.playerAccelMax() );

    //
    // set estimated command effect: accel angle
    //

    AngleDeg accel_angle = M_agent.world().self().body() + command_dir;
    if ( command_power < 0.0 )
    {
        accel_angle += 180.0;
    }

    /*
     * accel_mag = std::min( effort * command_power * mydprate,
     *                       ServerParam::playerAccelMax() );
     */

    M_dash_power = command_power;
    M_dash_dir = command_dir;
    M_dash_accel.setPolar( accel_mag, accel_angle );

#if 0
    double mincos, maxcos, minsin, maxsin;
    M_agent.world().self().body().getCosMinMax( M_agent.world().self().faceError(), &mincos, &maxcos );
    M_agent.world().self().body().getSinMinMax( M_agent.world().self().faceError(), &minsin, &maxsin );

    // set only dir info, radius is 1.0
    M_dash_accel_error.assign( ( maxcos - mincos ) * 0.5,
                               ( maxsin - minsin ) * 0.5 );
    // set radius
    M_dash_accel_error *= accel_mag;

    // rcssserver/src/object.C
    // PVector MPObject::noise()
    //  {
    //    Value maxrnd = randp * vel.r() ;
    //    return PVector(drand(-maxrnd,maxrnd),drand(-maxrnd,maxrnd)) ;
    //  }
    //
    // vel += noise();
    //

    // player_rand : default value is 0.05
    M_dash_accel_error.add( accel_mag * ServerParam::i().playerRand(),
                            accel_mag * ServerParam::i().playerRand() );
#endif
    dlog.addText( Logger::SYSTEM,
                   __FILE__" (setDash) power=%.3f dir=%.3f, accel=(%.3f, %.3f) r=%.3f th=%.3f",
                  command_power, command_dir,
                  M_dash_accel.x, M_dash_accel.y,
                  accel_mag, accel_angle.degree() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setTurn( const AngleDeg & moment )
{
    const double my_speed = M_agent.world().self().vel().r();
    double command_moment = moment.degree();
    // required turn param
    command_moment
        *= ( 1.0
             + ( my_speed
                 * M_agent.world().self().playerType().inertiaMoment() ) );

    dlog.addText( Logger::ACTION,
                   __FILE__" (setTurn) register turn. moment=%.1f, cmd_param=%.1f, my_inertia=%.1f",
                  moment.degree(), command_moment,
                  M_agent.world().self().playerType().inertiaMoment() );

    // check parameter range
    if ( command_moment > ServerParam::i().maxMoment() )
    {
        dlog.addText( Logger::ACTION,
                       __FILE__" (setTurn) over max moment. moment=%.1f, command=%.1f",
                      moment.degree(), command_moment );
        command_moment = ServerParam::i().maxMoment();
    }
    if ( command_moment < ServerParam::i().minMoment() )
    {
        dlog.addText( Logger::ACTION,
                       __FILE__" (setTurn) under min moment. moment=%.1f, command=%.1f",
                      moment.degree(), command_moment );
        command_moment = ServerParam::i().minMoment();
    }

    command_moment = rint( command_moment * 1000.0 ) * 0.001;

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }

    // moment is a command param, not a real moment.
    M_command_body = new PlayerTurnCommand( command_moment );

    // set estimated action effect
    /*
      turn noise algorithm
      drand(h,l) returns the random value within [h,l].

      player_rand : default value is 0.05

      double r1 = vel.mod();
      double actual_moment
      = ((1.0 + drand(-player_rand, player_rand)) * moment/(1.0 + HP_inertia_moment * r1));
      angle_body = normalize_angle(angle_body + moment);
    */
    // convert to real moment
    M_turn_actual = command_moment
        / ( 1.0 + my_speed * M_agent.world().self().playerType().inertiaMoment() );
    M_turn_error = std::fabs( ServerParam::i().playerRand()
                              * M_turn_actual );

    dlog.addText( Logger::SYSTEM,
                   __FILE__" (setTurn) command_moment=%.2f. actual_turn=%.2f. error=%.2f",
                  command_moment, M_turn_actual, M_turn_error );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setMove( const double & x,
                         const double & y )

{
    dlog.addText( Logger::ACTION,
                  __FILE__" (setMove) register move. (%.1f, %.1f)",
                  x, y );

    double command_x = x;
    double command_y = y;

    // check move point.
    // move point must be in pitch.
    if ( std::fabs( command_y ) > ServerParam::i().pitchHalfWidth()
         || std::fabs( command_x ) > ServerParam::i().pitchHalfLength() )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " Must move to a place in the pitch ("
                  << command_x << ", " << command_y << ")" << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__" (setMove) (%.1f, %.1f). must move to a place in pitch",
                      command_x, command_y );
        command_x = min_max( - ServerParam::i().pitchHalfLength(),
                             command_x,
                             ServerParam::i().pitchHalfLength() );
        command_y = min_max( - ServerParam::i().pitchHalfWidth(),
                             command_y,
                             ServerParam::i().pitchHalfWidth() );
    }

    // when kickoff & kickoff offside is enabled,
    // move point must be in our side
    if ( ServerParam::i().kickoffOffside()
         && command_x > 0.0 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " Must move to a place in our half ("
                  << command_x << ", " << command_y << ")" << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__" (setMove) (%.1f, %.1f). must move to a place in our half",
                      command_x, command_y );
        command_x = -0.1;
    }

    // when goalie catch mode,
    // move point must be in our penalty area
    if ( M_agent.world().gameMode().type() == GameMode::GoalieCatch_
         && M_agent.world().gameMode().side() == M_agent.world().ourSide() )
    {
        if ( command_x < -ServerParam::i().pitchHalfLength() + 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(1) ("
                      << command_x << ", " << command_y << ")" << std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__" (setMove) (%.1f, %.1f). must move to a place in penalty area(1)",
                          command_x, command_y );
            command_x = - ServerParam::i().pitchHalfLength() + 1.0;
        }
        if ( command_x > - ServerParam::i().ourPenaltyAreaLineX() - 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(2) ("
                      << command_x << ", " << command_y << ")"<< std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__" (setMove) (%.1f, %.1f). must move to a place in penalty area(2)",
                          command_x, command_y );
            command_x = - ServerParam::i().ourPenaltyAreaLineX() - 1.0;
        }
        if ( command_y > ServerParam::i().penaltyAreaHalfWidth() - 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(3) ("
                      << command_x << ", " << command_y << ")"<< std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__" (setMove) (%.1f, %.1f). must move to a place in penalty area(3)",
                          command_x, command_y );
            command_y = ServerParam::i().penaltyAreaHalfWidth() - 1.0;
        }
        if ( command_y < - ServerParam::i().penaltyAreaHalfWidth() + 1.0 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << " Must move to a place within penalty area(4) ("
                      << command_x << ", " << command_y << ")"<< std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__" (setMove) (%.1f, %.1f). must move to a place in penalty area(4)",
                          command_x, command_y );
            command_y = - ServerParam::i().penaltyAreaHalfWidth() + 1.0;
        }
    }

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }
    M_command_body = new PlayerMoveCommand( command_x, command_y );

    M_move_pos.assign( command_x, command_y );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setCatch()
{
    dlog.addText( Logger::ACTION,
                  __FILE__" (setCatch) register catch" );

    // catch area is rectangle.
    // "real" catchable length is the length of diagonal line.
    const double diagonal_angle
        = AngleDeg::atan2_deg( ServerParam::i().catchAreaWidth() * 0.5,
                               ServerParam::i().catchAreaLength() );

    const AngleDeg ball_rel_angle = M_agent.world().ball().angleFromSelf() - M_agent.world().self().body();
    // add diagonal angle
    AngleDeg catch_angle = ( ball_rel_angle.degree() > 0.0
                             ? ball_rel_angle - diagonal_angle
                             : ball_rel_angle + diagonal_angle );

    dlog.addText( Logger::ACTION,
                   __FILE__" (setCatch) (raw) ball_angle=%.1f diagonal_angle=%.1f catch_angle=%.1f",
                  ball_rel_angle.degree(),
                  diagonal_angle,
                  catch_angle.degree() );

    if ( catch_angle.degree() < ServerParam::i().minCatchAngle() )
    {
        catch_angle = ServerParam::i().minCatchAngle();
    }

    if ( catch_angle.degree() > ServerParam::i().maxCatchAngle() )
    {
        catch_angle = ServerParam::i().maxCatchAngle();
    }

    dlog.addText( Logger::ACTION,
                   __FILE__" (setCatch) (result) catch_angle=%.1f(gloabl=%.1f)",
                  catch_angle.degree(),
                  ( catch_angle + M_agent.world().self().body() ).degree() );


    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }
    M_command_body = new PlayerCatchCommand( catch_angle.degree() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setTackle( const double & power_or_dir,
                           const bool foul )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setTackle) register tackle. power_or_dir=%.1f foul=%s",
                  power_or_dir,
                  foul ? "on" : "off" );

    double actual_power_or_dir = power_or_dir;

    if ( M_agent.config().version() >= 12.0 )
    {
        if ( actual_power_or_dir < -180.0 || 180.0 < actual_power_or_dir )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << "tackle dir over the range. dir=" << actual_power_or_dir
                      << std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__": (setTackle) dir over. %f",
                          actual_power_or_dir );
        }
        actual_power_or_dir = AngleDeg::normalize_angle( power_or_dir );
    }
    else
    {
        if ( actual_power_or_dir > ServerParam::i().maxTacklePower() + 0.01 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << "tackle power overflow. com=" << actual_power_or_dir
                      << " > sparam=" << ServerParam::i().maxTacklePower()
                      << std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__": (setTackle) power over. %f",
                          actual_power_or_dir );
            actual_power_or_dir = ServerParam::i().maxTacklePower();
        }

        if ( actual_power_or_dir < - ServerParam::i().maxBackTacklePower() - 0.01 )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << ": "
                      << M_agent.world().time()
                      << "tackle power underflow " << actual_power_or_dir
                      << std::endl;
            dlog.addText( Logger::ACTION,
                           __FILE__": (setTackle) power underflow. %f",
                          actual_power_or_dir );
            actual_power_or_dir = ServerParam::i().minPower();
        }
    }

    actual_power_or_dir = rint( actual_power_or_dir * 1000.0 ) * 0.001;

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_body )
    {
        delete M_command_body;
        M_command_body = nullptr;
    }
    M_command_body = new PlayerTackleCommand( actual_power_or_dir, foul );

    // set estimated command effect
    M_tackle_power = actual_power_or_dir;
    if ( M_agent.config().version() >= 12.0 )
    {
        M_tackle_power = ServerParam::i().maxTacklePower();
        M_tackle_dir = actual_power_or_dir;
    }
    else
    {
        M_tackle_dir = ( actual_power_or_dir > 0.0
                         ? M_agent.world().self().body().degree()
                         : ( M_agent.world().self().body() + 180.0 ).degree() );
    }

    M_tackle_foul = foul;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setTurnNeck( const AngleDeg & moment )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setTurnNeck) register turn_neck. moment=%.1f",
                  moment.degree() );

    double command_moment = moment.degree();

    // adjust for neck moment range
    if ( command_moment > ServerParam::i().maxNeckMoment() + 0.01 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " (setTurnNeck) over max moment. "
                  << command_moment << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__" (setTurnNeck) over max moment. %.1f",
                      command_moment );
        command_moment = ServerParam::i().maxNeckMoment();
    }
    if ( command_moment < ServerParam::i().minNeckMoment() - 0.01 )
    {
        std::cerr << M_agent.world().teamName() << ' '
                  << M_agent.world().self().unum() << ": "
                  << M_agent.world().time()
                  << " (setTurnNeck) under min moment. "
                  << command_moment << std::endl;
        dlog.addText( Logger::ACTION,
                       __FILE__" (setTurnNeck) under min moment. %.1f",
                      command_moment );
        command_moment = ServerParam::i().minNeckMoment();
    }

    command_moment = rint( command_moment );

    // adjust for neck angle range
    AngleDeg next_neck_angle = M_agent.world().self().neck();
    next_neck_angle += command_moment;

    if ( next_neck_angle.degree() > ServerParam::i().maxNeckAngle() )
    {
        command_moment = rint( ServerParam::i().maxNeckAngle()
                               - M_agent.world().self().neck().degree() );
        dlog.addText( Logger::ACTION,
                       __FILE__" (setTurnNeck) next_neck= %.1f. over max. new-moment= %.1f",
                      next_neck_angle.degree(), command_moment );
    }

    if ( next_neck_angle.degree() < ServerParam::i().minNeckAngle() )
    {
        command_moment = rint( ServerParam::i().minNeckAngle()
                               - M_agent.world().self().neck().degree() );
        dlog.addText( Logger::ACTION,
                       __FILE__" (setTurnNeck) next_neck= %.1f. under min. new-momment= %.1f",
                      next_neck_angle.degree(), command_moment );
    }

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_turn_neck )
    {
        delete M_command_turn_neck;
        M_command_turn_neck = nullptr;
    }
    M_command_turn_neck = new PlayerTurnNeckCommand( command_moment );

    // set estimated command effect
    M_turn_neck_moment = command_moment;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setChangeView( const ViewWidth & width )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setChangeView) register change_view. width= %d",
                  width.type() );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_change_view )
    {
        delete M_command_change_view;
        M_command_change_view = nullptr;
    }

    M_command_change_view = new PlayerChangeViewCommand( width,
                                                         ViewQuality::HIGH );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setChangeFocus( const double moment_dist,
                                const AngleDeg & moment_dir )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setChangeFocus) register change_focus. moment_dist=%lf moment_dir=%lf",
                  moment_dist, moment_dir );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_change_focus )
    {
        delete M_command_change_focus;
        M_command_change_focus = nullptr;
    }

    double command_moment_dist = rint( moment_dist * 1000.0 ) * 0.001;
    double command_moment_dir = rint( moment_dir.degree() * 1000.0 ) * 0.001;

    M_command_change_focus = new PlayerChangeFocusCommand( command_moment_dist, command_moment_dir );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::addSayMessage( SayMessage * message )
{
    if ( ! message )
    {
        dlog.addText( Logger::ACTION,
                      __FILE__" (addSayMessage) NULL message" );
        std::cerr << __FILE__ << ' '<< __LINE__
                  << ": (addSayMessage) NULL message." << std::endl;
        return;
    }

    dlog.addText( Logger::ACTION,
                  __FILE__" (addSayMessage) add new say message.[%c]",
                  message->header() );

    SayMessage::Ptr ptr( message );

    M_say_message_cont.push_back( ptr );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ActionEffector::removeSayMessage( const char header )
{
    dlog.addText( Logger::ACTION,
                  __FILE__" (removeSayMessage) header=[%c]", header );

    bool removed = false;

    std::vector< SayMessage::Ptr >::iterator it = M_say_message_cont.begin();

    while ( it != M_say_message_cont.end() )
    {
        if ( (*it)->header() == header )
        {
            it = M_say_message_cont.erase( it );
            removed = true;
            dlog.addText( Logger::ACTION,
                          __FILE__" (removeSayMessage) removed" );
        }
        else
        {
            ++it;
        }
    }

    return removed;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::clearSayMessage()
{
    M_say_message_cont.clear();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setPointto( const double & x,
                            const double & y )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setPointto) register pointto. (%.2f, %.2f)",
                  x, y );

    Vector2D target_pos( x, y );
    Vector2D target_rel = target_pos - M_agent.world().self().pos();
    target_rel.rotate( - M_agent.world().self().face() );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = nullptr;
    }
    M_command_pointto = new PlayerPointtoCommand( target_rel.r(),
                                                  target_rel.th().degree() );

    // set estimated commadn effect
    M_pointto_pos = target_pos;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setPointtoOff()
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setPointtoOff) register pointto off" );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_pointto )
    {
        delete M_command_pointto;
        M_command_pointto = nullptr;
    }
    M_command_pointto = new PlayerPointtoCommand();

    // set estimated command effect
    M_pointto_pos.invalidate();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setAttentionto( const SideID side,
                                const int unum )
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setAttentionto) register attentionto. side= %d, unum= %d",
                  side, unum );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = nullptr;
    }

    M_command_attentionto
        = new PlayerAttentiontoCommand( ( M_agent.world().ourSide() == side
                                          ? PlayerAttentiontoCommand::OUR
                                          : PlayerAttentiontoCommand::OPP ),
                                        unum );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::setAttentiontoOff()
{
    dlog.addText( Logger::ACTION,
                   __FILE__" (setAttentiontoOff) register attentionto off" );

    //////////////////////////////////////////////////
    // create command object
    if ( M_command_attentionto )
    {
        delete M_command_attentionto;
        M_command_attentionto = nullptr;
    }
    M_command_attentionto = new PlayerAttentiontoCommand();
}

/*-------------------------------------------------------------------*/
/*!

*/
int
ActionEffector::getSayMessageLength() const
{
    int len = 0;

    for ( const SayMessage::Ptr & i : M_say_message_cont )
    {
        len += i->length();
    }

    return len;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
ActionEffector::makeSayCommand()
{
    if ( M_command_say )
    {
        delete M_command_say;
        M_command_say = nullptr;
    }

    M_say_message.erase();

    // std::sort( M_say_message_cont.begin(), M_say_message_cont.end(),
    //            SayMessagePtrSorter() );

    for ( const SayMessage::Ptr & i : M_say_message_cont )
    {
        if ( ! i->appendTo( M_say_message ) )
        {
            std::cerr << M_agent.world().teamName() << ' '
                      << M_agent.world().self().unum() << " : "
                      << M_agent.world().time() << " Error say message builder. type=["
                      << i->header() << ']'
                      << std::endl;
            dlog.addText( Logger::ACTION,
                          __FILE__" (makeSayCommand) error occured." );
        }
    }

    if ( M_say_message.empty() )
    {
        return;
    }

    M_command_say = new PlayerSayCommand( M_say_message,
                                          M_agent.config().version() );

    dlog.addText( Logger::ACTION,
                  __FILE__" (makeSayCommand) say message [%s]",
                  M_say_message.c_str() );
}

////////////////////////////////////////////////////////////////////////////////

/*-------------------------------------------------------------------*/
/*!

*/
AngleDeg
ActionEffector::queuedNextSelfBody() const
{
    AngleDeg next_angle = M_agent.world().self().body();
    if ( M_command_body
         && M_command_body->type() == PlayerCommand::TURN )
    {
        double moment = 0.0;
        getTurnInfo( &moment, NULL );
        next_angle += moment;
    }

    return next_angle;
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
ActionEffector::queuedNextSelfPos() const
{
    Vector2D vel = M_agent.world().self().vel();
    if ( M_command_body
         && M_command_body->type() == PlayerCommand::DASH )
    {
        Vector2D accel( 0.0, 0.0 );
        getDashInfo( &accel, NULL );
        vel += accel;

        double tmp = vel.r();
        if ( tmp > M_agent.world().self().playerType().playerSpeedMax() )
        {
            vel *= M_agent.world().self().playerType().playerSpeedMax() / tmp;
        }
    }

    return M_agent.world().self().pos() + vel;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ActionEffector::queuedNextBallKickable() const
{
    if ( M_agent.world().ball().rposCount() >= 3 )
    {
        return false;
    }

    Vector2D my_next = queuedNextSelfPos();
    Vector2D ball_next = queuedNextBallPos();

    return my_next.dist( ball_next )
        < M_agent.world().self().playerType().kickableArea() - 0.06;
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
ActionEffector::queuedNextBallPos() const
{
    if ( ! M_agent.world().ball().posValid() )
    {
        return Vector2D::INVALIDATED;
    }

    Vector2D vel( 0.0, 0.0 ), accel( 0.0, 0.0 );

    if ( M_agent.world().ball().velValid() )
    {
        vel = M_agent.world().ball().vel();
    }

    if ( M_command_body
         && M_command_body->type() == PlayerCommand::KICK )
    {
        getKickInfo( &accel, NULL );
    }

    vel += accel;

    return M_agent.world().ball().pos() + vel;
}

/*-------------------------------------------------------------------*/
/*!

*/
Vector2D
ActionEffector::queuedNextBallVel() const
{
    Vector2D vel( 0.0, 0.0 ), accel( 0.0, 0.0 );

    if ( M_agent.world().ball().velValid() )
    {
        vel = M_agent.world().ball().vel();
    }

    if ( M_command_body
         && M_command_body->type() == PlayerCommand::KICK )
    {
        getKickInfo( &accel, NULL );
    }

    vel += accel;
    vel *= ServerParam::i().ballDecay();
    return vel;
}

/*-------------------------------------------------------------------*/
/*!

*/
AngleDeg
ActionEffector::queuedNextAngleFromBody( const Vector2D & target ) const
{
    Vector2D next_rpos = target - queuedNextSelfPos();

    return next_rpos.th() - queuedNextSelfBody();
}

/*-------------------------------------------------------------------*/
/*!

*/
ViewWidth
ActionEffector::queuedNextViewWidth() const
{
    if ( M_command_change_view )
    {
        return M_command_change_view->width();
    }

    return M_agent.world().self().viewWidth();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ActionEffector::queuedNextCanSeeWithTurnNeck( const Vector2D & point,
                                              const double & angle_buf ) const
{
    int see_cycle = queuedNextSeeCycles();
    if ( see_cycle > 1 )
    {
        return false;
    }

    Vector2D next_self_pos = queuedNextSelfPos();
    AngleDeg target_neck_angle = ( point - next_self_pos ).th() - queuedNextSelfBody();

    double view_half_width = queuedNextViewWidth().width() - std::max( 0.0, angle_buf );

    double neck_min = ServerParam::i().minNeckAngle() - view_half_width;
    double neck_max = ServerParam::i().maxNeckAngle() + view_half_width;

    return ( neck_min <= target_neck_angle.degree()
             && target_neck_angle.degree() <= neck_max );
}

/*-------------------------------------------------------------------*/
/*!

*/
int
ActionEffector::queuedNextSeeCycles() const
{
    int cycle = M_agent.seeState().cyclesTillNextSee();
    if ( cycle > 0 )
    {
        return cycle;
    }

    ViewWidth view_width = M_agent.world().self().viewWidth();
    if ( M_command_change_view )
    {
        view_width = M_command_change_view->width();
    }

    if ( SeeState::synch_see_mode() )
    {
        switch ( view_width.type() ) {
        case ViewWidth::WIDE:
            return 3;
        case ViewWidth::NORMAL:
            return 2;
        case ViewWidth::NARROW:
            return 1;
        default:
            break;
        }

        return 3;
    }

    const SeeState::Timing last_timing = M_agent.seeState().lastTiming();

    // case 1
    if ( last_timing == SeeState::TIME_0_00 )
    {
        switch ( view_width.type() ) {
        case ViewWidth::WIDE:
            return 3;
        case ViewWidth::NORMAL:
            return 1;
        case ViewWidth::NARROW:
        default:
            break;
        }

        return 3;
    }

    // case 2
    if ( last_timing == SeeState::TIME_50_0 )
    {
        switch ( view_width.type() ) {
        case ViewWidth::WIDE:
            return 3;
        case ViewWidth::NORMAL:
            return 2;
        case ViewWidth::NARROW:
            return 1;
        default:
            break;
        }

        return 3;
    }

    // case 3
    if ( last_timing == SeeState::TIME_22_5 )
    {
        switch ( view_width.type() ) {
        case ViewWidth::WIDE:
            return 3;
        case ViewWidth::NORMAL:
            break;
        case ViewWidth::NARROW:
            return 1;
        default:
            break;
        }

        return 3;
    }

    // no synchronization...
    switch ( view_width.type() ) {
    case ViewWidth::WIDE:
        return 3;
    case ViewWidth::NORMAL:
        return 2;
    case ViewWidth::NARROW:
        return 1;
    default:
        break;
    }

    return 3;
}

}
