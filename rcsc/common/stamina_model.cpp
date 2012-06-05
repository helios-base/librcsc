// -*-c++-*-

/*!
  \file stamina_model.cpp
  \brief player's stamina model Source File
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

#include "stamina_model.h"

#include "server_param.h"
#include "player_type.h"
// #include "logger.h"

#include <rcsc/game_time.h>

#include <algorithm>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
StaminaModel:: StaminaModel()
    : M_stamina( ServerParam::DEFAULT_STAMINA_MAX ),
      M_effort( ServerParam::DEFAULT_EFFORT_INIT ),
      M_recovery( ServerParam::DEFAULT_RECOVER_INIT ),
      M_capacity( -1.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::init( const PlayerType & player_type )
{
    M_stamina = ServerParam::i().staminaMax();
    M_effort = player_type.effortMax();
    M_recovery = ServerParam::i().recoverInit();
    M_capacity = ServerParam::i().staminaCapacity();
}

/*-------------------------------------------------------------------*/
/*!

*/
#if 0
void
StaminaModel::update( const PlayerType & player_type,
                      const double & dash_power )
{
    // substitute dash power from stamina value
    if ( dash_power >= 0.0 )
    {
        M_stamina -= dash_power;
    }
    else
    {
        M_stamina -= dash_power * -2.0;
    }

    // update recovery value
    if ( M_stamina <= ServerParam::i().recoverDecThrValue() )
    {
        if ( M_recovery > ServerParam::i().recoverMin() )
        {
            M_recovery = std::max( ServerParam::i().recoverMin(),
                                   M_recovery - ServerParam::i().recoverDec() );
            std::cerr << "recover dec " << M_recovery << std::endl;
        }
    }

    // update effort value
    // !!! using HETERO PLAYER PARAMS !!!
    if ( M_stamina <= ServerParam::i().effortDecThrValue() )
    {
        if ( M_effort > player_type.effortMin() )
        {
            M_effort = std::max( player_type.effortMin(),
                                 M_effort - ServerParam::i().effortDec() );
        }
    }
    else if ( M_stamina >= ServerParam::i().effortIncThrValue() )
    {
        if ( M_effort < player_type.effortMax() )
        {
            M_effort = std::min( player_type.effortMax(),
                                 M_effort + ServerParam::i().effortInc() );
        }
    }

    // recover stamina value & update stamina capacity

    double stamina_inc = std::min( player_type.staminaIncMax() * M_recovery,
                                   ServerParam::i().staminaMax() - M_stamina );
    if ( ServerParam::i().staminaCapacity() >= 0.0 )
    {
        M_stamina += std::min( stamina_inc, M_capacity );
        M_capacity -= stamina_inc;
        M_capacity = std::max( 0.0, M_capacity );
    }
    else
    {
        M_stamina += stamina_inc;
    }
    M_stamina = std::min( M_stamina, ServerParam::i().staminaMax() );
}
#endif

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::updateBySenseBody( const double sensed_stamina,
                                 const double sensed_effort,
                                 const double sensed_capacity,
                                 const GameTime & current )
{
    M_stamina = sensed_stamina;
    M_effort = sensed_effort;
    M_capacity = sensed_capacity;

    const ServerParam & SP = ServerParam::i();

    // reset recover value, when new harf start

    const int half_time = SP.actualHalfTime();
    const int normal_time = half_time * SP.nrNormalHalfs();

    if ( half_time >= 0 // server setting is normal game mode
         && SP.nrNormalHalfs() >= 0
         && current.cycle() < normal_time
         && current.cycle() % half_time == 1 ) // just after kickoff
    {
        M_recovery = SP.recoverInit();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
const StaminaModel &
StaminaModel::setValues( const double new_stamina,
                         const double new_effort,
                         const double new_recovery,
                         const double new_capacity )
{
    M_stamina = new_stamina;
    M_effort = new_effort;
    M_recovery = new_recovery;
    M_capacity = new_capacity;

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::simulateWait( const PlayerType & player_type )
{
    const ServerParam & SP = ServerParam::i();

    // check recovery
    if ( M_stamina <= SP.recoverDecThrValue() )
    {
        if ( M_recovery > SP.recoverMin() )
        {
            M_recovery -= SP.recoverDec();
            M_recovery = std::max( M_recovery, SP.recoverMin() );
        }
    }

    // check effort
    if ( M_stamina <= SP.effortDecThrValue() )
    {
        if ( M_effort > player_type.effortMin() )
        {
            M_effort -= SP.effortDec();
            M_effort = std::max( M_effort, player_type.effortMin() );
        }
    }
    else if ( M_stamina >= SP.effortIncThrValue() )
    {
        if ( M_effort < player_type.effortMax() )
        {
            M_effort += SP.effortInc();
            M_effort = std::min( M_effort, player_type.effortMax() );
        }
    }

    double stamina_inc = std::min( player_type.staminaIncMax() * M_recovery,
                                   SP.staminaMax() - M_stamina );
    if ( SP.staminaCapacity() >= 0.0 )
    {
        M_stamina += std::min( stamina_inc, M_capacity );
        M_capacity -= stamina_inc;
        M_capacity = std::max( 0.0, M_capacity );
    }
    else
    {
        M_stamina += stamina_inc;
    }
    M_stamina = std::min( M_stamina, SP.staminaMax() );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::simulateWaits( const PlayerType & player_type,
                             const int n_wait )
{
    for ( int i = 0; i < n_wait; ++i )
    {
        simulateWait( player_type );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::simulateDash( const PlayerType & player_type,
                            const double & dash_power )
{
    M_stamina -= ( dash_power >= 0.0
                   ? dash_power
                   : dash_power * -2.0 );
    M_stamina = std::max( 0.0, M_stamina );

    simulateWait( player_type );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::simulateDashes( const PlayerType & player_type,
                              const int n_dash,
                              const double & dash_power )
{
    const double consumption = ( dash_power >= 0.0
                                 ? dash_power
                                 : dash_power * -2.0 );

    for ( int i = 0; i < n_dash; ++i )
    {
        M_stamina -= consumption;
        M_stamina = std::max( 0.0, M_stamina );

        simulateWait( player_type );
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
void
StaminaModel::simulate( const PlayerType & player_type,
                        const int n_wait,
                        const int n_dash,
                        const double & dash_power )
{
    simulateWaits( player_type, n_wait );
    simulateDashes( player_type, n_dash, dash_power );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
StaminaModel::getSafetyDashPower( const PlayerType & player_type,
                                  const double dash_power,
                                  const double stamina_buffer ) const
{
    double normalized_power = ServerParam::i().normalizeDashPower( dash_power );

    double required_stamina = ( normalized_power > 0.0
                                ? normalized_power
                                : normalized_power * -2.0 );

    if ( required_stamina < 0.0 )
    {
        std::cerr << "required stamina should be positive. "
                  << " input dash power = " << dash_power
                  << " normalized power=" << normalized_power
                  << std::endl;
        // dlog.addText( Logger::ACTION,
        //               __FILE__" (getSafetyDashPower)"
        //               " input_dash_power=%.2f"
        //               " normalized_power=%.1f"
        //               " required_stamina=%.1f",
        //               dash_power,
        //               normalized_power,
        //               required_stamina );

    }


    double threshold = ( capacityIsEmpty()
                         ? -player_type.extraStamina()
                         : ServerParam::i().recoverDecThrValue() + std::max( stamina_buffer, 1.0 ) );
    double safety_stamina = stamina() - threshold;
    double available_stamina = std::max( 0.0, safety_stamina );
    double result_power = std::min( required_stamina, available_stamina );

    // backward dash case
    if ( normalized_power < 0.0 )
    {
        result_power *= -0.5;
    }

    if ( std::fabs( result_power ) > std::fabs( normalized_power ) )
    {
        return normalized_power;
    }

    return result_power;
}

}
