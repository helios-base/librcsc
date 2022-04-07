// -*-c++-*-

/*!
  \file formation_static.cpp
  \brief static type formation method classes Source File.
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

#include "formation_static.h"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <cstdio>

namespace rcsc {

const std::string FormationStatic::NAME( "Static" );

/*-------------------------------------------------------------------*/
FormationStatic::FormationStatic()
    : Formation()
{

}

/*-------------------------------------------------------------------*/
std::string
FormationStatic::methodName() const
{
    return NAME;
}

/*-------------------------------------------------------------------*/
Vector2D
FormationStatic::getPosition( const int num,
                              const Vector2D & ) const
{
    if ( num < 1 || 11 < num )
    {
        std::cerr << "(FormationStatic::getPosition) Invalid position number " << num << std::endl;
        return Vector2D( 0.0, 0.0 );
    }

    return M_positions[num - 1];
}

/*-------------------------------------------------------------------*/
void
FormationStatic::getPositions( const Vector2D & focus_point,
                               std::vector< Vector2D > & positions ) const
{
    positions.clear();

    for ( int num = 1; num <= 11; ++num )
    {
        positions.push_back( getPosition( num, focus_point ) );
    }
}

/*-------------------------------------------------------------------*/
bool
FormationStatic::train( const FormationData & data )
{
    //std::cerr << "Static method never support any training" << std::endl;

    if ( data.dataCont().empty() )
    {
        return false;
    }

    if ( data.dataCont().size() > 1 )
    {
        std::cerr << "(FormationStatic::train) too many data. size = " << data.dataCont().size() << std::endl;
    }

    if ( data.dataCont().front().players_.size() != M_positions.size() )
    {
        std::cerr << "(FormationStatic::train) Invalid player array size " << std::endl;
        return false;
    }

    for ( size_t i = 0; i < M_positions.size(); ++i )
    {
        M_positions[i] = data.dataCont().front().players_[i];
    }

    return true;
}

/*-------------------------------------------------------------------*/
FormationData::Ptr
FormationStatic::toData() const
{
    FormationData::Ptr ptr( new FormationData() );

    FormationData::Data d;

    d.ball_.assign( 0.0, 0.0 );
    for ( size_t i = 0; i < 11; ++i )
    {
        d.players_.push_back( M_positions[i] );
    }

    ptr->addData( d );

    return ptr;
}

/*-------------------------------------------------------------------*/
namespace {
const std::string tab = "  ";
}

/*-------------------------------------------------------------------*/
bool
FormationStatic::printData( std::ostream & os ) const
{
    os << tab << "\"data\"" << " : [\n";
    os << tab << tab << "{\n";
    os << tab << tab << tab << "\"index\" : " << 0 << ",\n";
    os << tab << tab << tab << "\"ball\" : { \"x\" :   0.00, \"y\" :   0.00 }";

    char buf[128];
    for ( size_t i = 0; i < M_positions.size(); ++i )
    {
        os << ",\n";
        snprintf( buf, sizeof( buf ) - 1,
                  "  %s\"%zd\" : { \"x\" : % 6.2f, \"y\" : % 6.2f }",
                  ( i < 9 ? " " : "" ), i + 1, M_positions[i].x, M_positions[i].y );
        os << tab << tab << tab << buf;
        // os << tab << tab << tab;
        // os << '"' << i + 1 << '"' // number
        //    << " : { "
        //    << "\"x\" : " << M_positions[i].x << ", "
        //    << "\"y\" : " << M_positions[i].y
        //    << " }";
    }

    os << tab << tab << '}';
    os << "\n"
       << tab << "]";

    return true;
}

}
