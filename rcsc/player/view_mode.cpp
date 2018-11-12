// -*-c++-*-

/*!
  \file view_mode.cpp
  \brief player view mode data classes Source File
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

#include "view_mode.h"

#include "see_state.h"

#include <rcsc/common/server_param.h>

#include <iostream>
#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
const
ViewWidth &
ViewWidth::operator++()
{
    switch ( this->type() ) {
    case ViewWidth::NARROW:
        M_type = ViewWidth::NORMAL;
        break;
    case ViewWidth::NORMAL:
        M_type = ViewWidth::WIDE;
        break;
    case ViewWidth::WIDE:
        M_type = ViewWidth::ILLEGAL;
        break;
    default:
        M_type = ViewWidth::ILLEGAL;
        break;
    }

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
const
ViewWidth
ViewWidth::operator++( int )
{
    ViewWidth old = *this;
    switch ( this->type() ) {
    case ViewWidth::NARROW:
        M_type = ViewWidth::NORMAL;
        break;
    case ViewWidth::NORMAL:
        M_type = ViewWidth::WIDE;
        break;
    case ViewWidth::WIDE:
        M_type = ViewWidth::ILLEGAL;
        break;
    default:
        M_type = ViewWidth::ILLEGAL;
        break;
    }

    return old;
}


/*-------------------------------------------------------------------*/
/*!

*/
const
ViewWidth &
ViewWidth::operator--()
{
    switch ( this->type() ) {
    case ViewWidth::NARROW:
        M_type = ViewWidth::ILLEGAL;
        break;
    case ViewWidth::NORMAL:
        M_type = ViewWidth::NARROW;
        break;
    case ViewWidth::WIDE:
        M_type = ViewWidth::NORMAL;
        break;
    default:
        M_type = ViewWidth::WIDE;
        break;
    }

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

*/
const
ViewWidth
ViewWidth::operator--( int )
{
    ViewWidth old = *this;
    switch ( this->type() ) {
    case ViewWidth::NARROW:
        M_type = ViewWidth::ILLEGAL;
        break;
    case ViewWidth::NORMAL:
        M_type = ViewWidth::NARROW;
        break;
    case ViewWidth::WIDE:
        M_type = ViewWidth::NORMAL;
        break;
    default:
        M_type = ViewWidth::WIDE;
        break;
    }

    return old;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
ViewWidth::str() const
{
    switch ( this->type() ) {
    case ViewWidth::NARROW:
        return std::string( "narrow" );
    case ViewWidth::NORMAL:
        return std::string( "normal" );
    case ViewWidth::WIDE:
        return std::string( "wide" );
    default:
        break;
    }
    std::cerr << __FILE__ << ':' << __LINE__
              << " unknown view width detected." << std::endl;
    return std::string( "normal" );
}

/*-------------------------------------------------------------------*/
/*!

*/
double
ViewWidth::width( const ViewWidth::Type type )
{
    if ( SeeState::synch_see_mode() )
    {
        double r
            = static_cast< double >( ServerParam::i().simulatorStep() )
            / static_cast< double >( ServerParam::i().sendStep() );
        switch ( type ) {
        case ViewWidth::NARROW:
            return ServerParam::i().visibleAngle() * r;
        case ViewWidth::NORMAL:
            return ServerParam::i().visibleAngle() * 2.0 * r;
        case ViewWidth::WIDE:
            return ServerParam::i().visibleAngle() * 3.0 * r;
        default:
            break;
        }
    }
    else
    {
        switch ( type ) {
        case ViewWidth::NARROW:
            return ServerParam::i().visibleAngle() * 0.5;
        case ViewWidth::NORMAL:
            return ServerParam::i().visibleAngle();
        case ViewWidth::WIDE:
            return ServerParam::i().visibleAngle() * 2.0;
        default:
            break;
        }
    }

    std::cerr << __FILE__ << ':' << __LINE__
              << "Unknown View Width" << std::endl;

    return ServerParam::i().visibleAngle();
}

/*-------------------------------------------------------------------*/
/*!

*/
ViewWidth::Type
ViewWidth::parse( const char * msg )
{
    if ( ! std::strncmp( msg, "narrow", 6 ) )
    {
        return ViewWidth::NARROW;
    }
    else if ( ! std::strncmp( msg, "normal", 6 ) )
    {
        return ViewWidth::NORMAL;
    }
    else if ( ! std::strncmp( msg, "wide", 4 ) )
    {
        return ViewWidth::WIDE;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " view width parse error" << std::endl;
        return ViewWidth::ILLEGAL;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
ViewQuality::str() const
{
    switch ( M_type ) {
    case ViewQuality::HIGH:
        return std::string( "high" );
    case ViewQuality::LOW:
        return std::string( "low" );
    default:
        break;
    }
    std::cerr << __FILE__ << ':' << __LINE__
              << " unknown view quality detected." << std::endl;
    return std::string( "high" );
}

/*-------------------------------------------------------------------*/
/*!

*/
ViewQuality::Type
ViewQuality::parse( const char * msg )
{
    if ( ! std::strncmp( msg, "high", 4 ) )
    {
        return ViewQuality::HIGH;
    }
    else if ( ! std::strncmp( msg, "low", 3 ) )
    {
        return ViewQuality::LOW;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " view quality parse error" << std::endl;
        return ViewQuality::ILLEGAL;
    }
}

}
