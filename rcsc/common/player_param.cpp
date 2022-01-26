// -*-c++-*-

/*!
  \file player_param.cpp
  \brief player_param for rcssserver Source File
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

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include "player_param.h"

#include <rcsc/param/rcss_param_parser.h>
#include <rcsc/param/param_map.h>

#include <rcsc/rcg/util.h>
#include <rcsc/rcg/types.h>

#include <sstream>
#include <string>
#include <cassert>
#include <cmath>

namespace rcsc {

const int PlayerParam::DEFAULT_PLAYER_TYPES = 7;
const int PlayerParam::DEFAULT_SUBS_MAX = 3;
const int PlayerParam::DEFAULT_PT_MAX = 3;

//const bool PlayerParam::DEFAULT_ALLOW_MULT_DEFAULT_TYPE = false;
const bool PlayerParam::DEFAULT_ALLOW_MULT_DEFAULT_TYPE = true; // to keep the compatibility

const double PlayerParam::DEFAULT_PLAYER_SPEED_MAX_DELTA_MIN = 0.0;
const double PlayerParam::DEFAULT_PLAYER_SPEED_MAX_DELTA_MAX = 0.0;
const double PlayerParam::DEFAULT_STAMINA_INC_MAX_DELTA_FACTOR = 0.0;

// [13.0.0] -0.05 -> -0.1
// [12.0.0] 0.0 -> -0.05
const double PlayerParam::DEFAULT_PLAYER_DECAY_DELTA_MIN = -0.1;
const double PlayerParam::DEFAULT_PLAYER_DECAY_DELTA_MAX = 0.1; // [12.0.0] 0.2 -> 0.1
const double PlayerParam::DEFAULT_INERTIA_MOMENT_DELTA_FACTOR = 25.0;

const double PlayerParam::DEFAULT_DASH_POWER_RATE_DELTA_MIN = 0.0;
const double PlayerParam::DEFAULT_DASH_POWER_RATE_DELTA_MAX = 0.0;
const double PlayerParam::DEFAULT_PLAYER_SIZE_DELTA_FACTOR = -100.0;

const double PlayerParam::DEFAULT_KICKABLE_MARGIN_DELTA_MIN = -0.1; // [12.0.0] 0.0 -> -0.1;
const double PlayerParam::DEFAULT_KICKABLE_MARGIN_DELTA_MAX = 0.1; // [12.0.0 ]0.2 -> 0.1
const double PlayerParam::DEFAULT_KICK_RAND_DELTA_FACTOR = 1.0; // [12.0.0] 0.5 -> 1.0

const double PlayerParam::DEFAULT_EXTRA_STAMINA_DELTA_MIN = 0.0;
const double PlayerParam::DEFAULT_EXTRA_STAMINA_DELTA_MAX = 50.0; // [13.0.0] 100.0 -> 50.0
const double PlayerParam::DEFAULT_EFFORT_MAX_DELTA_FACTOR = -0.004; // [13.0.0] -0.002 -> -0.004
const double PlayerParam::DEFAULT_EFFORT_MIN_DELTA_FACTOR = -0.004; // [13.0.0] -0.002 -> -0.004

const int    PlayerParam::DEFAULT_RANDOM_SEED = -1; // negative means generate a new seed

// [13.0.0] -0.0005 -> -0.0012
// [12.0.0]  0      -> -0.0005
const double PlayerParam::DEFAULT_NEW_DASH_POWER_RATE_DELTA_MIN = -0.0012;
// [13.0.0] 0.0015 -> 0.0008
// [12.0.0] 0.002  -> 0.0015
const double PlayerParam::DEFAULT_NEW_DASH_POWER_RATE_DELTA_MAX = 0.0008;
// [12.0.0] -10000.0 -> -6000.0
const double PlayerParam::DEFAULT_NEW_STAMINA_INC_MAX_DELTA_FACTOR = -6000.0;

// v14
const double PlayerParam::DEFAULT_KICK_POWER_RATE_DELTA_MIN = 0.0;
const double PlayerParam::DEFAULT_KICK_POWER_RATE_DELTA_MAX = 0.0;
const double PlayerParam::DEFAULT_FOUL_DETECT_PROBABILITY_DELTA_FACTOR = 0.0;

const double PlayerParam::DEFAULT_CATCHABLE_AREA_L_STRETCH_MIN = 1.0;
const double PlayerParam::DEFAULT_CATCHABLE_AREA_L_STRETCH_MAX = 1.3;


/*-------------------------------------------------------------------*/
/*!

*/
PlayerParam &
PlayerParam::instance()
{
    static PlayerParam S_instance;
    return S_instance;
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerParam::PlayerParam()
    : M_param_map( new ParamMap( "player_param" ) )
{
    assert( M_param_map );

    setDefaultParam();
    createMap();
}

/*-------------------------------------------------------------------*/
/*!

*/
PlayerParam::~PlayerParam()
{
    //std::cerr << "delete PlayerParam" << std::endl;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerParam::setDefaultParam()
{
    M_player_types = DEFAULT_PLAYER_TYPES;
    M_subs_max = DEFAULT_SUBS_MAX;
    M_pt_max = DEFAULT_PT_MAX;

    M_allow_mult_default_type = DEFAULT_ALLOW_MULT_DEFAULT_TYPE;

    M_player_speed_max_delta_min = DEFAULT_PLAYER_SPEED_MAX_DELTA_MIN;
    M_player_speed_max_delta_max = DEFAULT_PLAYER_SPEED_MAX_DELTA_MAX;
    M_inertia_moment_delta_factor = DEFAULT_INERTIA_MOMENT_DELTA_FACTOR;

    M_dash_power_rate_delta_min = DEFAULT_DASH_POWER_RATE_DELTA_MIN;
    M_dash_power_rate_delta_max = DEFAULT_DASH_POWER_RATE_DELTA_MAX;
    M_player_size_delta_factor = DEFAULT_PLAYER_SIZE_DELTA_FACTOR;

    M_kickable_margin_delta_min = DEFAULT_KICKABLE_MARGIN_DELTA_MIN;
    M_kickable_margin_delta_max = DEFAULT_KICKABLE_MARGIN_DELTA_MAX;
    M_kick_rand_delta_factor = DEFAULT_KICK_RAND_DELTA_FACTOR;

    M_extra_stamina_delta_min = DEFAULT_EXTRA_STAMINA_DELTA_MIN;
    M_extra_stamina_delta_max = DEFAULT_EXTRA_STAMINA_DELTA_MAX;
    M_effort_max_delta_factor = DEFAULT_EFFORT_MAX_DELTA_FACTOR;
    M_effort_min_delta_factor = DEFAULT_EFFORT_MIN_DELTA_FACTOR;

    M_new_dash_power_rate_delta_min = DEFAULT_NEW_DASH_POWER_RATE_DELTA_MIN;
    M_new_dash_power_rate_delta_max = DEFAULT_NEW_DASH_POWER_RATE_DELTA_MAX;
    M_new_stamina_inc_max_delta_factor = DEFAULT_NEW_STAMINA_INC_MAX_DELTA_FACTOR;

    M_random_seed = DEFAULT_RANDOM_SEED;

    M_kick_power_rate_delta_min = DEFAULT_KICK_POWER_RATE_DELTA_MIN;
    M_kick_power_rate_delta_max = DEFAULT_KICK_POWER_RATE_DELTA_MAX;
    M_foul_detect_probability_delta_factor = DEFAULT_FOUL_DETECT_PROBABILITY_DELTA_FACTOR;

    M_catchable_area_l_stretch_min = DEFAULT_CATCHABLE_AREA_L_STRETCH_MIN;
    M_catchable_area_l_stretch_max = DEFAULT_CATCHABLE_AREA_L_STRETCH_MAX;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerParam::createMap()
{
    M_param_map->add()
        ( "player_types", "", &M_player_types )
        ( "subs_max", "", &M_subs_max )
        ( "pt_max", "", &M_pt_max )
        ( "allow_mult_default_type", "", &M_allow_mult_default_type )
        ( "player_speed_max_delta_min", "", &M_player_speed_max_delta_min )
        ( "player_speed_max_delta_max", "", &M_player_speed_max_delta_max )
        ( "stamina_inc_max_delta_factor", "", &M_stamina_inc_max_delta_factor )
        ( "player_decay_delta_min", "", &M_player_decay_delta_min )
        ( "player_decay_delta_max", "", &M_player_decay_delta_max )
        ( "inertia_moment_delta_factor", "", &M_inertia_moment_delta_factor )
        ( "dash_power_rate_delta_min", "", &M_dash_power_rate_delta_min )
        ( "dash_power_rate_delta_max", "", &M_dash_power_rate_delta_max )
        ( "player_size_delta_factor", "", &M_player_size_delta_factor )
        ( "kickable_margin_delta_min", "", &M_kickable_margin_delta_min )
        ( "kickable_margin_delta_max", "", &M_kickable_margin_delta_max )
        ( "kick_rand_delta_factor", "", &M_kick_rand_delta_factor )
        ( "extra_stamina_delta_min", "", &M_extra_stamina_delta_min )
        ( "extra_stamina_delta_max", "", &M_extra_stamina_delta_max )
        ( "effort_max_delta_factor", "", &M_effort_max_delta_factor )
        ( "effort_min_delta_factor", "", &M_effort_min_delta_factor )
        ( "random_seed", "", &M_random_seed )
        ( "new_dash_power_rate_delta_min", "", &M_new_dash_power_rate_delta_min )
        ( "new_dash_power_rate_delta_max", "", &M_new_dash_power_rate_delta_max )
        ( "new_stamina_inc_max_delta_factor", "", &M_new_stamina_inc_max_delta_factor )
        ( "kick_power_rate_delta_min", "", &M_kick_power_rate_delta_min )
        ( "kick_power_rate_delta_max", "", &M_kick_power_rate_delta_max )
        ( "foul_detect_probability_delta_factor", "", &M_foul_detect_probability_delta_factor )
        ( "catchable_area_l_stretch_min", "", &M_catchable_area_l_stretch_min )
        ( "catchable_area_l_stretch_max", "", &M_catchable_area_l_stretch_max )
        ;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerParam::parse( const char * msg,
                    const double & version )
{
    //std::cerr << "parse player_param\n";
    if ( version >= 8.0 )
    {
        // read v8 or later protocol
        RCSSParamParser parser( msg );
        return parser.parse( *M_param_map );
    }
    else
    {
        // read v7 protocol
        // read only param value, no param name
        return parseV7( msg );
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
PlayerParam::parseV7( const char * msg )
{
    /*
    // from rcssserver/src/playerparam.C
    std::ostream& toStr ( std::ostream& o, const PlayerParamSensor_v7::data_t& data )
    {
    return o << "(player_param "
    << data.M_player_types << " "
    << data.M_subs_max << " "
    << data.M_pt_max << " "

    << data.M_player_speed_max_delta_min << " "
    << data.M_player_speed_max_delta_max << " "
    << data.M_stamina_inc_max_delta_factor << " "

    << data.M_player_decay_delta_min << " "
    << data.M_player_decay_delta_max << " "
    << data.M_inertia_moment_delta_factor << " "

    << data.M_dash_power_rate_delta_min << " "
    << data.M_dash_power_rate_delta_max << " "
    << data.M_player_size_delta_factor << " "

    << data.M_kickable_margin_delta_min << " "
    << data.M_kickable_margin_delta_max << " "
    << data.M_kick_rand_delta_factor << " "

    << data.M_extra_stamina_delta_min << " "
    << data.M_extra_stamina_delta_max << " "
    << data.M_effort_max_delta_factor << " "
    << data.M_effort_min_delta_factor
    << ")";
    }
    */

    std::istringstream msg_strm( msg );
    std::string tmp;

    msg_strm >> tmp // "(player_param"
             >> M_player_types
             >> M_subs_max
             >> M_pt_max

             >> M_player_speed_max_delta_min
             >> M_player_speed_max_delta_max
             >> M_stamina_inc_max_delta_factor

             >> M_player_decay_delta_min
             >> M_player_decay_delta_max
             >> M_inertia_moment_delta_factor

             >> M_dash_power_rate_delta_min
             >> M_dash_power_rate_delta_max
             >> M_player_size_delta_factor

             >> M_kickable_margin_delta_min
             >> M_kickable_margin_delta_max
             >> M_kick_rand_delta_factor

             >> M_extra_stamina_delta_min
             >> M_extra_stamina_delta_max
             >> M_effort_max_delta_factor
             >> M_effort_min_delta_factor;

    return msg_strm.good();
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerParam::convertFrom( const rcg::player_params_t & from )
{
    M_player_types = rcg::nstohi( from.player_types );
    M_subs_max = rcg::nstohi( from.substitute_max );
    M_pt_max = rcg::nstohi( from.pt_max );

    M_player_speed_max_delta_min = rcg::nltohd( from.player_speed_max_delta_min );
    M_player_speed_max_delta_max = rcg::nltohd( from.player_speed_max_delta_max );
    M_stamina_inc_max_delta_factor = rcg::nltohd( from.stamina_inc_max_delta_factor );

    M_player_decay_delta_min = rcg::nltohd( from.player_decay_delta_min );
    M_player_decay_delta_max = rcg::nltohd( from.player_decay_delta_max );
    M_inertia_moment_delta_factor = rcg::nltohd( from.inertia_moment_delta_factor );

    M_dash_power_rate_delta_min = rcg::nltohd( from.dash_power_rate_delta_min );
    M_dash_power_rate_delta_max = rcg::nltohd( from.dash_power_rate_delta_max );
    M_player_size_delta_factor = rcg::nltohd( from.player_size_delta_factor );

    M_kickable_margin_delta_min = rcg::nltohd( from.kickable_margin_delta_min );
    M_kickable_margin_delta_max = rcg::nltohd( from.kickable_margin_delta_max );
    M_kick_rand_delta_factor = rcg::nltohd( from.kick_rand_delta_factor );

    M_extra_stamina_delta_min = rcg::nltohd( from.extra_stamina_delta_min );
    M_extra_stamina_delta_max = rcg::nltohd( from.extra_stamina_delta_max );
    M_effort_max_delta_factor = rcg::nltohd( from.effort_max_delta_factor );
    M_effort_min_delta_factor = rcg::nltohd( from.effort_min_delta_factor );

    M_random_seed = static_cast< int >( static_cast< rcg::Int32 >( ntohl( from.random_seed ) ) );

    M_new_dash_power_rate_delta_min = rcg::nltohd( from.new_dash_power_rate_delta_min );
    M_new_dash_power_rate_delta_max = rcg::nltohd( from.new_dash_power_rate_delta_max );
    M_new_stamina_inc_max_delta_factor = rcg::nltohd( from.new_stamina_inc_max_delta_factor );

    M_allow_mult_default_type = rcg::nstohb( from.allow_mult_default_type );

    M_kick_power_rate_delta_min = rcg::nltohd( from.kick_power_rate_delta_min );
    M_kick_power_rate_delta_max = rcg::nltohd( from.kick_power_rate_delta_max );
    M_foul_detect_probability_delta_factor = rcg::nltohd( from.foul_detect_probability_delta_factor );

    M_catchable_area_l_stretch_min = rcg::nltohd( from.catchable_area_l_stretch_min );
    M_catchable_area_l_stretch_max = rcg::nltohd( from.catchable_area_l_stretch_max );
}

/*-------------------------------------------------------------------*/
/*!

*/
void
PlayerParam::convertTo( rcg::player_params_t & to ) const
{
    to.player_types = rcg::hitons( M_player_types  );
    to.substitute_max  = rcg::hitons( M_subs_max  );
    to.pt_max = rcg::hitons( M_pt_max );

    to.player_speed_max_delta_min = rcg::hdtonl( M_player_speed_max_delta_min );
    to.player_speed_max_delta_max = rcg::hdtonl( M_player_speed_max_delta_max );
    to.stamina_inc_max_delta_factor = rcg::hdtonl( M_stamina_inc_max_delta_factor );;

    to.player_decay_delta_min = rcg::hdtonl( M_player_decay_delta_min );
    to.player_decay_delta_max = rcg::hdtonl( M_player_decay_delta_max );
    to.inertia_moment_delta_factor = rcg::hdtonl( M_inertia_moment_delta_factor );

    to.dash_power_rate_delta_min = rcg::hdtonl( M_dash_power_rate_delta_min );
    to.dash_power_rate_delta_max = rcg::hdtonl( M_dash_power_rate_delta_max );
    to.player_size_delta_factor = rcg::hdtonl( M_player_size_delta_factor );

    to.kickable_margin_delta_min = rcg::hdtonl( M_kickable_margin_delta_min );
    to.kickable_margin_delta_max = rcg::hdtonl( M_kickable_margin_delta_max );
    to.kick_rand_delta_factor = rcg::hdtonl( M_kick_rand_delta_factor );

    to.extra_stamina_delta_min = rcg::hdtonl( M_extra_stamina_delta_min );
    to.extra_stamina_delta_max = rcg::hdtonl( M_extra_stamina_delta_max );
    to.effort_max_delta_factor = rcg::hdtonl( M_effort_max_delta_factor );
    to.effort_min_delta_factor = rcg::hdtonl( M_effort_min_delta_factor );

    to.random_seed = htonl( static_cast< rcg::Int32 >( M_random_seed ) );

    to.new_dash_power_rate_delta_min = rcg::hdtonl( M_new_dash_power_rate_delta_min );
    to.new_dash_power_rate_delta_max = rcg::hdtonl( M_new_dash_power_rate_delta_max );
    to.new_stamina_inc_max_delta_factor = rcg::hdtonl( M_new_stamina_inc_max_delta_factor );

    to.allow_mult_default_type = rcg::hbtons( M_allow_mult_default_type );

    to.kick_power_rate_delta_min = rcg::hdtonl( M_kick_power_rate_delta_min );
    to.kick_power_rate_delta_max = rcg::hdtonl( M_kick_power_rate_delta_max );
    to.foul_detect_probability_delta_factor = rcg::hdtonl( M_foul_detect_probability_delta_factor );

    to.catchable_area_l_stretch_min = rcg::hdtonl( M_catchable_area_l_stretch_min );
    to.catchable_area_l_stretch_max = rcg::hdtonl( M_catchable_area_l_stretch_max );
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
PlayerParam::toServerString() const
{
    std::ostringstream os;

    os << "(player_param ";

    for ( const auto & m : M_param_map->longNameMap() )
    {
        os << '(' << m.second->longName() << ' ';
        m.second->printValue( os );
        os << ')';
    }

    os << ')';

    return os.str();
}

}
